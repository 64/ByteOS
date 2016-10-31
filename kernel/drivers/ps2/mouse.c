#include <drivers/ps2/mouse.h>
#include <drivers/ps2/ps2main.h>
#include <interrupt.h>
#include <klog.h>
#include <io.h>
#include <string.h>

#define MOUSE_LMB (1 << 0)
#define MOUSE_RMB (1 << 1)
#define MOUSE_MMB (1 << 2)
#define MOUSE_X_SIGN (1 << 4)
#define MOUSE_Y_SIGN (1 << 5)
#define MOUSE_X_OVERFLOW (1 << 6)
#define MOUSE_Y_OVERFLOW (1 << 7)
#define MOUSE_IRQ 12
#define MOUSE_ACK 0xFA
#define MOUSE_RESEND 0xFE
#define MOUSE_RESET 0xFF
#define MOUSE_RES_RESET 0xAA
#define MOUSE_ERROR 0xFC

#define MOUSE_X_MAX (80 - 1)
#define MOUSE_Y_MAX (25 - 1)

struct mouse_state mouse_state;

static inline void mouse_wait_ack() {
	uint8_t res;
	while((res = ps2_port_read()) != 0xFA)
		klog_notice("Expected ACK (0xFA), got 0x%x\n", res);
}

static int32_t mouse_increase_val(int32_t original, uint8_t val, bool sign, int32_t max) {
	int32_t addition = sign ? val | 0xFFFFFF00 : val;
	if (original + addition < 0)
		return 0;
	if (original + addition > max)
		return max;
	return original + addition;
}

static inline void mouse_parse_packet(uint8_t buf[3]) {
	if (buf[0] & MOUSE_X_OVERFLOW || buf[0] & MOUSE_Y_OVERFLOW)
		return;
	mouse_state.x = mouse_increase_val(mouse_state.x, buf[1], buf[0] & MOUSE_X_SIGN, MOUSE_X_MAX);
	mouse_state.y = mouse_increase_val(mouse_state.y, buf[2], buf[0] & MOUSE_Y_SIGN, MOUSE_Y_MAX);
	extern void vga_textmode_setcursor(size_t x, size_t y);
	vga_textmode_setcursor(mouse_state.x, mouse_state.y);
}

void mouse_interrupt(struct interrupt_frame *r) {
	(void)r;
	static uint32_t cycle = 0;
	static uint8_t buf[3] = { 0, 0, 0 };
	uint8_t status = io_inportb(PS2_STATUS);
	for (; status & 1; status = io_inportb(PS2_STATUS)) {
		uint8_t val = io_inportb(PS2_DATA);
		// PS/2 second port buffer full
		if ((status & 0x20) == 0)
			continue;
		switch (cycle) {
			case 0:
				buf[0] = val;
				cycle++;
				break;
			case 1:
				buf[1] = val;
				cycle++;
				break;
			case 2:
				buf[2] = val;
				mouse_parse_packet(buf);
				memset(buf, 0, sizeof(buf));
				cycle = 0;
				break;
		}
	}
	irq_ack(MOUSE_IRQ);
}

void mouse_init() {
	memset(&mouse_state, 0, sizeof(struct mouse_state));

	// TODO: Why doesn't this work...
	irq_install_handler(MOUSE_IRQ, mouse_interrupt);
	ps2_cmd_write(PS2_CMD_READ_CONFIG);
	uint8_t status = ps2_port_read();
	status &= ~(1 << 5);
	status |= (1 << 1);
	ps2_cmd_write(PS2_CMD_WRITE_CONFIG);
	ps2_port1_write(status);
	ps2_port2_write(0xF6); mouse_wait_ack();
	ps2_port2_write(0xF4); mouse_wait_ack();
}
