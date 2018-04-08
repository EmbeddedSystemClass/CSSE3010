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
#include <string.h>
#define ENTER_CHAR					(char)(13)
#define BACKSPACE_CHAR				(char)(8)
#define SPACE_CHAR					(char)(32)
#define MAXUSERCHARS				11
#define STX_CHAR 					(char)(0x02)
#define ETX_CHAR					(char)(0x03)
#define ACK_CHAR					(char)(0x07)

#define TRANSMIT_HEADER_BITS		0b0101000000000000000010
#define IR_RECEIVE_PERIOD 			100

#define START_MODE				0
#define IR_TRANSMIT_MODE		1
#define USER_INPUT_MODE			2
int irUserCharCount;
char irUserChars[11];
int currentIRDuplexMode = START_MODE;
int currentlyTransmitting = 0;
uint32_t bitsToTransmit = TRANSMIT_HEADER_BITS;
char stringToTransmit[13];
int transmitBit = 21;
char transmitChar;
int transmitCharIndex;

TIM_HandleTypeDef rx_TIM_Init;
TIM_IC_InitTypeDef sICConfig;
uint16_t PrescalerValue = 0;
uint32_t TIMxCLKfreq = 16000000; //SystemCoreClock;

/* Captured Values */
uint32_t lastCaptureValue = 0;
uint8_t captureChar = 0x00;
int bitsReceived, receivePeriod, receivedSTX, receivedChar, receivedString, irCharsReceived;
unsigned char rxBuffer[11];
unsigned char rxChar;
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
	strcpy(&stringToTransmit[1], string);
	stringToTransmit[0] = STX_CHAR;
	stringToTransmit[1 + irUserCharCount] = ETX_CHAR;
	debug_printf("Sent from IR: %s\r\n", string);
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
	captureChar = 0x00;
	bitsReceived = 0;
	receivedSTX = 0;
	receivedChar = 0;
	receivedString = 0;
	irCharsReceived = 0;
	irUserCharCount = 0;
}

void handle_received_char(char rxInput) {

	switch(rxInput) {

		//Begin new string
		case STX_CHAR:
			receivedSTX = 1;
			irCharsReceived = 0;
			//memset(&rxBuffer[0], 0, sizeof(unsigned char) * 11);
			break;

		//End string
		case ETX_CHAR:
			if(receivedSTX) {
				receivedString = 1;
				receivedSTX = 0;
			}
			break;

		//Add to string
		default:
			if(receivedSTX) {
				if(irCharsReceived < 11) {
					rxBuffer[irCharsReceived++] = rxInput;
				} else {
					receivedString = 1;
					receivedSTX = 0;
				}
			} else {
				rxChar = rxInput;
				receivedChar = 1;
			}
			break;
	}
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
		GPIO_PinState pinState = HAL_GPIO_ReadPin(RX_INPUT_PORT, RX_INPUT_PIN);
		uint32_t captureValueDifference;
		if (currentCaptureValue > lastCaptureValue) {
			captureValueDifference = (currentCaptureValue - lastCaptureValue);
		} else if (lastCaptureValue < currentCaptureValue) {
			captureValueDifference = ((0xFFFF - lastCaptureValue) + currentCaptureValue) + 1;
		}

		if(!bitsReceived) {
			/* Input capture must be initialised by rising edge */
			if(pinState == GPIO_PIN_RESET) {
				lastCaptureValue = currentCaptureValue;
				bitsReceived++;
			}
		} else if(bitsReceived == 1) {
			//if(pinState == GPIO_PIN_RESET) {} else {}
			receivePeriod = captureValueDifference;
			bitsReceived++;
		} else if(bitsReceived == 2) {
			lastCaptureValue = currentCaptureValue;
			bitsReceived++;
		} else if(bitsReceived < 11) {
			if(captureValueDifference > 1.5 * receivePeriod) {
				captureChar = captureChar << 1;
				lastCaptureValue = currentCaptureValue;

				if(pinState == GPIO_PIN_RESET) {
					captureChar |= 0x01;
					//debug_printf("1\r\n");
				} else {
					captureChar &= 0xFE;
					//debug_printf("0\r\n");
				}

				bitsReceived++;
			}

		/* Stop bit */
		} else {
			if(captureValueDifference > 1.5 * receivePeriod) {
				lastCaptureValue = currentCaptureValue;
				//Check that stop bit is falling edge
				if(pinState == GPIO_PIN_SET) {
					handle_received_char(captureChar);
					//debug_printf("Handled char %c\r\n", captureChar);
				}
				bitsReceived = 0;
			}
		}
	}
}

void TIM2_IRQHandler(void) {
	HAL_TIM_IRQHandler(&rx_TIM_Init);
}

void ir_duplex_timer1_handler(void) {
	if(bitsToTransmit & (1 << transmitBit)) {
		s4435360_hal_ir_datamodulation_set();
	} else {
		s4435360_hal_ir_datamodulation_clr();
	}

	if(!transmitBit) {
		s4435360_hal_ir_datamodulation_clr();
		s4435360_hal_ir_carrier_off();
		HAL_TIM_Base_Stop_IT(&timer1Init);
		currentlyTransmitting = 0;
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
	if(receivedChar) {
		debug_printf("Received char from IR: %c\r\n", rxChar);
		receivedChar = 0;
	} else if(receivedString) {
		debug_printf("Received string from IR: ");
		for(int i = 0; i < irCharsReceived; i++) {
			debug_printf("%c", rxBuffer[i]);
		}
		debug_printf("\r\n");

		//memset(&rxBuffer[0], 0, sizeof(unsigned char) * 11);
		irCharsReceived = 0;
		receivedString = 0;
	}
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
			} else {
				debug_printf("Sent from IR: %c\r\n", input);
				send_char(input);
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
					send_string(irUserChars);
					irUserCharCount = 0;
					currentIRDuplexMode = START_MODE;
					memset(&irUserChars[0], 0, 11 * sizeof(char));
					return;
				}

				/* Check for backspace */
				if(input == BACKSPACE_CHAR) {
					if(irUserCharCount) {
						irUserCharCount--;
					}

					/* Handle general case */
				} else {
					irUserChars[irUserCharCount] = input;
					irUserCharCount++;
				}

				/* Check for packet completion */
				if(irUserCharCount >= MAXUSERCHARS) {
					send_string(irUserChars);
					irUserCharCount = 0;
					currentIRDuplexMode = START_MODE;
					memset(&irUserChars[0], 0, 11 * sizeof(char));
					return;
				}
			}
			break;
		}

}

void ir_duplex_timer2_handler(void){}
