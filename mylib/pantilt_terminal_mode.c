/**
  ******************************************************************************
  * @file    project1/pantilt_terminal_mode.c
  * @author  SE
  * @date    21032018-18042018
  * @brief   Pantilt terminal mode functionality for project 1
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "structures.h"
#include "pantilt_terminal_mode.h"
#include "s4435360_hal_pantilt.h"

void pantilt_terminal_init(void) {
	debug_printf("Pantilt terminal mode\r\n");

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


void pantilt_terminal_deinit(void) {
	HAL_TIM_Base_Stop_IT(&timer1Init);
	debug_printf("Exiting pantilt terminal mode\r\n");
}

void pantilt_terminal_run(void) {
	debug_printf("Running pantilt terminal mode\r\n");
	HAL_Delay(100);
}

void pantilt_terminal_user_input(char* userChars, int userCharsReceived) {
	for(int i = 0; i < userCharsReceived; i++) {
		switch(userChars[i]) {
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
	}
}

void pantilt_terminal_timer1_handler(void) {
	debug_printf("Pan:  %d Tilt:  %d\r\n",
			s4435360_hal_pantilt_pan_read(),
			s4435360_hal_pantilt_tilt_read());
}

void pantilt_terminal_timer2_handler(void) {}
