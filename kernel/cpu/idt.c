#include "io.h"
#include "types.h"
#include "mm.h"
#include "libk.h"
#include "cpu.h"
#include "drivers/apic.h"

#define EXCEP_NAME(index) isr_exception_ ## index

#define ISR_EXCEPTION_IST(index, ist) \
	void EXCEP_NAME(index)(void); \
	idt_set_isr(index, EXCEP_NAME(index), ist, FLAGS_NONE)

#define ISR_EXCEPTION(index) ISR_EXCEPTION_IST(index, IST_NONE)

#define IST_NONE 0
#define FLAGS_NONE 0x8E

struct idt_entry {
	uint16_t offset_low;
	uint16_t selector;
	uint8_t ist_index;
	uint8_t type_attr;
	uint16_t offset_mid;
	uint32_t offset_high;
	uint32_t __zero;
};

extern struct idt_entry idt64[256];
void idt_init(void);

// ISR entry points
void isr_noop(void);
void isr_irq(void);

void idt_set_isr(uint8_t index, virtaddr_t entry, uint8_t ist, uint8_t type_attr)
{
	kassert(ist == 0 || index < 32);
	uintptr_t p = (uintptr_t)entry;
	struct idt_entry e = {
		.offset_low = (p & 0xFFFF),
		.selector = 0x08,  // Kernel code segment
		.ist_index = ist,
		.type_attr = type_attr,
		.offset_mid = ((p & 0xFFFF0000) >> 16),
		.offset_high = ((p & 0xFFFFFFFF00000000) >> 32)
	};
	idt64[index] = e;
}

void idt_init(void)
{
	// Set up the IDT exception entries
	ISR_EXCEPTION(0);
	ISR_EXCEPTION(1);
	ISR_EXCEPTION_IST(2, 1); // NMI
	ISR_EXCEPTION(3);
	ISR_EXCEPTION(4);
	ISR_EXCEPTION(5);
	ISR_EXCEPTION(6);
	ISR_EXCEPTION(7);
	ISR_EXCEPTION_IST(8, 2); // Double fault
	ISR_EXCEPTION(9);
	ISR_EXCEPTION(10);
	ISR_EXCEPTION(11);
	ISR_EXCEPTION(12);
	ISR_EXCEPTION(13);
	ISR_EXCEPTION(14);
	ISR_EXCEPTION(15);
	ISR_EXCEPTION(16);
	ISR_EXCEPTION(17);
	ISR_EXCEPTION(18);
	ISR_EXCEPTION(19);
	ISR_EXCEPTION(20);
	ISR_EXCEPTION(21);
	ISR_EXCEPTION(22);
	ISR_EXCEPTION(23);
	ISR_EXCEPTION(24);
	ISR_EXCEPTION(25);
	ISR_EXCEPTION(26);
	ISR_EXCEPTION(27);
	ISR_EXCEPTION(28);
	ISR_EXCEPTION(29);
	ISR_EXCEPTION(30);
	ISR_EXCEPTION(31);

	// Install the old unused PIC entries
	for (size_t i = 0; i < 16; i++)
		idt_set_isr((uint8_t)(i + 32), isr_noop, IST_NONE, FLAGS_NONE);

	// Install the generic IRQ handler for the rest, until the NMI entries
	for (size_t i = IRQ_APIC_BASE; i < IRQ_NMI_BASE; i++)
		idt_set_isr((uint8_t)i, isr_irq, IST_NONE, FLAGS_NONE);

	// Ignore all the rest (they are NMI entries or spurious interrupt entries
	for (size_t i = IRQ_NMI_BASE; i < 256; i++)
		idt_set_isr((uint8_t)i, isr_noop, IST_NONE, FLAGS_NONE);

	// Ignore the PIT, for now
	idt_set_isr(IRQ_APIC_BASE, isr_noop, IST_NONE, FLAGS_NONE);
}
