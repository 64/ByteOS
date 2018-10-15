#include <stdbool.h>
#include "smp.h"
#include "util.h"
#include "libk.h"
#include "mm.h"
#include "percpu.h"
#include "syscall.h"
#include "drivers/apic.h"
#include "drivers/pit.h"
#include "spin.h"

extern uintptr_t smp_trampoline_start;
extern uintptr_t smp_trampoline_end;

volatile bool smp_ap_started_flag;
volatile virtaddr_t smp_ap_stack;

atomic32_t smp_nr_cpus_ready = { 0 };

#define TRAMPOLINE_START 0x1000
// TODO: Add a define for the kernel stack size

// TODO: This is slow, but we store the value in the per-CPU data structure, so use that instead
cpuid_t smp_cpu_id_full(void)
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
}

// Boots all the cores
void smp_init(void)
{
	klog("smp", "CPU 0 online\n");
	atomic_inc_read32(&smp_nr_cpus_ready);

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

	// Wait for other CPUs to fully finish initialisation
	while (smp_nr_cpus() < lapic_list_size)
		;

	klog("smp", "Finished AP boot sequence\n");
}

void smp_ap_kmain(void)
{
	lapic_enable();
	irq_enable();
	syscall_enable();

	klog("smp", "CPU %u online\n", smp_cpu_id());
	atomic_inc_read32(&smp_nr_cpus_ready);
}
