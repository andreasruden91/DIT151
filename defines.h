#ifndef DEFINES_H
#define DEFINES_H

/* GPIO */
#define GPIO_MODER                  0
#define GPIO_OTYPER                 4
#define GPIO_OSPEEDR                8
#define GPIO_PUPDR                  0xC
#define GPIO_IDR                    0x10
#define GPIO_ODR                    0x14

#define GPIO_D                      0x40020C00
#define GPIO_E                      0x40021000

/* System Clock */
#define SysTick_BASE                0xE000E010
#define SysTick_CTRL                (SysTick_BASE)
#define SysTick_LOAD                (SysTick_BASE+4)
#define SysTick_VAL                 (SysTick_BASE+8)
#define DELAY_250NS_CYCLES          (42-1)
 

#endif
