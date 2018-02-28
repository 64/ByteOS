; Initialises SSE, AVX and AVX-512 (if available)
; TODO: Test this
global simd_init
simd_init:
	push rax
	push rbx
	push rcx
	push rdx

	; Detect XSAVE and AVX
	mov rax, 1
	xor rcx, rcx
	cpuid
	and ecx, (1 << 26) | (1 << 28)
	cmp ecx, (1 << 26) | (1 << 28)
	jne .done

	; Enable XSAVE
	mov rax, cr4
	or eax, (1 << 18)
	mov cr4, rax
	
	; Enable AVX
	mov rcx, 0
	xgetbv
	or eax, (1 << 1) | (1 << 2) ; SSE, AVX bits
	mov rcx, 0
	xsetbv

	; TODO: AVX-512
.done:
	pop rdx
	pop rcx
	pop rbx
	pop rax
	ret
