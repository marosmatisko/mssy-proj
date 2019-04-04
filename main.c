/*
 * LWM_MSSY.c
 *
 * Created: 6.4.2017 15:42:46
 * Author : Krajsa
 */ 

#include <avr/io.h>
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
	(void)req;
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
}

/*************************************************************************//**
*****************************************************************************/
static void appSendData(void) {
	if (appDataReqBusy || 0 == appUartBufferPtr)
		return;
/*
	memcpy(appDataReqBuffer, appUartBuffer, appUartBufferPtr);

	appDataReq.dstAddr = 1 - APP_ADDR;
	appDataReq.dstEndpoint = APP_ENDPOINT;
	appDataReq.srcEndpoint = APP_ENDPOINT;
	appDataReq.options = NWK_OPT_ENABLE_SECURITY;
	appDataReq.data = appDataReqBuffer;
	appDataReq.size = appUartBufferPtr;
	appDataReq.confirm = appDataConf; */
	NWK_DataReq(&appDataReq);

	appUartBufferPtr = 0;
	appDataReqBusy = true;
}

/*************************************************************************//**
*****************************************************************************/
void printDevices() {
	APP_WriteString("\r\n| DEVICE | ADDRESS | STATE |\r\n");
	for (uint8_t i = 0; i < connected_devices; ++i) {
		APP_WriteString("|   ");
		HAL_UartWriteByte(i + '0');
		APP_WriteString("     |   ");
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
			if (strcmp("devices", appUartBuffer) == 0) {
				printDevices(); // TODO 
			} else {
				if (strcmp("hello", appUartBuffer) == 0) {
					Device new_device;
					new_device.address = 0;
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
					memcpy(frame_payload, packet, 4);
					memcpy(&frame_payload[4], packet->data_part.items, 1);
					memcpy(&frame_payload[5], &packet->sleep, 3);
					
					prepareSendData(0, 1, frame_payload, 8);
					appSendData();
					APP_WriteString("\r\nHello packet sent!\r\n");
					appUartTempbufferPtr = 0;
				}
				
				if (appUartBuffer[0] == 'g' && appUartBuffer[1] == 'e' && appUartBuffer[2] == 't') {
				
				}
				if (appUartBuffer[0] == 's' && appUartBuffer[1] == 'e' && appUartBuffer[2] == 't') {
				}
				//prepareSendData();
				//appSendData();
			}
			
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

	SYS_TimerStop(&appTimer);
	SYS_TimerStart(&appTimer);
}

/*************************************************************************//**
*****************************************************************************/
static void appTimerHandler(SYS_Timer_t *timer) {
	appSendData();
	(void)timer;
}

/*************************************************************************//**
*****************************************************************************/
static bool appDataInd(NWK_DataInd_t *ind) { //prijem
	if (ind->dstEndpoint == 1 && ind->data[0] == 128) {
		Device new_device;
		new_device.address = ind->srcAddr;
		new_device.state = Disconnected;
		new_device.last_packet = AckPacket;
		devices[connected_devices++] = new_device; 
		APP_WriteString("New device connected!\r\n");
	}
	
	for (int i = 0; i < DEVICE_LIMIT; ++i) {
		if (devices->address == ind->srcAddr) {
			process_packet(devices[i], ind->dstEndpoint ,ind->data, ind->size);
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
