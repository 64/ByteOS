CFLAGS?=-Og
CPPFLAGS?=
LDFLAGS?=
LIBS?=

DESTDIR?=sysroot
PREFIX?=/usr/local
EXEC_PREFIX?=$(PREFIX)
BOOTDIR?=$(EXEC_PREFIX)/boot
INCLUDEDIR?=$(PREFIX)/include

CFLAGS:=$(CFLAGS) -ffreestanding -Wall -Wextra -fmax-errors=1 -Werror -Wno-unused-function
CPPFLAGS:=$(CPPFLAGS) -D__is_kernel -Iinclude
LDFLAGS:=$(LDFLAGS)
LIBS:=$(LIBS) -nostdlib -lk -lgcc

CFLAGS:=$(CFLAGS)
CPPFLAGS:=$(CPPFLAGS)
LDFLAGS:=$(LDFLAGS)
LIBS:=$(LIBS)

KERNEL_OBJS=\
boot.o \
kernel.o \
cpu/gdt_load.o \
cpu/gdt.o \
cpu/idt.o \
cpu/idt_load.o \
cpu/isr.o \
cpu/tss.o \
cpu/tss_load.o \
cpu/isrs.o \
cpu/irq.o \
cpu/asm.o \
cpu/cpu_extensions.o \
cpu/syscalls.o \
memory/memory.o \
memory/paging.o \
memory/stack_smash.o \
memory/kheap.o \
memory/pmm.o \
system/task.o \
system/task_switch.o \
drivers/vga/vga_textmode.o \
drivers/pit/pit.o \
drivers/ps2/kbd.o \
drivers/ps2/mouse.o \
drivers/ps2/ps2main.o \
drivers/acpi/acpi.o \
drivers/cmos/cmos.o \
algs/bitset.o \
algs/c_queue.o \

OBJS=\
util/crti.o \
crtbegin.o \
$(KERNEL_OBJS) \
crtend.o \
util/crtn.o \

LINK_LIST=\
$(LDFLAGS) \
util/crti.o \
crtbegin.o \
$(KERNEL_OBJS) \
$(LIBS) \
crtend.o \
util/crtn.o \

.PHONY: all clean install install-headers install-kernel
.SUFFIXES: .o .c .S

all: byteos.bin

byteos.bin: $(OBJS) util/linker.ld
	$(CC) -T util/linker.ld -o $@ $(CFLAGS) $(LINK_LIST)
	@grub-file --is-x86-multiboot byteos.bin
	@$(HOST)-objdump -S --disassemble $@ > ../bin/byteos.S

crtbegin.o crtend.o:
	@OBJ=`$(CC) $(CFLAGS) $(LDFLAGS) -print-file-name=$(@F)` && cp "$$OBJ" $@

%.o: %.c
	$(CC) -MD -c $< -o $@ -std=gnu99 $(CFLAGS) $(CPPFLAGS)

%.o: %.S
	$(CC) -MD -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

clean:
	@rm -f byteos.bin
	@rm -f $(OBJS) *.o */*.o */*/*.o
	@rm -f $(OBJS:.o=.d) *.d */*.d */*/*.d

install: install-headers install-kernel

install-headers:
	@mkdir -p $(DESTDIR)$(INCLUDEDIR)
	@rsync -quraE ./include/. $(DESTDIR)$(INCLUDEDIR)/.

install-kernel: byteos.bin
	@mkdir -p $(DESTDIR)$(BOOTDIR)
	@rsync -quraE byteos.bin $(DESTDIR)$(BOOTDIR)

-include $(OBJS:.o=.d)
