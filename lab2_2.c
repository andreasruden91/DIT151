/*
 * 	startup.c
 *
 */
 
 #include "defines.h"
 #include "timer.h"

void startup(void) __attribute__((naked)) __attribute__((section (".start_section")) );

void startup ( void )
{
__asm volatile(
    " LDR R0,=0x2001C000\n"		/* set stack */
    " MOV SP,R0\n"
    " BL main\n"				/* call main */
    "_exit: B .\n"				/* never return */
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

    *((unsigned long*)(GPIO_E+GPIO_MODER)) = 0x00005555;
}

void main(void)
{
    unsigned int key;
    init_app();
    
    while (1)
    {
        *((unsigned long*)(GPIO_E+GPIO_ODR)) = 0;
        delay_milli(500);
        *((unsigned long*)(GPIO_E+GPIO_ODR)) = 0xFF;
        delay_milli(500);
    }

    return 0;
}
