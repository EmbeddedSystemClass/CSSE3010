/**
  ******************************************************************************
  * @file    project1/ir_duplex_mode.c
  * @author  SE
  * @date    21032018-18042018
  * @brief   Encode decode mode functionality for project 1
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "structures.h"
#include "ir_duplex_mode.h"
#include "s4435360_hal_ir.h"
#include "s4435360_hal_manchester.h"

#define TRANSMIT_HEADER_BITS		0b0101000000000000000010

int currentlyTransmitting = 0;
uint32_t bitsToTransmit = TRANSMIT_HEADER_BITS;
char* stringToTransmit;
int transmitBit = 21;
char transmitChar;
int transmitCharIndex;


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
	HAL_TIM_Base_Start_IT(&timer1Init);
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
	__TIMER1_CLK_ENABLE();

	/* TIM Base configuration */
	timer1Init.Instance = TIMER1;
	timer1Init.Init.Period = 50000/100; //1Hz interrupt frequency
	timer1Init.Init.Prescaler = (uint16_t) ((SystemCoreClock / 2) / 50000) - 1;
	timer1Init.Init.ClockDivision = 0;
	timer1Init.Init.RepetitionCounter = 0;
	timer1Init.Init.CounterMode = TIM_COUNTERMODE_UP;

	/* Initialise Timer */
	HAL_TIM_Base_Init(&timer1Init);
	HAL_NVIC_SetPriority(TIMER1_IRQ, 10, 0);
	HAL_NVIC_EnableIRQ(TIMER1_IRQ);
}

void ir_duplex_timer1_handler(void) {
	debug_printf("Inside interrupt\r\n");
	if(bitsToTransmit & (1 << transmitBit)) {
		debug_printf("Should be setting\r\n");
		s4435360_hal_ir_datamodulation_set();
	} else {
		s4435360_hal_ir_datamodulation_clr();
	}

	if(!transmitBit) {
		s4435360_hal_ir_datamodulation_clr();
		s4435360_hal_ir_carrier_off();
		HAL_TIM_Base_Stop_IT(&timer1Init);
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
	HAL_TIM_Base_Stop_IT(&timer1Init);
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
	debug_printf("%c\r\n", input);
	char* string = "abc";
	send_string(string);
}

void ir_duplex_timer2_handler(void){}
