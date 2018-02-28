/**
 ******************************************************************************
 * @file    demo1/main.c
 * @author  SE
 * @date    21022018-28022018
 * @brief   Demonstrate light bar functionality
 ******************************************************************************
 *
 */
/* Includes ------------------------------------------------------------------*/
#include "board.h"
#include "stm32f4xx_hal_conf.h"
#include "debug_printf.h"
#include "s4435360_hal_lightbar.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define SHIFT_RIGHT(value) value = value << 1;
#define SHIFT_LEFT(value) value = value >> 1;
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
const int leftEdge = 3; //Display value of lightbar left edge
const int rightEdge = 768; //Display value of lightbar right edge
const int shortestDelay = 100; //Shortest delay allowed
const int longestDelay = 1000; //Longest delay allowed
const uint32_t debounceThreshold = 100; //Time threshold to debounce signal

volatile uint32_t lastInterruptTime = 0; //Time of the last button interrupt
volatile int delayTime = 1000; //The current delay time for the lightbar
volatile int isSlowingDown = 0; //Whether the lightbar is slowing

/* Private function prototypes -----------------------------------------------*/
void hardware_init();
/* Private functions ---------------------------------------------------------*/

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

	/* Main loop */
	while(1) {

		debug_printf("0x%04x\n\r", displayValue);
		s4435360_lightbar_write(displayValue);

		/* Check for touching edges */
		if((displayValue == leftEdge) || (displayValue == rightEdge)) {
			isShiftingRight = 1 - isShiftingRight;
		}

		/* Shift appropriate direction */
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

	/* Check time since last interrupt, to debounce */
	if(currentTime - lastInterruptTime >= debounceThreshold) {

		/* Shift delay appropriate direction */
		if(isSlowingDown) {
			SHIFT_RIGHT(delayTime);
		} else {
			SHIFT_LEFT(delayTime);
		}

		/* Check for delay length boundaries */
		if((delayTime >= longestDelay) || (delayTime <= shortestDelay)) {
			isSlowingDown = 1 - isSlowingDown;
		}

	}

	lastInterruptTime = currentTime;
}

/* Override default mapping of this handler to Default_Handler */
void EXTI15_10_IRQHandler(void) {
	HAL_GPIO_EXTI_IRQHandler(BRD_USER_BUTTON_PIN);
}
