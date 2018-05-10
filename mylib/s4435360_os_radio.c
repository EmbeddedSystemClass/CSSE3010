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

#include "s4435360_hal_radio.h"

#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"
#include "board.h"
#include "debug_printf.h"

/* Private typedef -----------------------------------------------------------*/
typedef struct {
	char payload[11];
	int payloadLength = 11;
	int retransmitAttempts = 0;
} RadioMessage;

/* Private define ------------------------------------------------------------*/
#define EVER						;;
#define RADIO_QUEUE_LENGTH			10

#define ACKNOWLEDGEMENT_STACK_SIZE			( configMINIMAL_STACK_SIZE * 2 )
#define FSM_PROCESSING_STACK_SIZE			( configMINIMAL_STACK_SIZE )

#define ACKNOWLEDGEMENT_TASK_PRIORITY 		( tskIDLE_PRIORITY + 2 )
#define FSM_PROCESSING_TASK_PRIORITY		( tskIDLE_PRIORITY + 1 )

//Event group definitions for ack/err
#define EVT_ACK				1 << 0		//ACK received
#define EVT_ERR				1 << 1		//ERR received
#define ACKCTRL_EVENT		EVT_ACK | EVT_ERR	//Control Event Group Mask

#define PAYLOAD_STARTING_INDEX		10
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
SemaphoreHandle_t transmitSemaphore;
QueueHandle_t txMessageQueue;
EventGroupHandle_t ackctrl_EventGroup;

unsigned char channel = 52;
unsigned char txAddress[5] = {0x52, 0x33, 0x22, 0x11, 0x00};
unsigned char rxAddress[5] = {0x07, 0x36, 0x35, 0x44, 0x00};
unsigned char packetHeader[10] = {0xA1, 		//Packet Type
			0x52, 0x33, 0x22, 0x11,				//Source address
			0x07, 0x36, 0x35, 0x44,				//Destination address
			0x00};								//Blank char
};
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

void form_packet(unsigned char* payload, int payloadLength, unsigned char* packet) {

	/* Add packet header */
	memcpy(packet, packetHeader, PAYLOAD_STARTING_INDEX);

	/* Encode and add payload */
	uint16_t encodedByte;
	for(int i = 0; i < payloadLength; i++) {
		encodedByte = hamming_byte_encoder(payload[i]);
		packet[PAYLOAD_STARTING_INDEX + (2 * i)] = (uint8_t)((encodedByte & 0xFF00) >> 8);
		packet[PAYLOAD_STARTING_INDEX + (2 * i) + 1] = (uint8_t)(encodedByte & 0x00FF);
	}
}

void s4435360_TaskRadio(void) {

	//Initialise hardware


	//Need more radio initialisation - see Radio Duplex
	s4435360_hal_radio_init();
	xTaskCreate((void *) &FSMProcessing_Task, (const char *) "FSMPROCESSING",
						FSM_PROCESSING_STACK_SIZE, NULL, FSM_PROCESSING_TASK_PRIORITY, NULL);

	//Initialise radio messages queue
	txMessageQueue = xQueueCreate(RADIO_QUEUE_LENGTH, sizeof(RadioMessage));

	//Initialise ack semaphore
	transmitSemaphore = xSemaphoreCreateBinary();
	xSemaphoreGive(transmitSemaphore); //Allow first transmission

	//Initialise reliability event groups
	ackctrl_EventGroup = xEventGroupCreate();

	RadioMessage toSend;
	TaskHandle_t ackTask;

	for(EVER) {

		//Transmission
		if(xQueueReceive(txMessageQueue, &toSend, 100)) {
			//Check if not currently in acknowledgement process
			if(xSemaphoreTake(transmitSemaphore, 100)) {
				//Send payload message
				form_packet(toSend.payload, toSend.payloadLength, s4435360_tx_buffer);
				s4435360_radio_sendpacket(channel, txAddress, s4435360_tx_buffer);
				memset(&s4435360_tx_buffer[0], 0, 32);

				//Clear bits for new acknowledgment routine
				xEventGroupClearBits(ackctrl_EventGroup, EVT_ACK | EVT_ERR);

				//Create task to check acknowledgment
				xTaskCreate((void *) &Acknowledgment_Task, (const char *) "ACK",
						ACKNOWLEDGEMENT_STACK_SIZE, NULL, ACKNOWLEDGEMENT_TASK_PRIORITY, &ackTask);
			}
		}

		//Received packet
		if(s4435360_radio_rxstatus) {

			//Get payload
			s4435360_radio_getpacket(s4435360_rx_buffer);
			char* payload = &s4435360_rx_buffer[PAYLOAD_STARTING_INDEX];

			char decodedPayload[11];
			if(hamming_decode_payload(decodedPayload, payload, 11)) {
				if(!strncmp(decodedPayload, "A C K", 5)) {
					//Received ACK - give semaphore
					xEventGroupSetBits(ackctrl_EventGroup, EVT_ACK);
				} else if(!strncmp(decodedPayload, "E R R", 5)) {
					xEventGroupSetBits(ackctrl_EventGroup, EVT_ERR);
				}

			//Received uncorrectable error
			} else {

			}
		}

		vTaskDelay(100);

	}
}


void Acknowledgment_Task(void* param) {

	//Radio message for possible retransmission
	RadioMessage retransmitMessage = (RadioMessage)(*param);

	//Block for ERR or ACK, 3 second timeout
	EventBits_t eventBits = xEventGroupWaitBits(ackctrl_EventGroup, EVT_ACK | EVT_ERR,	pdTRUE,	pdFALSE, 3000);

	//ACK received in 3 seconds
	if(eventBits & EVT_ACK) {

	} else if (eventBits & EVT_ERR) {
		//ERR received
		retransmitMessage.retransmitAttempts = 0;
		xQueueSendToFront(txMessageQueue, (void*) &retransmitMessage, 100);

	} else {
		//ACK not received - retransmit
		if(retransmitMessage.retransmitAttempts < 3) {
			retransmitMessage.retransmitAttempts++;
			xQueueSendToFront(txMessageQueue, (void*) &retransmitMessage, 100);
		}
	}

	xSemaphoreGive(transmitSemaphore);
}

void FSMProcessing_Task(void) {
	s4435360_hal_radiofsmprocessing();
	vTaskDelay(500);
}
