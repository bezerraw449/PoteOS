#include "idt.h"

extern "C" uint32_t isr_stub_table[256];

static struct idt_entry idt_entries[256];
static struct idt_ptr   idtp;

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static void pic_remap() {
    outb(0x20, 0x11); 
    outb(0xA0, 0x11); 
    
    outb(0x21, 0x20); 
    outb(0xA1, 0x28); 
    
    outb(0x21, 0x04); 
    outb(0xA1, 0x02); 
    
    outb(0x21, 0x01); 
    outb(0xA1, 0x01); 
    
    outb(0x21, 0x00); 
    outb(0xA1, 0x00); 
}

static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt_entries[num].base_low  = base & 0xFFFF;
    idt_entries[num].base_high = (base >> 16) & 0xFFFF;
    idt_entries[num].sel       = sel;
    idt_entries[num].always0   = 0;
    idt_entries[num].flags     = flags;
}

void idt_init() {
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base  = (uint32_t)&idt_entries;

    pic_remap();

    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, isr_stub_table[i], 0x08, 0x8E);
    }

    asm volatile("lidt %0" : : "m"(idtp));

    asm volatile("sti");
}
