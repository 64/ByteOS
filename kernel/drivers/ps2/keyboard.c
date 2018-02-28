#include "interrupts.h"
#include "libk.h"
#include "util.h"
#include "drivers/ps2.h"

static void ps2kbd_irq_handler(struct isr_context *);

void ps2kbd_init(void)
{
	uint8_t vec = ISA_TO_INTERRUPT(1);
	irq_register_handler(vec, ps2kbd_irq_handler);
	irq_unmask(ISA_TO_INTERRUPT(1));
	klog("ps2kbd", "Initialised keyboard on IRQ %u\n", vec);
}

static void ps2kbd_irq_handler(struct isr_context *UNUSED(regs))
{
	(void)inb(0x60);
	kprintf("Keyboard handler fired\n");
}
