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
TIM_HandleTypeDef squareTimInit, dataTimInit, rx_TIM_Init;;
TIM_OC_InitTypeDef squareConfig;
volatile int squareRising = 1;

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
	__TIM2_CLK_ENABLE();

	/* TIM Base configuration */
	dataTimInit.Instance = TIM2;				//Enable Timer 2
	dataTimInit.Init.Period = DATA_TIMER_FREQUENCY/frequency;
	dataTimInit.Init.Prescaler = (uint16_t) ((SystemCoreClock / 2) / DATA_TIMER_FREQUENCY) - 1;	//Set prescaler value
	dataTimInit.Init.ClockDivision = 0;			//Set clock division
	dataTimInit.Init.RepetitionCounter = 0;	// Set reload Value
	dataTimInit.Init.CounterMode = TIM_COUNTERMODE_UP;	//Set timer to count up.

	/* Initialise Timer */
	HAL_TIM_Base_Init(&dataTimInit);

	HAL_NVIC_SetPriority(TIM2_IRQn, 10, 0);		//Set Main priority to 10 and sub-priority to 0.

	// Enable the Timer 2 interrupt
	HAL_NVIC_EnableIRQ(TIM2_IRQn);

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
void TIM2_IRQHandler(void) {
	HAL_TIM_IRQHandler(&dataTimInit);
}




void ir_rx_init(void) {
	GPIO_InitTypeDef GPIO_InitStructure;

	// D33 is TIM3_CH3 so enable TIM3 clock
	__TIM3_CLK_ENABLE();

	// Enable D3 peripheral clock
	__BRD_D33_GPIO_CLK();

	// Configure the D33 pin with TIM3 alternate function
	GPIO_InitStructure.Pin = BRD_D33_PIN;					// Pin
	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;				// Set mode to alternate with push-pull drive
	GPIO_InitStructure.Pull = GPIO_NOPULL;					// No pull-(up/down) resistor enabled
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;		// I/O speed. See 29.3 in UM1725
	GPIO_InitStructure.Alternate = GPIO_AF2_TIM3;

	HAL_GPIO_Init(BRD_D33_GPIO_PORT, &GPIO_InitStructure);

	// Configure the NVIC for TIM3
	HAL_NVIC_SetPriority(TIM3_IRQn, 0x0F, 0x00);

	// Enable the TIM3 global interrupt
	HAL_NVIC_EnableIRQ(TIM3_IRQn);

	PrescalerValue = (uint16_t) ((SystemCoreClock / 2) / 50000) - 1;

	/* TIM Base configuration */
	rx_TIM_Init.Instance = TIM3;						//Enable Timer 1
	rx_TIM_Init.Init.Period = 0xFFFF;					//Minimise update events, set to max.
	rx_TIM_Init.Init.Prescaler = PrescalerValue;		//Set prescaler value
	rx_TIM_Init.Init.ClockDivision = 0;				//Set clock division
	rx_TIM_Init.Init.RepetitionCounter = 0;			// Set reload Value
	rx_TIM_Init.Init.CounterMode = TIM_COUNTERMODE_UP;	//Set timer to count up.

	if (HAL_TIM_IC_Init(&rx_TIM_Init) != HAL_OK) {
		/* Initialisation Error */
	}

	// Configure the Input Capture channel
	sICConfig.ICPolarity = TIM_ICPOLARITY_RISING;
	sICConfig.ICSelection = TIM_ICSELECTION_DIRECTTI;
	sICConfig.ICPrescaler = TIM_ICPSC_DIV1;
	sICConfig.ICFilter = 0;
	if (HAL_TIM_IC_ConfigChannel(&rx_TIM_Init, &sICConfig, TIM_CHANNEL_3)
			!= HAL_OK) {
		// Config error
	}

	// Start the Input Capture in interrupt mode
	if (HAL_TIM_IC_Start_IT(&rx_TIM_Init, TIM_CHANNEL_3) != HAL_OK) {
		// Start error
	}

}

void HAL_TIM3_IRQHandler (TIM_HandleTypeDef *htim) {
		if (uhCaptureIndex == 0) {
			/* Get the 1st Input Capture value */
			uwIC2Value1 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_3);
			uhCaptureIndex = 1;
		} else if (uhCaptureIndex == 1) {
			/* Get the 2nd Input Capture value */
			uwIC2Value2 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_3);

			/* Capture computation */
			if (uwIC2Value2 > uwIC2Value1) {
				uwDiffCapture = (uwIC2Value2 - uwIC2Value1);
			} else if (uwIC2Value2 < uwIC2Value1) {
				/* 0xFFFF is max TIM3_CCRx value */
				uwDiffCapture = ((0xFFFF - uwIC2Value1) + uwIC2Value2) + 1;
			} else {
				/* If capture values are equal, we have reached the limit of frequency
					 measures */
			}
			/* Frequency computation: How do I know if TIM3 is driven by PCLK1 or PCLK2?
			 *  What is the relationship between HCLK and PCLK1? TIMxCLK?
			 */
			uwFrequency = (TIMxCLKfreq / PrescalerValue) / uwDiffCapture;
			uhCaptureIndex = 0;

			if(uwFrequency <= 50) {
				debug_printf("Input capture TIM3_CH3 (frequency): %d\n\r", uwFrequency);
			}
	}

}

/**
  * @brief  Timer 1 Input Capture Interrupt handler
  * Override default mapping of this handler to Default_Handler
  * @param  None.
  * @retval None
  */
void TIM3_IRQHandler(void) {
	HAL_TIM3_IRQHandler(&rx_TIM_Init);
}


int main(void) {

	BRD_init();
	s4435360_hal_joystick_init();
	s4435360_lightbar_init();
	s4435360_hal_ir_init();
	ir_rx_init();
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


