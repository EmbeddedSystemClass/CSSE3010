/**
 ******************************************************************************
 * @file    mylib/s4435360_hal_dac.h
 * @author  Samuel Eadie - 44353607
 * @date    20052018
 * @brief   DAC peripheral driver
 *
 ******************************************************************************
 *     EXTERNAL FUNCTIONS
 ******************************************************************************
 */

#ifndef S4435360_DAC_H
#define S4435360_DAC_H

#include "stm32f4xx_hal_conf.h"
#include "board.h"
#include "stm32f4xx_hal_dac.h"

//DAC Channel X Definitions
#define DAC_X_GPIO_CLK()                        __BRD_D24_GPIO_CLK()
#define DAC_X_DATA_GPIO_PORT                    BRD_D24_GPIO_PORT
#define DAC_X_DATA_GPIO_PIN                     BRD_D24_PIN
#define DAC_CHANNEL_X							DAC_CHANNEL_1

//DAC Channel Y Definitions
#define DAC_Y_GPIO_CLK()                        __BRD_D13_GPIO_CLK()
#define DAC_Y_DATA_GPIO_PORT                    BRD_D13_GPIO_PORT
#define DAC_Y_DATA_GPIO_PIN                     BRD_D13_PIN
#define DAC_CHANNEL_Y							DAC_CHANNEL_2

//DAC sequence definitions
#define DAC_SEQUENCE_ON							1
#define DAC_SEQUENCE_OFF						0

//DAC config variables
DAC_HandleTypeDef dacHandle;
DAC_ChannelConfTypeDef dacChannelConfig;

//DAC sequence variables
int* dacXSequence;
int* dacYSequence;
int dacXSequenceLength;
int dacYSequenceLength;
TIM_HandleTypeDef dacTimInit;

extern void s4435360_hal_dac_init(void);

#define s4435360_hal_dac_x_write(value) 	HAL_DAC_SetValue(&dacHandle, DAC_CHANNEL_X, DAC_ALIGN_12B_R, value)
#define s4435360_hal_dac_y_write(value)		HAL_DAC_SetValue(&dacHandle, DAC_CHANNEL_Y, DAC_ALIGN_12B_R, value)

#define s4435360_hal_dac_sequence_on() 		HAL_TIM_Base_Start_IT(&dacTimInit);
#define s4435360_hal_dac_sequence_off()		HAL_TIM_Base_Stop_IT(&dacTimInit);

#endif

