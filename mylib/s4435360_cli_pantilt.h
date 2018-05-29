/**
 ******************************************************************************
 * @file    mylib/s4435360_cli_pantilt.h
 * @author  Samuel Eadie - 44353607
 * @date    14052018
 * @brief   Provides CLI commands for pantilt for testing
 *
 ******************************************************************************
 *     EXTERNAL FUNCTIONS
 ******************************************************************************

 ******************************************************************************
 */

#ifndef S4435360_CLI_PANTILT_H
#define S4435360_CLI_PANTILT_H

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

void register_pantilt_CLI_commands(void);

#endif
