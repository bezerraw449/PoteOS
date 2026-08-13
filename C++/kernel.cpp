#include "teclado.h"
#include "idt.h"

struct Registers {
    unsigned int ds;                                     
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax; 
    unsigned int int_no, err_code;                       
    unsigned int eip, cs, eflags, useresp, ss;           
};

static inline unsigned char inb(unsigned short port) {
    unsigned char ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(unsigned short port, unsigned char val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static void update_hardware_cursor(int pos) {
    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char)((pos >> 8) & 0xFF));
}

static void enable_cursor(unsigned char cursor_start, unsigned char cursor_end) {
    outb(0x3D4, 0x0A);
    outb(0x3D5, (inb(0x3D5) & 0xC0) | cursor_start);
    outb(0x3D4, 0x0B);
    outb(0x3D5, (inb(0x3D5) & 0xE0) | cursor_end);
}

static int cursor_pos = 1 * 80;
static const unsigned short cor_texto = 0x1F00; 

void kernel_putc(char c) {
    volatile unsigned short* vga_buffer = (volatile unsigned short*) 0xB8000;

    if (c == '\b') { 
        if (cursor_pos > 80) {
            cursor_pos--;
            vga_buffer[cursor_pos] = cor_texto | ' ';
        }
    } else if (c == '\n') { 
        int linha_atual = cursor_pos / 80;
        cursor_pos = (linha_atual + 1) * 80;
    } else if (c >= 32 || (unsigned char)c >= 128) { 
        vga_buffer[cursor_pos] = cor_texto | (unsigned char)c;
        cursor_pos++;
    }

    if (cursor_pos >= 80 * 25) {
        cursor_pos = 80;
    }

    update_hardware_cursor(cursor_pos);
}

extern "C" void isr_handler(Registers* r) {
    if (r->int_no == 33 || (inb(0x64) & 0x01)) {
        unsigned char scancode = inb(0x60);
        char c = teclado_processar_scancode(scancode);
        if (c != 0) {
            kernel_putc(c);
        }
    }

    if (r->int_no >= 32) {
        outb(0x20, 0x20);
        if (r->int_no >= 40) {
            outb(0xA0, 0x20); 
        }
    }
}

extern "C" void kernel_main() {
    volatile unsigned short* vga_buffer = (volatile unsigned short*) 0xB8000;
    unsigned short cor_azul = 0x1F00;

    for (int i = 0; i < 80 * 25; i++) {
        vga_buffer[i] = cor_azul | ' ';
    }

    const char* mensagem = "PoteOS version 0.1";
    for (int i = 0; mensagem[i] != '\0'; i++) {
        vga_buffer[i] = cor_azul | mensagem[i];
    }

    enable_cursor(0, 15);

    cursor_pos = 80;
    update_hardware_cursor(cursor_pos);

    idt_init();

    teclado_inicializar();

    while (1) {
        asm volatile("hlt");
    }
}
