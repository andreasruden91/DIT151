/*
 * 	startup.c
 *
 */
 
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
    /* initiera PLL */
    __asm volatile("LDR R0, =0x08000209\n BLX R0 \n")
#endif

    ascii_init();
}

void main(void)
{
    const char* s;
    const char* test1 = "Alfanumerisk ";
    const char* test2 = "Display - test";
    
    init_app();
    
    ascii_gotoxy(0, 0);
    s = test1;
    while (*s)
        ascii_write_char(*s++);
        
    ascii_gotoxy(0, 1);
    s = test2;
    while (*s)
        ascii_write_char(*s++);

    return 0;
}
