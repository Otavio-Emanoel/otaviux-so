/* gui.c - Graphical text-mode UI components */

#include "gui.h"
#include "vga.h"
#include "string.h"

/* ─── Internal helpers ───────────────────────────────────────────────────── */

/* Centre a string in a field of `width` characters, padding with spaces */
static void write_centered(const char *str, int width, int y, uint8_t color) {
    int len  = (int)strlen(str);
    int pad  = (width - len) / 2;
    int col  = pad < 0 ? 0 : pad;

    /* Fill the entire row first */
    for (int x = 0; x < width; x++)
        vga_put_char_at(' ', color, x, y);

    /* Write the text */
    for (int i = 0; i < len && col + i < width; i++)
        vga_put_char_at(str[i], color, col + i, y);
}

/* ─── Title bar ──────────────────────────────────────────────────────────── */

void gui_draw_titlebar(const char *os_name, const char *version) {
    /* Fill the whole row */
    for (int x = 0; x < VGA_WIDTH; x++)
        vga_put_char_at(' ', GUI_COLOR_TITLEBAR, x, GUI_TITLEBAR_ROW);

    /* OS name on the left */
    int col = 1;
    for (int i = 0; os_name[i] && col < VGA_WIDTH - 1; i++, col++)
        vga_put_char_at(os_name[i], GUI_COLOR_TITLEBAR, col, GUI_TITLEBAR_ROW);

    /* Version on the right */
    int vlen = (int)strlen(version);
    int vcol = VGA_WIDTH - vlen - 2;
    if (vcol > 0)
        for (int i = 0; i < vlen; i++)
            vga_put_char_at(version[i], GUI_COLOR_TITLEBAR, vcol + i, GUI_TITLEBAR_ROW);

    /* Decorative separator dots */
    vga_put_char_at('\xF9', GUI_COLOR_TITLEBAR, VGA_WIDTH / 2, GUI_TITLEBAR_ROW);
}

/* ─── Taskbar ────────────────────────────────────────────────────────────── */

void gui_draw_taskbar(const char *hint) {
    for (int x = 0; x < VGA_WIDTH; x++)
        vga_put_char_at(' ', GUI_COLOR_TASKBAR, x, GUI_TASKBAR_ROW);

    /* Help hint on the left */
    int col = 1;
    for (int i = 0; hint[i] && col < VGA_WIDTH - 1; i++, col++)
        vga_put_char_at(hint[i], GUI_COLOR_TASKBAR, col, GUI_TASKBAR_ROW);

    /* Right-aligned label */
    const char *label = "  otaviux-so  ";
    int llen = (int)strlen(label);
    int lcol = VGA_WIDTH - llen - 1;
    if (lcol > 0)
        for (int i = 0; i < llen; i++)
            vga_put_char_at(label[i], GUI_COLOR_TASKBAR, lcol + i, GUI_TASKBAR_ROW);
}

/* ─── Desktop background ─────────────────────────────────────────────────── */

void gui_draw_desktop(void) {
    for (int y = GUI_DESKTOP_TOP; y <= GUI_DESKTOP_BOTTOM; y++)
        for (int x = 0; x < VGA_WIDTH; x++)
            vga_put_char_at('\xB0', GUI_COLOR_DESKTOP, x, y);
}

/* ─── Generic window ─────────────────────────────────────────────────────── */

void gui_draw_window(const gui_window_t *win) {
    if (!win) return;

    /* Fill interior */
    vga_fill_rect(win->x + 1, win->y + 1, win->w - 2, win->h - 2, ' ', win->body_color);

    /* Border */
    vga_draw_box(win->x, win->y, win->w, win->h, win->border_color);

    /* Title bar (row just inside the top border) */
    if (win->title) {
        /* Highlight the title area */
        for (int x = win->x + 1; x < win->x + win->w - 1; x++)
            vga_put_char_at(' ', GUI_COLOR_WIN_TITLE, x, win->y);

        /* Center title text in title area */
        int inner_w = win->w - 2;
        int tlen = (int)strlen(win->title);
        int start = win->x + 1 + (inner_w - tlen) / 2;
        if (start < win->x + 1) start = win->x + 1;
        for (int i = 0; i < tlen && start + i < win->x + win->w - 1; i++)
            vga_put_char_at(win->title[i], GUI_COLOR_WIN_TITLE, start + i, win->y);
    }
}

/* ─── Boot / splash screen ──────────────────────────────────────────────── */

void gui_draw_boot_screen(void) {
    vga_clear(vga_make_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));

    uint8_t logo_color = vga_make_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    uint8_t msg_color  = vga_make_color(VGA_COLOR_WHITE,      VGA_COLOR_BLACK);
    uint8_t ok_color   = vga_make_color(VGA_COLOR_LIGHT_GREEN,VGA_COLOR_BLACK);

    write_centered("  ___  _              _              ___  ___  ", VGA_WIDTH, 4,  logo_color);
    write_centered(" / _ \\| |_ __ ___   (_) _   ___  __/ __/ / _ \\ ", VGA_WIDTH, 5,  logo_color);
    write_centered("| | | | __/ _` \\ \\ / / | | | \\ \\/ /\\__ \\| | | |", VGA_WIDTH, 6,  logo_color);
    write_centered("| |_| | || (_| |\\ V /| | |_| |>  < ___) | |_| |", VGA_WIDTH, 7,  logo_color);
    write_centered(" \\___/ \\__\\__,_| \\_/ |_|\\__,_/_/\\_\\____/ \\___/ ", VGA_WIDTH, 8,  logo_color);

    write_centered("Sistema Operacional desenvolvido em C e Assembly", VGA_WIDTH, 10, msg_color);
    write_centered("versao 1.0.0  -  inicializando...", VGA_WIDTH, 11, msg_color);

    /* Simple loading bar */
    int bar_x = (VGA_WIDTH - 42) / 2;
    vga_put_char_at('[', ok_color, bar_x, 13);
    for (int i = 1; i <= 40; i++)
        vga_put_char_at('\xDB', ok_color, bar_x + i, 13);
    vga_put_char_at(']', ok_color, bar_x + 41, 13);

    write_centered("[ OK ] Sistema pronto!", VGA_WIDTH, 15, ok_color);
}

/* ─── Title-bar clock (called periodically) ──────────────────────────────── */

void gui_update_titlebar_time(uint32_t ticks) {
    /* Derive a simple HH:MM:SS from tick count (assuming 18.2 Hz PIT) */
    uint32_t secs    = ticks / 18;
    uint32_t minutes = (secs / 60) % 60;
    uint32_t hours   = (secs / 3600) % 24;
    uint32_t sec_d   = secs % 60;

    char buf[10];
    /* HH */
    buf[0] = '0' + (char)(hours / 10);
    buf[1] = '0' + (char)(hours % 10);
    buf[2] = ':';
    buf[3] = '0' + (char)(minutes / 10);
    buf[4] = '0' + (char)(minutes % 10);
    buf[5] = ':';
    buf[6] = '0' + (char)(sec_d / 10);
    buf[7] = '0' + (char)(sec_d % 10);
    buf[8] = '\0';

    /* Write at a fixed position in the title bar */
    int col = (VGA_WIDTH - 8) / 2;
    for (int i = 0; i < 8; i++)
        vga_put_char_at(buf[i], GUI_COLOR_TITLEBAR, col + i, GUI_TITLEBAR_ROW);
}
