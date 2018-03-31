/**
  ******************************************************************************
  * @file    project1/pantilt_terminal_mode.c
  * @author  SE
  * @date    21032018-18042018
  * @brief   Pantilt terminal mode functionality for project 1
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "pantilt_terminal_mode.h"

#include "s4435360_hal_pantilt.h"

TIM_HandleTypeDef printTimInit;

void pantilt_terminal_init(void) {
	debug_printf("Entered pantilt terminal mode\r\n");

	__TIM5_CLK_ENABLE();

	/* TIM Base configuration */
	printTimInit.Instance = TIM5;
	printTimInit.Init.Period = 50000 / 1; //1Hz interrupt frequency
	printTimInit.Init.Prescaler = (uint16_t) ((SystemCoreClock / 2) / 50000) - 1;
	printTimInit.Init.ClockDivision = 0;
	printTimInit.Init.RepetitionCounter = 0;
	printTimInit.Init.CounterMode = TIM_COUNTERMODE_UP;

	/* Initialise Timer */
	HAL_TIM_Base_Init(&printTimInit);
	HAL_NVIC_SetPriority(TIM5_IRQn, 10, 0);
	HAL_NVIC_EnableIRQ(TIM5_IRQn);
	HAL_TIM_Base_Start_IT(&printTimInit);

}


void pantilt_terminal_deinit(void) {
	HAL_TIM_Base_Stop_IT(&printTimInit);
	debug_printf("Exiting pantilt terminal mode\r\n");
}

void pantilt_terminal_run(void) {
	debug_printf("Running pantilt terminal mode\r\n");
	HAL_Delay(100);
}

void pantilt_terminal_user_input(char input) {

	switch(input) {
		case 'A':
			s4435360_hal_pantilt_pan_write(s4435360_hal_pantilt_pan_read() + 5);
			break;
		case 'D':
			s4435360_hal_pantilt_pan_write(s4435360_hal_pantilt_pan_read() - 5);
			break;
		case 'W':
			s4435360_hal_pantilt_tilt_write(s4435360_hal_pantilt_tilt_read() + 5);
			break;
		case 'S':
			s4435360_hal_pantilt_tilt_write(s4435360_hal_pantilt_tilt_read() - 5);
			break;
		default:
			break;
		}

	debug_printf("Handling input for pantilt terminal mode\r\n");
}
