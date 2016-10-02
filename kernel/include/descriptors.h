#pragma once

#include <stdint.h>
#include <sys/cdefs.h>
#include <arch/i386/system.h>

typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} COMPILER_ATTR_PACKED gdt_entry;

typedef struct {
    uint16_t size;
    uintptr_t base;
} COMPILER_ATTR_PACKED gdt_pointer;

typedef struct {
    uint16_t base_low;
    uint16_t selector;
    uint8_t always_zero;
    uint8_t flags;
    uint16_t base_high;
} COMPILER_ATTR_PACKED idt_entry;

typedef struct {
    uint16_t size;
    uintptr_t base;
} COMPILER_ATTR_PACKED idt_pointer;

extern void gdt_load(uintptr_t);
extern void idt_load(uintptr_t);
typedef void (*idt_gate)(void);
