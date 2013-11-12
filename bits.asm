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
global testq
global disable_pic
global enable_ioapic_maybe
global enable_int
global disable_int
global readthemsrthing
global disable_local_apic
global io_wait

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

