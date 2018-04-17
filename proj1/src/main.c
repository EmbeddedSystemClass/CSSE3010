/**
  ******************************************************************************
  * @file    project1/main.c
  * @author  SE
  * @date    21032018-18042018
  * @brief   Functionality for project 1
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "structures.h"
#include "s4435360_hal_ir.h"
#include "s4435360_hal_lightbar.h"
#include "s4435360_hal_pantilt.h"
#include "s4435360_hal_radio.h"
#include "s4435360_hal_joystick.h"
#include "idle_mode.h"
#include "pantilt_terminal_mode.h"
#include "pantilt_joystick_mode.h"
#include "encode_decode_mode.h"
#include "ir_duplex_mode.h"
#include "radio_duplex_mode.h"
#include "integration_duplex.h"
#include "integration_speed.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
int heartbeatCounter = 0;
int heartBeatValue = 0;

/* Functionality structs for modes */
ModeFunctions idleModeFunctions = {.modeID = 0x00,
			.init = &idle_init,
			.deinit = &idle_deinit,
			.run = &idle_run,
			.userInput = &idle_user_input,
			.timer1Handler = &idle_timer1_handler,
			.timer2Handler = &idle_timer2_handler,
			.timer3Handler = &idle_timer3_handler
};

ModeFunctions pantiltTerminalModeFunctions = {.modeID = 0x01,
		.init = &pantilt_terminal_init,
		.deinit = &pantilt_terminal_deinit,
		.run = &pantilt_terminal_run,
		.userInput = &pantilt_terminal_user_input,
		.timer1Handler = &pantilt_terminal_timer1_handler,
		.timer2Handler = &pantilt_terminal_timer2_handler,
		.timer3Handler = &pantilt_terminal_timer3_handler
};

ModeFunctions pantiltJoystickModeFunctions = {.modeID = 0x02,
		.init = &pantilt_joystick_init,
		.deinit = &pantilt_joystick_deinit,
		.run = &pantilt_joystick_run,
		.userInput = &pantilt_joystick_user_input,
		.timer1Handler = &pantilt_joystick_timer1_handler,
		.timer2Handler = &pantilt_joystick_timer2_handler,
		.timer3Handler = &pantilt_joystick_timer3_handler
};

ModeFunctions encodeDecodeModeFunctions = {.modeID = 0x03,
		.init = &encode_decode_init,
		.deinit = &encode_decode_deinit,
		.run = &encode_decode_run,
		.userInput = &encode_decode_user_input,
		.timer1Handler = &encode_decode_timer1_handler,
		.timer2Handler = &encode_decode_timer2_handler
};

ModeFunctions irDuplexModeFunctions = {.modeID = 0x04,
		.init = &ir_duplex_init,
		.deinit = &ir_duplex_deinit,
		.run = &ir_duplex_run,
		.userInput = &ir_duplex_user_input,
		.timer1Handler = &ir_duplex_timer1_handler,
		.timer2Handler = &ir_duplex_timer2_handler,
		.timer3Handler = &ir_duplex_timer3_handler
};

ModeFunctions radioDuplexModeFunctions = {.modeID = 0x05,
		.init = &radio_duplex_init,
		.deinit = &radio_duplex_deinit,
		.run = &radio_duplex_run,
		.userInput = &radio_duplex_user_input,
		.timer1Handler = &radio_duplex_timer1_handler,
		.timer2Handler = &radio_duplex_timer2_handler,
		.timer3Handler = &radio_duplex_timer3_handler
};

ModeFunctions integrationDuplexModeFunctions = {.modeID = 0x06,
		.init = &integration_duplex_init,
		.deinit = &integration_duplex_deinit,
		.run = &integration_duplex_run,
		.userInput = &integration_duplex_user_input,
		.timer1Handler = &integration_duplex_timer1_handler,
		.timer2Handler = &integration_duplex_timer2_handler,
		.timer3Handler = &integration_duplex_timer3_handler
};

ModeFunctions integrationSpeedModeFunctions = {.modeID = 0x07,
		.init = &integration_speed_init,
		.deinit = &integration_speed_deinit,
		.run = &integration_speed_run,
		.userInput = &integration_speed_user_input,
		.timer1Handler = &integration_speed_timer1_handler,
		.timer2Handler = &integration_speed_timer2_handler,
		.timer3Handler = &integration_speed_timer3_handler
};

/**
 * @brief Prints help information to terminal
 * @param None
 * @retval None
 */
void print_help_information(void) {
	debug_printf("CSSE3010 Project 1\r\n");
	debug_printf("1 Idle\r\n");
	debug_printf("2 P/T Terminal\r\n");
	debug_printf("3 P/T Joystick\r\n");
	debug_printf("4 Encode/Decode\r\n");
	debug_printf("5 IR Duplex\r\n");
	debug_printf("6 Radio Duplex\r\n");
	debug_printf("7 Integration Duplex\r\n");
	debug_printf("8 Integration Speed\r\n");
	debug_printf("\r\n");
}

/**
 * @brief Changes the current mode. Calls the previous mode's
 * 			deinit function, updates the lightbar, calls the
 * 			new mode's init function and sets the current mode
 * 			to the new mode's function struct
 * @param mode: the new mode
 * @retval None
 */
void change_mode(char mode) {

	ModeFunctions nextModeFunctions;

	/* Change to new mode */
	switch(mode) {

		case IDLE_CHAR:
			nextModeFunctions = idleModeFunctions;
			break;

		case PANTILT_TERMINAL_CHAR:
			nextModeFunctions = pantiltTerminalModeFunctions;
			break;

		case PANTILT_JOYSTICK_CHAR:
			nextModeFunctions = pantiltJoystickModeFunctions;
			break;

		case ENCODE_DECODE_CHAR:
			nextModeFunctions = encodeDecodeModeFunctions;
			break;

		case IR_DUPLEX_CHAR:
			nextModeFunctions = irDuplexModeFunctions;
			break;

		case RADIO_DUPLEX_CHAR:
			nextModeFunctions = radioDuplexModeFunctions;
			break;

		case INTEGRATION_DUPLEX_CHAR:
			nextModeFunctions = integrationDuplexModeFunctions;
			break;

		case INTEGRATION_SPEED_CHAR:
			nextModeFunctions = integrationSpeedModeFunctions;
			break;

		default:
			return;
	}

	//Deinitialises old mode
	(*currentModeFunctions.deinit)();

	currentModeFunctions = nextModeFunctions;

	//Updates the lighbar segment
	lightbar_seg_set(MODE_ID_SEGMENT, (currentModeFunctions.modeID & 0x01));
	lightbar_seg_set(MODE_ID_SEGMENT + 1, (currentModeFunctions.modeID & 0x02));
	lightbar_seg_set(MODE_ID_SEGMENT + 2, (currentModeFunctions.modeID & 0x04));

	//Initialises new mode
	(*currentModeFunctions.init)();
}

/**
 * @brief Updates the lightbar heart beat
 * @param None
 * @retval None
 */
void update_heartbeat(void) {
	if(heartbeatCounter >= HEARTBEAT_PERIOD) {
		lightbar_seg_set(HEARTBEAT_SEGMENT, heartBeatValue);
		heartBeatValue = 1 - heartBeatValue;
		heartbeatCounter = 0;
	} else {
		heartbeatCounter++;
	}
}

void main(void) {

	/* Initialise hardware */
	BRD_init();
	BRD_LEDInit();
	s4435360_hal_ir_init();
	s4435360_hal_joystick_init();
	s4435360_lightbar_init();
	s4435360_hal_pantilt_init();
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
	s4435360_radio_init();

	currentModeFunctions = idleModeFunctions;
	s4435360_lightbar_write(0x00);
	heartBeatValue = 0;

	char userInput;
	char userChars[13];
	int userCharsReceived = 0;
	int passCharsToMode = 0;
	setbuf(stdout, NULL);

	while(1) {

		userInput = debug_getc();

		//Handle user input
		if(userInput) {
			debug_printf("%c", userInput);

			switch(userInput) {

				//Passes char to current mode to handle
				case ENTER_CHAR:
					passCharsToMode = 1;
					debug_printf("\r");
					break;

				//Removes the last entered character
				case BACKSPACE_CHAR:
					if(userCharsReceived) {
						userCharsReceived--;
					}
					break;

				//Prints help information
				case QUESTION_CHAR:
					print_help_information();
					break;

				default:

					//Check for valid input
					if(((userInput >= '0') && (userInput <= '9')) ||
										((userInput >= 'A') && (userInput <= 'Z')) ||
										((userInput >= 'a') && (userInput <= 'z')) ||
										(userInput == SPACE_CHAR)) {
							userChars[userCharsReceived] = userInput;
							userCharsReceived++;
					}

					//If buffer full
					if(userCharsReceived >= MAX_USER_CHARS) {
						passCharsToMode = 1;
					}
					break;
			}
		}

		//Buffer set to pass to current mode
		if(passCharsToMode) {

			//Don't pass in buffer if its a change of mode command
			if((userChars[0] >= '0') && (userChars[0] <= '9')) {
				change_mode(userChars[0]);
				userChars[0] = 0;

			//Pass in buffer to mode to handle
			} else {
				(*currentModeFunctions.userInput)(userChars, userCharsReceived);
			}

			//Reset buffer
			userCharsReceived = 0;
			passCharsToMode = 0;
		}

		//Run mode
		(*currentModeFunctions.run)();

		update_heartbeat();
		HAL_Delay(100);
	}
}

/**
 * @brief Handles generic timer capability for modes
 * @param htim: the elapsed timer
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if(htim->Instance == TIMER1) {
		(*currentModeFunctions.timer1Handler)();
	} else if (htim->Instance == TIMER2) {
		(*currentModeFunctions.timer2Handler)();
	} else if (htim->Instance == TIMER3) {
		(*currentModeFunctions.timer3Handler)();
	}
}

/**
 * @brief Handler for generic timer 1
 * @param None
 * @retval None
 */
void TIMER1_HANDLER(void) {
	HAL_TIM_IRQHandler(&timer1Init);
}

/**
 * @brief Handler for generic timer 2
 * @param None
 * @retval None
 */
void TIMER2_HANDLER(void) {
	HAL_TIM_IRQHandler(&timer2Init);
}

/**
 * @brief Handler for generic timer 3
 * @param None
 * @retval None
 */
void TIMER3_HANDLER(void) {
	HAL_TIM_IRQHandler(&timer3Init);
}
