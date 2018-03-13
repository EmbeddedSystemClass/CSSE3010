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
#define CHANGE_PERIOD_REGISTER(value) 		__HAL_TIM_SET_AUTORELOAD(&dataTimInit, value)
#define CHANGE_COUNTER_REGISTER(value) 		__HAL_TIM_SET_COUNTER(&dataTimInit, value)

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef dataTimInit;
TIM_OC_InitTypeDef squareConfig;
volatile int dataModulationClr = 1;

/* Private variables for input capture */
TIM_HandleTypeDef rx_TIM_Init;

/* Timer Input Capture Configuration Structure declaration */
TIM_IC_InitTypeDef sICConfig;

uint16_t PrescalerValue = 0;
uint32_t TIMxCLKfreq = 16000000; //SystemCoreClock;


/* Captured Values */
uint32_t uwIC2Value1 = 0;
uint32_t uwIC2Value2 = 0;
uint32_t uwDiffCapture = 0;

/* Capture index */
uint16_t uhCaptureIndex = 0;

/* Frequency Value */
uint32_t uwFrequency = 0;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
 * @brief Changes the modulation signal timer frequency
 * @param frequency: the new modulation signal frequency
 * @retval Nones
 */
void change_timer_frequency(float frequency) {
	int periodRegister;

	/* Check for edge case (register overflow at low frequencies) */
	if(frequency < DATA_TIMER_FREQUENCY / 65535.0) {
		periodRegister = 65535;
	} else {
		periodRegister = FREQUENCY_TO_PERIOD_REGISTER(frequency);
	}

	/* Change counter register to preserve portion through period */
	CHANGE_COUNTER_REGISTER((__HAL_TIM_GET_COUNTER(&dataTimInit) /
			(float)__HAL_TIM_GET_AUTORELOAD(&dataTimInit)) * periodRegister);

	/* Change period register to new frequency */
	CHANGE_PERIOD_REGISTER(periodRegister);

}

/**
 * @brief Initialises a timer to modulate the carrier wave
 * @param frequency: the initial frequency of the modulating wave
 * @retval None
 */
void square_wave_init(float frequency) {

	// Timer 3 clock enable
	__TIM3_CLK_ENABLE();

	/* TIM Base configuration */
	dataTimInit.Instance = TIM3;
	dataTimInit.Init.Period = DATA_TIMER_FREQUENCY/frequency;
	dataTimInit.Init.Prescaler = (uint16_t) ((SystemCoreClock / 2) / DATA_TIMER_FREQUENCY) - 1;
	dataTimInit.Init.ClockDivision = 0;
	dataTimInit.Init.RepetitionCounter = 0;
	dataTimInit.Init.CounterMode = TIM_COUNTERMODE_UP;

	/* Initialise Timer */
	HAL_TIM_Base_Init(&dataTimInit);
	HAL_NVIC_SetPriority(TIM3_IRQn, 10, 0);
	HAL_NVIC_EnableIRQ(TIM3_IRQn);
	HAL_TIM_Base_Start_IT(&dataTimInit);
}

/**
 * @brief Period elapsed callback routine, for toggling modulation signal wave
 * @param htim: Pointer to a TIM_HandleTypeDef that contains the configuration
 * 		 information for the TIM module.
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {

	/* Toggle modulation signal */
	if(dataModulationClr) {
		s4435360_hal_ir_datamodulation_set();
	} else {
		s4435360_hal_ir_datamodulation_clr();
	}

	dataModulationClr = 1 - dataModulationClr;

}

/**
 * @brief Interrupt handler for Timer 3
 * @param None
 * @retval None
 */
void TIM3_IRQHandler(void) {
	HAL_TIM_IRQHandler(&dataTimInit);
}

/**
 * @brief Initialises the receive functionality
 * @param None
 * @retval None
 */
void ir_rx_init(void) {

	/* Initialise receive pin */
	GPIO_InitTypeDef GPIO_InitStructure;
	__TIM2_CLK_ENABLE();
	__BRD_D35_GPIO_CLK();

	GPIO_InitStructure.Pin = BRD_D35_PIN;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;
	GPIO_InitStructure.Alternate = GPIO_AF1_TIM2;

	HAL_GPIO_Init(BRD_D35_GPIO_PORT, &GPIO_InitStructure);
	HAL_NVIC_SetPriority(TIM2_IRQn, 0x0F, 0x00);
	HAL_NVIC_EnableIRQ(TIM2_IRQn);

	/* Calculate prescalar value */
	PrescalerValue = (uint16_t) ((SystemCoreClock / 2) / 50000) - 1;

	//TIM Base configuration
	rx_TIM_Init.Instance = TIM2;
	rx_TIM_Init.Init.Period = 0xFFFF; //Minimise update events
	rx_TIM_Init.Init.Prescaler = PrescalerValue;
	rx_TIM_Init.Init.ClockDivision = 0;
	rx_TIM_Init.Init.RepetitionCounter = 0;
	rx_TIM_Init.Init.CounterMode = TIM_COUNTERMODE_UP;

	if (HAL_TIM_IC_Init(&rx_TIM_Init) != HAL_OK) {
		debug_printf("Initialisation Error: IR Receiver "
				"Timer Pin D35\r\n");
	}

	// Configure the Input Capture channel
	sICConfig.ICPolarity = TIM_ICPOLARITY_RISING;
	sICConfig.ICSelection = TIM_ICSELECTION_DIRECTTI;
	sICConfig.ICPrescaler = TIM_ICPSC_DIV1;
	sICConfig.ICFilter = 0;
	if (HAL_TIM_IC_ConfigChannel(&rx_TIM_Init, &sICConfig, TIM_CHANNEL_4)
			!= HAL_OK) {
		debug_printf("Initialisation Error: IR Receiver "
				"Input Capture Pin D35\r\n");
	}

	// Start the Input Capture in interrupt mode
	if (HAL_TIM_IC_Start_IT(&rx_TIM_Init, TIM_CHANNEL_4) != HAL_OK) {
		debug_printf("Initialisation Error: IR Receiver Input "
				"Capture Start Pin D35\r\n");
	}

}

/**
 * @brief Callback routine for input compare matches for IR receiver
 * @param htim: the timer handler of the triggering timer
 * @retval None
 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef* htim) {

	/* Check the triggering timer channel */
	if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4) {

		/* Check if first or second trigger */
		if (uhCaptureIndex == 0) {

			//Get the 1st Input Capture value
			uwIC2Value1 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_4);
			uhCaptureIndex = 1;

		} else if (uhCaptureIndex == 1) {

			//Get the 2nd Input Capture value
			uwIC2Value2 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_4);

			//Two captures, can compute frequency
			if (uwIC2Value2 > uwIC2Value1) {
				uwDiffCapture = (uwIC2Value2 - uwIC2Value1);
			} else if (uwIC2Value2 < uwIC2Value1) {
				uwDiffCapture = ((0xFFFF - uwIC2Value1) + uwIC2Value2) + 1;
			} else {
				debug_printf("Reached Max Frequency\r\n");
			}

			uwFrequency = (TIMxCLKfreq / PrescalerValue) / uwDiffCapture;
			uhCaptureIndex = 0;

			if(uwFrequency <= 50) {
				debug_printf("IR receiver frequency: %d\n\r", uwFrequency);
			}
		}
	}
}

/**
 * @brief Interrupt handler for Timer 2
 * @param None
 * @retval None
 */
void TIM2_IRQHandler(void) {
	HAL_TIM_IRQHandler(&rx_TIM_Init);
}

int main(void) {

	/* Initialise hardware */
	BRD_init();
	s4435360_hal_joystick_init();
	s4435360_lightbar_init();
	s4435360_hal_ir_init();
	square_wave_init(25.0);
	ir_rx_init();

	unsigned int adcX;

	/* Infinite loop */
	while (1) {

		adcX = s4435360_hal_joystick_x_read();

		change_timer_frequency(JOYSTICK_TO_FREQUENCY(adcX));
		s4435360_lightbar_write(1 << (int)JOYSTICK_TO_LIGHTBAR_INDEX(adcX));

		HAL_Delay(100);
	}
}
