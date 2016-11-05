#include <drivers/ps2/mouse.h>
#include <drivers/ps2/ps2main.h>
#include <algs/c_queue.h>
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
struct c_queue mouse_cmd_q;

static void mouse_send_data(uint8_t val) {
	bool send = c_queue_isempty(&mouse_cmd_q) ? 1 : 0;
	c_queue_push(&mouse_cmd_q, val);
	if (send)
		ps2_port2_write(val);
}

static int32_t mouse_increase_val(int32_t original, uint8_t val, bool sign, bool invert, int32_t max) {
	int32_t addition = sign ? val | 0xFFFFFF00 : val;
	if (invert)
		addition = -addition;
	if (original + addition < 0)
		return 0;
	if (original + addition > max)
		return max;
	return original + addition;
}

static inline void mouse_parse_packet(uint8_t buf[3]) {
	if (buf[0] & MOUSE_X_OVERFLOW || buf[0] & MOUSE_Y_OVERFLOW)
		return;
	mouse_state.x = mouse_increase_val(mouse_state.x, buf[1], buf[0] & MOUSE_X_SIGN, 0, MOUSE_X_MAX);
	mouse_state.y = mouse_increase_val(mouse_state.y, buf[2], buf[0] & MOUSE_Y_SIGN, 1, MOUSE_Y_MAX);
	extern void vga_textmode_setcursor(size_t x, size_t y);
	vga_textmode_setcursor(mouse_state.x, mouse_state.y);
}

void mouse_interrupt(struct interrupt_frame *r) {
	(void)r;
	static uint32_t cycle = 0;
	static uint8_t buf[3] = { 0, 0, 0 };
	static uint8_t num_retries = 0;
	uint8_t val = ps2_port_read();
	irq_ack(r->int_no - 32);

	if (!c_queue_isempty(&mouse_cmd_q))
		switch (val) {
			case 0x0:
				return;
			case MOUSE_CMD_ACK:
				c_queue_pop(&mouse_cmd_q);
				if (!c_queue_isempty(&mouse_cmd_q))
					ps2_port2_write(c_queue_peek(&mouse_cmd_q));
				num_retries = 0;
				return;
			case MOUSE_CMD_RESEND:
				if (num_retries++ <= 3)
					ps2_port2_write(c_queue_peek(&mouse_cmd_q));
				else {
					c_queue_pop(&mouse_cmd_q);
					num_retries = 0;
				}
				return;
			default:
				klog_detail("Unknown mouse response 0x%x to 0x%x\n", val, c_queue_peek(&mouse_cmd_q));
				break;
		}

	switch (cycle) {
		case 0:
			if (val & MOUSE_Y_OVERFLOW || val & MOUSE_X_OVERFLOW)
				break;
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
	};
}

void mouse_init() {
	c_queue_init(&mouse_cmd_q);
	memset(&mouse_state, 0, sizeof(struct mouse_state));
	irq_install_handler(MOUSE_IRQ, mouse_interrupt);

	// Initialisation sequence
	mouse_send_data(0xF6);
	mouse_send_data(0xF4);
}
