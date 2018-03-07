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
#include "s4435360_hal_pantilt.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define RIGHT_CHAR 'p'
#define LEFT_CHAR 'n'
#define METRONOME_CHAR 'm'
#define FASTER_CHAR '-'
#define SLOWER_CHAR '+'
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
int inMetronomeMode = 0; //Boolean
int metronomePeriod = 10; //in seconds
int metronomeDirection = METRONOME_UPDATE_TIME; //Increment amount per interrupt
int metronomeCount = 1; //Counter for metronome period
TIM_HandleTypeDef TIM_Init;
/* Private function prototypes -----------------------------------------------*/
void metronome_timer_init();
int angle_to_lightbar_index(int angle);
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
void TIM2_IRQHandler(void);
void move_needle(int increment);
void change_period(int increment);
void process_command(char command);
/* Private functions ---------------------------------------------------------*/
/**
 * @brief  Initialises TIM2 for the metronome
 * @param  None
 * @retval None
 */
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

/**
 * @brief  Converts a PAN angle to the corresponding lightbar index
 * @param  PAN angle
 * @retval lightbar index
 */
int angle_to_lightbar_index(int angle) {

	/* Calculate lightbar index */
	int ledPos = ((angle / 14) + 5);

	/* Check edge cases */
	if(ledPos > 9) {
		ledPos = 9;
	} else if (ledPos < 0) {
		ledPos = 0;
	}

	return ledPos;
}

/**
 * @brief Callback function for metronome timer interrupts
 * @param htim: Pointer to a TIM_HandleTypeDef that contains the configuration information for the TIM module.
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {

	/* Timer only in metronomeMode */
	if(inMetronomeMode) {

		/* Switch metronome direction if at edge */
		if((metronomeCount >= metronomePeriod * 1000) ||
				(metronomeCount <= 0)) {
			metronomeDirection *= -1;
		}

		metronomeCount += metronomeDirection;

		/* Write new angle to servo and lightbar */
		int angle = (80.0 * ((float)metronomeCount / (float)metronomePeriod / 1000.0)) - 40;
		s4435360_hal_pantilt_pan_write(angle);
		s4435360_lightbar_write(1 << angle_to_lightbar_index(angle));
	}
}

/**
 * @brief  Metronome timer handler
 * @param  None
 * @retval None
 */
void TIM2_IRQHandler(void) {
	HAL_TIM_IRQHandler(&TIM_Init);

}

/**
 * @brief  Increments the PAN servo angle
 * @param  increment
 * @retval None
 */
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

	/* Write new angle to servo and lightbar */
	s4435360_hal_pantilt_pan_write(newAngle);
	s4435360_lightbar_write(1 << angle_to_lightbar_index(newAngle));
}

/**
 * @brief  Increments the metronomes period
 * @param  increment
 * @retval None
 */
void change_period(int increment) {
	metronomePeriod += increment;

	/* Check edge cases */
	if(metronomePeriod > LONGEST_PERIOD) {
		metronomePeriod = LONGEST_PERIOD;
	} else if (metronomePeriod < SHORTEST_PERIOD) {
		metronomePeriod = SHORTEST_PERIOD;
	}

	/* Write new angle to servo and lightbar */
	int currentAngle = s4435360_hal_pantilt_pan_read();
	metronomeCount = ((currentAngle + 40) / 80.0) * metronomePeriod * 1000;
}

/**
 * @brief  Processes a user command from the console
 * @param  command character
 * @retval None
 */
void process_command(char command) {

	/* Swith on user input */
	switch(command) {

		case RIGHT_CHAR:

			/* Move needle if not in metronome mode */
			if(!inMetronomeMode) {
				move_needle(RIGHT_INCREMENT);
			}
			break;

		case LEFT_CHAR:

			/* Check for usage */
			if(inMetronomeMode) {
				inMetronomeMode = 0;
			} else {
				move_needle(LEFT_INCREMENT);
			}
			break;

		case METRONOME_CHAR:

			/* Enter metronome mode */
			inMetronomeMode = 1;
			break;

		case SLOWER_CHAR:

			/* Decrement metronome period */
			if(inMetronomeMode) {
				change_period(SLOWER_INCREMENT);
			}
			break;

		case FASTER_CHAR:

			/* Increment metronome period */
			if(inMetronomeMode) {
				change_period(FASTER_INCREMENT);
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

	/* Main loop, process user input */
	while(1) {

		commandChar = debug_getc();
		process_command(commandChar);
		HAL_Delay(250);

	}
}
