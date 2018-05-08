/**
 ******************************************************************************
 * @file    mylib/s4435360_os_printf.c
 * @author  Samuel Eadie - 44353607
 * @date    07052018
 * @brief   Thread safe printf
 *
 ******************************************************************************
 *     EXTERNAL FUNCTIONS
 ******************************************************************************
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <s4435360_os_printf.h>

#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "board.h"
#include "debug_printf.h"
#include <stdarg.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define EVER						;;
#define PRINTF_QUEUE_LENGTH			10
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

void send_to_queue(const char* format, ...) {
	va_list args;
	char message[80];

	va_start (args, format);
	vsprintf(message, format, args);
	va_end (args);

	xQueueSendToBack(s4435360_QueuePrintf, message, 100);

}

void s4435360_TaskPrintf(void) {

	s4435360_QueuePrintf = xQueueCreate(PRINTF_QUEUE_LENGTH, sizeof(char*));

	char* messageToPrint;
	for(EVER) {
		if(xQueueReceive(s4435360_QueuePrintf, &messageToPrint, 100)) {
			debug_printf(messageToPrint);
		}
	}
}
