/**
 ******************************************************************************
 * @file    mylib/s4435360_os_printf.h
 * @author  Samuel Eadie - 44353607
 * @date    070518
 * @brief   Thread safe printf
 *
 ******************************************************************************
 *     EXTERNAL FUNCTIONS
 ******************************************************************************

 ******************************************************************************
 */

#ifndef S4435360_OS_PRINTF_H
#define S4435360_OS_PRINTF_H

#include "FreeRTOS.h"
#include "queue.h"

QueueHandle_t s4435360_QueuePrintf;

void s4435360_TaskPrintf(void);
void send_to_queue(const char* format, ...);

#define myprintf 	send_to_queue

#endif

