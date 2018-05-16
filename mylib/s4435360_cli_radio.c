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

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
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
	const char* parameter = FreeRTOS_CLIGetParameter(pcCommandString, 1, &paramLen);

	unsigned char addr[5];
	char* remainder;
	uint64_t value = strtol(parameter, &remainder, 16);

	value = 0x80000055;

	if(!strlen(remainder)) {
		for(int i = 0; i < 4; i++) {
			addr[i] = (uint8_t)value;
			value >>= 8;
			myprintf("--> %X", addr[i]);
		}
	}

	addr[4] = 0x00;

	set_txAddress(addr);
	s4435360_radio_settxaddress(get_txAddress());
	return pdFALSE;
}

static BaseType_t prvSetRxAddrCommand (char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {

	long paramLen;
	const char* parameter = FreeRTOS_CLIGetParameter(pcCommandString, 1, &paramLen);

	unsigned char addr[5];
	char* remainder;
	uint32_t value = strtol(parameter, &remainder, 16);

	if(!strlen(remainder)) {
		for(int i = 0; i < 4; i++) {
			addr[i] = (uint8_t)value;
			value >>= 8;
		}
	}

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

	send_join_message(100);

	return pdFALSE;
}

static BaseType_t prvXYZCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {

	/* Get parameters from command string */
	long xLen, yLen, zLen;
	const char* xString = FreeRTOS_CLIGetParameter(pcCommandString, 1, &xLen);
	const char* yString = FreeRTOS_CLIGetParameter(pcCommandString, 2, &yLen);
	const char* zString = FreeRTOS_CLIGetParameter(pcCommandString, 3, &zLen);

	char* xRemainder, yRemainder, zRemainder;
	long x = strtol(xString, &xRemainder, 10);
	long y = strtol(yString, &yRemainder, 10);
	long z = strtol(zString, &zRemainder, 10);

	//if((!strlen(xRemainder)) && (!strlen(yRemainder)) && (!strlen(zRemainder))) {
		send_XYZ_message(x, y, z, 100);
		//}
	return pdFALSE;

}

static BaseType_t prvMoveCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {

	/* Get parameters from command string */
	long xLen, yLen;
	const char* xString = FreeRTOS_CLIGetParameter(pcCommandString, 1, &xLen);
	const char* yString = FreeRTOS_CLIGetParameter(pcCommandString, 2, &yLen);

	char* xRemainder, yRemainder;
	long x = strtol(xString, &xRemainder, 10);
	long y = strtol(yString, &yRemainder, 10);

	//if((!strlen(xRemainder)) && (!strlen(yRemainder)) && (!strlen(zRemainder))) {
		send_XY_message(x, y, 100);

	//}

	return pdFALSE;

}

static BaseType_t prvPenCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {

	/* Get parameters from command string */
	long paramLen;
	const char* param = FreeRTOS_CLIGetParameter(pcCommandString, 1, &paramLen);

	send_join_message(100);

	if(!strncmp((const char*) param, "up", 2)) {
		send_Z_message(DEFAULT_UP_Z_VALUE, 100);
	} else if(!strncmp((const char*) param, "down", 4)) {
		send_Z_message(DEFAULT_DOWN_Z_VALUE, 100);
	}

	return pdFALSE;

}

static BaseType_t prvOriginCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {

	send_join_message(100);
	send_XYZ_message(0, 0, 0, 100);

}

static BaseType_t prvLineCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {

	/* Get parameters from command string */
	long xLen, yLen, typeLen, lengthLen;
	const char* xString = FreeRTOS_CLIGetParameter(pcCommandString, 1, &xLen);
	const char* yString = FreeRTOS_CLIGetParameter(pcCommandString, 2, &yLen);
	const char* typeString = FreeRTOS_CLIGetParameter(pcCommandString, 3, &typeLen);
	const char* lengthString = FreeRTOS_CLIGetParameter(pcCommandString, 4, &lengthLen);

	char* xRemainder, yRemainder, lengthRemainder;
	long x = strtol(xString, &xRemainder, 10);
	long y = strtol(yString, &yRemainder, 10);
	long length = strtol(lengthString, &lengthRemainder, 10);

	//if((!strlen(xRemainder)) && (!strlen(yRemainder)) && (!strlen(lengthRemainder))
	//	&& (strlen(typeString) == 1) && ((typeString[0] == 'h') || (typeString[0] == 'v'))) {
		send_join_message(100);
		send_Z_message(DEFAULT_UP_Z_VALUE, 100);
		send_XYZ_message(x, y, DEFAULT_DOWN_Z_VALUE, 100);
		if(typeString[0] == 'h') {
			send_XY_message(x + length, y, 100);
		} else {
			send_XY_message(x, y + length, 100);
		}


	//}

	return pdFALSE;

}

static BaseType_t prvSquareCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {

	/* Get parameters from command string */
	long xLen, yLen, sideLen;
	const char* xString = FreeRTOS_CLIGetParameter(pcCommandString, 1, &xLen);
	const char* yString = FreeRTOS_CLIGetParameter(pcCommandString, 2, &yLen);
	const char* sideString = FreeRTOS_CLIGetParameter(pcCommandString, 3, &sideLen);

	char* xRemainder, yRemainder, sideRemainder;
	long x = strtol(xString, &xRemainder, 10);
	long y = strtol(yString, &yRemainder, 10);
	long side = strtol(sideString, &sideRemainder, 10);

	//if((!strlen(xRemainder)) && (!strlen(yRemainder)) && (!strlen(sideRemainder))) {
	send_join_message(100);
	send_Z_message(DEFAULT_UP_Z_VALUE, 100);
	send_XYZ_message(x, y, DEFAULT_DOWN_Z_VALUE, 100);
	//send_XY_message(x + side, y, 100);
	send_XY_message(x + side, y + side, 100);
	//send_XY_message(x, y + side, 100);
	send_XY_message(x, y, 100);
}

static BaseType_t prvBlineCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {

	/* Get parameters from command string */
	long x1Len, y1Len, x2Len, y2Len, stepSizeLen;
	const char* x1String = FreeRTOS_CLIGetParameter(pcCommandString, 1, &x1Len);
	const char* y1String = FreeRTOS_CLIGetParameter(pcCommandString, 2, &y1Len);
	const char* x2String = FreeRTOS_CLIGetParameter(pcCommandString, 3, &x2Len);
	const char* y2String = FreeRTOS_CLIGetParameter(pcCommandString, 4, &y2Len);
	const char* stepSizeString = FreeRTOS_CLIGetParameter(pcCommandString, 5, &stepSizeLen);

	char* x1Remainder, y1Remainder, x2Remainder, y2Remainder, stepSizeRemainder;
	long x1 = strtol(x1String, &x1Remainder, 10);
	long y1 = strtol(y1String, &y1Remainder, 10);
	long x2 = strtol(x2String, &x2Remainder, 10);
	long y2 = strtol(y2String, &y2Remainder, 10);
	long stepSize = strtol(stepSizeString, &stepSizeRemainder, 10);

	//if(strlen(x1Remainder) || strlen(y1Remainder) ||
	//		strlen(x2Remainder) || strlen(y2Remainder) || strlen(stepSizeRemainder)) {
	//	return pdFALSE;
	//}

	//Bresenham Line Generation
	send_join_message(100);
	send_Z_message(DEFAULT_UP_Z_VALUE, 100);
	send_XYZ_message(x1, y1, DEFAULT_DOWN_Z_VALUE, 100);

	int dx = x2 - x1;
	int dy = y2 - y1;
	int xi = x1;
	int yi = y1;
	int D;

	//m < 1
	if(dx > dy) {
		D = (2 * dy) - dx;

		for(int i = 0; i < dx; i++) {
			if(D > 0) {
				yi++;
				D -= (2 * dx);
				if(!(i % stepSize)) {
					send_XY_message(xi, yi, 100);
				}
			} else if(!(i % stepSize)) {
				send_XY_message(xi, yi, 100);
			}

			D += (2 * dy);
			xi++;
		}

		//m > 1
	} else {
		D = (2 * dx) - dy;

		for(int i = 0; i < dy; i++) {
			if(D > 0) {
				xi++;
				D -= (2 * dy);
				if(!(i % stepSize)) {
					send_XY_message(xi, yi, 100);
				}
			} else if(!(i % stepSize)) {
				send_XY_message(xi, yi, 100);
			}

			D += (2 * dx);
			yi++;
		}
	}

	send_XYZ_message(xi, yi, DEFAULT_UP_Z_VALUE, 100);


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

CLI_Command_Definition_t origin = {
	"origin",
	"origin: Moves the pen to the origin (0, 0, 0) via radio packet.\r\n",
	prvOriginCommand,
	0
};

CLI_Command_Definition_t line = {
	"line",
	"line: Draws a line from (x1, y1), either horizontal or vertical, of specified length.\r\n",
	prvLineCommand,
	4
};

CLI_Command_Definition_t square = {
	"square",
	"square: Draws a square from (x1, y1) of specified length.\r\n",
	prvSquareCommand,
	3
};

CLI_Command_Definition_t bline = {
	"bline",
	"bline: Draws a line from (x1, y1) to (x2, y2) using Bresenham's algorithm.\r\n",
	prvBlineCommand,
	5
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
	FreeRTOS_CLIRegisterCommand(&origin);
	FreeRTOS_CLIRegisterCommand(&line);
	FreeRTOS_CLIRegisterCommand(&square);
	FreeRTOS_CLIRegisterCommand(&bline);
}
