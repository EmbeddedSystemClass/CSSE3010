/***
 ******************************************************************************
 * @file    mylib/s4435360_hal_radio.h
 * @author  Samuel Eadie - 44353607
 * @date    14032018-21032018
 * @brief   Radio functionality
 ******************************************************************************
 */

#ifndef S4435360_HAL_RADIO_H
#define S4435360_HAL_RADIO_H

/* FSM states */
#define S4435360_IDLE_STATE			0
#define S4435360_RX_STATE			1
#define S4435360_TX_STATE			2
#define S4435360_WAITING_STATE		3
#define S4435360_INIT_STATE 		4

#define RF_CHANNEL					52

/* Radio variables */
int s4435360_radio_fsmcurrentstate;
int s4435360_radio_rxstatus, s4435360_radio_txstatus;
unsigned char s4435360_rx_buffer[32];
unsigned char s4435360_tx_buffer[32];

/* Function prototypes */
void s4435360_radio_init(void);
void s4435360_radio_fsmprocessing();
void s4435360_radio_setchan(unsigned char channel);
void s4435360_radio_settxaddress(unsigned char* addr);
void s4435360_radio_setrxaddress(unsigned char* addr);
void s4435360_radio_getrxaddress(unsigned char* addr) ;
unsigned char s4435360_radio_getchan(void);
void s4435360_radio_gettxaddress(unsigned char* addr);
void s4435360_radio_sendpacket(unsigned char channel, unsigned char* addr, unsigned char* txpacket);
void s4435360_radio_setfsmrx();
int s4435360_radio_getrxstatus();
int s4435360_radio_gettxstatus();
void s4435360_radio_getpacket(unsigned char* rxpacket);


#endif



