/**
 ******************************************************************************
 * @file    mylib/sxxxxxxx_ledbar.h
 * @author  MyName – MyStudent ID
 * @date    03032015
 * @brief   LED Light Bar peripheral driver
 *	     REFERENCE: LEDLightBar_datasheet.pdf
 *
 *			NOTE: REPLACE sxxxxxxx with your student login.
 ******************************************************************************
 *     EXTERNAL FUNCTIONS
 ******************************************************************************
 * sxxxxxx_ledbar_init() – intialise LED Light BAR
 * sxxxxxx_ledbar_write() – set LED Light BAR value
 ******************************************************************************
 */

#ifndef S4435360_LIGHTBAR_H
#define S4435360_LIGHTBAR_H

/* Includes ------------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* External function prototypes -----------------------------------------------*/

extern void s4435360_lightbar_init(void);

extern void s4435360_lightbar_deinit(void);

extern void s4435360_lightbar_write(unsigned short value);

#endif

