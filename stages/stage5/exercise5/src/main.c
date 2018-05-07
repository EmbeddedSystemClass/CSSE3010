/**
  ******************************************************************************
  * @file    exercise5/main.c
  * @author  SE
  * @date    02052018
  * @brief   Demonstrates freeRTOS functionality for exercise 5
  ******************************************************************************
  *
  */

#include "board.h"
#include "stm32f4xx_hal_conf.h"
#include "debug_printf.h"
#include <string.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define EVER ;;

/* Task stack size definitions */
#define TASK1_STACK_SIZE		( configMINIMAL_STACK_SIZE * 2 * 2 )
#define TASK2_STACK_SIZE		( configMINIMAL_STACK_SIZE * 2 * 2 )

/* Task priority definitions*/
#define TASK1_PRIORITY			( tskIDLE_PRIORITY + 2 )
#define TASK2_PRIORITY			( tskIDLE_PRIORITY + 2 )

/* Exercise definitions */
#define EXERCISE1_DELAY_TIME		(2 * 1000)
#define EXERCISE2_DELAY_TIME		(1 * 1000)
#define EXERCISE2_LETTER			'A'
#define EXERCISE3_DELAY_TIME		(2 * 1000)
#define EXERCISE3_LETTER			'B'
#define EXERCISE4_DELAY_TIME		(2 * 1000)
#define EXERCISE4_LIVE_TIME			(11 * 1000)
#define EXERCISE5_LETTER			'C'

#define DEBOUNCE_THRESHOLD		((uint32_t) 100)

#define EXERCISE_1
//#define EXERCISE_2
//#define EXERCISE_3
//#define EXERCISE_4
//#define EXERCISE_5

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
volatile uint32_t lastInterruptTime = 0;
TaskHandle_t task1Handle, task2Handle, task3Handle;
SemaphoreHandle_t exercise1Semaphore;
QueueHandle_t exercise2Queue, exercise3Queue, exercise5Queue;

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  Task 0 functionality
  * @param  None
  * @retval None
  */
void Task1_Task(void) {
	char receivedLetter;
#ifdef EXERCISE_1
	for(EVER) {
		vTaskDelay(EXERCISE1_DELAY_TIME);
		xSemaphoreGive(exercise1Semaphore);
	}
#endif

#ifdef EXERCISE_2
	char message = EXERCISE2_LETTER;
	for(EVER) {
		vTaskDelay(EXERCISE2_DELAY_TIME);
		xQueueSend(exercise2Queue, ( void * ) &message, ( portTickType ) 200 );
	}
#endif

#ifdef EXERCISE_3
	char letter = EXERCISE3_LETTER;
	xQueueSend(exercise3Queue, ( void * ) &letter, ( portTickType ) 20000 );
#endif

#ifdef EXERCISE_4
	for(EVER) {
		vTaskDelay(EXERCISE4_DELAY_TIME);
		portENTER_CRITICAL();
		debug_printf("Task One is Live\r\n");
		portEXIT_CRITICAL();
	}

#endif

#ifdef EXERCISE_5
	for(EVER) {
		if(xQueueReceive(exercise5Queue, &receivedLetter, (TickType_t)(1000))) {
			portENTER_CRITICAL();
			debug_printf("Received Letter: %c\r\n", receivedLetter);
			portEXIT_CRITICAL();
		}
	}
#endif

}

/**
  * @brief  Task 2 functionality
  * @param  None
  * @retval None
  */
void Task2_Task(void) {
	char receivedLetter;
#ifdef EXERCISE_1
	for(EVER) {
		if(xSemaphoreTake(exercise1Semaphore, (TickType_t)(10000 / portTICK_PERIOD_MS))) {
			BRD_LEDRedOn();
			vTaskDelay(400);
			BRD_LEDRedOff();
		}
	}
#endif

#ifdef EXERCISE_2
	for(EVER) {
		if(xQueueReceive(exercise2Queue, &receivedLetter, (TickType_t)(10000 / portTICK_PERIOD_MS))) {
			portENTER_CRITICAL();
			debug_printf("Received Letter: %c\r\n", receivedLetter);
			portEXIT_CRITICAL();
		}
	}
#endif

#ifdef EXERCISE_3
	for(EVER) {
		xQueuePeak(exercise3Queue, &receivedLetter, (TickType_t)(10000 / portTICK_PERIOD_MS));
		portENTER_CRITICAL();
		debug_printf("Received Letter: %c\r\n", receivedLetter);
		portEXIT_CRITICAL();
		vTaskDelay(EXERCISE3_DELAY_TIME);
	}
#endif

#ifdef EXERCISE_4
	vTaskDelay(EXERCISE4_LIVE_TIME);
	VTaskDelete(task1Handle);
	for(EVER);
#endif

#ifdef EXERCISE_5

#endif


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

	for( EVER );
}

void Hardware_init() {
	portDISABLE_INTERRUPTS();	//Disable interrupts

	BRD_LEDInit();				//Initialise Blue LED
	BRD_LEDRedOff();				//Turn off Red LED
	BRD_LEDGreenOff();				//Turn off Green LED
	BRD_LEDBlueOff();				//Turn off Blue LED

	//Set up board user button
	GPIO_InitTypeDef GPIO_InitStructure;

	BRD_USER_BUTTON_GPIO_CLK_ENABLE();

	GPIO_InitStructure.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Pin  = BRD_USER_BUTTON_PIN;
	HAL_GPIO_Init(BRD_USER_BUTTON_GPIO_PORT, &GPIO_InitStructure);

	HAL_NVIC_SetPriority(BRD_USER_BUTTON_EXTI_IRQn, 10, 0);
	HAL_NVIC_EnableIRQ(BRD_USER_BUTTON_EXTI_IRQn);

	portENABLE_INTERRUPTS();	//Enable interrupts
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {

#ifdef EXERCISE_5
	uint32_t currentTime = HAL_GetTick();
	BaseType_t xHigherPriorityTaskWoken;

	/* Debounce button press */
	if(currentTime - lastInterruptTime >= DEBOUNCE_THRESHOLD) {
		char message = EXERCISE5_LETTER;
		xQueueSendfromISR(exercise5Queue, ( void * ) &message, ( portTickType ) 200 );
	}

	lastInterruptTime = currentTime;
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
#endif

}

void EXTI15_10_IRQHandler(void) {
	HAL_GPIO_EXTI_IRQHandler(BRD_USER_BUTTON_PIN);
}

int main(void) {

	//Initialise hardware
	BRD_init();
	Hardware_init();

#ifdef EXERCISE_1
	exercise1Semaphore = xSemaphoreCreateBinary();
#endif

#ifdef EXERCISE_2
	exercise2Queue = xQueueCreate(10, sizeof(char));
#endif

#ifdef EXERCISE_3
	exercise3Queue = xQueueCreate(10, sizeof(char));
#endif

#ifdef EXERCISE_4

#endif

#ifdef EXERCISE_5

#endif
	//Create tasks
	xTaskCreate((void *) &Task1_Task,
			(const char *) "TASK1", TASK1_STACK_SIZE,
			NULL, TASK1_PRIORITY, &task1Handle);

#ifdef EXERCISE_5
	//Start scheduler
	vTaskStartScheduler();

	return 0;
#endif

	xTaskCreate((void *) &Task2_Task,
			(const char *) "TASK2", TASK2_STACK_SIZE,
			NULL, TASK2_PRIORITY, &task2Handle);

	//Start scheduler
	vTaskStartScheduler();

	return 0;
}



