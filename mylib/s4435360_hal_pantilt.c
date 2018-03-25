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

#include "stm32f4xx_hal_conf.h"
#include "debug_printf.h"
#include "board.h"

/* Clock scaling and timing values */
#define  COUNTER_CLOCK      500000
#define  PRESCALAR 			(uint32_t)((SystemCoreClock) / COUNTER_CLOCK) - 1
#define  PERIOD 			20 /* Period of PWM (ms) */
#define  PERIOD_VALUE       (COUNTER_CLOCK / (1000 / PERIOD))

/* Macros for converting between angle and pulse period/register value */
#define  ANGLE_TO_PULSE_PERIOD(angle) ((0.0106 * (float)angle) + 1.4956)
//#define  ANGLE_TO_PULSE_PERIOD(angle) (((float)angle / 90.0) + 1.45)
#define  PULSE_PERIOD_TO_ANGLE(pulsePeriod) ((94.515 * (float)pulsePeriod) - 141.35)
//#define  PULSE_PERIOD_TO_ANGLE(pulsePeriod) ((90.0 * (float)pulsePeriod) - 130.5)
#define  PULSE_PERIOD_TO_REGISTER(period) ((period / (float)PERIOD) * (float)PERIOD_VALUE)
#define  PULSE_REGISTER_TO_PERIOD(value) ((value / (float)PERIOD_VALUE) * (float)PERIOD)
#define  ANGLE_TO_PERIOD_REGISTER(angle) PULSE_PERIOD_TO_REGISTER(ANGLE_TO_PULSE_PERIOD(angle))
#define  PERIOD_REGISTER_TO_ANGLE(value) PULSE_PERIOD_TO_ANGLE(PULSE_REGISTER_TO_PERIOD(value))

/* Definition for TIMx clock resources */
#define TIMx       			    TIM4
#define TIMx_CLK_ENABLE()       __HAL_RCC_TIM4_CLK_ENABLE()

/* Definition for TIMx Channel Pins */
#define TIMx_CHANNEL_GPIO_PORT()       __HAL_RCC_GPIOD_CLK_ENABLE();
#define TIMx_GPIO_PORT_CHANNEL1        GPIOD
#define TIMx_GPIO_PORT_CHANNEL2        GPIOD
#define TIMx_GPIO_PIN_CHANNEL1         GPIO_PIN_13
#define TIMx_GPIO_PIN_CHANNEL2         GPIO_PIN_12
#define TIMx_GPIO_AF_CHANNEL1          GPIO_AF2_TIM4
#define TIMx_GPIO_AF_CHANNEL2          GPIO_AF2_TIM4


TIM_HandleTypeDef servoInit;

/* Macros for getting and setting PWM register compare values */
#define PWM_TIMER_HANDLER    		servoInit
#define PWM_CHANNEL1_GET() 			__HAL_TIM_GET_COMPARE(&PWM_TIMER_HANDLER, TIM_CHANNEL_1)
#define PWM_CHANNEL1_SET(value) 	__HAL_TIM_SET_COMPARE(&PWM_TIMER_HANDLER, TIM_CHANNEL_1, value)
#define PWM_CHANNEL2_GET() 			__HAL_TIM_GET_COMPARE(&PWM_TIMER_HANDLER, TIM_CHANNEL_2)
#define PWM_CHANNEL2_SET(value) 	__HAL_TIM_SET_COMPARE(&PWM_TIMER_HANDLER, TIM_CHANNEL_2, value)

/*Wrappers for defining tilt/pan set and get */
#define PWM_CHANNEL_PAN_SET(value)  PWM_CHANNEL1_SET(value)
#define PWM_CHANNEL_PAN_GET() 		PWM_CHANNEL1_GET()
#define PWM_CHANNEL_TILT_SET(value) PWM_CHANNEL2_SET(value)
#define PWM_CHANNEL_TILT_GET() 		PWM_CHANNEL2_GET()

/* Macros for pan/tilt specific reading and writing */
#define s4435360_hal_pantilt_pan_write(angle) pantilt_angle_write(PAN, angle)
#define s4435360_hal_pantilt_pan_read() pantilt_angle_read(PAN)
#define s4435360_hal_pantilt_tile_write(angle) pantilt_angle_write(TILT, angle)
#define s4435360_hal_pantilt_tilt_read() pantilt_angle_read(TILT)

uint32_t prescalerValue = 0; //Prescalar value for PWM timer
TIM_OC_InitTypeDef sConfig; //PWM config



/**
 * @brief  Initialises 4 PWM channels
 * @param  Initial pulse values for the PWM channels
 * @retval None
 */
void PWM_init(int pulse1, int pulse2) {

	GPIO_InitTypeDef GPIO_InitStruct;

	TIMx_CLK_ENABLE();

	TIMx_CHANNEL_GPIO_PORT();

	/* Channel generic fields */
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

	/* Channel specific fields */
	GPIO_InitStruct.Alternate = TIMx_GPIO_AF_CHANNEL1;
	GPIO_InitStruct.Pin = TIMx_GPIO_PIN_CHANNEL1;
	HAL_GPIO_Init(TIMx_GPIO_PORT_CHANNEL1, &GPIO_InitStruct);

	GPIO_InitStruct.Alternate = TIMx_GPIO_AF_CHANNEL2;
	GPIO_InitStruct.Pin = TIMx_GPIO_PIN_CHANNEL2;
	HAL_GPIO_Init(TIMx_GPIO_PORT_CHANNEL2, &GPIO_InitStruct);

	/* Set PWM timer fields */
	prescalerValue = PRESCALAR;
	servoInit.Instance = TIMx;

	servoInit.Init.Prescaler         = prescalerValue;
	servoInit.Init.Period            = PERIOD_VALUE;
	servoInit.Init.ClockDivision     = 0;
	servoInit.Init.CounterMode       = TIM_COUNTERMODE_UP;
	servoInit.Init.RepetitionCounter = 0;
	if (HAL_TIM_PWM_Init(&servoInit) != HAL_OK) {
		/* Initialization Error */
	}


	/* Common configuration for all channels */
    sConfig.OCMode       = TIM_OCMODE_PWM1;
    sConfig.OCPolarity   = TIM_OCPOLARITY_HIGH;
    sConfig.OCFastMode   = TIM_OCFAST_DISABLE;
    sConfig.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
    sConfig.OCNIdleState = TIM_OCNIDLESTATE_RESET;

    sConfig.OCIdleState  = TIM_OCIDLESTATE_RESET;

	/* Set the pulse value for channel 1 */
    sConfig.Pulse = pulse1;
    if (HAL_TIM_PWM_ConfigChannel(&servoInit, &sConfig, TIM_CHANNEL_1) != HAL_OK) {
		/* Configuration Error */
	}

	/* Set the pulse value for channel 2 */
    sConfig.Pulse = pulse2;
    if (HAL_TIM_PWM_ConfigChannel(&servoInit, &sConfig, TIM_CHANNEL_2) != HAL_OK) {

		/* Configuration Error */
	}

	/* Start PWM signals generation */
	if (HAL_TIM_PWM_Start(&servoInit, TIM_CHANNEL_1) != HAL_OK) {
		/* PWM Generation Error */
	}
	if (HAL_TIM_PWM_Start(&servoInit, TIM_CHANNEL_2) != HAL_OK) {
		/* PWM Generation Error */
	}
}

/**
 * @brief  Initialises pan and tilt servo PWM hardware
 * @param  None
 * @retval None
 */
void s4435360_hal_pantilt_init(void) {
	/* Initialises PWM to angle of 0 */
	PWM_init(ANGLE_TO_PERIOD_REGISTER(0),
			ANGLE_TO_PERIOD_REGISTER(0));
}

/**
 * @brief  Writes an angle to either the pan or tilt servo
 * @param  type (PAN, TILT), angle
 * @retval None
 */
void pantilt_angle_write(int type, int angle) {

	//Ensure angle is between -85 and 85
	if(angle > 85) {
		angle = 85;
	} else if(angle < -85) {
		angle = -85;
	}

	/* Switch PAN and TILT cases */
	switch(type) {
		case PAN:
			PWM_CHANNEL_PAN_SET(ANGLE_TO_PERIOD_REGISTER(angle));
			break;

		case TILT:

			PWM_CHANNEL_TILT_SET(ANGLE_TO_PERIOD_REGISTER(angle));
			break;

		default:
			break;

	}
}

/**
 * @brief  Reads the current angle of either the pan or tilt servo
 * @param  type (PAN, TILT)
 * @retval angle
 */
int pantilt_angle_read(int type) {

	/* Switch on PAN, TILT cases */
	switch(type) {
		case PAN:
			return PERIOD_REGISTER_TO_ANGLE(PWM_CHANNEL_PAN_GET());
		case TILT:
			return PERIOD_REGISTER_TO_ANGLE(PWM_CHANNEL_TILT_GET());
		default:
			return 0;
	}
}
