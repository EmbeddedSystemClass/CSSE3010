/**
 ******************************************************************************
 * @file    mylib/s4435360_os_pantilt.h
 * @author  Samuel Eadie - 44353607
 * @date    070518
 * @brief   Pantilt OS functionality
 *
 ******************************************************************************
 *     EXTERNAL FUNCTIONS
 ******************************************************************************

 ******************************************************************************
 */

#ifndef S4435360_OS_RADIO_H
#define S4435360_OS_RADIO_H

#include "FreeRTOS.h"
#include "queue.h"

#define DEFAULT_UP_Z_VALUE 			0
#define DEFAULT_DOWN_Z_VALUE 		30

typedef struct {
	char payload[11];
	int payloadLength;
	int retransmitAttempts;
	int isXYZ;
} RadioMessage;

QueueHandle_t txMessageQueue;

unsigned char* get_txAddress(void);
unsigned char* get_rxAddress(void);
unsigned char get_chan(void);
void set_txAddress(unsigned char* addr);
void set_txAddress(unsigned char* addr);
void set_chan(unsigned char chan);

extern void s4435360_TaskRadio(void);

void send_radio_message(char* payload, int payloadLength, int retransmitAttempts, int waitTime, int isXYZ);

void send_XYZ_message(int x, int y, int z, int waitTime);

void send_join_message(int waitTime);


#endif

