/**
 ******************************************************************************
 * @file    demo1.c
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

/* Includes ------------------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Main program
 * @param  None
 * @retval None
 */
#define SHIFT_RIGHT(value) value = value << 1;
#define SHIFT_LEFT(value) value = value >> 1;

int main(void) {

	//Init hardware
	BRD_init();
	s4435360_lightbar_init();


	int isShiftingRight = 1;
	int rightEdge = 768;
	int leftEdge = 0x03;

	unsigned short displayValue = leftEdge;

	s4435360_lightbar_write(0xFFFF);
	HAL_Delay(1000);
	s4435360_lightbar_write(0x00);

	s4435360_lightbar_write(displayValue);

	while(1) {
		HAL_Delay(1000);

		if(isShiftingRight) {
			SHIFT_RIGHT(displayValue);
		} else {
			SHIFT_LEFT(displayValue);
		}

		s4435360_lightbar_write(displayValue);

		if((displayValue == leftEdge) || (displayValue == rightEdge)) {
			isShiftingRight = 1 - isShiftingRight;
		}

	}
}
