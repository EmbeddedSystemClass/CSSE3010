/**
  ******************************************************************************
  * @file    proj1/integration_speed.c
  * @author  Samuel Eadie - 44353607
  * @date    21032018-18042018
  * @brief   Provides variable speed mode functionality for project 1
  ******************************************************************************
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
//Timer definitions
#define TIMER_FREQUENCY 				50000
#define INTERRUPT_FREQUENCY 			10

//Payload definitions
#define PAYLOAD_STARTING_INDEX			10
#define TX_ERROR_STARTING_INDEX			PAYLOAD_STARTING_INDEX + 7
#define RX_ERROR_STARTING_INDEX			PAYLOAD_STARTING_INDEX + 19

//Transmit definitions
#define BLAST_LETTER 					'a'
#define STARTING_TRANSMIT_BIT 			38
#define HAMMING_PAYLOAD_HEADER_BITS 	0x1400000002
#define MAX_TX_ERROR					5
#define MAX_TX_RATE						1000
#define MIN_TX_RATE						5
#define TX_RATE_MULTIPLIER				1.2
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
//Transmit constants
unsigned char speedTxAddress[5] = {0x52, 0x33, 0x22, 0x11, 0x00};
unsigned char speedRxAddress[5] = {0x07, 0x36, 0x35, 0x44, 0x00};
unsigned char speedChannel = 52;
unsigned char errorPacketHeader[10] = {0xA1,	//Packet type
		0x52, 0x33, 0x22, 0x11,					//Destination address
		0x07, 0x36, 0x35, 0x44,  				//Source address
		0x00};

//Integration speed variables
int charBlastOn = 0;
int txErrorsLastSecond = 0;
int rxErrorsLastSecond = 0;
int rxErrorsLastTen = 0;
int packetsSentLastSecond = 0;
uint16_t txRate, rxRate;
uint64_t packetsReceived = 0, packetsSent = 0;
int receivedRadioPacket = 0;

//Transmit hamming packet variables
uint64_t transmitBits = HAMMING_PAYLOAD_HEADER_BITS;
int transmitBitIndex = STARTING_TRANSMIT_BIT;

/**
 * @brief Forms an error packet into the specified pointer
 * @param packet: pointer to packet to form
 * @retval None
 */
void form_error_packet(unsigned char* packet) {
	/* Add packet header */
	memcpy(packet, errorPacketHeader, PAYLOAD_STARTING_INDEX);

	/* Create payload */
	char payload[11];
	sprintf(payload, "Errors%5d", rxErrorsLastSecond);

	/* Encode and add payload */
	uint16_t encodedByte;
	for(int i = 0; i < 11; i++) {
		encodedByte = hamming_byte_encoder(payload[i]);
		packet[PAYLOAD_STARTING_INDEX + (2 * i)] = (uint8_t)((encodedByte & 0xFF00) >> 8);
		packet[PAYLOAD_STARTING_INDEX + (2 * i) + 1] = (uint8_t)(encodedByte & 0x00FF);
	}
}

/**
 * @brief Reverses the bits in a byte to change its endianness
 * @param toReverse: byte to reverse
 * @retval the reversed byte
 */
uint8_t reverse_endianness(uint8_t toReverse) {
   toReverse = (toReverse & 0xF0) >> 4 | (toReverse & 0x0F) << 4;
   toReverse = (toReverse & 0xCC) >> 2 | (toReverse & 0x33) << 2;
   toReverse = (toReverse & 0xAA) >> 1 | (toReverse & 0x55) << 1;
   return toReverse;
}

/**
 * @brief Reverses the endianness of the bytes in the hamming code
 * @param original: hamming coded word to reverse
 * @retval reversed hamming coded word
 */
uint16_t flip_hamming(uint16_t original) {
	return (uint16_t)(reverse_endianness((uint8_t)(original >> 8))) << 8 |
			(uint16_t)(reverse_endianness((uint8_t)original));
}

/**
 * @brief Hamming encodes, manchester encodes and transmits a character
 * @param toTransmit: the character to transmit
 * @retval None
 */
void send_hamming_char(char toTransmit) {
	s4435360_hal_ir_carrier_on();

	//Hamming encode character
	uint16_t hammingEncoded = flip_hamming(hamming_byte_encoder(toTransmit));

	//Manchester encode bytes
	uint16_t manchesterEncoded1 = s4435360_hal_manchester_byte_encoder((uint8_t)(hammingEncoded >> 8));
	uint16_t manchesterEncoded2 = s4435360_hal_manchester_byte_encoder((uint8_t)hammingEncoded);

	//Form transmit packet
	transmitBits = HAMMING_PAYLOAD_HEADER_BITS | ((uint64_t)manchesterEncoded1 << 18) | ((uint64_t)manchesterEncoded2 << 2);

	if(packetsSent % 3 == 0) {
		transmitBits ^= (1 << 20) ^ (1 << 21) ^ (1 << 4) ^ (1 << 9);
	}

	//Transmit packet
	configure_transmission(transmitBits, STARTING_TRANSMIT_BIT, 50000 / txRate);
	currentlyTransmitting = 1;
	HAL_TIM_Base_Start_IT(&timer1Init);
	while(currentlyTransmitting) {}
	s4435360_hal_ir_carrier_off();
	HAL_Delay(10);
}

/**
 * @brief IR character receive handler for hamming encoded chars
 * @param capture: the received hamming encoded char
 * @retval None
 */
void hamming_char_handler(uint16_t capture) {
	HammingDecodedOutput output = hamming_byte_decoder(flip_hamming(capture));

	//Two bit error
	if(output.uncorrectableError) {
		rxErrorsLastSecond += 2;
		rxErrorsLastTen += 2;

	//One bit error
	} else if(output.errorMask) {
		rxErrorsLastSecond++;
		rxErrorsLastTen++;
	}

	packetsReceived++;
	rxRate = PERIOD_REGISTER_TIME_SCALAR / receivePeriod;
}

/**
 * @brief Initialises the variable speed functionality for integration challenge
 * @param None
 * @retval None
 */
void integration_speed_init(void) {

	debug_printf("Mode 8: IR Auto Speed\r\n");
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

	txErrorsLastSecond = 0;
	rxErrorsLastSecond = 0;
	rxErrorsLastTen = 0;
	packetsSentLastSecond = 0;

}

/**
 * @brief Deinitialises the speed functionality for integration challenge
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
 * @brief The run function for the speed integration challenge
 * @param None
 * @retval None
 */
void integration_speed_run(void) {

	//Only send packets in charBlastOn mode
	if(!charBlastOn) {
		return;
	}

	lightbar_seg_set(SEND_INDICATOR_SEGMENT, 0);
	lightbar_seg_set(RECEIVE_INDICATOR_SEGMENT, 0);

	/* Check for transmission */
	if(s4435360_radio_gettxstatus()) {
		if(radio_fsm_getstate() == RADIO_FSM_TX_STATE) {
			form_error_packet(s4435360_tx_buffer);
			s4435360_radio_sendpacket(speedChannel,  speedTxAddress, s4435360_tx_buffer);

			//Reset packet
			s4435360_radio_txstatus = 0;
			memset(&s4435360_tx_buffer[0], 0, sizeof(s4435360_tx_buffer));

			rxErrorsLastSecond = 0;
			txErrorsLastSecond = 0;
			lightbar_seg_set(SEND_INDICATOR_SEGMENT, 1);
		}
	}

	/* Check for received packet */
	if(s4435360_radio_getrxstatus()) {
		s4435360_radio_getpacket(s4435360_rx_buffer);

		unsigned char decodedOutput[11];
		HammingDecodedOutput hammingOutput;
		memset(&decodedOutput[0], 0, sizeof(decodedOutput));

		int receivedInvalidMessage = 0;

		for(int i = 0; i < 11; i++) {
			uint16_t encodedDoubleByte = (s4435360_rx_buffer[10 + (2 * i)] << 8) |
					(s4435360_rx_buffer[10 + (2 * i) + 1]);
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

		if(receivedInvalidMessage) {
			return;
		}

		//Calculate tx errors
		txErrorsLastSecond = 0;
		for(int i = 0; i < 4; i++) {
			txErrorsLastSecond *= 10;

			if(s4435360_rx_buffer[TX_ERROR_STARTING_INDEX + i] != SPACE_CHAR) {
				txErrorsLastSecond += (s4435360_rx_buffer[TX_ERROR_STARTING_INDEX + i] - '0');
			}

		}

		//Reset RX variables
		receivedRadioPacket = 1;
		s4435360_radio_rxstatus = 0;

		lightbar_seg_set(RECEIVE_INDICATOR_SEGMENT, 1);
	}

	//Prints information every 10 packets sent/received
	if(packetsReceived + packetsSent % 10 == 0) {
		debug_printf("Tx Rate: %d Rx Rate: %d\r\nTx Errs: %d Rx Errs: %d\r\n",
				txRate, PERIOD_REGISTER_TIME_SCALAR / receivePeriod,
				txErrorsLastSecond, rxErrorsLastTen);
		rxErrorsLastTen = 0;

	}

	//Send character
	if(!currentlyTransmitting) {
		send_hamming_char(BLAST_LETTER);
		packetsSent++;
		packetsSentLastSecond++;
		lightbar_seg_set(SEND_INDICATOR_SEGMENT, 1);
	}
}

/**
 * @brief The user input function for the integration speed mode
 * @param userChars: the chars received from the console
 * 		   userCharsReceived: the number of chars received
 * @retval None
 */
void integration_speed_user_input(char* userChars, int userCharsReceived) {

	//Toggle charBlastOn mode
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
 * @brief Handler for timer 1, ACK transmission
 * @param None
 * @retval None
 */
void integration_speed_timer1_handler(void) {
	ir_duplex_timer1_handler();
}

/**
 * @brief Handler for timer 3, 1s timer
 * @param None
 * @retval None
 */
void integration_speed_timer3_handler(void){

	//Only update speeds and send radio packets during char blast on
	if(!charBlastOn) {
		return;
	}

	//Send radio packet update
	s4435360_radio_txstatus = 1;

	//If received no packet
	if(!receivedRadioPacket) {
		txErrorsLastSecond = packetsSentLastSecond;
		packetsSentLastSecond = 0;
	}

	if(txErrorsLastSecond > MAX_TX_ERROR) {
			txRate /= TX_RATE_MULTIPLIER;
		} else {
			txRate *= TX_RATE_MULTIPLIER;
		}

		//Confine txRate between MIN_TX_RATE and MAX_TX_RATE
		txRate = txRate > MAX_TX_RATE ? MAX_TX_RATE : txRate;
		txRate = txRate < MIN_TX_RATE ? MIN_TX_RATE : txRate;

	receivedRadioPacket = 0;
}
