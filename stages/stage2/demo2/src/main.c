/**
 ******************************************************************************
 * @file    demo1/main.c
 * @author  SE
 * @date    21022018-28022018
 * @brief   Demonstrate light bar functionality
 ******************************************************************************
 *
 */
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_tim.h"
#include "stm32f4xx_hal_conf.h"
#include "debug_printf.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx.h"
#include "board.h"

/* Clock scaling and timing values */
#define  COUNTER_CLOCK      50000
#define  PRESCALAR 			(uint32_t)((SystemCoreClock) / COUNTER_CLOCK) - 1
#define  PERIOD 			20 /* Period of PWM (ms) */
#define  PERIOD_VALUE       COUNTER_CLOCK / (1000 / PERIOD)

float duty1Value, duty2Value, duty3Value, duty4Value;

/* PWM pulse values */
#define  PULSE1_VALUE       (uint32_t)(PERIOD_VALUE * duty1Value / 100)     /* Capture Compare 1 Value  */
#define  PULSE2_VALUE       (uint32_t)(PERIOD_VALUE * duty2Value / 100) 	/* Capture Compare 2 Value  */
#define  PULSE3_VALUE       (uint32_t)(PERIOD_VALUE * duty3Value / 100)     /* Capture Compare 3 Value  */
#define  PULSE4_VALUE       (uint32_t)(PERIOD_VALUE * duty4Value / 100) 	/* Capture Compare 4 Value  */

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

TIM_HandleTypeDef TimInit;

/* Macros for getting and setting PWM register compare values */
#define PWM_TIMER_HANDLER	TimInit
#define PWM_CHANNEL1_GET() 		__HAL_TIM_GET_COMPARE(&PWM_TIMER_HANDLER, TIM_CHANNEL_1)
#define PWM_CHANNEL1_SET(value) 	__HAL_TIM_SET_COMPARE(&PWM_TIMER_HANDLER, TIM_CHANNEL_1, value)
#define PWM_CHANNEL2_GET() 		__HAL_TIM_GET_COMPARE(&PWM_TIMER_HANDLER, TIM_CHANNEL_2)
#define PWM_CHANNEL2_SET(value) 	__HAL_TIM_SET_COMPARE(&PWM_TIMER_HANDLER, TIM_CHANNEL_2, value)
#define PWM_CHANNEL3_GET() 		__HAL_TIM_GET_COMPARE(&PWM_TIMER_HANDLER, TIM_CHANNEL_3)
#define PWM_CHANNEL3_SET(value) 	__HAL_TIM_SET_COMPARE(&PWM_TIMER_HANDLER, TIM_CHANNEL_3, value)
#define PWM_CHANNEL4_GET() 		__HAL_TIM_GET_COMPARE(&PWM_TIMER_HANDLER, TIM_CHANNEL_4)
#define PWM_CHANNEL4_SET(value) 	__HAL_TIM_SET_COMPARE(&PWM_TIMER_HANDLER, TIM_CHANNEL_4, value)

/* Macros for setting duty cycle of each channel */
#define PWM_DUTY1_SET(duty)({\
            duty1Value = duty;\
            PWM_CHANNEL1_SET(PULSE1_VALUE);\
           })
#define PWM_DUTY2_SET(duty)({\
            duty2Value = duty;\
            PWM_CHANNEL2_SET(PULSE2_VALUE);\
           })
#define PWM_DUTY3_SET(duty)({\
            duty3Value = duty;\
            PWM_CHANNEL3_SET(PULSE3_VALUE);\
           })
#define PWM_DUTY4_SET(duty)({\
            duty4Value = duty;\
            PWM_CHANNEL4_SET(PULSE4_VALUE);\
           })

/* Define PAN and TILT types */
#define PAN 0
#define TILT 1

/* Macros for converting between angle, duty cycle and pulse period */
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

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
void PWM_init(double duty1, double duty2, double duty3, double duty4) {

	duty1Value = duty1;
	duty2Value = duty2;
	duty3Value = duty3;
	duty4Value = duty4;

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


void s4435360_hal_pantilt_init() {
	PWM_init(0, 0, 0, 0);
}

void pantilt_angle_write(int type, int angle) {
	//Ensure angle is between -85 and 85
	angle = angle > 85 ? 85 : angle;
	angle = angle < -85 ? -85 : angle;

	if(type == PAN) {
		PWM_DUTY1_SET(ANGLE_TO_DUTYCYCLE(angle));
	} else if (type == TILT) {
		PWM_DUTY2_SET(ANGLE_TO_DUTYCYCLE(angle));
	} else {
		return;
	}
}

/**
 * @brief  Main program
 * @param  None
 * @retval None
 */
int main(void) {

	//Init hardware
	BRD_init();
	PWM_init(ANGLE_TO_DUTY_CYCLE(85),
			ANGLE_TO_DUTY_CYCLE(45),
			ANGLE_TO_DUTY_CYCLE(0),
			ANGLE_TO_DUTY_CYCLE(-45));
}
