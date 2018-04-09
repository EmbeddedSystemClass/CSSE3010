/**
  ******************************************************************************
  * @file    project1/hamming_encode_decode_mode.c
  * @author  SE
  * @date    21032018-18042018
  * @brief   Hamming encode decode mode functionality for project 1
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "structures.h"
#include "hamming_encode_decode_mode.h"
#include "s4435360_hal_hamming.h"

uint8_t toHammingEncode = 0x00;
uint16_t toHammingDecode = 0x0000;

/**
  * @brief Converts a char to its hex value
  * @param input: char to convert
  * @retval the converted hex value
  */
int hamming_char_to_hex(char input) {
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
int is_hamming_char_valid(char input) {
	return ((input >= '0') && (input <= '9')) ||
			((input >= 'A') && (input <= 'F'));
}

/**
  * @brief Initialises encode decode mode
  * @param None
  * @retval None
  */
void hamming_encode_decode_init(void) {
	debug_printf("Hamming encode/decode mode\r\n");
	toHammingEncode = 0x00;
	toHammingDecode = 0x0000;
}

/**
  * @brief Deinitialises encode decode mode
  * @param None
  * @retval None
  */
void hamming_encode_decode_deinit(void) {}

/**
  * @brief Encode decode mode run function
  * @param None
  * @retval None
  */
void hamming_encode_decode_run(void) {}

/**
  * @brief Handles user input for encode decode mode
  * @param input: the user input to handle
  * @retval None
  */
void hamming_encode_decode_user_input(char* userChars, int userCharsReceived) {
	if(userChars[0] != 'H') {
		return;
	}

	/* Encode Mode */
	if(userChars[1] == 'E') {
		if(userCharsReceived < 4) {
			return;
		}

		toHammingEncode = (char_to_hex(userChars[2]) << 4) | (char_to_hex(userChars[3]));
		debug_printf("%X\r\n", hamming_byte_encoder(toHammingEncode));

	/* Decode Mode */
	} else if(userChars[1] == 'D') {
		if(userCharsReceived < 6) {
			return;
		}

		toHammingDecode = (char_to_hex(userChars[2]) << 12) |
				(char_to_hex(userChars[3]) << 8) |
				(char_to_hex(userChars[4]) << 4) |
				(char_to_hex(userChars[5]) << 0);

		HammingDecodedOutput output = hamming_byte_decoder(toHammingDecode);
		if(output.uncorrectableError) {
			debug_printf("2-bit ERROR\r\n");
		} else {
			debug_printf("%X (Full: %X ErrMask: %04X)\r\n",
					output.decodedOutput,
					output.fullDecodedOutput,
					output.errorMask);
		}
	}
}

void hamming_encode_decode_timer1_handler(void){}

void hamming_encode_decode_timer2_handler(void){}
