/**
 ******************************************************************************
 * @file    mylib/s4435360_hal_manchester.c
 * @author  Samuel Eadie - 44353607
 * @date    25032018
 * @brief   Provides manchester encode/decode functionality
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <s4435360_hal_manchester.h>

#include "debug_printf.h"
#include "stm32f4xx_hal_conf.h"
#include "board.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define ZERO_CODE			2
#define ONE_CODE			1
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Performs manchester encoding on a full
  * byte of data.
  * @param input: Byte of data to encode
  * @retval encoded two bytes of data
  */
uint16_t s4435360_hal_manchester_byte_encoder(uint8_t input) {

	uint16_t encodedOutput = 0x0000;

	for(int i = 0; i < 8; i++) {
		if(input & (1 << i)) {
			encodedOutput |= (ONE_CODE << (2 * i));
		} else {
			encodedOutput |= (ZERO_CODE << (2 * i));
		}
	}

	return encodedOutput;

}

/**
  * @brief Decodes two bytes of data into one byte
  * @param input: encoded two bytes of data
  * @retval Decoded full byte of data
  */
uint8_t s4435360_hal_manchester_byte_decoder(uint16_t input) {

	uint8_t decodedOutput = 0x00;

	for(int i = 0; i < 8; i++) {
		if(((input & (3 << (2 * i))) >> (2 * i)) == ZERO_CODE) {
			decodedOutput |= (0 << i);
		} else if(((input & (3 << (2 * i))) >> (2 * i)) == ONE_CODE) {
			decodedOutput |= (1 << i);
		} else {
			//Error
		}

	}

	return decodedOutput;

}
