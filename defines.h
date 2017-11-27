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

#define SYSCFG_BASE                 0x40013800
#define SYSCFG_EXTICR1              0x40013808
#define EXTI_IMR                    0x40013C00
#define EXTI_FTSR                   0x40013C0C
#define EXTI_RTSR                   0x40013C08
#define EXTI_PR                     0x40013C14
#define EXTI3_IRQVEC                0x2001C064
#define EXTI2_IRQVEC                0x2001C060
#define EXTI1_IRQVEC                0x2001C05C
#define EXTI0_IRQVEC                0x2001C058
#define NVIC_ISER0                  0xE000E100
#define NVIC_EXTI3_IRQ_BPOS         (1<<9)
#define NVIC_EXTI2_IRQ_BPOS         (1<<8)
#define NVIC_EXTI1_IRQ_BPOS         (1<<7)
#define NVIC_EXTI0_IRQ_BPOS         (1<<6)
#define EXTI3_IRQ_BPOS              (1<<3)
#define EXTI2_IRQ_BPOS              (1<<2)
#define EXTI1_IRQ_BPOS              (1<<1)
#define EXTI0_IRQ_BPOS              (1<<0)

#endif
