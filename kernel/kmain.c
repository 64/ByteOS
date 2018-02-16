#include "libk.h"
#include "multiboot2.h"
#include "mm.h"
#include "proc.h"
#include "types.h"
#include "cpu.h"
#include "util.h"
#include "drivers/apic.h"
#include "drivers/acpi.h"

void kmain(physaddr_t);

void kmain(physaddr_t mboot_info_phys)
{
	paging_init();
	struct multiboot_info *mboot_info_virt = phys_to_kern(mboot_info_phys);
	struct mmap *mem_map = mmap_init(mboot_info_virt);
	paging_map_all(mem_map);
	mmap_dump_info();
	acpi_init();
	apic_init();
	pmm_init(mem_map);
	cpu_local_init();

	// At this point, we have physical and virtual memory allocation
	//run_tasks();
	
	// Shouldn't ever get here
}
