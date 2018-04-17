/**
  ******************************************************************************
  * @file    proj1/pantilt_joystick_mode.c
  * @author  Samuel Eadie - 44353607
  * @date    21032018-18042018
  * @brief   Provides pantilt joystick mode functionality for project 1
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "structures.h"
#include "pantilt_joystick_mode.h"
#include "s4435360_hal_pantilt.h"
#include "s4435360_hal_joystick.h"

#define DENOISING_TOLERANCE 5

int joystickPanAngle = 0;
int joystickTiltAngle = 0;

//Print flag
int anglePrintingFlag = 0;

/**
  * @brief Initialises pantilt joystick mode
  * @param None
  * @retval None
  */
void pantilt_joystick_init(void) {
	debug_printf("Mode 3: Pantilt joystick\r\n");

	//1s printing timer
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

	joystickPanAngle = 0;
	joystickTiltAngle = 0;

}

/**
  * @brief Deinitialises pantilt joystick mode
  * @param None
  * @retval None
  */
void pantilt_joystick_deinit(void) {
	HAL_TIM_Base_Stop_IT(&timer1Init);
	s4435360_hal_pantilt_tilt_write(0);
	s4435360_hal_pantilt_pan_write(0);
}

/**
  * @brief Run functionality for pantilt joystick mode
  * @param None
  * @retval None
  */
void pantilt_joystick_run(void) {

	int newPanAngle = ((s4435360_hal_joystick_x_read()/4096.0) * 170) - 85;
	int newTiltAngle = ((s4435360_hal_joystick_y_read()/4096.0) * 170) - 80;

	//Remove noise
	if((newPanAngle < DENOISING_TOLERANCE) &&
			(newPanAngle > -1 * DENOISING_TOLERANCE)) {
		newPanAngle = 0;

	} else if((newTiltAngle < DENOISING_TOLERANCE) &&
			(newTiltAngle > -1 * DENOISING_TOLERANCE)) {
		newTiltAngle = 0;
	}

	//s4435360_hal_pantilt_pan_write(-1 * newPanAngle);
	//s4435360_hal_pantilt_tilt_write(newTiltAngle);

	for(int i = 0; i < 10; i++) {
		s4435360_hal_pantilt_tilt_write((((newTiltAngle - joystickTiltAngle) * i) / 10) + joystickTiltAngle);
		s4435360_hal_pantilt_pan_write(-1 * ((((newPanAngle - joystickPanAngle) * i) / 10) + joystickPanAngle));
		HAL_Delay(10);
	}

	joystickPanAngle = newPanAngle;
	joystickTiltAngle = newTiltAngle;


	//Print current angles
	if(anglePrintingFlag) {
		debug_printf("Pan:  %d Tilt:  %d\r\n",
				-1 * joystickPanAngle,
				joystickTiltAngle);
		anglePrintingFlag = 0;
	}
}

/**
  * @brief Handles user input for pantilt joystick mode
  * @param userChars: chars received from console
  * 	   userCharsReceived: number of chars received
  * @retval None
  */
void pantilt_joystick_user_input(char* userChars, int userCharsReceived) {}

/**
  * @brief Sets the printing flag every second
  * @param None
  * @retval None
  */
void pantilt_joystick_timer1_handler(void) {
	anglePrintingFlag = 1;
}

/* Unused timer functionality */
void pantilt_joystick_timer2_handler(void) {}
void pantilt_joystick_timer3_handler(void) {}
