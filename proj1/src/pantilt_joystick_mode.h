/**
  ******************************************************************************
  * @file    proj1/pantilt_joystick_mode.h
  * @author  Samuel Eadie - 44353607
  * @date    21032018-18042018
  * @brief   Provides pantilt joystick mode functionality for project 1
  ******************************************************************************
  */
#ifndef PANTILT_JOYSTICK_MODE_H_
#define PANTILT_JOYSTICK_MODE_H_

void pantilt_joystick_init(void);
void pantilt_joystick_deinit(void);
void pantilt_joystick_run(void);
void pantilt_joystick_user_input(char* userChars, int userCharsReceived);
void pantilt_joystick_timer1_handler(void);
void pantilt_joystick_timer2_handler(void);
void pantilt_joystick_timer3_handler(void);

#endif
