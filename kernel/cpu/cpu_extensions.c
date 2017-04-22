#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <klog.h>
#include <asm.h>
#include <io.h>

// TODO: I'm not even sure if this works - needs testing
bool sse_fpu_enable() {
	uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;
	asm_cpuid(1, &eax, &ebx, &ecx, &edx);

	if ((edx & CPUID_SSE_SUPPORTED) == 0)
		klog_warn("SSE not supported!\n");

	if ((edx & CPUID_FPU_SUPPORTED) == 0)
		klog_warn("FPU not enabled!\n");

	if (edx & (CPUID_FPU_SUPPORTED | CPUID_SSE_SUPPORTED)) {
		sse_fpu_set_enabled();
		return 1;
	}

	return 0;
}

void cpu_get_vendor(int8_t *buf) {
	uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;
	asm_cpuid(0, &eax, &ebx, &ecx, &edx);
	memcpy(buf, &ebx, sizeof(uint32_t));
	memcpy(buf + 4, &edx, sizeof(uint32_t));
	memcpy(buf + 8, &ecx, sizeof(uint32_t));
	buf[12] = '\0';
}

void cpu_extensions_enable() {
	if (cpuid_supported())
		klog_warn("CPUID not supported!\n");
	else {
		sse_fpu_enable();

		int8_t vendor_string[13];
		cpu_get_vendor(vendor_string);
		//klog_detail("CPU vendor: %s\n", vendor_string);
	}
}
