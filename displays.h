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
void lcd_clear(void);

// Mark a pixel as needing to be drawn; takes effect on lcd_present()
void lcd_draw(int x, int y);

// Write changes set up by lcd_clear() and lcd_draw() to the LCD screen
void lcd_present(void);

#endif
