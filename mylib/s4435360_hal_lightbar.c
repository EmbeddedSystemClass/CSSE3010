/**
 ******************************************************************************
 * @file    mylib/s4435360_hal_lightbar.c
 * @author  Samuel Eadie - 44353607
 * @date    22022018-28022018
 * @brief   LED Light Bar peripheral driver
 *	     REFERENCE: LEDLightBar_datasheet.pdf
 *
 ******************************************************************************
 *     EXTERNAL FUNCTIONS
 ******************************************************************************
 * s4435360_lightbar_init() – intialise LED Light BAR
 * s4435360_lightbar_write() – set LED Light BAR value
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <s4435360_hal_lightbar.h>

#include "stm32f4xx_hal_conf.h"
#include "debug_printf.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx.h"
#include "board.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
const int numberLEDs = 10; //Number of LEDs on lightbar
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
 * @brief	Writes the specified segment_value
 * 			to the specified segment
 * @param	segment, segment_value
 * @retval 	None
 */
void lightbar_seg_set(int segment, unsigned char segment_value) {

	/* Write value to appropriate pin */
	switch(segment) {
		case 0:

			HAL_GPIO_WritePin(LIGHTBAR_PIN0_PORT, LIGHTBAR_PIN0, segment_value);
			break;

		case 1:

			HAL_GPIO_WritePin(LIGHTBAR_PIN1_PORT, LIGHTBAR_PIN1, segment_value);
			break;

		case 2:

			HAL_GPIO_WritePin(LIGHTBAR_PIN2_PORT, LIGHTBAR_PIN2, segment_value);
			break;

		case 3:

			HAL_GPIO_WritePin(LIGHTBAR_PIN3_PORT, LIGHTBAR_PIN3, segment_value);
			break;

		case 4:

			HAL_GPIO_WritePin(LIGHTBAR_PIN4_PORT, LIGHTBAR_PIN4, segment_value);
			break;

		case 5:

			HAL_GPIO_WritePin(LIGHTBAR_PIN5_PORT, LIGHTBAR_PIN5, segment_value);
			break;

		case 6:

			HAL_GPIO_WritePin(LIGHTBAR_PIN6_PORT, LIGHTBAR_PIN6, segment_value);
			break;

		case 7:

			HAL_GPIO_WritePin(LIGHTBAR_PIN7_PORT, LIGHTBAR_PIN7, segment_value);
			break;

		case 8:

			HAL_GPIO_WritePin(LIGHTBAR_PIN8_PORT, LIGHTBAR_PIN8, segment_value);
			break;

		case 9:

			HAL_GPIO_WritePin(LIGHTBAR_PIN9_PORT, LIGHTBAR_PIN9, segment_value);
			break;

		default:
			break;

	}
}

/**
  * @brief  Initialise LEDBar.
  * @param  None
  * @retval None
  */
void s4435360_lightbar_init(void) {

	GPIO_InitTypeDef GPIO_Init;

	__GPIOC_CLK_ENABLE();
	__GPIOD_CLK_ENABLE();
	__GPIOG_CLK_ENABLE();

	GPIO_Init.Pin = LIGHTBAR_PIN0;
	GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_Init.Pull = GPIO_PULLUP;
	GPIO_Init.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(LIGHTBAR_PIN0_PORT, &GPIO_Init);

	GPIO_Init.Pin = LIGHTBAR_PIN1;
	GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_Init.Pull = GPIO_PULLUP;
	GPIO_Init.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(LIGHTBAR_PIN1_PORT, &GPIO_Init);

	GPIO_Init.Pin = LIGHTBAR_PIN2;
	GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_Init.Pull = GPIO_PULLUP;
	GPIO_Init.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(LIGHTBAR_PIN2_PORT, &GPIO_Init);

	GPIO_Init.Pin = LIGHTBAR_PIN3;
	GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_Init.Pull = GPIO_PULLUP;
	GPIO_Init.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(LIGHTBAR_PIN3_PORT, &GPIO_Init);

	GPIO_Init.Pin = LIGHTBAR_PIN4;
	GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_Init.Pull = GPIO_PULLUP;
	GPIO_Init.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(LIGHTBAR_PIN4_PORT, &GPIO_Init);

	GPIO_Init.Pin = LIGHTBAR_PIN5;
	GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_Init.Pull = GPIO_PULLUP;
	GPIO_Init.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(LIGHTBAR_PIN5_PORT, &GPIO_Init);

	GPIO_Init.Pin = LIGHTBAR_PIN6;
	GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_Init.Pull = GPIO_PULLUP;
	GPIO_Init.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(LIGHTBAR_PIN6_PORT, &GPIO_Init);

	GPIO_Init.Pin = LIGHTBAR_PIN7;
	GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_Init.Pull = GPIO_PULLUP;
	GPIO_Init.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(LIGHTBAR_PIN7_PORT, &GPIO_Init);

	GPIO_Init.Pin = LIGHTBAR_PIN8;
	GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_Init.Pull = GPIO_PULLUP;
	GPIO_Init.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(LIGHTBAR_PIN8_PORT, &GPIO_Init);

	GPIO_Init.Pin = LIGHTBAR_PIN9;
	GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_Init.Pull = GPIO_PULLUP;
	GPIO_Init.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(LIGHTBAR_PIN9_PORT, &GPIO_Init);

}

/**
  * @brief  Set the LED Bar GPIO pins high or low, depending on
  * 		the bit of ‘value’: i.e. value bit 0 is 1 – LED Bar
  * 		0 on value bit 1 is 1 – LED BAR 1 on
  *
  * @param  value
  * @retval None
  */
void s4435360_lightbar_write(unsigned short value) {

	int segmentValue;

	/* Write each value */
	for(int segment = 0; segment < numberLEDs; segment++) {

		segmentValue = value & 0x01; //Isolate last bit
		lightbar_seg_set(segment, segmentValue); //Set last LED as last bit
		value >>= 1; //Shift new last bit

	}
}
