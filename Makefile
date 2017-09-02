CFLAGS += -ffreestanding -mno-red-zone -Wall -Wextra -std=gnu11
CFLAGS += -Iinclude -Iinclude/kernel -g -Werror -mcmodel=kernel
CFLAGS += -O1 # -msse -msse2
NASM_FLAGS := -f elf64 -F dwarf -g -w+all -Werror

KERNEL_LINK_FLAGS := $(LDFLAGS) -n -nostdlib -Lbuild -lk -lgcc
KERNEL_COMPILE_FLAGS := $(CFLAGS)
KERNEL_OBJS := $(addsuffix .o,$(shell find kernel -name '*.[cs]'))
DEPFILES := $(patsubst %.o,%.d,$(KERNEL_OBJS))

LIBK_COMPILE_FLAGS := $(CFLAGS)
LIBK_OBJS := $(addsuffix .o,$(shell find libk -name '*.[cs]'))
DEPFILES += $(patsubst %.o,%.d,$(LIBK_OBJS))

.PHONY: all clean clean-deep run debug disassemble copy-all copy-ds copy-cansid
.SUFFIXES: .o .c .s

all: build/byteos.iso

run: build/byteos.iso
	qemu-system-x86_64 -M accel=kvm:tcg -net none -serial stdio -cdrom build/byteos.iso

clean:
	@rm -rf build
	@rm -f iso/boot/byteos.elf
	@rm -f $(KERNEL_OBJS) $(LIBK_OBJS)

distclean: clean
	@rm -f $(DEPFILES)

debug: build/byteos.iso
	@qemu-system-x86_64 -d cpu_reset -no-reboot -s -S -cdrom build/byteos.iso &
	@../../../deps/bin/gdb
	@pkill qemu

disassemble: build/byteos.elf
	x86_64-elf-objdump --no-show-raw-insn -d -Mintel build/byteos.elf | source-highlight -s asm -f esc256 | less -eRiMX

copy-all: copy-ds copy-cansid

copy-ds:
	cp ../ds/include/ds/*.h ./include/kernel/ds
	cp ../ds/src/*.c ./kernel/ds

copy-cansid:
	cp ../cansid/cansid.c ./kernel/drivers/vga_tmode
	cp ../cansid/cansid.h ./include/kernel/drivers/
	sed -i '/^#include "cansid.h"/c\#include "drivers/cansid.h"' ./kernel/drivers/vga_tmode/cansid.c

iso/boot/byteos.elf: build/byteos.elf
	cp $< $@

build/:
	mkdir build

build/libk.a: $(LIBK_OBJS)
	x86_64-elf-ar rcs $@ $(LIBK_OBJS)

build/byteos.iso: build/ iso/boot/byteos.elf
	grub-mkrescue -o $@ iso 2> /dev/null

build/byteos.elf: $(KERNEL_OBJS) build/libk.a
	x86_64-elf-gcc -T linker.ld -o $@ $(KERNEL_OBJS) $(KERNEL_LINK_FLAGS)
	x86_64-elf-objcopy --only-keep-debug build/byteos.elf build/byteos.sym
	x86_64-elf-objcopy --strip-debug build/byteos.elf
	grub-file --is-x86-multiboot2 $@

kernel/%.s.o: kernel/%.s
	nasm $(NASM_FLAGS) -MD $(addsuffix .d,$<) $< -o $@

kernel/%.c.o: kernel/%.c
	x86_64-elf-gcc -c $< -o $@ -MD $(KERNEL_COMPILE_FLAGS)

libk/%.c.o: libk/%.c
	x86_64-elf-gcc -c $< -o $@ -MD $(LIBK_COMPILE_FLAGS)

-include $(DEPFILES)
