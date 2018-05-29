/**
  ******************************************************************************
  * @file    proj2/main.c
  * @author  Sam Eadie
  * @date    09052018
  * @brief   Functionality to control plotter for project 2
  *
  ******************************************************************************
  *
  */


/* Includes ------------------------------------------------------------------*/
#include "board.h"
#include "stm32f4xx_hal_conf.h"
#include "stm32f4xx_hal_dac.h"
#include "debug_printf.h"
#include <string.h>
#include <stdio.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "FreeRTOS_CLI.h"

#include "s4435360_os_radio.h"
#include "s4435360_os_printf.h"
#include "s4435360_cli_radio.h"
#include "s4435360_os_pantilt.h"
#include "stm32f4xx_hal_dac.h"
#include "s4435360_hal_dac.h"
#include "s4435360_os_dac.h"
#include "stm32f4xx_hal_dac.h"
#include "s4435360_os_control.h"
#include "s4435360_os_ir.h"
#include "s4435360_cli_pantilt.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void CLI_Task(void);
/* Private variables ---------------------------------------------------------*/
/* Task Priorities ------------------------------------------------------------*/
#define CLI_TASK_PRIORITY		( tskIDLE_PRIORITY + 2 )
#define RADIO_TASK_PRIORITY		( tskIDLE_PRIORITY + 3 )
#define PRINTF_TASK_PRIORITY	( tskIDLE_PRIORITY + 2 )
#define PANTILT_TASK_PRIORITY 	( tskIDLE_PRIORITY + 1 )
#define DAC_TASK_PRIORITY		( tskIDLE_PRIORITY + 2 )
#define CONTROL_TASK_PRIORITY	( tskIDLE_PRIORITY + 2 )
#define IR_TASK_PRIORITY		( tskIDLE_PRIORITY + 3 )
/* Task Stack Allocations -----------------------------------------------------*/
#define CLI_TASK_STACK_SIZE		( configMINIMAL_STACK_SIZE * 5 )
#define RADIO_TASK_STACK_SIZE 	( configMINIMAL_STACK_SIZE * 5 )
#define PRINTF_TASK_STACK_SIZE	( configMINIMAL_STACK_SIZE * 5 )
#define PANTILT_TASK_STACK_SIZE	( configMINIMAL_STACK_SIZE * 5 )
#define DAC_TASK_STACK_SIZE		( configMINIMAL_STACK_SIZE * 5 )
#define CONTROL_TASK_STACK_SIZE	( configMINIMAL_STACK_SIZE * 5 )
#define IR_TASK_STACK_SIZE		( configMINIMAL_STACK_SIZE * 3 )



/**
  * @brief  Starts all the other tasks, then starts the scheduler.
  * @param  None
  * @retval None
  */
int main( void ) {

	BRD_init();
	radio_fsm_getstate();
	HAL_TIM_PWM_Init(NULL);
	HAL_DAC_Init(NULL);

	//Start tasks
	xTaskCreate( (void *) &CLI_Task, (const char *) "CLI",
			CLI_TASK_STACK_SIZE, NULL, CLI_TASK_PRIORITY, NULL);
	xTaskCreate( (void *) &s4435360_TaskRadio, (const char *) "RADIO",
			RADIO_TASK_STACK_SIZE, NULL, RADIO_TASK_PRIORITY, NULL);
	xTaskCreate( (void *) &s4435360_TaskPrintf, (const char *) "PRINTF",
			PRINTF_TASK_STACK_SIZE, NULL, PRINTF_TASK_PRIORITY, NULL);
	xTaskCreate( (void *) &s4435360_TaskControl, (const char *) "CONTROL",
				CONTROL_TASK_STACK_SIZE, NULL, CONTROL_TASK_PRIORITY, NULL);
	xTaskCreate( (void *) &s4435360_IRTask, (const char *) "IR",
			IR_TASK_STACK_SIZE, NULL, IR_TASK_PRIORITY, NULL);
	xTaskCreate( (void *) &s4435360_TaskPanTilt, (const char *) "PANTILT",
			PANTILT_TASK_STACK_SIZE, NULL, PANTILT_TASK_PRIORITY, NULL);
	//xTaskCreate((void*) &s4435360_DACTask, (const char*) "DAC",
	//		DAC_TASK_STACK_SIZE, NULL, DAC_TASK_PRIORITY, NULL);
	/* Register CLI commands */
	register_radio_CLI_commands();
	register_pantilt_CLI_commands();

	/* Start the scheduler. */
	vTaskStartScheduler();

	/* We should never get here as control is now taken by the scheduler. */
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
					if( InputIndex < 100 ) {
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
  * @brief  vApplicationStackOverflowHook
  * @param  Task Handler and Task Name
  * @retval None
  */
void vApplicationStackOverflowHook( xTaskHandle pxTask, signed char *pcTaskName ) {
	/* This function will get called if a task overflows its stack.   If the
	parameters are corrupt then inspect pxCurrentTCB to find which was the
	offending task. */

	BRD_LEDBlueOff();
	( void ) pxTask;
	( void ) pcTaskName;

	portDISABLE_INTERRUPTS();
	debug_printf("Stack overflow from '%s'\r\n", pcTaskName);
	portENABLE_INTERRUPTS();

	for( ;; );
}
