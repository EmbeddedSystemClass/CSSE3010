/**
 ******************************************************************************
 * @file    demo1/main.c
 * @author  SE
 * @date    23022018
 * @brief   Demonstrate light bar functionality
 ******************************************************************************
 *
 */
#include "board.h"
#include "stm32f4xx_hal_conf.h"
#include "debug_printf.h"
#include "s4435360_hal_lightbar.h"

#define SHIFT_RIGHT(value) value = value << 1;
#define SHIFT_LEFT(value) value = value >> 1;

const int leftEdge = 3;
const int rightEdge = 768;
const int shortestDelay = 100;
const int longestDelay = 1000;
const uint32_t debounceThreshold = 100;

volatile uint32_t lastInterruptTime = 0;
volatile int delayTime = 1000;
volatile int isSlowingDown = 0;

void hardware_init();

/**
 * @brief  Main program
 * @param  None
 * @retval None
 */
int main(void) {

	//Init hardware
	BRD_init();
	hardware_init();
	s4435360_lightbar_init();

	int isShiftingRight = 0;
	unsigned short displayValue = (unsigned short) leftEdge;

	while(1) {

		debug_printf("0x%04x\n\r", displayValue);
		s4435360_lightbar_write(displayValue);

		if((displayValue == leftEdge) || (displayValue == rightEdge)) {
			isShiftingRight = 1 - isShiftingRight;
		}

		if(isShiftingRight) {
			SHIFT_RIGHT(displayValue);
		} else {
			SHIFT_LEFT(displayValue);
		}

		HAL_Delay(delayTime);

	}
}

/**
 * @brief Initialises the additional hardware for this demo,
 * 			the user button
 * @param None
 * @retval None
 */
void hardware_init(void) {

	GPIO_InitTypeDef GPIO_InitStructure;

	BRD_USER_BUTTON_GPIO_CLK_ENABLE();

	GPIO_InitStructure.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Pin  = BRD_USER_BUTTON_PIN;
	HAL_GPIO_Init(BRD_USER_BUTTON_GPIO_PORT, &GPIO_InitStructure);

	HAL_NVIC_SetPriority(BRD_USER_BUTTON_EXTI_IRQn, 10, 0);
	HAL_NVIC_EnableIRQ(BRD_USER_BUTTON_EXTI_IRQn);
}

/**
 * @brief EXTI line detection callback
 * @param GPIO_Pin: Specifies the pins connected EXTI line
 * @retval None
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {

	uint32_t currentTime = HAL_GetTick();

	if(currentTime - lastInterruptTime >= debounceThreshold) {

		//Button pressed
		if(isSlowingDown) {
			SHIFT_RIGHT(delayTime);
		} else {
			SHIFT_LEFT(delayTime);
		}

		if((delayTime >= longestDelay) || (delayTime <= shortestDelay)) {
			isSlowingDown = 1 - isSlowingDown;
		}

	}

	lastInterruptTime = currentTime;
}

//Override default mapping of this handler to Default_Handler
void EXTI15_10_IRQHandler(void) {
	HAL_GPIO_EXTI_IRQHandler(BRD_USER_BUTTON_PIN);
}
