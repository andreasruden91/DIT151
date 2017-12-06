/*
 * 	startup.c
 *
 */

#include "defines.h"
 
void startup(void) __attribute__((naked)) __attribute__((section (".start_section")) );

void startup ( void )
{
__asm volatile(
	" LDR R0,=0x2001C000\n"
	" MOV SP,R0\n"
	" BL main\n"			
	"_exit: B .\n"			
	) ;
}

unsigned char count = 0;

void flipflop_interrupt_handler(void)
{
    unsigned long data = *EXTI_PR;
    if (data & 8)
    {
        ++count;
        *EXTI_PR |= 8;
    }
}

void init_app(void)
{
#ifdef USBDM
    /* starta klockor port D och E */
    *((volatile unsigned long*)0x40023830) = 0x18;
    /* starta klockor för SYSCFG */
    *((volatile unsigned long*) 0x40023844) |= 0x4000;
    /* relokera vektortabell */
    *((volatile unsigned long*) 0xE000ED08) = 0x2001C000;
#endif

    // Setup GPIO-D as output
    *portD_moder = 0x55555555;
    // Connect PE3 to interrupt line EXTI3
    *SYSCFG_EXTICR1 &= ~0xF000;
    *SYSCFG_EXTICR1 |= 0x4000;
    
    // Setup EXTI3 to generate interrupts
    *EXTI_IMR |= 8;
    // Set EXTI3 to generate interupts at falling edge
    *EXTI_FTSR |= 8;
    *EXTI_RTSR &= ~8;
    
    // Setup interrupt vector
    *((void (**)(void)) 0x2001C064) = flipflop_interrupt_handler;
    
    // Setup NVIC
    *NVIC_ISER0 |= 1 << 9;
}

void main(void)
{
    init_app();
    while (1)
    {
        *portD_odr_lo = count;
    }
}
