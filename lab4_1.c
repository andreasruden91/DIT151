/*
 * 	startup.c
 *
 */

#include "defines.h"

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

#define DELAY_COUNT 10000

int systick_flag = 0;
int delay_count = 0;

void delay_250ns_irq(void)
{
    *SysTick_CTRL = 0;
    *SysTick_LOAD = DELAY_250NS_CYCLES;
    *SysTick_VAL = 0;
    *SysTick_CTRL = 7;
}

void systick_irq_handler(void)
{
    *SysTick_CTRL = 0;
    if (--delay_count > 0)
        delay_250ns_irq();
    else
        systick_flag = 1;
}

void delay(int count)
{
    if (count > 0)
    {
        systick_flag = 0;
        delay_count = count;
        delay_250ns_irq();
    }
}

void init_app(void)
{
#ifdef USBDM
    /* starta klockor port D och E */
    *((volatile unsigned long*) 0x40023830) = 0x18;
    /* initiera PLL */
    __asm volatile("LDR R0, =0x08000209\n BLX R0 \n")
    /* relokera vektortabell */
    *((volatile unsigned long*) 0xE000ED08) = 0x2001C000;
#endif
    /* initiera port D */
    *portD_moder = 0x00005555;
    
    /* initiera undantagsvektor */
    *((void (**)(void)) 0x2001C03C) = systick_irq_handler;
}

int main(void)
{
    init_app();
    
    delay(DELAY_COUNT);
    *portD_odr_lo = 0xFF;
    while(1)
    {
        if (systick_flag)
            break;
    }
    *portD_odr_lo = 0;
    
    return 0;
}
