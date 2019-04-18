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
void APP_byte_to_hex(uint8_t *byte, uint8_t *hex, uint8_t hex_length);
void APP_hex_to_byte(uint8_t hex[], uint8_t byte[], uint8_t byte_length);

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
		APP_WriteString("Send complete!\r\n");
	} else {
		APP_WriteString("Send error!\r\n");
		appSendData();
	}
}

/*************************************************************************//**
*****************************************************************************/
static void prepareSendData(uint8_t deviceIndex, uint8_t endpoint, uint8_t* data, uint8_t length) {
	memcpy(appDataReqBuffer, data, length);
	
	appDataReq.dstAddr = nodes[deviceIndex].device.address;
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
void APP_printnodes() {
	APP_WriteString("\r\n| DEVICE | ADDRESS | STATE |\r\n");
	for (uint8_t i = 0; i < connected_nodes; ++i) {
		APP_WriteString("|   ");
		HAL_UartWriteByte(i + '0');
		APP_WriteString("    |   ");
		HAL_UartWriteByte((uint8_t)nodes[i].device.address + '0');	
		APP_WriteString("     |");
		switch (nodes[i].device.state) {
			case Connected: { APP_WriteString(" CONNECT |\r\n"); break; }
			case InSleep: { APP_WriteString(" SLEEP |\r\n"); break; }
			case Disconnected: { APP_WriteString(" DISCONN |\r\n"); break; }
		}	
	}
}

/*************************************************************************//**
*****************************************************************************/
void APP_sendHello() { // dev&testing only
	Node new_device;
	new_device.device.address = 0x01;  //(APP_ADDR == 0x00 ) ? 0x01 : 0x00;
	new_device.device.state = Disconnected;
	nodes[connected_nodes++] = new_device;
	
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

/*************************************************************************//**
*****************************************************************************/
void HAL_UartBytesReceived(uint16_t bytes) { //citanie konzoly z klavesnice
	for (uint16_t i = 0; i < bytes; i++) {
		uint8_t byte = HAL_UartReadByte();
		HAL_UartWriteByte(byte);

		if (byte == '\r') {
			appUartBuffer[appUartTempbufferPtr++] = '\0';
			if (strcmp("nodes", (const char *)appUartBuffer) == 0) {
				APP_printnodes(); 
			} else {
				if (strcmp("hello", (const char *)appUartBuffer) == 0) { //simulating NODE
					APP_sendHello(); // dev&testing only
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
	//appSendData();
	(void)timer;
}

/*************************************************************************//**
*****************************************************************************/
static void createSendHelloAck(NWK_DataInd_t *ind, uint16_t sleep_time, uint8_t command_id) {
	APP_WriteString("Sending ACK command: ");
	HAL_UartWriteByte(command_id + '0'); //dev&testing only
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
static void createSendAck(NWK_DataInd_t *ind) {
	prepareSendData(ind->srcAddr, ind->srcEndpoint, ind->data, ind->size); // check size value!!
	appSendData();
}

static void createSendDataAck(NWK_DataInd_t *ind, uint8_t new_data) {
	ReconnectAckPacket_t *packet = (ReconnectAckPacket_t *)malloc(sizeof(ReconnectAckPacket_t));
	packet->command_id = 1;
	packet->data = new_data;
	packet->reserved = 0;
	uint8_t *frame_payload = (uint8_t *)malloc(4);
	serialize_reconnect_packet(packet, frame_payload);
	prepareSendData(ind->srcAddr, ind->srcEndpoint, frame_payload, 4);
	appSendData();
}

/*************************************************************************//**
*****************************************************************************/
static void printData(DataPacket_t *dataPacket, uint8_t packetLength) {
	uint8_t item_count;
	detect_data_packet_arrays_size(dataPacket->data, &item_count);
	uint8_t values_size = packetLength - item_count - 3;
	uint8_t *hex_values = (uint8_t *)malloc(values_size);
	APP_byte_to_hex(dataPacket->values, hex_values, values_size);
	APP_WriteString("---Data packet:");
	for (uint8_t i = 0; i < values_size; ++i ) {
		HAL_UartWriteByte(hex_values[i]);
		HAL_UartWriteByte(' ');
	}
	APP_WriteString("\r\n---End Of Data---\r\n");
}
/*************************************************************************//**
*****************************************************************************/
static bool appDataInd(NWK_DataInd_t *ind) { //prijem
	free(packet_buffer);
	
	uint8_t new_device = 0;
	if (ind->dstEndpoint == 1 && ind->data[0] == 128) {
		for (int i = 0; i < connected_nodes; ++i) { // duplicate hello packet from node
			if (nodes[i].device.address == ind->srcAddr) {
				nodes[i].device.state = Disconnected;
				new_device = 1;
			}
		}
		if (new_device == 0) { // first hello packet from node
			Device new_device = {ind->srcAddr, Disconnected};
			last_packet_type = process_packet(new_device, ind->dstEndpoint ,ind->data , ind->size, packet_buffer);
			HelloPacket_t *hello_packet = (HelloPacket_t *)packet_buffer;
			
			uint8_t values_count, items_count;
			detect_data_packet_arrays_size(hello_packet->data_part.data, &values_count);
			get_values_bytesize(hello_packet->data_part.data, &items_count); 
			
			DeviceProperties props = {hello_packet->data_part.data, (uint8_t *)malloc(values_count), (uint8_t *)malloc(items_count), 
				hello_packet->sleep, hello_packet->read_write};
			Node new_node = {new_device, props};
			nodes[connected_nodes++] = new_node;
			APP_WriteString("New device connected!\r\n");
		}
	}
	
	for (int i = 0; i < connected_nodes; ++i) {
		if (nodes->device.address == ind->srcAddr) {
			if (packet_buffer == NULL) {
				last_packet_type = process_packet(nodes[i].device, ind->dstEndpoint ,ind->data , ind->size, packet_buffer);
			}
			APP_WriteString("Received packet: ");
			HAL_UartWriteByte(last_packet_type + '0');
			APP_WriteString("\r\n");
			//_delay_us(50);
			switch (last_packet_type) {
				case HelloPacket: { 
					createSendHelloAck(ind, 1000, 2); 
					APP_WriteString("Hello ACK sent! \r\n"); 
					break;
				}
				case HelloAckPacket: { // dev&testing only
					 createSendHelloAck(ind, 1000, 1); 
					 APP_WriteString("ACK to Hello ACK sent! \r\n"); 
					 nodes[i].device.state = Connected;
					 break; 
				} 
				case AckPacket: { 
					if (ind->dstEndpoint == 1) {
						APP_WriteString("ACK to Hello ACK received! \r\n");
						nodes[i].device.state = Connected; 
					} else if (ind->dstEndpoint == 2) {
						APP_WriteString("ACK to SetValue received! \r\n");
					}
					break; 
				}
				case SleepPacket: { 
					createSendAck(ind); 
					APP_WriteString("Sleep packet received!\r\n"); 
					nodes[i].device.state = InSleep; 
					break; 
				} 
				case ReconnectPacket: { 
					createSendAck(ind); 
					APP_WriteString("Reconnect packet received!\r\n"); 
					nodes[i].device.state = Connected; 
					break; 
				}
				case DataPacket: {
					createSendDataAck(ind, 0);
					APP_WriteString("Periodical data packet received!\r\n");
					printData((DataPacket_t *)packet_buffer, ind->size); 
					break;
				}
				case SetValuePacket:
				case GetValuePacket:
				case UnknownPacket: {
					APP_WriteString("!!! Somethings wrong, unknown or bad packet received !!!\r\n");
					break;
				}
			}
			break; 
		}
	}
	
	//debug print
	/*
	for (uint8_t i = 0; i < ind->size; i++)
		HAL_UartWriteByte(ind->data[i]);*/
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
			APP_WriteString("APP initiated!\r\n");
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
uint8_t hex_char_to_value(uint8_t hex_char) {
	uint8_t result = -1;
	if (hex_char >= '0' && hex_char <= '9')
		result = hex_char - '0';
	else if (hex_char >= 'a' && hex_char <= 'f')
		result = hex_char - 'a' + 10;
	else if (hex_char >= 'A' && hex_char <= 'F')
		result = hex_char - 'A' + 10;
	return result;
}

void APP_hex_to_byte(uint8_t hex[], uint8_t byte[], uint8_t byte_length) {
	for (uint8_t i = 0; i < byte_length; ++i) {
		byte[i] = (hex_char_to_value(hex[2 * i]) << 4) + hex_char_to_value(hex[2 * i + 1]);
	}
}

/*************************************************************************//**
*****************************************************************************/
uint8_t byte_to_hex_value(uint8_t char_value) {
	return (char_value < 10) ? char_value + '0' : char_value + 'a' - 10;
}

void APP_byte_to_hex(uint8_t *byte, uint8_t *hex, uint8_t hex_length) {
	for (uint8_t i = 0; i < hex_length; i+=2) {
		hex[i] = byte_to_hex_value(byte[i / 2] >> 4);
		hex[i+1] = byte_to_hex_value(byte[i / 2] & 0xf);
	}
}

/*************************************************************************//**
*****************************************************************************/
int main(void) {
	SYS_Init();
	HAL_UartInit(38400);
	
	while (1) {
		SYS_TaskHandler();
		HAL_UartTaskHandler();
		APP_TaskHandler();
	}
}
