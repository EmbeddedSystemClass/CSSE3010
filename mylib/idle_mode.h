/**
  ******************************************************************************
  * @file    proj1/idle_mode.h
  * @author  Samuel Eadie - 44353607
  * @date    21032018-18042018
  * @brief   Provides idle mode functionality for project 1
  ******************************************************************************
  */
#ifndef IDLE_MODE_H
#define IDLE_MODE_H

void idle_init(void);
void idle_deinit(void);
void idle_run(void);
void idle_user_input(char* userChars, int userCharsReceived);
void idle_timer1_handler(void);
void idle_timer2_handler(void);
void idle_timer3_handler(void);

#endif
