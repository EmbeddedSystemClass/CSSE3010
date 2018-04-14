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

/**
 * @brief Initialises idle mode
 * @param None
 * @retval None
 */
void idle_init(void) {
	debug_printf("Idle mode\r\n");
}

/**
 * @brief Deinitialises idle mode
 * @param None
 * @retval None
 */
void idle_deinit(void) {}

/**
 * @brief Run function for idle mode
 * @param None
 * @retval None
 */
void idle_run(void) {
	HAL_Delay(100);
}

/**
 * @brief Handles input from the user in idle mode
 * @param userChars: an array of user inputted chars
 * 		  userCharsReceived: number of chars received
 * @retval None
 */
void idle_user_input(char* userChars, int userCharsReceived) {}

void idle_timer1_handler(void){}
void idle_timer2_handler(void){}
void idle_timer3_handler(void){}
