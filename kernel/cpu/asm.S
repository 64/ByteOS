.section .text
.align 4

.global cpuid_supported
.type cpuid_supported, @function

# Taken from http://wiki.osdev.org/CPUID
cpuid_supported:
	pushf                    # Save EFLAGS
	pushf                    # Store EFLAGS
	xorl $0x00200000, (%esp) # Invert the ID bit in stored EFLAGS
	popf                     # Load stored EFLAGS (with ID bit inverted)
	pushf                    # Store EFLAGS again (ID bit may or may not be inverted)
	pop %eax                 # eax = modified EFLAGS (ID bit may or may not be inverted)
	xorl (%esp), %eax        # eax = whichever bits were changed
	popf                     # Restore original EFLAGS
	andl $0x00200000, %eax   # eax = zero if ID bit can't be changed, else non-zero
	ret                      # Return

.global sse_set_enabled
.type sse_set_enabled, @function

sse_set_enabled:
	mov %cr0, %eax
	and $0xFFFB, %ax
	or $0x2, %ax
	mov %eax, %cr0
	mov %cr4, %eax
	or 3 << 9, %ax
	mov %eax, %cr4
	ret
