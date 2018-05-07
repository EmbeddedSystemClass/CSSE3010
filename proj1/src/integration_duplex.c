/**
  ******************************************************************************
  * @file    proj1/encode_decode_mode.c
  * @author  Samuel Eadie - 44353607
  * @date    21032018-18042018
  * @brief   Provides integration duplex mode functionality for project 1
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "string.h"
#include "structures.h"
#include "radio_fsm.h"
#include "nrf24l01plus.h"
#include "s4435360_hal_radio.h"
#include "s4435360_hal_hamming.h"
#include "ir_duplex_mode.h"
#include "radio_duplex_mode.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
//Timer definitions
#define TIMER_FREQUENCY 			50000
#define INTERRUPT_FREQUENCY 		10

//Payload index definitions
#define PAYLOAD_STARTING_INDEX		10
#define PACKET_READY_TO_SEND		1
#define PACKET_NOT_READY_TO_SEND	0

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
//TX packet constants
unsigned char duplexTxAddress[5] = {0x52, 0x33, 0x22, 0x11, 0x00};
unsigned char duplexRxAddress[5] = {0x07, 0x36, 0x35, 0x44, 0x00};
unsigned char duplexChannel = 52;

//User inputs for re/transmission
int duplexUserCharCount = 0;
unsigned char duplexUserChars[11];
int duplexUserCharRetransmitCount = 0;
unsigned char duplexUserCharsRetransmit[11];

//Flags
int duplexReceivedACK = 0;
int duplexReceivedInvalidMessage = 0;
int duplexRetransmitAttempts = 0;
int duplexTimerCounter = 0;
/* Private function prototypes -----------------------------------------------*/
void start_ACK_timer3(void);


/**
 * @brief Handles received characters from input capture
 * @param input: the received characters
 * @retval None
 */
void handle_received_ack(uint16_t input) {

	char rxInput = (char)(input);

	if((rxInput == ACK_CHAR) || (rxInput == NACK_CHAR)) {
		rxChar = rxInput;
		receivedChar = 1;
	}

}


/**
 * @brief Initialises the duplex functionality for integration challenge
 * @param None
 * @retval None
 */
void integration_duplex_init(void) {

	debug_printf("Mode 7: Full Duplex\r\n");
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

	__TIMER2_CLK_ENABLE();

	/* TIM Base configuration */
	timer2Init.Instance = TIMER2;
	timer2Init.Init.Period = TIMER_FREQUENCY / INTERRUPT_FREQUENCY;
	timer2Init.Init.Prescaler = (uint16_t) ((SystemCoreClock) / TIMER_FREQUENCY) - 1;
	timer2Init.Init.ClockDivision = 0;
	timer2Init.Init.RepetitionCounter = 0;
	timer2Init.Init.CounterMode = TIM_COUNTERMODE_UP;

	/* Initialise Timer */
	HAL_TIM_Base_Init(&timer2Init);
	HAL_NVIC_SetPriority(TIMER2_IRQ, 10, 0);
	HAL_NVIC_EnableIRQ(TIMER2_IRQ);
	HAL_TIM_Base_Start_IT(&timer2Init);

	/* Set rx address */
	radio_fsm_setstate(RADIO_FSM_IDLE_STATE);
	s4435360_radio_setrxaddress(duplexRxAddress);

	/* No received packets, no packet to transmit */
	s4435360_radio_txstatus = 0;
	s4435360_radio_rxstatus = 0;


	ir_rx_init();
	configure_receive(8, &handle_received_ack);

	ir_timer1_init();

	//Init flags
	duplexReceivedACK = 0;
	duplexReceivedInvalidMessage = 0;
	duplexRetransmitAttempts = 0;
	duplexTimerCounter = 0;

}

/**
 * @brief Deinitialises the duplex functionality for integration challenge
 * @param None
 * @retval None
 */
void integration_duplex_deinit(void) {
	HAL_TIM_Base_Stop_IT(&timer1Init);
	HAL_TIM_Base_Stop_IT(&timer2Init);
	ir_duplex_deinit();
}

/**
 * @brief The run function for the duplex integration challenge
 * @param None
 * @retval None
 */
void integration_duplex_run(void) {

	lightbar_seg_set(RECEIVE_INDICATOR_SEGMENT, 0);
	lightbar_seg_set(SEND_INDICATOR_SEGMENT, 0);

	/* Check for transmission */
	if(s4435360_radio_gettxstatus()) {
		if(radio_fsm_getstate() == RADIO_FSM_TX_STATE) {
			form_packet(duplexUserChars, duplexUserCharCount, s4435360_tx_buffer);

			//Copy for possible retransmit
			strcpy(duplexUserCharsRetransmit, duplexUserChars);
			duplexUserCharRetransmitCount = duplexUserCharCount;

			//Print sent packet
			debug_printf("Sent from Radio: ");
			for(int i = 0; i < duplexUserCharCount; i++) {
				debug_printf("%c", duplexUserChars[i]);
			}
			debug_printf("\r\n");

			/* Reset packet */
			duplexReceivedACK = 0;
			s4435360_radio_txstatus = 0;
			duplexUserCharCount = 0;

			start_ACK_timer3();

			s4435360_radio_sendpacket(duplexChannel,  duplexTxAddress, s4435360_tx_buffer);
			memset(&s4435360_tx_buffer[0], 0, 32);
			lightbar_seg_set(SEND_INDICATOR_SEGMENT, 1);
		}
	}


	/* Check for received packet */
	if(s4435360_radio_getrxstatus()) {

		s4435360_radio_getpacket(s4435360_rx_buffer);

		//Decode packet
		unsigned char decodedOutput[11];
		HammingDecodedOutput hammingOutput;
		memset(&decodedOutput[0], 0, sizeof(decodedOutput));

		for(int i = 0; i < 11; i++) {
			uint16_t encodedDoubleByte = (s4435360_rx_buffer[10 + (2 * i)] << 8) |
					(s4435360_rx_buffer[10 + (2 * i) + 1]);
			if(!encodedDoubleByte) {
				break;
			}

			hammingOutput = hamming_byte_decoder(encodedDoubleByte);

			if(hammingOutput.uncorrectableError) {
				duplexReceivedInvalidMessage = 1;
			} else {
				decodedOutput[i] = hammingOutput.decodedOutput;
			}
		}

		/* Check for uncorrectable decoding error */
		if(duplexReceivedInvalidMessage) {
			debug_printf("Received from Radio: 2-bit ERROR\r\n");
			send_char(NACK_CHAR);
			debug_printf("Sent from IR: NACK\r\n");

		/* Received correct packet */
		} else {
			print_received_packet(decodedOutput);
			send_char(ACK_CHAR);
			debug_printf("Sent from IR: ACK\r\n");
		}

		/* Reset RX variables */
		duplexReceivedInvalidMessage = 0;
		s4435360_radio_rxstatus = 0;

		lightbar_seg_set(RECEIVE_INDICATOR_SEGMENT, 1);
	}

	/* Check for IR receive acknowledgements */
	if(receivedChar) {
		if(rxChar == ACK_CHAR) {
			debug_printf("Received from IR: ACK\r\n");
			duplexReceivedACK = 1;
		} else if (rxChar == NACK_CHAR) {
			debug_printf("Received from IR: NACK\r\n");
			duplexReceivedACK = 1;
			strcpy(duplexUserChars, duplexUserCharsRetransmit);
			duplexUserCharCount = duplexUserCharRetransmitCount;
			duplexRetransmitAttempts = 0;
		}

		receivedChar = 0;

	}
}

/**
 * @brief The user input function for the integration duplex mode
 * @param userChars: the chars received from the console
 * 		   userCharsReceived: the number of chars received
 * @retval None
 */
void integration_duplex_user_input(char* userChars, int userCharsReceived) {

	//Check for DTabc... pattern
	if((userChars[0] == 'D') && (userChars[1] == 'T') && userCharsReceived > 2) {
		s4435360_radio_txstatus = PACKET_READY_TO_SEND;
		duplexRetransmitAttempts = 0;
		duplexUserCharCount = userCharsReceived - 2;
		strncpy(duplexUserChars, &userChars[2], duplexUserCharCount);
	}
}

/**
 * @brief Configures and starts a 3s timer to time for ACK receipts
 * @param None
 * @retval None
 */
void start_ACK_timer3(void) {
	__TIMER3_CLK_ENABLE();

	/* TIM Base configuration */
	timer3Init.Instance = TIMER3;
	timer3Init.Init.Period = 50000 / 100; //10Hz timer
	timer3Init.Init.Prescaler = (uint16_t) ((SystemCoreClock) / 50000) - 1;
	timer3Init.Init.ClockDivision = 0;
	timer3Init.Init.RepetitionCounter = 0;
	timer3Init.Init.CounterMode = TIM_COUNTERMODE_UP;

	/* Initialise Timer */
	HAL_TIM_Base_Init(&timer3Init);
	HAL_NVIC_SetPriority(TIMER3_IRQ, 10, 0);
	HAL_NVIC_EnableIRQ(TIMER3_IRQ);
	HAL_TIM_Base_Start_IT(&timer3Init);

	lightbar_seg_set(ACK_INDICATOR_SEGMENT, 1);
}

/**
 * @brief Handler for timer 2, radio fsm processing
 * @param None
 * @retval None
 */
void integration_duplex_timer2_handler(void) {
	s4435360_radio_fsmprocessing();
}

/**
 * @brief Handler for timer 1, ACK receipt and retransmission
 * @param None
 * @retval None
 */
void integration_duplex_timer1_handler(void) {
	ir_duplex_timer1_handler();
}

/**
 * @brief Handler for timer 3, ACK retransmission timer
 * @param None
 * @retval None
 */
void integration_duplex_timer3_handler(void) {
	//Timer counter for 3s
	if(duplexTimerCounter < (3 * 100)) {
		duplexTimerCounter++;
		return;
	}

	//3s elapsed
	duplexTimerCounter = 0;

	//If received ACK or too many retransmits, stop
	if(duplexReceivedACK || (duplexRetransmitAttempts >= 2)) {
		if(duplexRetransmitAttempts >= 2) {
			debug_printf("After 3 transmits giving up\r\n");
		}

		duplexReceivedACK = 0;
		duplexRetransmitAttempts = 0;
		HAL_TIM_Base_Stop_IT(&timer3Init);
		s4435360_radio_txstatus = 0;

		lightbar_seg_set(ACK_INDICATOR_SEGMENT, 0);

	//Retransmit packet
	} else {
		s4435360_radio_txstatus = 1;
		strcpy(duplexUserChars, duplexUserCharsRetransmit);
		duplexUserCharCount = duplexUserCharRetransmitCount;
		duplexRetransmitAttempts++;
	}

}
