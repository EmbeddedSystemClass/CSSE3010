/**
 ******************************************************************************
 * @file    mylib/s4435360_hal_ir.c
 * @author  Samuel Eadie - 44353607
 * @date    07032018-14032018
 * @brief   Infrared communication peripherals: transmit and receive
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <s4435360_hal_ir.h>

#include "debug_printf.h"
#include "stm32f4xx_hal_conf.h"
#include "board.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef carrierTimInit;
TIM_OC_InitTypeDef carrierConfig;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
 * @brief Initialises infrared communication hardware
 * @param None
 * @retval None
 */
void s4435360_hal_ir_init(void) {

	GPIO_InitTypeDef GPIO_InitStruct;
	TIM_OC_InitTypeDef PWMConf;

	CARRIER_TIM_CLK();
	CARRIER_PORT_CLK();

	/* Initialise carrier wave output pin */
	GPIO_InitStruct.Pin = CARRIER_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
	GPIO_InitStruct.Alternate = CARRIER_GPIO_AF;
	HAL_GPIO_Init(CARRIER_PORT, &GPIO_InitStruct);

	/* Initialise carrier wave timer */
	carrierTimInit.Instance = CARRIER_TIMER;
	carrierTimInit.Init.Period = 1;
	carrierTimInit.Init.Prescaler = ((SystemCoreClock / 2) / (CARRIER_TIMER_FREQUENCY)) - 1;
	carrierTimInit.Init.ClockDivision = 0;
	carrierTimInit.Init.RepetitionCounter = 0;
	carrierTimInit.Init.CounterMode = TIM_COUNTERMODE_UP;

	/* Initialise PWM for carrier wave */
	PWMConf.OCMode = TIM_OCMODE_PWM1;
	PWMConf.Pulse = 1;
	PWMConf.OCPolarity = TIM_OCPOLARITY_HIGH;
	PWMConf.OCNPolarity = TIM_OCNPOLARITY_HIGH;
	PWMConf.OCFastMode = TIM_OCFAST_DISABLE;
	PWMConf.OCIdleState = TIM_OCIDLESTATE_RESET;
	PWMConf.OCNIdleState = TIM_OCNIDLESTATE_RESET;

	HAL_TIM_PWM_Init(&carrierTimInit);
	HAL_TIM_PWM_ConfigChannel(&carrierTimInit, &PWMConf, CARRIER_CHANNEL);

	/* Start carrier wave */
	HAL_TIM_PWM_Start(&carrierTimInit, CARRIER_CHANNEL);


	/* Initialise modulation signal output pin */
	GPIO_InitTypeDef GPIO_Init;

	__MODULATION_CLK_ENABLE();

	GPIO_Init.Pin = MODULATION_PIN;
	GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_Init.Pull = GPIO_PULLUP;
	GPIO_Init.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(MODULATION_PORT, &GPIO_Init);
}

/**
 * @brief Changes the state of the carrier wave
 * @param state: The new state of the carrier wave (CARRIER_ON, CARRIER_OFF)
 * @retval None
 */
void irhal_carrier(int state) {

	/* Switch on state */
	switch(state) {

		case CARRIER_ON:

			/* Enable carrier wave */
			__HAL_TIM_ENABLE(&CARRIER_TIMER_HANDLER);
			break;

		case CARRIER_OFF:

			/*Disable carrier wave */
			__HAL_TIM_DISABLE(&CARRIER_TIMER_HANDLER);
			break;

		default:
			break;
	}
}
