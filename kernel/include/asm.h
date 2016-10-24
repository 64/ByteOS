#include <cpuid.h>
#include <stdbool.h>
#include <klog.h>
#include <stdint.h>

#define CPUID_VENDOR_OLDAMD       "AMDisbetter!"
#define CPUID_VENDOR_AMD          "AuthenticAMD"
#define CPUID_VENDOR_INTEL        "GenuineIntel"
#define CPUID_VENDOR_VIA          "CentaurHauls"
#define CPUID_VENDOR_OLDTRANSMETA "TransmetaCPU"
#define CPUID_VENDOR_TRANSMETA    "GenuineTMx86"
#define CPUID_VENDOR_CYRIX        "CyrixInstead"
#define CPUID_VENDOR_CENTAUR      "CentaurHauls"
#define CPUID_VENDOR_NEXGEN       "NexGenDriven"
#define CPUID_VENDOR_UMC          "UMC UMC UMC "
#define CPUID_VENDOR_SIS          "SiS SiS SiS "
#define CPUID_VENDOR_NSC          "Geode by NSC"
#define CPUID_VENDOR_RISE         "RiseRiseRise"

#define CPUID_SSE_SUPPORTED (1 << 25)

extern void cpu_extensions_enable();

// Returns zero if supported, non-zero if not
extern bool cpuid_supported();

extern void sse_set_enabled();

// Calls GCC's builtin cpuid function, defined in <cpuid.h>
static inline uint32_t asm_cpuid(uint32_t level, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx) {
	return __get_cpuid(level, (unsigned*)eax, (unsigned*)ebx, (unsigned*)ecx, (unsigned*)edx);
}
