/**
 ******************************************************************************
 * @file    mylib/s4435360_os_dac.c
 * @author  Samuel Eadie - 44353607
 * @date    22052018
 * @brief   Provides OS functionality for DAC
 *
 ******************************************************************************
 *     EXTERNAL FUNCTIONS
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <s4435360_os_ir.h>

#include "s4435360_hal_ir.h"
#include "s4435360_os_printf.h"
#include "s4435360_os_radio.h"
#include "stm32f4xx_hal_conf.h"
#include "board.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define EVER		;;
#define POWER_BUTTON 81
#define MENU_BUTTON 113
#define TEST_BUTTON 17
#define PLUS_BUTTON 1
#define BACK_BUTTON 97
#define LEFT_BUTTON 112
#define RIGHT_BUTTON 72
#define PLAY_BUTTON 84
#define MINUS_BUTTON 76
#define C_BUTTON 88
#define ZERO_BUTTON 52
#define ONE_BUTTON 102
#define TWO_BUTTON 12
#define THREE_BUTTON 61
#define FOUR_BUTTON 8
#define FIVE_BUTTON 28
#define SIX_BUTTON 45
#define SEVEN_BUTTON 33
#define EIGHT_BUTTON 37
#define NINE_BUTTON 41

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/


void s4435360_IRTask(void) {

	s4435360_hal_ir_rx_init();

	for(EVER) {
		if(receivedIRFlag) {
			myprintf("Received %d from IR\r\n", irCommand);

			switch(irCommand) {
			case PLUS_BUTTON:
				send_Y_increment_message(10, portMAX_DELAY);
				myprintf("Moved up from IR command\r\n");
				break;

			case MINUS_BUTTON:
				send_Y_increment_message(-10, portMAX_DELAY);
				myprintf("Moved down from IR command\r\n");
				break;

			case LEFT_BUTTON:
				send_X_increment_message(-10, portMAX_DELAY);
				myprintf("Moved left from IR command\r\n");
				break;

			case RIGHT_BUTTON:
				send_X_increment_message(10, portMAX_DELAY);
				myprintf("Moved right from IR command\r\n");
				break;

			default:
				break;
			}

			receivedIRFlag = 0;
			irCommand = 0;
		}

		vTaskDelay(500);
	}
}

