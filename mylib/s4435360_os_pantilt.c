/**
 ******************************************************************************
 * @file    mylib/s4435360_os_pantilt.c
 * @author  Samuel Eadie - 44353607
 * @date    07052018
 * @brief   Pantilt OS functionality
 *
 ******************************************************************************
 *     EXTERNAL FUNCTIONS
 ******************************************************************************
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <s4435360_os_pantilt.h>

#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "board.h"
#include "debug_printf.h"
#include "s4435360_hal_pantilt.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define EVER						;;
#define PANTILT_QUEUE_LENGTH		10
#define PANTILT_STACK_SIZE			(configMINIMAL_STACK_SIZE * 2)
#define PANTILT_TASK_PRIORITY		(tskIDLE_PRIORITY + 2)
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

void s4435360_TaskPanTilt(void) {

	//Initialise hardware
	s4435360_hal_pantilt_init();

	//Initialise pan and tilt queues
	s4435360_QueuePan = xQueueCreate(PANTILT_QUEUE_LENGTH, sizeof(int));
	s4435360_QueueTilt = xQueueCreate(PANTILT_QUEUE_LENGTH, sizeof(int));

	//initialise pan and tilt semaphores
	s4435360_SemaphorePanLeft = xSemaphoreCreateBinary();
	s4435360_SemaphorePanRight = xSemaphoreCreateBinary();
	s4435360_SemaphoreTiltUp = xSemaphoreCreateBinary();
	s4435360_SemaphoreTiltDown = xSemaphoreCreateBinary();

	int controlCommand;

	for(EVER) {
		if(xQueueReceive(s4435360_QueuePan, &controlCommand, 0)) {
			s4435360_hal_pantilt_pan_write(controlCommand);
		} else if (xQueueReceive(s4435360_QueueTilt, &controlCommand, 0)) {
			s4435360_hal_pantilt_tilt_write(controlCommand);
		} else if (xSemaphoreTake(s4435360_SemaphorePanLeft, 0)) {
			s4435360_hal_pantilt_pan_write(s4435360_hal_pantilt_pan_read() - 5);
		} else if (xSemaphoreTake(s4435360_SemaphorePanRight, 0)) {
			s4435360_hal_pantilt_pan_write(s4435360_hal_pantilt_pan_read() + 5);
		} else if (xSemaphoreTake(s4435360_SemaphoreTiltUp, 0)) {
			s4435360_hal_pantilt_tilt_write(s4435360_hal_pantilt_tilt_read() + 5);
		} else if (xSemaphoreTake(s4435360_SemaphoreTiltDown, 0)) {
			s4435360_hal_pantilt_tilt_write(s4435360_hal_pantilt_tilt_read() - 5);
		}

		vTaskDelay(100);

	}
}
