/* shell.c - Interactive command-line shell for otaviux */

#include "shell.h"
#include "vga.h"
#include "keyboard.h"
#include "gui.h"
#include "string.h"

/* ─── Shell viewport ──────────────────────────────────────────────────────── */
/*
 * The shell occupies the full desktop area (rows 1-23).
 * Row 0  = title bar (drawn by gui)
 * Row 24 = task bar  (drawn by gui)
 */
#define SH_TOP     GUI_DESKTOP_TOP
#define SH_BOT     GUI_DESKTOP_BOTTOM
#define SH_HEIGHT  GUI_DESKTOP_HEIGHT
#define SH_WIDTH   VGA_WIDTH

/* Scrollback buffer: SH_HEIGHT rows × SH_WIDTH columns */
#define SB_ROWS 200
static char   sb_chars[SB_ROWS][SH_WIDTH];
static uint8_t sb_colors[SB_ROWS][SH_WIDTH];
static int sb_top    = 0;   /* oldest row index (circular)   */
static int sb_count  = 0;   /* number of rows in buffer      */
static int sb_write  = 0;   /* next row to write into        */

/* Current cursor within the write row */
static int  sh_col = 0;
static uint8_t sh_color = 0;

/* ─── Scrollback buffer helpers ───────────────────────────────────────────── */

static void sb_reset_row(int row) {
    for (int i = 0; i < SH_WIDTH; i++) {
        sb_chars[row][i]  = ' ';
        sb_colors[row][i] = GUI_COLOR_SHELL_BG;
    }
}

static void sb_new_row(void) {
    sb_reset_row(sb_write);
    sh_col = 0;
    sb_write = (sb_write + 1) % SB_ROWS;
    if (sb_count < SB_ROWS) sb_count++;
    else sb_top = (sb_top + 1) % SB_ROWS;
}

static void sb_put(char c, uint8_t color) {
    if (sh_col >= SH_WIDTH) {
        sb_new_row();
    }
    int row = (sb_write - 1 + SB_ROWS) % SB_ROWS;
    sb_chars[row][sh_col]  = c;
    sb_colors[row][sh_col] = color;
    sh_col++;
}

static void sb_render(void) {
    /* Determine which rows to display */
    int display = (sb_count < SH_HEIGHT) ? sb_count : SH_HEIGHT;
    int start_buf = (sb_write - display + SB_ROWS) % SB_ROWS;

    /* Fill rows that don't have content yet with background */
    for (int screen_row = SH_TOP; screen_row <= SH_BOT; screen_row++) {
        int buf_row_idx = screen_row - SH_TOP;
        if (buf_row_idx < SH_HEIGHT - display) {
            /* Empty rows at the top */
            for (int x = 0; x < SH_WIDTH; x++)
                vga_put_char_at(' ', GUI_COLOR_SHELL_BG, x, screen_row);
        } else {
            int buf_row = (start_buf + buf_row_idx - (SH_HEIGHT - display) + SB_ROWS) % SB_ROWS;
            for (int x = 0; x < SH_WIDTH; x++)
                vga_put_char_at(sb_chars[buf_row][x], sb_colors[buf_row][x], x, screen_row);
        }
    }

    /* Position the hardware cursor */
    int cur_screen_row = SH_TOP + (SH_HEIGHT - 1);
    if (sb_count < SH_HEIGHT) cur_screen_row = SH_TOP + sb_count - 1;
    vga_set_cursor(sh_col, cur_screen_row);
}

/* ─── Output helpers ──────────────────────────────────────────────────────── */

static void sh_write_color(const char *s, uint8_t color) {
    while (*s) {
        if (*s == '\n') {
            sb_new_row();
        } else {
            sb_put(*s, color);
        }
        s++;
    }
    sb_render();
}

static void sh_write(const char *s) {
    sh_write_color(s, GUI_COLOR_OUTPUT);
}

static void sh_writeln(const char *s) {
    sh_write(s);
    sb_new_row();
    sb_render();
}

static void sh_writeln_color(const char *s, uint8_t color) {
    sh_write_color(s, color);
    sb_new_row();
    sb_render();
}

/* ─── Built-in commands ───────────────────────────────────────────────────── */

static void cmd_help(void) {
    sh_writeln_color("Comandos disponíveis:", GUI_COLOR_INFO);
    sh_writeln_color("  help     - exibe esta mensagem", GUI_COLOR_OUTPUT);
    sh_writeln_color("  about    - informações sobre o otaviux", GUI_COLOR_OUTPUT);
    sh_writeln_color("  clear    - limpa a tela", GUI_COLOR_OUTPUT);
    sh_writeln_color("  echo     - imprime texto na tela", GUI_COLOR_OUTPUT);
    sh_writeln_color("  color    - altera a cor de saída (ex: color 0A)", GUI_COLOR_OUTPUT);
    sh_writeln_color("  mem      - exibe informações de memória", GUI_COLOR_OUTPUT);
    sh_writeln_color("  cpu      - informações sobre o processador", GUI_COLOR_OUTPUT);
    sh_writeln_color("  reboot   - reinicia o sistema", GUI_COLOR_OUTPUT);
    sh_writeln_color("  shutdown - desliga o sistema", GUI_COLOR_OUTPUT);
    sh_writeln_color("  banner   - exibe o banner do sistema", GUI_COLOR_OUTPUT);
    sh_writeln_color("  version  - versão do sistema", GUI_COLOR_OUTPUT);
}

static void cmd_about(void) {
    sh_writeln_color("=== otaviux-so ===", GUI_COLOR_INFO);
    sh_writeln_color("Sistema operacional desenvolvido em C e Assembly.", GUI_COLOR_OUTPUT);
    sh_writeln_color("Funcionalidades:", GUI_COLOR_INFO);
    sh_writeln_color("  - Boot via Multiboot (GRUB compatível)", GUI_COLOR_OUTPUT);
    sh_writeln_color("  - Modo protegido 32-bit com GDT plana", GUI_COLOR_OUTPUT);
    sh_writeln_color("  - Driver VGA texto 80x25 com cores", GUI_COLOR_OUTPUT);
    sh_writeln_color("  - IDT + PIC (interrupções de hardware)", GUI_COLOR_OUTPUT);
    sh_writeln_color("  - Driver de teclado PS/2 com mapa US QWERTY", GUI_COLOR_OUTPUT);
    sh_writeln_color("  - Interface gráfica em modo texto", GUI_COLOR_OUTPUT);
    sh_writeln_color("  - Shell interativo com comandos built-in", GUI_COLOR_OUTPUT);
    sh_writeln_color("  - Buffer de scrollback 200 linhas", GUI_COLOR_OUTPUT);
}

static void cmd_clear(void) {
    /* Reset scrollback */
    sb_top   = 0;
    sb_count = 0;
    sb_write = 0;
    sh_col   = 0;
    sb_new_row();
    /* Repaint desktop background */
    for (int y = SH_TOP; y <= SH_BOT; y++)
        for (int x = 0; x < SH_WIDTH; x++)
            vga_put_char_at(' ', GUI_COLOR_SHELL_BG, x, y);
    vga_set_cursor(0, SH_TOP);
}

static void cmd_echo(const char *args) {
    sh_writeln(args ? args : "");
}

static void cmd_color(const char *args) {
    if (!args || !args[0]) {
        sh_writeln_color("Uso: color <HEX>  (ex: color 0A = verde no preto)", GUI_COLOR_ERROR);
        return;
    }
    /* Parse two hex nibbles */
    uint8_t val = 0;
    for (int i = 0; i < 2 && args[i]; i++) {
        val <<= 4;
        char c = args[i];
        if (c >= '0' && c <= '9') val |= (uint8_t)(c - '0');
        else if (c >= 'a' && c <= 'f') val |= (uint8_t)(c - 'a' + 10);
        else if (c >= 'A' && c <= 'F') val |= (uint8_t)(c - 'A' + 10);
    }
    sh_color = val;
    sh_writeln_color("Cor alterada com sucesso.", GUI_COLOR_SUCCESS);
}

static void cmd_mem(void) {
    sh_writeln_color("=== Memória ===", GUI_COLOR_INFO);
    sh_writeln_color("  Stack do kernel : 16 KB (reservada em .bss)", GUI_COLOR_OUTPUT);
    sh_writeln_color("  VGA buffer      : 0xB8000 (4 KB, modo texto)", GUI_COLOR_OUTPUT);
    sh_writeln_color("  Kernel carregado: 1 MB (endereço base)", GUI_COLOR_OUTPUT);
    sh_writeln_color("  Scrollback shell: ~46 KB (200 linhas × 80 cols × 2 bytes)", GUI_COLOR_OUTPUT);
}

static void cmd_cpu(void) {
    char vendor[13] = {0};
    uint32_t eax, ebx, ecx, edx;
    /* CPUID leaf 0 */
    __asm__ volatile("cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(0));
    /* Vendor string is EBX + EDX + ECX */
    ((uint32_t *)vendor)[0] = ebx;
    ((uint32_t *)vendor)[1] = edx;
    ((uint32_t *)vendor)[2] = ecx;

    sh_writeln_color("=== Processador ===", GUI_COLOR_INFO);
    sh_write_color("  Fabricante: ", GUI_COLOR_OUTPUT);
    sh_writeln_color(vendor, GUI_COLOR_SUCCESS);

    /* CPUID leaf 1 - get family/model/stepping */
    __asm__ volatile("cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(1));

    uint32_t stepping = eax & 0xF;
    uint32_t model    = (eax >> 4) & 0xF;
    uint32_t family   = (eax >> 8) & 0xF;

    char buf[16];
    sh_write_color("  Família: ", GUI_COLOR_OUTPUT);
    uint_to_str(family, buf, 10);
    sh_writeln(buf);
    sh_write_color("  Modelo:  ", GUI_COLOR_OUTPUT);
    uint_to_str(model, buf, 10);
    sh_writeln(buf);
    sh_write_color("  Revisão: ", GUI_COLOR_OUTPUT);
    uint_to_str(stepping, buf, 10);
    sh_writeln(buf);
    sh_write_color("  Modo:    ", GUI_COLOR_OUTPUT);
    sh_writeln("32-bit Modo Protegido");
}

static void cmd_reboot(void) {
    sh_writeln_color("Reiniciando...", GUI_COLOR_INFO);
    /* Pulse the keyboard controller reset line */
    uint8_t good = 0x02;
    while (good & 0x02) {
        __asm__ volatile("inb $0x64, %0" : "=a"(good));
    }
    __asm__ volatile("outb %0, $0x64" : : "a"((uint8_t)0xFE));
    /* Triple-fault fallback */
    __asm__ volatile("cli; hlt");
}

static void cmd_shutdown(void) {
    sh_writeln_color("Desligando... (ACPI poweroff via porta 0x604)", GUI_COLOR_INFO);
    /* QEMU/Bochs ACPI shutdown port */
    __asm__ volatile("outw %0, %1" : : "a"((uint16_t)0x2000), "Nd"((uint16_t)0x604));
    /* VirtualBox / older QEMU */
    __asm__ volatile("outw %0, %1" : : "a"((uint16_t)0x0034), "Nd"((uint16_t)0x4004));
    sh_writeln_color("(Desligue manualmente se necessário)", GUI_COLOR_ERROR);
}

static void cmd_banner(void) {
    sh_writeln_color(" ___  _              _              ___  ___  ", GUI_COLOR_INFO);
    sh_writeln_color("/ _ \\| |_ __ ___   (_) _   ___  __/ __// _ \\", GUI_COLOR_INFO);
    sh_writeln_color("| | | | __/ _` \\ \\/ /| | | | \\ \\/ /\\__ \\| | |", GUI_COLOR_INFO);
    sh_writeln_color("| |_| | || (_| |>  < | |_| |>  < ___) | |_|", GUI_COLOR_INFO);
    sh_writeln_color("\\___/ \\__\\__,_/_/\\_\\|\\__,_/_/\\_\\____/ \\___/", GUI_COLOR_INFO);
    sh_writeln_color("", GUI_COLOR_OUTPUT);
    sh_writeln_color("    Sistema Operacional  -  versao 1.0.0", GUI_COLOR_SUCCESS);
}

static void cmd_version(void) {
    sh_writeln_color("otaviux-so versao 1.0.0", GUI_COLOR_SUCCESS);
    sh_writeln_color("Compilado com GCC + NASM", GUI_COLOR_OUTPUT);
    sh_writeln_color("Bootloader: GRUB (Multiboot v1)", GUI_COLOR_OUTPUT);
}

/* ─── Command dispatcher ───────────────────────────────────────────────────── */

static void dispatch(char *line) {
    /* Skip leading spaces */
    while (*line == ' ') line++;
    if (!*line) return;

    /* Split at first space to separate command from args */
    char *args = NULL;
    for (int i = 0; line[i]; i++) {
        if (line[i] == ' ') {
            line[i] = '\0';
            args = line + i + 1;
            while (*args == ' ') args++;
            break;
        }
    }

    if (strcmp(line, "help")     == 0) cmd_help();
    else if (strcmp(line, "about")    == 0) cmd_about();
    else if (strcmp(line, "clear")    == 0) cmd_clear();
    else if (strcmp(line, "echo")     == 0) cmd_echo(args);
    else if (strcmp(line, "color")    == 0) cmd_color(args);
    else if (strcmp(line, "mem")      == 0) cmd_mem();
    else if (strcmp(line, "cpu")      == 0) cmd_cpu();
    else if (strcmp(line, "reboot")   == 0) cmd_reboot();
    else if (strcmp(line, "shutdown") == 0) cmd_shutdown();
    else if (strcmp(line, "banner")   == 0) cmd_banner();
    else if (strcmp(line, "version")  == 0) cmd_version();
    else {
        sh_write_color("Comando nao encontrado: ", GUI_COLOR_ERROR);
        sh_writeln_color(line, GUI_COLOR_ERROR);
        sh_writeln_color("Digite 'help' para listar os comandos.", GUI_COLOR_OUTPUT);
    }
}

/* ─── Prompt ──────────────────────────────────────────────────────────────── */

static void print_prompt(void) {
    sh_write_color("otaviux", GUI_COLOR_SUCCESS);
    sh_write_color("@", vga_make_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    sh_write_color("so", GUI_COLOR_INFO);
    sh_write_color(":~$ ", GUI_COLOR_PROMPT);
    sb_render();
}

/* ─── Public API ──────────────────────────────────────────────────────────── */

void shell_init(void) {
    sh_color = GUI_COLOR_OUTPUT;
    /* Initialise scrollback buffer */
    for (int r = 0; r < SB_ROWS; r++) sb_reset_row(r);
    sb_new_row();
}

void shell_run(void) {
    cmd_banner();
    sh_writeln_color("", GUI_COLOR_OUTPUT);
    sh_writeln_color("Bem-vindo ao otaviux! Digite 'help' para ver os comandos.", GUI_COLOR_SUCCESS);
    sh_writeln_color("", GUI_COLOR_OUTPUT);

    char cmd_buf[SHELL_MAX_CMD];
    int  cmd_len = 0;

    print_prompt();

    while (1) {
        char c = keyboard_getchar();

        if (c == '\n' || c == '\r') {
            /* Echo newline */
            sb_new_row();
            cmd_buf[cmd_len] = '\0';
            if (cmd_len > 0) {
                dispatch(cmd_buf);
            }
            cmd_len = 0;
            sb_new_row();
            print_prompt();

        } else if (c == '\b' || c == (char)KEY_DELETE) {
            if (cmd_len > 0) {
                cmd_len--;
                /* Remove last char from scrollback */
                if (sh_col > 0) {
                    sh_col--;
                    int row = (sb_write - 1 + SB_ROWS) % SB_ROWS;
                    sb_chars[row][sh_col]  = ' ';
                    sb_colors[row][sh_col] = GUI_COLOR_SHELL_BG;
                    sb_render();
                }
            }

        } else if (c >= 32 && c < 127) {
            if (cmd_len < SHELL_MAX_CMD - 1) {
                cmd_buf[cmd_len++] = c;
                sb_put(c, GUI_COLOR_INPUT);
                sb_render();
            }

        } else if ((uint8_t)c == KEY_ARROW_UP || (uint8_t)c == KEY_ARROW_DOWN ||
                   (uint8_t)c == KEY_ARROW_LEFT || (uint8_t)c == KEY_ARROW_RIGHT) {
            /* Arrow keys - ignore for now */
        }
        /* ESC, F-keys, etc. - ignored */
    }
}
