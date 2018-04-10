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
#include "hamming_encode_decode_mode.h"
#include "ir_duplex_mode.h"
#include "radio_duplex_mode.h"
#include "integration_duplex.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
int heartbeatCounter = 0;
uint8_t lightbarValue = 0x00;
char userChars[13];
int userCharsReceived = 0;
int passCharsToMode = 0;


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

ModeFunctions hammingEncodeDecodeModeFunctions = {.modeID = 0x05,
		.init = &hamming_encode_decode_init,
		.deinit = &hamming_encode_decode_deinit,
		.run = &hamming_encode_decode_run,
		.userInput = &hamming_encode_decode_user_input,
		.timer1Handler = &hamming_encode_decode_timer1_handler,
		.timer2Handler = &hamming_encode_decode_timer2_handler,
		.timer3Handler = &hamming_encode_decode_timer3_handler
};

ModeFunctions radioDuplexModeFunctions = {.modeID = 0x06,
		.init = &radio_duplex_init,
		.deinit = &radio_duplex_deinit,
		.run = &radio_duplex_run,
		.userInput = &radio_duplex_user_input,
		.timer1Handler = &radio_duplex_timer1_handler,
		.timer2Handler = &radio_duplex_timer2_handler,
		.timer3Handler = &radio_duplex_timer3_handler
};

ModeFunctions integrationDuplexModeFunctions = {.modeID = 0x07,
		.init = &integration_duplex_init,
		.deinit = &integration_duplex_deinit,
		.run = &integration_duplex_run,
		.userInput = &integration_duplex_user_input,
		.timer1Handler = &integration_duplex_timer1_handler,
		.timer2Handler = &integration_duplex_timer2_handler,
		.timer3Handler = &integration_duplex_timer3_handler
};



/* Private function prototypes -----------------------------------------------*/

void print_help_information(void) {
	debug_printf("CSSE3010 Project 1\r\n");
	debug_printf("1 Idle\r\n");
	debug_printf("2 P/T Terminal\r\n");
	debug_printf("3 P/T Joystick\r\n");
	debug_printf("4 Encode/Decode\r\n");
	debug_printf("5 IR Duplex\r\n");
	debug_printf("6 Hamming Encode/Decode\r\n");
	debug_printf("7 Radio Duplex\r\n");
	debug_printf("8 Integration Duplex\r\n");
	debug_printf("9 Integration Speed\r\n");
	debug_printf("\r\n");
}

void change_mode(char mode) {

	ModeFunctions nextModeFunctions;

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

		case HAMMING_ENCODE_DECODE_CHAR:
			nextModeFunctions = hammingEncodeDecodeModeFunctions;
			break;

		case RADIO_DUPLEX_CHAR:
			nextModeFunctions = radioDuplexModeFunctions;
			break;

		case INTEGRATION_DUPLEX_CHAR:
			nextModeFunctions = integrationDuplexModeFunctions;
			break;

		case INTEGRATION_SPEED_CHAR:
			nextModeFunctions = idleModeFunctions;
			break;

		default:
			return;
	}

	(*currentModeFunctions.deinit)();
	currentModeFunctions = nextModeFunctions;
	lightbarValue &= (0xFF & (currentModeFunctions.modeID << MODE_ID_SEGMENT));
	s4435360_lightbar_write(lightbarValue);
	(*currentModeFunctions.init)();
}

void update_heartbeat(void) {
	if(heartbeatCounter >= HEARTBEAT_PERIOD) {
		lightbarValue ^= (1 << HEARTBEAT_SEGMENT);
		s4435360_lightbar_write(lightbarValue);
		heartbeatCounter = 0;
	} else {
		heartbeatCounter++;
	}
}

void main(void) {

	BRD_init();
	BRD_LEDInit();
	s4435360_hal_ir_init();
	s4435360_hal_joystick_init();
	s4435360_lightbar_init();
	s4435360_hal_pantilt_init();
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
	s4435360_radio_init();

	currentModeFunctions = idleModeFunctions;
	char userInput;

	while(1) {

		userInput = debug_getc();

		if(userInput) {
			debug_printf("%c", userInput);

			switch(userInput) {
				case ENTER_CHAR:
					passCharsToMode = 1;
					debug_printf("\r");
					break;

				case BACKSPACE_CHAR:
					if(userCharsReceived) {
						userCharsReceived--;
					}
					break;

				case QUESTION_CHAR:
					print_help_information();
					break;

				default:
					if(((userInput >= '0') && (userInput <= '9')) ||
										((userInput >= 'A') && (userInput <= 'Z')) ||
										((userInput >= 'a') && (userInput <= 'z')) ||
										(userInput == SPACE_CHAR)) {
							userChars[userCharsReceived] = userInput;
							userCharsReceived++;
					}

					if(userCharsReceived >= MAX_USER_CHARS) {
						passCharsToMode = 1;
					}
					break;
			}
		}


		if(passCharsToMode) {
			if((userChars[0] >= '0') && (userChars[0] <= '9')) {
				change_mode(userChars[0]);
			} else {
				(*currentModeFunctions.userInput)(userChars, userCharsReceived);
			}
			userCharsReceived = 0;
			passCharsToMode = 0;
		}

		(*currentModeFunctions.run)();

		update_heartbeat();
		HAL_Delay(100);
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if(htim->Instance == TIMER1) {
		(*currentModeFunctions.timer1Handler)();
	} else if (htim->Instance == TIMER2) {
		(*currentModeFunctions.timer2Handler)();
	} else if (htim->Instance == TIMER3) {
		(*currentModeFunctions.timer3Handler)();
	}
}

void TIMER1_HANDLER(void) {
	HAL_TIM_IRQHandler(&timer1Init);
}

void TIMER2_HANDLER(void) {
	HAL_TIM_IRQHandler(&timer2Init);
}

void TIMER3_HANDLER(void) {
	HAL_TIM_IRQHandler(&timer3Init);
}





