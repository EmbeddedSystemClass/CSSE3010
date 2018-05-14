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

#ifndef S4435360_OS_PANTILT_H
#define S4435360_OS_PANTILT_H

#include "FreeRTOS.h"
#include "queue.h"

typedef struct {
	char payload[11];
	int payloadLength;
	int retransmitAttempts;
} RadioMessage;

QueueHandle_t txMessageQueue;

extern void s4435360_TaskRadio(void);


#endif

