/*
 * integration_duplex_mode.h
 *
 *  Created on: 27Mar.,2018
 *      Author: Sam Eadie
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
