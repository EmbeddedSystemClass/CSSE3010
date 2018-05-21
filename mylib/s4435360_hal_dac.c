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

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
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
}
