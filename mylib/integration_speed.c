/**
  ******************************************************************************
  * @file    project1/integration_speed_mode.c
  * @author  SE
  * @date    14032018-21032018
  * @brief   Variable speed functionality for integration mode for Project 1
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
#include "s4435360_hal_ir.h"
#include "ir_duplex_mode.h"
#include "radio_duplex_mode.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define TIMER_FREQUENCY 				50000
#define INTERRUPT_FREQUENCY 			10
#define PAYLOAD_STARTING_INDEX			10
#define PACKET_READY_TO_SEND			1
#define PACKET_NOT_READY_TO_SEND		0
#define TX_ERROR_INDEX					PAYLOAD_STARTING_INDEX + 1
#define RX_ERROR_INDEX					PAYLOAD_STARTING_INDEX + 2
#define BLAST_LETTER 					'a'
#define STARTING_TRANSMIT_BIT 			38
#define HAMMING_PAYLOAD_HEADER_BITS 	0x1400000002
#define TX_ERROR_STARTING_INDEX			PAYLOAD_STARTING_INDEX + 7
#define RX_ERROR_STARTING_INDEX			PAYLOAD_STARTING_INDEX + 19
#define MAX_TX_ERROR					10
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
unsigned char speedTxAddress[5] = {0x52, 0x33, 0x22, 0x11, 0x00};
unsigned char speedRxAddress[5] = {0x07, 0x36, 0x35, 0x44, 0x00};
unsigned char speedChannel = 52;
unsigned char errorPacketHeader[10] = {0xA1,					//Packet type
		0x52, 0x33, 0x22, 0x11,					//Destination address
		0x07, 0x36, 0x35, 0x44,  				//Source address
		0x00};

int charBlastOn = 0;
uint8_t txErrors = 0, rxErrors = 0;
uint16_t txRate, rxRate;

int packetsReceived = 0, packetsSent = 0;

int receivedRadioPacket = 0;

uint64_t transmitBits = HAMMING_PAYLOAD_HEADER_BITS;
int transmitBitIndex = STARTING_TRANSMIT_BIT;


/* Private function prototypes -----------------------------------------------*/

void form_error_packet(unsigned char* packet) {
	/* Add packet header */
	memcpy(packet, errorPacketHeader, PAYLOAD_STARTING_INDEX);

	/* Create payload */
	char payload[22];
	sprintf(payload, "Errors %3d\r\nErrors %3d", txErrors, rxErrors);

	memcpy(&packet[PAYLOAD_STARTING_INDEX], payload, 22);
}

uint8_t reverse_endianness(uint8_t toReverse) {
   toReverse = (toReverse & 0xF0) >> 4 | (toReverse & 0x0F) << 4;
   toReverse = (toReverse & 0xCC) >> 2 | (toReverse & 0x33) << 2;
   toReverse = (toReverse & 0xAA) >> 1 | (toReverse & 0x55) << 1;
   return toReverse;
}

uint16_t flip_hamming(uint16_t original) {
	return (uint16_t)(reverse_endianness((uint8_t)(original >> 8))) << 8 |
			(uint16_t)(reverse_endianness((uint8_t)original));
}

void send_hamming_char(char c) {
	s4435360_hal_ir_carrier_on();
	uint16_t hammingEncoded = flip_hamming(hamming_byte_encoder(c));

	uint16_t manchesterEncoded1 = s4435360_hal_manchester_byte_encoder((uint8_t)(hammingEncoded >> 8));
	uint16_t manchesterEncoded2 = s4435360_hal_manchester_byte_encoder((uint8_t)hammingEncoded);

	//debug_printf("%X --> %X, %X --> %X\r\n", (uint8_t)(hammingEncoded >> 8), manchesterEncoded1, (uint8_t)hammingEncoded, manchesterEncoded2);

	transmitBits = HAMMING_PAYLOAD_HEADER_BITS | ((uint64_t)manchesterEncoded1 << 18) | ((uint64_t)manchesterEncoded2 << 2);

	//debug_printf("---%X\r\n", transmitBits);
	configure_transmission(transmitBits, STARTING_TRANSMIT_BIT, 50000 / txRate);
	currentlyTransmitting = 1;
	HAL_TIM_Base_Start_IT(&timer1Init);
	while(currentlyTransmitting) {}
	s4435360_hal_ir_carrier_off();
	HAL_Delay(10); //A659 695A

}

void hamming_char_handler(uint16_t capture) {
	HammingDecodedOutput output = hamming_byte_decoder(capture);

	if(output.uncorrectableError) {
		rxErrors += 2;
	}

	if(output.errorMask) {
		rxErrors++;
	}

	packetsReceived++;

	rxRate = PERIOD_REGISTER_TIME_SCALAR / receivePeriod;
	debug_printf("Received from IR: %c\r\n", output.decodedOutput);
	debug_printf("Receive Rate: %d\r\n", rxRate);

}

/**
 * @brief Initialises the duplex functionality for integration challenge
 * @param None
 * @retval None
 */
void integration_speed_init(void) {

	debug_printf("Integration Speed mode\r\n");
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

	__TIMER3_CLK_ENABLE();

	/* TIM Base configuration */
	timer3Init.Instance = TIMER3;
	timer3Init.Init.Period = 50000;
	timer3Init.Init.Prescaler = (uint16_t) ((SystemCoreClock) / 50000) - 1;
	timer3Init.Init.ClockDivision = 0;
	timer3Init.Init.RepetitionCounter = 0;
	timer3Init.Init.CounterMode = TIM_COUNTERMODE_UP;

	/* Initialise Timer */
	HAL_TIM_Base_Init(&timer3Init);
	HAL_NVIC_SetPriority(TIMER3_IRQ, 10, 0);
	HAL_NVIC_EnableIRQ(TIMER3_IRQ);
	HAL_TIM_Base_Start_IT(&timer3Init);

	/* Set rx address */
	radio_fsm_setstate(RADIO_FSM_IDLE_STATE);
	s4435360_radio_setrxaddress(speedRxAddress);

	/* No received packets, no packet to transmit */
	s4435360_radio_txstatus = 0;
	s4435360_radio_rxstatus = 0;


	ir_rx_init();
	ir_timer1_init();
	txRate = 100;
	receivedRadioPacket = 0;
	configure_receive(16, &hamming_char_handler);

}

/**
 * @brief Deinitialises the duplex functionality for integration challenge
 * @param None
 * @retval None
 */
void integration_speed_deinit(void) {
	HAL_TIM_Base_Stop_IT(&timer1Init);
	HAL_TIM_Base_Stop_IT(&timer2Init);
	HAL_TIM_Base_Stop_IT(&timer3Init);
	ir_duplex_deinit();
	charBlastOn = 0;
}

/**
 * @brief The run function for the duplex integration challenge
 * @param None
 * @retval None
 */
void integration_speed_run(void) {

	if(!charBlastOn) {
		return;
	}

	lightbar_seg_set(SEND_INDICATOR_SEGMENT, 0);
	lightbar_seg_set(RECEIVE_INDICATOR_SEGMENT, 0);

	/* Check for transmission */
	if(s4435360_radio_gettxstatus()) {
		if(radio_fsm_getstate() == RADIO_FSM_TX_STATE) {
			form_error_packet(s4435360_tx_buffer);

			//Reset packet
			s4435360_radio_txstatus = 0;
			s4435360_radio_sendpacket(speedChannel,  speedTxAddress, s4435360_tx_buffer);
			memset(&s4435360_tx_buffer[0], 0, sizeof(s4435360_tx_buffer));

			lightbar_seg_set(SEND_INDICATOR_SEGMENT, 1);
		}
	}

	/* Check for received packet */
	if(s4435360_radio_getrxstatus()) {
		s4435360_radio_getpacket(s4435360_rx_buffer);

		//Calculate tx and rx errors
		uint16_t txError = 0, rxError = 0;
		for(int i = 0; i < 3; i++) {
			txError *= 10;
			rxError *= 10;

			if(s4435360_rx_buffer[TX_ERROR_STARTING_INDEX + i] != SPACE_CHAR) {
				txError += (s4435360_rx_buffer[TX_ERROR_STARTING_INDEX + i] - '0');
			}

			if(s4435360_rx_buffer[RX_ERROR_STARTING_INDEX + i] != SPACE_CHAR) {
				rxError += (s4435360_rx_buffer[RX_ERROR_STARTING_INDEX + i] - '0');
			}

		}

		txErrors = txError;
		rxErrors = rxError;

		//Reset RX variables
		receivedRadioPacket = 1;
		s4435360_radio_rxstatus = 0;

		lightbar_seg_set(RECEIVE_INDICATOR_SEGMENT, 1);
	}

	if(packetsReceived + packetsSent >= 10) {
		debug_printf("Tx Rate: %d Rx Rate: %d\r\nTx Errs: %d Rx Errs: %d\r\n",
				txRate, PERIOD_REGISTER_TIME_SCALAR / receivePeriod,
				txErrors, rxErrors);
		packetsReceived = 0; // %= 10;
		packetsSent = 0; // %= 10;
	}

	if(!currentlyTransmitting) {
		send_hamming_char(BLAST_LETTER);
		packetsSent++;
		lightbar_seg_set(SEND_INDICATOR_SEGMENT, 1);
	}
}

/**
 * @brief The user input function for the integration duplex mode
 * @param userChars: the chars received from the console
 * 		   userCharsReceived: the number of chars received
 * @retval None
 */
void integration_speed_user_input(char* userChars, int userCharsReceived) {
	charBlastOn = 1 - charBlastOn;
}


/**
 * @brief Handler for timer 2, radio fsm processing
 * @param None
 * @retval None
 */
void integration_speed_timer2_handler(void) {
	s4435360_radio_fsmprocessing();
}

/**
 * @brief Handler for timer 1, ACK receipt and retransmission
 * @param None
 * @retval None
 */
void integration_speed_timer1_handler(void) {
	ir_duplex_timer1_handler();
}


//Should be set up to trigger every second
void integration_speed_timer3_handler(void){
	s4435360_radio_txstatus = 1;

	if(!receivedRadioPacket) {
		txErrors = packetsSent;
	}

	receivedRadioPacket = 0;

	if(txErrors > MAX_TX_ERROR) {
		txRate /= 2;
	} else {
		txRate *= 2;
	}
}
