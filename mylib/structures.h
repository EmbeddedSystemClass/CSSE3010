/*
 * structures.h
 *
 *  Created on: 27Mar.,2018
 *      Author: Sam Eadie
 */

#ifndef STRUCTURES_H_
#define STRUCTURES_H_

#include "board.h"
#include "stm32f4xx_hal.h"
#include "debug_printf.h"

#define IDLE_CHAR						'1'
#define PANTILT_TERMINAL_CHAR			'2'
#define PANTILT_JOYSTICK_CHAR			'3'
#define ENCODE_DECODE_CHAR				'4'
#define IR_DUPLEX_CHAR					'5'
#define HAMMING_ENCODE_DECODE_CHAR		'6'
#define RADIO_DUPLEX_CHAR				'7'
#define INTEGRATION_DUPLEX_CHAR			'8'
#define INTEGRATION_SPEED_CHAR 			'9'
#define HELP_CHAR						'?'

#define HANDLED_USER_INPUT			1
#define UNHANDLED_USER_INPUT		0

#define HEARTBEAT_PERIOD 			5
#define HEARTBEAT_SEGMENT			0
#define MODE_ID_SEGMENT				1

#define ENTER_CHAR					(char)(13)
#define BACKSPACE_CHAR				(char)(8)
#define SPACE_CHAR					(char)(32)
#define STX_CHAR 					(char)(0x02)
#define ETX_CHAR					(char)(0x03)
#define ACK_CHAR					(char)(0x06)
#define NACK_CHAR					(char)(0x15)
#define ESCAPE_CHAR 				(char)(27)
#define QUESTION_CHAR				(char)(63)
#define MAX_USER_CHARS				13

#define TIMER1						TIM5
#define TIMER2						TIM7
#define TIMER3						TIM6
#define TIMER1_IRQ					TIM5_IRQn
#define TIMER2_IRQ					TIM7_IRQn
#define TIMER3_IRQ					TIM6_DAC_IRQn
#define TIMER1_AF					GPIO_AF2_TIM5
#define __TIMER1_CLK_ENABLE() 		__TIM5_CLK_ENABLE()
#define __TIMER2_CLK_ENABLE()		__TIM7_CLK_ENABLE()
#define __TIMER3_CLK_ENABLE()		__TIM6_CLK_ENABLE()
#define TIMER1_HANDLER				TIM5_IRQHandler
#define TIMER2_HANDLER				TIM7_IRQHandler
#define TIMER3_HANDLER				TIM6_DAC_IRQHandler
TIM_HandleTypeDef timer1Init, timer2Init, timer3Init;

typedef struct {

	uint8_t modeID;
	void (*init)(void);
	void (*deinit)(void);
	void (*run)(void);
	void (*userInput)(char* userChars, int userCharsReceived);
	void (*timer1Handler)(void);
	void (*timer2Handler)(void);
	void (*timer3Handler)(void);

} ModeFunctions;

ModeFunctions currentModeFunctions;

#endif /* STRUCTURES_H_ */
