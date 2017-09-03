# Thank you https://github.com/no92 for cleaning this up a lot!
ISO		:= build/byteos.iso
KERNEL		:= build/byteos.elf
OS		:= $(shell uname -s)

AS		:= nasm
EMU		:= qemu-system-x86_64
AR		:= x86_64-elf-ar
CC		:= x86_64-elf-gcc
OBJDUMP		:= x86_64-elf-objdump
OBJCOPY		:= x86_64-elf-objcopy

CFLAGS		?= -O1 -g
CFLAGS		+= -ffreestanding -mno-red-zone -mcmodel=kernel -Iinclude -Iinclude/kernel -std=gnu11
CFLAGS		+= -Wall -Wbad-function-cast -Werror -Wextra -Wparentheses -Wmissing-braces -Wmissing-declarations
CFLAGS		+= -Wmissing-field-initializers -Wmissing-prototypes -Wnested-externs -Wpointer-arith -Wpedantic
CFLAGS		+= -Wredundant-decls -Wshadow -Wstrict-prototypes -Wswitch-default -Wswitch-enum -Wuninitialized -Wunreachable-code
CFLAGS		+= -Wunused
ASFLAGS		:= -f elf64 -F dwarf -g -w+all -Werror
EMUFLAGS	:= -net none -serial stdio -cdrom $(ISO)

KERNEL_OBJ	:= $(addsuffix .o,$(shell find kernel -name '*.c' -o -name '*.asm'))
DEPFILES	:= $(patsubst %.o,%.d,$(KERNEL_OBJ))

LIBK_OBJ	:= $(addsuffix .o,$(shell find libk -name '*.c' -o -name '*.asm'))
DEPFILES	+= $(patsubst %.o,%.d,$(LIBK_OBJ))

ifeq ($(OS),Linux)
	EMUFLAGS += -M accel=kvm:tcg
endif

.PHONY: all clean distclean run debug disassemble
.SUFFIXES: .o .c .asm

all: $(ISO)

run: $(ISO)
	@$(EMU) $(EMUFLAGS)

clean:
	@$(RM) -r build
	@$(RM) iso/boot/byteos.elf
	@$(RM) $(KERNEL_OBJ) $(LIBK_OBJ)

distclean: clean
	@$(RM) $(DEPFILES)

debug: $(ISO)
	@$(EMU) $(EMUFLAGS) -d cpu_reset -no-reboot -s -S &
	@../../../deps/bin/gdb
	@pkill qemu

disassemble: $(KERNEL)
	@$(OBJDUMP) --no-show-raw-insn -d -Mintel $(KERNEL) | source-highlight -s asm -f esc256 | less -eRiMX

iso/boot/byteos.elf: $(KERNEL)
	@cp $< $@

build/:
	@mkdir build

build/libk.a: $(LIBK_OBJ)
	@$(AR) rcs $@ $(LIBK_OBJ)

$(ISO): build/ iso/boot/byteos.elf
	@grub-mkrescue -o $@ iso 2> /dev/null

$(KERNEL): $(KERNEL_OBJ) build/libk.a
	@$(CC) -T linker.ld -o $@ $(KERNEL_OBJ) $(LDFLAGS) -n -nostdlib -Lbuild -lk -lgcc
	@$(OBJCOPY) --only-keep-debug $(KERNEL) build/byteos.sym
	@$(OBJCOPY) --strip-debug $(KERNEL)
	@grub-file --is-x86-multiboot2 $@

kernel/%.asm.o: kernel/%.asm
	@$(AS) $(ASFLAGS) -MD $(addsuffix .d,$<) $< -o $@

kernel/%.c.o: kernel/%.c
	@$(CC) -c $< -o $@ -MD $(CFLAGS)

libk/%.c.o: libk/%.c
	@$(CC) -c $< -o $@ -MD $(CFLAGS)

-include $(DEPFILES)
