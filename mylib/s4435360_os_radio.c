/**
 ******************************************************************************
 * @file    mylib/s4435360_os_radio.c
 * @author  Samuel Eadie - 44353607
 * @date    09052018
 * @brief   FreeRTOS functionality to control radio HAL
 *
 ******************************************************************************
*/

 /*Includes ------------------------------------------------------------------*/
#include <s4435360_os_radio.h>

#include "nrf24l01plus.h"
#include "radio_fsm.h"
#include "s4435360_hal_radio.h"
#include "s4435360_hal_hamming.h"
#include "s4435360_os_printf.h"
#include "s4435360_os_pantilt.h"

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
/*Private define ------------------------------------------------------------*/
#define EVER						;;
#define RADIO_QUEUE_LENGTH			10

#define ACKNOWLEDGEMENT_STACK_SIZE			( configMINIMAL_STACK_SIZE * 5 )

#define ACKNOWLEDGEMENT_TASK_PRIORITY 		( tskIDLE_PRIORITY + 5 )

//Event group definitions for ack/err
#define EVT_ACK				1 << 0		//ACK received
#define EVT_ERR				1 << 1		//ERR received
#define ACKCTRL_EVENT		EVT_ACK | EVT_ERR	//Control Event Group Mask

#define PAYLOAD_STARTING_INDEX		10

//Private macro -------------------------------------------------------------
//Private variables ---------------------------------------------------------
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
//Private function prototypes -----------------------------------------------
void FSMProcessing_Task(void);
void Acknowledgment_Task(void);
//Private functions ---------------------------------------------------------

/**
  * @brief Returns the transmission address of the radio
  * @param None
  * @retval the radio transmission address
  */
unsigned char* get_txAddress(void) {
	return txAddress;
}

/**
  * @brief Returns the receive address of the radio
  * @param None
  * @retval the radio receive address
  */
unsigned char* get_rxAddress(void) {
	return rxAddress;
}

/**
  * @brief Returns the radio's current channel
  * @param None
  * @retval the radio channel
  */
unsigned char get_chan(void) {
	return channel;
}

/**
  * @brief Sets the radios receive address
  * @param addr: the receive address
  * @retval None
  */
void set_rxAddress(unsigned char* addr) {
	memcpy((void*) rxAddress, (void*) addr, 4);
}

/**
  * @brief Sets the radio's transmission address
  * @param addr: the transmission address
  * @retval None
  */
void set_txAddress(unsigned char* addr) {
	memcpy((void*) txAddress, (void*) addr, 4);
}

/**
  * @brief Sets the radio's channel
  * @param chan: the radio channel
  * @retval None
  */
void set_chan(unsigned char chan) {
	channel = chan;
}

/**
  * @brief Sends a message via radio
  * @param
  * 	payload: the payload to send
  * 	payloadLength: the length of the payload to send
  * 	retransmitAttempts: the number of attempts to retransmit this packet
  * 	waitTime: the time to wait adding this message to the queue
  * 	isXYZ: whether this message is an XYZ message, needs pantilt tracking
  * @retval None
  */
void send_radio_message(char* payload, int payloadLength, int retransmitAttempts, int waitTime, int isXYZ) {
	RadioMessage message;

	message.retransmitAttempts = retransmitAttempts;
	message.payloadLength = payloadLength;
	message.isXYZ = isXYZ;
	memcpy((void*) message.payload, (void*) payload, payloadLength);

	xQueueSendToBack(txMessageQueue, ( void * ) &message, ( TickType_t ) waitTime);
}

/**
  * @brief Sends an XYZ packet via radio
  * @param
  * 	x: the x value
  * 	y: the y value
  * 	z: the z value
  * 	waitTime: the time to block to add the message to the queue
  * @retval None
  */
void send_XYZ_message(int x, int y, int z, int waitTime) {
	char xyzPayload[11];
	sprintf(xyzPayload, "XYZ%03d%03d%02d", x, y, z);

	lastX = x;
	lastY = y;
	lastZ = z;

	send_radio_message(xyzPayload, 11, 0, waitTime, 1);
	s4435360_pantilt_changeX(lastX);
	s4435360_pantilt_changeY(lastY);
}

/**
  * @brief Sends a radio packet to increment the current X position
  * @param
  * 	increment: the amount to increment
  * 	waitTime: the time to block to add the message to the queue
  * @retval None
  */
void send_X_increment_message(int increment, int waitTime) {
	lastX += increment;
	send_XYZ_message(lastX, lastY, lastZ, waitTime);
}

/**
  * @brief Sends a radio packet to increment the current Y position
  * @param
  * 	increment: the amount to increment
  * 	waitTime: the time to block to add the message to the queue
  * @retval None
  */
void send_Y_increment_message(int increment, int waitTime) {
	lastY += increment;
	send_XYZ_message(lastX, lastY, lastZ, waitTime);
}

/**
  * @brief Sends a radio packet to change the Z position
  * @param
  * 	z: the new z value
  * 	waitTime: the time to block to add the message to the queue
  * @retval None
  */
void send_Z_message(int z, int waitTime) {
	send_XYZ_message(lastX, lastY, z, waitTime);
	lastZ = z;
}

/**
  * @brief Sends a radio packet to change the X and Y position
  * @param
  * 	x: the new x value
  * 	y: the new y value
  * 	waitTime: the time to block to add the message to the queue
  * @retval None
  */
void send_XY_message(int x, int y, int waitTime) {
	send_XYZ_message(x, y, lastZ, waitTime);
	lastX = x;
	lastY = y;
}


/**
  * @brief Sends a join message via radio
  * @param
  * 	waitTime: the time to block to add the message to the queue
  * @retval None
  */
void send_join_message(int waitTime) {
	char* joinPayload = "JOIN";
	send_radio_message(joinPayload, 4, 0, waitTime, 0);
}

/**
  * @brief Forms the specified payload into the packet
  * @param
  * 	payload: the payload to form into a packet
  * 	payloadLength: the length of the payload
  * 	packet: the paket to form the payload into
  * @retval None
  */
void form_packet(char* payload, int payloadLength, char* packet) {

	//Add packet header
	//memcpy((void*) packet, (void*) packetHeader, PAYLOAD_STARTING_INDEX);
	packet[0] = 0xA1;
	memcpy((void*) &packet[1], (void*) txAddress, 4);
	memcpy((void*) &packet[5], (void*) rxAddress, 4);
	packet[9] = 0x00;

	//Encode and add payload
	uint16_t encodedByte;
	for(int i = 0; i < payloadLength; i++) {
		encodedByte = hamming_byte_encoder(payload[i]);
		packet[PAYLOAD_STARTING_INDEX + (2 * i)] = (uint8_t)((encodedByte & 0xFF00) >> 8);
		packet[PAYLOAD_STARTING_INDEX + (2 * i) + 1] = (uint8_t)(encodedByte & 0x00FF);
	}
}

/**
  * @brief Task for managing radio communication
  * @param None
  * @retval None
  */
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
				myprintf("Packet sent\r\n");

				if(!strncmp(toSend.payload, "XYZ", 3)) {
					xSemaphoreGive(s4435360_SemaphoreUpdatePantilt);
				}

				//Clear bits for new acknowledgment routine
				xEventGroupClearBits(ackctrl_EventGroup, EVT_ACK | EVT_ERR);

				//vTaskDelay(1000);
				retransmitMessage = toSend;

				//Create task to check acknowledgment
				xTaskCreate((void *) &Acknowledgment_Task, (const char *) "ACK",
						ACKNOWLEDGEMENT_STACK_SIZE, NULL, ACKNOWLEDGEMENT_TASK_PRIORITY, &ackTask);
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
				xEventGroupSetBits(ackctrl_EventGroup, EVT_ACK);
			}

			memset(&s4435360_rx_buffer[0], 0, sizeof(s4435360_rx_buffer));
			s4435360_radio_rxstatus = 0;
		}

		s4435360_radio_fsmprocessing();
		vTaskDelay(300);
	}
}


/**
  * @brief Task for managing the radio acknowledgment procedure
  * @param None
  * @retval None
  */
void Acknowledgment_Task(void) {
	myprintf("Started ack\r\n");

	//Block for ERR or ACK, 3 second timeout
	EventBits_t eventBits = xEventGroupWaitBits(ackctrl_EventGroup, EVT_ACK | EVT_ERR,	pdTRUE,	pdFALSE, 3000);

	//ACK received in 3 seconds
	if((eventBits & EVT_ACK) != 0) {
		xSemaphoreGive(transmitSemaphore);

	} else if ((eventBits & EVT_ERR) != 0) {
		//ERR received
		myprintf("Adjusting for ERR\r\n");
		retransmitMessage.retransmitAttempts = 0;
		xQueueSendToFront(txMessageQueue, (void*) &retransmitMessage, portMAX_DELAY);
		xSemaphoreGive(transmitSemaphore);

	} else {
		//vTaskDelay(100);
		retransmitMessage.retransmitAttempts++;
		xQueueSendToFront(txMessageQueue, (void*) &retransmitMessage, portMAX_DELAY);

		xSemaphoreGive(transmitSemaphore);
	}

	vTaskDelete(ackTask);
}
