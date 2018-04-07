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
#define ENTER_CHAR					(char)(13)
#define BACKSPACE_CHAR				(char)(8)
#define SPACE_CHAR					(char)(32)
#define MAXUSERCHARS				11

#define TRANSMIT_HEADER_BITS		0b0101000000000000000010
#define IR_RECEIVE_PERIOD 			100

#define START_MODE				0
#define IR_TRANSMIT_MODE		1
#define USER_INPUT_MODE			2
int userCharCount;
int currentIRDuplexMode = START_MODE;
int currentlyTransmitting = 0;
uint32_t bitsToTransmit = TRANSMIT_HEADER_BITS;
char* stringToTransmit;
int transmitBit = 21;
char transmitChar;
int transmitCharIndex;
TIM_HandleTypeDef rx_TIM_Init;
/* Timer Input Capture Configuration Structure declaration */
TIM_IC_InitTypeDef sICConfig;
uint16_t PrescalerValue = 0;
uint32_t TIMxCLKfreq = 16000000; //SystemCoreClock;

/* Captured Values */
uint32_t lastCaptureValue = 0;

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


	/////////////////////////IR Receive Init///////////////////////////////
	/* Initialise receive pin */
	GPIO_InitTypeDef GPIO_InitStructure;
	__TIM2_CLK_ENABLE();
	__BRD_D35_GPIO_CLK();

	GPIO_InitStructure.Pin = BRD_D35_PIN;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;
	GPIO_InitStructure.Alternate = GPIO_AF1_TIM2;

	HAL_GPIO_Init(BRD_D35_GPIO_PORT, &GPIO_InitStructure);
	HAL_NVIC_SetPriority(TIM2_IRQn, 0x0F, 0x00);
	HAL_NVIC_EnableIRQ(TIM2_IRQn);

	/* Calculate prescalar value */
	PrescalerValue = (uint16_t) ((SystemCoreClock / 2) / 50000) - 1;

	//TIM Base configuration
	rx_TIM_Init.Instance = TIM2;
	rx_TIM_Init.Init.Period = 0xFFFF; //Minimise update events
	rx_TIM_Init.Init.Prescaler = PrescalerValue;
	rx_TIM_Init.Init.ClockDivision = 0;
	rx_TIM_Init.Init.RepetitionCounter = 0;
	rx_TIM_Init.Init.CounterMode = TIM_COUNTERMODE_UP;

	if (HAL_TIM_IC_Init(&rx_TIM_Init) != HAL_OK) {
		debug_printf("Initialisation Error: IR Receiver "
				"Timer Pin D35\r\n");
	}

	// Configure the Input Capture channel
	sICConfig.ICPolarity = TIM_ICPOLARITY_BOTHEDGE; //Trigger on both edges
	sICConfig.ICSelection = TIM_ICSELECTION_DIRECTTI;
	sICConfig.ICPrescaler = TIM_ICPSC_DIV1;
	sICConfig.ICFilter = 0;
	if (HAL_TIM_IC_ConfigChannel(&rx_TIM_Init, &sICConfig, TIM_CHANNEL_4)
			!= HAL_OK) {
		debug_printf("Initialisation Error: IR Receiver "
				"Input Capture Pin D35\r\n");
	}

	// Start the Input Capture in interrupt mode
	if (HAL_TIM_IC_Start_IT(&rx_TIM_Init, TIM_CHANNEL_4) != HAL_OK) {
		debug_printf("Initialisation Error: IR Receiver Input "
				"Capture Start Pin D35\r\n");
	}

	lastCaptureValue = HAL_TIM_ReadCapturedValue(&rx_TIM_Init, TIM_CHANNEL_4);
}

/**
 * @brief Callback routine for input compare matches for IR receiver
 * @param htim: the timer handler of the triggering timer
 * @retval None
 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef* htim) {

	/* Check the triggering timer channel */
	if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4) {

		uint32_t currentCaptureValue = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_4);

		uint32_t captureValueDifference;
		if (currentCaptureValue > lastCaptureValue) {
			captureValueDifference = (currentCaptureValue - lastCaptureValue);
		} else if (lastCaptureValue < currentCaptureValue) {
			captureValueDifference = ((0xFFFF - lastCaptureValue) + currentCaptureValue) + 1;
		}

		if((captureValueDifference / ((TIMxCLKfreq / PrescalerValue))) > IR_RECEIVE_PERIOD) {
			lastCaptureValue = currentCaptureValue;

		}
	}
}

void TIM2_IRQHandler(void) {
	HAL_TIM_IRQHandler(&rx_TIM_Init);
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
	switch(currentIRDuplexMode) {

		case START_MODE:
			if(input == 'I') {
				currentIRDuplexMode = IR_TRANSMIT_MODE;
			}
			break;

		case IR_TRANSMIT_MODE:
			if(input == 'T') {
				currentIRDuplexMode = USER_INPUT_MODE;
			}
			break;

		case USER_INPUT_MODE:
			// Check for valid user inputs
			if(((input >= '0') && (input <= '9')) ||
					((input >= 'A') && (input <= 'Z')) ||
					((input >= 'a') && (input <= 'z')) ||
					(input == ENTER_CHAR) ||
					(input == SPACE_CHAR) ||
					(input == BACKSPACE_CHAR)) {

				/* Check for user-forced packet end */
				if(input == ENTER_CHAR) {
					//s4435360_radio_txstatus = PACKET_READY_TO_SEND;
					currentIRDuplexMode = START_MODE;
					return;
				}

				/* Check for backspace */
				if(input == BACKSPACE_CHAR) {
					if(userCharCount) {
						userCharCount--;
					}

					/* Handle general case */
				} else {
					//userInputs[userCharCount] = input;
					userCharCount++;
				}

				/* Check for packet completion */
				if(userCharCount >= MAXUSERCHARS) {
					//s4435360_radio_txstatus = PACKET_READY_TO_SEND;
					currentIRDuplexMode = START_MODE;
					return;
				} else {
					//s4435360_radio_txstatus = PACKET_NOT_READY_TO_SEND;
					return;
				}
			}
			break;
		}

}

void ir_duplex_timer2_handler(void){}
