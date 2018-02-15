#include "libk.h"
#include "proc.h"
#include "types.h"
#include "cpu.h"
#include "io.h"
#include "drivers/apic.h"

#define INT_PAGE_FAULT 14
#define INT_PIT 32
#define IST_LEN 7

static const char *const exception_messages[32];

static void page_fault(uint8_t int_no, struct interrupt_frame *frame)
{
	uintptr_t faulting_address;
	asm volatile("mov %%cr2, %0" : "=r" (faulting_address));
	panic(
		"%s:\n"
		"\tfaulting address: %p\n"
		"\trip: %p, rsp: %p\n"
		"\tint_no: %u, err_code: %lu, flags: %u\n",
		exception_messages[int_no],
		(void *)faulting_address,
		(void *)frame->rip, (void *)frame->rsp,
		int_no, (frame->err_code & 0xFFFFFFFF),
		(unsigned int)frame->err_code & 0x1F
	);
}

void exception_handler(struct interrupt_frame *frame)
{
	uint8_t int_no = (uint8_t)(frame->err_code >> 32);
	switch (int_no) {
		case INT_PAGE_FAULT:
			page_fault(int_no, frame);
			break;
		default:
			panic(
				"%s:\n"
				"\trip: %p, rsp: %p\n"
				"\tint_no: %u, err_code: %lu\n",
				exception_messages[int_no],
				(void *)frame->rip, (void *)frame->rsp,
				int_no, frame->err_code
			);
	}
}

static inline void irq_ack(void)
{
	lapic_send_eoi();
}

void irq_handler(struct interrupt_frame *frame)
{
	irq_ack();
	kprintf("Unhandled IRQ at %p\n", (void *)frame->rip);
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
