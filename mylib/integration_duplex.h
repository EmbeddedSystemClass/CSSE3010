/**
  ******************************************************************************
  * @file    proj1/integration_duplex.h
  * @author  Samuel Eadie - 44353607
  * @date    21032018-18042018
  * @brief   Provides integration duplex mode functionality for project 1
  ******************************************************************************
  */
#ifndef INTEGRATION_DUPLEX_MODE_H_
#define INTEGRATION_DUPLEX_MODE_H_

void integration_duplex_init(void);
void integration_duplex_deinit(void);
void integration_duplex_run(void);
void integration_duplex_user_input(char* userChars, int userCharsReceived);
void integration_duplex_timer1_handler(void);
void integration_duplex_timer2_handler(void);
void integration_duplex_timer3_handler(void);

#endif
