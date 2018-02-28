/**
 ******************************************************************************
 * @file    mylib/s4435360_ledbar.h
 * @author  Samuel Eadie - 44353607
 * @date    21022018-28022018
 * @brief   LED Light Bar peripheral driver
 *	     REFERENCE: LEDLightBar_datasheet.pdf
 *
 ******************************************************************************
 *     EXTERNAL FUNCTIONS
 ******************************************************************************
 * s4435360_ledbar_init() – intialise LED Light BAR
 * s4435360_ledbar_write() – set LED Light BAR value
 ******************************************************************************
 */

#ifndef S4435360_LIGHTBAR_H
#define S4435360_LIGHTBAR_H

#define LIGHTBAR_PIN0 BRD_D16_PIN
#define LIGHTBAR_PIN0_PORT BRD_D16_GPIO_PORT
#define LIGHTBAR_PIN1 BRD_D17_PIN
#define LIGHTBAR_PIN1_PORT BRD_D17_GPIO_PORT
#define LIGHTBAR_PIN2 BRD_D18_PIN
#define LIGHTBAR_PIN2_PORT BRD_D18_GPIO_PORT
#define LIGHTBAR_PIN3 BRD_D19_PIN
#define LIGHTBAR_PIN3_PORT BRD_D19_GPIO_PORT
#define LIGHTBAR_PIN4 BRD_D20_PIN
#define LIGHTBAR_PIN4_PORT BRD_D20_GPIO_PORT
#define LIGHTBAR_PIN5 BRD_D21_PIN
#define LIGHTBAR_PIN5_PORT BRD_D21_GPIO_PORT
#define LIGHTBAR_PIN6 BRD_D22_PIN
#define LIGHTBAR_PIN6_PORT BRD_D22_GPIO_PORT
#define LIGHTBAR_PIN7 BRD_D23_PIN
#define LIGHTBAR_PIN7_PORT BRD_D23_GPIO_PORT
#define LIGHTBAR_PIN8 BRD_D24_PIN
#define LIGHTBAR_PIN8_PORT BRD_D24_GPIO_PORT
#define LIGHTBAR_PIN9 BRD_D25_PIN
#define LIGHTBAR_PIN9_PORT BRD_D25_GPIO_PORT

/* External function prototypes -----------------------------------------------*/

void s4435360_lightbar_init(void);

void s4435360_lightbar_write(unsigned short value);

#endif

