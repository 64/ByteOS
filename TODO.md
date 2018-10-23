# TODO

- [ ] Use `const ptr*` types for function parameters
- [ ] Finish `slob` code - merge blocks
- [ ] Calibrate local APIC timer, RTC (and HPET?)
- [ ] Lazy TLB shootdown and IPIs (https://forum.osdev.org/viewtopic.php?f=15&t=23919, http://archive.is/KnVr6)
- [ ] Use init data section
- [ ] Save caller preserved registers on syscall?
- [ ] Separate IST stacks per CPU
- [ ] Reap dead processes (stack, struct task etc)
- [ ] Implement out of memory checks (killing a process appropriately)
- [ ] Use unions in struct page
- [ ] Per-CPU IDT
- [ ] RAII style mutex macro
- [ ] Rewrite the shitty bits
- [ ] Inode locking
- [ ] Proper mutex, rename rwspin
- [ ] Process table
