/**
 ******************************************************************************
 * @file    mylib/s4435360_cli_graphics.c
 * @author  Samuel Eadie - 44353607
 * @date    24052018
 * @brief   Provides CLI commands for graphics
 *
 ******************************************************************************
 *     EXTERNAL FUNCTIONS
 ******************************************************************************
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
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
#include "board.h"
#include "stm32f4xx_hal.h"
#include <math.h>
#include "s4435360_os_control.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define PI				3.14159265359
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
 * @brief	Handles origin CLI commands
 * @param	pcWriteBuffer: buffer to write command outputs to
 * 			xWriteBufferLen: length of chars written to the buffer
 * 			pcCommandString: the input command
 * @retval	returns pdFALSE
 */
static BaseType_t prvOriginCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {

	Command originCommand;
	originCommand.type = origin;

	xQueueSendToBack(s4435360_QueueCommands, (void*) &originCommand, portMAX_DELAY);
	return pdFALSE;
}

/**
 * @brief	Handles line CLI commands
 * @param	pcWriteBuffer: buffer to write command outputs to
 * 			xWriteBufferLen: length of chars written to the buffer
 * 			pcCommandString: the input command
 * @retval	returns pdFALSE
 */
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

	if((!strlen(xRemainder)) && (!strlen(yRemainder)) && (!strlen(lengthRemainder))
		&& (strlen(typeString) == 1) && ((typeString[0] == 'h') || (typeString[0] == 'v'))) {

		Command lineCommand;
		lineCommand.type = line;

		lineCommand.args[0] = x;
		lineCommand.args[1] = y;
		lineCommand.args[2] = typeString[0];
		lineCommand.args[3] = length;
		xQueueSendToBack(s4435360_QueueCommands, (void*) &lineCommand, portMAX_DELAY);
	}

	return pdFALSE;
}

/**
 * @brief	Handles square CLI commands
 * @param	pcWriteBuffer: buffer to write command outputs to
 * 			xWriteBufferLen: length of chars written to the buffer
 * 			pcCommandString: the input command
 * @retval	returns pdFALSE
 */
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

	Command squareCommand;
	squareCommand.type = square;
	squareCommand.args[0] = x;
	squareCommand.args[1] = y;
	squareCommand.args[2] = side;
	xQueueSendToBack(s4435360_QueueCommands, (void*) &squareCommand, portMAX_DELAY);
	//send_join_message(portMAX_DELAY);
	//move_square(x, y, side);

	return pdFALSE;

}

/**
 * @brief	Handles Bresenham line CLI commands
 * @param	pcWriteBuffer: buffer to write command outputs to
 * 			xWriteBufferLen: length of chars written to the buffer
 * 			pcCommandString: the input command
 * @retval	returns pdFALSE
 */
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

	Command blineCommand;
	blineCommand.type = bline;
	blineCommand.args[0] = x1;
	blineCommand.args[1] = y1;
	blineCommand.args[2] = x2;
	blineCommand.args[3] = y2;
	blineCommand.args[4] = stepSize;

	xQueueSendToBack(s4435360_QueueCommands, (void*) &blineCommand, portMAX_DELAY);

	return pdFALSE;

}

/**
 * @brief	Handles n polygon CLI commands
 * @param	pcWriteBuffer: buffer to write command outputs to
 * 			xWriteBufferLen: length of chars written to the buffer
 * 			pcCommandString: the input command
 * @retval	returns pdFALSE
 */
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

	Command polygonCommand;
	polygonCommand.type = polygon;
	polygonCommand.args[0] = n;
	polygonCommand.args[1] = x1;
	polygonCommand.args[2] = y1;
	polygonCommand.args[3] = radius;

	xQueueSendToBack(s4435360_QueueCommands, (void*) &polygonCommand, portMAX_DELAY);

	return pdFALSE;

}

/**
 * @brief	Handles rose compass CLI commands
 * @param	pcWriteBuffer: buffer to write command outputs to
 * 			xWriteBufferLen: length of chars written to the buffer
 * 			pcCommandString: the input command
 * @retval	returns pdFALSE
 */
static BaseType_t prvRoseCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {

	/* Get parameters from command string */
	long x1Len, y1Len, sideLengthLen, incrementLen;
	const char* x1String = FreeRTOS_CLIGetParameter(pcCommandString, 1, &x1Len);
	const char* y1String = FreeRTOS_CLIGetParameter(pcCommandString, 2, &y1Len);
	const char* sideLengthString = FreeRTOS_CLIGetParameter(pcCommandString, 3, &sideLengthLen);
	const char* incrementString = FreeRTOS_CLIGetParameter(pcCommandString, 4, &incrementLen);

	char* x1Remainder, *y1Remainder, *sideLengthRemainder, *incrementRemainder;
	long x1 = strtol(x1String, &x1Remainder, 10);
	long y1 = strtol(y1String, &y1Remainder, 10);
	long sideLength = strtol(sideLengthString, &sideLengthRemainder, 10);
	long increment = strtol(incrementString, &incrementRemainder, 10);

	Command roseCommand;
	roseCommand.type = rose;
	roseCommand.args[0] = x1;
	roseCommand.args[1] = y1;
	roseCommand.args[2] = sideLength;
	roseCommand.args[3] = increment;

	xQueueSendToBack(s4435360_QueueCommands, (void*) &roseCommand, portMAX_DELAY);

	return pdFALSE;

}


//CLI definition for origin command
CLI_Command_Definition_t originCommand = {
		"origin",
		"origin: Moves the pen to the origin (0, 0, 0) via radio packet.\r\n",
		prvOriginCommand,
		0
};

//CLI definition for line command
CLI_Command_Definition_t lineCommand = {
		"line",
		"line: Draws a line from (x1, y1), either horizontal or vertical, of specified length.\r\n",
		prvLineCommand,
		4
};

//CLI definition for square command
CLI_Command_Definition_t squareCommand = {
		"square",
		"square: Draws a square from (x1, y1) of specified length.\r\n",
		prvSquareCommand,
		3
};

//CLI definition for bresenham line command
CLI_Command_Definition_t blineCommand = {
		"bline",
		"bline: Draws a line from (x1, y1) to (x2, y2) using Bresenham's algorithm.\r\n",
		prvBlineCommand,
		5
};

//CLI definition for polygon command
CLI_Command_Definition_t polygonCommand = {
		"polygon",
		"polygon: Draws an n-sided regular polygon of specified radius starting at (x1, y1).\r\n",
		prvNPolygonCommand,
		4
};

//CLI definition for rose compass command
CLI_Command_Definition_t roseCommand = {
		"rose",
		"rose: Draws a compass rose of specified size at (x1, y1).\r\n",
		prvRoseCommand,
		4
};


/**
 * @brief	Registers all CLI graphics commands
 * @param	None
 * @retval	None
 */
void register_graphics_CLI_commands(void) {

	FreeRTOS_CLIRegisterCommand(&originCommand);
	FreeRTOS_CLIRegisterCommand(&lineCommand);
	FreeRTOS_CLIRegisterCommand(&squareCommand);
	FreeRTOS_CLIRegisterCommand(&blineCommand);
	FreeRTOS_CLIRegisterCommand(&polygonCommand);
	FreeRTOS_CLIRegisterCommand(&roseCommand);
}
