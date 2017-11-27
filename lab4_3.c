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

void reset_irq(unsigned char bit)
{
    *((volatile unsigned long*) GPIO_E) = 0x00005555;           // Mark E output
    *((volatile unsigned char*) (GPIO_E + GPIO_ODR)) |= bit;    // Toggle reset bit on
    *((volatile unsigned char*) (GPIO_E + GPIO_ODR)) &= ~bit;   // Toggle it off
    *((volatile unsigned long*) GPIO_E) = 0;                    // Mark E input again
}

void flipflop_interrupt_handler(void)
{
    unsigned long trigger, ctrl;

    trigger = *((volatile unsigned long*) (0x40013C14));
    
    if (trigger & 8)
    {
        ctrl = *((volatile unsigned long*) (GPIO_E + GPIO_IDR));
        
        if (ctrl & 1)
        {
            reset_irq(0x10);
            ++count;
        }
        if (ctrl & 2)
        {
            reset_irq(0x20);
            count = 0;
        }
        if (ctrl & 4)
        {
            reset_irq(0x40);
            count = (count == 0xFF) ? 0 : 0xFF;
        }
        
        *((volatile unsigned long*) (0x40013C14)) |= 8; // reset trigger
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
    *((volatile unsigned long*) GPIO_D) = 0x55555555;
    // Connect PE3 to interrupt line EXTI3
    *((volatile unsigned long*) (0x40013808)) &= ~0xF000;
    *((volatile unsigned long*) (0x40013808)) |= 0x4000;
    
    // Setup EXTI3 to generate interrupts
    *((volatile unsigned long*) (0x40013C00)) |= 8;
    // Set EXTI3 to generate interupts at falling edge
    *((volatile unsigned long*) (0x40013C0C)) |= 8;
    *((volatile unsigned long*) (0x40013C08)) &= ~8;
    
    // Setup interrupt vector
    *((void (**)(void)) 0x2001C064) = flipflop_interrupt_handler;
    
    // Setup NVIC
    *((volatile unsigned long*) 0xE000E100) |= 1 << 9;
}

void main(void)
{
    init_app();
    while (1)
    {
        *((volatile unsigned char*) 0x40020C14) = count;
    }
}
