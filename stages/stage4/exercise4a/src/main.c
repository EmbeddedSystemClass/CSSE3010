/**
  ******************************************************************************
  * @file    ex13_radio_tx/main.c
  * @author  MDS
  * @date    21022016
  * @brief   Send 32 byte packets using the nrf9051plus radio transciever
  *			 See the nrf9051plus datasheet.
  ******************************************************************************
  *
  */

/* Includes ------------------------------------------------------------------*/
#include "board.h"
#include "stm32f4xx_hal_conf.h"
#include "debug_printf.h"
#include "radio_fsm.h"
#include "nrf24l01plus.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define IDLE_STATE		0
#define TX_STATE		1
#define WAITING_STATE	2
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t packetbuffer[32];	/* Packet buffer initialised to 32 bytes (max length) */
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
	uint8_t current_channel;

	BRD_init();
	HardwareInit();

	/* Initialise radio FSM */
	radio_fsm_init();

	/* set radio FSM state to IDLE */
	radio_fsm_setstate(RADIO_FSM_IDLE_STATE);

	uint8_t channel = 43;
	radio_fsm_register_write(NRF24L01P_RF_CH, &channel);

    while (1) {

		//Transmit FSM
		switch(current_state) {

			case IDLE_STATE:	//Idle state for reading current channel

				/* Get current channel , if radio FSM is in IDLE State */
				if (radio_fsm_getstate() == RADIO_FSM_IDLE_STATE) {

					radio_fsm_register_read(NRF24L01P_RF_CH, &current_channel);	//Read channel
					debug_printf("Current Channel %d\n\r", current_channel);

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

						/* Fill packet with 'dummy' alphabet data */
						for (i = 0; i < 32; i++) {
							packetbuffer[i] = 'a'+ (i%26);
						}

						debug_printf("sending...\n\r");

						/* Send packet - radio FSM will automatically go to IDLE state, after write completes. */
						radio_fsm_write(packetbuffer);

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
