/*
 * integration_speed_mode.h
 *
 *  Created on: 27Mar.,2018
 *      Author: Sam Eadie
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
