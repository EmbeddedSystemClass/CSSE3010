/**
 ******************************************************************************
 * @file    mylib/s4435360_os_ir.h
 * @author  Samuel Eadie - 44353607
 * @date    26052018
 * @brief   Provides IR NEC receive functionality for remote
 *
 ******************************************************************************
 *     EXTERNAL FUNCTIONS
 ******************************************************************************
 */

#ifndef S4435360_OS_IR_H
#define S4435360_OS_IR_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

void s4435360_IRTask(void);


#endif

