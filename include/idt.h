#ifndef IDT_H
#define IDT_H

#include <stdint.h>

/* IDT entry structure (8 bytes) */
typedef struct __attribute__((packed)) {
    uint16_t base_low;    /* Lower 16 bits of handler address */
    uint16_t selector;    /* Kernel code segment selector */
    uint8_t  zero;        /* Must be zero */
    uint8_t  flags;       /* Type and attributes */
    uint16_t base_high;   /* Upper 16 bits of handler address */
} idt_entry_t;

/* IDT pointer structure */
typedef struct __attribute__((packed)) {
    uint16_t limit;       /* Size of IDT - 1 */
    uint32_t base;        /* Base address of IDT */
} idt_ptr_t;

/* Interrupt handler function type */
typedef void (*isr_handler_t)(void);

void idt_init(void);
void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags);

#endif /* IDT_H */
