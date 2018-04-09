/*
 * idle_mode.h
 *
 *  Created on: 27Mar.,2018
 *      Author: Sam Eadie
 */

#ifndef IDLE_MODE_H
#define IDLE_MODE_H

void idle_init(void);
void idle_deinit(void);
void idle_run(void);
void idle_user_input(char* userChars, int userCharsReceived);
void idle_timer1_handler(void);
void idle_timer2_handler(void);

#endif
