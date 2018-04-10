/*
 * ir_duplex_mode.h
 *
 *  Created on: 27Mar.,2018
 *      Author: Sam Eadie
 */

#ifndef IR_DUPLEX_MODE_H_
#define IR_DUPLEX_MODE_H_

int receivedChar;
unsigned char rxBuffer[11];
unsigned char rxChar;

void send_char(char input);
void send_string(char* string, int numChars);
void ir_duplex_init(void);
void ir_timer1_init(void);
void ir_rx_init(void);
void handle_received_char(char rxInput);
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef* htim);
void TIM2_IRQHandler(void);
void ir_duplex_timer1_handler(void);
void ir_duplex_deinit(void);
void ir_duplex_run(void);
void ir_duplex_user_input(char* userChars, int userCharsReceived);
void ir_duplex_timer2_handler(void);
void ir_duplex_timer3_handler(void);

#endif
