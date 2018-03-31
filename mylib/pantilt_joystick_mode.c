/**
  ******************************************************************************
  * @file    project1/pantilt_joystick_mode.c
  * @author  SE
  * @date    21032018-18042018
  * @brief   Pantilt joystick mode functionality for project 1
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "pantilt_joystick_mode.h"

#include "s4435360_hal_pantilt.h"
#include "s4435360_hal_joystick.h"

TIM_HandleTypeDef printTimInit;

void pantilt_joystick_init(void) {
	debug_printf("Entered pantilt joystick mode\r\n");

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

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	debug_printf("Pan:  %d Tilt:  %d\r\n",
			s4435360_hal_pantilt_pan_read(),
			s4435360_hal_pantilt_tilt_read());
}

void TIM5_IRQHandler(void) {
	HAL_TIM_IRQHandler(&printTimInit);
}

void pantilt_joystick_deinit(void) {
	debug_printf("Exiting pantilt joystick mode\r\n");
	HAL_TIM_Base_Stop_IT(&printTimInit);
}

void pantilt_joystick_run(void) {
	debug_printf("Running pantilt joystick mode\r\n");

	//Update pantilt with joystick readings
	s4435360_hal_pantilt_pan_write((((s4435360_hal_joystick_x_read()/4096.0) * 170) - 85));
	s4435360_hal_pantilt_tilt_write((((s4435360_hal_joystick_y_read()/4096.0) * 170) - 85));

}

void pantilt_joystick_user_input(char input) {
	debug_printf("Handling input for pantilt joystick mode\r\n");
}
