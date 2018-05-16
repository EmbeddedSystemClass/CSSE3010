/**
  ******************************************************************************
  * @file    proj1/radio_duplex_mode.c
  * @author  Samuel Eadie - 44353607
  * @date    21032018-18042018
  * @brief   Duplex radio communication mode functionality for project 1
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "radio_duplex_mode.h"

#include "nrf24l01plus.h"
#include "radio_fsm.h"
#include "s4435360_hal_radio.h"
#include "s4435360_hal_hamming.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define TIMER_FREQUENCY 			50000
#define INTERRUPT_FREQUENCY 		10
#define PAYLOAD_STARTING_INDEX		10
#define PACKET_READY_TO_SEND		1
#define PACKET_NOT_READY_TO_SEND	0

unsigned char txAddress[5] = {0x52, 0x33, 0x22, 0x11, 0x00};
unsigned char rxAddress[5] = {0x07, 0x36, 0x35, 0x44, 0x00};
unsigned char channel = 52;
unsigned char packetHeader[10] = {0xA1,					//Packet type
		0x52, 0x33, 0x22, 0x11,					//Destination address
		0x07, 0x36, 0x35, 0x44,  				//Source address
		0x00};									//Blank
unsigned char ackPayload[5] = "A C K";
unsigned char errPayload[5] = "E R R";

//Payload and char counts for re/transmit
int radioUserCharCount = 0;
unsigned char radioUserChars[11];
int radioUserCharRetransmitCount = 0;
unsigned char radioUserCharsRetransmit[11];

//Flags
int receivedInvalidMessage = 0;
int receivedACK = 0;
int retransmitAttempts = 0;
int transmitACK = 0;
int transmitERR = 0;
int oldTxStatus;
int timerCounter = 0;

/* Private function prototypes -----------------------------------------------*/
void start_ACK_timer(void);
int isPacketACK(unsigned char* packet);
int isPacketERR(unsigned char* packet);

/**
 * @brief 	Forms a packet by appending the header and payloadLength
 * 			bytes from the payload to the packet
 * @param 	payload: the payload bytes to form the packet
 * 			payloadLength: the number of bytes in the payload
 * 			packet: a pointer to the packet to create
 * @retval 	None
 */
void form_packet(unsigned char* payload, int payloadLength, unsigned char* packet) {

	/* Add packet header */
	memcpy(packet, packetHeader, PAYLOAD_STARTING_INDEX);

	/* Encode and add payload */
	uint16_t encodedByte;
	for(int i = 0; i < payloadLength; i++) {
		encodedByte = hamming_byte_encoder(payload[i]);
		packet[PAYLOAD_STARTING_INDEX + (2 * i)] = (uint8_t)((encodedByte & 0xFF00) >> 8);
		packet[PAYLOAD_STARTING_INDEX + (2 * i) + 1] = (uint8_t)(encodedByte & 0x00FF);
	}
}

/**
 * @brief Prints the last sent user packet to console
 * @param None
 * @retval None
 */
void print_sent_packet(void) {

	debug_printf("Sent from Radio:  %c", '"');

	for(int i = 0; i < radioUserCharCount; i++) {
		debug_printf("%c", radioUserChars[i]);
	}

	debug_printf("%c\r\n", '"');
}

/**
 * @brief Prints the received decoded packet to console
 * @param packet_buffer: the decoded packet payload
 * @retval None
 */
void print_received_packet(uint8_t* packet_buffer) {
	debug_printf("Received from Radio:  %c", '"');

	for(int i = 0; i < 11; i++) {
		debug_printf("%c", packet_buffer[i]);
	}

	debug_printf("%c\r\n", '"');
}

/**
 * @brief Initialises the radio duplex mode
 * @param None
 * @retval None
 */
void radio_duplex_init(void) {

	debug_printf("Mode 6: Radio Duplex\r\n");
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
	s4435360_radio_init();

	/* Set rx address */
	radio_fsm_setstate(RADIO_FSM_IDLE_STATE);
	s4435360_radio_setrxaddress(rxAddress);

	/* No received packets, no packet to transmit */
	s4435360_radio_txstatus = 0;
	s4435360_radio_rxstatus = 0;

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

	//Init user payload
	radioUserCharCount = 0;
	radioUserCharRetransmitCount = 0;

	//Init flags
	receivedInvalidMessage = 0;
	receivedACK = 0;
	retransmitAttempts = 0;
	transmitACK = 0;
	transmitERR = 0;
	oldTxStatus = 0;
	timerCounter = 0;

}

/**
 * @brief Deinitialises the radio duplex mode
 * @param None
 * @retval None
 */
void radio_duplex_deinit(void) {
	HAL_TIM_Base_Stop_IT(&timer1Init);
	HAL_TIM_Base_Stop_IT(&timer2Init);
	HAL_TIM_Base_Stop_IT(&timer3Init);
}

/**
 * @brief The run function for the radio duplex mode
 * @param None
 * @retval None
 */
void radio_duplex_run(void) {
	lightbar_seg_set(SEND_INDICATOR_SEGMENT, 0);
	lightbar_seg_set(RECEIVE_INDICATOR_SEGMENT, 0);


	/* Check for transmission */
	if(s4435360_radio_gettxstatus()) {
		if(radio_fsm_getstate() == RADIO_FSM_TX_STATE) {

			//Send ACK
			if(transmitACK) {
				form_packet(ackPayload, 5, s4435360_tx_buffer);
				s4435360_radio_txstatus = oldTxStatus;
				transmitACK = 0;
				debug_printf("Sent from Radio: %cA C K%c\r\n", '"', '"');

			//Send ERR
			} else if (transmitERR) {
				form_packet(errPayload, 5, s4435360_tx_buffer);
				s4435360_radio_txstatus = oldTxStatus;
				transmitERR = 0;
				debug_printf("Sent from Radio: %cE R R%c\r\n", '"', '"');

			//Send packet - original or retransmission
			} else {
				form_packet(radioUserChars, radioUserCharCount, s4435360_tx_buffer);
				strcpy(radioUserCharsRetransmit, radioUserChars);
				radioUserCharRetransmitCount = radioUserCharCount;
				print_sent_packet();

				/* Reset packet */
				receivedACK = 0;
				s4435360_radio_txstatus = 0;
				radioUserCharCount = 0;

				start_ACK_timer();

			}

			s4435360_radio_sendpacket(channel,  txAddress, s4435360_tx_buffer);
			memset(&s4435360_tx_buffer[0], 0, 32);
			memset(&radioUserChars[0], 0, 11);
			lightbar_seg_set(SEND_INDICATOR_SEGMENT, 1);
		}
	}


	/* Check for received packet */
	if(s4435360_radio_getrxstatus()) {
		s4435360_radio_getpacket(s4435360_rx_buffer);

		lightbar_seg_set(RECEIVE_INDICATOR_SEGMENT, 1);

		//Decode packet
		unsigned char decodedOutput[11];
		HammingDecodedOutput hammingOutput;
		memset(&decodedOutput[0], 0, sizeof(decodedOutput));

		for(int i = 0; i < 11; i++) {
			uint16_t encodedDoubleByte = (s4435360_rx_buffer[2 * i] << 8) |
					(s4435360_rx_buffer[(2 * i) + 1]);
			if(!encodedDoubleByte) {
				break;
			}

			hammingOutput = hamming_byte_decoder(encodedDoubleByte);

			//Check for correct decoding
			if(hammingOutput.uncorrectableError) {
				receivedInvalidMessage = 1;
			} else {
				decodedOutput[i] = hammingOutput.decodedOutput;
			}
		}

		/* Check if message is an acknowledgement */
		if(isPacketACK(decodedOutput)) {
			receivedACK = 1;
			print_received_packet(decodedOutput);
		} else {

			/* Check if message is an ERR */
			if(isPacketERR(decodedOutput)) {
				receivedACK = 1;
				print_received_packet(decodedOutput);
				strcpy(radioUserChars, radioUserCharsRetransmit);
				radioUserCharCount = radioUserCharRetransmitCount;
				retransmitAttempts = 0;
				lightbar_seg_set(ERR_INDICATOR_SEGMENT, 1);

			/* Received general packet */
			} else {
				/* Check for uncorrectable decoding error */
				if(receivedInvalidMessage) {
					debug_printf("Received from Radio: 2-bit ERROR\r\n");
					transmitERR = 1;

				/* Received correct general packet */
				} else {
					print_received_packet(decodedOutput);
					transmitACK = 1;
					lightbar_seg_set(ERR_INDICATOR_SEGMENT, 0);
				}
			}

			/* Store tx status to restore after response reply */
			oldTxStatus = s4435360_radio_txstatus;
			s4435360_radio_txstatus = 1;
		}

		/* Reset RX variables */
		receivedInvalidMessage = 0;
		s4435360_radio_rxstatus = 0;
	}
}

/**
 * @brief The user input function for the radio duplex mode
 * @param userChars: The user input from the console
 * 		  userCharsReceived: The number of characters received
 * @retval None
 */
void radio_duplex_user_input(char* userChars, int userCharsReceived) {

	//Check for RTabc... pattern
	if((userChars[0] == 'R') && (userChars[1] == 'T') && userCharsReceived > 2) {
		s4435360_radio_txstatus = PACKET_READY_TO_SEND;
		retransmitAttempts = 0;
		radioUserCharCount = userCharsReceived - 2;
		strncpy(radioUserChars, &userChars[2], userCharsReceived);
	}
}

/**
 * @brief Configures and starts a 3s timer to time for ACK receipts
 * @param None
 * @retval None
 */
void start_ACK_timer(void) {
	__TIMER1_CLK_ENABLE();

	/* TIM Base configuration */
	timer1Init.Instance = TIMER1;
	timer1Init.Init.Period = 50000 / 100; //10Hz timer
	timer1Init.Init.Prescaler = (uint16_t) ((SystemCoreClock) / 50000) - 1;
	timer1Init.Init.ClockDivision = 0;
	timer1Init.Init.RepetitionCounter = 0;
	timer1Init.Init.CounterMode = TIM_COUNTERMODE_UP;

	/* Initialise Timer */
	HAL_TIM_Base_Init(&timer1Init);
	HAL_NVIC_SetPriority(TIMER1_IRQ, 10, 0);
	HAL_NVIC_EnableIRQ(TIMER1_IRQ);
	//__HAL_TIM_SET_COUNTER(&timer1Init, 0);
	HAL_TIM_Base_Start_IT(&timer1Init);

	lightbar_seg_set(ACK_INDICATOR_SEGMENT, 1);

}

/**
 * @brief Handler for timer 1, ACK receipt and retransmission
 * @param None
 * @retval None
 */
void radio_duplex_timer1_handler(void) {
	//Timer counter for 3s
	if(timerCounter < (3 * 100)) {
		timerCounter++;
		return;
	}

	//3s elapsed
	timerCounter = 0;

	//If received ACK or too many retransmits, stop
	if(receivedACK || (retransmitAttempts >= 2)) {
		if(!receivedACK) {
			debug_printf("After 3 transmits giving up\r\n");
		}
		receivedACK = 0;
		retransmitAttempts = 0;
		HAL_TIM_Base_Stop_IT(&timer1Init);
		s4435360_radio_txstatus = 0;
		lightbar_seg_set(ACK_INDICATOR_SEGMENT, 0);

	//Retransmit packet
	} else {
		//oldTxStatus = s4435360_radio_txstatus;
		s4435360_radio_txstatus = 1;
		strcpy(radioUserChars, radioUserCharsRetransmit);
		radioUserCharCount = radioUserCharRetransmitCount;
		retransmitAttempts++;
	}

}

/**
 * @brief Handler for timer 2, radio fsm processing
 * @param None
 * @retval None
 */
void radio_duplex_timer2_handler(void) {
	s4435360_radio_fsmprocessing();
}

//Timer 3 not used
void radio_duplex_timer3_handler(void){}

/**
 * @brief Checks if packet is ACK
 * @param packet: the packet to check
 * @retval Returns 1 if the packet is an ACK, 0 otherwise
 */
int isPacketACK(unsigned char* packet) {
	return (packet[0] == 'A') &&
			(packet[1] == ' ') &&
			(packet[2] == 'C') &&
			(packet[3] == ' ') &&
			(packet[4] == 'K');
}

/**
 * @brief Checks if packet is ERR
 * @param packet: the packet to check
 * @retval Returns 1 if the packet is an ERR, 0 otherwise
 */
int isPacketERR(unsigned char* packet) {
	return (packet[0] == 'E') &&
			(packet[1] == ' ') &&
			(packet[2] == 'R') &&
			(packet[3] == ' ') &&
			(packet[4] == 'R');
}
