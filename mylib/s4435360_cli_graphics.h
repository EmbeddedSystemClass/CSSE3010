/**
 ******************************************************************************
 * @file    mylib/s4435360_cli_graphics.h
 * @author  Samuel Eadie - 44353607
 * @date    24052018
 * @brief   Provides CLI commands for graphics
 *
 ******************************************************************************
 *     EXTERNAL FUNCTIONS
 ******************************************************************************

 ******************************************************************************
 */

#ifndef S4435360_CLI_GRAPHICS_H
#define S4435360_CLI_GRAPHICS_H

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

void register_graphics_CLI_commands(void);

#endif

