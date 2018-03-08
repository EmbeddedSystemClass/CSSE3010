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
#define TIMER_FREQUENCY 50000

#define PWM_CHANNEL			TIM_CHANNEL_3
#define PWM_PIN				BRD_D3_PIN
#define PWM_TIMER			TIM1
#define PWM_GPIO_AF			GPIO_AF1_TIM1
#define PWM_PIN_CLK()		__TIM1_CLK_ENABLE()
#define PWM_TIMER_HANDLER	TIM_Init
#define PWM_DC_GET() 		__HAL_TIM_GET_COMPARE(&PWM_TIMER_HANDLER, PWM_CHANNEL)
#define PWM_DC_SET(value) 	__HAL_TIM_SET_COMPARE(&PWM_TIMER_HANDLER, PWM_CHANNEL, value)


#define JOYSTICK_TO_FREQUENCY(adcValue) 		((adcValue / 4095.0) * 50)
#define JOYSTICK_TO_LIGHTBAR_INDEX(adcValue) 	((adcValue / 4095.0) * 10)
#define FREQUENCY_TO_PERIOD_REGISTER(frequency) (TIMER_FREQUENCY / (frequency / 2.0))
#define FREQUENCY_TO_PULSE_REGISTER(frequency) 	(((FREQUENCY_TO_PERIOD_REGISTER(frequency) + 1) / 2) - 1)
#define CHANGE_PERIOD_REGISTER(value) 			__HAL_TIM_SET_AUTORELOAD(&PWM_TIMER_HANDLER, value)
#define CHANGE_PULSE_REGISTER(value)           	__HAL_TIM_SET_COMPARE(&PWM_TIMER_HANDLER, PWM_CHANNEL, value)

#define CHANGE_TIMER_FREQUENCY(frequency) CHANGE_PERIOD_REGISTER(FREQUENCY_TO_PERIOD_REGISTER(frequency)); \
											CHANGE_PULSE_REGISTER(FREQUENCY_TO_PULSE_REGISTER(frequency))

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef TIM_Init;
TIM_OC_InitTypeDef sConfig;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

void change_timer_frequency(float frequency) {
	int periodRegister, pulseRegister;

	if(frequency < ((2 * TIMER_FREQUENCY) / 65535)) {
		periodRegister = 65535;
		pulseRegister = 65535;
	} else {
		periodRegister = FREQUENCY_TO_PERIOD_REGISTER(frequency);
		pulseRegister = ((periodRegister + 1) / 2) - 1;
	}

	CHANGE_PERIOD_REGISTER(periodRegister);
	CHANGE_PULSE_REGISTER(pulseRegister);

}


void square_wave_init(float frequency) {

	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_OC_InitTypeDef PWMConfig;

	PWM_PIN_CLK();
	__BRD_D3_GPIO_CLK();

	GPIO_InitStructure.Pin = PWM_PIN;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
	GPIO_InitStructure.Alternate = PWM_GPIO_AF;
	HAL_GPIO_Init(BRD_D3_GPIO_PORT, &GPIO_InitStructure);

	TIM_Init.Instance = PWM_TIMER;
	TIM_Init.Init.Period = FREQUENCY_TO_PERIOD_REGISTER(frequency);
	TIM_Init.Init.Prescaler = ((SystemCoreClock /2) / TIMER_FREQUENCY) - 1;;
	TIM_Init.Init.ClockDivision = 0;
	TIM_Init.Init.RepetitionCounter = 0;
	TIM_Init.Init.CounterMode = TIM_COUNTERMODE_UP;

	PWMConfig.OCMode = TIM_OCMODE_PWM1;
	PWMConfig.Pulse = FREQUENCY_TO_PULSE_REGISTER(frequency);
	PWMConfig.OCPolarity = TIM_OCPOLARITY_HIGH;
	PWMConfig.OCNPolarity = TIM_OCNPOLARITY_HIGH;
	PWMConfig.OCFastMode = TIM_OCFAST_DISABLE;
	PWMConfig.OCIdleState = TIM_OCIDLESTATE_RESET;
	PWMConfig.OCNIdleState = TIM_OCNIDLESTATE_RESET;

	HAL_TIM_PWM_Init(&TIM_Init);
	HAL_TIM_PWM_ConfigChannel(&TIM_Init, &PWMConfig, PWM_CHANNEL);

	HAL_TIM_PWM_Start(&TIM_Init, PWM_CHANNEL);
}

int main(void) {
	square_wave_init(25.0);
	BRD_init();
	s4435360_hal_joystick_init();
	s4435360_lightbar_init();

	unsigned int adcX;

	/* Infinite loop */
	while (1) {

		adcX = s4435360_hal_joystick_x_read();
		change_timer_frequency(JOYSTICK_TO_FREQUENCY(adcX));
		s4435360_lightbar_write(1 << (int)JOYSTICK_TO_LIGHTBAR_INDEX(adcX));

		HAL_Delay(100);
	}
}
