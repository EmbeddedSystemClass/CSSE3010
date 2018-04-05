/***
 ******************************************************************************
 * @file    mylib/s4435360_hal_hamming.h
 * @author  Samuel Eadie - 44353607
 * @date    25032018
 * @brief   Provides hamming functionality
 ******************************************************************************
 */

#ifndef S4435360_HAL_HAMMING_H
#define S4435360_HAL_HAMMING_H

/* Includes ------------------------------------------------------------------*/

#include "stdint.h"

#define UNCORRECTABLE_ERROR		0xFF

typedef struct {

	uint8_t decodedOutput;
	uint16_t fullDecodedOutput;
	int uncorrectableError;
	uint8_t errorMask;

} HammingDecodedOutput;


uint8_t hamming_hbyte_encoder(uint8_t in);
uint8_t hamming_hbyte_decoder(uint8_t in);
uint16_t hamming_byte_encoder(uint8_t input);
HammingDecodedOutput hamming_byte_decoder(uint16_t input);

#endif
