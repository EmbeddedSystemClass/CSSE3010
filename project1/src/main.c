/**
  ******************************************************************************
  * @file    project1/main.c
  * @author  SE
  * @date    21032018-18042018
  * @brief   Functionality for project 1
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "board.h"
#include "stm32f4xx_hal_conf.h"
#include "debug_printf.h"
#include "radio_fsm.h"
#include "nrf24l01plus.h"
#include "s4435360_hal_hamming.h"
#include "s4435360_hal_ir.h"
#include "s4435360_hal_lightbar.h"
#include "s4435360_hal_pantilt.h"
#include "s4435360_hal_radio.h"
#include "structures.h"
#include "idle_mode.h"
#include "pantilt_terminal_mode.h"
#include "pantilt_joystick_mode.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define HEARTBEAT_PERIOD 			20
#define HEARTBEAT_SEGMENT			0
#define MODE_ID_SEGMENT				1
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
ModeFunctions currentModeFunctions;
int heartbeatCounter = 0;
uint8_t lightbarValue = 0x00;
/* Private function prototypes -----------------------------------------------*/

int handle_mode_user_input(char input) {

	ModeFunctions nextModeFunctions;

	switch(input) {

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
			nextModeFunctions = idleModeFunctions;
			break;

		case IR_DUPLEX_CHAR:
			nextModeFunctions = idleModeFunctions;
			break;

		case RADIO_DUPLEX_CHAR:
			nextModeFunctions = idleModeFunctions;
			break;

		case INTEGRATION_CHAR:
			nextModeFunctions = idleModeFunctions;
			break;

		default:
			return UNHANDLED_USER_INPUT;
	}

	(*currentModeFunctions.deinit)();
	currentModeFunctions = nextModeFunctions;
	lightbarValue &= (0xFF & (currentModeFunctions.modeID << MODE_ID_SEGMENT));
	s4435360_lightbar_write(lightbarValue);
	(*currentModeFunctions.init)();

	return HANDLED_USER_INPUT;
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

	BRD_LEDInit();
	BRD_init();
	s4435360_hal_ir_init();
	s4435360_hal_joystick_init();
	s4435360_lightbar_init();
	s4435360_pantilt_init();
	//s4435360_hal_radio_init();

	currentModeFunctions = idleModeFunctions;

	char userInput;

	while(1) {

		userInput = debug_getc();

		if(userInput) {
			if(handle_mode_user_input(userInput) == UNHANDLED_USER_INPUT) {
				(*currentModeFunctions.userInput)(userInput);
			}
		}

		(*currentModeFunctions.run)();

		update_heartbeat();
		HAL_Delay(10);
	}
}
