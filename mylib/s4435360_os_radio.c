/**
 ******************************************************************************
 * @file    mylib/s4435360_os_radio.c
 * @author  Samuel Eadie - 44353607
 * @date    09052018
 * @brief   FreeRTOS functionality to control radio HAL
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <s4435360_os_radio.h>

#include "nrf24l01plus.h"
#include "radio_fsm.h"
#include "s4435360_hal_radio.h"
#include "s4435360_hal_hamming.h"
#include "s4435360_os_printf.h"

#include <stdio.h>
#include "string.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"
#include "board.h"
#include "stm32f4xx_hal.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define EVER						;;
#define RADIO_QUEUE_LENGTH			10

#define ACKNOWLEDGEMENT_STACK_SIZE			( configMINIMAL_STACK_SIZE * 5 )
#define FSM_PROCESSING_STACK_SIZE			( configMINIMAL_STACK_SIZE * 5 )

#define ACKNOWLEDGEMENT_TASK_PRIORITY 		( tskIDLE_PRIORITY + 2 )
#define FSM_PROCESSING_TASK_PRIORITY		( tskIDLE_PRIORITY + 2 )

//Event group definitions for ack/err
#define EVT_ACK				1 << 0		//ACK received
#define EVT_ERR				1 << 1		//ERR received
#define ACKCTRL_EVENT		EVT_ACK | EVT_ERR	//Control Event Group Mask

#define PAYLOAD_STARTING_INDEX		10
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
SemaphoreHandle_t transmitSemaphore;
EventGroupHandle_t ackctrl_EventGroup;
RadioMessage retransmitMessage;
TaskHandle_t ackTask;

unsigned char txAddress[5] = {0x52, 0x33, 0x22, 0x11, 0x00};
unsigned char rxAddress[5] = {0x07, 0x36, 0x35, 0x44, 0x00};
unsigned char channel = 52;

unsigned char packetHeader[10] = {0xA1, 		//Packet Type
			0x59, 0x00, 0x00, 0x80, //0x52, 0x33, 0x22, 0x11,				//Source address
			0x07, 0x36, 0x35, 0x44,				//Destination address
			0x00};								//Blank char
int lastX = 0, lastY = 0, lastZ = 0;
/* Private function prototypes -----------------------------------------------*/
void FSMProcessing_Task(void);
void Acknowledgment_Task(void);
/* Private functions ---------------------------------------------------------*/

unsigned char* get_txAddress(void) {
	return txAddress;
}

unsigned char* get_rxAddress(void) {
	return rxAddress;
}

unsigned char get_chan(void) {
	return channel;
}

void set_rxAddress(unsigned char* addr) {
	memcpy((void*) rxAddress, (void*) addr, 4);
}

void set_txAddress(unsigned char* addr) {
	memcpy((void*) txAddress, (void*) addr, 4);
}

void set_chan(unsigned char chan) {
	channel = chan;
}
void send_radio_message(char* payload, int payloadLength, int retransmitAttempts, int waitTime) {
	RadioMessage message;

	message.retransmitAttempts = retransmitAttempts;
	message.payloadLength = payloadLength;
	memcpy((void*) message.payload, (void*) payload, payloadLength);

	xQueueSendToBack(txMessageQueue, ( void * ) &message, ( TickType_t ) waitTime);
}

void send_XYZ_message(int x, int y, int z, int waitTime) {
	char xyzPayload[11];
	sprintf(xyzPayload, "XYZ%03d%03d%02d", x, y, z);

	lastX = x;
	lastY = y;
	lastZ = z;

	send_radio_message(xyzPayload, 11, 0, waitTime);
}

void send_Z_message(int z, int waitTime) {
	send_XYZ_message(lastX, lastY, z, waitTime);
	lastZ = z;
}

void send_XY_message(int x, int y, int waitTime) {
	send_XYZ_message(x, y, lastZ, waitTime);
	lastX = x;
	lastY = y;
}

void send_join_message(int waitTime) {
	char* joinPayload = "JOIN";
	send_radio_message(joinPayload, 4, 0, waitTime);
}

void form_packet(char* payload, int payloadLength, char* packet) {

	/* Add packet header */
	//memcpy((void*) packet, (void*) packetHeader, PAYLOAD_STARTING_INDEX);
	packet[0] = 0xA1;
	memcpy((void*) &packet[1], (void*) txAddress, 4);
	memcpy((void*) &packet[5], (void*) rxAddress, 4);
	packet[9] = 0x00;

	/* Encode and add payload */
	uint16_t encodedByte;
	for(int i = 0; i < payloadLength; i++) {
		encodedByte = hamming_byte_encoder(payload[i]);
		packet[PAYLOAD_STARTING_INDEX + (2 * i)] = (uint8_t)((encodedByte & 0xFF00) >> 8);
		packet[PAYLOAD_STARTING_INDEX + (2 * i) + 1] = (uint8_t)(encodedByte & 0x00FF);
	}
}

extern void s4435360_TaskRadio(void) {

	//Initialise hardware
	s4435360_radio_init();
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
	radio_fsm_setstate(RADIO_FSM_IDLE_STATE);
	s4435360_radio_setrxaddress(rxAddress);
	s4435360_radio_settxaddress(txAddress);
	s4435360_radio_setchan(channel);
	s4435360_radio_txstatus = 0;
	s4435360_radio_rxstatus = 0;

	//xTaskCreate((void *) &FSMProcessing_Task, (const char *) "FSMPROCESSING",
	//					FSM_PROCESSING_STACK_SIZE, NULL, FSM_PROCESSING_TASK_PRIORITY, NULL);

	//Initialise radio messages queue
	txMessageQueue = xQueueCreate(RADIO_QUEUE_LENGTH, sizeof(RadioMessage));

	//Initialise transmit semaphore
	transmitSemaphore = xSemaphoreCreateBinary();
	xSemaphoreGive(transmitSemaphore); //Allow first transmission

	//Initialise reliability event groups
	ackctrl_EventGroup = xEventGroupCreate();

	RadioMessage toSend;

	for(EVER) {

		//Transmission
		if(xQueuePeek(txMessageQueue, &toSend, 10)) {
			//Check previous ack process has finished
			if(xSemaphoreTake(transmitSemaphore, 10)) {
				xQueueReceive(txMessageQueue, &toSend, 10);
				//Send payload message
				memset(&s4435360_tx_buffer[0], 0, 32);
				form_packet(toSend.payload, toSend.payloadLength, (char*)s4435360_tx_buffer);
				s4435360_radio_sendpacket(channel, txAddress, s4435360_tx_buffer);

				//Clear bits for new acknowledgment routine
				xEventGroupClearBits(ackctrl_EventGroup, EVT_ACK | EVT_ERR);

				vTaskDelay(1000);
				retransmitMessage = toSend;

				//Create task to check acknowledgment
				xTaskCreate((void *) &Acknowledgment_Task, (const char *) "ACK",
						ACKNOWLEDGEMENT_STACK_SIZE, NULL, ACKNOWLEDGEMENT_TASK_PRIORITY, &ackTask);
				//xSemaphoreGive(transmitSemaphore);

			}
		}

		//Received packet
		if(s4435360_radio_rxstatus) {
			//Get payload
			s4435360_radio_getpacket(s4435360_rx_buffer);
			char* payload = (char*)(&s4435360_rx_buffer[PAYLOAD_STARTING_INDEX]);

			//Decode payload
			char decodedPayload[11];
			memset(&decodedPayload[0], 0, 11);
			if(hamming_decode_payload(decodedPayload, payload, 11)) {
				if(!strncmp(decodedPayload, "A C K", 5)) {
					//Received ACK - give semaphore
					myprintf("Received ACK\r\n");
					xEventGroupSetBits(ackctrl_EventGroup, EVT_ACK);
				} else if(!strncmp(decodedPayload, "E R R", 5)) {
					myprintf("Received ERR\r\n");
					xEventGroupSetBits(ackctrl_EventGroup, EVT_ERR);
				} else {
					myprintf("Received other message: '%s'\r\n", decodedPayload);
				}

			//Received uncorrectable error
			} else {
				myprintf("Received message with uncorrectable error\r\n");
			}

			memset(&s4435360_rx_buffer[0], 0, sizeof(s4435360_rx_buffer));
			s4435360_radio_rxstatus = 0;
		}

		s4435360_radio_fsmprocessing();
		vTaskDelay(300);

	}
}

void Acknowledgment_Task(void) {
	//myprintf("ACK\r\n");
	//Block for ERR or ACK, 3 second timeout
	EventBits_t eventBits = xEventGroupWaitBits(ackctrl_EventGroup, EVT_ACK | EVT_ERR,	pdTRUE,	pdFALSE, 3000);

	vTaskDelay(1000);

	//ACK received in 3 seconds
	if((eventBits & EVT_ACK) != 0) {

	} else if ((eventBits & EVT_ERR) != 0) {
		//ERR received
		myprintf("Adjusting for ERR\r\n");
		retransmitMessage.retransmitAttempts = 0;
		xQueueSendToFront(txMessageQueue, (void*) &retransmitMessage, 100);

	} else {
		//ACK not received - retransmit
		if(retransmitMessage.retransmitAttempts < 2) {
			//myprintf("Retransmit message added to front of queue\r\n");
			vTaskDelay(100);
			retransmitMessage.retransmitAttempts++;
			xQueueSendToFront(txMessageQueue, (void*) &retransmitMessage, 100);
		} else {
			myprintf("Giving up after 3 attempts\r\n");
		}
	}

	xSemaphoreGive(transmitSemaphore);
	vTaskDelete(ackTask);
}

void FSMProcessing_Task(void) {
	for(EVER) {
		debug_printf("5a");
		s4435360_radio_fsmprocessing();
		debug_printf("5b");
		vTaskDelay(500);
	}
}
