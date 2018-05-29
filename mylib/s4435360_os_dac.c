/**
 ******************************************************************************
 * @file    mylib/s4435360_os_dac.c
 * @author  Samuel Eadie - 44353607
 * @date    22052018
 * @brief   Provides OS functionality for DAC
 *
 ******************************************************************************
 *     EXTERNAL FUNCTIONS
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <s4435360_os_dac.h>

#include "s4435360_hal_dac.h"
#include "stm32f4xx_hal_dac.h"

#include "stm32f4xx_hal_conf.h"
#include "board.h"
#include "stm32f4xx_hal_dac.h"
#include "s4435360_hal_dac.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define EVER		;;
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
int xSequence[100];
int ySequence[100];
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/


/**
  * @brief Task for handling DAC
  * @param None
  * @retval None
  */
void s4435360_DACTask(void) {

	s4435360_hal_dac_init();

	int xSequence[16] = {0, 400, 800, 1200, 1600, 1600, 1600, 1600, 1600, 1200, 800, 400, 0, 0, 0, 0};
	int ySequence[16] = {0, 0, 0, 0, 0, 400, 800, 1200, 1600, 1600, 1600, 1600, 1600, 1200, 800, 400};
	dacXSequence = xSequence;
	dacYSequence = ySequence;
	dacXSequenceLength = 16; //1;
	dacYSequenceLength = 16; //1;

	s4435360_SemaphoreDACoff = xSemaphoreCreateBinary();
	s4435360_SemaphoreDACon = xSemaphoreCreateBinary();
	s4435360_QueueSequence = xQueueCreate(5, sizeof(DACSequence));

	DACSequence sequence;
	s4435360_hal_dac_sequence_on();

	for(EVER) {
		if(xSemaphoreTake(s4435360_SemaphoreDACoff, 0)) {
			s4435360_hal_dac_sequence_off();
		}

		if(xSemaphoreTake(s4435360_SemaphoreDACon, 0)) {
			s4435360_hal_dac_sequence_on();
		}

		if(xQueueReceive(s4435360_QueueSequence, &sequence, 0)) {
			updateDACSequence(sequence);
		}

		vTaskDelay(500);
	}
}
