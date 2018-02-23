/**
 ******************************************************************************
 * @file    mylib/sxxxxxxx_ledbar.c
 * @author  MyName – MyStudent ID
 * @date    17022018
 * @brief   LED Light Bar peripheral driver
 *	     REFERENCE: LEDLightBar_datasheet.pdf
 *
 ******************************************************************************
 *     EXTERNAL FUNCTIONS
 ******************************************************************************
 * sxxxxxx_lightbar_init() – intialise LED Light BAR
 * sxxxxxx_lightbar_write() – set LED Light BAR value
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <s4435360_hal_lightbar.h>

//#include "sideboard.h"
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
const int numberLEDs = 10;


void lightbar_seg_set(int segment, unsigned char segment_value) {

	switch(segment) {
		case 0:

			HAL_GPIO_WritePin(BRD_D16_GPIO_PORT, BRD_D16_PIN, segment_value);
			break;

		case 1:

			HAL_GPIO_WritePin(BRD_D17_GPIO_PORT, BRD_D17_PIN, segment_value);
			break;

		case 2:

			HAL_GPIO_WritePin(BRD_D18_GPIO_PORT, BRD_D18_PIN, segment_value);
			break;

		case 3:

			HAL_GPIO_WritePin(BRD_D19_GPIO_PORT, BRD_D19_PIN, segment_value);
			break;

		case 4:

			HAL_GPIO_WritePin(BRD_D20_GPIO_PORT, BRD_D20_PIN, segment_value);
			break;

		case 5:

			HAL_GPIO_WritePin(BRD_D21_GPIO_PORT, BRD_D21_PIN, segment_value);
			break;

		case 6:

			HAL_GPIO_WritePin(BRD_D22_GPIO_PORT, BRD_D22_PIN, segment_value);
			break;

		case 7:

			HAL_GPIO_WritePin(BRD_D23_GPIO_PORT, BRD_D23_PIN, segment_value);
			break;

		case 8:

			HAL_GPIO_WritePin(BRD_D24_GPIO_PORT, BRD_D24_PIN, segment_value);
			break;

		case 9:

			HAL_GPIO_WritePin(BRD_D25_GPIO_PORT, BRD_D25_PIN, segment_value);
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
extern void s4435360_lightbar_init(void) {

	GPIO_InitTypeDef GPIO_Init;

	__GPIOA_CLK_ENABLE();
	__GPIOC_CLK_ENABLE();
	__GPIOD_CLK_ENABLE();

	/* Configure pins D0 to D9 as output*/
	GPIO_Init.Pin = BRD_D16_PIN;
	GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_Init.Pull = GPIO_PULLUP;
	GPIO_Init.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(BRD_D16_GPIO_PORT, &GPIO_Init);

	GPIO_Init.Pin = BRD_D17_PIN;
	GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_Init.Pull = GPIO_PULLUP;
	GPIO_Init.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(BRD_D17_GPIO_PORT, &GPIO_Init);

	GPIO_Init.Pin = BRD_D18_PIN;
	GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_Init.Pull = GPIO_PULLUP;
	GPIO_Init.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(BRD_D18_GPIO_PORT, &GPIO_Init);

	GPIO_Init.Pin = BRD_D19_PIN;
	GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_Init.Pull = GPIO_PULLUP;
	GPIO_Init.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(BRD_D19_GPIO_PORT, &GPIO_Init);

	GPIO_Init.Pin = BRD_D20_PIN;
	GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_Init.Pull = GPIO_PULLUP;
	GPIO_Init.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(BRD_D20_GPIO_PORT, &GPIO_Init);

	GPIO_Init.Pin = BRD_D21_PIN;
	GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_Init.Pull = GPIO_PULLUP;
	GPIO_Init.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(BRD_D21_GPIO_PORT, &GPIO_Init);

	GPIO_Init.Pin = BRD_D22_PIN;
	GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_Init.Pull = GPIO_PULLUP;
	GPIO_Init.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(BRD_D22_GPIO_PORT, &GPIO_Init);

	GPIO_Init.Pin = BRD_D23_PIN;
	GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_Init.Pull = GPIO_PULLUP;
	GPIO_Init.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(BRD_D23_GPIO_PORT, &GPIO_Init);

	GPIO_Init.Pin = BRD_D24_PIN;
	GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_Init.Pull = GPIO_PULLUP;
	GPIO_Init.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(BRD_D24_GPIO_PORT, &GPIO_Init);

	GPIO_Init.Pin = BRD_D25_PIN;
	GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_Init.Pull = GPIO_PULLUP;
	GPIO_Init.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(BRD_D25_GPIO_PORT, &GPIO_Init);
}

/**
 * @brief Deinitialise the LEDBar
 * @param
 * @retval
 */
extern void s4435360_lightbar_deinit(void) {

	HAL_GPIO_DeInit(BRD_D16_GPIO_PORT, 16);
	HAL_GPIO_DeInit(BRD_D17_GPIO_PORT, 17);
	HAL_GPIO_DeInit(BRD_D18_GPIO_PORT, 18);
	HAL_GPIO_DeInit(BRD_D19_GPIO_PORT, 19);
	HAL_GPIO_DeInit(BRD_D20_GPIO_PORT, 20);
	HAL_GPIO_DeInit(BRD_D21_GPIO_PORT, 21);
	HAL_GPIO_DeInit(BRD_D22_GPIO_PORT, 22);
	HAL_GPIO_DeInit(BRD_D23_GPIO_PORT, 23);
	HAL_GPIO_DeInit(BRD_D24_GPIO_PORT, 24);
	HAL_GPIO_DeInit(BRD_D25_GPIO_PORT, 25);

}

/**
  * @brief  Set the LED Bar GPIO pins high or low, depending on the bit of ‘value’
  *         i.e. value bit 0 is 1 – LED Bar 0 on
  *          value bit 1 is 1 – LED BAR 1 on
  *
  * @param  value
  * @retval None
  */
extern void s4435360_lightbar_write(unsigned short value) {

	int segmentValue;

	for(int segment = 0; segment < numberLEDs; segment++) {
		segmentValue = value & 0x01; //Isolate last bit
		lightbar_seg_set(segment, segmentValue); //Set last LED as last bit
		value >>= 1; //Shift new last bit
	}
}

int main(void) {
	BRD_init();			//Initialise NP2 board.
	s4435360_lightbar_init();

	while(1) {
		s4435360_lightbar_write((unsigned short) 0b0101010101);
		HAL_Delay(1000);
		s4435360_lightbar_write((unsigned short) 0b1010101010);
		HAL_Delay(1000);
	}
}

