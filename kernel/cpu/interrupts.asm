%include "include.asm"

section .data
global idt64
idt64:
	times 256 dq 0, 0
.pointer:
	dw $ - idt64 - 1
	dq idt64

section .text
global load_idt
load_idt:
	jmp .isr_install
.isr_post_install:
	; Remap the PIC
	outb 0x20, 0x11
	outb 0xA0, 0x11
	outb 0x21, 0x20
	outb 0xA1, 0x28
	outb 0x21, 0x04
	outb 0xA1, 0x02
	outb 0x21, 0x01
	outb 0xA1, 0x01
	outb 0x21, 0x00
	outb 0xA1, 0x00

	mov rax, idt64.pointer ; Is this needed?
	lidt [rax]
	ret

.isr_install_single:
	mov rdi, idt64
	lea rdi, [rdi + rdx]
	mov word [rdi], ax
	mov word [rdi + 2], 0x08
	mov byte [rdi + 4], sil ; IST index
	mov byte [rdi + 5], 0x8E
	shr rax, 16
	mov word [rdi + 6], ax
	shr rax, 16
	mov dword [rdi + 8], eax
	mov dword [rdi + 12], 0x00000000
	ret

%macro isr_addentry_ist 2
	mov rax, interrupt_service_routines.isr_%1
	mov rdx, %1 * 16
	mov sil, %2
	call .isr_install_single
%endmacro

%macro isr_addentry 1
	isr_addentry_ist %1, 0
%endmacro


.isr_install:
	isr_addentry 0
	isr_addentry 1
	isr_addentry_ist 2, 1 ; NMI
	isr_addentry 3
	isr_addentry 4
	isr_addentry 5
	isr_addentry 6
	isr_addentry 7
	isr_addentry_ist 8, 2 ; Double fault
	isr_addentry 9
	isr_addentry 10
	isr_addentry 11
	isr_addentry 12
	isr_addentry 13
	isr_addentry 14
	isr_addentry 15
	isr_addentry 16
	isr_addentry 17
	isr_addentry 18
	isr_addentry 19
	isr_addentry 20
	isr_addentry 21
	isr_addentry 22
	isr_addentry 23
	isr_addentry 24
	isr_addentry 25
	isr_addentry 26
	isr_addentry 27
	isr_addentry 28
	isr_addentry 29
	isr_addentry 30
	isr_addentry 31
	isr_addentry 32
	isr_addentry 33
	isr_addentry 34
	isr_addentry 35
	isr_addentry 36
	isr_addentry 37
	isr_addentry 38
	isr_addentry 39
	isr_addentry 40
	isr_addentry 41
	isr_addentry 42
	isr_addentry 43
	isr_addentry 44
	isr_addentry 45
	isr_addentry 46
	isr_addentry 47
	isr_addentry 47
	isr_addentry 48
	isr_addentry 49
	isr_addentry 50
	isr_addentry 51
	isr_addentry 52
	isr_addentry 53
	isr_addentry 54
	isr_addentry 55
	isr_addentry 56
	isr_addentry 57
	isr_addentry 58
	isr_addentry 59
	isr_addentry 60
	isr_addentry 61
	isr_addentry 62
	isr_addentry 63
	isr_addentry 64
	isr_addentry 65
	isr_addentry 66
	isr_addentry 67
	isr_addentry 68
	isr_addentry 69
	isr_addentry 70
	isr_addentry 71
	isr_addentry 72
	isr_addentry 73
	isr_addentry 74
	isr_addentry 75
	isr_addentry 76
	isr_addentry 77
	isr_addentry 78
	isr_addentry 79
	isr_addentry 80
	isr_addentry 81
	isr_addentry 82
	isr_addentry 83
	isr_addentry 84
	isr_addentry 85
	isr_addentry 86
	isr_addentry 87
	isr_addentry 88
	isr_addentry 89
	isr_addentry 90
	isr_addentry 91
	isr_addentry 92
	isr_addentry 93
	isr_addentry 94
	isr_addentry 95
	isr_addentry 96
	isr_addentry 97
	isr_addentry 98
	isr_addentry 99
	isr_addentry 100
	isr_addentry 101
	isr_addentry 102
	isr_addentry 103
	isr_addentry 104
	isr_addentry 105
	isr_addentry 106
	isr_addentry 107
	isr_addentry 108
	isr_addentry 109
	isr_addentry 110
	isr_addentry 111
	isr_addentry 112
	isr_addentry 113
	isr_addentry 114
	isr_addentry 115
	isr_addentry 116
	isr_addentry 117
	isr_addentry 118
	isr_addentry 119
	isr_addentry 120
	isr_addentry 121
	isr_addentry 122
	isr_addentry 123
	isr_addentry 124
	isr_addentry 125
	isr_addentry 126
	isr_addentry 127
	isr_addentry 128
	isr_addentry 129
	isr_addentry 130
	isr_addentry 131
	isr_addentry 132
	isr_addentry 133
	isr_addentry 134
	isr_addentry 135
	isr_addentry 136
	isr_addentry 137
	isr_addentry 138
	isr_addentry 139
	isr_addentry 140
	isr_addentry 141
	isr_addentry 142
	isr_addentry 143
	isr_addentry 144
	isr_addentry 145
	isr_addentry 146
	isr_addentry 147
	isr_addentry 148
	isr_addentry 149
	isr_addentry 150
	isr_addentry 151
	isr_addentry 152
	isr_addentry 153
	isr_addentry 154
	isr_addentry 155
	isr_addentry 156
	isr_addentry 157
	isr_addentry 158
	isr_addentry 159
	isr_addentry 160
	isr_addentry 161
	isr_addentry 162
	isr_addentry 163
	isr_addentry 164
	isr_addentry 165
	isr_addentry 166
	isr_addentry 167
	isr_addentry 168
	isr_addentry 169
	isr_addentry 170
	isr_addentry 171
	isr_addentry 172
	isr_addentry 173
	isr_addentry 174
	isr_addentry 175
	isr_addentry 176
	isr_addentry 177
	isr_addentry 178
	isr_addentry 179
	isr_addentry 180
	isr_addentry 181
	isr_addentry 182
	isr_addentry 183
	isr_addentry 184
	isr_addentry 185
	isr_addentry 186
	isr_addentry 187
	isr_addentry 188
	isr_addentry 189
	isr_addentry 190
	isr_addentry 191
	isr_addentry 192
	isr_addentry 193
	isr_addentry 194
	isr_addentry 195
	isr_addentry 196
	isr_addentry 197
	isr_addentry 198
	isr_addentry 199
	isr_addentry 200
	isr_addentry 201
	isr_addentry 202
	isr_addentry 203
	isr_addentry 204
	isr_addentry 205
	isr_addentry 206
	isr_addentry 207
	isr_addentry 208
	isr_addentry 209
	isr_addentry 210
	isr_addentry 211
	isr_addentry 212
	isr_addentry 213
	isr_addentry 214
	isr_addentry 215
	isr_addentry 216
	isr_addentry 217
	isr_addentry 218
	isr_addentry 219
	isr_addentry 220
	isr_addentry 221
	isr_addentry 222
	isr_addentry 223
	isr_addentry 224
	isr_addentry 225
	isr_addentry 226
	isr_addentry 227
	isr_addentry 228
	isr_addentry 229
	isr_addentry 230
	isr_addentry 231
	isr_addentry 232
	isr_addentry 233
	isr_addentry 234
	isr_addentry 235
	isr_addentry 236
	isr_addentry 237
	isr_addentry 238
	isr_addentry 239
	isr_addentry 240
	isr_addentry 241
	isr_addentry 242
	isr_addentry 243
	isr_addentry 244
	isr_addentry 245
	isr_addentry 246
	isr_addentry 247
	isr_addentry 248
	isr_addentry 249
	isr_addentry 250
	isr_addentry 251
	isr_addentry 252
	isr_addentry 253
	isr_addentry 254
	isr_addentry 255

	jmp .isr_post_install

%macro isr_call_fn 1
	push rax
	push rcx
	push rdx
	push rsi
	push rdi
	push r8
	push r9
	push r10
	push r11
	lea rdi, [rsp + 72] ; Pointer to interrupt frame
	extern %1
	call %1
	pop r11
	pop r10
	pop r9
	pop r8
	pop rdi
	pop rsi
	pop rdx
	pop rcx
	pop rax
	add rsp, 16 ; rsp points to rip before iretq
	iretq
%endmacro

%macro isr_fn 2
.isr_%1:
	nop
	push qword 0
	push qword %1
	isr_call_fn %2
%endmacro
%macro isr_err 1
.isr_%1:
	nop
	push qword %1
	isr_call_fn exception_handler
%endmacro
%macro isr_noerr 1
	isr_fn %1, exception_handler
%endmacro
%macro isr_noop 1
.isr_%1:
	nop
	iretq	
%endmacro

interrupt_service_routines:
	isr_noerr 0
	isr_noerr 1
	isr_noerr 2
	isr_noerr 3
	isr_noerr 4
	isr_noerr 5
	isr_noerr 6
	isr_noerr 7
	isr_err 8
	isr_noerr 9
	isr_err 10
	isr_err 11
	isr_err 12
	isr_err 13
	isr_err 14
	isr_noerr 15
	isr_noerr 16
	isr_err 17
	isr_noerr 18
	isr_noerr 19
	isr_noerr 20
	isr_noerr 21
	isr_noerr 22
	isr_noerr 23
	isr_noerr 24
	isr_noerr 25
	isr_noerr 26
	isr_noerr 27
	isr_noerr 28
	isr_noerr 29
	isr_err 30
	isr_noerr 31

	; IRQs 0-15
	isr_fn 32, irq_handler
	isr_fn 33, irq_handler
	isr_fn 34, irq_handler
	isr_fn 35, irq_handler
	isr_fn 36, irq_handler
	isr_fn 37, irq_handler
	isr_fn 38, irq_handler
	isr_fn 39, irq_handler
	isr_fn 40, irq_handler
	isr_fn 41, irq_handler
	isr_fn 42, irq_handler
	isr_fn 43, irq_handler
	isr_fn 44, irq_handler
	isr_fn 45, irq_handler
	isr_fn 46, irq_handler
	isr_fn 47, irq_handler
	
	; Extra stuff
	isr_noop 48
	isr_noop 49
	isr_noop 50
	isr_noop 51
	isr_noop 52
	isr_noop 53
	isr_noop 54
	isr_noop 55
	isr_noop 56
	isr_noop 57
	isr_noop 58
	isr_noop 59
	isr_noop 60
	isr_noop 61
	isr_noop 62
	isr_noop 63
	isr_noop 64
	isr_noop 65
	isr_noop 66
	isr_noop 67
	isr_noop 68
	isr_noop 69
	isr_noop 70
	isr_noop 71
	isr_noop 72
	isr_noop 73
	isr_noop 74
	isr_noop 75
	isr_noop 76
	isr_noop 77
	isr_noop 78
	isr_noop 79
	isr_noop 80
	isr_noop 81
	isr_noop 82
	isr_noop 83
	isr_noop 84
	isr_noop 85
	isr_noop 86
	isr_noop 87
	isr_noop 88
	isr_noop 89
	isr_noop 90
	isr_noop 91
	isr_noop 92
	isr_noop 93
	isr_noop 94
	isr_noop 95
	isr_noop 96
	isr_noop 97
	isr_noop 98
	isr_noop 99
	isr_noop 100
	isr_noop 101
	isr_noop 102
	isr_noop 103
	isr_noop 104
	isr_noop 105
	isr_noop 106
	isr_noop 107
	isr_noop 108
	isr_noop 109
	isr_noop 110
	isr_noop 111
	isr_noop 112
	isr_noop 113
	isr_noop 114
	isr_noop 115
	isr_noop 116
	isr_noop 117
	isr_noop 118
	isr_noop 119
	isr_noop 120
	isr_noop 121
	isr_noop 122
	isr_noop 123
	isr_noop 124
	isr_noop 125
	isr_noop 126
	isr_noop 127
	isr_noop 128
	isr_noop 129
	isr_noop 130
	isr_noop 131
	isr_noop 132
	isr_noop 133
	isr_noop 134
	isr_noop 135
	isr_noop 136
	isr_noop 137
	isr_noop 138
	isr_noop 139
	isr_noop 140
	isr_noop 141
	isr_noop 142
	isr_noop 143
	isr_noop 144
	isr_noop 145
	isr_noop 146
	isr_noop 147
	isr_noop 148
	isr_noop 149
	isr_noop 150
	isr_noop 151
	isr_noop 152
	isr_noop 153
	isr_noop 154
	isr_noop 155
	isr_noop 156
	isr_noop 157
	isr_noop 158
	isr_noop 159
	isr_noop 160
	isr_noop 161
	isr_noop 162
	isr_noop 163
	isr_noop 164
	isr_noop 165
	isr_noop 166
	isr_noop 167
	isr_noop 168
	isr_noop 169
	isr_noop 170
	isr_noop 171
	isr_noop 172
	isr_noop 173
	isr_noop 174
	isr_noop 175
	isr_noop 176
	isr_noop 177
	isr_noop 178
	isr_noop 179
	isr_noop 180
	isr_noop 181
	isr_noop 182
	isr_noop 183
	isr_noop 184
	isr_noop 185
	isr_noop 186
	isr_noop 187
	isr_noop 188
	isr_noop 189
	isr_noop 190
	isr_noop 191
	isr_noop 192
	isr_noop 193
	isr_noop 194
	isr_noop 195
	isr_noop 196
	isr_noop 197
	isr_noop 198
	isr_noop 199
	isr_noop 200
	isr_noop 201
	isr_noop 202
	isr_noop 203
	isr_noop 204
	isr_noop 205
	isr_noop 206
	isr_noop 207
	isr_noop 208
	isr_noop 209
	isr_noop 210
	isr_noop 211
	isr_noop 212
	isr_noop 213
	isr_noop 214
	isr_noop 215
	isr_noop 216
	isr_noop 217
	isr_noop 218
	isr_noop 219
	isr_noop 220
	isr_noop 221
	isr_noop 222
	isr_noop 223
	isr_noop 224
	isr_noop 225
	isr_noop 226
	isr_noop 227
	isr_noop 228
	isr_noop 229
	isr_noop 230
	isr_noop 231
	isr_noop 232
	isr_noop 233
	isr_noop 234
	isr_noop 235
	isr_noop 236
	isr_noop 237
	isr_noop 238
	isr_noop 239
	isr_noop 240
	isr_noop 241
	isr_noop 242
	isr_noop 243
	isr_noop 244
	isr_noop 245
	isr_noop 246
	isr_noop 247
	isr_noop 248
	isr_noop 249
	isr_noop 250
	isr_noop 251
	isr_noop 252
	isr_noop 253
	isr_noop 254
	isr_noop 255
