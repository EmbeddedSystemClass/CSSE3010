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

static BaseType_t prvOriginCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {

	send_join_message(portMAX_DELAY);
	send_XYZ_message(0, 0, 0, portMAX_DELAY);

}

static BaseType_t prvLineCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {

	/* Get parameters from command string */
	long xLen, yLen, typeLen, lengthLen;
	const char* xString = FreeRTOS_CLIGetParameter(pcCommandString, 1, &xLen);
	const char* yString = FreeRTOS_CLIGetParameter(pcCommandString, 2, &yLen);
	const char* typeString = FreeRTOS_CLIGetParameter(pcCommandString, 3, &typeLen);
	const char* lengthString = FreeRTOS_CLIGetParameter(pcCommandString, 4, &lengthLen);

	char* xRemainder, *yRemainder, *lengthRemainder;
	long x = strtol(xString, &xRemainder, 10);
	long y = strtol(yString, &yRemainder, 10);
	long length = strtol(lengthString, &lengthRemainder, 10);

	//if((!strlen(xRemainder)) && (!strlen(yRemainder)) && (!strlen(lengthRemainder))
	//	&& (strlen(typeString) == 1) && ((typeString[0] == 'h') || (typeString[0] == 'v'))) {
	send_join_message(portMAX_DELAY);
	send_Z_message(DEFAULT_UP_Z_VALUE, portMAX_DELAY);
	send_XYZ_message(x, y, DEFAULT_DOWN_Z_VALUE, portMAX_DELAY);
	if(typeString[0] == 'h') {
		send_XY_message(x + length, y, portMAX_DELAY);
	} else {
		send_XY_message(x, y + length, portMAX_DELAY);
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

	char* xRemainder, *yRemainder, *sideRemainder;
	long x = strtol(xString, &xRemainder, 10);
	long y = strtol(yString, &yRemainder, 10);
	long side = strtol(sideString, &sideRemainder, 10);

	//if((!strlen(xRemainder)) && (!strlen(yRemainder)) && (!strlen(sideRemainder))) {
	send_join_message(portMAX_DELAY);
	send_Z_message(DEFAULT_UP_Z_VALUE, portMAX_DELAY);
	send_XYZ_message(x, y, DEFAULT_DOWN_Z_VALUE, portMAX_DELAY);
	send_XY_message(x + side, y + side, portMAX_DELAY);
	send_XY_message(x, y, portMAX_DELAY);

	return pdFALSE;
}

void bline_low(int x1, int y1, int x2, int y2, int stepSize) {
	int dx = x2 - x1;
	int dy = y2 - y1 > 0 ? y2 - y1 : y1 - y2;
	int deltay = y2 - y1 > 0 ? 1 : -1;
	int xi = x1;
	int yi = y1;
	int D = (2 * dy) - dx;

	for(int i = 0; i < dx; i++) {
		if(!(i % stepSize)) {
			send_XYZ_message(xi, yi, DEFAULT_DOWN_Z_VALUE, portMAX_DELAY);
		}

		if(D > 0) {
			yi += deltay;
			D -= (2 * dx);
		}

		D += (2 * dy);
		xi += 1;
	}

	send_XYZ_message(x2, y2, DEFAULT_UP_Z_VALUE, portMAX_DELAY);
}

void bline_high(int x1, int y1, int x2, int y2, int stepSize) {
	int dx = x2 - x1 > 0 ? x2 - x1 : x1 - x2;
	int dy = y2 - y1;
	int deltax = x2 - x1 > 0 ? 1 : -1;
	int xi = x1;
	int yi = y1;
	int D = (2 * dx) - dy;

	for(int i = 0; i < dy; i++) {
		if(!(i % stepSize)) {
			send_XYZ_message(xi, yi, DEFAULT_DOWN_Z_VALUE, portMAX_DELAY);
		}

		if(D > 0) {
			xi += deltax;
			D -= (2 * dy);
		}

		D += (2 * dx);
		yi+=1;
	}

	send_XYZ_message(x2, y2, DEFAULT_UP_Z_VALUE, portMAX_DELAY);
}

void bresenham_line(int x1, int y1, int x2, int y2, int stepSize) {
	if(((y2 - y1)*(y2 - y1)) < ((x2 - x1)*(x2 - x1))) {
			if(x1 > x2) {
				myprintf("1\r\n");
				bline_low(x2, y2, x1, y1, stepSize);
			} else {
				myprintf("2\r\n");
				bline_low(x1, y1, x2, y2, stepSize);
			}
		} else {
			if(y1 > y2) {
				myprintf("3\r\n");
				bline_high(x2, y2, x1, y1, stepSize);
			} else {
				myprintf("4\r\n");
				bline_high(x1, y1, x2, y2, stepSize);
			}
		}

}
static BaseType_t prvBlineCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {

	/* Get parameters from command string */
	long x1Len, y1Len, x2Len, y2Len, stepSizeLen;
	const char* x1String = FreeRTOS_CLIGetParameter(pcCommandString, 1, &x1Len);
	const char* y1String = FreeRTOS_CLIGetParameter(pcCommandString, 2, &y1Len);
	const char* x2String = FreeRTOS_CLIGetParameter(pcCommandString, 3, &x2Len);
	const char* y2String = FreeRTOS_CLIGetParameter(pcCommandString, 4, &y2Len);
	const char* stepSizeString = FreeRTOS_CLIGetParameter(pcCommandString, 5, &stepSizeLen);

	char* x1Remainder, *y1Remainder, *x2Remainder, *y2Remainder, *stepSizeRemainder;
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
	send_join_message(portMAX_DELAY);
	send_Z_message(DEFAULT_UP_Z_VALUE, portMAX_DELAY);

	bresenham_line(x1, y1, x2, y2, stepSize);


	return pdFALSE;

}

void n_polygon(int n, int x1, int y1, int radius) {
	int x[n];
	int y[n];
	float thetaOffset = ((n - 2) * PI) / 2; //Straighten bottom side
	float centeringOffsetX = radius * cos(thetaOffset / 2);
	float centeringOffsetY = radius * sin(thetaOffset / 2);

	myprintf("%f, %f\r\n", centeringOffsetX, centeringOffsetY);

	//Calculate points in polygon
	for(int i = 0; i < n; i++) {
		x[i] = radius * cos(((2 * PI * i) - thetaOffset) / n) + x1 + centeringOffsetX;
		y[i] = radius * sin(((2 * PI * i) - thetaOffset) / n) + y1 + centeringOffsetY;
		myprintf("(%d, %d)\r\n", x[i], y[i]);

		//Check calculated points are on 200x200 board
		if((x[i] > 200) || (x[i] < 0) || (y[i] > 200) || (y[i] < 0)) {
			myprintf("Entered polygon exceeds board dimensions");
			return;
		}
	}

	//Use Bresenham to draw line between points
	for(int i = 0; i < n - 1; i++) {
		bresenham_line(x[i], y[i], x[i+1], y[i+1], radius / 10);
	}

	//Return to start
	bresenham_line(x[n-1], y[n-1], x[0], y[0], radius / 10);
}

static BaseType_t prvNPolygonCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {

	/* Get parameters from command string */
	long nLen, x1Len, y1Len, radiusLen;
	const char* nString = FreeRTOS_CLIGetParameter(pcCommandString, 1, &nLen);
	const char* x1String = FreeRTOS_CLIGetParameter(pcCommandString, 2, &x1Len);
	const char* y1String = FreeRTOS_CLIGetParameter(pcCommandString, 3, &y1Len);
	const char* radiusString = FreeRTOS_CLIGetParameter(pcCommandString, 4, &radiusLen);

	char* nRemainder, *x1Remainder, *y1Remainder, *radiusRemainder;
	long n = strtol(nString, &nRemainder, 10);
	long x1 = strtol(x1String, &x1Remainder, 10);
	long y1 = strtol(y1String, &y1Remainder, 10);
	long radius = strtol(radiusString, &radiusRemainder, 10);

	//if(strlen(x1Remainder) || strlen(y1Remainder) ||
	//		strlen(x2Remainder) || strlen(y2Remainder)) {
	//	return pdFALSE;
	//}

	send_join_message(portMAX_DELAY);
	send_Z_message(DEFAULT_UP_Z_VALUE, portMAX_DELAY);
	n_polygon(n, x1, y1, radius);

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

CLI_Command_Definition_t polygon = {
		"polygon",
		"polygon: Draws an n-sided regular polygon of specified radius starting at (x1, y1).\r\n",
		prvNPolygonCommand,
		4
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
	FreeRTOS_CLIRegisterCommand(&polygon);
}
