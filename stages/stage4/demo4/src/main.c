/**
  ******************************************************************************
  * @file    demo4/main.c
  * @author  SE
  * @date    14032018-21032018
  * @brief   Send 16-byte packets to communications base
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

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define TIMER_FREQUENCY 			50000
#define INTERRUPT_FREQUENCY 		2
#define ENTER_CHAR					(char)(13)
#define BACKSPACE_CHAR				(char)(8)
#define MAXUSERCHARS				23
#define PAYLOAD_STARTING_INDEX		9
#define PACKET_READY_TO_SEND		1
#define PACKET_NOT_READY_TO_SEND	0
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t packet[16] = {0x20,						//Packet type
		0x52, 0x33, 0x22, 0x11,					//Destination address
		0x07, 0x36, 0x35, 0x44,					//Source address
		's', 'a', 'm', 'u', 'e', 'l', '-'};		//Payload

unsigned char txAddress[5] = {0x52, 0x33, 0x22, 0x11, 0x00};
unsigned char rxAddress[5] = {0x07, 0x36, 0x35, 0x44, 0x00};
unsigned char channel = 52;
int userCharCount = 0;
char inputChar;
uint8_t userPacket[32] = {0x20,						//Packet type
		0x52, 0x33, 0x22, 0x11,					//Destination address
		0x07, 0x36, 0x35, 0x44};				//Source address
TIM_HandleTypeDef TimInit;
/* Private function prototypes -----------------------------------------------*/
void HardwareInit();

int handle_user_input(char input) {

	if((userCharCount >= MAXUSERCHARS) || (input == ENTER_CHAR)) {
		return PACKET_READY_TO_SEND;
	}

	if(input == BACKSPACE_CHAR) {
		if(userCharCount) {
			userCharCount--;
		}
	} else {
		userPacket[PAYLOAD_STARTING_INDEX + userCharCount] = inputChar;
		userCharCount++;
	}

	/*debug_printf("Received: ");
	for (int i = 0; i < PAYLOAD_STARTING_INDEX; i++) {
		debug_printf("%x ", userPacket[i]);
	}
	for (int i = PAYLOAD_STARTING_INDEX; i < PAYLOAD_STARTING_INDEX + userCharCount + 1; i++) {
		debug_printf("%c ", userPacket[i]);
	}
	debug_printf("\r\n");*/

	return PACKET_NOT_READY_TO_SEND;
}

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void) {

	BRD_init();
	HardwareInit();
	s4435360_radio_init();
	setbuf(stdout, NULL);

	radio_fsm_setstate(RADIO_FSM_IDLE_STATE);
	s4435360_radio_setchan(channel);
	s4435360_radio_settxaddress(txAddress);
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
				unsigned char packetToSend[PAYLOAD_STARTING_INDEX + userCharCount];
				memcpy(packetToSend, userPacket, PAYLOAD_STARTING_INDEX + userCharCount);

				s4435360_radio_sendpacket(channel,  txAddress, packetToSend);
				s4435360_radio_txstatus = 0;
				userCharCount = 0;
    		}
    	}


    	/* Check for received packet */
    	if(s4435360_radio_getrxstatus()) {
    		debug_printf("Received: ");
    		for (int i = 0; i < 32; i++) {
    			debug_printf("%x ", s4435360_rx_buffer[i]);
    		}
    		debug_printf("\r\n");
    		s4435360_radio_rxstatus = 0;
    	}


    	HAL_Delay(100); //Delay for 100ms.
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
	TimInit.Instance = TIM3;
	TimInit.Init.Period = TIMER_FREQUENCY/INTERRUPT_FREQUENCY;
	TimInit.Init.Prescaler = (uint16_t) ((SystemCoreClock) / TIMER_FREQUENCY) - 1;
	TimInit.Init.ClockDivision = 0;
	TimInit.Init.RepetitionCounter = 0;
	TimInit.Init.CounterMode = TIM_COUNTERMODE_UP;

	/* Initialise Timer */
	HAL_TIM_Base_Init(&TimInit);
	HAL_NVIC_SetPriority(TIM3_IRQn, 10, 0);
	HAL_NVIC_EnableIRQ(TIM3_IRQn);
	HAL_TIM_Base_Start_IT(&TimInit);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	s4435360_radio_fsmprocessing();
}

void TIM3_IRQHandler(void) {
	//debug_printf("Inside interrupt handler\r\n");
	HAL_TIM_IRQHandler(&TimInit);
}
