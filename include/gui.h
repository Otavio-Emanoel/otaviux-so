#ifndef GUI_H
#define GUI_H

#include <stdint.h>
#include "vga.h"

/* GUI layout constants */
#define GUI_TITLEBAR_ROW  0
#define GUI_TASKBAR_ROW   (VGA_HEIGHT - 1)
#define GUI_DESKTOP_TOP   1
#define GUI_DESKTOP_BOTTOM (VGA_HEIGHT - 2)
#define GUI_DESKTOP_HEIGHT (GUI_DESKTOP_BOTTOM - GUI_DESKTOP_TOP + 1)

/* Pre-defined color schemes */
#define GUI_COLOR_TITLEBAR  vga_make_color(VGA_COLOR_WHITE,      VGA_COLOR_BLUE)
#define GUI_COLOR_TASKBAR   vga_make_color(VGA_COLOR_WHITE,      VGA_COLOR_CYAN)
#define GUI_COLOR_DESKTOP   vga_make_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_CYAN)
#define GUI_COLOR_WINDOW    vga_make_color(VGA_COLOR_BLACK,      VGA_COLOR_LIGHT_GREY)
#define GUI_COLOR_WIN_TITLE vga_make_color(VGA_COLOR_WHITE,      VGA_COLOR_BLUE)
#define GUI_COLOR_WIN_BODY  vga_make_color(VGA_COLOR_BLACK,      VGA_COLOR_LIGHT_GREY)
#define GUI_COLOR_SHELL_BG  vga_make_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK)
#define GUI_COLOR_PROMPT    vga_make_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK)
#define GUI_COLOR_INPUT     vga_make_color(VGA_COLOR_WHITE,       VGA_COLOR_BLACK)
#define GUI_COLOR_OUTPUT    vga_make_color(VGA_COLOR_LIGHT_GREY,  VGA_COLOR_BLACK)
#define GUI_COLOR_ERROR     vga_make_color(VGA_COLOR_LIGHT_RED,   VGA_COLOR_BLACK)
#define GUI_COLOR_INFO      vga_make_color(VGA_COLOR_LIGHT_CYAN,  VGA_COLOR_BLACK)
#define GUI_COLOR_SUCCESS   vga_make_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK)

/* Window descriptor */
typedef struct {
    int     x, y;       /* Top-left corner (border included) */
    int     w, h;       /* Width / height (border included)  */
    const char *title;
    uint8_t border_color;
    uint8_t body_color;
} gui_window_t;

void gui_draw_titlebar(const char *os_name, const char *version);
void gui_draw_taskbar(const char *hint);
void gui_draw_desktop(void);
void gui_draw_window(const gui_window_t *win);
void gui_update_titlebar_time(uint32_t ticks);
void gui_draw_boot_screen(void);

#endif /* GUI_H */
