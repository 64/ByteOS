CFLAGS += -ffreestanding -mno-red-zone -Wall -Wextra -std=gnu11
CFLAGS += -Iinclude -Iinclude/kernel -g -Werror -mcmodel=kernel
CFLAGS += -O1 # -msse -msse2
NASM_FLAGS := -f elf64 -F dwarf -g -w+all -Werror

KERNEL_LINK_FLAGS = $(LDFLAGS) -n -nostdlib -Lbuild -lk -lgcc
KERNEL_COMPILE_FLAGS = $(CFLAGS)
KERNEL_OBJ_LIST = \
boot.o \
long_mode.o \
interrupts.o \
isr_handler.o \
vga_tmode.o \
cansid.o \
serial.o \
pmm.o \
boot_heap.o \
bitmap.o \
kmain.o
KERNEL_OBJS = $(addprefix build/,$(KERNEL_OBJ_LIST))

LIBK_COMPILE_FLAGS = $(CFLAGS)
LIBK_OBJ_LIST = \
string.o \
kprintf.o \
abort.o
LIBK_OBJS = $(addprefix build/,$(LIBK_OBJ_LIST))

.PHONY: all clean run debug disassemble copy-all copy-ds copy-cansid
.SUFFIXES: .o .c .asm

all: build/byteos.iso

run: build/byteos.iso
	qemu-system-x86_64 -serial stdio -cdrom build/byteos.iso

clean:
	rm -rf build
	rm -f iso/boot/byteos.elf

debug: build/byteos.iso
	qemu-system-x86_64 -d cpu_reset -no-reboot -s -S -cdrom build/byteos.iso &
	../../../deps/bin/gdb
	pkill qemu

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

define kernel_folder
build/%.o: $1/%.asm
	nasm $(NASM_FLAGS) -MD $$(patsubst %.o,%.d,$$@) $$< -o $$@

build/%.o: $1/%.c
	x86_64-elf-gcc -c $$< -o $$@ -MD $(KERNEL_COMPILE_FLAGS)
endef

$(eval $(call kernel_folder,kernel))
$(eval $(call kernel_folder,kernel/cpu))
$(eval $(call kernel_folder,kernel/mm))
$(eval $(call kernel_folder,kernel/ds))
$(eval $(call kernel_folder,kernel/drivers/serial))
$(eval $(call kernel_folder,kernel/drivers/vga_tmode))

build/%.o: libk/%.c
	x86_64-elf-gcc -c $< -o $@ -MD $(LIBK_COMPILE_FLAGS)

-include build/*.d
