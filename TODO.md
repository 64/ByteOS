# TODO

[ ] Use `const ptr*` types for function parameters
[ ] Finish `slob` code - merge blocks
[ ] Add paging code to unmap tables (for trampoline code etc.)
[ ] Should we have a separate interrupt stack per-CPU?
[ ] Should task registers be stored on the stack?
[ ] Use 2MB pages
[ ] Calibrate local APIC timer and RTC
[ ] Lazy TLB shootdown and IPIs (https://forum.osdev.org/viewtopic.php?f=15&t=23919)
[ ] Disable the PIC if necessary with ICMR (https://forum.osdev.org/viewtopic.php?p=107868#107868)
[ ] Num CPUs started
[ ] Refcount page tables
[ ] Have the `pit_sleep_ms` function wait on a given variable for SMP startup
