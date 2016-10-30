#include <drivers/ps2/mouse.h>
#include <interrupt.h>
#include <klog.h>
#include <io.h>

#define WAIT(x) if (mouse_wait((x)) == 0) return;

static bool mouse_wait(bool write) {
	uint32_t timeout = 100000;
	if (!write) {
		while (timeout--)
			if ((io_inportb(MOUSE_STATUS) & 0x1) == 1)
				return 1;
		klog_notice("Mouse timeout\n");
		return 0;
	} else {
		while (timeout--)
			if (!((io_inportb(MOUSE_STATUS) & 0x2)))
				return 1;
		klog_notice("Mouse timeout\n");
		return 0;
	}
}

static inline void mouse_write(uint8_t data) {
	mouse_wait(1); io_outportb(MOUSE_STATUS, MOUSE_WRITE);
	mouse_wait(1); io_outportb(MOUSE_PORT, data);
}

static inline uint8_t mouse_read() {
	mouse_wait(0);
	return io_inportb(MOUSE_PORT);
}

void mouse_init() {
	WAIT(1); io_outportb(MOUSE_STATUS, MOUSE_ENABLE_AUX);
	WAIT(1); io_outportb(MOUSE_STATUS, MOUSE_COMPAQ_STATUS);
	uint8_t status = mouse_read() | 2;
	WAIT(1); io_outportb(MOUSE_STATUS, 0x60);
	WAIT(1); io_outportb(MOUSE_PORT, status);
	irq_install_handler(MOUSE_IRQ, mouse_interrupt);
}

void mouse_interrupt(struct interrupt_frame *r) {
	(void)r; // TODO: Implement this
	irq_ack(MOUSE_IRQ);
}
