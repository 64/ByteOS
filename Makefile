HOST?=i686-elf
PROJECTS?=kernel libc

.PHONY: all build clean run rebuild debug-server debug-gdb

all: bin/byteos.iso

build:
	@mkdir -p bin
	@./scripts/build.sh

bin/byteos.iso: isodir/boot/byteos.bin isodir/boot/grub/grub.cfg
	@grub-mkrescue -o $@ isodir 2> /dev/null

isodir/boot/byteos.bin: build
	@mkdir -p isodir/boot
	@rsync -quraE sysroot/boot/byteos.bin isodir/boot/byteos.bin

isodir/boot/grub/grub.cfg: build
	@mkdir -p isodir/boot/grub
	@printf "menuentry \"byteos\" {\n\tmultiboot /boot/byteos.bin \n}\n" > isodir/boot/grub/grub.cfg

clean:
	@for PROJECT in $(PROJECTS); do \
	  cd $$PROJECT && make clean && cd ../; \
	done
	@rm -rf sysroot
	@rm -rf isodir
	@rm -rf bin

rebuild: clean build

debug-server: bin/byteos.iso
	qemu-system-i386 -s -cdrom bin/byteos.iso

debug-gdb:
	sudo ../vendor/gdb/gdb/gdb

run: bin/byteos.iso
	qemu-system-i386 -cdrom bin/byteos.iso

vbox: bin/byteos.iso
	vboxmanage startvm "ByteOS"
