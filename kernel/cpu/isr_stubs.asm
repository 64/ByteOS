%include "include.asm"

; Interrupt descriptor table
section .data
align 16
global idt64
idt64:
	times 256 dq 0, 0
.pointer:
	dw $ - idt64 - 1
	dq idt64

section .text
global load_idt
load_idt:
	mov rax, idt64.pointer ; Is this needed?
	lidt [rax]
	ret

%macro isr_common_fn 1
	push rdi
	push rsi
	push rdx
	push rcx
	push rax
	push r8
	push r9
	push r10
	push r11

	; Call the handler
	mov rdi, rsp
	extern %1
	call %1
	
	extern ret_from_interrupt
	jmp ret_from_interrupt
%endmacro

isr_common_exception:
	isr_common_fn exception_handler

isr_common_irq:
	isr_common_fn irq_handler

%macro isr_stub_err 1
global isr_stub_%1
isr_stub_%1:
	; Store the interrupt number in the highest four bytes of the error code
	; This way we can always increment rsp by 8 before iretq and no memory is wasted.
	mov dword [rsp + 4], %1
	jmp isr_common_exception
%endmacro

%macro isr_stub_noerr 1
global isr_stub_%1
isr_stub_%1:
	; For consistency with the err variant
	push qword 0
	mov dword [rsp + 4], %1
	jmp isr_common_exception
%endmacro

%macro isr_stub_irq 1
global isr_stub_%1
isr_stub_%1:
	push qword %1
	jmp isr_common_irq
%endmacro

%macro isr_stub_nop 1
global isr_stub_%1
isr_stub_%1:
	iretq
%endmacro

%macro isr_stub_ipi 2
global isr_stub_%1
isr_stub_%1:
	push qword %1
	isr_common_fn ipi_%2
%endmacro

; Exceptions
isr_stub_noerr 0
isr_stub_noerr 1
isr_stub_noerr 2
isr_stub_noerr 3
isr_stub_noerr 4
isr_stub_noerr 5
isr_stub_noerr 6
isr_stub_noerr 7
isr_stub_err 8
isr_stub_noerr 9
isr_stub_err 10
isr_stub_err 11
isr_stub_err 12
isr_stub_err 13
isr_stub_err 14
isr_stub_noerr 15
isr_stub_noerr 16
isr_stub_err 17
isr_stub_noerr 18
isr_stub_noerr 19
isr_stub_noerr 20
isr_stub_noerr 21
isr_stub_noerr 22
isr_stub_noerr 23
isr_stub_noerr 24
isr_stub_noerr 25
isr_stub_noerr 26
isr_stub_noerr 27
isr_stub_noerr 28
isr_stub_noerr 29
isr_stub_err 30
isr_stub_noerr 31

; Unused PIC interrupts
isr_stub_nop 32
isr_stub_nop 33
isr_stub_nop 34
isr_stub_nop 35
isr_stub_nop 36
isr_stub_nop 37
isr_stub_nop 38
isr_stub_nop 39
isr_stub_nop 40
isr_stub_nop 41
isr_stub_nop 42
isr_stub_nop 43
isr_stub_nop 44
isr_stub_nop 45
isr_stub_nop 46
isr_stub_nop 47

; Other (IRQs)
isr_stub_irq 48
isr_stub_irq 49
isr_stub_irq 50
isr_stub_irq 51
isr_stub_irq 52
isr_stub_irq 53
isr_stub_irq 54
isr_stub_irq 55
isr_stub_irq 56
isr_stub_irq 57
isr_stub_irq 58
isr_stub_irq 59
isr_stub_irq 60
isr_stub_irq 61
isr_stub_irq 62
isr_stub_irq 63
isr_stub_irq 64
isr_stub_irq 65
isr_stub_irq 66
isr_stub_irq 67
isr_stub_irq 68
isr_stub_irq 69
isr_stub_irq 70
isr_stub_irq 71
isr_stub_irq 72
isr_stub_irq 73
isr_stub_irq 74
isr_stub_irq 75
isr_stub_irq 76
isr_stub_irq 77
isr_stub_irq 78
isr_stub_irq 79
isr_stub_irq 80
isr_stub_irq 81
isr_stub_irq 82
isr_stub_irq 83
isr_stub_irq 84
isr_stub_irq 85
isr_stub_irq 86
isr_stub_irq 87
isr_stub_irq 88
isr_stub_irq 89
isr_stub_irq 90
isr_stub_irq 91
isr_stub_irq 92
isr_stub_irq 93
isr_stub_irq 94
isr_stub_irq 95
isr_stub_irq 96
isr_stub_irq 97
isr_stub_irq 98
isr_stub_irq 99
isr_stub_irq 100
isr_stub_irq 101
isr_stub_irq 102
isr_stub_irq 103
isr_stub_irq 104
isr_stub_irq 105
isr_stub_irq 106
isr_stub_irq 107
isr_stub_irq 108
isr_stub_irq 109
isr_stub_irq 110
isr_stub_irq 111
isr_stub_irq 112
isr_stub_irq 113
isr_stub_irq 114
isr_stub_irq 115
isr_stub_irq 116
isr_stub_irq 117
isr_stub_irq 118
isr_stub_irq 119
isr_stub_irq 120
isr_stub_irq 121
isr_stub_irq 122
isr_stub_irq 123
isr_stub_irq 124
isr_stub_irq 125
isr_stub_irq 126
isr_stub_irq 127
isr_stub_irq 128
isr_stub_irq 129
isr_stub_irq 130
isr_stub_irq 131
isr_stub_irq 132
isr_stub_irq 133
isr_stub_irq 134
isr_stub_irq 135
isr_stub_irq 136
isr_stub_irq 137
isr_stub_irq 138
isr_stub_irq 139
isr_stub_irq 140
isr_stub_irq 141
isr_stub_irq 142
isr_stub_irq 143
isr_stub_irq 144
isr_stub_irq 145
isr_stub_irq 146
isr_stub_irq 147
isr_stub_irq 148
isr_stub_irq 149
isr_stub_irq 150
isr_stub_irq 151
isr_stub_irq 152
isr_stub_irq 153
isr_stub_irq 154
isr_stub_irq 155
isr_stub_irq 156
isr_stub_irq 157
isr_stub_irq 158
isr_stub_irq 159
isr_stub_irq 160
isr_stub_irq 161
isr_stub_irq 162
isr_stub_irq 163
isr_stub_irq 164
isr_stub_irq 165
isr_stub_irq 166
isr_stub_irq 167
isr_stub_irq 168
isr_stub_irq 169
isr_stub_irq 170
isr_stub_irq 171
isr_stub_irq 172
isr_stub_irq 173
isr_stub_irq 174
isr_stub_irq 175
isr_stub_irq 176
isr_stub_irq 177
isr_stub_irq 178
isr_stub_irq 179
isr_stub_irq 180
isr_stub_irq 181
isr_stub_irq 182
isr_stub_irq 183
isr_stub_irq 184
isr_stub_irq 185
isr_stub_irq 186
isr_stub_irq 187
isr_stub_irq 188
isr_stub_irq 189
isr_stub_irq 190
isr_stub_irq 191
isr_stub_irq 192
isr_stub_irq 193
isr_stub_irq 194
isr_stub_irq 195
isr_stub_irq 196
isr_stub_irq 197
isr_stub_irq 198
isr_stub_irq 199
isr_stub_irq 200
isr_stub_irq 201
isr_stub_irq 202
isr_stub_irq 203
isr_stub_irq 204
isr_stub_irq 205
isr_stub_irq 206
isr_stub_irq 207
isr_stub_irq 208
isr_stub_irq 209
isr_stub_irq 210
isr_stub_irq 211
isr_stub_irq 212
isr_stub_irq 213
isr_stub_irq 214
isr_stub_irq 215
isr_stub_irq 216
isr_stub_irq 217
isr_stub_irq 218
isr_stub_irq 219
isr_stub_irq 220
isr_stub_irq 221
isr_stub_irq 222
isr_stub_irq 223
isr_stub_irq 224
isr_stub_irq 225
isr_stub_irq 226
isr_stub_irq 227
isr_stub_irq 228
isr_stub_irq 229
isr_stub_irq 230
isr_stub_irq 231
isr_stub_irq 232
isr_stub_irq 233
isr_stub_irq 234
isr_stub_irq 235
isr_stub_irq 236
isr_stub_irq 237
isr_stub_irq 238
isr_stub_irq 239
isr_stub_irq 240
isr_stub_irq 241
isr_stub_irq 242
isr_stub_irq 243
isr_stub_irq 244
isr_stub_irq 245
isr_stub_irq 246
isr_stub_irq 247
isr_stub_irq 248
isr_stub_irq 249
isr_stub_irq 250
isr_stub_ipi 251, tlb_shootdown
isr_stub_ipi 252, abort ; Must match interrupts.h
isr_stub_irq 253 ; LINT0
isr_stub_irq 254 ; LINT1
isr_stub_nop 255 ; APIC Spurious interrupt vector
