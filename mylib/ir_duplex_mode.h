/*
 * ir_duplex_mode.h
 *
 *  Created on: 27Mar.,2018
 *      Author: Sam Eadie
 */

#ifndef IR_DUPLEX_MODE_H_
#define IR_DUPLEX_MODE_H_

void ir_duplex_init(void);
void ir_duplex_deinit(void);
void ir_duplex_run(void);
void ir_duplex_user_input(char* userChars, int userCharsReceived);
void ir_duplex_timer1_handler(void);
void ir_duplex_timer2_handler(void);

#endif
