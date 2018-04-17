/**
  ******************************************************************************
  * @file    proj1/pantilt_terminal_mode.h
  * @author  Samuel Eadie - 44353607
  * @date    21032018-18042018
  * @brief   Pantilt terminal mode functionality for project 1
  ******************************************************************************
  */

#ifndef PANTILT_TERMINAL_MODE_H_
#define PANTILT_TERMINAL_MODE_H_

void pantilt_terminal_init(void);
void pantilt_terminal_deinit(void);
void pantilt_terminal_run(void);
void pantilt_terminal_user_input(char* userChars, int userCharsReceived);
void pantilt_terminal_timer1_handler(void);
void pantilt_terminal_timer2_handler(void);
void pantilt_terminal_timer3_handler(void);

#endif
