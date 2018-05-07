/**
  ******************************************************************************
  * @file    proj1/integration_speed.h
  * @author  Samuel Eadie - 44353607
  * @date    21032018-18042018
  * @brief   Provides integration speed mode functionality for project 1
  ******************************************************************************
  */

#ifndef INTEGRATION_SPEED_MODE_H_
#define INTEGRATION_SPEED_MODE_H_

void integration_speed_init(void);
void integration_speed_deinit(void);
void integration_speed_run(void);
void integration_speed_user_input(char* userChars, int userCharsReceived);
void integration_speed_timer1_handler(void);
void integration_speed_timer2_handler(void);
void integration_speed_timer3_handler(void);

#endif
