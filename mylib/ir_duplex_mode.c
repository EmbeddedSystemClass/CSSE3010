/**
  ******************************************************************************
  * @file    proj1/ir_duplex_mode.c
  * @author  Samuel Eadie - 44353607
  * @date    21032018-18042018
  * @brief   Provides IR duplex mode functionality for project 1
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include <s4435360_hal_ircomms.h>
#include "structures.h"
#include "ir_duplex_mode.h"
#include "s4435360_hal_ir.h"
#include <string.h>

#define TRANSMIT_HEADER_BITS		(uint64_t)(0b0101000000000000000010)

//Payload re/transmit variables
char irUserCharCount = 0;
char iruserCharCountRetransmit = 0;
char irUserChars[11];
char irUserCharsRetransmit[11];

//Transmit variables
uint64_t bitsToTransmit = TRANSMIT_HEADER_BITS;
int transmitBit;

//Flags
int txFlag = 0;
int receivedIrAck, IRretransmitAttempts;

//Receive timer input capture variables
TIM_HandleTypeDef rx_TIM_Init;
TIM_IC_InitTypeDef sICConfig;
uint16_t PrescalerValue = 0;
uint32_t TIMxCLKfreq = 16000000; //SystemCoreClock;

/* Received Captured Values */
uint32_t lastCaptureValue = 0;
uint16_t captureChar = 0x0000;
int bitsReceived, receivedSTX, receivedString, irCharsReceived;
unsigned char rxBuffer[11];
unsigned char rxChar;
int irAckTimerCounter;

/* Receive configuration variables */
int receiveBitLength;
void (*rxCharHandler)(uint16_t);

/**
  * @brief Configures IR transmission
  * @param bits: bits to transmit
  * 	   index: starting index of bit transmission
  * 	   irPeriod: period for ir transmission
  * @retval None
  */
void configure_transmission(uint64_t bits, int index, uint16_t irPeriod) {
	bitsToTransmit = bits;
	transmitBit = index;
	__HAL_TIM_SET_AUTORELOAD(&timer1Init, irPeriod);
}

/**
  * @brief Configures IR receive
  * @param length: expected bit length of packets
  * 	   charHandler: function to handle received chars
  * @retval None
  */
void configure_receive(int length, void (*charHandler)(uint16_t)) {
	receiveBitLength = length;
	rxCharHandler = charHandler;
}

/**
  * @brief Instigates sending a char. Returns 1 iff
  * 		no char is currently being sent and the
  * 		input char was successfully instigated.
  * 		Timer interrupt sends character
  * @param input: character to transmit
  * @retval whether the character could successfully
  * 		be transmitted
  */
void send_char(char input) {

	s4435360_hal_ir_carrier_on();

	//Configures transmission
	uint64_t bits = TRANSMIT_HEADER_BITS | ((uint64_t)(s4435360_hal_manchester_byte_encoder(input)) << 2);
	configure_transmission(bits, 21, 50000 / 100);

	//Transmit char
	currentlyTransmitting = 1;
	HAL_TIM_Base_Start_IT(&timer1Init);
	while(currentlyTransmitting) {}
	s4435360_hal_ir_carrier_off();
	HAL_Delay(10);

}

/**
  * @brief Sends a string over IR
  * @param string: string to send
  * 	   numChars: number of chars from string to send
  * @retval None
  */
void send_string(char* string, int numChars) {
	lightbar_seg_set(SEND_INDICATOR_SEGMENT, 1);

	//Send and print string
	send_char(STX_CHAR);
	debug_printf("Sent from IR: ");
	for(int i = 0; i < numChars; i++) {
		send_char(string[i]);
		debug_printf("%c", string[i]);
	}
	debug_printf("\r\n");
	send_char(ETX_CHAR);

	lightbar_seg_set(SEND_INDICATOR_SEGMENT, 0);
}

/**
  * @brief Initialises ir duplex mode
  * @param None
  * @retval None
  */
void ir_duplex_init(void) {
	debug_printf("Mode 5: IR Duplex\r\n");
	ir_timer1_init();

	//Configure IR receive
	ir_rx_init();
	configure_receive(8, &handle_received_char);

	lastCaptureValue = HAL_TIM_ReadCapturedValue(&rx_TIM_Init, TIM_CHANNEL_4);

	//Init tx and rx variables
	captureChar = 0x0000;
	bitsReceived = 0;
	IRretransmitAttempts = 0;
	currentlyTransmitting = 0;

	//Init flags
	receivedSTX = 0;
	receivedChar = 0;
	receivedString = 0;
	irCharsReceived = 0;
	txFlag = 0;
	receivedIrAck = 0;
}

/**
  * @brief Initialises timer 1 for IR transmission
  * @param None
  * @retval None
  */
void ir_timer1_init(void) {
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

/**
  * @brief Initialises IR receive input capture
  * @param None
  * @retval None
  */
void ir_rx_init(void) {

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
}

/**
  * @brief Processes chars received from IR
  * @param rxinput: char from IR
  * @retval None
  */
void handle_received_char(uint16_t input) {

	char rxInput = (char)(input);
	debug_printf("Received char: %c\r\n", rxInput);

	switch(rxInput) {

		//Begin new string
		case STX_CHAR:
			receivedSTX = 1;
			irCharsReceived = 0;
			break;

		//End string
		case ETX_CHAR:
			if(receivedSTX) {
				receivedString = 1;
				receivedSTX = 0;
			}
			break;

		//General character
		default:

			//Add to string
			if(receivedSTX) {

				//Space in buffer
				if(irCharsReceived < 11) {
					rxBuffer[irCharsReceived++] = rxInput;
				} else {
					receivedString = 1;
					receivedSTX = 0;
				}

			//Lone character
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

		//Record capture compare
		uint32_t currentCaptureValue = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_4);
		GPIO_PinState pinState = HAL_GPIO_ReadPin(RX_INPUT_PORT, RX_INPUT_PIN);

		//Calculate compare difference
		uint32_t captureValueDifference;
		if (currentCaptureValue > lastCaptureValue) {
			captureValueDifference = (currentCaptureValue - lastCaptureValue);
		} else if (lastCaptureValue < currentCaptureValue) {
			captureValueDifference = ((0xFFFF - lastCaptureValue) + currentCaptureValue) + 1;
		}

		//First bit
		if(!bitsReceived) {
			/* Input capture must be initialised by rising edge */
			if(pinState == GPIO_PIN_RESET) {
				lastCaptureValue = currentCaptureValue;
				bitsReceived++;
				captureChar = 0x0000;
			}

		//Second half bit
		} else if(bitsReceived == 1) {
			receivePeriod = captureValueDifference;
			bitsReceived++;

		//Third trigger, 2 bit
		} else if(bitsReceived == 2) {
			lastCaptureValue = currentCaptureValue;
			bitsReceived++;

		//Remaining bits
		} else if(bitsReceived < receiveBitLength + 3) {
			if(captureValueDifference > 1.5 * receivePeriod) {
				captureChar = captureChar << 1;
				lastCaptureValue = currentCaptureValue;

				//Received 1
				if(pinState == GPIO_PIN_RESET) {
					captureChar |= 0x0001;
					debug_printf("");

				//Received 0
				} else {
					captureChar &= 0xFFFE;
					debug_printf("");
				}

				bitsReceived++;
			}

		/* Stop bit */
		} else {
			if(captureValueDifference > 1.5 * receivePeriod) {
				lastCaptureValue = currentCaptureValue;

				//Check that stop bit is falling edge
				if(pinState == GPIO_PIN_SET) {
					rxCharHandler(captureChar);
				}
				bitsReceived = 0;
				ir_rx_init();
			}
		}
	}
}

/**
  * @brief TIM2 handler, rx input capture timer
  * @param None
  * @retval None
  */
void TIM2_IRQHandler(void) {
	HAL_TIM_IRQHandler(&rx_TIM_Init);
}

/**
  * @brief Timer 1 handler, IR transmission
  * @param None
  * @retval None
  */
void ir_duplex_timer1_handler(void) {

	//Modulate wave
	if(bitsToTransmit & ((uint64_t)1 << transmitBit)) {
		s4435360_hal_ir_datamodulation_set();
	} else {
		s4435360_hal_ir_datamodulation_clr();
	}

	//Check for last bit
	if(!transmitBit) {

		//Deinit character transmission
		s4435360_hal_ir_datamodulation_clr();
		s4435360_hal_ir_carrier_off();
		HAL_TIM_Base_Stop_IT(&timer1Init);
		currentlyTransmitting = 0;
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
	HAL_TIM_Base_Stop_IT(&timer2Init);
	s4435360_hal_ir_carrier_off();
}

/**
  * @brief IR duplex mode run function
  * @param None
  * @retval None
  */
void ir_duplex_run(void) {
	lightbar_seg_set(SEND_INDICATOR_SEGMENT, 0);
	lightbar_seg_set(RECEIVE_INDICATOR_SEGMENT, 0);

	//Check receive char flag
	if(receivedChar) {

		//Received ACK
		if(rxChar == ACK_CHAR) {
			debug_printf("Received from IR: ACK\r\n");
			receivedIrAck = 1;

		//Received general char
		} else if (((rxChar >= 'a') && (rxChar <= 'z')) ||
				((rxChar >= 'A') && (rxChar <= 'Z')) ||
				(rxChar == SPACE_CHAR)) {
			debug_printf("Received char from IR: %c\r\n", rxChar);
		}
		receivedChar = 0;

    //Check receive string flag
	} else if(receivedString) {
		debug_printf("Received string from IR: ");
		for(int i = 0; i < irCharsReceived; i++) {
			debug_printf("%c", rxBuffer[i]);
		}
		debug_printf("\r\n");

		send_char(ACK_CHAR);
		debug_printf("Sent from IR: ACK\r\n");

		irCharsReceived = 0;
		receivedString = 0;

		lightbar_seg_set(RECEIVE_INDICATOR_SEGMENT, 1);
	}

	//Check tx flag
	if(txFlag) {
		send_string(irUserChars, irUserCharCount);
		receivedIrAck = 0;
		strcpy(irUserCharsRetransmit, irUserChars);
		memset(&irUserChars[0], 0, 11 * sizeof(char));
		txFlag = 0;
		ir_ack_init();
	}
}

/**
  * @brief Handles user input for ir duplex mode
  * @param input: the user input to handle
  * @retval None
  */
void ir_duplex_user_input(char* userChars, int userCharsReceived) {

	//Check for ITabc... pattern
	if((userChars[0] == 'I') && (userChars[1] == 'T') && userCharsReceived > 2){
		txFlag = 1;
		IRretransmitAttempts = 0;
		irUserCharCount = userCharsReceived - 2;
		strncpy(irUserChars, &userChars[2], userCharsReceived);

	//Else send single character
	} else {
		debug_printf("Sent from IR: %c\r\n", userChars[0]);
		send_char(userChars[0]);
	}
}

/**
  * @brief Initialises ACK retransmission timer
  * @param None
  * @retval None
  */
void ir_ack_init(void) {
	__TIMER2_CLK_ENABLE();

	/* TIM Base configuration */
	timer2Init.Instance = TIMER2;
	timer2Init.Init.Period = 50000 / 100; //10Hz timer
	timer2Init.Init.Prescaler = (uint16_t) ((SystemCoreClock) / 50000) - 1;
	timer2Init.Init.ClockDivision = 0;
	timer2Init.Init.RepetitionCounter = 0;
	timer2Init.Init.CounterMode = TIM_COUNTERMODE_UP;

	/* Initialise Timer */
	HAL_TIM_Base_Init(&timer2Init);
	HAL_NVIC_SetPriority(TIMER2_IRQ, 10, 0);
	HAL_NVIC_EnableIRQ(TIMER2_IRQ);
	HAL_TIM_Base_Start_IT(&timer2Init);
	irAckTimerCounter = 0;
	receivedIrAck = 0;


	lightbar_seg_set(ACK_INDICATOR_SEGMENT, 1);
}

/**
  * @brief Handler for ACK, retransmission
  * @param None
  * @retval None
  */
void ir_duplex_timer2_handler(void){
	//Timer counter for 3s
	if(irAckTimerCounter < (3 * 100)) {
		irAckTimerCounter++;
		return;
	}

	//3s elapsed
	irAckTimerCounter = 0;

	//If received ACK or too many retransmits, stop
	if(receivedIrAck || (IRretransmitAttempts >= 2)) {
		if(!receivedIrAck) {
			debug_printf("After 3 transmits giving up\r\n");
		}

		receivedIrAck = 0;
		HAL_TIM_Base_Stop_IT(&timer2Init);
		txFlag = 0;
		lightbar_seg_set(ACK_INDICATOR_SEGMENT, 0);

	//Retransmit packet
	} else {
		txFlag = 1;
		strcpy(irUserChars, irUserCharsRetransmit);
		IRretransmitAttempts++;
	}
}

void ir_duplex_timer3_handler(void){}
