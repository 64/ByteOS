#include <stdint.h>

#include "drivers/smbios.h"
#include "libk.h"
#include "util.h"
#include "mm.h"

#define SMBIOS_PHYS_START 0xF0000
#define SMBIOS_PHYS_END   0xFFFFF
#define SMBIOS3_ANCHOR "_SM3_"
#define SMBIOS2_ANCHOR "_SM_"

struct smbios2_entry {
	char anchor[4];
	uint8_t checksum;
	uint8_t ep_length;
	uint8_t ver_major;
	uint8_t ver_minor;
	uint16_t max_struct;
	uint8_t ep_revision;
	uint8_t formatted_area[5];
	char intermediate_anchor[5];
	uint8_t intermediate_checksum;
	uint16_t struct_table_len;
	uint32_t struct_table_addr;
	uint16_t nstructs;
	uint8_t bcd_revision;
} __attribute__((packed));

struct smbios3_entry {
	char anchor[5];
	uint8_t checksum;
	uint8_t ep_length;
	uint8_t ver_major;
	uint8_t ver_minor;
	uint8_t docrev;
	uint8_t ep_revision;
	uint8_t __reserved;
	uint32_t struct_table_max;
	physaddr_t struct_table_addr;
} __attribute__((packed));

static struct smbios_info {
	union {
		struct smbios2_entry *entry2;
		struct smbios3_entry *entry3;
		void *entry;
	};
	uint8_t major;
	uint8_t minor;
} smbios_info;

static bool calc_checksum(const virtaddr_t ventry, size_t len)
{
	uint8_t sum = 0;
	const uint8_t *entry = ventry;
	for (size_t i = 0; i < len; i++)
		sum += entry[i];
	return sum == 0;
}

static struct smbios_info find_smbios(void)
{
	for (uintptr_t addr = SMBIOS_PHYS_START; addr < SMBIOS_PHYS_END; addr += 16) {
		void *smbios = phys_to_virt(addr);

		if (memcmp(smbios, SMBIOS3_ANCHOR, 5) == 0) {

			struct smbios3_entry *eps = smbios;
			if (!calc_checksum(eps, eps->ep_length))
				continue;

			kassert_dbg(eps->__reserved == 0);
			return (struct smbios_info){
				.entry3 = eps,
				.major = eps->ver_major,
				.minor = eps->ver_minor
			};
		} else if (memcmp(smbios, SMBIOS2_ANCHOR, 4) == 0) {

			struct smbios2_entry *eps = smbios;
			if (!calc_checksum(eps, eps->ep_length))
				continue;

			// Calculate intermediate checksum
			size_t ieps_offset = offsetof(struct smbios2_entry, intermediate_anchor);
			if (!calc_checksum(&eps->intermediate_anchor, sizeof(struct smbios2_entry) - ieps_offset))
				continue;

			kassert_dbg(eps->ep_revision != 0 || memcmp(&eps->formatted_area, "\0\0\0\0", 5) == 0);
			return (struct smbios_info){
				.entry2 = eps,
				.major = eps->ver_major,
				.minor = eps->ver_minor
			};
		}
	}

	return (struct smbios_info){ .major = 0 };
}

static void dump_smbios(struct smbios_info info)
{
	#define info(msg, ...) klog_verbose("smbios", msg,##__VA_ARGS__)
	klog("smbios", "Found SMBIOS %u.%u entry point at %p\n", info.major, info.minor, info.entry);
	if (info.major == 3) {
		struct smbios3_entry UNUSED(*eps) = info.entry3;
		info("EP revision: %u\n", eps->ep_revision);
		info("Docrev: %u\n", eps->docrev);
		info("Structure table maximum size: %u\n", eps->struct_table_max);
		info("Structure table address: %p\n", (virtaddr_t)(uintptr_t)eps->struct_table_addr);
	} else if (info.major == 2) {
		struct smbios2_entry UNUSED(*eps) = info.entry2;
		info("EP revision: %u\n", eps->ep_revision);
		info("BCD revision: %u\n", eps->bcd_revision);
		info("Maximum structure size: %u\n", eps->max_struct);
		info("Number of structures: %u\n", eps->nstructs);
		info("Structure table length: %u\n", eps->struct_table_len);
		info("Structure table address: %p\n", (virtaddr_t)(uintptr_t)eps->struct_table_addr);
	}
	#undef info
}


void smbios_init(void)
{
	smbios_info = find_smbios();
	if (smbios_info.major != 2 && smbios_info.major != 3)
		panic("No SMBIOS entry point found");
	dump_smbios(smbios_info);
}

