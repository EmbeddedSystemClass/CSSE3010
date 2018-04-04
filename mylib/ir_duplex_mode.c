/**
  ******************************************************************************
  * @file    project1/ir_duplex_mode.c
  * @author  SE
  * @date    21032018-18042018
  * @brief   Encode decode mode functionality for project 1
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "ir_duplex_mode.h"
#include "s4435360_hal_ir.h"
#include "s4435360_hal_manchester.h"
#include "debug_printf.h"

#define TRANSMIT_HEADER_BITS		0b0101000000000000000010

int currentlyTransmitting = 0;
uint32_t bitsToTransmit = TRANSMIT_HEADER_BITS;
char* stringToTransmit;
int transmitBit = 21;
char transmitChar;
int transmitCharIndex;
TIM_HandleTypeDef irTimInit;


/**
  * @brief Instigates sending a char. Returns 1 iff
  * 		no char is currently being sent and the
  * 		input char was successfully instigated.
  * 		Timer interrupt sends character
  * @param input: character to transmit
  * @retval whether the character could successfully
  * 		be transmitted
  */
int send_char(char input) {
	if(currentlyTransmitting) {
		return 0;
	}
	s4435360_hal_ir_carrier_on();
	bitsToTransmit = TRANSMIT_HEADER_BITS | ((uint32_t)(s4435360_hal_manchester_byte_encoder(input)) << 2);
	transmitBit = 21;
	currentlyTransmitting = 1;
	HAL_TIM_Base_Start_IT(&irTimInit);
	transmitChar = input;
	return 1;
}

void send_string(char* string) {
	stringToTransmit = string;
	transmitCharIndex = 0;
	send_char(stringToTransmit[transmitCharIndex]);

}


/**
  * @brief Initialises ir duplex mode
  * @param None
  * @retval None
  */
void ir_duplex_init(void) {
	__TIM4_CLK_ENABLE();

	/* TIM Base configuration */
	irTimInit.Instance = TIM4;
	irTimInit.Init.Period = 16000; //1Hz interrupt frequency
	irTimInit.Init.Prescaler = (uint16_t) ((SystemCoreClock / 2) / 500) - 1;
	irTimInit.Init.ClockDivision = 0;
	irTimInit.Init.RepetitionCounter = 0;
	irTimInit.Init.CounterMode = TIM_COUNTERMODE_UP;

	/* Initialise Timer */
	HAL_TIM_Base_Init(&irTimInit);
	HAL_NVIC_SetPriority(TIM4_IRQn, 10, 0); //TIM8_BRK_TIM12_IRQn
	HAL_NVIC_EnableIRQ(TIM4_IRQn);
}

void TIM4_IRQHandler(void) {
	if(bitsToTransmit & (1 << transmitBit)) {
		s4435360_hal_ir_datamodulation_set();
	} else {
		s4435360_hal_ir_datamodulation_clr();
	}

	if(!transmitBit) {
		s4435360_hal_ir_datamodulation_clr();
		s4435360_hal_ir_carrier_off();
		HAL_TIM_Base_Stop_IT(&irTimInit);
		currentlyTransmitting = 0;
		debug_printf("Sent from IR: %c\r\n", transmitChar);
		transmitCharIndex++;

		if(transmitCharIndex < (sizeof(stringToTransmit) / sizeof(char))) {
			send_char(stringToTransmit[transmitCharIndex]);
		}
	} else {
		transmitBit--;
	}
}

/**
  * @brief Deinitialises ir duplex mode
  * @param None
  * @retval None
  */
void ir_duplex_deinit(void) {
	s4435360_hal_ir_datamodulation_clr();
	HAL_TIM_Base_Stop_IT(&irTimInit);
	s4435360_hal_ir_carrier_off();
}

/**
  * @brief IR duplex mode run function
  * @param None
  * @retval None
  */
void ir_duplex_run(void) {

}

/**
  * @brief Handles user input for ir duplex mode
  * @param input: the user input to handle
  * @retval None
  */
void ir_duplex_user_input(char input) {
	char* string = "abc";
	send_string(string);
}
