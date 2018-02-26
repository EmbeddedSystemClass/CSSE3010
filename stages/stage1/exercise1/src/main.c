/**
 ******************************************************************************
 * @file    exercise1.c
 * @author  Samuel Eadie - 44353607
 * @date    21022018
 * @brief   Debounces a mechanical button - toggles an LED
 ******************************************************************************
 *
 */
/* Includes ------------------------------------------------------------------*/
#include "stdio.h"
#include "board.h"
#include "stm32f4xx_hal_conf.h"
#include "debug_printf.h"
#include "stm32f4xx_hal_gpio.h"

void Hardware_init(void);
void EXTI15_10_IRQHandler(void);

const int debounceThreshold = 100;
volatile int lastInterruptTime = 0;
volatile int toggleLED = 0;

/**
  * @brief  Main program - flashes onboard LEDs, prints statement on button press
  * @param  None
  * @retval None
  */
int main(void) {

	BRD_init();			/* Initalise board */
	Hardware_init();	/* Initalise hardware modules */

	/* Main processing loop */
    while (1) {

    	/* Check if your LED toggle variable, set from the interrupt, is indicating to toggle */
    	if(toggleLED){

    		BRD_LEDRedToggle();
    		BRD_LEDGreenToggle();
    		BRD_LEDBlueToggle();

    		debug_printf("LED Toggled!");

    		toggleLED = 0;

    	}
	}
}

/**
  * @brief  Initialise Hardware
  * @param  None
  * @retval None
  */
void Hardware_init(void) {

	/* Initialise LEDs */
	BRD_LEDInit();

	/* Turn off LEDs */
	BRD_LEDRedOff();
	BRD_LEDGreenOff();
	BRD_LEDBlueOff();

	/*Initialise user push button*/
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
		toggleLED = 1;
	}

	lastInterruptTime = currentTime;

}


//Override default mapping of this handler to Default_Handler
void EXTI15_10_IRQHandler(void) {
	HAL_GPIO_EXTI_IRQHandler(BRD_USER_BUTTON_PIN);
}
