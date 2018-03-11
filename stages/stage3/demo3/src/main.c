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
#include "s4435360_hal_joystick.h"
#include "s4435360_hal_ir.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define DATA_TIMER_FREQUENCY 500000

#define JOYSTICK_TO_FREQUENCY(adcValue) 		((adcValue / 4095.0) * 50)
#define JOYSTICK_TO_LIGHTBAR_INDEX(adcValue) 	((adcValue * 10 / 4095) > 10 ? 9 : (adcValue * 10 / 4095))
#define FREQUENCY_TO_PERIOD_REGISTER(frequency) (DATA_TIMER_FREQUENCY / frequency)
#define CHANGE_PERIOD_REGISTER(value) 			__HAL_TIM_SET_AUTORELOAD(&dataTimInit, value)
#define CHANGE_COUNTER_REGISTER(value) 		__HAL_TIM_SET_COUNTER(&dataTimInit, value)

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef squareTimInit, dataTimInit;
TIM_OC_InitTypeDef squareConfig;
volatile int squareRising = 1;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

void change_timer_frequency(float frequency) {
	int periodRegister;
	if(frequency < DATA_TIMER_FREQUENCY / 65535.0) {
		periodRegister = 65535;
	} else {
		periodRegister = FREQUENCY_TO_PERIOD_REGISTER(frequency);
	}

	CHANGE_COUNTER_REGISTER((__HAL_TIM_GET_COUNTER(&dataTimInit) / (float)__HAL_TIM_GET_AUTORELOAD(&dataTimInit)) * periodRegister);
	CHANGE_PERIOD_REGISTER(periodRegister);

}

void square_wave_init(float frequency) {

	// Timer 2 clock enable
	__TIM3_CLK_ENABLE();

	/* TIM Base configuration */
	dataTimInit.Instance = TIM3;				//Enable Timer 2
	dataTimInit.Init.Period = DATA_TIMER_FREQUENCY/frequency;
	dataTimInit.Init.Prescaler = (uint16_t) ((SystemCoreClock / 2) / DATA_TIMER_FREQUENCY) - 1;	//Set prescaler value
	dataTimInit.Init.ClockDivision = 0;			//Set clock division
	dataTimInit.Init.RepetitionCounter = 0;	// Set reload Value
	dataTimInit.Init.CounterMode = TIM_COUNTERMODE_UP;	//Set timer to count up.

	/* Initialise Timer */
	HAL_TIM_Base_Init(&dataTimInit);

	HAL_NVIC_SetPriority(TIM3_IRQn, 10, 0);		//Set Main priority to 10 and sub-priority to 0.

	// Enable the Timer 2 interrupt
	HAL_NVIC_EnableIRQ(TIM3_IRQn);

	// Start Timer 2 base unit in interrupt mode
	HAL_TIM_Base_Start_IT(&dataTimInit);
}


/**
 * @brief Period elapsed callback in non blocking mode
 * @param htim: Pointer to a TIM_HandleTypeDef that contains the configuration information for the TIM module.
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
		if(squareRising) {
			s4435360_hal_ir_datamodulation_set();
		} else {
			s4435360_hal_ir_datamodulation_clr();
		}

		squareRising = 1 - squareRising;

}

//Override default mapping of this handler to Default_Handler
void TIM3_IRQHandler(void) {
	HAL_TIM_IRQHandler(&dataTimInit);
}


int main(void) {

	BRD_init();
	s4435360_hal_joystick_init();
	s4435360_lightbar_init();
	s4435360_hal_ir_init();
	square_wave_init(25.0);

	unsigned int adcX;

	/* Infinite loop */
	while (1) {

		adcX = s4435360_hal_joystick_x_read();
		change_timer_frequency(JOYSTICK_TO_FREQUENCY(adcX));
		s4435360_lightbar_write(1 << (int)JOYSTICK_TO_LIGHTBAR_INDEX(adcX));
		HAL_Delay(100);
	}
}


