#ifndef SHELL_H
#define SHELL_H

/* Maximum length of a command line */
#define SHELL_MAX_CMD 256

/* Maximum number of arguments */
#define SHELL_MAX_ARGS 16

/* Shell window geometry (inner area, excluding border) */
#define SHELL_WIN_X   0
#define SHELL_WIN_Y   1
#define SHELL_WIN_W   VGA_WIDTH
#define SHELL_WIN_H   (VGA_HEIGHT - 2)

void shell_init(void);
void shell_run(void);   /* Never returns */

#endif /* SHELL_H */
