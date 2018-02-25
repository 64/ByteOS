set architecture i386:x86-64
set disassembly-flavor intel
set pagination off
symbol-file build/byteos.sym
target remote localhost:1234
hbreak long_mode_entry
continue
define hook-stop
list *$pc
end
define myn
next
refresh
end
define mys
step
refresh
end
tui enable
