/**
  ******************************************************************************
  * @file    proj1/pantilt_terminal_mode.c
  * @author  Samuel Eadie - 44353607
  * @date    21032018-18042018
  * @brief   Pantilt terminal mode functionality for project 1
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "structures.h"
#include "pantilt_terminal_mode.h"
#include "s4435360_hal_pantilt.h"

//Flag for printing, set in 1s interrupt
int printFlag = 0;

//Current pan and tilt angles
int panAngle = 0, tiltAngle = 0;

/**
 * @brief Initialises the pantilt terminal mode
 * @param None
 * @retval None
 */
void pantilt_terminal_init(void) {
	debug_printf("Mode 2: Pantilt Terminal\r\n");

	//Initialises 1s printing timer
	__TIMER1_CLK_ENABLE();

	/* TIM Base configuration */
	timer1Init.Instance = TIMER1;
	timer1Init.Init.Period = 50000 / 1; //1Hz interrupt frequency
	timer1Init.Init.Prescaler = (uint16_t) ((SystemCoreClock) / 50000) - 1;
	timer1Init.Init.ClockDivision = 0;
	timer1Init.Init.RepetitionCounter = 0;
	timer1Init.Init.CounterMode = TIM_COUNTERMODE_UP;

	/* Initialise Timer */
	HAL_TIM_Base_Init(&timer1Init);
	HAL_NVIC_SetPriority(TIMER1_IRQ, 10, 0);
	HAL_NVIC_EnableIRQ(TIMER1_IRQ);
	HAL_TIM_Base_Start_IT(&timer1Init);

	//Initialises pan tilt angles to 0
	panAngle = 0;
	tiltAngle = 0;

}

/**
 * @brief Deinitialises the pantilt terminal mode
 * @param None
 * @retval None
 */
void pantilt_terminal_deinit(void) {
	HAL_TIM_Base_Stop_IT(&timer1Init);
	s4435360_hal_pantilt_tilt_write(0);
	s4435360_hal_pantilt_pan_write(0);
}

/**
 * @brief Run functionality for pantilt terminal mode
 * @param None
 * @retval None
 */
void pantilt_terminal_run(void) {

	//Print current angles every 1s
	if(printFlag) {
		debug_printf("Pan:  %d Tilt:  %d\r\n",
					panAngle, tiltAngle);
		printFlag = 0;
	}
}

/**
 * @brief Handles user input from terminal console
 * @param userChars: the chars received from the console
 * 		  userCharsReceived: the number of chars received from the console
 * @retval None
 */
void pantilt_terminal_user_input(char* userChars, int userCharsReceived) {

	int newPanAngle = panAngle;
	int newTiltAngle = tiltAngle;

	/* Process all valid WASD in user input */
	for(int i = 0; i < userCharsReceived; i++) {
		switch(userChars[i]) {

			case 'A':
				newPanAngle += 5;
				break;

			case 'D':
				newPanAngle -= 5;
				break;

			case 'W':
				newTiltAngle += 5;
				break;

			case 'S':
				newTiltAngle -= 5;
				break;

			default:
				break;
		}
	}

	//Confine angle to +-85
	newTiltAngle = newTiltAngle > 85 ? 85 : newTiltAngle;
	newTiltAngle = newTiltAngle < -85 ? -85 : newTiltAngle;

	newPanAngle = newPanAngle > 85 ? 85 : newPanAngle;
	newPanAngle = newPanAngle < -85 ? -85 : newPanAngle;

	for(int i = 0; i < 10; i++) {
		s4435360_hal_pantilt_tilt_write((((newTiltAngle - tiltAngle) * i) / 10) + tiltAngle);
		s4435360_hal_pantilt_pan_write((((newPanAngle - panAngle) * i) / 10) + panAngle);
		HAL_Delay(10);
	}

	//Write net change to pantilt
	//s4435360_hal_pantilt_tilt_write(tiltAngle);
	//s4435360_hal_pantilt_pan_write(panAngle);

	tiltAngle = newTiltAngle;
	panAngle = newPanAngle;
}

/**
 * @brief Sets the timer flag for printing
 * @param None
 * @retval None
 */
void pantilt_terminal_timer1_handler(void) {
	printFlag = 1;
}

void pantilt_terminal_timer2_handler(void) {}
void pantilt_terminal_timer3_handler(void) {}
