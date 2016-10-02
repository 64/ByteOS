#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/cdefs.h>
#include <arch/i386/descriptors.h>

static struct {
    idt_entry entries[256];
    idt_pointer pointer;
} idt COMPILER_ATTR_USED;

void idt_set_entry(uint8_t index, idt_gate base, uint16_t selector, uint8_t flags) {
    idt.entries[index].base_low = ((uintptr_t)base & 0xFFFF);
    idt.entries[index].base_high = ((uintptr_t)base >> 16) & 0xFFFF;
    idt.entries[index].selector = selector;
    idt.entries[index].always_zero = 0;
    idt.entries[index].flags = flags | 0x60;
}

#define INSTALL_ISR(index) { \
        extern void  _isr##index(); \
        idt_set_entry((index), (_isr##index), 0x08, 0x8E); \
    }

void idt_install(void) {
    idt_pointer *idt_p = &idt.pointer;
    idt_p->size = (uint16_t)(sizeof(idt.entries) - 1);
    idt_p->base = (uintptr_t)(idt.entries);
    memset((void*)idt.entries, 0, sizeof(idt.entries));

    INSTALL_ISR(0);
    INSTALL_ISR(1);
    INSTALL_ISR(2);
    INSTALL_ISR(3);
    INSTALL_ISR(4);
    INSTALL_ISR(5);
    INSTALL_ISR(6);
    INSTALL_ISR(7);
    INSTALL_ISR(8);
    INSTALL_ISR(9);
    INSTALL_ISR(10);
    INSTALL_ISR(11);
    INSTALL_ISR(12);
    INSTALL_ISR(13);
    INSTALL_ISR(14);
    INSTALL_ISR(15);
    INSTALL_ISR(16);
    INSTALL_ISR(17);
    INSTALL_ISR(18);
    INSTALL_ISR(19);
    INSTALL_ISR(20);
    INSTALL_ISR(21);
    INSTALL_ISR(22);
    INSTALL_ISR(23);
    INSTALL_ISR(24);
    INSTALL_ISR(25);
    INSTALL_ISR(26);
    INSTALL_ISR(27);
    INSTALL_ISR(28);
    INSTALL_ISR(29);
    INSTALL_ISR(30);
    INSTALL_ISR(31);

    idt_load((uintptr_t)idt_p);

    extern void irq_install(void);
    irq_install();
}
