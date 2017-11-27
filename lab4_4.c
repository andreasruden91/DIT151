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

void irq0_interrupt_handler(void)
{
    unsigned long trigger;

    trigger = *((volatile unsigned long*) (EXTI_PR));
    
    if (trigger & EXTI0_IRQ_BPOS)
    {
        reset_irq(0x10);
        ++count;
        
        *((volatile unsigned long*) (EXTI_PR)) |= EXTI0_IRQ_BPOS; // reset trigger
    }
}

void irq1_interrupt_handler(void)
{
    unsigned long trigger;

    trigger = *((volatile unsigned long*) (EXTI_PR));
    
    if (trigger & EXTI1_IRQ_BPOS)
    {
        reset_irq(0x20);
        count = 0;
        
        *((volatile unsigned long*) (EXTI_PR)) |= EXTI1_IRQ_BPOS; // reset trigger
    }
}

void irq2_interrupt_handler(void)
{
    unsigned long trigger;

    trigger = *((volatile unsigned long*) (EXTI_PR));
    
    if (trigger & EXTI2_IRQ_BPOS)
    {
        reset_irq(0x40);
        count = (count == 0xFF) ? 0 : 0xFF;
        
        *((volatile unsigned long*) (EXTI_PR)) |= EXTI2_IRQ_BPOS; // reset trigger
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
    // Connect PE2-0 to interrupt line EXTI2-0
    *((volatile unsigned long*) (SYSCFG_EXTICR1)) &= ~0x0FFF;
    *((volatile unsigned long*) (SYSCFG_EXTICR1)) |= 0x0444;
    
    // Setup EXTI2-0 to generate interrupts
    *((volatile unsigned long*) (EXTI_IMR)) |= (EXTI2_IRQ_BPOS | EXTI1_IRQ_BPOS | EXTI0_IRQ_BPOS);
    // Set EXTI2-0 to generate interupts at rising edge
    *((volatile unsigned long*) (EXTI_FTSR)) &= ~(EXTI2_IRQ_BPOS | EXTI1_IRQ_BPOS | EXTI0_IRQ_BPOS);
    *((volatile unsigned long*) (EXTI_RTSR)) |= (EXTI2_IRQ_BPOS | EXTI1_IRQ_BPOS | EXTI0_IRQ_BPOS);
    
    // Setup interrupt vector
    *((void (**)(void)) EXTI2_IRQVEC) = irq2_interrupt_handler;
    *((void (**)(void)) EXTI1_IRQVEC) = irq1_interrupt_handler;
    *((void (**)(void)) EXTI0_IRQVEC) = irq0_interrupt_handler;
    
    // Setup NVIC
    *((volatile unsigned long*) NVIC_ISER0) |= (NVIC_EXTI2_IRQ_BPOS | NVIC_EXTI1_IRQ_BPOS | NVIC_EXTI0_IRQ_BPOS);
}

void main(void)
{
    init_app();
    while (1)
    {
        *((volatile unsigned char*) (GPIO_D + GPIO_ODR)) = count;
    }
}
