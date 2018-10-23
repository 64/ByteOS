#include "libk.h"
#include "multiboot2.h"
#include "mm.h"
#include "smp.h"
#include "sync.h"
#include "interrupts.h"
#include "proc.h"
#include "util.h"
#include "percpu.h"
#include "drivers/apic.h"
#include "drivers/ps2.h"
#include "drivers/pit.h"
#include "drivers/acpi.h"
#include "drivers/smbios.h"

struct multiboot_info *mboot_info;

void kmain(physaddr_t);

void kmain(physaddr_t mboot_info_phys)
{
	// Get the virtual address of the multiboot info structure
	mboot_info = phys_to_kern(mboot_info_phys);

	// Initialise our per-CPU data area
	percpu_init_bsp();

	// Initialise paging
	vmm_init();

	// Create the bootstrapping memory allocator
	struct mmap *mem_map = mmap_init(mboot_info);

	// Linearly map all physical memory
	vmm_map_all(mem_map);
	mmap_dump_info();

	// Find ACPI tables
	acpi_init();

	// Find SMBIOS tables
	smbios_init();

	// Gather info from the MADT
	apic_init();

	// Start the physical memory manager
	// After this point, we have access to pmm_alloc_order and kmalloc
	pmm_init(mem_map);

	// Enable the LAPIC for the BSP
	lapic_enable();

	// Initialise all I/O APICs
	ioapic_init();

	// Initialise mouse and keyboard
	ps2_init();

	// Initialise the PIT
	pit_init();

	// Turn on IRQs
	irq_enable();

	// Boot all the cores
	smp_init();

	// Run the scheduler
	sched_run_bsp();
}
