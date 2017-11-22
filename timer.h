#ifndef TIMER_H
#define TIMER_H

// Each cycle is slightly below 6 nano seconds
void delay_cycles(unsigned short cycles);

void delay_micro(unsigned int us);

void delay_milli(unsigned int ms);

void delay_250ns(void);
void delay_500ns(void);

#endif
