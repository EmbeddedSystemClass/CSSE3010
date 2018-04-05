/**
  ******************************************************************************
  * @file    project1/pantilt_joystick_mode.c
  * @author  SE
  * @date    21032018-18042018
  * @brief   Pantilt joystick mode functionality for project 1
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "structures.h"
#include "pantilt_joystick_mode.h"
#include "s4435360_hal_pantilt.h"
#include "s4435360_hal_joystick.h"

void pantilt_joystick_init(void) {
	debug_printf("Entered pantilt joystick mode\r\n");

	__TIMER1_CLK_ENABLE();

	/* TIM Base configuration */
	timer1Init.Instance = TIMER1;
	timer1Init.Init.Period = 50000 / 1; //1Hz interrupt frequency
	timer1Init.Init.Prescaler = (uint16_t) ((SystemCoreClock / 2) / 50000) - 1;
	timer1Init.Init.ClockDivision = 0;
	timer1Init.Init.RepetitionCounter = 0;
	timer1Init.Init.CounterMode = TIM_COUNTERMODE_UP;

	/* Initialise Timer */
	HAL_TIM_Base_Init(&timer1Init);
	HAL_NVIC_SetPriority(TIMER1_IRQ, 10, 0);
	HAL_NVIC_EnableIRQ(TIMER1_IRQ);
	HAL_TIM_Base_Start_IT(&timer1Init);

}

void pantilt_joystick_deinit(void) {
	debug_printf("Exiting pantilt joystick mode\r\n");
	HAL_TIM_Base_Stop_IT(&timer1Init);
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

void pantilt_joystick_timer1_handler(void) {
	debug_printf("Pan:  %d Tilt:  %d\r\n",
			s4435360_hal_pantilt_pan_read(),
			s4435360_hal_pantilt_tilt_read());
}

void pantilt_joystick_timer2_handler(void) {

}
