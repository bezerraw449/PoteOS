#include "teclado.h"

static inline unsigned char inb(unsigned short port) {
    unsigned char ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(unsigned short port, unsigned char val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static bool shift_pressionado = false;
static bool caps_lock_ativo = false;
static bool altgr_pressionado = false;

static const unsigned char teclado_abnt2_normal[128] = {
    0,   27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '`', '[', '\n',
     0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 0x87, '~', '\'',
     0, ']', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', ';',   0,  '*',
     0, ' ',   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0, '-',   0,   0,   0,
   '+',   0,   0,   0,   0,   0,   0,   0, '\\',  0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0, '/',   0,   0,   0,   0,   0
};

static const unsigned char teclado_abnt2_shift[128] = {
    0,   27, '!', '@', '#', '$', '%', 0x9B, '&', '*', '(', ')', '_', '+', '\b',
  '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '^', '{', '\n',
     0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 0x80, '^', '"',
     0, '}', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', ':',   0,  '*',
     0, ' ',   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0, '-',   0,   0,   0,
   '+',   0,   0,   0,   0,   0,   0,   0, '|',   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0, '?',   0,   0,   0,   0,   0
};

static const unsigned char teclado_abnt2_altgr[128] = {
    0,    0, 0xFD, 0xFD, 0xFD, 0x9C, 0xBD, 0xAA,   0,   0,   0,   0,   0, 0x15,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 0xA6,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0, 0xA7,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0, 0xF8,    0,    0,    0,    0,    0
};

void teclado_inicializar() {
    shift_pressionado = false;
    caps_lock_ativo = false;
    altgr_pressionado = false;
}

char teclado_processar_scancode(unsigned char scancode) {
    if (scancode & 0x80) {
        unsigned char codigo_solto = scancode & 0x7F;
        if (codigo_solto == 0x2A || codigo_solto == 0x36) { 
            shift_pressionado = false;
        } else if (codigo_solto == 0x38) { 
            altgr_pressionado = false;
        }
        return 0;
    }

    switch (scancode) {
        case 0x2A: 
        case 0x36: 
            shift_pressionado = true;
            return 0;

        case 0x3A: 
            caps_lock_ativo = !caps_lock_ativo;
            return 0;

        case 0x38: 
            altgr_pressionado = true;
            return 0;

        default:
            break;
    }

    if (scancode >= 128) return 0;

    if (altgr_pressionado && teclado_abnt2_altgr[scancode] != 0) {
        return teclado_abnt2_altgr[scancode];
    }

    char c_normal = teclado_abnt2_normal[scancode];
    char c_shift  = teclado_abnt2_shift[scancode];

    if (c_normal == 0) return 0;

    bool e_letra = (c_normal >= 'a' && c_normal <= 'z') || (c_normal == (char)0x87);

    if (e_letra) {
        bool maiuscula = shift_pressionado ^ caps_lock_ativo;
        if (c_normal == (char)0x87) { 
            return maiuscula ? (char)0x80 : (char)0x87; 
        }
        return maiuscula ? c_shift : c_normal;
    } else {
        return shift_pressionado ? c_shift : c_normal;
    }
}

char teclado_ler_caractere() {
    if (inb(0x64) & 0x01) {
        unsigned char scancode = inb(0x60);
        return teclado_processar_scancode(scancode);
    }
    return 0;
}
