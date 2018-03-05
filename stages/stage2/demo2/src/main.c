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
#include "s4435360_hal_lightbar.h"

/* Clock scaling and timing values */
#define  COUNTER_CLOCK      500000
#define  PRESCALAR 			(uint32_t)((SystemCoreClock) / COUNTER_CLOCK) - 1
#define  PERIOD 			20 /* Period of PWM (ms) */
#define  PERIOD_VALUE       (COUNTER_CLOCK / (1000 / PERIOD))

/* Macros for converting between angle and pulse period/register value */
#define  ANGLE_TO_PULSE_PERIOD(angle) ((angle / 90.0) + 1.45)
#define  PULSE_PERIOD_TO_ANGLE(pulsePeriod) ((90.0 * pulsePeriod) - 130.5)
#define  PULSE_PERIOD_TO_REGISTER(period) ((period / (float)PERIOD) * (float)PERIOD_VALUE)
#define  PULSE_REGISTER_TO_PERIOD(value) ((value / (float)PERIOD_VALUE) * (float)PERIOD)
#define  ANGLE_TO_PERIOD_REGISTER(angle) PULSE_PERIOD_TO_REGISTER(ANGLE_TO_PULSE_PERIOD(angle))
#define  PERIOD_REGISTER_TO_ANGLE(value) PULSE_PERIOD_TO_ANGLE(PULSE_REGISTER_TO_PERIOD(value))

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


/*Wrappers for defining tilt/pan set and get */
#define PWM_CHANNEL_PAN_SET(value)  PWM_CHANNEL1_SET(value)
#define PWM_CHANNEL_PAN_GET() 		PWM_CHANNEL1_GET()
#define PWM_CHANNEL_TILT_SET(value) PWM_CHANNEL2_SET(value)
#define PWM_CHANNEL_TILT_GET() 		PWM_CHANNEL2_GET()

/* Define PAN and TILT types */
#define PAN 0
#define TILT 1

void pantilt_angle_write(int type, int angle);
int pantilt_angle_read(int type);

/* External function prototypes -----------------------------------------------*/
#define s4435360_hal_pantilt_pan_write(angle) pantilt_angle_write(PAN, angle)
#define s4435360_hal_pantilt_pan_read() pantilt_angle_read(PAN)
#define s4435360_hal_pantilt_tile_write(angle) pantilt_angle_write(TILT, angle)
#define s4435360_hal_pantilt_tilt_read() pantilt_angle_read(TILT)
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define RIGHT_CHAR 'p'
#define LEFT_CHAR 'n'
#define METRONOME_CHAR 'm'
#define FASTER_CHAR '+'
#define SLOWER_CHAR '-'
#define RIGHT_INCREMENT 10
#define LEFT_INCREMENT -10
#define FASTER_INCREMENT -1
#define SLOWER_INCREMENT 1
#define RIGHT_EDGE 70
#define LEFT_EDGE -70
#define SHORTEST_PERIOD 2
#define LONGEST_PERIOD 20
#define METRONOME_UPDATE_TIME 100
/* Private variables ---------------------------------------------------------*/
uint32_t prescalerValue = 0;
TIM_OC_InitTypeDef sConfig;
int inMetronomeMode = 0;
int metronomePeriod = 10; //in seconds
int metronomeDirection = METRONOME_UPDATE_TIME;
int metronomeCount = 1;
TIM_HandleTypeDef TIM_Init;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

void PWM_init(int pulse1, int pulse2, int pulse3, int pulse4) {

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
	sConfig.Pulse = pulse1;
	if (HAL_TIM_PWM_ConfigChannel(&TimInit, &sConfig, TIM_CHANNEL_1) != HAL_OK) {
		/* Configuration Error */
	}

	/* Set the pulse value for channel 2 */
	sConfig.Pulse = pulse2;
	if (HAL_TIM_PWM_ConfigChannel(&TimInit, &sConfig, TIM_CHANNEL_2) != HAL_OK) {
		/* Configuration Error */
	}

	/* Set the pulse value for channel 3 */
	sConfig.Pulse = pulse3;
	if (HAL_TIM_PWM_ConfigChannel(&TimInit, &sConfig, TIM_CHANNEL_3) != HAL_OK) {
		/* Configuration Error */
	}

	/* Set the pulse value for channel 4 */
	sConfig.Pulse = pulse4;
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

void s4435360_hal_pantilt_init(void) {
	PWM_init(ANGLE_TO_PERIOD_REGISTER(0),
			ANGLE_TO_PERIOD_REGISTER(0),
			ANGLE_TO_PERIOD_REGISTER(0),
			ANGLE_TO_PERIOD_REGISTER(0));
}

void metronome_timer_init(void) {

	__TIM2_CLK_ENABLE();

	/* TIM Base configuration */
	TIM_Init.Instance = TIM2;				//Enable Timer 2
  	TIM_Init.Init.Period = 50000 / (1000 / METRONOME_UPDATE_TIME); //1ms timer
  	TIM_Init.Init.Prescaler = (uint16_t) (SystemCoreClock / 50000) - 1;	//Set prescaler value
  	TIM_Init.Init.ClockDivision = 0;			//Set clock division
	TIM_Init.Init.RepetitionCounter = 0;	// Set reload Value
  	TIM_Init.Init.CounterMode = TIM_COUNTERMODE_UP;	//Set timer to count up.

	/* Initialise Timer */
	HAL_TIM_Base_Init(&TIM_Init);

	/* Set priority of Timer 2 update interrupt to lowest priority */
	HAL_NVIC_SetPriority(TIM2_IRQn, 10, 0);

	// Enable the Timer 2 interrupt
	HAL_NVIC_EnableIRQ(TIM2_IRQn);

	// Start Timer 2 base unit in interrupt mode
	HAL_TIM_Base_Start_IT(&TIM_Init);
}

void move_needle(int increment);

int angle_to_lightbar_index(int angle) {
	int ledPos = ((angle / 14) + 5);
	if(ledPos > 9) {
		ledPos = 9;
	} else if (ledPos < 0) {
		ledPos = 0;
	}

	return ledPos;
}

/**
 * @brief Period elapsed callback in non blocking mode
 * @param htim: Pointer to a TIM_HandleTypeDef that contains the configuration information for the TIM module.
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if(inMetronomeMode) {

		/* Switch metronome direction if at edge */
		if((metronomeCount >= metronomePeriod * 1000) ||
				(metronomeCount <= 0)) {
			metronomeDirection *= -1;
		}

		metronomeCount += metronomeDirection;

		int angle = (80.0 * ((float)metronomeCount / (float)metronomePeriod / 1000.0)) - 40;
		s4435360_hal_pantilt_pan_write(angle);
		s4435360_lightbar_write(1 << angle_to_lightbar_index(angle));
	}
}

//Override default mapping of this handler to Default_Handler
void TIM2_IRQHandler(void) {
	HAL_TIM_IRQHandler(&TIM_Init);

}

void pantilt_angle_write(int type, int angle) {

	//Ensure angle is between -85 and 85
	if(angle > 85) {
		angle = 85;
	} else if(angle < -85) {
		angle = -85;
	}

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

int pantilt_angle_read(int type) {

	switch(type) {
		case PAN:
			return PERIOD_REGISTER_TO_ANGLE(PWM_CHANNEL_PAN_GET());
		case TILT:
			return PERIOD_REGISTER_TO_ANGLE(PWM_CHANNEL_TILT_GET());
		default:
			return 0;
	}
}

void move_needle(int increment) {

	int currentAngle = s4435360_hal_pantilt_pan_read();
	int newAngle;

	/* Check angle is between +-70 */
	if(currentAngle + increment > RIGHT_EDGE) {
		newAngle = RIGHT_EDGE;
	} else if(currentAngle + increment < LEFT_EDGE) {
		newAngle = LEFT_EDGE;
	} else {
		newAngle = currentAngle + increment;
	}

	s4435360_hal_pantilt_pan_write(newAngle);
	s4435360_lightbar_write(1 << angle_to_lightbar_index(newAngle));
}

void changePeriod(int increment) {
	metronomePeriod += increment;

	if(metronomePeriod > LONGEST_PERIOD) {
		metronomePeriod = LONGEST_PERIOD;
	} else if (metronomePeriod < SHORTEST_PERIOD) {
		metronomePeriod = SHORTEST_PERIOD;
	}

	int currentAngle = s4435360_hal_pantilt_pan_read();
	metronomeCount = ((currentAngle + 40) / 80.0) * metronomePeriod * 1000;
}

void process_command(char command) {
	switch(command) {

		case RIGHT_CHAR:
			if(!inMetronomeMode) {
				move_needle(RIGHT_INCREMENT);
			}
			break;

		case LEFT_CHAR:
			if(inMetronomeMode) {
				inMetronomeMode = 0;
			} else {
				move_needle(LEFT_INCREMENT);
			}
			break;

		case METRONOME_CHAR:
			inMetronomeMode = 1;
			break;
		case SLOWER_CHAR:
			if(inMetronomeMode) {
				changePeriod(SLOWER_INCREMENT);
			}
			break;
		case FASTER_CHAR:
			if(inMetronomeMode) {
				changePeriod(FASTER_INCREMENT);
			}
			break;
		default:
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
	s4435360_hal_pantilt_init();
	metronome_timer_init();
	s4435360_lightbar_init();
	s4435360_lightbar_write(1 << angle_to_lightbar_index(0));

	char commandChar;
	setbuf(stdout, NULL);

	while(1) {
		commandChar = debug_getc();
		process_command(commandChar);
		HAL_Delay(250);
	}
}
