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

//Enumerate command types for control task
typedef enum CommandType{origin, line, square, bline, polygon, rose} CommandType;

//Default z values
#define DEFAULT_UP_Z_VALUE 			0
#define DEFAULT_DOWN_Z_VALUE 		50

//Commands for executing
QueueHandle_t s4435360_QueueCommands;

//Struct for storing user commands
typedef struct {
	CommandType type;
	int args[5];
} Command;

void s4435360_TaskControl(void);

#endif

