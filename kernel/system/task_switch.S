.section .text
.align 4
.global task_switch
.type task_switch,@function
task_switch:
	pusha
	pushf
	mov %cr3, %eax
	push %eax
	mov 44(%esp), %eax
	mov %ebx, 4(%eax)
	mov %ecx, 8(%eax)
	mov %edx, 12(%eax)
	mov %esi, 16(%eax)
	mov %edi, 20(%eax)
	mov 36(%esp), %ebx
	mov 40(%esp), %ecx
	mov 20(%esp), %edx
	add $4, %edx
	mov 16(%esp), %esi
	mov 4(%esp), %edi
	mov %ebx, (%eax)
	mov %edx, 24(%eax)
	mov %esi, 28(%eax)
	mov %ecx, 32(%eax)
	mov %edi, 36(%eax)
	pop %ebx
	mov %ebx, 40(%eax)
	push %ebx
	mov 48(%esp), %eax
	mov 4(%eax), %ebx
	mov 8(%eax), %ecx
	mov 12(%eax), %edx
	mov 16(%eax), %esi
	mov 20(%eax), %edi
	mov 28(%eax), %ebp
	push %eax
	mov 36(%eax), %eax
	push %eax
	popf
	pop %eax
	mov 24(%eax), %esp
	push %eax
	mov 40(%eax), %eax
	mov %eax, %cr3
	pop %eax
	push %eax
	mov 32(%eax), %eax
	xchg (%esp), %eax
	mov (%eax), %eax
	ret
