set architecture i386:x86-64
set disassembly-flavor intel
set pagination off
target remote localhost:1234
symbol-file build/byteos.sym
break long_mode_start
continue
define hook-stop
disassemble
end