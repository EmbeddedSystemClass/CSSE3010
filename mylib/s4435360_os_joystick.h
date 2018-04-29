/**
 ******************************************************************************
 * @file    mylib/s4435360_os_joystick.h
 * @author  Samuel Eadie - 44353607
 * @date    27042018
 * @brief   OS control of joystick
 *
 ******************************************************************************
 *     EXTERNAL FUNCTIONS
 ******************************************************************************

 ******************************************************************************
 */

#ifndef S4435360_OS_JOYSTICK_H
#define S4435360_OS_JOYSTICK_H


#include "FreeRTOS.h"
#include "semphr.h"

SemaphoreHandle_t s4435360_SemaphoreJoystickZ;

void s4435360_os_joystick_init(void);

#endif

