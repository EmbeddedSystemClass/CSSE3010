/**
 ******************************************************************************
 * @file    mylib/s4435360_hal_dac.c
 * @author  Samuel Eadie - 44353607
 * @date    20052018
 * @brief   DAC peripheral driver
 *
 ******************************************************************************
 *     EXTERNAL FUNCTIONS
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <s4435360_hal_dac.h>


#include "stm32f4xx_hal_dac.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define __TIM_DAC_CLK_ENABLE() 		__TIM2_CLK_ENABLE()
#define DAC_TIM						TIM2
#define DAC_FREQUENCY				2
#define DAC_TIM_IRQn				TIM2_IRQn
#define DAC_GPIO_AF					GPIO_AF1_TIM2
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
int dacSequenceIndex;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

extern void s4435360_hal_dac_init(void) {
	//Enable DAC clock
	__HAL_RCC_DAC_CLK_ENABLE();

	//TypeDef handle for DAC
	dacHandle.Instance = DAC;

	// Initialise DAC.
	HAL_DAC_Init(&dacHandle);

	// Set up the GPIO as analog output.
	GPIO_InitTypeDef  dacGPIO;

	//Settings general to both channels
	dacGPIO.Mode = GPIO_MODE_ANALOG;
	dacGPIO.Pull = GPIO_NOPULL;
	dacGPIO.Speed = GPIO_SPEED_FAST;

	//Config x channel
	DAC_X_GPIO_CLK();
	dacGPIO.Pin = DAC_X_DATA_GPIO_PIN;
	HAL_GPIO_Init(DAC_X_DATA_GPIO_PORT, &dacGPIO);

	//Config y channel
	DAC_Y_GPIO_CLK();
	dacGPIO.Pin = DAC_Y_DATA_GPIO_PIN;
	HAL_GPIO_Init(DAC_Y_DATA_GPIO_PORT, &dacGPIO);

	//Channel config
	dacChannelConfig.DAC_Trigger = DAC_TRIGGER_NONE;
	dacChannelConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;

	//Config channels
	HAL_DAC_ConfigChannel(&dacHandle, &dacChannelConfig, DAC_CHANNEL_X);
	HAL_DAC_ConfigChannel(&dacHandle, &dacChannelConfig, DAC_CHANNEL_Y);

	//Start both channels
	HAL_DAC_Start(&dacHandle, DAC_CHANNEL_X);
	HAL_DAC_Start(&dacHandle, DAC_CHANNEL_Y);


	//Initialise timer to run DAC sequence
	__TIM_DAC_CLK_ENABLE();

	/* TIM Base configuration */
	dacTimInit.Instance = DAC_TIM;
	dacTimInit.Init.Period = 50000 / (DAC_FREQUENCY);
	dacTimInit.Init.Prescaler = (uint16_t) (SystemCoreClock / 50000) - 1;	//Set prescaler value
	dacTimInit.Init.ClockDivision = 0;			//Set clock division
	dacTimInit.Init.RepetitionCounter = 0;	// Set reload Value
	dacTimInit.Init.CounterMode = TIM_COUNTERMODE_UP;	//Set timer to count up.

	/* Initialise Timer */
	HAL_TIM_Base_Init(&dacTimInit);
	HAL_NVIC_SetPriority(DAC_TIM_IRQn, 10, 0);
	HAL_NVIC_EnableIRQ(DAC_TIM_IRQn);

	dacSequenceIndex = 0;

}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	s4435360_hal_dac_y_write(dacYSequence[dacSequenceIndex % dacYSequenceLength]);
	s4435360_hal_dac_x_write(dacXSequence[dacSequenceIndex % dacXSequenceLength]);
	dacSequenceIndex++;
}

/**
 * @brief  DAC sequence timer handler
 * @param  None
 * @retval None
 */
void TIM2_IRQHandler(void) {
	HAL_TIM_IRQHandler(&dacTimInit);
}
