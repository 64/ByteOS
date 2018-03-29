# Thank you https://github.com/no92 for cleaning this up a lot!
ISO		:= build/byteos.iso
KERNEL		:= build/byteos.elf
OS		?= $(shell uname -s)

AS		:= nasm
QEMU		?= qemu-system-x86_64
BOCHS           ?= bochs
AR		:= x86_64-elf-ar
CC		?= cc
TEST_CC		:= $(CC)
CC		:= x86_64-elf-gcc
OBJDUMP		:= x86_64-elf-objdump
READELF		:= x86_64-elf-readelf
OBJCOPY		:= x86_64-elf-objcopy
GDB		?= gdb
PYTHON		?= python3

CFLAGS		+= -ffreestanding -mno-red-zone -mcmodel=kernel -Iinclude -std=gnu11
CFLAGS		+= -Wall -Werror -Wextra -Wparentheses -Wmissing-declarations -Wunreachable-code -Wunused 
CFLAGS		+= -Wmissing-field-initializers -Wmissing-prototypes -Wpointer-arith -Wswitch-enum
CFLAGS		+= -Wredundant-decls -Wshadow -Wstrict-prototypes -Wswitch-default -Wuninitialized
CFLAGS		+= -mno-sse -mno-mmx -mno-sse2 -mno-sse3 -mno-ssse3 -mno-sse4 -mno-sse4.1 -mno-sse4.2 -mno-avx -mno-sse4a
DEBUG_CFLAGS    ?= -fsanitize=undefined -Og -g -DDEBUG
RELEASE_CFLAGS  ?= -O3 -flto
ASFLAGS		?= -f elf64 -F dwarf -g -w+all -Werror -i$(shell pwd)/include/
QEMUFLAGS	?= -net none -smp sockets=1,cores=4,threads=1 -serial stdio -cdrom $(ISO)
ASTYLEFLAGS	:= --style=linux -z2 -k3 -H -xg -p -T8 -S
BOCHSFLAGS      ?= -f .bochsrc -q

CRTI_OBJ	:= kernel/crt/crti.asm.o
CRTBEGIN_OBJ    := kernel/crt/crtbegin.o
CRTEND_OBJ	:= kernel/crt/crtend.o
CRTN_OBJ	:= kernel/crt/crtn.asm.o
LIBK_OBJ	:= $(addsuffix .o,$(shell find libk -not -path "*tests*" -name '*.c' -o -name '*.asm'))
KERNEL_OBJ_RAW	:= $(addsuffix .o,$(shell find kernel -path kernel/crt -prune -type f -o -name '*.c' -o -name '*.asm'))
KERNEL_OBJ_ALL	:= $(CRTI_OBJ) $(CRTN_OBJ) $(KERNEL_OBJ_RAW) $(LIBK_OBJ)
KERNEL_OBJ	:= $(CRTI_OBJ) $(CRTBEGIN_OBJ) $(KERNEL_OBJ_RAW) $(LIBK_OBJ) $(CRTEND_OBJ) $(CRTN_OBJ)
KERNEL_SRC_DEPS := include/gen/syscall_gen.h include/gen/syscall_gen.c include/gen/syscall_gen.asm

DEPFILES	:= $(patsubst %.o,%.d,$(KERNEL_OBJ_ALL))

LIBK_TESTABLE	:= $(addprefix libk/,string.c)
DEPFILES	+= $(patsubst %.o,%.d,$(LIBK_OBJ))
TIME_START 	:= $(shell date +"%s.%N")

MOD_DS		:= vendor/ds
MOD_CANSID	:= vendor/cansid
MOD_SNOW	:= vendor/snow
SUBMODULES	:= ds cansid snow

ifeq ($(OS),Linux)
	QEMUFLAGS += -M accel=kvm:tcg
endif

ifeq ($(DEBUG),0)
	CFLAGS += $(RELEASE_CFLAGS)
else
	CFLAGS += $(DEBUG_CFLAGS)
endif

ifeq ($(VERBOSE),1)
	CFLAGS += -DVERBOSE
endif

.PHONY: all clean run vbox bochs gdb disassemble update-modules copy-all copy-snow copy-ds copy-cansid loc tidy test
.SUFFIXES: .o .c .asm

all: $(ISO)

run: $(ISO)
	@$(QEMU) $(QEMUFLAGS)

bochs: $(ISO)
	@$(BOCHS) $(BOCHSFLAGS)

vbox: $(ISO)
	@virtualbox --startvm "ByteOS" --dbg

clean:
	@$(RM) -r build
	@$(RM) -r include/gen
	@$(RM) iso/boot/byteos.elf
	@$(RM) -v $(shell find kernel libk -name "*.orig")
	@$(RM) $(KERNEL_OBJ_ALL) $(LIBK_OBJ)
	@$(RM) $(DEPFILES)

gdb: $(ISO)
	@$(QEMU) $(QEMUFLAGS) -no-reboot -s -S &
	@sleep 0.5
	@$(GDB)
	@pkill qemu

disassemble: build/ $(KERNEL)
	@$(OBJDUMP) --no-show-raw-insn -d -Mintel $(KERNEL) | source-highlight -s asm -f esc256 | less -eRiMX

symbols: build/ $(KERNEL)
	@$(READELF) build/byteos.sym -s | less

update-modules:
	git submodule update --init --recursive --remote

$(MOD_SNOW) $(MOD_DS) $(MOD_CANSID): update-modules

copy-all: copy-ds copy-cansid copy-snow

copy-snow: $(MOD_SNOW)
	@cp $(MOD_SNOW)/snow/snow.h ./libk/tests/snow.h

copy-ds: $(MOD_DS)
	@cp $(MOD_DS)/include/ds/*.h ./include/ds/
	@cp $(MOD_DS)/src/*.c ./libk/ds/

copy-cansid: $(MOD_CANSID)
	@cp $(MOD_CANSID)/cansid.c ./kernel/drivers/vga_tmode
	@cp $(MOD_CANSID)/cansid.h ./include/drivers/
	@cat ./kernel/drivers/vga_tmode/cansid.c | sed 's/cansid.h/drivers\/cansid.h/g' > temp.c
	@mv temp.c ./kernel/drivers/vga_tmode/cansid.c

loc:
	@cloc --vcs=git --force-lang="C",h --exclude-lang="Markdown"

tidy:
	@astyle $(filter-out libk/tests/snow.h,$(shell find kernel libk -name "*.[ch]")) $(ASTYLEFLAGS)

build/tmain: build/ libk/tests/tstring.c
	$(TEST_CC) libk/tests/tmain.c -Iinclude -Wall -std=gnu11 -o $@

test: build/tmain
	@./build/tmain	

iso/boot/byteos.elf: $(KERNEL)
	@cp $< $@

build/:
	@mkdir build
	@mkdir -p include/gen

$(ISO): build/ iso/boot/byteos.elf
	@printf "\t\e[32;1mCreating\e[0m $(ISO)\n"
	@grub-mkrescue -o $@ iso 2> /dev/null
	@printf "\t\e[32;1;4mDone\e[24m in $(shell date +%s.%3N --date='$(TIME_START) seconds ago')s\e[0m\n"

$(KERNEL): $(KERNEL_SRC_DEPS) $(KERNEL_OBJ_ALL)
	@printf "\t\e[32;1mLinking\e[0m $(KERNEL)\n"
	@$(CC) -T linker.ld -o $@ $(KERNEL_OBJ) $(LDFLAGS) -n -nostdlib -lgcc
	@$(OBJCOPY) --only-keep-debug $(KERNEL) build/byteos.sym
	@$(OBJCOPY) --strip-debug $(KERNEL)
	@grub-file --is-x86-multiboot2 $@

kernel/%.asm.o: kernel/%.asm
	@printf "\t\e[32;1mAssembling\e[0m $<\n"
	@$(AS) $(ASFLAGS) -MD $(addsuffix .d,$<) $< -o $@

kernel/%.c.o: kernel/%.c
	@printf "\t\e[32;1mCompiling\e[0m $<\n"
	@$(CC) -c $< -o $@ -MMD $(CFLAGS)

libk/%.c.o: libk/%.c
	@printf "\t\e[32;1mCompiling\e[0m $<\n"
	@$(CC) -c $< -o $@ -MMD $(CFLAGS)

# Syscall generation code
include/gen/syscall_gen.h:
	@$(PYTHON) util/syscall_gen.py h
include/gen/syscall_gen.c:
	@$(PYTHON) util/syscall_gen.py c
include/gen/syscall_gen.asm: util/syscall_gen.py
	@$(PYTHON) util/syscall_gen.py asm

-include $(DEPFILES)
