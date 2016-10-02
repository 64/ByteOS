#include <drivers/ps2/kbd.h>
#include <isr.h>
#include <stdio.h>
#include <io.h>

#define KEY_PENDING 0x64

static inline void keyboard_wait(void) {
    while (io_inportb(KEY_PENDING) & 2)
        ;
}

void keyboard_handler(struct regs *r) {
    keyboard_wait();
    uint8_t scancode = io_inportb(0x60);
    irq_ack(r->int_no - 32);

    printf("%c", scancode);
}

void keyboard_install(void) {
    printf("%s Initializing PS/2 keyboard driver...\n", info_header);
    irq_install_handler(1, keyboard_handler);
}
