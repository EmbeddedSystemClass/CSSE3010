/**
  ******************************************************************************
  * @file    project1/encode_decode_mode.c
  * @author  SE
  * @date    21032018-18042018
  * @brief   Encode decode mode functionality for project 1
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "structures.h"
#include "encode_decode_mode.h"
#include "s4435360_hal_manchester.h"

uint8_t toEncode = 0x00;
uint16_t toDecode = 0x0000;

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
	debug_printf("Manchester encode/decode mode\r\n");
	toEncode = 0x00;
	toDecode = 0x0000;
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
void encode_decode_run(void) {}

/**
  * @brief Handles user input for encode decode mode
  * @param input: the user input to handle
  * @retval None
  */
void encode_decode_user_input(char* userChars, int userCharsReceived) {
	if(userChars[0] != 'M') {
		return;
	}

	/* Encode Mode */
	if(userChars[1] == 'E') {
		if(userCharsReceived < 4) {
			return;
		}

		toEncode = (char_to_hex(userChars[2]) << 4) | (char_to_hex(userChars[3]));
		debug_printf("%X --> %X\r\n", toEncode, s4435360_hal_manchester_byte_encoder(toEncode));

	/* Decode Mode */
	} else if(userChars[1] == 'D') {
		if(userCharsReceived < 6) {
			return;
		}

		toDecode = (uint16_t)(char_to_hex(userChars[2]) << 12) |
				(char_to_hex(userChars[3]) << 8) |
				(char_to_hex(userChars[4]) << 4) |
				(char_to_hex(userChars[5]) << 0);

		debug_printf("%X --> %X\r\n", toDecode, s4435360_hal_manchester_byte_decoder(toDecode));
	}
}

void encode_decode_timer1_handler(void){}

void encode_decode_timer2_handler(void){}
