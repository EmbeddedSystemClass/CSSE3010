/*
 * encode_decode_mode.h
 *
 *  Created on: 27Mar.,2018
 *      Author: Sam Eadie
 */

#ifndef ENCODE_DECODE_MODE_H_
#define ENCODE_DECODE_MODE_H_

void encode_decode_init(void);
void encode_decode_deinit(void);
void encode_decode_run(void);
void encode_decode_user_input(char input);
void encode_decode_timer1_handler(void);
void encode_decode_timer2_handler(void);

#endif
