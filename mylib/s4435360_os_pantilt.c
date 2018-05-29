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
#include <math.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "board.h"
#include "debug_printf.h"
#include "s4435360_hal_pantilt.h"
#include "s4435360_os_printf.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define EVER						;;
#define PANTILT_QUEUE_LENGTH		10

#define PI			3.14159265
#define RAD_TO_DEG(angle) 			((180.0 * angle) / PI)
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
void s4435360_pantilt_changeX(int x) {
	float x2 = 1.3 * x;
	float x3 = x2 - 130.0;
	float x4 = x3 / 300.0;
	float angleRadians = atan(x4);
	int panAngle = -1 * RAD_TO_DEG(angleRadians);
	myprintf("%d, %d\r\n", x, panAngle);

	if(s4435360_QueuePan != NULL) {
		xQueueSendToBack(s4435360_QueuePan, ( void * ) &panAngle, portMAX_DELAY);
	}
}

void s4435360_pantilt_changeY(int y) {
	int tiltAngle = 55 - RAD_TO_DEG(atan((float)y / 300.0));
	myprintf("Tilt angle = %d, from %d\r\n", tiltAngle, y);
	if(s4435360_QueueTilt != NULL) {
		xQueueSendToBack(s4435360_QueueTilt, ( void * ) &tiltAngle, portMAX_DELAY);
	}
}

void s4435360_TaskPanTilt(void) {

	//Initialise hardware
	s4435360_hal_pantilt_init();

	//Initialise pan and tilt queues
	s4435360_QueuePan = xQueueCreate(PANTILT_QUEUE_LENGTH, sizeof(int));
	s4435360_QueueTilt = xQueueCreate(PANTILT_QUEUE_LENGTH, sizeof(int));

	//initialise pan and tilt semaphores
	s4435360_SemaphoreUpdatePantilt = xSemaphoreCreateBinary();
	s4435360_SemaphorePanLeft = xSemaphoreCreateBinary();
	s4435360_SemaphorePanRight = xSemaphoreCreateBinary();
	s4435360_SemaphoreTiltUp = xSemaphoreCreateBinary();
	s4435360_SemaphoreTiltDown = xSemaphoreCreateBinary();

	int controlCommand;

	for(EVER) {
		//myprintf("Pantilt task here\r\n");
		if(xSemaphoreTake(s4435360_SemaphoreUpdatePantilt, 0)) {
			if(xQueueReceive(s4435360_QueuePan, &controlCommand, 0)) {
				s4435360_hal_pantilt_pan_write(controlCommand);
				myprintf("Pan write of %d\r\n", controlCommand);
			}

			vTaskDelay(1000);

			if (xQueueReceive(s4435360_QueueTilt, &controlCommand, 0)) {
				s4435360_hal_pantilt_tilt_write(controlCommand);
				myprintf("Tilt write of %d\r\n", controlCommand);
			}
		}

		if (xSemaphoreTake(s4435360_SemaphorePanLeft, 0)) {
			s4435360_hal_pantilt_pan_write(s4435360_hal_pantilt_pan_read() - 5);
			myprintf("Pan Left\r\n");
		}

		if (xSemaphoreTake(s4435360_SemaphorePanRight, 0)) {
			s4435360_hal_pantilt_pan_write(s4435360_hal_pantilt_pan_read() + 5);
			myprintf("Pan Right\r\n");
		}

		if (xSemaphoreTake(s4435360_SemaphoreTiltUp, 0)) {
			s4435360_hal_pantilt_tilt_write(s4435360_hal_pantilt_tilt_read() + 5);
			myprintf("Tilt up\r\n");
		}

		if (xSemaphoreTake(s4435360_SemaphoreTiltDown, 0)) {
			s4435360_hal_pantilt_tilt_write(s4435360_hal_pantilt_tilt_read() - 5);
			myprintf("Tilt down\r\n");
		}

		vTaskDelay(500);

	}
}
