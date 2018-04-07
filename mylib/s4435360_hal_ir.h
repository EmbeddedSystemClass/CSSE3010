/***
 ******************************************************************************
 * @file    mylib/s4435360_hal_ir.h
 * @author  Samuel Eadie - 44353607
 * @date    07032018-14032018
 * @brief   IR functionality: transmit, receive
 ******************************************************************************
 */

#ifndef S4435360_HAL_IR_H
#define S4435360_HAL_IR_H

#include "stm32f4xx_hal_conf.h"
#include "board.h"

/* States for irhal_carrier */
#define CARRIER_ON 1
#define CARRIER_OFF 0

/* Carrier wave parameters */
#define CARRIER_TIMER_FREQUENCY 37900 //Frequency of carrier wave
#define CARRIER_CHANNEL 		TIM_CHANNEL_4 //Timer channel for carrier wave
#define CARRIER_PORT			BRD_D38_GPIO_PORT //Port to output carrier wave
#define CARRIER_PIN				BRD_D38_PIN //Pin to output carrier wave
#define CARRIER_PORT_CLK()		__BRD_D38_GPIO_CLK() //Clock enable function for CARRIER_PIN
#define CARRIER_TIMER			TIM1 //Timer for carrier wave
#define CARRIER_GPIO_AF			GPIO_AF1_TIM1
#define CARRIER_TIM_CLK()		__TIM1_CLK_ENABLE() //Clock enable for carrier wave timer
#define CARRIER_TIMER_HANDLER	carrierTimInit //Timer handler for carrier wave timer

/* Modulation signal parameters */
#define MODULATION_PORT		BRD_D37_GPIO_PORT //Port to output modulation signal
#define MODULATION_PIN		BRD_D37_PIN //Pin to output modulation signal
#define __MODULATION_CLK_ENABLE()	__BRD_D37_GPIO_CLK() //Clock enable function for modulation signal output pin

/* Input capture parameters */
#define __INPUT_CAPTURE_CLK_ENABLE() 	__TIM2_CLK_ENABLE()
#define __RX_PIN_CLK_ENABLE()			__BRD_D35_GPIO_CLK()
#define RX_INPUT_PIN					BRD_D35_PIN
#define RX_INPUT_PORT					BRD_D35_GPIO_PORT
#define INPUT_GPIO_AF					GPIO_AF1_TIM2
#define RX_TIM							TIM2
#define RX_TIM_CHANNEL					TIM_CHANNEL_4


void irhal_carrier(int state);

/**
 * @brief Turns the carrier wave on
 * @param None
 * @retval None
 */
#define s4435360_hal_ir_carrier_on() irhal_carrier(CARRIER_ON)

/**
 * @brief Turns the carrier wave off
 * @param None
 * @retval None
 */
#define s4435360_hal_ir_carrier_off() irhal_carrier(CARRIER_OFF)

/**
 * @brief Sets the data modulation signal to 1
 * @param None
 * @retval None
 */
#define s4435360_hal_ir_datamodulation_set() HAL_GPIO_WritePin(MODULATION_PORT, MODULATION_PIN, 1)

/**
 * @brief Clears the data modulation signal to 0
 * @param None
 * @retval None
 */
#define s4435360_hal_ir_datamodulation_clr() HAL_GPIO_WritePin(MODULATION_PORT, MODULATION_PIN, 0)

void s4435360_hal_ir_init(void);


#endif
