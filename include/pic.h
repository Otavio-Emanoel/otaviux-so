#ifndef PIC_H
#define PIC_H

#include <stdint.h>

/* PIC ports */
#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

/* End-of-interrupt command */
#define PIC_EOI      0x20

/* IRQ base vectors after remapping */
#define PIC1_OFFSET  0x20  /* IRQ 0-7  -> IDT entries 32-39 */
#define PIC2_OFFSET  0x28  /* IRQ 8-15 -> IDT entries 40-47 */

void pic_init(void);
void pic_send_eoi(uint8_t irq);
void pic_mask_irq(uint8_t irq);
void pic_unmask_irq(uint8_t irq);

static inline void io_wait(void) {
    __asm__ volatile("outb %%al, $0x80" : : "a"(0));
}

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

#endif /* PIC_H */
