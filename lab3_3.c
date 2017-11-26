/*
 * 	startup.c
 *
 */
 
#include "displays.h"
#include "gameobject.h"
#include "keyboard.h"

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
    kbd_init();
}

static const POINT ballGeometry[] =
{
    {0,1}, {0,2}, {1,0}, {1,1}, {1,2},
    {1,3}, {2,0}, {2,1}, {2,2}, {2,3},
    {3,1}, {3,2}
};

int main(void)
{
    int key;
    GAMEOBJECT ball;
    
    init_app();
    
    ball = make_gameobject_raw(1, 1, 4, 4, 12, ballGeometry);
    
#ifndef SIMULATOR
    lcd_clear_force();
#endif
    
    while (1)
    {
        key = kbdchr();
        switch (key)
        {
        case 6:
            gameobject_set_speed(&ball, 2, 0);
            break;
        case 4:
            gameobject_set_speed(&ball, -2, 0);
            break;
        case 2:
            gameobject_set_speed(&ball, 0, -2);
            break;
        case 8:
            gameobject_set_speed(&ball, 0, 2);
            break;
        }
        
        gameobject_update(&ball);
        delay_milli(40);
    }
    
    return 0;
}
