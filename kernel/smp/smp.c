#include <stdbool.h>
#include "smp.h"
#include "util.h"
#include "libk.h"
#include "mm.h"
#include "percpu.h"
#include "drivers/apic.h"
#include "drivers/pit.h"
#include "spin.h"

extern uintptr_t smp_trampoline_start;
extern uintptr_t smp_trampoline_end;

volatile bool smp_ap_started_flag;
volatile virtaddr_t smp_ap_stack;

volatile unsigned int smp_nr_cpus_ready = 1;

#define TRAMPOLINE_START 0x1000
// TODO: Add a define for the kernel stack size

// This is slow, but we store the value in the per-CPU data structure, so use that instead
cpuid_t smp_cpu_id(void)
{
	cpuid_t lid = lapic_id();
	for (size_t i = 0; i < lapic_list_size; i++)
		if (lapic_list[i].id == lid)
			return i;
	panic("CPU not found in lapic_list");
}

static void smp_boot_ap(size_t index)
{
	struct lapic_info *lapic = &lapic_list[index];

	if (smp_ap_stack == NULL) {
		uintptr_t stack_top = (uintptr_t)page_to_virt(pmm_alloc_order(2, GFP_NONE));
		smp_ap_stack = (virtaddr_t)(stack_top + (4 * PAGE_SIZE));
	}

	// Set by the AP when initialisation is complete
	smp_ap_started_flag = 0;

	// Adapted from https://nemez.net/osdev/lapic.txt
	// Send the INIT IPI
	lapic_send_ipi(lapic->id, IPI_INIT);
	pit_sleep_ms(10);

	// Send the SIPI (first attempt)
	lapic_send_ipi(lapic->id, IPI_START_UP | ((uint32_t)TRAMPOLINE_START / PAGE_SIZE));
	pit_sleep_ms(5);

	if (!smp_ap_started_flag) {
		// Send SIPI again (second attempt)
		lapic_send_ipi(lapic->id, IPI_START_UP | ((uint32_t)TRAMPOLINE_START / PAGE_SIZE));
		pit_sleep_watch_flag(10, &smp_ap_started_flag, false);
		if (!smp_ap_started_flag) {
			klog_warn("smp", "CPU %zu failed to boot\n", index);
			lapic->present = 0;
			return;
		}
	}

	smp_ap_stack = NULL;

	// Check flag is set
	klog("smp", "CPU %zu online\n", index);
}

// Boots all the cores
void smp_init(void)
{
	klog("smp", "CPU 0 online\n");

	uintptr_t vstart = (uintptr_t)&smp_trampoline_start;
	uintptr_t vend = (uintptr_t)&smp_trampoline_end;
	physaddr_t trampoline_start = TRAMPOLINE_START;
	physaddr_t trampoline_end = trampoline_start + (vend - vstart);
	for (size_t i = 0; i < (trampoline_end - trampoline_start); i += PAGE_SIZE) {
		vmm_map_page(&kernel_mmu, trampoline_start + i, (virtaddr_t)(trampoline_start + i), PAGE_WRITABLE | PAGE_EXECUTABLE);
		memcpy((virtaddr_t)(trampoline_start + i), (virtaddr_t)(vstart + i), PAGE_SIZE);
	}

	for (size_t i = 1; i < lapic_list_size; i++)
		smp_boot_ap(i);

	// Unmap trampoline code from memory.
	// We rely on the fact that no CPU is going to access their
	// trampoline memory (before doing a cr3 reload to context switch).
	for (size_t i = 0; i < (trampoline_end - trampoline_start); i += PAGE_SIZE) {
		vmm_map_page(&kernel_mmu, 0, (virtaddr_t)(trampoline_start + i), VMM_UNMAP);
	}

	// Free any unused stacks if there were any
	if (smp_ap_stack != NULL) {
		uintptr_t stack_top = (uintptr_t)smp_ap_stack - (4 * PAGE_SIZE);
		pmm_free_order(virt_to_page((virtaddr_t)stack_top), 2);
	}

	klog("smp", "Finished AP boot sequence\n");
}

void smp_ap_kmain(void)
{
	lapic_enable();
	irq_enable();

	__atomic_add_fetch(&smp_nr_cpus_ready, 1, __ATOMIC_RELAXED);
}
