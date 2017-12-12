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

void kbd_init(void)
{
    *((unsigned int*)(GPIO_D + GPIO_MODER)) &= 0x0000FFFF;
    *((unsigned int*)(GPIO_D + GPIO_MODER)) |= 0x55000000;
    
    // Set Output Type
    *((unsigned short*)(GPIO_D + GPIO_OTYPER)) &= 0x0FFF;
    
    // Set Pull Down for Input
    *((unsigned int*)(GPIO_D + GPIO_PUPDR)) &= 0xFF00FFFF;
    *((unsigned int*)(GPIO_D + GPIO_PUPDR)) |= 0x00AA0000;
}

int kbdchr(void)
{
    int i, j;
    
    // Loop through rows
    for (i = 0; i < 4; ++i)
    {
        // Activate row
        //*((unsigned char*)(GPIO_D + GPIO_ODR + 1)) &= 0x0F;
        //*((unsigned char*)(GPIO_D + GPIO_ODR + 1)) |= (1 << (4+i));
        *((unsigned char*)(GPIO_D + GPIO_ODR + 1)) = (1 << (4+i));
        delay_250ns();
        // Check input pins
        unsigned char idr = *((unsigned char*)(GPIO_D + GPIO_IDR + 1));
        for (j = 0; j < 4; ++j)
        {
            if (idr & (1 << (j)))
                return keyValues[i*4+j];
        }
    }
    
    return -1;
}

int iskeydown(int chr)
{
    int i, j;
    
    // Loop through rows
    for (i = 0; i < 4; ++i)
    {
        // Activate row
        //*((unsigned char*)(GPIO_D + GPIO_ODR + 1)) &= 0x0F;
        //*((unsigned char*)(GPIO_D + GPIO_ODR + 1)) |= (1 << (4+i));
        *((unsigned char*)(GPIO_D + GPIO_ODR + 1)) = (1 << (4+i));
        delay_250ns();
        // Check input pins
        unsigned char idr = *((unsigned char*)(GPIO_D + GPIO_IDR + 1));
        for (j = 0; j < 4; ++j)
        {
            if (idr & (1 << (j)))
            {
                if (keyValues[i*4+j] == chr)
                    return 1;
            }
        }
    }
    
    return 0;
}
