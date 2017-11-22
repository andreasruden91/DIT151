/*
 * 	startup.c
 *
 */
 
 #include "keyboard.h"
 #include "displays.h"

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
#endif
    
    kbd_init();
    seg7_init();
}

void main(void)
{
    unsigned int key;
    init_app();
    
    while (1)
    {
        key = kbdchr();
        seg7_write(key);
    }

    return 0;
}
