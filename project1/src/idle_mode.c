/**
  ******************************************************************************
  * @file    project1/idle_mode.c
  * @author  SE
  * @date    21032018-18042018
  * @brief   Idle mode functionality for project 1
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "idle_mode.h"

#include "board.h"
#include "stm32f4xx_hal_conf.h"
#include "debug_printf.h"

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
	if(input == '?') {
		debug_printf("CSSE3010 Project 1\r\n");
		debug_printf("1 Idle\r\n");
		debug_printf("2 P/T Terminal\r\n");
		debug_printf("3 P/T Joystick\r\n");
		debug_printf("4 Encode/Decode\r\n");
		debug_printf("5 IR Duplex\r\n");
		debug_printf("6 Radio Duplex\r\n");
		debug_printf("7 Integration\r\n");
		debug_printf("\r\n");
	}

	debug_printf("Handling input for idle mode\r\n");
}
