/**
 ******************************************************************************
 * @file    mylib/s4435360_os_control.c
 * @author  Samuel Eadie - 44353607
 * @date    24052018
 * @brief   Controller for project 2
 *
 ******************************************************************************
 *     EXTERNAL FUNCTIONS
 ******************************************************************************
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <s4435360_os_control.h>

#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "board.h"
#include "debug_printf.h"
#include "s4435360_os_printf.h"
#include "s4435360_cli_graphics.h"
#include "s4435360_os_radio.h"
#include <math.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define EVER						;;
#define COMMAND_QUEUE_LENGTH			10
#define PI				3.14159265359
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
void move_straight(int x, int y, char type, int length) {
	if((type != 'h') && (type != 'v')) {
		return;
	}

	//Send starting position
	send_Z_message(DEFAULT_UP_Z_VALUE, portMAX_DELAY);
	send_XYZ_message(x, y, DEFAULT_DOWN_Z_VALUE, portMAX_DELAY);

	//Horizontal line
	if(type == 'h') {
		send_XY_message(x + length, y, portMAX_DELAY);

		//Vertical Line
	} else {
		send_XY_message(x, y + length, portMAX_DELAY);
	}
}

void move_square(int x, int y, int side) {
	//Move to starting position
	send_Z_message(DEFAULT_UP_Z_VALUE, portMAX_DELAY);
	send_XYZ_message(x, y, DEFAULT_DOWN_Z_VALUE, portMAX_DELAY);

	//Move to diagonal
	send_XY_message(x + side, y + side, portMAX_DELAY);

	//Return to start
	send_XYZ_message(x, y, DEFAULT_UP_Z_VALUE, portMAX_DELAY);

}


void bline_low(int x1, int y1, int x2, int y2, int stepSize) {
	int dx = x2 - x1;
	int dy = y2 - y1 > 0 ? y2 - y1 : y1 - y2;
	int deltay = y2 - y1 > 0 ? 1 : -1;
	int xi = x1;
	int yi = y1;
	int D = (2 * dy) - dx;

	for(int i = 0; i < dx; i++) {
		if(!(i % stepSize)) {
			send_XYZ_message(xi, yi, DEFAULT_DOWN_Z_VALUE, portMAX_DELAY);
		}

		if(D > 0) {
			yi += deltay;
			D -= (2 * dx);
		}

		D += (2 * dy);
		xi += 1;
	}

	send_XYZ_message(x2, y2, DEFAULT_UP_Z_VALUE, portMAX_DELAY);
}

void bline_high(int x1, int y1, int x2, int y2, int stepSize) {
	int dx = x2 - x1 > 0 ? x2 - x1 : x1 - x2;
	int dy = y2 - y1;
	int deltax = x2 - x1 > 0 ? 1 : -1;
	int xi = x1;
	int yi = y1;
	int D = (2 * dx) - dy;

	for(int i = 0; i < dy; i++) {
		if(!(i % stepSize)) {
			send_XYZ_message(xi, yi, DEFAULT_DOWN_Z_VALUE, portMAX_DELAY);
		}

		if(D > 0) {
			xi += deltax;
			D -= (2 * dy);
		}

		D += (2 * dx);
		yi+=1;
	}

	send_XYZ_message(x2, y2, DEFAULT_UP_Z_VALUE, portMAX_DELAY);
}

void bresenham_line(int x1, int y1, int x2, int y2, int stepSize) {
	if(((y2 - y1)*(y2 - y1)) < ((x2 - x1)*(x2 - x1))) {
		if(x1 > x2) {
			myprintf("1\r\n");
			bline_low(x2, y2, x1, y1, stepSize);
		} else {
			myprintf("2\r\n");
			bline_low(x1, y1, x2, y2, stepSize);
		}
	} else {
		if(y1 > y2) {
			myprintf("3\r\n");
			bline_high(x2, y2, x1, y1, stepSize);
		} else {
			myprintf("4\r\n");
			bline_high(x1, y1, x2, y2, stepSize);
		}
	}

}

void n_polygon(int n, int x1, int y1, int radius) {
	int x[n];
	int y[n];
	float thetaOffset = ((n - 2) * PI) / 2; //Straighten bottom side
	float centeringOffsetX = radius * cos(thetaOffset / 2);
	float centeringOffsetY = radius * sin(thetaOffset / 2);

	myprintf("%f, %f\r\n", centeringOffsetX, centeringOffsetY);

	//Calculate points in polygon
	for(int i = 0; i < n; i++) {
		x[i] = radius * cos(((2 * PI * i) - thetaOffset) / n) + x1 + centeringOffsetX;
		y[i] = radius * sin(((2 * PI * i) - thetaOffset) / n) + y1 + centeringOffsetY;
		myprintf("(%d, %d)\r\n", x[i], y[i]);

		//Check calculated points are on 200x200 board
		if((x[i] > 200) || (x[i] < 0) || (y[i] > 200) || (y[i] < 0)) {
			myprintf("Entered polygon exceeds board dimensions");
			return;
		}
	}

	//Use Bresenham to draw line between points
	for(int i = 0; i < n - 1; i++) {
		bresenham_line(x[i], y[i], x[i+1], y[i+1], radius / 10);
	}

	//Return to start
	bresenham_line(x[n-1], y[n-1], x[0], y[0], radius / 10);
}

void compass_rose(int x1, int y1, int sideLength, int increment) {
	int x[6] = {sideLength, 1.5 * sideLength, sideLength, 0, -0.5 * sideLength, 0};
	int y[6] = {0, 0.866 * sideLength, 1.732 * sideLength, 1.732 * sideLength, 0.866 * sideLength, 0};
	int startAngle[6] = {60, 120, 180, 240, 300, 0};
	int endAngle[6] = {180, 240, 300, 360, 420, 120};

	//Check points in polygon are on 200x200
	for(int i = 0; i < 6; i++) {
		myprintf("(%d, %d)\r\n", x1 + x[i], y1 + y[i]);
		if((x1 + x[i] > 200) || (x1 + x[i] < 0) || (y1 + y[i] > 200) || (y1 + y[i] < 0)) {
			myprintf("Entered compass roses exceeds board dimensions");
			return;
		}
	}

	//For each corner of hexagon
	for(int i = 0; i < 6; i++) {
		send_Z_message(DEFAULT_UP_Z_VALUE, portMAX_DELAY);

		int xj, yj;
		//Draw arc segment
		for(int j = startAngle[i]; j <= endAngle[i]; j += increment) {
			xj = (sideLength * cos((PI * j) / 180)) + x[i] + x1;
			yj = (sideLength * sin((PI * j) / 180)) + y[i] + y1;

			send_XYZ_message(xj, yj, DEFAULT_DOWN_Z_VALUE, portMAX_DELAY);
		}
	}
}

void s4435360_TaskControl(void) {

	s4435360_QueueCommands = xQueueCreate(COMMAND_QUEUE_LENGTH, sizeof(Command));
	register_graphics_CLI_commands();

	Command command;

	for(EVER) {
		if(xQueueReceive(s4435360_QueueCommands, &command, portMAX_DELAY)) {
			myprintf("%d\r\n", command.type);
			switch(command.type) {
				case origin:
					send_XYZ_message(0, 0, 0, portMAX_DELAY);
					break;

				case line:
					send_join_message(portMAX_DELAY);
					move_straight(command.args[0], command.args[1],
							command.args[2], command.args[3]);
					break;

				case square:
					send_join_message(portMAX_DELAY);
					move_square(command.args[0], command.args[1], command.args[2]);
					break;

				case bline:
					send_join_message(portMAX_DELAY);
					send_Z_message(DEFAULT_UP_Z_VALUE, portMAX_DELAY);
					bresenham_line(command.args[0], command.args[1],
							command.args[2], command.args[3], command.args[4]);
					break;

				case polygon:
					send_join_message(portMAX_DELAY);
					send_Z_message(DEFAULT_UP_Z_VALUE, portMAX_DELAY);
					n_polygon(command.args[0], command.args[1],
							command.args[2], command.args[3]);
					break;

				case rose:
					send_join_message(portMAX_DELAY);
					compass_rose(command.args[0], command.args[1],
							command.args[2], command.args[3]);
					break;

				default:
					myprintf("Received unrecognisable command.\r\n");
					break;

			}
		}
	}
}
