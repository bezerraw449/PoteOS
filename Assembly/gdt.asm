BITS 32

section .text
    global gdt_flush 
    global tss_flush 

gdt_flush:
    mov eax, [esp + 4]  
    lgdt [eax]          

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    push dword 0x08     
    push dword .flush   
    retf                

.flush:
    ret

tss_flush:
    mov ax, [esp + 4]  
    ltr ax             
    ret
