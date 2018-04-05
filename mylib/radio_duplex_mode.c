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

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define TIMER_FREQUENCY 			50000
#define INTERRUPT_FREQUENCY 		10
#define ENTER_CHAR					(char)(13)
#define BACKSPACE_CHAR				(char)(8)
#define SPACE_CHAR					(char)(32)
#define MAXUSERCHARS				11
#define PAYLOAD_STARTING_INDEX		9
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
char inputChar;
uint8_t userPacket[32] = {0x20,					//Packet type
		0x52, 0x33, 0x22, 0x11,					//Destination address
		0x07, 0x36, 0x35, 0x44}; 				//Source address

/* Private function prototypes -----------------------------------------------*/

void radio_duplex_init(void) {
	currentRadioDuplexMode = START_MODE;
	BRD_init();
	BRD_LEDInit();		//Initialise Blue LED
	/* Turn off LEDs */
	BRD_LEDRedOff();
	BRD_LEDGreenOff();
	BRD_LEDBlueOff();

	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

	// Timer 3 clock enable
	__TIM3_CLK_ENABLE();

	/* TIM Base configuration */
	timer1Init.Instance = TIM6;
	timer1Init.Init.Period = TIMER_FREQUENCY / INTERRUPT_FREQUENCY;
	timer1Init.Init.Prescaler = (uint16_t) ((SystemCoreClock) / TIMER_FREQUENCY) - 1;
	timer1Init.Init.ClockDivision = 0;
	timer1Init.Init.RepetitionCounter = 0;
	timer1Init.Init.CounterMode = TIM_COUNTERMODE_UP;

	/* Initialise Timer */
	HAL_TIM_Base_Init(&timer1Init);
	HAL_NVIC_SetPriority(TIMER1_IRQ, 10, 0);
	HAL_NVIC_EnableIRQ(TIMER1_IRQ);
	HAL_TIM_Base_Start_IT(&timer1Init);


	s4435360_radio_init();
	setbuf(stdout, NULL);

	/* Set rx address */
	radio_fsm_setstate(RADIO_FSM_IDLE_STATE);
	s4435360_radio_setrxaddress(rxAddress);
}

void radio_duplex_deinit(void) {
	HAL_TIM_Base_Stop_IT(&timer1Init);
}

void radio_duplex_run(void) {
	/* Check for transmission */
	if(s4435360_radio_gettxstatus()) {

		if(radio_fsm_getstate() == RADIO_FSM_TX_STATE) {
			/*Isolate user entered chars and header */
			memcpy(s4435360_tx_buffer, userPacket, PAYLOAD_STARTING_INDEX + userCharCount);

			s4435360_radio_sendpacket(channel,  txAddress, s4435360_tx_buffer);
			//print_sent_packet(s4435360_tx_buffer);

			/* Reset packet */
			s4435360_radio_txstatus = 0;
			userCharCount = 0;
			memset(&s4435360_tx_buffer[0], 0, sizeof(s4435360_tx_buffer));

			///////////////////////////////////////////////////////////////////////////////////////////////////
			///////////////////////////////////////////////////////////////////////////////////////////////////
			///////////////////////////////////////ADD TX FOR PROJECT 1////////////////////////////////////////
			///////////////////////////////////////////////////////////////////////////////////////////////////
			///////////////////////////////////////////////////////////////////////////////////////////////////
		}
	}


	/* Check for received packet */
	s4435360_radio_setfsmrx();

	if(s4435360_radio_getrxstatus()) {
		s4435360_radio_getpacket(s4435360_rx_buffer);
		//print_received_packet(s4435360_rx_buffer);
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
						((input >= 'A') && (input <= 'F')) ||
						(input == ENTER_CHAR) ||
						(input == SPACE_CHAR) ||
						(input == BACKSPACE_CHAR)) {

				/* Check for user-forced packet end */
				if(input == ENTER_CHAR) {
					s4435360_radio_txstatus = PACKET_READY_TO_SEND;
					currentRadioDuplexMode = START_MODE;
					return;
				}

				/* Check for backspace */
				if(input == BACKSPACE_CHAR) {
					if(userCharCount) {
						userCharCount--;
					}

					/* Handle general case */
				} else {
					userPacket[PAYLOAD_STARTING_INDEX + userCharCount] = inputChar;
					userCharCount++;
				}

				/* Check for packet completion */
				if(userCharCount >= MAXUSERCHARS) {
					s4435360_radio_txstatus = PACKET_READY_TO_SEND;
					currentRadioDuplexMode = START_MODE;
					return;
				} else {
					s4435360_radio_txstatus = PACKET_NOT_READY_TO_SEND;
					return;
				}
			}
		break;
	}

}

void radio_duplex_timer1_handler(void) {
	s4435360_radio_fsmprocessing();
}

void radio_duplex_timer2_handler(void) {}
