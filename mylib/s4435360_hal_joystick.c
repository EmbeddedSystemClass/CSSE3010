/**
 ******************************************************************************
 * @file    mylib/s4435360_hal_joystick.c
 * @author  Samuel Eadie - 44353607
 * @date    07032018-14032018
 * @brief   Joystick driver
 *
 ******************************************************************************
 *     EXTERNAL FUNCTIONS
 ******************************************************************************
 * s4435360_hal_joystick_init() – intialises joystick, ADCs
 * s4435360_hal_joystick_x_read() - reads joystick X
 * s4435360_hal_joystick_y_read() - reads hoystick Y
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <s4435360_hal_joystick.h>
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef AdcHandleX;
ADC_HandleTypeDef AdcHandleY;
ADC_ChannelConfTypeDef AdcChanConfig1;
ADC_ChannelConfTypeDef AdcChanConfig2;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

#define s4435360_hal_joystick_x_read() joystick_read(AdcHandleX)
#define s4435360_hal_joystick_y_read() joystick_read(AdcHandleY)

/**
  * @brief Reads and returns a value from the specified ADC
  * @param AdcHandle: the handle for the ADC to read
  * @retval returns the read ADC value
  */
int joystick_read(ADC_HandleTypeDef AdcHandle) {

	/* Start ADC conversion */
	HAL_ADC_Start(&AdcHandle);

	/* Wait for ADC conversion to finish */
	while (HAL_ADC_PollForConversion(&AdcHandle, 10) != HAL_OK);

	/* Return value */
	return (uint16_t)(HAL_ADC_GetValue(&AdcHandle));
}

/**
  * @brief  Initialises the joystick hardware
  * @param None
  * @retval None
  */
void s4435360_hal_joystick_init(void) {

	GPIO_InitTypeDef GPIO_InitStructure;

	__BRD_A0_GPIO_CLK();

	GPIO_InitStructure.Pin = BRD_A0_PIN;
	GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStructure.Pull = GPIO_NOPULL;

	HAL_GPIO_Init(BRD_A0_GPIO_PORT, &GPIO_InitStructure);

	__ADC1_CLK_ENABLE();

	AdcHandleX.Instance = (ADC_TypeDef *)(ADC1_BASE);						//Use ADC1
	AdcHandleX.Init.ClockPrescaler        = ADC_CLOCKPRESCALER_PCLK_DIV2;	//Set clock prescaler
	AdcHandleX.Init.Resolution            = ADC_RESOLUTION12b;				//Set 12-bit data resolution
	AdcHandleX.Init.ScanConvMode          = DISABLE;
	AdcHandleX.Init.ContinuousConvMode    = DISABLE;
	AdcHandleX.Init.DiscontinuousConvMode = DISABLE;
	AdcHandleX.Init.NbrOfDiscConversion   = 0;
	AdcHandleX.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;	//No Trigger
	AdcHandleX.Init.ExternalTrigConv      = ADC_EXTERNALTRIGCONV_T1_CC1;		//No Trigger
	AdcHandleX.Init.DataAlign             = ADC_DATAALIGN_RIGHT;				//Right align data
	AdcHandleX.Init.NbrOfConversion       = 1;
	AdcHandleX.Init.DMAContinuousRequests = DISABLE;
	AdcHandleX.Init.EOCSelection          = DISABLE;

	HAL_ADC_Init(&AdcHandleX);		//Initialise ADC

	/* Configure ADC Channel */
	AdcChanConfig1.Channel = BRD_A0_ADC_CHAN;							//Use AO pin
	AdcChanConfig1.Rank         = 1;
	AdcChanConfig1.SamplingTime = ADC_SAMPLETIME_3CYCLES;
	AdcChanConfig1.Offset       = 0;

	HAL_ADC_ConfigChannel(&AdcHandleX, &AdcChanConfig1);		//Initialise ADC channel

	__BRD_A1_GPIO_CLK();

	GPIO_InitStructure.Pin = BRD_A1_PIN;

	HAL_GPIO_Init(BRD_A1_GPIO_PORT, &GPIO_InitStructure);

	__ADC2_CLK_ENABLE();

	AdcHandleY.Instance = (ADC_TypeDef *)(ADC2_BASE);						//Use ADC2
	AdcHandleY.Init.ClockPrescaler        = ADC_CLOCKPRESCALER_PCLK_DIV2;	//Set clock prescaler
	AdcHandleY.Init.Resolution            = ADC_RESOLUTION12b;				//Set 12-bit data resolution
	AdcHandleY.Init.ScanConvMode          = DISABLE;
	AdcHandleY.Init.ContinuousConvMode    = DISABLE;
	AdcHandleY.Init.DiscontinuousConvMode = DISABLE;
	AdcHandleY.Init.NbrOfDiscConversion   = 0;
	AdcHandleY.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;	//No Trigger
	AdcHandleY.Init.ExternalTrigConv      = ADC_EXTERNALTRIGCONV_T1_CC1;		//No Trigger
	AdcHandleY.Init.DataAlign             = ADC_DATAALIGN_RIGHT;				//Right align data
	AdcHandleY.Init.NbrOfConversion       = 1;
	AdcHandleY.Init.DMAContinuousRequests = DISABLE;
	AdcHandleY.Init.EOCSelection          = DISABLE;

	HAL_ADC_Init(&AdcHandleY);		//Initialise ADC

	/* Configure ADC Channel */
	AdcChanConfig2.Channel = BRD_A1_ADC_CHAN;							//Use AO pin
	AdcChanConfig2.Rank         = 1;
	AdcChanConfig2.SamplingTime = ADC_SAMPLETIME_3CYCLES;
	AdcChanConfig2.Offset       = 0;

	HAL_ADC_ConfigChannel(&AdcHandleY, &AdcChanConfig2);		//Initialise ADC channel
}
