/**
 ******************************************************************************
 * @file    mylib/s4435360_hal_radio.c
 * @author  Samuel Eadie - 44353607
 * @date    14032018-21032018
 * @brief   Radio communications
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <s4435360_hal_radio.h>

#include "debug_printf.h"
#include "stm32f4xx_hal_conf.h"
#include "stm32f4xx_hal.h"
#include "board.h"
#include "radio_fsm.h"
#include "nrf24l01plus.h"
#include "assert.h"
#include "string.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
 * @brief Initialises radio (GPIO, SPI etc)
 * @param: None
 * @retval None
 */
void s4435360_radio_init(void) {

	radio_fsm_init();

	/* Initialise flags to 0 */
	s4435360_radio_rxstatus = 0;
	s4435360_radio_txstatus = 0;

	s4435360_radio_fsmcurrentstate = S4435360_IDLE_STATE;

}

/**
 * @brief Radio FSM processing loop, called on timer interrupt
 * @param None
 * @retval None
 */
void s4435360_radio_fsmprocessing() {

	/* Process current state */
	switch (s4435360_radio_fsmcurrentstate) {

		case S4435360_IDLE_STATE:

			//If IDLE, get appropriate next state
			if (radio_fsm_getstate() == RADIO_FSM_IDLE_STATE) {
				if(s4435360_radio_gettxstatus()) {
					s4435360_radio_fsmcurrentstate = S4435360_TX_STATE;
				} else {
					s4435360_radio_fsmcurrentstate = S4435360_RX_STATE; // Set next state as RX state
				}

			//Error
			} else {

				debug_printf("ERROR: Radio FSM not in IDLE state\r\n");
				radio_fsm_setstate(RADIO_FSM_IDLE_STATE);

			}

			break;


		case S4435360_TX_STATE:

			//Set radio to TX state if required
			if (radio_fsm_getstate() == RADIO_FSM_IDLE_STATE) {
				if (radio_fsm_setstate(RADIO_FSM_TX_STATE) == RADIO_FSM_INVALIDSTATE) {
					debug_printf("ERROR: Cannot set radio FSM to TX state\r\n");
					HAL_Delay(100);
				}

			}

			//Exit TX state if packet has been sent
			if(!s4435360_radio_gettxstatus()) {
				s4435360_radio_fsmcurrentstate = S4435360_IDLE_STATE;
				radio_fsm_setstate(RADIO_FSM_IDLE_STATE);
			}

			break;

		case S4435360_RX_STATE:

			//Set state to RX if required
			if ((radio_fsm_getstate() == RADIO_FSM_IDLE_STATE)
					|| (radio_fsm_getstate() == RADIO_FSM_WAIT_STATE)) {

				if (radio_fsm_setstate(RADIO_FSM_RX_STATE)
						== RADIO_FSM_INVALIDSTATE) {
					debug_printf("ERROR: Cannot set radio FSM to RX state\r\n");
					HAL_Delay(100);

				//Wait for packet receive
				} else {
					s4435360_radio_fsmcurrentstate = S4435360_WAITING_STATE;
				}

			//Error
			} else {

				debug_printf("ERROR: Radio FSM not in IDLE state\r\n");
				radio_fsm_setstate(RADIO_FSM_IDLE_STATE);
			}

			break;

		case S4435360_WAITING_STATE:

			//If radio in WAIT also
			if (radio_fsm_getstate() == RADIO_FSM_WAIT_STATE) {

				//Check for packet received
				if (radio_fsm_read(s4435360_rx_buffer) == RADIO_FSM_DONE) {
					s4435360_radio_rxstatus = 1;

				}

				//Return to IDLE
				radio_fsm_setstate(RADIO_FSM_IDLE_STATE);

				s4435360_radio_fsmcurrentstate = S4435360_IDLE_STATE;
			}

			break;
		}

}

/**
 * @brief Set the channel of the radio
 * @param channel: the new radio channel
 * @retval None
 */
void s4435360_radio_setchan(unsigned char channel) {
	radio_fsm_register_write(NRF24L01P_RF_CH, &channel);
	unsigned char rfsetup = 0x06;
	radio_fsm_register_write(NRF24L01P_RF_SETUP, &rfsetup);
}

/**
 * @brief Set the transmit address of the radio
 * @param addr: the transmission address
 * @retval None
 */
void s4435360_radio_settxaddress(unsigned char* addr) {
	radio_fsm_buffer_write(NRF24L01P_TX_ADDR, addr, 5);
}

/**
 * @brief Set the receive address of the radio
 * @param addr: the transmission address
 * @retval None
 */
void s4435360_radio_setrxaddress(unsigned char* addr) {
	radio_fsm_buffer_write(NRF24L01P_RX_ADDR_P0, addr, 5);
}

/**
 * @brief Gets the channel of the radio
 * @param None
 * @retval the radio channel
 */
unsigned char s4435360_radio_getchan(void) {
	unsigned char channel;
	radio_fsm_register_read(NRF24L01P_RF_CH, &channel);

	return channel;
}

/**
 * @brief Get the transmit address of the radio
 * @param Pointer to store the transmit address
 * @retval None
 */
void s4435360_radio_gettxaddress(unsigned char* addr) {
	radio_fsm_buffer_read(NRF24L01P_TX_ADDR, addr, 5);
}

/**
 * @brief Get the transmit address of the radio
 * @param Pointer to store the transmit address
 * @retval None
 */
void s4435360_radio_getrxaddress(unsigned char* addr) {
	radio_fsm_buffer_read(NRF24L01P_RX_ADDR_P0, addr, 5);
}

/**
 * @brief Function to send a packet
 * @param 	channel: channel to send on
 * 			addr: the address to send to
 * 			txpacket: the packet to transmit
 * @retval None
 */
void s4435360_radio_sendpacket(char channel, unsigned char* addr, unsigned char* txpacket) {
	radio_fsm_setstate(RADIO_FSM_IDLE_STATE);
	s4435360_radio_setchan(channel);
	s4435360_radio_settxaddress(addr);

	memcpy(s4435360_tx_buffer, txpacket, 16);
	radio_fsm_setstate(RADIO_FSM_TX_STATE);
	radio_fsm_write(s4435360_tx_buffer);

}

/**
 * @brief Set the radio FSM to RX mode
 * @param None
 * @retval None
 */
void s4435360_radio_setfsmrx() {
	//s4435360_radio_fsmcurrentstate = S4435360_RX_STATE;
}

/**
 * @brief Checks for received packets, returns value of s4435360_radio_rxstatus
 * @param None
 * @retval The rx status
 */
int s4435360_radio_getrxstatus(void) {
	return s4435360_radio_rxstatus;
}

/**
 * @brief Checks for packet ready to send, returns value of s4435360_radio_txstatus
 * @param None
 * @retval The tx status
 */
int s4435360_radio_gettxstatus() {
	return s4435360_radio_txstatus;
}

/**
 * @brief Function to receive a packet, when s4435360_radio_rxstatus == 1
 * @param Pointer to store the packet
 * @retval None
 */
void s4435360_radio_getpacket(unsigned char* rxpacket) {
	rxpacket = s4435360_rx_buffer;
}
