/**
 * @LD2
 *
 * Where is the LED connected
 * PORT A
 * PIN 5
 * Datasheet page no. 55
 *
 * RCC - Reset and Clock Control
 * AHB - Advance High-performance Bus
 * APB - Advance Peripheral Bus
 ******************************************************************************
 */
#include <stdint.h>

#define PERIPH_BASE			(0x40000000UL)
#define AHB1_PERIPH_OFFSET	(0x00020000UL)
#define AHB1_PERIPH_BASE		(PERIPH_BASE + AHB1_PERIPH_OFFSET)
#define GPIOA_OFFSET			(0x0000U)
#define GPIOA_ADDR				(AHB1_PERIPH_BASE + GPIOA_OFFSET)

#define RCC_OFFSET				(0x3800U)
#define RCC_ADDR				(AHB1_PERIPH_BASE + RCC_OFFSET)

#define __IO volatile

/// 8.4 GPIO registers
// since each type here is 32bit in size; meaning each register would occupy 4 byte in memory
typedef struct {
	__IO uint32_t MODER;   /*!< GPIO port mode register               (Address offset: 0x00) */
	__IO uint32_t OTYPER;  /*!< GPIO port output type register        (Address offset: 0x04) */
	__IO uint32_t OSPEEDR; /*!< GPIO port output speed register       (Address offset: 0x08) */
	__IO uint32_t PUPDR;   /*!< GPIO port pull-up/pull-down register  (Address offset: 0x0C) */
	__IO uint32_t IDR;     /*!< GPIO port input data register         (Address offset: 0x10) */
	__IO uint32_t ODR;     /*!< GPIO port output data register        (Address offset: 0x14) */
	__IO uint32_t BSRR;    /*!< GPIO port bit set/reset register      (Address offset: 0x18) */
	__IO uint32_t LCKR;    /*!< GPIO port configuration lock register (Address offset: 0x1C) */
	__IO uint32_t AFRL;    /*!< GPIO alternate function low register  (Address offset: 0x20) */
	__IO uint32_t AFRH;    /*!< GPIO alternate function high register (Address offset: 0x24) */
} GPIO_TypeDef;

/// 6.3 RCC registers
typedef struct {
    volatile uint32_t CR;           /*!< RCC clock control register                     (Address offset: 0x00) */
    volatile uint32_t PLLCFGR;      /*!< RCC PLL configuration register                 (Address offset: 0x04) */
    volatile uint32_t CFGR;         /*!< RCC clock configuration register               (Address offset: 0x08) */
    volatile uint32_t CIR;          /*!< RCC clock interrupt register                   (Address offset: 0x0C) */
    volatile uint32_t AHB1RSTR;     /*!< RCC AHB1 peripheral reset register             (Address offset: 0x10) */
    volatile uint32_t AHB2RSTR;     /*!< RCC AHB2 peripheral reset register             (Address offset: 0x14) */
    volatile uint32_t Reserved[2];  /*!< Reserved locations (0x18 - 0x1C)               (Address offset: 0x18) */
    volatile uint32_t APB1RSTR;     /*!< RCC APB1 peripheral reset register             (Address offset: 0x20) */
    volatile uint32_t APB2RSTR;     /*!< RCC APB2 peripheral reset register             (Address offset: 0x24) */
    volatile uint32_t Reserved2[2]; /*!< Reserved locations (0x28 - 0x2C)               (Address offset: 0x28) */
    volatile uint32_t AHB1ENR;      /*!< RCC AHB1 peripheral clock enable register      (Address offset: 0x30) */
} RCC_TypeDef;

#define GPIOA		((GPIO_TypeDef*) GPIOA_ADDR)
#define RCC		((RCC_TypeDef*) RCC_ADDR)


/**
 * Bit mask to enable GPIO Port A clock in RCC AHB1ENR (Bit 0).
 *
 * 1U in 32-bit binary:
 * 0b0000 0000 0000 0000 0000 0000 0000 0001  (Bit 0 is set)
 *
 * (1U << 0) shifts by 0 positions (remains at Bit 0):
 * 0b 0000 0000 0000 0000 0000 0000 0000 0001
 *
 * For comparison, (1U << 8) shifts 8 positions left (moves to Bit 8):
 * 0b 0000 0000 0000 0000 0000 0001 0000 0000
 */
#define GPIOAEN 	(1U << 0)
#define PIN5	 	(1U<<5)
#define LED_PIN 	PIN5


#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif

static void delay_cycles(volatile uint32_t count)
{
    while (count--) {
        __asm("nop");
    }
}

/*
 * (1U<<10) sets bit 10 to 1
 * |= (1U<<10) means no other bits will be changed apart from bit 10
 * &=~ (1U<<10) clears (resets) bit 10 to 0 while leaving all other bits unchanged.
 * */

int main(void)
{
    // 1. Enable clock access to GPIOA
	RCC->AHB1ENR |= GPIOAEN;

	/* 2. Configure PA5 as General-Purpose Output mode (01b)
	 * Reference RM0383 Section 8.4.1 (GPIOx_MODER):
	 *   - Field MODERy[1:0] occupies bits [2y+1 : 2y], where y = Pin 5 (bits 11:10)
	 *   - Mode values: 00 = Input, 01 = Output, 10 = Alternate Function, 11 = Analog
	 * Clear bits 11:10 first to avoid invalid intermediate states, then set bit 10
	 */
	GPIOA->MODER &=~ (3U << (5 * 2)); // 3U (0b11) clears MODER5[1:0] (bits 11:10)
	GPIOA->MODER |=  (1U << (5 * 2)); // Set bit 10 to 1 (MODER5 = 01b -> Output)

	while (1){
		// Set PA5 HIGH
		//GPIOA_OD_R |= LED_PIN;

		// Toggle PA5 HIGH
		GPIOA->ODR ^= LED_PIN;
		delay_cycles(500000);
	}
}









