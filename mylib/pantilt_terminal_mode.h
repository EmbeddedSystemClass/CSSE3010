/*
 * pantilt_terminal_mode.h
 *
 *  Created on: 27Mar.,2018
 *      Author: Sam Eadie
 */

#ifndef PANTILT_TERMINAL_MODE_H_
#define PANTILT_TERMINAL_MODE_H_

#include "structures.h"

void pantilt_terminal_init(void);
void pantilt_terminal_deinit(void);
void pantilt_terminal_run(void);
void pantilt_terminal_user_input(char input);

#endif