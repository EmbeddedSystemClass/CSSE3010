/**
  ******************************************************************************
  * @file    demo5/main.c
  * @author  SE
  * @date    27042018
  * @brief   Demonstrates basic OS functionality for demo 5
  ******************************************************************************
  *
  */

#include <stdio.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "board.h"
#include "debug_printf.h"

#include "s4435360_hal_sysmon.h"
#include "s4435360_os_joystick.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define EVER ;;

/* Task stack size definitions */
#define TASK1_STACK_SIZE		( configMINIMAL_STACK_SIZE * 2 ) //Min 17
#define TASK2_STACK_SIZE		( configMINIMAL_STACK_SIZE * 2 )
#define TASK3_STACK_SIZE		( configMINIMAL_STACK_SIZE * 2 )

/* Task priority definitions*/
#define TASK1_PRIORITY			( tskIDLE_PRIORITY + 2 )
#define TASK2_PRIORITY			( tskIDLE_PRIORITY + 2 )
#define TASK3_PRIORITY			( tskIDLE_PRIORITY + 2 )

/* Delay time definitions */
#define MAJOR_DELAY_TIME		3
#define MINOR_DELAY_TIME		1

#define DEBOUNCE_THRESHOLD		((uint32_t) 100)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
volatile uint32_t lastInterruptTime = 0;
TaskHandle_t task1Handle, task2Handle, task3Handle;
int task2Deleted = 0;

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  Task 0 functionality
  * @param  None
  * @retval None
  */
void Task1_Task(void) {
	s4435360_hal_sysmon_chan0_clr();

	for(EVER) {
		s4435360_hal_sysmon_chan0_set();

		//Perform random task
		BRD_LEDGreenToggle();
		vTaskDelay(MAJOR_DELAY_TIME);

		s4435360_hal_sysmon_chan0_clr();
		vTaskDelay(MINOR_DELAY_TIME);
	}
}

/**
  * @brief  Task 2 functionality
  * @param  None
  * @retval None
  */
void Task2_Task(void) {
	s4435360_hal_sysmon_chan1_clr();

	for(EVER) {
		s4435360_hal_sysmon_chan1_set();

		//Perform random task
		BRD_LEDBlueToggle();
		vTaskDelay(MAJOR_DELAY_TIME);

		s4435360_hal_sysmon_chan1_clr();
		vTaskDelay(MINOR_DELAY_TIME);
	}
}

/**
  * @brief  Task 3 functionality
  * @param  None
  * @retval None
  */
void Task3_Task(void) {
	s4435360_hal_sysmon_chan2_clr();

	for(EVER) {
		s4435360_hal_sysmon_chan2_set();

		//Perform random task
		BRD_LEDRedToggle();
		vTaskDelay(MAJOR_DELAY_TIME);

		s4435360_hal_sysmon_chan2_clr();
		vTaskDelay(MINOR_DELAY_TIME);

		if(s4435360_SemaphoreJoystickZ != NULL) {

			//Can successfully give if interrupt has taken
			if(xSemaphoreGive(s4435360_SemaphoreJoystickZ) == pdTRUE) {

				//Handle Task 2 change
				if(task2Deleted) { //task2Handle == NULL) {
					xTaskCreate((void *) &Task2_Task, (const char *) "TASK2",
											TASK2_STACK_SIZE, NULL, TASK2_PRIORITY, &task2Handle);
					task2Deleted = 0;
				} else {
					vTaskDelete(task2Handle);
					s4435360_hal_sysmon_chan1_clr();
					task2Deleted = 1;
				}
			}
		}
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

	uint32_t currentTime = HAL_GetTick();
	BaseType_t xHigherPriorityTaskWoken;

	/* Debounce button press */
	if(currentTime - lastInterruptTime >= DEBOUNCE_THRESHOLD) {
		xHigherPriorityTaskWoken = pdFALSE;

		if(s4435360_SemaphoreJoystickZ != NULL) {
			xSemaphoreTakeFromISR(s4435360_SemaphoreJoystickZ,
					&xHigherPriorityTaskWoken);
		}
	}

	lastInterruptTime = currentTime;
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

}

void EXTI15_10_IRQHandler(void) {
	HAL_GPIO_EXTI_IRQHandler(BRD_USER_BUTTON_PIN);
}

int main(void) {

	//Initialise hardware
	BRD_init();
	Hardware_init();
	s4435360_hal_sysmon_init();
	s4435360_os_joystick_init();

	//Create tasks
	xTaskCreate((void *) &Task1_Task,
			(const char *) "TASK1", TASK1_STACK_SIZE,
			NULL, TASK1_PRIORITY, task1Handle);
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



