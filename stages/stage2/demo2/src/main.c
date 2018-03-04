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
#define  COUNTER_CLOCK      500000
#define  PRESCALAR 			(uint32_t)((SystemCoreClock) / COUNTER_CLOCK) - 1
#define  PERIOD 			20 /* Period of PWM (ms) */
#define  PERIOD_VALUE       ((SystemCoreClock / PRESCALAR) / (1000 / PERIOD))

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

/* Macros for converting between angle (degrees), duty cycle (%) and pulse period (ms) */
#define ANGLE_TO_DUTY_CYCLE(angle) ((angle *5 / 90) + 7.25)
#define ANGLE_TO_PULSE_PERIOD(angle) ((angle / 90) + 1.45)
#define DUTY_CYCLE_TO_ANGLE(dutyCycle) ((18 * dutyCycle) - 130.5)
#define PULSE_PERIOD_TO_ANGLE(pulsePeriod) ((90 * pulsePeriod) - 130.5)

/* Macros for converting between physical values and register values */
#define DUTY_TO_PULSE(duty)  (PERIOD_VALUE * duty / 100)
#define PULSE_TO_DUTY(pulse) (pulse * 100 / (PERIOD_VALUE))
#define ANGLE_TO_PULSE(angle) DUTY_TO_PULSE(ANGLE_TO_DUTY_CYCLE(angle))
#define PULSE_TO_ANGLE(pulse) DUTY_CYCLE_TO_ANGLE(PULSE_TO_DUTY(pulse))


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
#define RIGHT_INCREMENT 30
#define LEFT_INCREMENT -30
#define FASTER_INCREMENT -1
#define SLOWER_INCREMENT 1
#define RIGHT_EDGE 70
#define LEFT_EDGE -70
#define SHORTEST_PERIOD 2
#define LONGEST_PERIOD 20
/* Private variables ---------------------------------------------------------*/
uint32_t prescalerValue = 0;
TIM_OC_InitTypeDef sConfig;
int inMetronomeMode = 0;
int metronomePeriod = 10;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
void PWM_init(double duty1, double duty2, double duty3, double duty4) {

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
	sConfig.Pulse = DUTY_TO_PULSE(duty1);
	if (HAL_TIM_PWM_ConfigChannel(&TimInit, &sConfig, TIM_CHANNEL_1) != HAL_OK) {
		/* Configuration Error */
	}

	/* Set the pulse value for channel 2 */
	sConfig.Pulse = DUTY_TO_PULSE(duty2);
	if (HAL_TIM_PWM_ConfigChannel(&TimInit, &sConfig, TIM_CHANNEL_2) != HAL_OK) {
		/* Configuration Error */
	}

	/* Set the pulse value for channel 3 */
	sConfig.Pulse = DUTY_TO_PULSE(duty3);
	if (HAL_TIM_PWM_ConfigChannel(&TimInit, &sConfig, TIM_CHANNEL_3) != HAL_OK) {
		/* Configuration Error */
	}

	/* Set the pulse value for channel 4 */
	sConfig.Pulse = DUTY_TO_PULSE(duty4);
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
	PWM_init(ANGLE_TO_DUTY_CYCLE(0),
			ANGLE_TO_DUTY_CYCLE(0),
			ANGLE_TO_DUTY_CYCLE(0),
			ANGLE_TO_DUTY_CYCLE(0));
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
			debug_printf("Pulse = %d\n\r", ANGLE_TO_PULSE(angle));
			PWM_CHANNEL_PAN_SET(ANGLE_TO_PULSE(angle));
			break;

		case TILT:

			PWM_CHANNEL_TILT_SET(ANGLE_TO_PULSE(angle));
			break;

		default:
			break;

	}
}

int pantilt_angle_read(int type) {

	switch(type) {
		case PAN:
			return PULSE_TO_ANGLE(PWM_CHANNEL_PAN_GET());
		case TILT:
			return PULSE_TO_ANGLE(PWM_CHANNEL_TILT_GET());
		default:
			return 0;
	}
}

void move_needle(int increment) {

	int currentAngle = s4435360_hal_pantilt_pan_read();
	debug_printf("Current angle = %d\r\n", currentAngle);
	int newAngle;

	/* Check angle is between +-70 */
	if(currentAngle + increment > RIGHT_EDGE) {
		newAngle = RIGHT_EDGE;
	} else if(currentAngle + increment < LEFT_EDGE) {
		newAngle = LEFT_EDGE;
	} else {
		newAngle = currentAngle + increment;
	}

	debug_printf("new angle = %d\r\n", newAngle);
	s4435360_hal_pantilt_pan_write(newAngle);
}

void changePeriod(int increment) {
	/* Check period is between 2 and 20s */
	if(metronomePeriod + increment > LONGEST_PERIOD) {
		metronomePeriod = LONGEST_PERIOD;
	} else if(metronomePeriod + increment < SHORTEST_PERIOD) {
		metronomePeriod = SHORTEST_PERIOD;
	} else {
		metronomePeriod = metronomePeriod + increment;
	}
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

	char commandChar;
	setbuf(stdout, NULL);

	while(1) {
		commandChar = debug_getc();
		process_command(commandChar);
		HAL_Delay(125);
	}
}
