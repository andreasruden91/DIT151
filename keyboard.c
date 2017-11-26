#include "keyboard.h"
#include "defines.h"
#include "timer.h"

static const int keyValues[] =
{
    1,    2,    3,    0xA,
    4,    5,    6,    0xB,
    7,    8,    9,    0xC,
    0x10, 0,    0x11, 0xD,
};


/*void kbd_init(void)
{
    // Set pin modes
    *((unsigned int*)(GPIO_D + GPIO_MODER)) &= 0x0000FFFF;
    *((unsigned int*)(GPIO_D + GPIO_MODER)) |= 0x55000000;
    
    // Set Output Type
    *((unsigned int*)(GPIO_D + GPIO_OTYPER)) &= 0xFFFF0FFF;
    
    // Set Pull Down for Input
    *((unsigned int*)(GPIO_D + GPIO_PUPDR)) &= 0xFF00FFFF;
    *((unsigned int*)(GPIO_D + GPIO_PUPDR)) |= 0x00AA0000;
}*/

/*int kbdchr(void)
{
    int i, j;
    
    for (i = 0; i < 4; ++i)
    {
        // Activate row
        *((unsigned int*)(GPIO_D + GPIO_ODR)) &= 0xFFFF00FF;
        *((unsigned int*)(GPIO_D + GPIO_ODR)) |= (1 << (12+i));
        
        // Check input pins
        unsigned int idr = *((unsigned int*)(GPIO_D + GPIO_IDR));
        for (j = 0; j < 4; ++j)
        {
            if (idr & (1 << (8+j)))
                return keyValues[i*4+j];
        }
    }
    
    return -1;
}*/

void kbd_init(void)
{
    // Set pin modes
    *((volatile unsigned long*)(GPIO_D + GPIO_MODER)) &= 0x0000FFFF;
    *((volatile unsigned long*)(GPIO_D + GPIO_MODER)) |= 0x55000000;
    
    // Set Output Type
    *((volatile unsigned char*)(GPIO_D + GPIO_OTYPER + 1)) = 0x00;
    
    // Set Pull Down for Input
    *((volatile unsigned char*)(GPIO_D + GPIO_PUPDR + 2)) = 0xAA;
}

int kbdchr(void)
{
    int i, j;
    
    for (i = 0; i < 4; ++i)
    {
        // Activate row
        *((volatile unsigned char*)(GPIO_D + GPIO_ODR + 1)) = (1 << (4+i));
        // delay_250ns(); // Wait for keyboard to update
        
        // Check input pins
        unsigned char idr = *((volatile unsigned char*)(GPIO_D + GPIO_IDR + 1));
        for (j = 0; j < 4; ++j)
        {
            if (idr & (1 << j))
                return keyValues[i*4+j];
        }
    }
    
    return -1;
}
