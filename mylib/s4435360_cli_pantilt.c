/**
 ******************************************************************************
 * @file    mylib/s4435360_cli_pantilt.c
 * @author  Samuel Eadie - 44353607
 * @date    14052018
 * @brief   Provides CLI commands for pantilt
 *
 ******************************************************************************
 *     EXTERNAL FUNCTIONS
 ******************************************************************************
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <s4435360_os_pantilt.h>

#include "s4435360_hal_pantilt.h"
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

/**
 * @brief	Handles pantilt position CLI commands, for testing
 * @param	pcWriteBuffer: buffer to write command outputs to
 * 			xWriteBufferLen: length of chars written to the buffer
 * 			pcCommandString: the input command
 * @retval	returns pdFALSE
 */
static BaseType_t prvPantiltCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {

	/* Get parameters from command string */
	long xLen, yLen;
	const char* xString = FreeRTOS_CLIGetParameter(pcCommandString, 1, &xLen);
	const char* yString = FreeRTOS_CLIGetParameter(pcCommandString, 2, &yLen);

	char* xRemainder, *yRemainder;
	long x = strtol(xString, &xRemainder, 10);
	long y = strtol(yString, &yRemainder, 10);

	s4435360_pantilt_changeX(x);
	s4435360_pantilt_changeY(y);
	xSemaphoreGive(s4435360_SemaphoreUpdatePantilt);

	return pdFALSE;

}

/**
 * @brief	Handles pantilt angle CLI commands, for testing
 * @param	pcWriteBuffer: buffer to write command outputs to
 * 			xWriteBufferLen: length of chars written to the buffer
 * 			pcCommandString: the input command
 * @retval	returns pdFALSE
 */
static BaseType_t prvPantiltAngleCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {

	/* Get parameters from command string */
	long panAngleLen, tiltAngleLen;
	const char* panAngleString = FreeRTOS_CLIGetParameter(pcCommandString, 1, &panAngleLen);
	const char* tiltAngleString = FreeRTOS_CLIGetParameter(pcCommandString, 2, &tiltAngleLen);

	char* xRemainder, *yRemainder;
	long panAngle = strtol(panAngleString, &xRemainder, 10);
	long tiltAngle = strtol(tiltAngleString, &yRemainder, 10);

	s4435360_hal_pantilt_pan_write(panAngle);
	s4435360_hal_pantilt_tilt_write(tiltAngle);

	return pdFALSE;

}

//CLI definition for pantilt position command
CLI_Command_Definition_t pantilt = {
	"pantilt",
	"pantilt: moves the pantilt to point at the specified (x, y).\r\n",
	prvPantiltCommand,
	2
};

//CLI definition for pantilt angle command
CLI_Command_Definition_t pantiltangle = {
	"pantiltangle",
	"pantilt angle: moves the pantilt to the specified angle.\r\n",
	prvPantiltAngleCommand,
	2
};


/**
 * @brief	Register all pantilt CLI commands
 * @param	None
 * @retval	None
 */
void register_pantilt_CLI_commands(void) {

	FreeRTOS_CLIRegisterCommand(&pantilt);
	FreeRTOS_CLIRegisterCommand(&pantiltangle);

}
