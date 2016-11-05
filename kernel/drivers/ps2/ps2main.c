#include <drivers/ps2/ps2main.h>
#include <drivers/acpi.h>
#include <klog.h>
#include <io.h>

void ps2_wait_input() {
	uint32_t timer = 100000;
	while (timer-- && (io_inportb(PS2_STATUS) & 2) != 0)
		;
	if (timer == 0)
		klog_notice("PS/2 controller timed out\n");
}

void ps2_wait_output() {
	uint32_t timer = 100000;
	while (timer-- && (io_inportb(PS2_STATUS) & 1) != 1)
		;
	if (timer == 0)
		klog_notice("PS/2 controller timed out\n");
}

uint8_t ps2_port_read() {
	ps2_wait_output();
	uint8_t val = io_inportb(PS2_DATA);
	return val;
}

void ps2_port1_write(uint8_t data) {
	ps2_wait_input();
	io_outportb(PS2_DATA, data);
}

void ps2_port2_write(uint8_t data) {
	ps2_cmd_write(PS2_CMD_PORT2_WRITE);
	ps2_wait_input();
	io_outportb(PS2_DATA, data);
}

void ps2_cmd_write(uint8_t cmd) {
	ps2_wait_input();
	io_outportb(PS2_COMMAND, cmd);
}

// Steps taken from http://wiki.osdev.org/%228042%22_PS/2_Controller#Initialising_the_PS.2F2_Controller
void ps2_init() {
	// Step 1:
	// Do nothing, we don't use USB (yet)

	// Step 2:
	// This doesn't seem to work on QEMU and VBox, will inspect later
	/*if ((acpi_info.boot_arch_flags & 2) == 0)
		klog_panic("PS/2 controller not found");*/

	// Step 3:
	ps2_cmd_write(PS2_CMD_PORT1_DISABLE);
	ps2_cmd_write(PS2_CMD_PORT2_DISABLE);

	// Step 4:
	while(io_inportb(PS2_STATUS) & 1)
		io_inportb(PS2_DATA);

	// Step 5:
	ps2_cmd_write(PS2_CMD_READ_CONFIG);
	uint8_t status = ps2_port_read();
	status &= ~(PS2_CONFIG_PORT1_INT | PS2_CONFIG_PORT2_INT);
	ps2_cmd_write(PS2_CMD_WRITE_CONFIG);
	ps2_port1_write(status);
	if ((status & PS2_CONFIG_PORT2_CLOCK) == 0)
		klog_panic("PS/2 controller is single channel");

	// Step 6:
	ps2_cmd_write(PS2_CMD_CONTROLLER_TEST);
	if (ps2_port_read() != PS2_RES_CONTROLLER_TEST_PASS)
		klog_panic("PS/2 controller failed self test");

	// Step 7:
	ps2_cmd_write(PS2_CMD_PORT2_ENABLE);
	ps2_cmd_write(PS2_CMD_READ_CONFIG);
	status = ps2_port_read();
	if (status & PS2_CONFIG_PORT2_CLOCK)
		klog_panic("PS/2 controller is single channel");


	// Step 8:
	ps2_cmd_write(PS2_CMD_PORT1_TEST);
	if (ps2_port_read() != PS2_RES_PORT_TEST_PASS)
		klog_panic("PS/2 port 1 failed self test");

	ps2_cmd_write(PS2_CMD_PORT2_TEST);
	if (ps2_port_read() != PS2_RES_PORT_TEST_PASS)
		klog_panic("PS/2 port 2 failed self test");

	// Step 9:
	ps2_cmd_write(PS2_CMD_PORT1_ENABLE);
	ps2_cmd_write(PS2_CMD_PORT2_ENABLE);

	// Disable interrupts 1 and 2
	ps2_cmd_write(PS2_CMD_READ_CONFIG);
	status = ps2_port_read();
	status |= PS2_CONFIG_PORT1_INT;
	status |= PS2_CONFIG_PORT2_INT;
	ps2_cmd_write(PS2_CMD_WRITE_CONFIG);
	ps2_port1_write(status);
}
