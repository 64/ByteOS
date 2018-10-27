#include "interrupts.h"
#include "drivers/apic.h"
#include "proc.h"
#include "libk.h"
#include "asm.h"
#include "util.h"

static struct isr_info isr_table[256];

void isr_init(void)
{
	// Remap the PIC to interrupts 0x20-0x2F
	outb(0x20, 0x11);
	outb(0xA0, 0x11);
	outb(0x21, 0x20);
	outb(0xA1, 0x28);
	outb(0x21, 0x04);
	outb(0xA1, 0x02);
	outb(0x21, 0x01);
	outb(0xA1, 0x01);
	outb(0x21, 0x00);
	outb(0xA1, 0x00);

	// Disable the PIC by masking all interrupts
	outb(0xA1, 0xFF);
	outb(0x21, 0xFF);

	memset(isr_table, 0, sizeof isr_table);

	irq_disable();
}

void isr_set_info(uint8_t vec, struct isr_info *info)
{
	memcpy(&isr_table[vec], info, sizeof(struct isr_info));
}

void isr_irq_mask(uint8_t vec)
{
	kassert_dbg(vec >= IRQ_APIC_BASE);
	ioapic_mask(vec - IRQ_APIC_BASE);
}

void isr_irq_unmask(uint8_t vec)
{
	kassert_dbg(vec >= IRQ_APIC_BASE);
	ioapic_unmask(vec - IRQ_APIC_BASE);
}

void isr_global_handler(struct isr_ctx *regs)
{
	uint8_t int_no = regs->info & 0xFF;
	struct isr_info *info = &isr_table[int_no];
	uint32_t int_depth = 0;

	if (info->type == ISR_IRQ) {
		atomic_inc_read32(&percpu_self()->int_depth);
		lapic_eoi(int_no);
	}

	if (info->handler != NULL)
		info->handler(regs);

	if (info->type == ISR_IRQ)
		int_depth = atomic_dec_read32(&percpu_self()->int_depth);

	// TODO: Check if task was killed
	
	// Only preempt if we are about to return to a non-IRQ
	if (int_depth == 0 && percpu_get(reschedule))
		sched_yield_preempt();

	// TODO: Check if task was killed
}
