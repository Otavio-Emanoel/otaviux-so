#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

/* Keyboard IRQ number */
#define IRQ_KEYBOARD 1

/* Special key codes returned by keyboard_getchar() */
#define KEY_NONE      0
#define KEY_BACKSPACE 8
#define KEY_TAB       9
#define KEY_ENTER     13
#define KEY_ESCAPE    27
#define KEY_ARROW_UP    0x80
#define KEY_ARROW_DOWN  0x81
#define KEY_ARROW_LEFT  0x82
#define KEY_ARROW_RIGHT 0x83
#define KEY_F1        0x90
#define KEY_F2        0x91
#define KEY_F3        0x92
#define KEY_DELETE    0x7F

void keyboard_init(void);
char keyboard_getchar(void);    /* blocking: waits for a keypress */
int  keyboard_has_input(void);  /* non-blocking: 1 if key available */
uint8_t keyboard_get_modifiers(void);

/* Modifier flags */
#define MOD_SHIFT    (1 << 0)
#define MOD_CTRL     (1 << 1)
#define MOD_ALT      (1 << 2)
#define MOD_CAPSLOCK (1 << 3)

#endif /* KEYBOARD_H */
