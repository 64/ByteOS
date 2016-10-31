#include <syscalls.h>
#include <interrupt.h>
#include <klog.h>

#define NUM_SYSCALLS (sizeof(syscalls) / sizeof(syscall_func))

uint32_t syscall_test(uint32_t t) {
	klog_info("Test syscall invoked with argument %d\n", t);
	return 4;
}

typedef uint32_t (*syscall_func)();
static syscall_func syscalls[] = {
	[SYSCALL_TEST] = syscall_test
};

void syscall_handler(struct interrupt_frame *r) {
	if (r->eax >= NUM_SYSCALLS)
		return;

	syscall_func f = syscalls[r->eax];
	r->eax = f(r->ebx, r->ecx, r->edx, r->esi, r->edi);
}

void syscalls_install() {
	isr_install_handler(0x64, syscall_handler);
}
