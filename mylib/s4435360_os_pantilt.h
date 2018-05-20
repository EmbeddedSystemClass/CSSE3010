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
#include "semphr.h"

QueueHandle_t s4435360_QueuePan, s4435360_QueueTilt;
SemaphoreHandle_t s4435360_SemaphoreUpdatePantilt;
SemaphoreHandle_t s4435360_SemaphorePanLeft, s4435360_SemaphorePanRight;
SemaphoreHandle_t s4435360_SemaphoreTiltUp, s4435360_SemaphoreTiltDown;

void s4435360_TaskPanTilt(void);
void s4435360_pantilt_changeX(int x);
void s4435360_pantilt_changeY(int y);

#endif

