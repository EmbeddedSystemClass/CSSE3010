/***
 ******************************************************************************
 * @file    mylib/s4435360_hal_sysmon.h
 * @author  Samuel Eadie - 44353607
 * @date    27042018
 * @brief   Provides system monitoring capabilities
 ******************************************************************************
 */

#ifndef S4435360_HAL_SYSMON_H
#define S4435360_HAL_SYSMON_H

//Channel definitions
#define CHANNEL_0_PIN							BRD_D10_PIN
#define CHANNEL_1_PIN							BRD_D11_PIN
#define CHANNEL_2_PIN							BRD_D12_PIN
#define CHANNEL_0_PORT							BRD_D10_GPIO_PORT
#define CHANNEL_1_PORT							BRD_D11_GPIO_PORT
#define CHANNEL_2_PORT							BRD_D12_GPIO_PORT
#define CHANNEL_0_PORT_CLK()					__BRD_D10_GPIO_CLK()
#define CHANNEL_1_PORT_CLK()					__BRD_D11_GPIO_CLK()
#define CHANNEL_2_PORT_CLK()					__BRD_D12_GPIO_CLK()

//Set and clear channel functionality
#define s4435360_hal_sysmon_chan0_clr()			HAL_GPIO_WritePin(CHANNEL_0_PORT, CHANNEL_0_PIN, 0);
#define s4435360_hal_sysmon_chan0_set()			HAL_GPIO_WritePin(CHANNEL_0_PORT, CHANNEL_0_PIN, 1);
#define s4435360_hal_sysmon_chan1_clr()			HAL_GPIO_WritePin(CHANNEL_1_PORT, CHANNEL_1_PIN, 0);
#define s4435360_hal_sysmon_chan1_set()			HAL_GPIO_WritePin(CHANNEL_1_PORT, CHANNEL_1_PIN, 1);
#define s4435360_hal_sysmon_chan2_clr()			HAL_GPIO_WritePin(CHANNEL_2_PORT, CHANNEL_2_PIN, 0);
#define s4435360_hal_sysmon_chan2_set()			HAL_GPIO_WritePin(CHANNEL_2_PORT, CHANNEL_2_PIN, 1);

//System monitoring functionality
extern void s4435360_hal_sysmon_init(void);

#endif
