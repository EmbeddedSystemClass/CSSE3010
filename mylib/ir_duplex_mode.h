/**
  ******************************************************************************
  * @file    proj1/ir_duplex_mode.h
  * @author  Samuel Eadie - 44353607
  * @date    21032018-18042018
  * @brief   Provides IR duplex mode functionality for project 1
  ******************************************************************************
  */
#ifndef IR_DUPLEX_MODE_H_
#define IR_DUPLEX_MODE_H_

int receivedChar;
unsigned char rxBuffer[11];
unsigned char rxChar;
int receivePeriod, currentlyTransmitting;

#define PERIOD_REGISTER_TIME_SCALAR 			50000

void configure_transmission(uint64_t bits, int index, uint16_t irPeriod);
void configure_receive(int length, void (*charHandler)(uint16_t));
void send_char(char input);
void send_string(char* string, int numChars);
void ir_duplex_init(void);
void ir_timer1_init(void);
void ir_rx_init(void);
void handle_received_char(uint16_t rxInput);
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef* htim);
void TIM2_IRQHandler(void);
void ir_duplex_timer1_handler(void);
void ir_duplex_deinit(void);
void ir_duplex_run(void);
void ir_duplex_user_input(char* userChars, int userCharsReceived);
void ir_duplex_timer2_handler(void);
void ir_duplex_timer3_handler(void);

#endif
