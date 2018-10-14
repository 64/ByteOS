# TODO

- [ ] Use `const ptr*` types for function parameters
- [ ] Finish `slob` code - merge blocks
- [ ] Calibrate local APIC timer, RTC (and HPET?)
- [ ] Lazy TLB shootdown and IPIs (https://forum.osdev.org/viewtopic.php?f=15&t=23919, http://archive.is/KnVr6)
- [ ] Use init data section
- [x] Implement address space sharing for threading
- [ ] Finish run queue balancing
- [ ] Implement priority based scheduling
- [ ] Save caller preserved registers on syscall?
- [ ] Separate IST stacks per CPU
- [ ] Reap dead processes (stack, struct task etc)
- [x] Implement R/W locks for page tables
- [ ] Implement out of memory checks (killing a process appropriately)
- [ ] Use unions in struct page
- [ ] Per-CPU IDT
- [ ] 4 cores
