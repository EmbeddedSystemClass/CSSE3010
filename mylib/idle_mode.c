/**
  ******************************************************************************
  * @file    project1/idle_mode.c
  * @author  SE
  * @date    21032018-18042018
  * @brief   Idle mode functionality for project 1
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "structures.h"


void idle_init(void) {
	debug_printf("Idle mode\r\n");
}

void idle_deinit(void) {
}

void idle_run(void) {
	HAL_Delay(100);
}

void idle_user_input(char* userChars, int userCharsReceived) {
}
void idle_timer1_handler(void){}

void idle_timer2_handler(void){}
