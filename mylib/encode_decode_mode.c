/**
  ******************************************************************************
  * @file    project1/encode_decode_mode.c
  * @author  SE
  * @date    21032018-18042018
  * @brief   Encode decode mode functionality for project 1
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "encode_decode_mode.h"
#include "s4435360_hal_manchester.h"
#include "debug_printf.h"

#define START_MODE				0
#define MANCHESTER_MODE 		1
#define ENCODE_MODE				2
#define DECODE_MODE				3

uint8_t toEncode = 0x00;
uint16_t toDecode = 0x0000;
int currentMode = START_MODE;
int charsReceived = 0;

/**
  * @brief Converts a char to its hex value
  * @param input: char to convert
  * @retval the converted hex value
  */
int char_to_hex(char input) {
	if((input >= '0') && (input <= '9')) {
		return input - '0';
	} else if((input >= 'A') && (input <= 'F')) {
		return input - 'A' + 10;
	} else {
		//Error
		return 0xFF;
	}
}

/**
  * @brief Returns true iff the char is valid hex
  * @param the character to check
  * @retval 1 iff the char is valid hex
  */
int is_char_valid(char input) {
	return ((input >= '0') && (input <= '9')) ||
			((input >= 'A') && (input <= 'F'));
}

/**
  * @brief Initialises encode decode mode
  * @param None
  * @retval None
  */
void encode_decode_init(void) {
	toEncode = 0x00;
	toDecode = 0x0000;
	currentMode = START_MODE;
	charsReceived = 0;
}

/**
  * @brief Deinitialises encode decode mode
  * @param None
  * @retval None
  */
void encode_decode_deinit(void) {}

/**
  * @brief Encode decode mode run function
  * @param None
  * @retval None
  */
void encode_decode_run(void) {
	if((currentMode == ENCODE_MODE) && (charsReceived == 2)) {
		debug_printf("%X\r\n", s4435360_hal_manchester_byte_encoder(toEncode));
		toEncode = 0x00;
	} else if((currentMode == DECODE_MODE) && (charsReceived == 4)) {
		debug_printf("%X\r\n", s4435360_hal_manchester_byte_decoder(toDecode));
		toDecode = 0x0000;
	} else {
		return;
	}

	charsReceived = 0;
	currentMode = START_MODE;
}

/**
  * @brief Handles user input for encode decode mode
  * @param input: the user input to handle
  * @retval None
  */
void encode_decode_user_input(char input) {
	switch(currentMode) {
		case START_MODE:
			if(input == 'M') {
				currentMode = MANCHESTER_MODE;
			}
			break;

		case MANCHESTER_MODE:
			if(input == 'E') {
				currentMode = ENCODE_MODE;
			} else if(input == 'D') {
				currentMode = DECODE_MODE;
			}
			break;

		case ENCODE_MODE:
			if(is_char_valid(input)) {
				toEncode |= char_to_hex(input) << (4 - (4 * charsReceived));
			}
			charsReceived++;
			break;

		case DECODE_MODE:
			if(is_char_valid(input)) {
				toDecode |= char_to_hex(input) << (12 - (4 * charsReceived));
			}
			charsReceived++;
			break;

	}
}
