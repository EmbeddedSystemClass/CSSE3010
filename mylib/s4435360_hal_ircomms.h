/***
 ******************************************************************************
 * @file    mylib/s4435360_hal_manchester.h
 * @author  Samuel Eadie - 44353607
 * @date    31032018
 * @brief   Provides manchester encode/decode functionality
 ******************************************************************************
 */

#ifndef S4435360_HAL_MANCHESTER_H
#define S4435360_HAL_MANCHESTER_H

/* Includes ------------------------------------------------------------------*/

#include "stdint.h"

uint16_t s4435360_hal_manchester_byte_encoder(uint8_t input);
uint8_t s4435360_hal_manchester_byte_decoder(uint16_t input);

#endif
