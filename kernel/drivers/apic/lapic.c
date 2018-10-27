#include <stdint.h>
#include <stdbool.h>
#include <cpuid.h>
#include "libk.h"
#include "mm.h"
#include "percpu.h"
#include "util.h"
#include "proc.h"
#include "interrupts.h"
#include "drivers/acpi.h"
#include "drivers/apic.h"
#include "drivers/pit.h"

#define APIC_CPUID_BIT (1 << 9)
#define APIC_DISABLE 0x10000U // Not exactly sure what this is, found it on the wiki
#define APIC_TIMER_PERIODIC 0x20000U

#define APIC_REG_ID 0x20
#define APIC_REG_VERSION 0x30
#define APIC_REG_EOI 0xB0
#define APIC_REG_SPURIOUS 0xF0U
#define APIC_REG_LINT0 0x350U
#define APIC_REG_LINT1 0x360U
#define APIC_REG_ICR0 0x300U
#define APIC_REG_ICR1 0x310U
#define APIC_REG_TIMER_LVT 0x320U
#define APIC_REG_TIMER_INITIAL 0x380U
#define APIC_REG_TIMER_CURRENT 0x390U
#define APIC_REG_TIMER_DIVIDE 0x3E0U
#define APIC_REG_ISR_BASE 0x100U

virtaddr_t lapic_base; // Shared by all CPUs

static bool has_lapic(void)
{
	uint32_t eax, ebx, ecx, edx = 0;
	__get_cpuid(1, &eax, &ebx, &ecx, &edx);
	return (edx & APIC_CPUID_BIT) != 0;
}

static inline void lapic_write(uint32_t reg_offset, uint32_t data)
{
	*(volatile uint32_t *)((uintptr_t)lapic_base + reg_offset) = data;
}

static inline uint32_t lapic_read(uint32_t reg_offset)
{
	return *(volatile uint32_t *)((uintptr_t)lapic_base + reg_offset);
}

static inline struct lapic_info *find_lapic(uint8_t id)
{
	for (size_t i = 0; i < lapic_list_size; i++)
		if (lapic_list[i].id == id)
			return &lapic_list[i];
	return NULL;
}

static inline void lapic_set_nmi(uint8_t vec, struct madt_entry_nmi *nmi_info)
{
	kassert_dbg(vec == IRQ_NMI);
	uint32_t nmi = 800 | vec;
	if (nmi_info->flags & 2)
		nmi |= (1 << 13);
	if (nmi_info->flags & 8)
		nmi |= (1 << 15);
	if (nmi_info->lint_num == 0)
		lapic_write(APIC_REG_LINT0, nmi);
	else if (nmi_info->lint_num == 1)
		lapic_write(APIC_REG_LINT1, nmi);
}

uint8_t lapic_id(void)
{
	return lapic_read(APIC_REG_ID) >> 24;
}

void lapic_eoi(uint8_t vec)
{
	// Check the corresponding bit in the LAPIC's ISR
	kassert_dbg(vec >= IRQ_APIC_BASE);
	uint32_t reg = APIC_REG_ISR_BASE + 0x10 * (vec / 32);
	if (lapic_read(reg) & (1 << (vec % 32)))
		lapic_write(APIC_REG_EOI, 0);
}

void lapic_send_ipi(uint8_t target, uint32_t flags)
{
	if (lapic_base == NULL)
		panic("Tried to send IPI before LAPIC was initialized");
	
	// Getting preempted could mean a race condition here
	preempt_inc();
	if (!(flags & IPI_BROADCAST))
		lapic_write(APIC_REG_ICR1, (uint32_t)target << 24);
	lapic_write(APIC_REG_ICR0, flags);
	preempt_dec();
}

// Returns the number of ticks in 10ms
uint32_t lapic_timer_prepare(void)
{
	const uint32_t test_ms = 30;
	const uint32_t ticks_initial = 0xFFFFFFFF;

	lapic_write(APIC_REG_TIMER_DIVIDE, 0x3);
	lapic_write(APIC_REG_TIMER_INITIAL, ticks_initial);

	pit_sleep_ms(test_ms); // TODO: Use interrupts for better accuracy (might be tricky getting interrupts with SMP)

	lapic_write(APIC_REG_TIMER_LVT, APIC_DISABLE);

	uint32_t ticks_per_10ms = (ticks_initial - lapic_read(APIC_REG_TIMER_CURRENT)) / (test_ms / 10);
	klog_verbose("lapic", "CPU%u timer: %u/10ms\n", smp_cpu_id(), ticks_per_10ms);

	return ticks_per_10ms;
}

static void lapic_timer_callback(struct isr_ctx *UNUSED(regs))
{
	struct task *t = current;

	// Preempt a task after it's ran for 5 time slices
	if (!percpu_get(reschedule)) {
		t->sched.num_slices++;

		if (t->sched.num_slices >= MAX_SLICES)
			percpu_set(reschedule, true);
	}
}

// Enables the lapic timer with interrupts every 10ms
void lapic_timer_enable(void)
{
	uint8_t id = lapic_id();
	struct lapic_info *lapic = find_lapic(id);

#ifdef DEBUG
	const uint32_t period = lapic->ticks_per_10ms * 10;
#else
	const uint32_t period = lapic->ticks_per_10ms;
#endif

	lapic_write(APIC_REG_TIMER_DIVIDE, 0x3);
	lapic_write(APIC_REG_TIMER_LVT, IRQ_LINT_TIMER | APIC_TIMER_PERIODIC);
	lapic_write(APIC_REG_TIMER_INITIAL, period);
}

void lapic_enable(void)
{
	// TODO: Fallback to PIC
	if (!has_lapic())
		panic("No local APIC found for current CPU");

	uint8_t id = lapic_id();
	struct lapic_info *lapic = find_lapic(id);
	percpu_set(apic_id, id);

	// Make sure APIC is globally enabled
	kassert((msr_read(0x1B) & (1 << 11)) > 0); 

	klog_verbose("lapic", "LAPIC Version: %u\n", lapic_read(APIC_REG_VERSION) & 0xFF);

	for (size_t i = 0; i < nmi_list_size; i++)
		if (nmi_list[i]->acpi_id == lapic->acpi_id || nmi_list[i]->acpi_id == 0xFF)
			lapic_set_nmi(IRQ_NMI, nmi_list[i]);

	// Enable the LAPIC via the spurious interrupt register
	lapic_write(APIC_REG_SPURIOUS, lapic_read(APIC_REG_SPURIOUS) | (1 << 8) | IRQ_APIC_SPURIOUS);

	// Calibrate timer
	lapic->ticks_per_10ms = lapic_timer_prepare();

	// Register timer callback (only do this once)
	if (id == 0) {
		struct isr_info timer_info = {
			.type = ISR_IRQ,
			.handler = lapic_timer_callback,
		};

		isr_set_info(IRQ_LINT_TIMER, &timer_info);
	}

	// Clear any pending interrupts
	lapic_write(APIC_REG_EOI, 0);
}

