#include <stdint.h>
#include <klog.h>
#include <stdlib.h>
#include <sys/cdefs.h>
#include <descriptors.h>

static const char *exception_messages[32] = {
	"Division by zero",
	"Debug",
	"Non-maskable interrupt",
	"Breakpoint",
	"Detected overflow",
	"Out-of-bounds",
	"Invalid opcode",
	"No coprocessor",
	"Double fault",
	"Coprocessor segment overrun",
	"Bad TSS",
	"Segment not present",
	"Stack fault",
	"General protection fault",
	"Page fault",
	"Unknown interrupt",
	"Coprocessor fault",
	"Alignment check",
	"Machine check",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved"
};

static void (*isr_handlers[32])(struct regs *r) = {
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0
};

void isr_install_handler(uint32_t index, void (*handler)(struct regs *r)) {
	isr_handlers[index] = handler;
}

void isr_fault_handler(struct regs *r) {
	if (isr_handlers[r->int_no] != 0)
		isr_handlers[r->int_no](r);
	else {
		klog_fatal("Unhandled Exception: %s\n", exception_messages[r->int_no]);
		abort();
	}
}
