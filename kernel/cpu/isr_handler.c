#include "libk.h"
#include "proc.h"
#include "system.h"
#include "io.h"

#define INT_PAGE_FAULT 14
#define INT_PIT 32
#define IST_LEN 7

extern virtaddr_t interrupt_stack_table[IST_LEN];

static const char *const exception_messages[32];

void isr_handler(struct interrupt_frame *frame);

static void page_fault(struct interrupt_frame *frame)
{
	uintptr_t faulting_address;
	asm volatile("mov %%cr2, %0" : "=r" (faulting_address));
	panic(
		"%s:\n"
		"\tfaulting address: %p\n"
		"\trip: %p, rsp: %p\n"
		"\tint_no: %lu, err_code: %lu, flags: %u\n",
		exception_messages[frame->int_no],
		(void *)faulting_address,
		(void *)frame->rip, (void *)frame->rsp,
		frame->int_no, frame->err_code,
		(unsigned int)frame->err_code & 0x1F
	);
}

static void exception_handler(struct interrupt_frame *frame)
{
	switch (frame->int_no) {
		case INT_PAGE_FAULT:
			page_fault(frame);
			break;
		default:
			panic(
				"%s:\n"
				"\trip: %p, rsp: %p\n"
				"\tint_no: %lu, err_code: %lu\n",
				exception_messages[frame->int_no],
				(void *)frame->rip, (void *)frame->rsp,
				frame->int_no, frame->err_code
			);
	}
}

static void syscall_handler(struct interrupt_frame *frame)
{
	// Handle syscall
	(void)frame;
}

static void irq_handler(struct interrupt_frame *frame)
{
	if (frame->int_no != INT_PIT)
		kprintf("Hit interrupt %zu: %p\n", frame->int_no, (void *)frame->rip);
	irq_ack(frame->int_no);
}

void irq_ack(int int_no)
{
	if (int_no >= 40)
		outb(0xA0, 0x20);
	outb(0x20, 0x20);
}

void isr_handler(struct interrupt_frame *frame)
{
	if (frame->int_no < 32)
		exception_handler(frame);
	else if (frame->int_no < 48) {
		irq_handler(frame);
	} else if (frame->int_no == 0x40) {
		syscall_handler(frame);
	} else {
		kprintf("Hit interrupt %zu: %p\n", frame->int_no, (void *)frame->rip);
	}
}

static const char *const exception_messages[32] = {
	"Division by zero",
	"Debug",
	"Non-maskable interrupt",
	"Breakpoint",
	"Overflow",
	"Bound range exceeded",
	"Invalid opcode",
	"Device not available",
	"Double fault",
	"(reserved exception 9)",
	"Invalid TSS",
	"Segment not present",
	"Stack segment fault",
	"General protection fault",
	"Page fault",
	"(reserved exception 15)",
	"x87 floating-point exception",
	"Alignment check",
	"Machine check",
	"SIMD floating-point exception",
	"Virtualization exception",
	"(reserved exception 21)",
	"(reserved exception 22)",
	"(reserved exception 23)",
	"(reserved exception 24)",
	"(reserved exception 25)",
	"(reserved exception 26)",
	"(reserved exception 27)",
	"(reserved exception 28)",
	"(reserved exception 29)",
	"(reserved exception 30)",
	"(reserved exception 31)"
};
