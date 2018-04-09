/*
 * radio_duplex_mode.h
 *
 *  Created on: 27Mar.,2018
 *      Author: Sam Eadie
 */

#ifndef RADIO_DUPLEX_MODE_H_
#define RADIO_DUPLEX_MODE_H_

void radio_duplex_init(void);
void radio_duplex_deinit(void);
void radio_duplex_run(void);
void radio_duplex_user_input(char* userChars, int userCharsReceived);
void radio_duplex_timer1_handler(void);
void radio_duplex_timer2_handler(void);

#endif
