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


#define CARRIER_ON 1
#define CARRIER_OFF 0

#define CARRIER_TIMER_FREQUENCY 37900
#define CARRIER_CHANNEL 		TIM_CHANNEL_1
#define CARRIER_PORT			BRD_D29_GPIO_PORT
#define CARRIER_PIN				BRD_D29_PIN
#define CARRIER_PORT_CLK()		__BRD_D29_GPIO_CLK()
#define CARRIER_TIMER			TIM4
#define CARRIER_GPIO_AF			GPIO_AF2_TIM4
#define CARRIER_TIM_CLK()		__TIM4_CLK_ENABLE()
#define CARRIER_TIMER_HANDLER	carrierTimInit

#define MODULATION_PORT		BRD_D30_GPIO_PORT
#define MODULATION_PIN		BRD_D30_PIN

#define __MODULATION_CLK_ENABLE()	__BRD_D30_GPIO_CLK()

void irhal_carrier(int state);

#define s4435360_hal_ir_carrier_on() irhal_carrier(CARRIER_ON)
#define s4435360_hal_ir_carrier_off() irhal_carrier(CARRIER_OFF)

#define s4435360_hal_ir_datamodulation_set() HAL_GPIO_WritePin(MODULATION_PORT, MODULATION_PIN, 1)
#define s4435360_hal_ir_datamodulation_clr() HAL_GPIO_WritePin(MODULATION_PORT, MODULATION_PIN, 0)

void s4435360_hal_ir_init(void);


#endif
