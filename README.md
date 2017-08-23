# ByteOS

A simple hobby operating system for the x86_64 architecture, written in C.

![Screenshot](https://i.gyazo.com/cbd3707fdcc2e3e01776f62399c53a1b.png)

## Building

You will need:
* GNU `binutils` for `x86_64-elf`
* `gcc` for `x86_64-elf` with `-mno-red-zone` and C11 capabilities ([instructions](http://wiki.osdev.org/Libgcc_without_red_zone))
* Make
* NASM
* GRUB 2.02

Then, to build `byteos.iso`:
```sh
make
```

## Running

You will need (in addition to the above dependencies):
* QEMU `x86_64`
* Debugging also requires a [specially patched version of GDB](http://wiki.osdev.org/QEMU_and_GDB_in_long_mode#Workaround_2%3A_Patching_GDB) for interoperability with QEMU

Then, to run in QEMU:
```sh
make run
```
or to debug with GDB:
```sh
make debug
```

See [`Makefile`](https://github.com/64/ByteOS/blob/master/Makefile) for more details.

## Contributing

Feel free to open an issue if you have any questions/concerns or a pull-request if you would like to contribute some code.
