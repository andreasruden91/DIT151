/*
 * 	startup.c
 *
 */
 
#include "displays.h"

void startup(void) __attribute__((naked)) __attribute__((section (".start_section")) );

void startup (void)
{
__asm volatile(
    " LDR R0,=0x2001C000\n"
    " MOV SP,R0\n"
    " BL main\n"
    "_exit: B .\n"
    ) ;
}

void init_app(void)
{
#ifdef USBDM
    /* starta klockor port D och E */
    *((unsigned long*)0x40023830) = 0x18;
    /* initiera PLL */
    __asm volatile("LDR R0, =0x08000209\n BLX R0 \n")
#endif

    lcd_init();
}

int main(void)
{
    int i;
    
    init_app();
    
#ifndef SIMULATOR
    lcd_clear();
#endif

    for (i = 0; i < 128; ++i)
        lcd_draw(i, 10);
        
    for (i = 0; i < 64; ++i)
        lcd_draw(10, i);

    return 0;
}
