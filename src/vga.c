/* vga.c - VGA text mode (80×25) driver */

#include "vga.h"
#include "string.h"

/* I/O port helpers */
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}
static inline uint8_t inb(uint16_t port) {
    uint8_t val;
    __asm__ volatile("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

/* VGA hardware registers */
#define VGA_CTRL_PORT 0x3D4
#define VGA_DATA_PORT 0x3D5

/* Video memory base */
static volatile uint16_t * const VGA_MEM = (volatile uint16_t *)0xB8000;

/* Internal state */
static int     vga_col;
static int     vga_row;
static uint8_t vga_color;

void vga_init(void) {
    vga_col   = 0;
    vga_row   = 0;
    vga_color = vga_make_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_enable_cursor();
    vga_clear(vga_color);
}

void vga_clear(uint8_t color) {
    for (int y = 0; y < VGA_HEIGHT; y++)
        for (int x = 0; x < VGA_WIDTH; x++)
            VGA_MEM[y * VGA_WIDTH + x] = vga_make_entry(' ', color);
    vga_col   = 0;
    vga_row   = 0;
    vga_set_cursor(0, 0);
}

void vga_set_color(uint8_t color) { vga_color = color; }
uint8_t vga_get_color(void)       { return vga_color; }

void vga_put_char_at(char c, uint8_t color, int x, int y) {
    if (x < 0 || x >= VGA_WIDTH || y < 0 || y >= VGA_HEIGHT) return;
    VGA_MEM[y * VGA_WIDTH + x] = vga_make_entry(c, color);
}

void vga_fill_row(int row, char c, uint8_t color) {
    if (row < 0 || row >= VGA_HEIGHT) return;
    for (int x = 0; x < VGA_WIDTH; x++)
        VGA_MEM[row * VGA_WIDTH + x] = vga_make_entry(c, color);
}

void vga_fill_rect(int x, int y, int w, int h, char c, uint8_t color) {
    for (int row = y; row < y + h && row < VGA_HEIGHT; row++)
        for (int col = x; col < x + w && col < VGA_WIDTH; col++)
            vga_put_char_at(c, color, col, row);
}

/*
 * Draw a box using CP437 box-drawing characters.
 * Top-left corner: (x, y), dimensions: w × h (border inclusive).
 */
void vga_draw_box(int x, int y, int w, int h, uint8_t color) {
    if (w < 2 || h < 2) return;

    /* Corners */
    vga_put_char_at('\xDA', color, x,         y);           /* ┌ */
    vga_put_char_at('\xBF', color, x + w - 1, y);           /* ┐ */
    vga_put_char_at('\xC0', color, x,         y + h - 1);   /* └ */
    vga_put_char_at('\xD9', color, x + w - 1, y + h - 1);   /* ┘ */

    /* Horizontal edges */
    for (int i = x + 1; i < x + w - 1; i++) {
        vga_put_char_at('\xC4', color, i, y);           /* ─ */
        vga_put_char_at('\xC4', color, i, y + h - 1);  /* ─ */
    }

    /* Vertical edges */
    for (int j = y + 1; j < y + h - 1; j++) {
        vga_put_char_at('\xB3', color, x,         j);  /* │ */
        vga_put_char_at('\xB3', color, x + w - 1, j);  /* │ */
    }
}

void vga_scroll(void) {
    /* Move all rows up by one */
    for (int y = 0; y < VGA_HEIGHT - 1; y++)
        for (int x = 0; x < VGA_WIDTH; x++)
            VGA_MEM[y * VGA_WIDTH + x] = VGA_MEM[(y + 1) * VGA_WIDTH + x];
    /* Clear last row */
    for (int x = 0; x < VGA_WIDTH; x++)
        VGA_MEM[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_make_entry(' ', vga_color);
}

void vga_newline(void) {
    vga_col = 0;
    if (++vga_row >= VGA_HEIGHT) {
        vga_row = VGA_HEIGHT - 1;
        vga_scroll();
    }
    vga_set_cursor(vga_col, vga_row);
}

void vga_put_char(char c) {
    if (c == '\n') {
        vga_newline();
        return;
    }
    if (c == '\r') {
        vga_col = 0;
        vga_set_cursor(vga_col, vga_row);
        return;
    }
    if (c == '\t') {
        /* Tab stop every 4 columns */
        vga_col = (vga_col + 4) & ~3;
        if (vga_col >= VGA_WIDTH) vga_newline();
        else vga_set_cursor(vga_col, vga_row);
        return;
    }
    if (c == '\b') {
        if (vga_col > 0) {
            vga_col--;
            vga_put_char_at(' ', vga_color, vga_col, vga_row);
            vga_set_cursor(vga_col, vga_row);
        }
        return;
    }

    vga_put_char_at(c, vga_color, vga_col, vga_row);
    if (++vga_col >= VGA_WIDTH) vga_newline();
    else vga_set_cursor(vga_col, vga_row);
}

void vga_write(const char *str) {
    while (*str) vga_put_char(*str++);
}

void vga_write_color(const char *str, uint8_t color) {
    uint8_t saved = vga_color;
    vga_color = color;
    vga_write(str);
    vga_color = saved;
}

void vga_write_at(const char *str, uint8_t color, int x, int y) {
    int col = x;
    while (*str && col < VGA_WIDTH) {
        vga_put_char_at(*str++, color, col++, y);
    }
}

/* ─── Hardware cursor ─────────────────────────────────────────────────────── */

void vga_enable_cursor(void) {
    outb(VGA_CTRL_PORT, 0x0A);
    outb(VGA_DATA_PORT, (inb(VGA_DATA_PORT) & 0xC0) | 13);
    outb(VGA_CTRL_PORT, 0x0B);
    outb(VGA_DATA_PORT, (inb(VGA_DATA_PORT) & 0xE0) | 15);
}

void vga_disable_cursor(void) {
    outb(VGA_CTRL_PORT, 0x0A);
    outb(VGA_DATA_PORT, 0x20);
}

void vga_set_cursor(int x, int y) {
    uint16_t pos = (uint16_t)(y * VGA_WIDTH + x);
    outb(VGA_CTRL_PORT, 0x0F);
    outb(VGA_DATA_PORT, (uint8_t)(pos & 0xFF));
    outb(VGA_CTRL_PORT, 0x0E);
    outb(VGA_DATA_PORT, (uint8_t)((pos >> 8) & 0xFF));
    vga_col = x;
    vga_row = y;
}

void vga_get_cursor(int *x, int *y) {
    if (x) *x = vga_col;
    if (y) *y = vga_row;
}

/* ─── Number output helpers ──────────────────────────────────────────────── */

void vga_write_dec(uint32_t n) {
    char buf[12];
    uint_to_str(n, buf, 10);
    vga_write(buf);
}

void vga_write_hex(uint32_t n) {
    char buf[10];
    vga_write("0x");
    uint_to_str(n, buf, 16);
    vga_write(buf);
}
