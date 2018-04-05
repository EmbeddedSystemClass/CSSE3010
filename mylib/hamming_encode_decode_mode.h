/*
 * hamming_encode_decode_mode.h
 *
 *  Created on: 27Mar.,2018
 *      Author: Sam Eadie
 */

#ifndef HAMMING_ENCODE_DECODE_MODE_H_
#define HAMMING_ENCODE_DECODE_MODE_H_

void hamming_encode_decode_init(void);
void hamming_encode_decode_deinit(void);
void hamming_encode_decode_run(void);
void hamming_encode_decode_user_input(char input);
void hamming_encode_decode_timer1_handler(void);
void hamming_encode_decode_timer2_handler(void);

#endif
