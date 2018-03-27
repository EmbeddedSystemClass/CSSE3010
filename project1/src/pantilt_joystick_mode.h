/*
 * pantilt_joystick_mode.h
 *
 *  Created on: 27Mar.,2018
 *      Author: Sam Eadie
 */

#ifndef PANTILT_JOYSTICK_MODE_H_
#define PANTILT_JOYSTICK_MODE_H_

#include "structures.h"

void pantilt_joystick_init(void);
void pantilt_joystick_deinit(void);
void pantilt_joystick_run(void);
void pantilt_joystick_user_input(char input);

ModeFunctions pantiltJoystickModeFunctions = {.modeID = 0x02,
												.init = &pantilt_joystick_init,
												.deinit = &pantilt_joystick_deinit,
												.run = &pantilt_joystick_run,
												.userInput = &pantilt_joystick_user_input};

#endif
