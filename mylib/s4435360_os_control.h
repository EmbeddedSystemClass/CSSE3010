/**
 ******************************************************************************
 * @file    mylib/s4435360_os_control.h
 * @author  Samuel Eadie - 44353607
 * @date    240518
 * @brief   Controller for project 2
 *
 ******************************************************************************
 *     EXTERNAL FUNCTIONS
 ******************************************************************************

 ******************************************************************************
 */

#ifndef S4435360_OS_CONTROL_H
#define S4435360_OS_CONTROL_H

#include "FreeRTOS.h"
#include "queue.h"

typedef enum CommandType{origin, line, square, bline, polygon, rose} CommandType;

#define DEFAULT_UP_Z_VALUE 			0
#define DEFAULT_DOWN_Z_VALUE 		50

QueueHandle_t s4435360_QueueCommands;

typedef struct {
	CommandType type;
	int args[5];
} Command;

void s4435360_TaskControl(void);

#endif

