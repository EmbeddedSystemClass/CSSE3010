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
#include "board.h"
#include "radio_fsm.h"
#include "nrf24l01plus.h"
#include "assert.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define IN_IDLE_STATE			s4435360_radio_fsmcurrentstate == S4435360_IDLE_STATE
#define CHECK_IN_IDLE_STATE()	assert(IN_IDLE_STATE)
/* Private variables ---------------------------------------------------------*/
int s4435360_radio_fsmcurrentstate = S4435360_INIT_STATE;
int s4435360_radio_rxstatus;
unsigned char s4435360_rx_buffer[32];
unsigned char s4435360_tx_buffer[16];
/* Private function prototypes -----------------------------------------------*/
void s4435360_radio_setchan(unsigned char channel);
/* Private functions ---------------------------------------------------------*/

/**
 * @brief Initialises radio (GPIO, SPI etc)
 * @param: None
 * @retval None
 */
void s4435360_radio_init(void) {
	radio_fsm_init();
	//s4435360_radio_setchan((unsigned char) RF_CHANNEL);
	s4435360_radio_fsmcurrentstate = S4435360_IDLE_STATE;
	unsigned char enableRXRegister = 0b00111111;
	radio_fsm_register_write(NRF24L01P_EN_RXADDR, &enableRXRegister);


}

/**
 * @brief Radio FSM processing loop, called on timer interrupt
 * @param None
 * @retval None
 */
void s4435360_radio_fsmprocessing() {

}

/**
 * @brief Set the channel of the radio
 * @param channel: the new radio channel
 * @retval None
 */
void s4435360_radio_setchan(unsigned char channel) {
	//CHECK_IN_IDLE_STATE();
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
	//CHECK_IN_IDLE_STATE();

	radio_fsm_buffer_write(NRF24L01P_TX_ADDR, addr, 5);
	radio_fsm_buffer_write(NRF24L01P_RX_ADDR_P0, addr, 5);
	radio_fsm_buffer_write(NRF24L01P_RX_ADDR_P5, addr, 5);
	//radio_fsm_register_write(NRF24L01P_TX_ADDR, addr);
}

/**
 * @brief Gets the channel of the radio
 * @param None
 * @retval the radio channel
 */
unsigned char s4435360_radio_getchan(void) {
	//CHECK_IN_IDLE_STATE();

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
	//CHECK_IN_IDLE_STATE();

	radio_fsm_buffer_read(NRF24L01P_TX_ADDR, addr, 5);
	//radio_fsm_register_read(NRF24L01P_TX_ADDR, addr);
}

/**
 * @brief Function to send a packet
 * @param 	channel: channel to send on
 * 			addr: the address to send to
 * 			txpacket: the packet to transmit
 * @retval None
 */
void s4435360_radio_sendpacket(char channel, unsigned char* addr, unsigned char* txpacket) {
	//radio_fsm_setstate(S4435360_TX_STATE);
	//s4435360_radio_setchan(channel);
	//s4435360_radio_settxaddress(addr);
	memcpy(s4435360_tx_buffer, txpacket, 16);
	radio_fsm_write(s4435360_tx_buffer);

}

/**
 * @brief Set the radio FSM to RX mode
 * @param None
 * @retval None
 */
void s4435360_radio_setfsmrx() {
	s4435360_radio_fsmcurrentstate = S4435360_RX_STATE;
	radio_fsm_setstate(S4435360_RX_STATE);
}

/**
 * @brief Checks for received packets, returns value of s4435360_radio_rxstatus
 * @param None
 * @retval The rx status
 */
int s4435360_radio_getrxstatus() {
	unsigned char state;
	radio_fsm_getstate(&state);
	return state;
}

/**
 * @brief Function to receive a packet, when s4435360_radio_rxstatus == 1
 * @param Pointer to store the packet
 * @retval None
 */
void s4435360_radio_getpacket(unsigned char* rxpacket) {
	radio_fsm_read(rxpacket);
	HAL_Delay(10);
}
