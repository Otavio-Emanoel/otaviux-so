/* keyboard.c - PS/2 keyboard driver */

#include "keyboard.h"
#include "idt.h"
#include "pic.h"

/* Keyboard I/O port */
#define KB_DATA_PORT  0x60

/* IRQ1 vector after PIC remapping */
#define KB_IDT_VECTOR (PIC1_OFFSET + IRQ_KEYBOARD)

/* Circular key buffer */
#define KB_BUFFER_SIZE 64

static volatile char kb_buffer[KB_BUFFER_SIZE];
static volatile int  kb_head = 0;  /* write index */
static volatile int  kb_tail = 0;  /* read index  */

static volatile uint8_t kb_modifiers = 0;

/* Declared in keyboard_asm.asm */
extern void keyboard_irq_stub(void);

/* ─── Scan-code to ASCII tables (US QWERTY, Set 1) ──────────────────────── */

static const char sc_normal[128] = {
    0,    27,  '1', '2', '3', '4', '5', '6', '7', '8',  /* 0x00-0x09 */
   '9',  '0', '-', '=', '\b','\t','q', 'w', 'e', 'r',   /* 0x0A-0x13 */
   't',  'y', 'u', 'i', 'o', 'p', '[', ']', '\n',  0,   /* 0x14-0x1D */
   'a',  's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',   /* 0x1E-0x27 */
   '\'', '`',  0,  '\\','z', 'x', 'c', 'v', 'b', 'n',   /* 0x28-0x31 */
   'm',  ',', '.', '/',  0,  '*',  0,  ' ',  0,   0,    /* 0x32-0x3B */
    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,    /* 0x3C-0x45 */
    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,    /* 0x46-0x4F */
    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,    /* 0x50-0x59 */
    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,    /* 0x5A-0x63 */
    0,    0,   0,   0,   0,   0,   0,   0               /* 0x64-0x6B */
};

static const char sc_shift[128] = {
    0,    27,  '!', '@', '#', '$', '%', '^', '&', '*',  /* 0x00-0x09 */
   '(',  ')', '_', '+', '\b','\t','Q', 'W', 'E', 'R',   /* 0x0A-0x13 */
   'T',  'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',  0,  /* 0x14-0x1D */
   'A',  'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',  /* 0x1E-0x27 */
   '"',  '~',  0,  '|', 'Z', 'X', 'C', 'V', 'B', 'N',  /* 0x28-0x31 */
   'M',  '<', '>', '?',  0,  '*',  0,  ' ',  0,   0,   /* 0x32-0x3B */
    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   /* 0x3C-0x45 */
    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   /* 0x46-0x4F */
    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   /* 0x50-0x59 */
    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   /* 0x5A-0x63 */
    0,    0,   0,   0,   0,   0,   0,   0              /* 0x64-0x6B */
};

/* ─── IRQ handler (called from keyboard_irq_stub) ────────────────────────── */

void keyboard_irq_handler(void) {
    uint8_t scan = inb(KB_DATA_PORT);
    uint8_t released = scan & 0x80;
    uint8_t code     = scan & 0x7F;

    /* Handle modifier keys */
    switch (code) {
        case 0x2A: /* Left Shift */
        case 0x36: /* Right Shift */
            if (released) kb_modifiers &= (uint8_t)~MOD_SHIFT;
            else           kb_modifiers |= MOD_SHIFT;
            break;
        case 0x1D: /* Ctrl */
            if (released) kb_modifiers &= (uint8_t)~MOD_CTRL;
            else           kb_modifiers |= MOD_CTRL;
            break;
        case 0x38: /* Alt */
            if (released) kb_modifiers &= (uint8_t)~MOD_ALT;
            else           kb_modifiers |= MOD_ALT;
            break;
        case 0x3A: /* Caps Lock - toggle on press */
            if (!released) kb_modifiers ^= MOD_CAPSLOCK;
            break;
        default:
            if (!released && code < 128) {
                /* Determine effective character */
                int shifted = (kb_modifiers & MOD_SHIFT) != 0;
                if (kb_modifiers & MOD_CAPSLOCK) shifted = !shifted;

                char c = shifted ? sc_shift[code] : sc_normal[code];

                /* Arrow keys (extended - simplified single-byte version) */
                if (code == 0x48) c = (char)KEY_ARROW_UP;
                if (code == 0x50) c = (char)KEY_ARROW_DOWN;
                if (code == 0x4B) c = (char)KEY_ARROW_LEFT;
                if (code == 0x4D) c = (char)KEY_ARROW_RIGHT;
                if (code == 0x53) c = (char)KEY_DELETE;

                /* Function keys */
                if (code >= 0x3B && code <= 0x44)
                    c = (char)(KEY_F1 + (code - 0x3B));

                /* Push to circular buffer if we have space */
                if (c != 0) {
                    int next = (kb_head + 1) % KB_BUFFER_SIZE;
                    if (next != kb_tail) {
                        kb_buffer[kb_head] = c;
                        kb_head = next;
                    }
                }
            }
            break;
    }

    pic_send_eoi(IRQ_KEYBOARD);
}

/* ─── Public API ─────────────────────────────────────────────────────────── */

void keyboard_init(void) {
    /* Install the IRQ1 handler in IDT slot (PIC1_OFFSET + 1) */
    idt_set_gate(KB_IDT_VECTOR,
                 (uint32_t)keyboard_irq_stub,
                 0x08,   /* kernel code segment */
                 0x8E);  /* present, ring 0, 32-bit interrupt gate */

    /* Unmask IRQ1 */
    pic_unmask_irq(IRQ_KEYBOARD);
}

int keyboard_has_input(void) {
    return kb_head != kb_tail;
}

char keyboard_getchar(void) {
    /* Spin until a key is available (interrupts must be enabled) */
    while (!keyboard_has_input()) {
        __asm__ volatile("hlt");
    }
    char c = kb_buffer[kb_tail];
    kb_tail = (kb_tail + 1) % KB_BUFFER_SIZE;
    return c;
}

uint8_t keyboard_get_modifiers(void) {
    return kb_modifiers;
}
