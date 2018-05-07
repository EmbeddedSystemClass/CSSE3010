/***
 ******************************************************************************
 * @file    proj1/encode_decode_mode.h
 * @author  Samuel Eadie - 44353607
 * @date    21032018-18042018
 * @brief   Provides encode decode mode functionality for project 1
 ******************************************************************************
 */

#ifndef ENCODE_DECODE_MODE_H_
#define ENCODE_DECODE_MODE_H_

void encode_decode_init(void);
void encode_decode_deinit(void);
void encode_decode_run(void);
void encode_decode_user_input(char* userChars, int userCharsReceived);
void encode_decode_timer1_handler(void);
void encode_decode_timer2_handler(void);
void encode_decode_timer3_handler(void);

#endif
