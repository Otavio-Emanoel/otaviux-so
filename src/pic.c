/* pic.c - 8259A Programmable Interrupt Controller driver */

#include "pic.h"

/* Initialization Control Words */
#define ICW1_ICW4   0x01  /* ICW4 needed        */
#define ICW1_INIT   0x10  /* Initialization     */
#define ICW4_8086   0x01  /* 8086 mode          */

void pic_init(void) {
    /* Save masks */
    uint8_t mask1 = inb(PIC1_DATA);
    uint8_t mask2 = inb(PIC2_DATA);

    /* Start initialization sequence (cascade mode) */
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4); io_wait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4); io_wait();

    /* ICW2: vector offsets */
    outb(PIC1_DATA, PIC1_OFFSET); io_wait();
    outb(PIC2_DATA, PIC2_OFFSET); io_wait();

    /* ICW3: cascade identity */
    outb(PIC1_DATA, 4); io_wait();  /* PIC1: IRQ2 connected to PIC2 */
    outb(PIC2_DATA, 2); io_wait();  /* PIC2: cascade identity = 2   */

    /* ICW4: 8086 mode */
    outb(PIC1_DATA, ICW4_8086); io_wait();
    outb(PIC2_DATA, ICW4_8086); io_wait();

    /* Restore masks (mask everything except IRQ1 = keyboard) */
    outb(PIC1_DATA, mask1);
    outb(PIC2_DATA, mask2);
}

void pic_send_eoi(uint8_t irq) {
    if (irq >= 8)
        outb(PIC2_COMMAND, PIC_EOI);
    outb(PIC1_COMMAND, PIC_EOI);
}

void pic_mask_irq(uint8_t irq) {
    uint16_t port;
    uint8_t  value;
    if (irq < 8) {
        port  = PIC1_DATA;
        value = inb(port) | (uint8_t)(1 << irq);
    } else {
        port  = PIC2_DATA;
        value = inb(port) | (uint8_t)(1 << (irq - 8));
    }
    outb(port, value);
}

void pic_unmask_irq(uint8_t irq) {
    uint16_t port;
    uint8_t  value;
    if (irq < 8) {
        port  = PIC1_DATA;
        value = inb(port) & (uint8_t)(~(1 << irq));
    } else {
        port  = PIC2_DATA;
        value = inb(port) & (uint8_t)(~(1 << (irq - 8)));
    }
    outb(port, value);
}
