#include <asm.h>
#include <io.h>
#include <klog.h>
#include <string.h>

extern bool sse_enable();
extern bool cpuid_supported();

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
		sse_enable();

		int8_t vendor_string[13];
		cpu_get_vendor(vendor_string);
		//klog_detail("CPU vendor: %s\n", vendor_string);
	}
}
