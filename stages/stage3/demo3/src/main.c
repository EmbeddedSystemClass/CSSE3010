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
#include "debug_printf.h"
#include "stm32f4xx_hal_conf.h"
#include "board.h"
#include "s4435360_hal_lightbar.h"
#include "s4435360_hal_pantilt.h"
#include "s4435360_hal_joystick.h"
#include "stdint.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define SQUARE_TIMER_FREQUENCY  50000
#define SQUARE_CHANNEL			TIM_CHANNEL_3
#define SQUARE_PORT 			BRD_D3_GPIO_PORT
#define SQUARE_PIN				BRD_D3_PIN
#define SQUARE_PORT_CLK()		__BRD_D3_GPIO_CLK()
#define SQUARE_TIMER			TIM1
#define SQUARE_GPIO_AF			GPIO_AF1_TIM1
#define SQUARE_TIM_CLK()		__TIM1_CLK_ENABLE()
#define SQUARE_TIMER_HANDLER	squareTimInit

#define CARRIER_TIMER_FREQUENCY 37900
#define CARRIER_CHANNEL 		TIM_CHANNEL_1
#define CARRIER_PORT			BRD_D29_GPIO_PORT
#define CARRIER_PIN				BRD_D29_PIN
#define CARRIER_PORT_CLK()		__BRD_D29_GPIO_CLK()
#define CARRIER_TIMER			TIM4
#define CARRIER_GPIO_AF			GPIO_AF2_TIM4
#define CARRIER_TIM_CLK()		__TIM4_CLK_ENABLE()
#define CARRIER_TIMER_HANDLER	carrierTimInit

#define JOYSTICK_TO_FREQUENCY(adcValue) 		((adcValue / 4095.0) * 50)
#define JOYSTICK_TO_LIGHTBAR_INDEX(adcValue) 	((adcValue / 4095.0) * 10)
#define FREQUENCY_TO_PERIOD_REGISTER(frequency) (SQUARE_TIMER_FREQUENCY / (frequency / 2.0))
#define FREQUENCY_TO_PULSE_REGISTER(frequency) 	(((FREQUENCY_TO_PERIOD_REGISTER(frequency) + 1) / 2) - 1)
#define CHANGE_PERIOD_REGISTER(value) 			__HAL_TIM_SET_AUTORELOAD(&SQUARE_TIMER_HANDLER, value)
#define CHANGE_PULSE_REGISTER(value)           	__HAL_TIM_SET_COMPARE(&SQUARE_TIMER_HANDLER, SQUARE_CHANNEL, value)

#define CHANGE_TIMER_FREQUENCY(frequency) CHANGE_PERIOD_REGISTER(FREQUENCY_TO_PERIOD_REGISTER(frequency)); \
											CHANGE_PULSE_REGISTER(FREQUENCY_TO_PULSE_REGISTER(frequency))

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef squareTimInit, carrierTimInit;
TIM_OC_InitTypeDef squareConfig, carrierConfig;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

void change_timer_frequency(float frequency) {
	int periodRegister, pulseRegister;

	if(frequency < ((2 * SQUARE_TIMER_FREQUENCY) / 65535)) {
		periodRegister = 65535;
		pulseRegister = 32767;
	} else {
		periodRegister = FREQUENCY_TO_PERIOD_REGISTER(frequency);
		pulseRegister = ((periodRegister + 1) / 2) - 1;
	}

	CHANGE_PERIOD_REGISTER(periodRegister);
	CHANGE_PULSE_REGISTER(pulseRegister);

}

void carrier_wave_init(void) {

	GPIO_InitTypeDef GPIO_InitStruct;
	TIM_OC_InitTypeDef PWMConf;

	CARRIER_TIM_CLK();
	CARRIER_PORT_CLK();

	GPIO_InitStruct.Pin = CARRIER_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
	GPIO_InitStruct.Alternate = CARRIER_GPIO_AF;
	HAL_GPIO_Init(CARRIER_PORT, &GPIO_InitStruct);

	carrierTimInit.Instance = CARRIER_TIMER;
	carrierTimInit.Init.Period = 1;
	carrierTimInit.Init.Prescaler = ((SystemCoreClock / 2) / (CARRIER_TIMER_FREQUENCY)) - 1;
	carrierTimInit.Init.ClockDivision = 0;
	carrierTimInit.Init.RepetitionCounter = 0;
	carrierTimInit.Init.CounterMode = TIM_COUNTERMODE_UP;

	PWMConf.OCMode = TIM_OCMODE_PWM1;
	PWMConf.Pulse = 1;
	PWMConf.OCPolarity = TIM_OCPOLARITY_HIGH;
	PWMConf.OCNPolarity = TIM_OCNPOLARITY_HIGH;
	PWMConf.OCFastMode = TIM_OCFAST_DISABLE;
	PWMConf.OCIdleState = TIM_OCIDLESTATE_RESET;
	PWMConf.OCNIdleState = TIM_OCNIDLESTATE_RESET;

	HAL_TIM_PWM_Init(&carrierTimInit);
	HAL_TIM_PWM_ConfigChannel(&carrierTimInit, &PWMConf, CARRIER_CHANNEL);

	HAL_TIM_PWM_Start(&carrierTimInit, CARRIER_CHANNEL);
}

void square_wave_init(float frequency) {
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_OC_InitTypeDef PWMConfig;

	SQUARE_TIM_CLK();
	SQUARE_PORT_CLK();


	GPIO_InitStructure.Pin = SQUARE_PIN;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
	GPIO_InitStructure.Alternate = SQUARE_GPIO_AF;
	HAL_GPIO_Init(SQUARE_PORT, &GPIO_InitStructure);

	squareTimInit.Instance = SQUARE_TIMER;
	squareTimInit.Init.Period = FREQUENCY_TO_PERIOD_REGISTER(frequency);
	squareTimInit.Init.Prescaler = ((SystemCoreClock /2) / SQUARE_TIMER_FREQUENCY) - 1;;
	squareTimInit.Init.ClockDivision = 0;
	squareTimInit.Init.RepetitionCounter = 0;
	squareTimInit.Init.CounterMode = TIM_COUNTERMODE_UP;

	PWMConfig.OCMode = TIM_OCMODE_PWM1;
	PWMConfig.Pulse = FREQUENCY_TO_PULSE_REGISTER(frequency);
	PWMConfig.OCPolarity = TIM_OCPOLARITY_HIGH;
	PWMConfig.OCNPolarity = TIM_OCNPOLARITY_HIGH;
	PWMConfig.OCFastMode = TIM_OCFAST_DISABLE;
	PWMConfig.OCIdleState = TIM_OCIDLESTATE_RESET;
	PWMConfig.OCNIdleState = TIM_OCNIDLESTATE_RESET;

	HAL_TIM_PWM_Init(&squareTimInit);
	HAL_TIM_PWM_ConfigChannel(&squareTimInit, &PWMConfig, SQUARE_CHANNEL);

	HAL_TIM_PWM_Start(&squareTimInit, SQUARE_CHANNEL);
}

int main(void) {
	BRD_init();
	s4435360_hal_joystick_init();
	s4435360_lightbar_init();
	square_wave_init(25.0);
	carrier_wave_init();


	unsigned int adcX;

	/* Infinite loop */
	while (1) {

		adcX = s4435360_hal_joystick_x_read();
		change_timer_frequency(JOYSTICK_TO_FREQUENCY(adcX));
		s4435360_lightbar_write(1 << (int)JOYSTICK_TO_LIGHTBAR_INDEX(adcX));

		HAL_Delay(100);
	}
}
