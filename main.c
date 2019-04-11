/*
 * LWM_MSSY.c
 *
 * Created: 6.4.2017 15:42:46
 * Author : Krajsa
 */ 

#include <avr/io.h>
#include <util/delay.h>
/*- Includes ---------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "config.h"
#include "hal.h"
#include "phy.h"
#include "sys.h"
#include "nwk.h"
#include "sysTimer.h"
#include "halBoard.h"
#include "halUart.h"
#include "main.h"
#include "device.h"
#include "packet_parser.h"

/*- Definitions ------------------------------------------------------------*/
#ifdef NWK_ENABLE_SECURITY
#define APP_BUFFER_SIZE     (NWK_MAX_PAYLOAD_SIZE - NWK_SECURITY_MIC_SIZE)
#else
#define APP_BUFFER_SIZE     NWK_MAX_PAYLOAD_SIZE
#endif

/*- Types ------------------------------------------------------------------*/
typedef enum AppState_t
{
	APP_STATE_INITIAL,
	APP_STATE_IDLE,
} AppState_t;

/*- Prototypes -------------------------------------------------------------*/
static void appSendData(void);

/*- Variables --------------------------------------------------------------*/
static AppState_t appState = APP_STATE_INITIAL;
static SYS_Timer_t appTimer;
static NWK_DataReq_t appDataReq;
static bool appDataReqBusy = false;
static uint8_t appDataReqBuffer[APP_BUFFER_SIZE];
static uint8_t appUartBuffer[APP_BUFFER_SIZE];
static uint8_t appUartBufferPtr = 0;
static uint8_t appUartTempbufferPtr = 0;

/*- Implementations --------------------------------------------------------*/

/*************************************************************************//**
*****************************************************************************/
static void appDataConf(NWK_DataReq_t *req)
{
	appDataReqBusy = false;
	if (req->status == NWK_SUCCESS_STATUS) {	
		(void)req;
	} else {
		appSendData();
	}
}

/*************************************************************************//**
*****************************************************************************/
static void prepareSendData(uint8_t deviceIndex, uint8_t endpoint, uint8_t* data, uint8_t length) {
	memcpy(appDataReqBuffer, data, length);
	
	appDataReq.dstAddr = devices[deviceIndex].address;
	appDataReq.dstEndpoint = endpoint;
	appDataReq.srcEndpoint = endpoint;
	appDataReq.options = NWK_OPT_ENABLE_SECURITY;
	appDataReq.data = appDataReqBuffer;
	appDataReq.size = appUartBufferPtr = length;
	appDataReq.confirm = appDataConf;
	
	//SYS_TimerStop(&appTimer);
	//SYS_TimerStart(&appTimer);
	//_delay_us(100);
}

/*************************************************************************//**
*****************************************************************************/
static void appSendData(void) {
	if (appDataReqBusy || 0 == appUartBufferPtr) {
		APP_WriteString("Busy - cannot send data! \r\n");
		_delay_us(100);
		appSendData();
		return;
	}

	NWK_DataReq(&appDataReq);

	appDataReqBusy = true;
	appUartBufferPtr = 0;
}

/*************************************************************************//**
*****************************************************************************/
void printDevices() {
	APP_WriteString("\r\n| DEVICE | ADDRESS | STATE |\r\n");
	for (uint8_t i = 0; i < connected_devices; ++i) {
		APP_WriteString("|   ");
		HAL_UartWriteByte(i + '0');
		APP_WriteString("    |   ");
		HAL_UartWriteByte((uint8_t)devices[i].address + '0');	
		APP_WriteString("     |");
		switch (devices[i].state) {
			case Connected: { APP_WriteString(" CONNECT |\r\n"); break; }
			case InSleep: { APP_WriteString(" SLEEP |\r\n"); break; }
			case Disconnected: { APP_WriteString(" DISCONN |\r\n"); break; }
		}	
	}
}

/*************************************************************************//**
*****************************************************************************/
void HAL_UartBytesReceived(uint16_t bytes) { //citanie konzoly z klavesnice
	for (uint16_t i = 0; i < bytes; i++) {
		uint8_t byte = HAL_UartReadByte();
		HAL_UartWriteByte(byte);

		if (byte == '\r') {
			appUartBuffer[appUartTempbufferPtr++] = '\0';
			if (strcmp("devices", (const char *)appUartBuffer) == 0) {
				printDevices(); 
			} else {
				if (strcmp("hello", (const char *)appUartBuffer) == 0) { //simulating NODE
					Device new_device;
					new_device.address = 1;
					new_device.state = Disconnected;
					new_device.last_packet = AckPacket;
					devices[connected_devices++] = new_device;
					
					HelloPacket_t *packet = (HelloPacket_t *)malloc(sizeof(HelloPacket_t));
					packet->data_part.command_id = 128;
					packet->data_part.data = 32;
					packet->data_part.device_type = 128;
					packet->data_part.items = (uint8_t *)malloc(1);
					packet->data_part.items[0] = 1;
					packet->read_write = 0;
					packet->sleep = 0;
					
					uint8_t *frame_payload = (uint8_t *)malloc(8);
					serialize_hello_packet(packet, frame_payload, 8);					
					prepareSendData(0, 1, frame_payload, 8);
					appSendData();
					
					APP_WriteString("\r\nHello packet sent!\r\n");
				}
				
				if (appUartBuffer[0] == 'g' && appUartBuffer[1] == 'e' && appUartBuffer[2] == 't') {
				
				}
				if (appUartBuffer[0] == 's' && appUartBuffer[1] == 'e' && appUartBuffer[2] == 't') {
				}			
			}
			appUartTempbufferPtr = 0;	
			
		} else {
			appUartBuffer[appUartTempbufferPtr++] = byte;
		}
		
		/*
		if (appUartBufferPtr == sizeof(appUartBuffer)) {
		}

		if (appUartBufferPtr < sizeof(appUartBuffer))
			appUartBuffer[appUartBufferPtr++] = byte;
			*/
	}
}

/*************************************************************************//**
*****************************************************************************/
static void appTimerHandler(SYS_Timer_t *timer) {
	appSendData();
	(void)timer;
}

static void createSendHelloAck(NWK_DataInd_t *ind, uint16_t sleep_time, uint8_t command_id) {
	HAL_UartWriteByte(command_id + '0');
	APP_WriteString("\r\n");
	HelloAckPacket_t *packet = (HelloAckPacket_t *)malloc(sizeof(HelloAckPacket_t));
	packet->address = ind->srcAddr;
	packet->command_id = command_id;
	packet->reserved = 0;
	packet->sleep_period = sleep_time;
	uint8_t *frame_payload = (uint8_t *)malloc(7);
	serialize_hello_ack_packet(packet, frame_payload);
	prepareSendData(packet->address, 1, frame_payload, 7);
	appSendData();
}

/*************************************************************************//**
*****************************************************************************/
static bool appDataInd(NWK_DataInd_t *ind) { //prijem
	uint8_t new_device = 0;
	if (ind->dstEndpoint == 1 && ind->data[0] == 128) {
		for (int i = 0; i < connected_devices; ++i) {
			if (devices[i].address == ind->srcAddr) {
				devices[i].state = Disconnected;
				new_device = 1;
			}
		}
		if (new_device == 0) {
			Device new_device;
			new_device.address = ind->srcAddr;
			new_device.state = Disconnected;
			new_device.last_packet = AckPacket;
			devices[connected_devices++] = new_device;
			APP_WriteString("New device connected!\r\n");
		}
	}
	
	for (int i = 0; i < connected_devices; ++i) {
		if (devices->address == ind->srcAddr) {
			free(packet_buffer);
			last_packet_type = process_packet(devices[i], ind->dstEndpoint ,ind->data, ind->size, packet_buffer);
			HAL_UartWriteByte(last_packet_type + '0');
			APP_WriteString("\r\n");
			//_delay_us(50);
			switch (last_packet_type) {
				case HelloPacket: { createSendHelloAck(ind, 1000, 2); APP_WriteString("Hello ACK sent! \r\n"); break; }
				case HelloAckPacket: { createSendHelloAck(ind, 1000, 1); APP_WriteString("ACK to Hello ACK sent! \r\n"); break; }
				case AckPacket: { APP_WriteString("ACK to Hello ACK received! \r\n"); devices[i].state = Connected; break; }
			}
		}
	}
	
	//debug print
	
	for (uint8_t i = 0; i < ind->size; i++)
		HAL_UartWriteByte(ind->data[i]);
	return true;
}

/*************************************************************************//**
*****************************************************************************/
static void appInit(void)
{
	NWK_SetAddr(APP_ADDR);
	NWK_SetPanId(APP_PANID);
	PHY_SetChannel(APP_CHANNEL);
#ifdef PHY_AT86RF212
	PHY_SetBand(APP_BAND);
	PHY_SetModulation(APP_MODULATION);
#endif
	PHY_SetRxState(true);

	NWK_OpenEndpoint(NETWORK_ENDPOINT, appDataInd);
	NWK_OpenEndpoint(GET_SET_ENDPOINT, appDataInd);
	NWK_OpenEndpoint(PERIODIC_ENDPOINT, appDataInd);
	NWK_OpenEndpoint(ALARM_ENDPOINT, appDataInd);

	HAL_BoardInit();

	appTimer.interval = APP_FLUSH_TIMER_INTERVAL;
	appTimer.mode = SYS_TIMER_INTERVAL_MODE;
	appTimer.handler = appTimerHandler;
}

/*************************************************************************//**
*****************************************************************************/
static void APP_TaskHandler(void) {
	switch (appState) {
		case APP_STATE_INITIAL: {
			appInit();
			appState = APP_STATE_IDLE;
		}	break;

		case APP_STATE_IDLE:
			break;

		default:
			break;
	}
}

/*************************************************************************//**
*****************************************************************************/
void APP_WriteString(char *string) {
	int index = 0;
	while (string[index] != '\0') {
		HAL_UartWriteByte(string[index]);
		++index;
	}
}

/*************************************************************************//**
*****************************************************************************/
int main(void) {
	SYS_Init();
	HAL_UartInit(38400);
	
	APP_WriteString("APP started!\r\n");
	
	while (1) {
		SYS_TaskHandler();
		HAL_UartTaskHandler();
		APP_TaskHandler();
	}
}
