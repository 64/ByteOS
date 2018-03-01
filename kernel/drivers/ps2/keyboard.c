#include "interrupts.h"
#include "libk.h"
#include "util.h"
#include "drivers/ps2.h"

static void ps2kbd_irq_handler(struct isr_context *);

static uint8_t ps2kbd_cmd(uint8_t data)
{
	// TODO: Add a timeout
	uint8_t response;
	do {
		ps2_write_data(data);
		response = ps2_read_data();
	} while (response == 0xFE);
	return response;
}

void ps2kbd_init(void)
{
	uint8_t self_test_status = ps2kbd_cmd(0xFF);
	if (self_test_status == 0xFA)
		self_test_status = ps2_read_data();
	kassert(self_test_status == 0xAA);
#ifdef VERBOSE
	klog("ps2", "Status byte is %x\n", ps2_read_status());
	klog("ps2", "Configuration byte is %x\n", ps2_read_config());
#endif

	// Don't send any commands after this point, since the interrupt handler will eat the data
	uint8_t vec = ISA_TO_INTERRUPT(1);
	irq_register_handler(vec, ps2kbd_irq_handler);
	irq_unmask(ISA_TO_INTERRUPT(1));
	klog("ps2kbd", "Initialised keyboard on IRQ %u\n", vec);
}

static void ps2kbd_irq_handler(struct isr_context *UNUSED(regs))
{
	(void)ps2_read_data();
	kprintf("Keyboard handler fired\n");
}
