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

#define START_MODE				0
#define HAMMING_MODE	 		1
#define ENCODE_MODE				2
#define DECODE_MODE				3

uint8_t toHammingEncode = 0x00;
uint16_t toHammingDecode = 0x0000;
int currentHammingMode = START_MODE;
int hammingCharsReceived = 0;

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
	toHammingEncode = 0x00;
	toHammingDecode = 0x0000;
	currentHammingMode = START_MODE;
	hammingCharsReceived = 0;
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
void hamming_encode_decode_run(void) {
	if((currentHammingMode == ENCODE_MODE) && (hammingCharsReceived == 2)) {
		debug_printf("%X\r\n", hamming_byte_encoder(toHammingEncode));
		toHammingEncode = 0x00;
	} else if((currentHammingMode == DECODE_MODE) && (hammingCharsReceived == 4)) {
		HammingDecodedOutput output = hamming_byte_decoder(toHammingDecode);

		if(output.uncorrectableError) {
			debug_printf("2-bit ERROR\r\n");
		} else {
			debug_printf("%X (Full: %X ErrMask: %d)\r\n",
					output.decodedOutput,
					output.fullDecodedOutput,
					output.errorMask);
		}


		toHammingDecode = 0x0000;
	} else {
		return;
	}

	hammingCharsReceived = 0;
	currentHammingMode = START_MODE;
}

/**
  * @brief Handles user input for encode decode mode
  * @param input: the user input to handle
  * @retval None
  */
void hamming_encode_decode_user_input(char input) {
	switch(currentHammingMode) {
		case START_MODE:
			if(input == 'H') {
				currentHammingMode = HAMMING_MODE;
			}
			break;

		case HAMMING_MODE:
			if(input == 'E') {
				currentHammingMode = ENCODE_MODE;
			} else if(input == 'D') {
				currentHammingMode = DECODE_MODE;
			}
			break;

		case ENCODE_MODE:
			if(is_hamming_char_valid(input)) {
				toHammingEncode |= hamming_char_to_hex(input) << (4 - (4 * hammingCharsReceived));
			}
			hammingCharsReceived++;
			break;

		case DECODE_MODE:
			if(is_hamming_char_valid(input)) {
				toHammingDecode |= hamming_char_to_hex(input) << (12 - (4 * hammingCharsReceived));
			}
			hammingCharsReceived++;
			break;

	}
}

void hamming_encode_decode_timer1_handler(void){}

void hamming_encode_decode_timer2_handler(void){}
