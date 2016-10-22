#include <asm.h>
#include <io.h>
#include <klog.h>

extern bool sse_enable();
extern bool cpuid_supported();

void cpu_extensions_enable() {
	if (cpuid_supported())
		klog_warn("CPUID not supported!\n");
	else {
		sse_enable();
	}
}
