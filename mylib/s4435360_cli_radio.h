/**
 ******************************************************************************
 * @file    mylib/s4435360_cli_radio.h
 * @author  Samuel Eadie - 44353607
 * @date    14052018
 * @brief   Provides CLI commands for radio
 *
 ******************************************************************************
 *     EXTERNAL FUNCTIONS
 ******************************************************************************

 ******************************************************************************
 */

#ifndef S4435360_CLI_RADIO_H
#define S4435360_CLI_RADIO_H

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

void register_radio_CLI_commands(void);

#endif

