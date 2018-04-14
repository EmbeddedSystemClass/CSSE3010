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

#define LIGHTBAR_PIN0 BRD_D43_PIN
#define LIGHTBAR_PIN0_PORT BRD_D43_GPIO_PORT
#define LIGHTBAR_PIN1 BRD_D44_PIN
#define LIGHTBAR_PIN1_PORT BRD_D44_GPIO_PORT
#define LIGHTBAR_PIN2 BRD_D45_PIN
#define LIGHTBAR_PIN2_PORT BRD_D45_GPIO_PORT
#define LIGHTBAR_PIN3 BRD_D46_PIN
#define LIGHTBAR_PIN3_PORT BRD_D46_GPIO_PORT
#define LIGHTBAR_PIN4 BRD_D47_PIN
#define LIGHTBAR_PIN4_PORT BRD_D47_GPIO_PORT
#define LIGHTBAR_PIN5 BRD_D48_PIN
#define LIGHTBAR_PIN5_PORT BRD_D48_GPIO_PORT
#define LIGHTBAR_PIN6 BRD_D49_PIN
#define LIGHTBAR_PIN6_PORT BRD_D49_GPIO_PORT
#define LIGHTBAR_PIN7 BRD_D50_PIN
#define LIGHTBAR_PIN7_PORT BRD_D50_GPIO_PORT
#define LIGHTBAR_PIN8 BRD_D51_PIN
#define LIGHTBAR_PIN8_PORT BRD_D51_GPIO_PORT
#define LIGHTBAR_PIN9 BRD_D52_PIN
#define LIGHTBAR_PIN9_PORT BRD_D52_GPIO_PORT

/* External function prototypes -----------------------------------------------*/

void s4435360_lightbar_init(void);

void s4435360_lightbar_write(unsigned short value);
void lightbar_seg_set(int segment, unsigned char segment_value);

#endif

