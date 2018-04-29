/**
 ******************************************************************************
 * @file    mylib/s4435360_hal_sysmon.c
 * @author  Samuel Eadie - 44353607
 * @date    27041998
 * @brief   Provides system monitoring capabilities
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <s4435360_hal_sysmon.h>

#include "debug_printf.h"
#include "stm32f4xx_hal_conf.h"
#include "board.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
 * @brief Initialises the GPIO pins for system monitoring
 * @param None
 * @retval None
 */
extern void s4435360_hal_sysmon_init(void) {

	GPIO_InitTypeDef GPIO_Init;

	//Init channel 0
	CHANNEL_0_PORT_CLK();
	GPIO_Init.Pin = CHANNEL_0_PIN;
	GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_Init.Pull = GPIO_PULLUP;
	GPIO_Init.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(CHANNEL_0_PORT, &GPIO_Init);

	//Init channel 1
	CHANNEL_1_PORT_CLK();
	GPIO_Init.Pin = CHANNEL_1_PIN;
	GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_Init.Pull = GPIO_PULLUP;
	GPIO_Init.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(CHANNEL_1_PORT, &GPIO_Init);

	//Init channel 2
	CHANNEL_2_PORT_CLK();
	GPIO_Init.Pin = CHANNEL_2_PIN;
	GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_Init.Pull = GPIO_PULLUP;
	GPIO_Init.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(CHANNEL_2_PORT, &GPIO_Init);

}
