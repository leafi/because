[bits 64]

global outb
global outw
global outd
global inb
global inw
global ind
global simple
global first
global snd
global intq
global irq1_ps2_port1
global irq1_ps2_port2
global testq
global disable_pic
global enable_ioapic_maybe
global enable_int
global disable_int
global readthemsrthing
global disable_local_apic
global io_wait

extern call_lua_int

extern wait_for_out_data

global a_irq4_handler
extern irq4_handler

simple:
  mov rax, 0x87654321
  ret

first:
  mov rax, rdi
  ret

snd:
  mov rax, rsi
  ret

outb:
  push dx
  mov dx, di
  mov ax, si
  out dx, al
  pop dx
  ret

outd:
  push dx
  mov dx, di
  mov eax, esi
  out dx, eax
  pop dx
  ret

outw:
	push dx
	mov dx, di
	mov ax, si
	out dx, ax
	pop dx
	ret

inb:
  push dx
  mov dx, di
  in al, dx
  pop dx
  ret

ind:
  push dx
  mov dx, di
  in eax, dx
  pop dx
  ret

inw:
  push dx
  mov dx, di
  in ax, dx
  pop dx
  ret

intq:
	push rax
	mov al, [0xb8000]
	inc al
	mov [0xb8000], al
	;mov [0x200000], al
	; write to EOI
	mov rax, 0xfee000b0
	mov [rax], rax 
	pop rax
	iretq

irq1_ps2_port1:
	push rax
	mov al, [0xb8000]
	inc al
	mov [0xb8000], al

	in al, 0x60

	;write eoi
	mov al, 0x20
	out 0x20, al

	; write eoi to apic???
	;mov rax, 0xfee000b0
	;mov [rax], rax

	pop rax
	iretq

%macro pushforccall 0
	push rax
	;push rbx ; PRESERVED
	push rcx
	push rdx
	;push rbp ; PRESERVED
	push rdi
	push rsi
	push r8
	push r9
	push r10
	push r11
	;push r12
	;push r13
	;push r14
	;push r15
%endmacro

%macro popforccall 0
	;pop r15
	;pop r14
	;pop r13
	;pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rsi
	pop rdi
	;pop rbp
	pop rdx
	pop rcx
	;pop rbx
	pop rax
%endmacro

%macro lirq 1
	pushforccall
	mov rdi, %1
	call call_lua_int
	popforccall
	iretq
%endmacro

%macro dlirq 1
	global irq%1
	irq%1: lirq 0x20 + %1
%endmacro

dlirq 0
dlirq 1
dlirq 2
dlirq 3
dlirq 4
dlirq 5
dlirq 6
dlirq 7
dlirq 8
dlirq 9
dlirq 10
dlirq 11
dlirq 12
dlirq 13
dlirq 14
dlirq 15

irq1_ps2_port2:
	push rax
	mov al, [0xb8000]
	inc al
	mov [0xb8000], al

	in al, 0x60

	; write slave eoi
	mov al, 0x20
	out 0xa0, al

	; write master eoi
	mov al, 0x20
	out 0x20, al

	pop rax
	iretq

a_irq4_handler:
	push rax
	mov al, [0xb8002]
	inc al
	mov [0xb8002], al

	call irq4_handler

	; eoi
	mov al, 0x20
	out 0x20, al

	; shouldnt need this (slave eoi)
	out 0xa0, al

	; ??? shouldnt need this
	mov rax, 0xfee000b0
	mov [rax], rax

	pop rax
	iretq

testq:
	;int 80h
	int 0x24
	;int 0
	ret

disable_pic:
	mov al, 0xff
	out 0xa1, al
	out 0x21, al
	ret

enable_ioapic_maybe:
	mov al, 0x70
	out 0x22, al
	mov al, 0x01
	out 0x23, al
	ret

disable_int:
	cli
	ret

enable_int:
	sti
	ret

readthemsrthing:
	mov ecx, 0x1b
	rdmsr
	mov [0x200000], edx
	mov [0x200004], eax
	ret

disable_local_apic:
	mov ecx, 0x1b
	rdmsr
	and eax, 0xfffff7ff
	wrmsr
	ret

io_wait:
	; apparently this is okay
	push rax
	mov al, 0
	out 0x80, al
	pop rax
	ret

