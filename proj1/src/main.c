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
#include "pantilt_terminal_mode.h"
#include "pantilt_joystick_mode.h"
#include "encode_decode_mode.h"
#include "hamming_encode_decode_mode.h"
#include "ir_duplex_mode.h"
#include "radio_duplex_mode.h"

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
		.userInput = &pantilt_terminal_user_input,
		.timer1Handler = &pantilt_terminal_timer1_handler,
		.timer2Handler = &pantilt_terminal_timer2_handler};

ModeFunctions pantiltJoystickModeFunctions = {.modeID = 0x02,
		.init = &pantilt_joystick_init,
		.deinit = &pantilt_joystick_deinit,
		.run = &pantilt_joystick_run,
		.userInput = &pantilt_joystick_user_input,
		.timer1Handler = &pantilt_joystick_timer1_handler,
		.timer2Handler = &pantilt_joystick_timer2_handler};

ModeFunctions encodeDecodeModeFunctions = {.modeID = 0x03,
		.init = &encode_decode_init,
		.deinit = &encode_decode_deinit,
		.run = &encode_decode_run,
		.userInput = &encode_decode_user_input,
		.timer1Handler = &encode_decode_timer1_handler,
		.timer2Handler = &encode_decode_timer2_handler};

ModeFunctions irDuplexModeFunctions = {.modeID = 0x04,
		.init = &ir_duplex_init,
		.deinit = &ir_duplex_deinit,
		.run = &ir_duplex_run,
		.userInput = &ir_duplex_user_input,
		.timer1Handler = &ir_duplex_timer1_handler,
		.timer2Handler = &ir_duplex_timer2_handler};

ModeFunctions hammingEncodeDecodeModeFunctions = {.modeID = 0x05,
		.init = &hamming_encode_decode_init,
		.deinit = &hamming_encode_decode_deinit,
		.run = &hamming_encode_decode_run,
		.userInput = &hamming_encode_decode_user_input,
		.timer1Handler = &hamming_encode_decode_timer1_handler,
		.timer2Handler = &hamming_encode_decode_timer2_handler};

ModeFunctions radioDuplexModeFunctions = {.modeID = 0x06,
		.init = &radio_duplex_init,
		.deinit = &radio_duplex_deinit,
		.run = &radio_duplex_run,
		.userInput = &radio_duplex_user_input,
		.timer1Handler = &radio_duplex_timer1_handler,
		.timer2Handler = &radio_duplex_timer2_handler};

void idle_init(void);
void idle_deinit(void);
void idle_run(void);
void idle_user_input(char input);
void idle_timer1_handler(void);
void idle_timer2_handler(void);

ModeFunctions idleModeFunctions = {.modeID = 0x00,
			.init = &idle_init,
			.deinit = &idle_deinit,
			.run = &idle_run,
			.userInput = &idle_user_input,
			.timer1Handler = &idle_timer1_handler,
			.timer2Handler = &idle_timer2_handler};

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

	debug_printf("Entering switch with input: %c\r\n", input);

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
			debug_printf("6 Hamming Encode/Decode\r\n");
			debug_printf("7 Radio Duplex\r\n");
			debug_printf("8 Integration\r\n");
			debug_printf("\r\n");
			return;
		default:
			return;
	}

	(*currentModeFunctions.deinit)();
	currentModeFunctions = nextModeFunctions;
	lightbarValue &= (0xFF & (currentModeFunctions.modeID << MODE_ID_SEGMENT));
	s4435360_lightbar_write(lightbarValue);
	(*currentModeFunctions.init)();
}

void idle_timer1_handler(void){}

void idle_timer2_handler(void){}
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

	BRD_init();
	BRD_LEDInit();
	s4435360_hal_ir_init();
	s4435360_hal_joystick_init();
	s4435360_lightbar_init();
	s4435360_hal_pantilt_init();
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
	s4435360_radio_init(); //CHECK PB10, IR TX and radio SCK both using same pin - can't init

	currentModeFunctions = idleModeFunctions;
	char userInput;

	while(1) {

		userInput = debug_getc();

		if(userInput) {
			debug_printf("%c", userInput);
			if(userInput == ESCAPE_CHAR) {
				idle_user_input('1');
			} else {
				(*currentModeFunctions.userInput)(userInput);
			}
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
	}
}

void TIMER1_HANDLER(void) {
	HAL_TIM_IRQHandler(&timer1Init);
}

void TIMER2_HANDLER(void) {
	HAL_TIM_IRQHandler(&timer2Init);
}





