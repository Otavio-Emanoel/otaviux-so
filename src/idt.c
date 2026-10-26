/* idt.c - Interrupt Descriptor Table setup */

#include "idt.h"
#include "string.h"

#define IDT_ENTRIES 256

static idt_entry_t idt[IDT_ENTRIES];
static idt_ptr_t   idt_ptr;

/* Defined in idt_asm.asm */
extern void idt_load(idt_ptr_t *ptr);

void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags) {
    idt[num].base_low  = (uint16_t)(base & 0xFFFF);
    idt[num].base_high = (uint16_t)((base >> 16) & 0xFFFF);
    idt[num].selector  = selector;
    idt[num].zero      = 0;
    idt[num].flags     = flags;
}

void idt_init(void) {
    memset(idt, 0, sizeof(idt));
    idt_ptr.limit = (uint16_t)(sizeof(idt) - 1);
    idt_ptr.base  = (uint32_t)idt;
    idt_load(&idt_ptr);
}
