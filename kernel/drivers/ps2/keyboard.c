#include "interrupts.h"
#include "libk.h"
#include "util.h"
#include "drivers/ps2.h"

static void ps2kbd_irq_handler(struct isr_ctx *);

static __attribute__((unused)) uint8_t ps2kbd_cmd(uint8_t data)
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
	/* TODO: Why does this not work?
	* Bochs and VirtualBox seem to only respond with 0xAA (and then maybe 0xFA).
	* QEMU seems to respond with 0xFA, then 0xAA.
	* On my PC it doesn't respond at all to the command.
	*/
	/* uint8_t self_test_status = ps2kbd_cmd(0xFF);
	klog("ps2", "Self-test and reset: %x\n", self_test_status);
	kassert(self_test_status == 0xFA || self_test_status == 0xAA);
	while ((ps2_read_status() & 1) == 1)
		ps2_read_data();*/
	
#ifdef VERBOSE
	klog("ps2", "Status byte is %x\n", ps2_read_status());
	klog("ps2", "Configuration byte is %x\n", ps2_read_config());
#endif

	// Don't send any commands after this point, since the interrupt handler will eat the data
	uint8_t vec = ISA_TO_INTERRUPT(1);
	irq_register_handler(vec, ps2kbd_irq_handler);
	irq_unmask(ISA_TO_INTERRUPT(1));
	klog("ps2kbd", "Initialised keyboard on IRQ %u\n", vec);

	// Flush the output buffer (again)
	while ((inb(PS2_STATUS) & 1) != 0)
		(void)inb(PS2_DATA);
}

static void ps2kbd_irq_handler(struct isr_ctx *UNUSED(regs))
{
	(void)ps2_read_data();
#ifdef VERBOSE
	kprintf("Keyboard handler fired\n");
#endif
}
