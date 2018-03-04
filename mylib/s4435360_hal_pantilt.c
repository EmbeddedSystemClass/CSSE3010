/**
 ******************************************************************************
 * @file    mylib/s4435360_hal_pantilt.c
 * @author  Samuel Eadie - 44353607
 * @date    28022018-07032018
 * @brief   Pan/tilt servo peripheral driver
 *
 ******************************************************************************
 *     EXTERNAL FUNCTIONS
 ******************************************************************************
 *****************
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <s4435360_hal_pantilt.h>

#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_tim.h"
#include "stm32f4xx_hal_conf.h"
#include "debug_printf.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx.h"
#include "board.h"

#define  COUNTER_CLOCK      50000
#define  PRESCALAR 			(uint32_t)((SystemCoreClock) / COUNTER_CLOCK) - 1
#define  PERIOD 			20 /* Period of PWM (ms) */
#define  PERIOD_VALUE       COUNTER_CLOCK / (1000 / PERIOD)
#define  DUTY1_VALUE 		75     /* Duty cycle 1 (%) */
#define  DUTY2_VALUE 		50   /* Duty cycle 2 (%) */
#define  DUTY3_VALUE 		25     /* Duty cycle 3 (%) */
#define  DUTY4_VALUE 		10   /* Duty cycle 4 (%) */
#define  PULSE1_VALUE       (uint32_t)(PERIOD_VALUE * DUTY1_VALUE / 100)    /* Capture Compare 1 Value  */
#define  PULSE2_VALUE       (uint32_t)(PERIOD_VALUE * DUTY2_VALUE / 100) 	/* Capture Compare 2 Value  */
#define  PULSE3_VALUE       (uint32_t)(PERIOD_VALUE * DUTY3_VALUE / 100)    /* Capture Compare 3 Value  */
#define  PULSE4_VALUE       (uint32_t)(PERIOD_VALUE * DUTY4_VALUE / 100) 	/* Capture Compare 4 Value  */

/* Definition for TIMx clock resources */
#define TIMx       			    TIM1
#define TIMx_CLK_ENABLE()       __HAL_RCC_TIM1_CLK_ENABLE()

/* Definition for TIMx Channel Pins */
#define TIMx_CHANNEL_GPIO_PORT()       __HAL_RCC_GPIOE_CLK_ENABLE();
#define TIMx_GPIO_PORT_CHANNEL1        GPIOE
#define TIMx_GPIO_PORT_CHANNEL2        GPIOE
#define TIMx_GPIO_PORT_CHANNEL3        GPIOE
#define TIMx_GPIO_PORT_CHANNEL4        GPIOE
#define TIMx_GPIO_PIN_CHANNEL1         GPIO_PIN_9
#define TIMx_GPIO_PIN_CHANNEL2         GPIO_PIN_11
#define TIMx_GPIO_PIN_CHANNEL3         GPIO_PIN_13
#define TIMx_GPIO_PIN_CHANNEL4         GPIO_PIN_14
#define TIMx_GPIO_AF_CHANNEL1          GPIO_AF1_TIM1
#define TIMx_GPIO_AF_CHANNEL2          GPIO_AF1_TIM1
#define TIMx_GPIO_AF_CHANNEL3          GPIO_AF1_TIM1
#define TIMx_GPIO_AF_CHANNEL4          GPIO_AF1_TIM1

/*Define PAN and TILT types */
#define PAN 0
#define TILT 1

/* Private macro -------------------------------------------------------------*/
#define ANGLE_TO_DUTY_CYCLE(angle) (angle *5 / 90) + 7.25
#define ANGLE_TO_PULSE_PERIOD(angle) (angle / 90) + 1.45
#define DUTY_CYCLE_TO_ANGLE(dutyCycle) (18 * dutyCycle) - 130.5
#define PULSE_PERIOD_TO_ANGLE(pulsePeriod) (90 * pulsePeriod) - 130.5

/* External function prototypes -----------------------------------------------*/
#define s4435360_hal_pantilt_pan_write(angle) pantilt_angle_write(PAN, angle)
#define s4435360_hal_pantilt_pan_read() pantilt_angle_read(PAN)
#define s4435360_hal_pantilt_tile_write(angle) pantilt_angle_write(TILT, angle)
#define s4435360_hal_pantilt_tilt_read() pantilt_angle_read(TILT)
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint32_t prescalerValue = 0;
TIM_OC_InitTypeDef sConfig;
TIM_HandleTypeDef TimInit;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
void PWM_init(void) {

	GPIO_InitTypeDef GPIO_InitStruct;

	BRD_LEDInit();	//Initialise LEDs

	/* Turn off LEDs */
	BRD_LEDRedOff();
	BRD_LEDGreenOn();
	BRD_LEDBlueOff();

	TIMx_CLK_ENABLE();

	TIMx_CHANNEL_GPIO_PORT();

	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

	GPIO_InitStruct.Alternate = TIMx_GPIO_AF_CHANNEL1;
	GPIO_InitStruct.Pin = TIMx_GPIO_PIN_CHANNEL1;
	HAL_GPIO_Init(TIMx_GPIO_PORT_CHANNEL1, &GPIO_InitStruct);

	GPIO_InitStruct.Alternate = TIMx_GPIO_AF_CHANNEL2;
	GPIO_InitStruct.Pin = TIMx_GPIO_PIN_CHANNEL2;
	HAL_GPIO_Init(TIMx_GPIO_PORT_CHANNEL2, &GPIO_InitStruct);

	GPIO_InitStruct.Alternate = TIMx_GPIO_AF_CHANNEL3;
	GPIO_InitStruct.Pin = TIMx_GPIO_PIN_CHANNEL3;
	HAL_GPIO_Init(TIMx_GPIO_PORT_CHANNEL3, &GPIO_InitStruct);

	GPIO_InitStruct.Alternate = TIMx_GPIO_AF_CHANNEL4;
	GPIO_InitStruct.Pin = TIMx_GPIO_PIN_CHANNEL4;
	HAL_GPIO_Init(TIMx_GPIO_PORT_CHANNEL4, &GPIO_InitStruct);

	prescalerValue = PRESCALAR;
	TimInit.Instance = TIMx;

	TimInit.Init.Prescaler         = prescalerValue;
	TimInit.Init.Period            = PERIOD_VALUE;
	TimInit.Init.ClockDivision     = 0;
	TimInit.Init.CounterMode       = TIM_COUNTERMODE_UP;
	TimInit.Init.RepetitionCounter = 0;
	if (HAL_TIM_PWM_Init(&TimInit) != HAL_OK) {
		/* Initialization Error */
	}


	/*##-2- Configure the PWM channels #########################################*/
	/* Common configuration for all channels */
	sConfig.OCMode       = TIM_OCMODE_PWM1;
	sConfig.OCPolarity   = TIM_OCPOLARITY_HIGH;
	sConfig.OCFastMode   = TIM_OCFAST_DISABLE;
	sConfig.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
	sConfig.OCNIdleState = TIM_OCNIDLESTATE_RESET;

	sConfig.OCIdleState  = TIM_OCIDLESTATE_RESET;

	/* Set the pulse value for channel 1 */
	sConfig.Pulse = PULSE1_VALUE;
	if (HAL_TIM_PWM_ConfigChannel(&TimInit, &sConfig, TIM_CHANNEL_1) != HAL_OK) {
		/* Configuration Error */
	}

	/* Set the pulse value for channel 2 */
	sConfig.Pulse = PULSE2_VALUE;
	if (HAL_TIM_PWM_ConfigChannel(&TimInit, &sConfig, TIM_CHANNEL_2) != HAL_OK) {
		/* Configuration Error */
	}

	/* Set the pulse value for channel 3 */
	sConfig.Pulse = PULSE3_VALUE;
	if (HAL_TIM_PWM_ConfigChannel(&TimInit, &sConfig, TIM_CHANNEL_3) != HAL_OK) {
		/* Configuration Error */
	}

	/* Set the pulse value for channel 4 */
	sConfig.Pulse = PULSE4_VALUE;
	if (HAL_TIM_PWM_ConfigChannel(&TimInit, &sConfig, TIM_CHANNEL_4) != HAL_OK) {
		/* Configuration Error */
	}

	/* Start PWM signals generation */
	if (HAL_TIM_PWM_Start(&TimInit, TIM_CHANNEL_1) != HAL_OK) {
		/* PWM Generation Error */
	}
	if (HAL_TIM_PWM_Start(&TimInit, TIM_CHANNEL_2) != HAL_OK) {
		/* PWM Generation Error */
	}
	if (HAL_TIM_PWM_Start(&TimInit, TIM_CHANNEL_3) != HAL_OK) {
		/* PWM generation Error */
	}
	if (HAL_TIM_PWM_Start(&TimInit, TIM_CHANNEL_4) != HAL_OK) {
		/* PWM generation Error */
	}
}



/**
 * @brief	Writes the specified segment_value
 * 			to the specified segment
 * @param	segment, segment_value
 * @retval 	None
 */
void s4435360_hal_pantilt_init(void) {

}

void pantilt_angle_write(int type, int angle) {

}

void pantilt_angle_read(int type) {

}
