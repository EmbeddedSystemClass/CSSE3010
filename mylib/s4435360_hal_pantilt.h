

/**
 ******************************************************************************
 * @file    mylib/s4435360_hal_pantilt.h
 * @author  Samuel Eadie - 44353607
 * @date    28022018-07032018
 * @brief   Pan/tilt servo peripheral driver
 *
 ******************************************************************************
 *     EXTERNAL FUNCTIONS
 ******************************************************************************
 **********
 ******************************************************************************
 */

#ifndef S4435360_PANTILT_H
#define S4435360_PANTILT_H

#define PAN 0
#define TILT 1

void pantilt_angle_write(int type, int angle);

int pantilt_angle_read(int type);

void s4435360_hal_pantilt_init(void);

#define s4435360_hal_pantilt_pan_write(angle) pantilt_angle_write(PAN, angle)
#define s4435360_hal_pantilt_pan_read() pantilt_angle_read(PAN)
#define s4435360_hal_pantilt_tilt_write(angle) pantilt_angle_write(TILT, angle)
#define s4435360_hal_pantilt_tilt_read() pantilt_angle_read(TILT)
#endif

