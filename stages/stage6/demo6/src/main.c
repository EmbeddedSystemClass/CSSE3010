/**
  ******************************************************************************
  * @file    demo6/main.c
  * @author  SE
  * @date    07052018
  * @brief   Demonstrates further OS functionality for demo 6
  ******************************************************************************
  *
  */

#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "board.h"
#include "debug_printf.h"

#include "FreeRTOS_CLI.h"

#include "s4435360_os_printf.h"
#include "s4435360_os_pantilt.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define EVER ;;

/* Task stack size definitions */
#define TASK1_STACK_SIZE		( configMINIMAL_STACK_SIZE * 20 )
#define CLI_STACK_SIZE			( configMINIMAL_STACK_SIZE * 20 )
#define PRINTF_STACK_SIZE		( configMINIMAL_STACK_SIZE * 20 )
#define PANTILT_STACK_SIZE		( configMINIMAL_STACK_SIZE * 20 )

/* Task priority definitions*/
#define TASK1_PRIORITY			( tskIDLE_PRIORITY + 2 )
#define CLI_TASK_PRIORITY		( tskIDLE_PRIORITY + 5 )
#define PRINTF_TASK_PRIORITY	( tskIDLE_PRIORITY + 5 )
#define PANTILT_TASK_PRIORITY	( tskIDLE_PRIORITY + 3 )

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static BaseType_t prvTiltCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvPanCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
void CLI_Task(void);

CLI_Command_Definition_t pan = {
	"pan",
	"pan: Moves the pan to the specified angle, or moves it left or right 5 degrees.\r\n",
	prvPanCommand,
	1
};

CLI_Command_Definition_t tilt = {
	"tilt",
	"tilt: Moves the tilt to the specified angle, or moves it left or right 5 degrees.\r\n",
	prvTiltCommand,
	1
};

/**
  * @brief  Task 1 functionality
  * @param  None
  * @retval None
  */
void Task1_Task(void) {
	int i = 0;

	//Infinite loop
	for(EVER) {
		myprintf("hello number is %d\r\n", i++);
		vTaskDelay(1000);
	}
}

/**
  * @brief  vApplicationStackOverflowHook
  * @param  Task Handler and Task Name
  * @retval None
  */
void vApplicationStackOverflowHook( xTaskHandle pxTask, signed char *pcTaskName ) {
	/* This function will get called if a task overflows its stack.   If the
	parameters are corrupt then inspect pxCurrentTCB to find which was the
	offending task. */

	//BRD_LEDBlueOff();
	( void ) pxTask;
	( void ) pcTaskName;

	portDISABLE_INTERRUPTS();
	debug_printf("Stack overflow for: '%s'\r\n", pcTaskName);
	portENABLE_INTERRUPTS();

	for( EVER );
}

int main(void) {

	//Initialise hardware
	BRD_init();
	HAL_TIM_PWM_ConfigChannel(NULL, NULL, NULL);

	//Create tasks
	xTaskCreate((void *) &Task1_Task,
			(const char *) "TASK1", TASK1_STACK_SIZE,
			NULL, TASK1_PRIORITY, NULL);
	xTaskCreate((void *)  &CLI_Task,
			(const char *) "CLI", CLI_STACK_SIZE,
			NULL, CLI_TASK_PRIORITY, NULL);
	xTaskCreate((void *) &s4435360_TaskPrintf,
			(const char *) "PRINTF", PRINTF_STACK_SIZE,
			NULL, PRINTF_TASK_PRIORITY, NULL);
	xTaskCreate((void *) &s4435360_TaskPanTilt,
			(const char *) "PANTILT", PANTILT_STACK_SIZE,
			NULL, PANTILT_TASK_PRIORITY, NULL);
	FreeRTOS_CLIRegisterCommand(&pan);
	FreeRTOS_CLIRegisterCommand(&tilt);

	//Start scheduler
	vTaskStartScheduler();

	return 0;
}



/**
  * @brief  CLI Receive Task.
  * @param  None
  * @retval None
  */
void CLI_Task(void) {

	int i;
	char cRxedChar;
	char cInputString[100];
	int InputIndex = 0;
	char *pcOutputString;
	BaseType_t xReturned;

	/* Initialise pointer to CLI output buffer. */
	memset(cInputString, 0, sizeof(cInputString));
	pcOutputString = FreeRTOS_CLIGetOutputBuffer();

	for (;;) {

		/* Receive character from terminal */
		cRxedChar = debug_getc();

		/* Process if character if not Null */
		if (cRxedChar != '\0') {

			/* Echo character */
			debug_putc(cRxedChar);

			/* Process only if return is received. */
			if (cRxedChar == '\r') {

				//Put new line and transmit buffer
				debug_putc('\n');
				debug_flush();

				/* Put null character in command input string. */
				cInputString[InputIndex] = '\0';

				xReturned = pdTRUE;
				/* Process command input string. */
				while (xReturned != pdFALSE) {

					/* Returns pdFALSE, when all strings have been returned */
					xReturned = FreeRTOS_CLIProcessCommand( cInputString, pcOutputString, configCOMMAND_INT_MAX_OUTPUT_SIZE );

					/* Display CLI command output string (not thread safe) */
					portENTER_CRITICAL();
					for (i = 0; i < strlen(pcOutputString); i++) {
						debug_putc(*(pcOutputString + i));
					}
					portEXIT_CRITICAL();

				    vTaskDelay(5);	//Must delay between debug_printfs.
				}

				memset(cInputString, 0, sizeof(cInputString));
				InputIndex = 0;

			} else {

				debug_flush();		//Transmit USB buffer

				if( cRxedChar == '\r' ) {

					/* Ignore the character. */
				} else if( cRxedChar == '\b' ) {

					/* Backspace was pressed.  Erase the last character in the
					 string - if any.*/
					if( InputIndex > 0 ) {
						InputIndex--;
						cInputString[ InputIndex ] = '\0';
					}

				} else {

					/* A character was entered.  Add it to the string
					   entered so far.  When a \n is entered the complete
					   string will be passed to the command interpreter. */
					if( InputIndex < 20 ) {
						cInputString[ InputIndex ] = cRxedChar;
						InputIndex++;
					}
				}
			}
		}

		vTaskDelay(50);
	}
}

/**
  * @brief  Echo Command.
  * @param  writebuffer, writebuffer length and command strength
  * @retval None
  */
static BaseType_t prvPanCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {
	const char *cCmd_string;

	/* Get parameters from command string */
	long paramLen;
	const char* parameter = FreeRTOS_CLIGetParameter(pcCommandString, 1, &paramLen);

	if(strcmp((const char*) parameter, "left")) {
		xSemaphoreGive(s4435360_SemaphorePanLeft);
	} else if(strcmp((const char*) parameter, "right")) {
		xSemaphoreGive(s4435360_SemaphorePanRight);
	} else if(atoi(parameter)) {
		xQueueSend(s4435360_QueuePan, &parameter, 100);
	}

	return pdFALSE;
}

static BaseType_t prvTiltCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {
	const char *cCmd_string;

	/* Get parameters from command string */
	long paramLen;
	const char* parameter = FreeRTOS_CLIGetParameter(pcCommandString, 1, &paramLen);

	if(strcmp((const char*) parameter, "up")) {
		xSemaphoreGive(s4435360_SemaphoreTiltUp);
	} else if(strcmp((const char*) parameter, "down")) {
		xSemaphoreGive(s4435360_SemaphoreTiltDown);
	} else if(atoi(parameter)) {
		xQueueSend(s4435360_QueueTilt, &parameter, 100);
	}

	return pdFALSE;
}
