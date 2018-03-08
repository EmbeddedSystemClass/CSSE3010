/**
 ******************************************************************************
 * @file    exercise3.c
 * @author  Sam Eadie
 * @date    07032018
 * @brief   Read ADC-x values
 ******************************************************************************
 *
 */

/* Includes ------------------------------------------------------------------*/
#include "board.h"
#include "stm32f4xx_hal_conf.h"
#include "debug_printf.h"
#include "s4435360_hal_pantilt.h"
#include "s4435360_hal_joystick.h"

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
int main(void) {

	BRD_init();
	s4435360_hal_joystick_init();
	s4435360_hal_pantilt_init();

	/* Infinite loop */
	while (1) {

		// Print ADC conversion values
		s4435360_hal_pantilt_pan_write((((s4435360_hal_joystick_x_read()/4096.0) * 170) - 85));
		s4435360_hal_pantilt_tilt_write((((s4435360_hal_joystick_y_read()/4096.0) * 170) - 85));

		HAL_Delay(100);
	}
}
