/**
 ******************************************************************************
 * @file    mylib/s4435360_cli_radio.c
 * @author  Samuel Eadie - 44353607
 * @date    14052018
 * @brief   Provides CLI commands for radio
 *
 ******************************************************************************
 *     EXTERNAL FUNCTIONS
 ******************************************************************************
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <s4435360_os_pantilt.h>

#include "nrf24l01plus.h"
#include "radio_fsm.h"
#include "s4435360_hal_radio.h"
#include "s4435360_hal_hamming.h"
#include "s4435360_os_printf.h"
#include "s4435360_os_radio.h"

#include "FreeRTOS_CLI.h"

#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"
#include "board.h"
#include "stm32f4xx_hal.h"
#include <math.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define PI				3.14159265359
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/


static BaseType_t prvGetSysCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {

	//unsigned char sysTime = //SOME FUNCTION HERE;
	//pcWriteBuffer[0] = sysTime;
	return pdFALSE;
}

static BaseType_t prvSetChanCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {

	long paramLen;
	const char* parameter = FreeRTOS_CLIGetParameter(pcCommandString, 1, &paramLen);

	char* remainder;
	uint32_t chan = strtol(parameter, &remainder, 10);

	if(!strlen(remainder)) {
		set_chan(chan);
		s4435360_radio_setchan(chan);
	}

	return pdFALSE;
}

static BaseType_t prvGetChanCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {

	xWriteBufferLen = sprintf((char *) pcWriteBuffer, "%d\n\r", get_chan());
	return pdFALSE;
}

static BaseType_t prvSetTxAddrCommand (char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {

	long paramLen;
	const char* param = FreeRTOS_CLIGetParameter(pcCommandString, 1, &paramLen);

	char addr1[2];
	char addr2[2];
	char addr3[2];
	char addr4[2];

	memcpy(addr1, (void*)&param[0], 2);
	memcpy(addr2, (void*)&param[2], 2);
	memcpy(addr3, (void*)&param[4], 2);
	memcpy(addr4, (void*)&param[6], 2);

	unsigned char addr[5];
	char* remainder;
	addr[0] = (unsigned char) strtol(addr4, &remainder, 16);
	addr[1] = (unsigned char) strtol(addr3, &remainder, 16);
	addr[2] = (unsigned char) strtol(addr2, &remainder, 16);
	addr[3] = (unsigned char) strtol(addr1, &remainder, 16);
	addr[4] = 0x00;

	set_txAddress(addr);
	s4435360_radio_settxaddress(addr);

	return pdFALSE;
}

static BaseType_t prvSetRxAddrCommand (char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {

	long paramLen;
	const char* param = FreeRTOS_CLIGetParameter(pcCommandString, 1, &paramLen);

	char addr1[2];
	char addr2[2];
	char addr3[2];
	char addr4[2];

	memcpy(addr1, (void*)&param[0], 2);
	memcpy(addr2, (void*)&param[2], 2);
	memcpy(addr3, (void*)&param[4], 2);
	memcpy(addr4, (void*)&param[6], 2);

	unsigned char addr[5];
	char* remainder;
	addr[0] = (unsigned char) strtol(addr4, &remainder, 16);
	addr[1] = (unsigned char) strtol(addr3, &remainder, 16);
	addr[2] = (unsigned char) strtol(addr2, &remainder, 16);
	addr[3] = (unsigned char) strtol(addr1, &remainder, 16);
	addr[4] = 0x00;

	set_rxAddress(addr);
	s4435360_radio_setrxaddress(addr);

	return pdFALSE;
}

static BaseType_t prvGetRxAddrCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {

	unsigned char* rxAddress = get_rxAddress();
	xWriteBufferLen = sprintf((char *) pcWriteBuffer, "0x%02X 0x%02X 0x%02X 0x%02X\n\r", rxAddress[3],
			rxAddress[2],
			rxAddress[1],
			rxAddress[0]);
	return pdFALSE;
}

static BaseType_t prvGetTxAddrCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {

	unsigned char* txAddress = get_txAddress();
	xWriteBufferLen = sprintf((char *) pcWriteBuffer, "0x%02X 0x%02X 0x%02X 0x%02X\n\r", txAddress[3],
			txAddress[2],
			txAddress[1],
			txAddress[0]);
	return pdFALSE;
}

static BaseType_t prvJoinCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {

	send_join_message(portMAX_DELAY);

	return pdFALSE;
}

static BaseType_t prvXYZCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {

	/* Get parameters from command string */
	long xLen, yLen, zLen;
	const char* xString = FreeRTOS_CLIGetParameter(pcCommandString, 1, &xLen);
	const char* yString = FreeRTOS_CLIGetParameter(pcCommandString, 2, &yLen);
	const char* zString = FreeRTOS_CLIGetParameter(pcCommandString, 3, &zLen);

	char* xRemainder, *yRemainder, *zRemainder;
	long x = strtol(xString, &xRemainder, 10);
	long y = strtol(yString, &yRemainder, 10);
	long z = strtol(zString, &zRemainder, 10);

	//if((!strlen(xRemainder)) && (!strlen(yRemainder)) && (!strlen(zRemainder))) {
	send_XYZ_message(x, y, z, portMAX_DELAY);
	//}
	return pdFALSE;

}

static BaseType_t prvMoveCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {

	/* Get parameters from command string */
	long xLen, yLen;
	const char* xString = FreeRTOS_CLIGetParameter(pcCommandString, 1, &xLen);
	const char* yString = FreeRTOS_CLIGetParameter(pcCommandString, 2, &yLen);

	char* xRemainder, *yRemainder;
	long x = strtol(xString, &xRemainder, 10);
	long y = strtol(yString, &yRemainder, 10);

	//if((!strlen(xRemainder)) && (!strlen(yRemainder)) && (!strlen(zRemainder))) {
	send_XY_message(x, y, portMAX_DELAY);

	//}

	return pdFALSE;

}

static BaseType_t prvPenCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {

	/* Get parameters from command string */
	long paramLen;
	const char* param = FreeRTOS_CLIGetParameter(pcCommandString, 1, &paramLen);

	send_join_message(portMAX_DELAY);

	if(!strncmp((const char*) param, "up", 2)) {
		send_Z_message(DEFAULT_UP_Z_VALUE, portMAX_DELAY);
	} else if(!strncmp((const char*) param, "down", 4)) {
		send_Z_message(DEFAULT_DOWN_Z_VALUE, portMAX_DELAY);
	}

	return pdFALSE;

}


static BaseType_t prvRadioCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {
	RadioMessage message;
	message.retransmitAttempts = 0;

	/* Get parameters from command string */
	char* param = FreeRTOS_CLIGetParameter(pcCommandString, 1, (BaseType_t)(&(message.payloadLength)));

	memcpy((void*) message.payload, (void*) param, message.payloadLength);

	xQueueSendToBack(txMessageQueue, ( void * ) &message, ( TickType_t ) 100 );
	return pdFALSE;
}

CLI_Command_Definition_t radio = {
		"radio",
		"radio: Transmits the payload over radio.\r\n",
		prvRadioCommand,
		1
};

CLI_Command_Definition_t getsys = {
		"getsys",
		"getsys: Returns the current system time in ms.\r\n",
		prvGetSysCommand,
		0
};

CLI_Command_Definition_t setchan = {
		"setchan",
		"setchan: Sets the radio channel.\r\n",
		prvSetChanCommand,
		1
};

CLI_Command_Definition_t getchan = {
		"getchan",
		"getchan: Returns the radio's current channel.\r\n",
		prvGetChanCommand,
		0
};

CLI_Command_Definition_t settxaddr = {
		"settxaddr",
		"settxaddr: Sets the radio's transmit address.\r\n",
		prvSetTxAddrCommand,
		1
};

CLI_Command_Definition_t setrxaddr = {
		"setrxaddr",
		"setrxaddr: Sets the radio's receive address.\r\n",
		prvSetRxAddrCommand,
		1
};

CLI_Command_Definition_t getrxaddr = {
		"getrxaddr",
		"getrxaddr: Returns the radio's receive address.\r\n",
		prvGetRxAddrCommand,
		0
};

CLI_Command_Definition_t gettxaddr = {
		"gettxaddr",
		"gettxaddr: Returns the radio's transmit address.\r\n",
		prvGetTxAddrCommand,
		0
};

CLI_Command_Definition_t join = {
		"join",
		"join: Sends a join request packet via radio.\r\n",
		prvJoinCommand,
		0
};

CLI_Command_Definition_t xyz = {
		"xyz",
		"xyz: Sends an XYZ packet via radio.\r\n",
		prvXYZCommand,
		3
};

CLI_Command_Definition_t move = {
		"move",
		"move: Moves the pen to the given position via radio packet.\r\n",
		prvMoveCommand,
		2
};

CLI_Command_Definition_t pen = {
		"pen",
		"pen: Moves the pen up or down via radio packet.\r\n",
		prvPenCommand,
		1
};


void register_radio_CLI_commands(void) {

	FreeRTOS_CLIRegisterCommand(&radio);
	FreeRTOS_CLIRegisterCommand(&getsys);
	FreeRTOS_CLIRegisterCommand(&setchan);
	FreeRTOS_CLIRegisterCommand(&getchan);
	FreeRTOS_CLIRegisterCommand(&settxaddr);
	FreeRTOS_CLIRegisterCommand(&setrxaddr);
	FreeRTOS_CLIRegisterCommand(&getrxaddr);
	FreeRTOS_CLIRegisterCommand(&gettxaddr);
	FreeRTOS_CLIRegisterCommand(&join);
	FreeRTOS_CLIRegisterCommand(&xyz);
	FreeRTOS_CLIRegisterCommand(&move);
	FreeRTOS_CLIRegisterCommand(&pen);
}
