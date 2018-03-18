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
#define IDLE_STATE		0
#define TX_STATE		1
#define WAITING_STATE	2
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t packetbuffer[32];	/* Packet buffer initialised to 32 bytes (max length) */

uint8_t packet[16] = {0x20,						//Packet type
		0x52, 0x33, 0x22, 0x11,					//Destination address
		0x07, 0x36, 0x35, 0x44,					//Source address
		's', 'a', 'm', 'u', 'e', 'l', '-'};		//Payload
/* Private function prototypes -----------------------------------------------*/
void HardwareInit();

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void) {

	int i;
	int current_state = IDLE_STATE;		//Current state of FSM

	BRD_init();
	HardwareInit();

	/* Initialise radio FSM */
	s4435360_radio_init();

	/* set radio FSM state to IDLE */
	radio_fsm_setstate(RADIO_FSM_IDLE_STATE);

	unsigned char channel = 52;

	unsigned char txAddress[5] = {0x52, 0x33, 0x22, 0x11, 0x00};
	unsigned char readAddress[5] = {0x01, 0x02, 0x03, 0x04, 0x05};


	s4435360_radio_setchan(channel);
	s4435360_radio_settxaddress(txAddress);

    while (1) {

		//Transmit FSM
		switch(current_state) {

			case IDLE_STATE:	//Idle state for reading current channel
				/* Get current channel , if radio FSM is in IDLE State */
				if (radio_fsm_getstate() == RADIO_FSM_IDLE_STATE) {

					s4435360_radio_gettxaddress(readAddress);
					debug_printf("Current Channel %d\n\rTransmission Address ", s4435360_radio_getchan());
					for(int i = 0; i < 5; i++) {
						debug_printf("0x%X ", readAddress[i]);
					}
					debug_printf("\n\r");

					current_state = TX_STATE;	//Set next state as TX state.

				} else {

						/* if error occurs, set state back to IDLE state */
						debug_printf("ERROR: Radio FSM not in Idle state\n\r");
						radio_fsm_setstate(RADIO_FSM_IDLE_STATE);
				}

				break;

			case TX_STATE:	//TX state for writing packet to be sent.

				/* Put radio FSM in TX state, if radio FSM is in IDLE state */
				if (radio_fsm_getstate() == RADIO_FSM_IDLE_STATE) {

					if (radio_fsm_setstate(RADIO_FSM_TX_STATE) == RADIO_FSM_INVALIDSTATE) {
						debug_printf("ERROR: Cannot set Radio FSM RX state\n\r");
						HAL_Delay(100);
					} else {
						debug_printf("sending...\n\r");

						/* Send packet - radio FSM will automatically go to IDLE state, after write completes. */
						//radio_fsm_write(packet);
						s4435360_radio_sendpacket(channel, readAddress, packet);

						debug_printf("Sent packet: ");
						for (i = 0; i < 16; i++) {
							debug_printf("%x ", packet[i]);
						}
						debug_printf("\r\n");

						current_state = IDLE_STATE;		//set next state as Waiting state
					}
				} else {

						/* if error occurs, set state back to IDLE state */
						debug_printf("ERROR: Radio FSM not in Idle state\n\r");
						radio_fsm_setstate(RADIO_FSM_IDLE_STATE);
				}

				break;

		}

    	HAL_Delay(1000); //Delay for 100ms.
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
}
