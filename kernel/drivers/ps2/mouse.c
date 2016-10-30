#include <drivers/ps2/mouse.h>
#include <interrupt.h>
#include <klog.h>
#include <io.h>

static bool mouse_wait(bool write) {
	uint32_t timeout = 1;
	if (!write) {
		while (timeout)
			if ((io_inportb(MOUSE_STATUS) & 0x1) == 1)
				return 1;
		klog_notice("Mouse timeout\n");
		return 0;
	} else {
		while (timeout)
			if (!((io_inportb(MOUSE_STATUS) & 0x2)))
				return 1;
		klog_notice("Mouse timeout\n");
		return 0;
	}
}

static inline uint8_t mouse_read() {
	mouse_wait(0);
	return io_inportb(MOUSE_PORT);
}

static void mouse_write(uint8_t port, uint8_t data) {
	if (port == MOUSE_PORT) {
		mouse_wait(1); io_outportb(MOUSE_STATUS, MOUSE_WRITE);
		mouse_wait(1); io_outportb(MOUSE_PORT, data);
		while(mouse_read() != MOUSE_ACK)
			;
	} else {
		mouse_wait(1);
		io_outportb(port, data);
	}
}

void mouse_init() {

	klog_detail("1\n");
	mouse_write(MOUSE_STATUS, MOUSE_ENABLE_AUX);
	klog_detail("2\n");
	mouse_write(MOUSE_STATUS, MOUSE_COMPAQ_STATUS);
	klog_detail("3\n");
	uint8_t status = mouse_read(MOUSE_PORT) | 2;
	klog_detail("4\n");
	mouse_write(MOUSE_STATUS, 0x60);
	klog_detail("5\n");
	mouse_wait(1); io_outportb(MOUSE_PORT, status);
	klog_detail("6\n");
	mouse_write(MOUSE_PORT, 0xF6);
	klog_detail("7\n");
	mouse_write(MOUSE_PORT, 0xF4);
	klog_detail("8\n");
	
	irq_install_handler(MOUSE_IRQ, mouse_interrupt);
}

void mouse_interrupt(struct interrupt_frame *r) {
	(void)r; // TODO: Implement this
	klog_detail("Mouse interrupt fired\n");
	irq_ack(MOUSE_IRQ);
}
