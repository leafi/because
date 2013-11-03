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

