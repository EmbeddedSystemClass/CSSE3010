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

TIM_HandleTypeDef rx_TIM_Init;
TIM_IC_InitTypeDef sICConfig;

//NEC protocol IR receive variables
int stage = 0;
uint32_t lastCaptureValue = 0;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
uint32_t PrescalerValue;
void s4435360_hal_ir_rx_init(void) {

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
	sICConfig.ICPolarity = TIM_ICPOLARITY_FALLING; //Trigger on both edges
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

	receivedIRFlag = 0;
	irCommand = 0;
}


/**
 * @brief Callback routine for input compare matches for IR receiver
 * @param htim: the timer handler of the triggering timer
 * @retval None
 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef* htim) {

	/* Check the triggering timer channel */
	if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4) {

		//Record capture compare
		uint32_t currentCaptureValue = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_4);

		//Calculate compare difference
		uint32_t captureValueDifference;
		if (currentCaptureValue > lastCaptureValue) {
			captureValueDifference = (currentCaptureValue - lastCaptureValue);
		} else if (lastCaptureValue < currentCaptureValue) {
			captureValueDifference = ((0xFFFF - lastCaptureValue) + currentCaptureValue) + 1;
		}


		//debug_printf("%d", stage);

		if(stage == 0) {
			lastCaptureValue = currentCaptureValue;
		}

		if(stage == 1) {
			//debug_printf("(%d)", captureValueDifference);
			if(captureValueDifference < 1200) {
				stage = 0;
			} else {
				stage++;
			}
		} else if(stage < 17) {
			//irCommand = 0;
			stage++;
		} else if(stage == 17) {
			lastCaptureValue = currentCaptureValue;
			stage++;
		} else if(stage < 25){

			stage++;

			//debug_printf("%d, ", captureValueDifference);

			if(captureValueDifference  > 150) {
				irCommand = (irCommand << 1) + 1;
			} else if (captureValueDifference < 300){
				irCommand = (irCommand << 1);
			}

			lastCaptureValue = currentCaptureValue;

		} else {
			stage++;
		}

		if(stage == 34) {
			receivedIRFlag = 1;
			stage = 0;
		}

	}
}

/**
 * @brief TIM2 handler, rx input capture timer
 * @param None
 * @retval None
 */
void TIM2_IRQHandler(void) {
	HAL_TIM_IRQHandler(&rx_TIM_Init);
}

/**
 * @brief Initialises infrared communication hardware
 * @param None
 * @retval None
 */
void s4435360_hal_ir_init(void) {

	/* TX carrier and modulation pin init */
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

	/* RX input capture init */
	GPIO_InitTypeDef GPIO_InitStructure;
	__INPUT_CAPTURE_CLK_ENABLE();
	__RX_PIN_CLK_ENABLE();

	GPIO_InitStructure.Pin = RX_INPUT_PIN;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;
	GPIO_InitStructure.Alternate = INPUT_GPIO_AF;

	HAL_GPIO_Init(RX_INPUT_PORT, &GPIO_InitStructure);
	HAL_NVIC_SetPriority(TIM2_IRQn, 0x0F, 0x00);
	HAL_NVIC_EnableIRQ(TIM2_IRQn);

	//TIM Base configuration
	rx_TIM_Init.Instance = RX_TIM;
	rx_TIM_Init.Init.Period = 0xFFFF; //Minimise update events
	rx_TIM_Init.Init.Prescaler = (uint16_t) ((SystemCoreClock / 2) / 50000) - 1;
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
	if (HAL_TIM_IC_ConfigChannel(&rx_TIM_Init, &sICConfig, RX_TIM_CHANNEL)
			!= HAL_OK) {
		debug_printf("Initialisation Error: IR Receiver "
				"Input Capture Pin D35\r\n");
	}

	// Start the Input Capture in interrupt mode
	if (HAL_TIM_IC_Start_IT(&rx_TIM_Init, RX_TIM_CHANNEL) != HAL_OK) {
		debug_printf("Initialisation Error: IR Receiver Input "
				"Capture Start Pin D35\r\n");
	}
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
