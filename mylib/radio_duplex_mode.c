/**
  ******************************************************************************
  * @file    project1/radio_duplex_mode.c
  * @author  SE
  * @date    14032018-21032018
  * @brief   Radio duplex mode functionality for Project 1
  ******************************************************************************
  *
  */

/* Includes ------------------------------------------------------------------*/
#include "string.h"
#include "structures.h"
#include "radio_fsm.h"
#include "nrf24l01plus.h"
#include "s4435360_hal_radio.h"
#include "s4435360_hal_hamming.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define TIMER_FREQUENCY 			50000
#define INTERRUPT_FREQUENCY 		10
#define ENTER_CHAR					(char)(13)
#define BACKSPACE_CHAR				(char)(8)
#define SPACE_CHAR					(char)(32)
#define MAXUSERCHARS				11
#define PAYLOAD_STARTING_INDEX		10
#define PACKET_READY_TO_SEND		1
#define PACKET_NOT_READY_TO_SEND	0

#define START_MODE				0
#define RADIO_TRANSMIT_MODE		1
#define USER_INPUT_MODE			2
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
int currentRadioDuplexMode = START_MODE;
unsigned char txAddress[5] = {0x52, 0x33, 0x22, 0x11, 0x00};
unsigned char rxAddress[5] = {0x07, 0x36, 0x35, 0x44, 0x00};
unsigned char channel = 52;
int userCharCount = 0;
unsigned char packetHeader[10] = {0xA1,					//Packet type
		0x52, 0x33, 0x22, 0x11,					//Destination address
		0x07, 0x36, 0x35, 0x44,  				//Source address
		0x00};									//Blank
unsigned char ackPayload[5] = "A C K";
unsigned char errPayload[5] = "E R R";
unsigned char userInputs[11];
unsigned char userInputsRetransmit[11];
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

void print_sent_packet() {

	debug_printf("Sent from Radio:  %c", '"');

	for(int i = 0; i < 11; i++) {
		debug_printf("%c", userInputs[i]);
	}

	debug_printf("%c\r\n", '"');
}

void print_received_packet(uint8_t* packet_buffer) {
	debug_printf("Received from Radio:  %c", '"');

	for(int i = 0; i < 11; i++) {
		debug_printf("%c", packet_buffer[i]);
	}

	debug_printf("%c\r\n", '"');
}

void radio_duplex_init(void) {
	currentRadioDuplexMode = START_MODE;
	BRD_init();
	BRD_LEDInit();		//Initialise Blue LED
	/* Turn off LEDs */
	BRD_LEDRedOff();
	BRD_LEDGreenOff();
	BRD_LEDBlueOff();

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

	setbuf(stdout, NULL);

	/* Set rx address */
	radio_fsm_setstate(RADIO_FSM_IDLE_STATE);
	s4435360_radio_setrxaddress(rxAddress);
	s4435360_radio_txstatus = 0;
	s4435360_radio_rxstatus = 0;

}

void radio_duplex_deinit(void) {
	HAL_TIM_Base_Stop_IT(&timer1Init);
	HAL_TIM_Base_Stop_IT(&timer2Init);
}

void radio_duplex_run(void) {
	/* Check for transmission */
	if(s4435360_radio_gettxstatus()) {
		if(radio_fsm_getstate() == RADIO_FSM_TX_STATE) {
			if(transmitACK) {
				form_packet(ackPayload, 5, s4435360_tx_buffer);
				s4435360_radio_txstatus = oldTxStatus;
				transmitACK = 0;
				debug_printf("Sent from Radio: %cA C K%c\r\n", '"', '"');
			} else if (transmitERR) {
				form_packet(errPayload, 5, s4435360_tx_buffer);
				s4435360_radio_txstatus = oldTxStatus;
				transmitERR = 0;
				debug_printf("Sent from Radio: %cE R R%c\r\n", '"', '"');
			} else {
				form_packet(userInputs, 11, s4435360_tx_buffer);
				strcpy(userInputsRetransmit, userInputs);
				print_sent_packet();

				/* Reset packet */
				receivedACK = 0;
				s4435360_radio_txstatus = 0;
				userCharCount = 0;

				start_ACK_timer();

			}

			s4435360_radio_sendpacket(channel,  txAddress, s4435360_tx_buffer);
			memset(&s4435360_tx_buffer[0], 0, sizeof(s4435360_tx_buffer));
		}
	}


	/* Check for received packet */
	if(s4435360_radio_getrxstatus()) {
		s4435360_radio_getpacket(s4435360_rx_buffer);

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
				receivedInvalidMessage = 1;
			} else {
				decodedOutput[i] = hammingOutput.decodedOutput;
			}
		}

		/* Check if message is an acknolwedgement */
		if(isPacketACK(decodedOutput)) {
			receivedACK = 1;
			print_received_packet(decodedOutput);
		} else {
			if(isPacketERR(decodedOutput)) {
				receivedACK = 1;
				print_received_packet(decodedOutput);
				strcpy(userInputs, userInputsRetransmit);
				retransmitAttempts = 0;
			} else {
				if(receivedInvalidMessage) {
					debug_printf("Received from Radio: 2-bit ERROR\r\n");
					transmitERR = 1;
				} else {
					print_received_packet(decodedOutput);
					transmitACK = 1;
				}
			}

			/* Store tx status to restore after response reply */
			oldTxStatus = s4435360_radio_txstatus;
			s4435360_radio_txstatus = 1;
		}
		receivedInvalidMessage = 0;
		s4435360_radio_rxstatus = 0;
	}
}

void radio_duplex_user_input(char input) {
	switch(currentRadioDuplexMode) {

		case START_MODE:
			if(input == 'R') {
				currentRadioDuplexMode = RADIO_TRANSMIT_MODE;
			}
			break;

		case RADIO_TRANSMIT_MODE:
			if(input == 'T') {
				currentRadioDuplexMode = USER_INPUT_MODE;
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
					s4435360_radio_txstatus = PACKET_READY_TO_SEND;
					currentRadioDuplexMode = START_MODE;
					retransmitAttempts = 0;
					return;
				}

				/* Check for backspace */
				if(input == BACKSPACE_CHAR) {
					if(userCharCount) {
						userCharCount--;
					}

					/* Handle general case */
				} else {
					userInputs[userCharCount] = input;
					userCharCount++;
				}

				/* Check for packet completion */
				if(userCharCount >= MAXUSERCHARS) {
					s4435360_radio_txstatus = PACKET_READY_TO_SEND;
					currentRadioDuplexMode = START_MODE;
					retransmitAttempts = 0;
					return;
				} else {
					s4435360_radio_txstatus = PACKET_NOT_READY_TO_SEND;
					return;
				}
			}
		break;
	}

}

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
}

void radio_duplex_timer2_handler(void) {
	s4435360_radio_fsmprocessing();
}

void radio_duplex_timer1_handler(void) {
	if(timerCounter < (3 * 100)) {
		timerCounter++;
		return;
	}

	timerCounter = 0;
	if(receivedACK || (retransmitAttempts >= 2)) {
		receivedACK = 0;
		retransmitAttempts = 0;
		HAL_TIM_Base_Stop_IT(&timer1Init);
		s4435360_radio_txstatus = 0;
	} else {
		//oldTxStatus = s4435360_radio_txstatus;
		s4435360_radio_txstatus = 1;
		strcpy(userInputs, userInputsRetransmit);
		retransmitAttempts++;
	}

}

int isPacketACK(unsigned char* packet) {
	return (packet[0] == 'A') &&
			(packet[1] == ' ') &&
			(packet[2] == 'C') &&
			(packet[3] == ' ') &&
			(packet[4] == 'K');
}

int isPacketERR(unsigned char* packet) {
	return (packet[0] == 'E') &&
			(packet[1] == ' ') &&
			(packet[2] == 'R') &&
			(packet[3] == ' ') &&
			(packet[4] == 'R');
}
