AS			:= nasm
EMU			:= qemu-system-x86_64
AR			:= x86_64-elf-ar
CC			:= x86_64-elf-gcc
OBJDUMP		:= x86_64-elf-objdump
OBJCOPY		:= x86_64-elf-objcopy

CFLAGS		?= -O1 -g
CFLAGS		+= -ffreestanding -mno-red-zone -mcmodel=kernel -Iinclude -Iinclude/kernel -std=gnu11
CFLAGS		+= -Werror -Wall -Wextra
ASFLAGS		:= -f elf64 -F dwarf -g -w+all -Werror
EMUFLAGS	:= -M accel=kvm:tcg -net none -serial stdio -cdrom build/byteos.iso

KERNEL_OBJ	:= $(addsuffix .o,$(shell find kernel -name '*.[cs]'))
DEPFILES	:= $(patsubst %.o,%.d,$(KERNEL_OBJ))

LIBK_OBJ	:= $(addsuffix .o,$(shell find libk -name '*.[cs]'))
DEPFILES	+= $(patsubst %.o,%.d,$(LIBK_OBJ))

.PHONY: all clean distclean run debug disassemble
.SUFFIXES: .o .c .s

all: build/byteos.iso

run: build/byteos.iso
	@$(EMU) $(EMUFLAGS)

clean:
	@$(RM) -r build
	@$(RM) iso/boot/byteos.elf
	@$(RM) $(KERNEL_OBJ) $(LIBK_OBJ)

distclean: clean
	@$(RM) $(DEPFILES)

debug: build/byteos.iso
	@$(EMU) $(EMUFLAGS) -d cpu_reset -no-reboot -s -S &
	@../../../deps/bin/gdb
	@pkill qemu

disassemble: build/byteos.elf
	@$(OBJDUMP) --no-show-raw-insn -d -Mintel build/byteos.elf | source-highlight -s asm -f esc256 | less -eRiMX

iso/boot/byteos.elf: build/byteos.elf
	@cp $< $@

build/:
	@mkdir build

build/libk.a: $(LIBK_OBJ)
	@$(AR) rcs $@ $(LIBK_OBJ)

build/byteos.iso: build/ iso/boot/byteos.elf
	@grub-mkrescue -o $@ iso 2> /dev/null

build/byteos.elf: $(KERNEL_OBJ) build/libk.a
	@$(CC) -T linker.ld -o $@ $(KERNEL_OBJ) $(LDFLAGS) -n -nostdlib -Lbuild -lk -lgcc
	@$(OBJCOPY) --only-keep-debug build/byteos.elf build/byteos.sym
	@$(OBJCOPY) --strip-debug build/byteos.elf
	@grub-file --is-x86-multiboot2 $@

kernel/%.s.o: kernel/%.s
	@$(AS) $(ASFLAGS) -MD $(addsuffix .d,$<) $< -o $@

kernel/%.c.o: kernel/%.c
	@$(CC) -c $< -o $@ -MD $(CFLAGS)

libk/%.c.o: libk/%.c
	@$(CC) -c $< -o $@ -MD $(CFLAGS)

-include $(DEPFILES)
