#include "timer.h"
#include "defines.h"

void delay_cycles(unsigned short cycles)
{
    // Set all control bits to 0
    *((unsigned int*)SysTick_CTRL) &= 0xFFFEFF8;
    
    // Load delay value into register
    *((unsigned int*)SysTick_LOAD) = cycles;
    
    // Activate timer
    *((unsigned int*)SysTick_CTRL) |= 1;
    
    // Busy wait for COUNTFLAG to become 1
    while (*((unsigned int*)SysTick_CTRL) & (1 << 16) == 0)
        /*nothing*/;
}

void delay_250ns(void)
{
    delay_cycles(DELAY_250NS_CYCLES);
}

void delay_500ns(void)
{
    delay_cycles(2 * DELAY_250NS_CYCLES);
}

void delay_micro(unsigned int us)
{
    us *= 4;
    while (us--)
        delay_250ns();
}

void delay_milli(unsigned int ms)
{
    unsigned int us;
    
#ifdef SIMULATOR
    us = ms;
#else
    us = ms * 1000;
#endif

    delay_micro(us);
}
