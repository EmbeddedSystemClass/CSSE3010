/**
  ******************************************************************************
  * @file    proj1/radio_duplex_mode.h
  * @author  Samuel Eadie - 44353607
  * @date    21032018-18042018
  * @brief   Duplex radio communication mode functionality for project 1
  ******************************************************************************
  */

#ifndef RADIO_DUPLEX_MODE_H_
#define RADIO_DUPLEX_MODE_H_

#include "string.h"
#include "structures.h"

void form_packet(unsigned char* payload, int payloadLength, unsigned char* packet);
void print_sent_packet(void);
void print_received_packet(uint8_t* packet_buffer);
void radio_duplex_init(void);
void radio_duplex_deinit(void);
void radio_duplex_run(void);
void radio_duplex_user_input(char* userChars, int userCharsReceived);
void radio_duplex_timer1_handler(void);
void radio_duplex_timer2_handler(void);
void radio_duplex_timer3_handler(void);

#endif
