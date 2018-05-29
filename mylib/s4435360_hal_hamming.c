/**
 ******************************************************************************
 * @file    mylib/s4435360_hal_hamming.c
 * @author  Samuel Eadie - 44353607
 * @date    25032018
 * @brief   Provides data hamming functionality
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <s4435360_hal_hamming.h>

#include "debug_printf.h"
#include "stm32f4xx_hal_conf.h"
#include "board.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Performs hamming encoding on half
  * a byte and adds an even parity check.
  *
  * y = xG
  *
  * G = [1 1 1 | 1 0 0 0]
  * 	[1 0 1 | 0 1 0 0]
  * 	[1 1 0 | 0 0 1 0]
  * 	[0 1 1 | 0 0 0 1]
  *
  * @param in: lower half of byte as input data
  * @retval encoded full byte
  */
uint8_t hamming_hbyte_encoder(uint8_t in) {

	uint8_t d0, d1, d2, d3;
	uint8_t p0 = 0, h0, h1, h2;
	uint8_t z;
	uint8_t out;

	/* extract bits */
	d0 = !!(in & 0x1);
	d1 = !!(in & 0x2);
	d2 = !!(in & 0x4);
	d3 = !!(in & 0x8);

	/* calculate hamming parity bits */
	h0 = d0 ^ d1 ^ d2;
	h1 = d0 ^ d2 ^ d3;
	h2 = d0 ^ d1 ^ d3;

	/* generate out byte without parity bit P0 */
	out = (h0 << 1) | (h1 << 2) | (h2 << 3) |
		(d0 << 4) | (d1 << 5) | (d2 << 6) | (d3 << 7);

	/* calculate even parity bit */
	for (z = 1; z < 8; z++)
		p0 = p0 ^ !!(out & (1 << z));

	out |= p0;

	return(out);
}

/**
  * @brief  Decodes a full byte of encoded data
  * into half a byte.
  *
  * s = Hy(transpose)
  *
  * H = [1 0 0 | 1 1 1 0]
  * 	[0 1 0 | 1 0 1 1]
  * 	[0 0 1 | 1 1 0 1]
  *
  * @param in: the encoded byte of data
  * @retval half byte of decoded data
  */
uint8_t hamming_hbyte_decoder(uint8_t in) {

	uint8_t s0, s1, s2;

	uint8_t h0 = !!(in & 2);
	uint8_t h1 = !!(in & 4);
	uint8_t h2 = !!(in & 8);

	uint8_t d0 = !!(in & 16);
	uint8_t d1 = !!(in & 32);
	uint8_t d2 = !!(in & 64);
	uint8_t d3 = !!(in & 128);

	uint8_t p0 = h0 ^ h1 ^ h2 ^ d0 ^ d1 ^ d2 ^ d3;

	s0 = h0 ^ d0 ^ d1 ^ d2;
	s1 = h1 ^ d0 ^ d2 ^ d3;
	s2 = h2 ^ d0 ^ d1 ^ d3;


	int8_t syndrome = (s0 << 0) | (s1 << 1) | (s2 <<2);

	/* If syndrome indicates no error */
	if(!syndrome) {
		/* If parity correct - no errors */
		if(p0 == !!(in & 1)) {
			return (d3 << 7) | (d2 << 6) | (d1 << 5) | (d0 << 4) |
				(h2 << 3) | (h1 << 2) | (h0 << 1) | (p0 << 0);

		/* If parity incorrect - parity is error */
		} else {
			return (d3 << 7) | (d2 << 6) | (d1 << 5) | (d0 << 4) |
				(h2 << 3) | (h1 << 2) | (h0 << 1) | ((!!(in & 1) ^ 1) << 0);
		}

	} else {
		/* If parity is 'correct' and there is a syndrome - two bit error */
		if(p0 == !!(in & 1)) {
			return UNCORRECTABLE_ERROR;
		}
	}


	/* H = [1 0 0 | 1 1 1 0]
	  * 	[0 1 0 | 1 0 1 1]
	  * 	[0 0 1 | 1 1 0 1]
	*/
	switch(syndrome) {
		case 1:
			h0 ^= (0x01);
			break;
		case 2:
			h1 ^= (0x01);
			break;
		case 4:
			h2 ^= (0x01);
			break;
		case 7:
			d0 ^= (0x01);
			break;
		case 5:
			d1 ^= (0x01);
			break;
		case 3:
			d2 ^= (0x01);
			break;
		case 6:
			d3 ^= (0x01);
			break;
		default:
			return UNCORRECTABLE_ERROR;
	}

	/* Parity bit should be odd, bit flipped */
	if(!!(in & 1)) {
		return (d3 << 7) | (d2 << 6) | (d1 << 5) | (d0 << 4) |
			(h2 << 3) | (h1 << 2) | (h0 << 1) | ((p0 ^ 0x01) << 0);
	}

	return UNCORRECTABLE_ERROR;

}

/**
  * @brief  Performs hamming encoding on a full
  * byte of data.
  * @param input: Byte of data to encode
  * @retval encoded two bytes of data
  */
uint16_t hamming_byte_encoder(uint8_t input) {
	return hamming_hbyte_encoder(input & 0xF) |
		(hamming_hbyte_encoder(input >> 4) << 8);

}

/**
  * @brief Decodes two bytes of data into one byte
  * @param input: encoded two bytes of data
  * @retval Decoded full byte of data
  */
HammingDecodedOutput hamming_byte_decoder(uint16_t input) {

	uint8_t out1, out2;

	HammingDecodedOutput output;

	out1 = hamming_hbyte_decoder((uint8_t)(input >> 8));
	out2 = hamming_hbyte_decoder((uint8_t) input);

	if ((out1 == UNCORRECTABLE_ERROR) || (out2 == UNCORRECTABLE_ERROR)) {
		output.uncorrectableError = 1;
	} else {
		output.uncorrectableError = 0;
		output.fullDecodedOutput = (out1 << 8) | out2;
		output.decodedOutput = (out1 & 0xF0) | ((out2 & 0xF0) >> 4);
		output.errorMask = input ^ output.fullDecodedOutput;
	}

	return output;

}

/**
 * @brief Hamming decodes a payload into the specified buffer
 * @param decodedBuffer: the buffer to decode the payload into
 * 		  payload: the payload to decode
 * 		  length: the length of the payload to decode
 * @retval returns whether the payload was successfully decoded
 */
int hamming_decode_payload(char* decodedBuffer, char* payload, int length) {

	HammingDecodedOutput hammingOutput;

	for(int i = 0; i < length; i++) {
		uint16_t encodedDoubleByte = (payload[2 * i] << 8) |
				(payload[(2 * i) + 1]);
		if(!encodedDoubleByte) {
			break;
		}

		hammingOutput = hamming_byte_decoder(encodedDoubleByte);

		//Check for correct decoding
		if(hammingOutput.uncorrectableError) {
			return 0;
		} else {
			decodedBuffer[i] = hammingOutput.decodedOutput;
		}
	}

	return 1;
}
