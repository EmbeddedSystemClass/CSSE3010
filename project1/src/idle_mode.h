/*
 * idle_mode.h
 *
 *  Created on: 27Mar.,2018
 *      Author: Sam Eadie
 */

#ifndef IDLE_MODE_H_
#define IDLE_MODE_H_

#include "structures.h"

void idle_init(void);
void idle_deinit(void);
void idle_run(void);
void idle_user_input(char input);

ModeFunctions idleModeFunctions = {.modeID = 0x00,
									.init = &idle_init,
									.deinit = &idle_deinit,
									.run = &idle_run,
									.userInput = &idle_user_input};

#endif /* IDLE_MODE_H_ */
