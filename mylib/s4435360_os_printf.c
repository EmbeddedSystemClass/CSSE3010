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

typedef struct {
	char message[80];
} PrintElement;

/* Private define ------------------------------------------------------------*/
#define EVER						;;
#define PRINTF_QUEUE_LENGTH			10
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

void send_to_queue(const char* format, ...) {
	
	PrintElement message = {0};

	va_list args;

	va_start (args, format);
	vsprintf(message.message, format, args);
	va_end (args);
	if(s4435360_QueuePrintf != NULL) {
		xQueueSendToBack(s4435360_QueuePrintf, (void *) &message, 0);
	}
}

void s4435360_TaskPrintf(void) {

	s4435360_QueuePrintf = xQueueCreate(PRINTF_QUEUE_LENGTH, sizeof(PrintElement));

	PrintElement messageToPrint = {0};
	for(EVER) {
		if(xQueueReceive(s4435360_QueuePrintf, &messageToPrint, 100)) {
			debug_printf(messageToPrint.message);
		}
	}
}
