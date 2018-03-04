/**
 ******************************************************************************
 * @file    exercise2.c
 * @author  SE
 * @date    28022018
 * @brief
 ******************************************************************************
 *
 */
#include "stdio.h"
#include "board.h"
#include "stm32f4xx_hal_conf.h"
#include "debug_printf.h"
/* Includes ------------------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define PERIOD 60
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef TIM_Init;
volatile int toggleValue = 0;
/* Private function prototypes -----------------------------------------------*/
void Hardware_init(void);
void TIM2_IRQHandler(void);
/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Main program - Timer 2 compare update compare interrupt
 * @param  None
 * @retval None
 */
int main(void) {
	BRD_init(); // Initialise Board
	Hardware_init(); // Initialise hardware peripherals

	/* Infinite loop */
	while (1) {
		//Do nothing - wait for interrupts to trigger
	}
}

/**
 * @brief  Initialise Hardware
 * @param  None
 * @retval None
 */
void Hardware_init(void) {

	unsigned short PrescalerValue;

	// Timer 2 clock enable
	__TIM2_CLK_ENABLE();

	// Compute the prescaler value
	// Set the clock prescaler to 50kHz
	// SystemCoreClock is the system clock frequency
	PrescalerValue = (uint16_t) ((SystemCoreClock) / 50000) - 1;

	/* TIM Base configuration */
	TIM_Init.Instance = TIM2;				//Enable Timer 2
  	TIM_Init.Init.Period = 50000 * PERIOD / 1000;		//Cause interrupt every period milliseconds
  	TIM_Init.Init.Prescaler = PrescalerValue;	//Set prescaler value
  	TIM_Init.Init.ClockDivision = 0;			//Set clock division
	TIM_Init.Init.RepetitionCounter = 0;	// Set reload Value
  	TIM_Init.Init.CounterMode = TIM_COUNTERMODE_UP;	//Set timer to count up.

	/* Initialise Timer */
	HAL_TIM_Base_Init(&TIM_Init);

	/* Set priority of Timer 2 update Interrupt [0 (HIGH priority) to 15(LOW priority)] */
	/* 	DO NOT SET INTERRUPT PRIORITY HIGHER THAN 3 */
	HAL_NVIC_SetPriority(TIM2_IRQn, 10, 0);		//Set Main priority to 10 and sub-priority to 0.

	// Enable the Timer 2 interrupt
	HAL_NVIC_EnableIRQ(TIM2_IRQn);

	// Start Timer 2 base unit in interrupt mode
	HAL_TIM_Base_Start_IT(&TIM_Init);

	GPIO_InitTypeDef GPIO_Init;

	__GPIOB_CLK_ENABLE();
	GPIO_Init.Pin = BRD_D15_PIN;
	GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_Init.Pull = GPIO_PULLUP;
	GPIO_Init.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(BRD_D15_GPIO_PORT, &GPIO_Init);

}

/**
 * @brief Period elapsed callback in non blocking mode
 * @param htim: Pointer to a TIM_HandleTypeDef that contains the configuration information for the TIM module.
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {

	HAL_GPIO_WritePin(BRD_D15_GPIO_PORT, BRD_D15_PIN, toggleValue);
	toggleValue = 1 - toggleValue;

}

//Override default mapping of this handler to Default_Handler
void TIM2_IRQHandler(void) {
	HAL_TIM_IRQHandler(&TIM_Init);
}
