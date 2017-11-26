#ifndef DISPLAYS_H
#define DISPLAYS_H

// Prerequisite: Seg 7 display connected to lo byte of GPIO D
void seg7_init(void);

// Write int to 7seg (prereq: called seg7_init)
void seg7_write(int out);

/* Prerequisite is that the display is connected to GPIO E such that
 * bit 15-8 is the data register and 7-0 the control register. This
 * applies for all functions that follow */

void ascii_init(void);

// Set cursor of ascii display (prereq: called ascii_init + see above)
// x=[0,19], y=[0,1]
void ascii_gotoxy(int x, int y);

// Write a char to ascii display (prereq: called ascii_init + see above)
void ascii_write_char(char c);

void lcd_init(void);

// The buffer of the LCD is cleared; preparing it for a drawing cycle
// Only the pixels that were 1 last frame are cleared
void lcd_clear(void);

// Draw a pixel on the screen
void lcd_draw(int x, int y);

// Erase a pixel on the screen
void lcd_erase(int x, int y);

#endif
