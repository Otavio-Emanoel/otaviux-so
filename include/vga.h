#ifndef VGA_H
#define VGA_H

#include <stdint.h>
#include <stddef.h>

/* VGA text mode dimensions */
#define VGA_WIDTH  80
#define VGA_HEIGHT 25

/* VGA color codes */
typedef enum {
    VGA_COLOR_BLACK         = 0,
    VGA_COLOR_BLUE          = 1,
    VGA_COLOR_GREEN         = 2,
    VGA_COLOR_CYAN          = 3,
    VGA_COLOR_RED           = 4,
    VGA_COLOR_MAGENTA       = 5,
    VGA_COLOR_BROWN         = 6,
    VGA_COLOR_LIGHT_GREY    = 7,
    VGA_COLOR_DARK_GREY     = 8,
    VGA_COLOR_LIGHT_BLUE    = 9,
    VGA_COLOR_LIGHT_GREEN   = 10,
    VGA_COLOR_LIGHT_CYAN    = 11,
    VGA_COLOR_LIGHT_RED     = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN   = 14,
    VGA_COLOR_WHITE         = 15,
} vga_color_t;

/* Pack foreground and background into a single color byte */
static inline uint8_t vga_make_color(vga_color_t fg, vga_color_t bg) {
    return (uint8_t)(fg | (bg << 4));
}

/* Pack character + color into a VGA entry */
static inline uint16_t vga_make_entry(char c, uint8_t color) {
    return (uint16_t)c | ((uint16_t)color << 8);
}

void vga_init(void);
void vga_clear(uint8_t color);
void vga_set_color(uint8_t color);
uint8_t vga_get_color(void);
void vga_put_char_at(char c, uint8_t color, int x, int y);
void vga_put_char(char c);
void vga_write(const char *str);
void vga_write_color(const char *str, uint8_t color);
void vga_write_at(const char *str, uint8_t color, int x, int y);
void vga_fill_row(int row, char c, uint8_t color);
void vga_fill_rect(int x, int y, int w, int h, char c, uint8_t color);
void vga_draw_box(int x, int y, int w, int h, uint8_t color);
void vga_set_cursor(int x, int y);
void vga_get_cursor(int *x, int *y);
void vga_enable_cursor(void);
void vga_disable_cursor(void);
void vga_scroll(void);
void vga_newline(void);
void vga_write_dec(uint32_t n);
void vga_write_hex(uint32_t n);

#endif /* VGA_H */
