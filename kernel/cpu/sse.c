#include <stdint.h>
#include <stdbool.h>
#include <klog.h>
#include <asm.h>

bool sse_enable() {
	uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;
	asm_cpuid(1, &eax, &ebx, &ecx, &edx);
	if (edx & CPUID_SSE_SUPPORTED) {
		sse_set_enabled();
		return 1;
	}

	klog_warn("SSE not supported!\n");
	return 0;
}
