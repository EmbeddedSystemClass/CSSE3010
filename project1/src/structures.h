/*
 * structures.h
 *
 *  Created on: 27Mar.,2018
 *      Author: Sam Eadie
 */

#ifndef STRUCTURES_H_
#define STRUCTURES_H_

#define IDLE_CHAR					'1'
#define PANTILT_TERMINAL_CHAR		'2'
#define PANTILT_JOYSTICK_CHAR		'3'
#define ENCODE_DECODE_CHAR			'4'
#define IR_DUPLEX_CHAR				'5'
#define RADIO_DUPLEX_CHAR			'6'
#define INTEGRATION_CHAR			'7'
#define HELP_CHAR					'?'

#define HANDLED_USER_INPUT			1
#define UNHANDLED_USER_INPUT		0

typedef struct {

	uint8_t modeID;
	void (*init)(void);
	void (*deinit)(void);
	void (*run)(void);
	void (*userInput)(char);

} ModeFunctions;


#endif /* STRUCTURES_H_ */
