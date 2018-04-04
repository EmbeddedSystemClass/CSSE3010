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
#include "s4435360_hal_hamming.h"
#include "s4435360_hal_ir.h"
#include "s4435360_hal_lightbar.h"
#include "s4435360_hal_pantilt.h"
#include "s4435360_hal_radio.h"
#include "s4435360_hal_joystick.h"
#include "s4435360_hal_pantilt.h"
#include "pantilt_terminal_mode.h"
#include "pantilt_joystick_mode.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define ESCAPE_CHAR 			(char)(27)
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
int heartbeatCounter = 0;
uint8_t lightbarValue = 0x00;

ModeFunctions pantiltTerminalModeFunctions = {.modeID = 0x01,
												.init = &pantilt_terminal_init,
												.deinit = &pantilt_terminal_deinit,
												.run = &pantilt_terminal_run,
												.userInput = &pantilt_terminal_user_input};

ModeFunctions pantiltJoystickModeFunctions = {.modeID = 0x02,
												.init = &pantilt_joystick_init,
												.deinit = &pantilt_joystick_deinit,
												.run = &pantilt_joystick_run,
												.userInput = &pantilt_joystick_user_input};

void idle_init(void);
void idle_deinit(void);
void idle_run(void);
void idle_user_input(char input);

ModeFunctions idleModeFunctions = {.modeID = 0x00,
			.init = &idle_init,
			.deinit = &idle_deinit,
			.run = &idle_run,
			.userInput = &idle_user_input};

/* Private function prototypes -----------------------------------------------*/

///////////////////////////////IDLE_MODE////////////////////////////////////////
void idle_init(void) {
	debug_printf("Entered idle mode\r\n");
}

void idle_deinit(void) {
	debug_printf("Exiting idle mode\r\n");
}

void idle_run(void) {
	debug_printf("Running idle mode\r\n");
	HAL_Delay(100);
}

void idle_user_input(char input) {

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

		case HELP_CHAR:
			debug_printf("CSSE3010 Project 1\r\n");
			debug_printf("1 Idle\r\n");
			debug_printf("2 P/T Terminal\r\n");
			debug_printf("3 P/T Joystick\r\n");
			debug_printf("4 Encode/Decode\r\n");
			debug_printf("5 IR Duplex\r\n");
			debug_printf("6 Radio Duplex\r\n");
			debug_printf("7 Integration\r\n");
			debug_printf("\r\n");
			break;
	}

	(*currentModeFunctions.deinit)();
	currentModeFunctions = nextModeFunctions;
	lightbarValue &= (0xFF & (currentModeFunctions.modeID << MODE_ID_SEGMENT));
	s4435360_lightbar_write(lightbarValue);
	(*currentModeFunctions.init)();
}

//////////////////////////////////////END IDLE MODE////////////////////////////

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
	s4435360_hal_pantilt_init();
	//s4435360_hal_radio_init();

	currentModeFunctions = idleModeFunctions;
	char userInput;

	while(1) {

		userInput = debug_getc();

		if(userInput == ESCAPE_CHAR) {
			idle_user_input('1');
		} else {
			(*currentModeFunctions.userInput)(userInput);
		}

		(*currentModeFunctions.run)();

		update_heartbeat();
		HAL_Delay(10);
	}
}
