#include "asm.h"
#include "libk.h"
#include "util.h"
#include "drivers/ps2.h"

#define PS2_INT0 (1 << 0)
#define PS2_INT1 (1 << 1)
#define PS2_SYS_FLAG (1 << 2)
#define PS2_CLOCK0 (1 << 4)
#define PS2_CLOCK1 (1 << 5)
#define PS2_TRANSLATE (1 << 6)

void ps2_init(void)
{
	// TODO: Initialise USB controllers
	// TODO: Determine if the PS/2 controller exists

	// Disable devices
	ps2_write_cmd(0xAD);
	ps2_write_cmd(0xA7);

	// Flush the output buffer
	while ((inb(PS2_STATUS) & 1) != 0)
		(void)inb(PS2_DATA);

	// Set the controller configuration byte
	uint8_t config_byte = ps2_read_config();
	kassert(config_byte & PS2_CLOCK1);
	config_byte &= ~(PS2_INT0 | PS2_INT1 | PS2_TRANSLATE);
	ps2_write_config(config_byte);

	// Perform controller self test
	ps2_write_cmd(0xAA);
	kassert(ps2_read_data() == 0x55);

	// Determine if there are 2 channels
	ps2_write_cmd(0xA8);
	ps2_write_cmd(0x20);
	config_byte = ps2_read_data();
	kassert((config_byte & PS2_CLOCK1) == 0);
	ps2_write_cmd(0xA7);

	// Perform interface tests
	ps2_write_cmd(0xAB);
	kassert(ps2_read_data() == 0x00);
	ps2_write_cmd(0xA9);
	kassert(ps2_read_data() == 0x00);

	// Enable devices
	config_byte = ps2_read_config();
	config_byte |= (PS2_INT0 | PS2_INT1);
	ps2_write_config(config_byte);
	ps2_write_cmd(0xAE);
	ps2_write_cmd(0xA8);

	// Reset devices
	ps2kbd_init();
}
