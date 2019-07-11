/*
*
  ******************************************************************************
  * @file    demo5/main.c
  * @author  SE
  * @date    27042018
  * @brief   Demonstrates basic OS functionality for demo 5
  ******************************************************************************
  *


#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "board.h"
#include "debug_printf.h"
#include "s4435360_hal_sysmon.h"
#include "s4435360_os_joystick.h"

 Private typedef -----------------------------------------------------------
 Private define ------------------------------------------------------------
#define EVER ;;

//Define button type
#define USER_BUTTON
//#define JOYSTICK

 Task stack size definitions
#define TASK1_STACK_SIZE		( configMINIMAL_STACK_SIZE * 2 ) //Min 17
#define TASK2_STACK_SIZE		( configMINIMAL_STACK_SIZE * 2 )
#define TASK3_STACK_SIZE		( configMINIMAL_STACK_SIZE * 2 )

 Task priority definitions
#define TASK1_PRIORITY			( tskIDLE_PRIORITY + 2 )
#define TASK2_PRIORITY			( tskIDLE_PRIORITY + 2 )
#define TASK3_PRIORITY			( tskIDLE_PRIORITY + 2 )

 Delay time definitions
#define MAJOR_DELAY_TIME		3
#define MINOR_DELAY_TIME		1

#define DEBOUNCE_THRESHOLD		((uint32_t) 100)

 Private macro -------------------------------------------------------------
 Private variables ---------------------------------------------------------
//For button debouncing
volatile uint32_t lastInterruptTime = 0;

//Task handles
TaskHandle_t task1Handle, task2Handle, task3Handle;

 Private function prototypes -----------------------------------------------

*
  * @brief  Task 0 functionality
  * @param  None
  * @retval None

void Task1_Task(void) {

	//Clear channel initially
	s4435360_hal_sysmon_chan0_clr();

	//Inifinite loop
	for(EVER) {

		s4435360_hal_sysmon_chan0_set();

		//Perform random task
		BRD_LEDGreenToggle();
		vTaskDelay(MAJOR_DELAY_TIME);

		s4435360_hal_sysmon_chan0_clr();
		vTaskDelay(MINOR_DELAY_TIME);
	}
}

*
  * @brief  Task 2 functionality
  * @param  None
  * @retval None

void Task2_Task(void) {

	s4435360_hal_sysmon_chan1_clr();

	//Infinite loop
	for(EVER) {

		s4435360_hal_sysmon_chan1_set();

		//Perform random task
		BRD_LEDRedToggle();
		vTaskDelay(MAJOR_DELAY_TIME);

		s4435360_hal_sysmon_chan1_clr();
		vTaskDelay(MINOR_DELAY_TIME);
	}
}

*
  * @brief  Task 3 functionality
  * @param  None
  * @retval None

void Task3_Task(void) {

	s4435360_hal_sysmon_chan2_clr();

	//Infinite loop
	for(EVER) {

		s4435360_hal_sysmon_chan2_set();

		//Perform random task
		BRD_LEDBlueToggle();
		vTaskDelay(MAJOR_DELAY_TIME);

		s4435360_hal_sysmon_chan2_clr();
		vTaskDelay(MINOR_DELAY_TIME);

		//Check for semaphore from input trigger
		if(s4435360_SemaphoreJoystickZ != NULL) {

			//Can successfully give if interrupt has taken
			if(xSemaphoreTake(s4435360_SemaphoreJoystickZ, 0) == pdTRUE) {

				//Handle Task 2 change
				if(task2Handle == NULL) {
					xTaskCreate((void *) &Task2_Task, (const char *) "TASK2",
											TASK2_STACK_SIZE, NULL, TASK2_PRIORITY, &task2Handle);
				} else {
					vTaskDelete(task2Handle);
					s4435360_hal_sysmon_chan1_clr(); //Ensure channel isnt left on
					task2Handle = NULL;
				}
			}
		}
	}
}

*
  * @brief  vApplicationStackOverflowHook
  * @param  Task Handler and Task Name
  * @retval None

void vApplicationStackOverflowHook( xTaskHandle pxTask, signed char *pcTaskName ) {
	 This function will get called if a task overflows its stack.   If the
	parameters are corrupt then inspect pxCurrentTCB to find which was the
	offending task.

	//BRD_LEDBlueOff();
	( void ) pxTask;
	( void ) pcTaskName;

	portDISABLE_INTERRUPTS();
	debug_printf("Stack overflow for: '%s'\r\n", pcTaskName);

	for( EVER );
}

*
  * @brief  Initialise hardware
  * @param  None
  * @retval None

void Hardware_init() {
	portDISABLE_INTERRUPTS();	//Disable interrupts

	BRD_LEDInit();					//Initialise Blue LED
	BRD_LEDRedOff();				//Turn off Red LED
	BRD_LEDGreenOff();				//Turn off Green LED
	BRD_LEDBlueOff();				//Turn off Blue LED

	//Set up button
	GPIO_InitTypeDef GPIO_InitStructure;

#ifdef JOYSTICK
	//Config joystick as input trigger
	__BRD_D13_GPIO_CLK();

	GPIO_InitStructure.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Pin  = BRD_D13_PIN;
	HAL_GPIO_Init(BRD_D13_GPIO_PORT, &GPIO_InitStructure);

	HAL_NVIC_SetPriority(BRD_D13_EXTI_IRQ, 10, 0);
	HAL_NVIC_EnableIRQ(BRD_D13_EXTI_IRQ);
#endif

#ifdef USER_BUTTON
	//Config board user button as input trigger
	BRD_USER_BUTTON_GPIO_CLK_ENABLE();

	GPIO_InitStructure.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Pin  = BRD_USER_BUTTON_PIN;
	HAL_GPIO_Init(BRD_USER_BUTTON_GPIO_PORT, &GPIO_InitStructure);

	HAL_NVIC_SetPriority(BRD_USER_BUTTON_EXTI_IRQn, 10, 0);
	HAL_NVIC_EnableIRQ(BRD_USER_BUTTON_EXTI_IRQn);
#endif
	portENABLE_INTERRUPTS();	//Enable interrupts
}

*
  * @brief  Callback function for button presses
  * @param  GPIO_Pin: pin that caused trigger
  * @retval None

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {

	uint32_t currentTime = HAL_GetTick();
	BaseType_t xHigherPriorityTaskWoken;

	 Debounce button press
	if(currentTime - lastInterruptTime >= DEBOUNCE_THRESHOLD) {
		xHigherPriorityTaskWoken = pdFALSE;

		if(s4435360_SemaphoreJoystickZ != NULL) {
			xSemaphoreGiveFromISR(s4435360_SemaphoreJoystickZ,
					&xHigherPriorityTaskWoken);
		}
	}

	lastInterruptTime = currentTime;
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

}

#ifdef JOYSTICK
*
  * @brief  External interrupt handler for joystick button
  * @param  None
  * @retval None

void EXTI9_5_IRQHandler(void) {
	HAL_GPIO_EXTI_IRQHandler(BRD_D13_PIN);
}
#endif

#ifdef USER_BUTTON
*
  * @brief  External interrupt handler for board user button
  * @param  None
  * @retval None

void EXTI15_10_IRQHandler(void) {
	HAL_GPIO_EXTI_IRQHandler(BRD_USER_BUTTON_PIN);
}
#endif

int main(void) {

	//Initialise hardware
	BRD_init();
	Hardware_init();
	s4435360_hal_sysmon_init();
	s4435360_os_joystick_init();

	//Create tasks
	xTaskCreate((void *) &Task1_Task,
			(const char *) "TASK1", TASK1_STACK_SIZE,
			NULL, TASK1_PRIORITY, &task1Handle);
	xTaskCreate((void *) &Task2_Task,
			(const char *) "TASK2", TASK2_STACK_SIZE,
			NULL, TASK2_PRIORITY, &task2Handle);
	xTaskCreate((void *) &Task3_Task,
			(const char *) "TASK3", TASK3_STACK_SIZE,
			NULL, TASK3_PRIORITY, &task3Handle);

	//Start scheduler
	vTaskStartScheduler();

	return 0;
}
*/


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
#define TASK1_STACK_SIZE		( configMINIMAL_STACK_SIZE * 5 )
#define CLI_STACK_SIZE			( configMINIMAL_STACK_SIZE * 5 )
#define PRINTF_STACK_SIZE		( configMINIMAL_STACK_SIZE * 5 )
#define PANTILT_STACK_SIZE		( configMINIMAL_STACK_SIZE * 5 )

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


int main(void) {
	//Initialise hardware
	BRD_init();

	//Create tasks
	//xTaskCreate((void *) &Task1_Task,
	//		(const char *) "TASK1", TASK1_STACK_SIZE,
	//		NULL, TASK1_PRIORITY, NULL);
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
	myprintf("Pan command entered\r\n");
	const char *cCmd_string;

	/* Get parameters from command string */
	long paramLen;
	const char* parameter = FreeRTOS_CLIGetParameter(pcCommandString, 1, &paramLen);

	if(!strcmp((const char*) parameter, "left")) {
		xSemaphoreGive(s4435360_SemaphorePanLeft);
	} else if(!strcmp((const char*) parameter, "right")) {
		xSemaphoreGive(s4435360_SemaphorePanRight);
	} else {
		char* remainder;
		long value;
		value = strtol(parameter, &remainder, 10);

		if(!strlen(remainder)) {
			xQueueSend(s4435360_QueuePan, (void *) &value, 100);
		}
	}

	return pdFALSE;
}

static BaseType_t prvTiltCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {
	myprintf("Tilt command entered\r\n");
	const char *cCmd_string;

	/* Get parameters from command string */
	long paramLen;
	const char* parameter = FreeRTOS_CLIGetParameter(pcCommandString, 1, &paramLen);

	if(!strcmp((const char*) parameter, "up")) {
		xSemaphoreGive(s4435360_SemaphoreTiltUp);
	} else if(!strcmp((const char*) parameter, "down")) {
		xSemaphoreGive(s4435360_SemaphoreTiltDown);
	} else {
		char* remainder;
		long value;
		value = strtol(parameter, &remainder, 10);

		if(!strlen(remainder)) {
			xQueueSend(s4435360_QueueTilt, (void *) &value, 100);
		}
	}

	return pdFALSE;
}
