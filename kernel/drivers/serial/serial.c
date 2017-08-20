#include <stdint.h>

#include "drivers/serial.h"
#include "io.h"

#define COM1_PORT 0x3F8
#define COM2_PORT 0x2F8

void serial_init(void) {
	// Init COM1 and COM2
	outb(COM1_PORT + 1, 0x00);    // Disable all interrupts
        outb(COM1_PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
        outb(COM1_PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
        outb(COM1_PORT + 1, 0x00);    //                  (hi byte)
        outb(COM1_PORT + 3, 0x02);    // 7 bits, no parity, one stop bit
        outb(COM1_PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
        outb(COM1_PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set

	outb(COM2_PORT + 1, 0x00);    // Disable all interrupts
	outb(COM2_PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
	outb(COM2_PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
	outb(COM2_PORT + 1, 0x00);    //                  (hi byte)
	outb(COM2_PORT + 3, 0x02);    // 7 bits, no parity, one stop bit
	outb(COM2_PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
	outb(COM2_PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set

	// Print a small separator so we can see output clearly
	const char *msg = "-----------------------------------\n";
	while (*msg)
		serial_write_com(1, *msg++);
}

static inline int serial_transmit_empty(uint16_t port) {
	return inb(port + 5) & 0x20;
}

void serial_write_com(int com, unsigned char data) {
	uint16_t port;
	switch (com) {
		case 1:
			port = COM1_PORT;
			break;
		case 2:
			port = COM2_PORT;
			break;
		// TODO: COM3 and COM4
		default:
			return;
	}

	while (serial_transmit_empty(port) == 0)
		;

	outb(port, data);
}
