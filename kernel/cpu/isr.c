#include <stdint.h>
#include <klog.h>
#include <stdlib.h>
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

extern void irq_handler_ignore(struct interrupt_frame *r);

void (*isr_handlers[256])(struct interrupt_frame *r) = {
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, irq_handler_ignore, 0, 0, 0, 0, 0, 0, // IRQs at 32 - 39
	0, 0, 0, 0, irq_handler_ignore, 0, 0, 0, // IRQs at 40-47
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0
};

void isr_install_handler(uint32_t index, void (*handler)(struct interrupt_frame *r)) {
	isr_handlers[index] = handler;
}

void isr_fault_handler(struct interrupt_frame *r) {
	if (isr_handlers[r->int_no] != 0)
		isr_handlers[r->int_no](r);
	else {
		if (r->int_no < 32) {
			klog_fatal("Unhandled Exception: %s\n", exception_messages[r->int_no]);
			klog_fatal_nohdr("\tinterrupt no. %d\n", r->int_no);
			klog_fatal_nohdr("\tgs: %x, fs: %x, es: %x, ds: %x\n", r->gs, r->fs, r->es, r->ds);
			klog_fatal_nohdr("\tedi: %x, esi: %x, ebp: %x, esp: %x\n", r->edi, r->esi, r->ebp, r->esp);
			klog_fatal_nohdr("\tebx: %x, edx: %x, ecx: %x, eax: %x\n", r->ebx, r->edx, r->ecx, r->eax);
			klog_fatal_nohdr("\teip: %x, eflags: %x\n", r->eip, r->eflags);
			abort();
		} else
			klog_info("Recieved unhandled interrupt number 0x%x\n", r->int_no);
	}
}
