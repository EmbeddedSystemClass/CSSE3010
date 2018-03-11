/**
 ******************************************************************************
 * @file    mylib/s4435360_hal_joystick.h
 * @author  Samuel Eadie - 44353607
 * @date    07032018-14032018
 * @brief   Joystick peripheral driver
 *
 ******************************************************************************
 *     EXTERNAL FUNCTIONS
 ******************************************************************************
 * s4435360_hal_joystick_init() – intialise joystick
 * s4435360_hal_joystick_x_read() - read X-axis
 * s4435360_hal_joystick_y_read() - read Y-axis
 ******************************************************************************
 */

#ifndef S4435360_JOYSTICK_H
#define S4435360_JOYSTICK_H

/* External function prototypes -----------------------------------------------*/
#include "stm32f4xx_hal_conf.h"
#include "board.h"

ADC_HandleTypeDef AdcHandleX;
ADC_HandleTypeDef AdcHandleY;

void s4435360_hal_joystick_init(void);

int joystick_read(ADC_HandleTypeDef AdcHandle);

#define s4435360_hal_joystick_x_read() joystick_read(AdcHandleX)
#define s4435360_hal_joystick_y_read() joystick_read(AdcHandleY)

#endif

