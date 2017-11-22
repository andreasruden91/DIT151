#ifndef KEYBOARD_H
#define KEYBOARD_H

// Prerequisite: Keyboard connected to hi byte of GPIO D
void kbd_init(void);

// Returns first noticed key that is currently pressed down
int kbdchr(void);

#endif
