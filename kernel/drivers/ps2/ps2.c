#include "asm.h"
#include "libk.h"
#include "util.h"
#include "drivers/ps2.h"

#define PS2_STATUS 0x64
#define PS2_COMMAND 0x64
#define PS2_DATA 0x60

#if 0
static uint8_t send_command(uint8_t cmd)
{
	outb(PS2_COMMAND, cmd);
	return inb(PS2_DATA);
}
#endif

void ps2_init(void)
{
	// Flush the output buffer
	while ((inb(PS2_STATUS) & 1) != 0)
		(void)inb(PS2_DATA);

	outb(PS2_COMMAND, 0xAE);

	ps2kbd_init();
}
