/**
 ******************************************************************************
 * @file    mylib/s4435360_os_dac.h
 * @author  Samuel Eadie - 44353607
 * @date    22052018
 * @brief   Provides OS functionality for DAC
 *
 ******************************************************************************
 *     EXTERNAL FUNCTIONS
 ******************************************************************************
 */

#ifndef S4435360_OS_DAC_H
#define S4435360_OS_DAC_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

typedef struct {
	char* command;
	int args[5];
} DACSequence;

SemaphoreHandle_t s4435360_SemaphoreDACoff, s4435360_SemaphoreDACon;
QueueHandle_t s4435360_QueueSequence;
void s4435360_DACTask(void);


#endif

