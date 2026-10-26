/* kernel.c - otaviux kernel entry point */

#include <stdint.h>
#include <stddef.h>

#include "vga.h"
#include "string.h"
#include "idt.h"
#include "pic.h"
#include "keyboard.h"
#include "gui.h"
#include "shell.h"

/* Simple busy-wait delay using a loop counter */
static void delay(volatile uint32_t count) {
    while (count--) {
        __asm__ volatile("nop");
    }
}

void kernel_main(void) {
    /* ── 1. Initialize VGA driver ──────────────────────────────────────── */
    vga_init();

    /* ── 2. Show boot splash ──────────────────────────────────────────── */
    gui_draw_boot_screen();
    delay(8000000UL);   /* brief pause so the user can see the splash    */

    /* ── 3. Set up IDT (must be before PIC / keyboard) ───────────────── */
    idt_init();

    /* ── 4. Initialize and remap the 8259A PIC ────────────────────────── */
    pic_init();

    /* Mask all IRQs initially; keyboard_init will unmask IRQ1 */
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);

    /* ── 5. Initialize keyboard driver ───────────────────────────────── */
    keyboard_init();

    /* ── 6. Enable hardware interrupts ───────────────────────────────── */
    __asm__ volatile("sti");

    /* ── 7. Draw the main GUI chrome ──────────────────────────────────── */
    vga_clear(vga_make_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    gui_draw_titlebar("  otaviux-so", "v1.0.0  ");
    gui_draw_taskbar(" [Tab] Alternar janelas  [Ctrl+C] Interromper  [help] Ajuda");
    gui_draw_desktop();

    /* ── 8. Initialize and run the interactive shell ──────────────────── */
    shell_init();
    shell_run();   /* Never returns */
}
