[BITS 16]

lgdt [gdt_desc]
mov eax, cr0
or al, 1
mov cr0, eax
jmp dword 0x08:_start

[BITS 32]

GLOBAL _start
EXTERN kernel_main

_start:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    cld

    mov dx, 0x3D4
    mov al, 0x0A
    out dx, al
    mov dx, 0x3D5
    mov al, 0x20
    out dx, al

    call kernel_main

trava:
    cli                
    hlt               
    jmp trava          

gdt:
    dq 0
    dq 0x00CF9A000000FFFF
    dq 0x00CF92000000FFFF

gdt_desc:
    dw $ - gdt - 1
    dd gdt
