/**
  ******************************************************************************
  * @file    demo4/main.c
  * @author  SE
  * @date    14032018-21032018
  * @brief   Send/receive 16-byte packets to/from communications base
  ******************************************************************************
  *
  */

/* Includes ------------------------------------------------------------------*/
#include "board.h"
#include "stm32f4xx_hal_conf.h"
#include "debug_printf.h"
#include "radio_fsm.h"
#include "nrf24l01plus.h"
#include "s4435360_hal_radio.h"
#include "string.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define TIMER_FREQUENCY 			50000
#define INTERRUPT_FREQUENCY 		10
#define ENTER_CHAR					(char)(13)
#define BACKSPACE_CHAR				(char)(8)
#define SPACE_CHAR					(char)(32)
#define MAXUSERCHARS				7
#define PAYLOAD_STARTING_INDEX		9
#define PACKET_READY_TO_SEND		1
#define PACKET_NOT_READY_TO_SEND	0
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
unsigned char txAddress[5] = {0x52, 0x33, 0x22, 0x11, 0x00};
unsigned char rxAddress[5] = {0x07, 0x36, 0x35, 0x44, 0x00};
unsigned char channel = 52;
int userCharCount = 0;
char inputChar;
uint8_t userPacket[16] = {0xA1,					//Packet type
		0x52, 0x33, 0x22, 0x11,					//Destination address
		0x07, 0x36, 0x35, 0x44}; 				//Source address

TIM_HandleTypeDef fsmTimInit;
/* Private function prototypes -----------------------------------------------*/
void HardwareInit();

/**
  * @brief  Processes a character input from terminal
  * @param  Input character
  * @retval 1 if a packet is ready to send, 0 otherwise
  */
int handle_user_input(char input) {

	/* Check for user-forced packet end */
	if(input == ENTER_CHAR) {
		return PACKET_READY_TO_SEND;
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
		return PACKET_READY_TO_SEND;
	} else {
		return PACKET_NOT_READY_TO_SEND;
	}

}

/**
  * @brief  Prints the specified packet as received
  * 		to terminal
  * @param  The packet to print
  * @retval None
  */
void print_received_packet(unsigned char* packet) {

	debug_printf("RECV: ");

	//Print source address
	for (int i = 8; i > 4; i--) {
		debug_printf("%x", packet[i]);
	}

	debug_printf(" > ");

	//Print packet content
	for (int i = 9; i < 15; i++) {
		debug_printf("%c-", packet[i]);
	}

	debug_printf("%c\r\n", packet[15]);
	debug_flush();
}

/**
  * @brief  Prints the specified packet as sent to
  * 		terminal
  * @param  The packet to print
  * @retval None
  */
void print_sent_packet(unsigned char* packet) {

	debug_printf("SENT: ");

	//Print destination address
	for (int i = 4; i > 0; i--) {
		debug_printf("%x", packet[i]);
	}

	debug_printf(" > ");

	//Print packet content
	for (int i = 9; i < 15; i++) {
		debug_printf("%c-", packet[i]);
	}

	debug_printf("%c\r\n", packet[15]);
	debug_flush();
}

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void) {

	/* Initialise hardware */
	BRD_init();
	HardwareInit();
	s4435360_radio_init();
	setbuf(stdout, NULL);

	/* Set rx address */
	radio_fsm_setstate(RADIO_FSM_IDLE_STATE);
	s4435360_radio_setrxaddress(rxAddress);

    while (1) {
    	/* Check for user input*/
    	inputChar = debug_getc();
    	if(inputChar) {
    		s4435360_radio_txstatus = handle_user_input(inputChar);
    	}

    	/* Check for transmission */
    	if(s4435360_radio_gettxstatus()) {

    		if(radio_fsm_getstate() == RADIO_FSM_TX_STATE) {
				/*Isolate user entered chars and header */
				memcpy(s4435360_tx_buffer, userPacket, PAYLOAD_STARTING_INDEX + userCharCount);

				s4435360_radio_sendpacket(channel,  txAddress, s4435360_tx_buffer);
				print_sent_packet(s4435360_tx_buffer);

				/* Reset packet */
				s4435360_radio_txstatus = 0;
				userCharCount = 0;
				memset(&s4435360_tx_buffer[0], 0, sizeof(s4435360_tx_buffer));
    		}
    	}


    	/* Check for received packet */
    	s4435360_radio_setfsmrx();

    	if(s4435360_radio_getrxstatus()) {
    		s4435360_radio_getpacket(s4435360_rx_buffer);
    		print_received_packet(s4435360_rx_buffer);
    		s4435360_radio_rxstatus = 0;
    	}


    	HAL_Delay(10);
		BRD_LEDGreenToggle();
  	}

}

/**
  * @brief Hardware Initialisation Function.
  * @param  None
  * @retval None
  */
void HardwareInit() {

	BRD_LEDInit();		//Initialise Blue LED
	/* Turn off LEDs */
	BRD_LEDRedOff();
	BRD_LEDGreenOff();
	BRD_LEDBlueOff();

	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

	// Timer 3 clock enable
	__TIM3_CLK_ENABLE();

	/* TIM Base configuration */
	fsmTimInit.Instance = TIM3;
	fsmTimInit.Init.Period = TIMER_FREQUENCY / INTERRUPT_FREQUENCY;
	fsmTimInit.Init.Prescaler = (uint16_t) ((SystemCoreClock) / TIMER_FREQUENCY) - 1;
	fsmTimInit.Init.ClockDivision = 0;
	fsmTimInit.Init.RepetitionCounter = 0;
	fsmTimInit.Init.CounterMode = TIM_COUNTERMODE_UP;

	/* Initialise Timer */
	HAL_TIM_Base_Init(&fsmTimInit);
	HAL_NVIC_SetPriority(TIM3_IRQn, 10, 0);
	HAL_NVIC_EnableIRQ(TIM3_IRQn);
	HAL_TIM_Base_Start_IT(&fsmTimInit);
}

/**
  * @brief Executed when TIM3 period expires
  * @param The timer handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	s4435360_radio_fsmprocessing();
}

void TIM3_IRQHandler(void) {
	HAL_TIM_IRQHandler(&fsmTimInit);
}


