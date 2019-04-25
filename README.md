
# MSSY-project
Master branch - packet library for gateway & nodes.   
Gateway branch - implementation of gateway functions.   

## Features

 - structures for all supported packets
 - methods for serialization of supported packets
 - *gateway* branch contains sample *main.c* class, which supports sending/receiving *Hello* and *Data* packets (see section *How to use* down below).
 - support for sending/receiving other packets needs to be implemented by each device separately. This library provides neccessary features, which other users can use to send packets

## FAQ: 
**Q:** How to use?   
**A:** Add to you main.h:   
``` c
    #include "device.h"
    #include "packet_parser.h"
    
    void APP_WriteString(char *string);
```
and to your main.c:
``` c
    void APP_WriteString(char *string) {
	int index = 0;
	while (string[index] != '\0') {
		HAL_UartWriteByte(string[index]);
		++index;
	}
  }   
``` 
APP_WriteString is used for testing & debug purposes. Then in packet_parser.c add call to your functions with every packet structure.   
   
   
**Q:** What if I do not want to use this debug?   
**A:** Just comment in packet_parser.h
``` c
//extern void APP_WriteString(char *string);
``` 
and in packet_parser.c
``` c
//#define DEBUGGING
```    
   
   
**Q:** What are that files for?   
**A:** Take a look on table below.      

## Example code for sending packets
*HelloPacket*:
``` c
void APP_sendHello() { //dev&testing only
	//useful for gateway (for keeping state of multiple devices)
	Node *new_node = (Node *)malloc(sizeof(Node)); 
	Device dev;
	dev.address = (APP_ADDR == 0x00 ) ? 0x01 : 0x00;
	dev.state = Disconnected;
	new_node->device = dev;
	nodes[connected_nodes++] = *new_node;
	
	HelloPacket_t *packet = (HelloPacket_t *)malloc(sizeof(HelloPacket_t));
	packet->data_part.command_id = 128; //Hello
	packet->data_part.data = 32; //binary
	packet->data_part.device_type = 128; //"Klavesnice+OLED"
	packet->data_part.items = (uint8_t *)malloc(1);
	packet->data_part.items[0] = 1;
	packet->read_write = 0;
	packet->sleep = 0;
	
	uint8_t *frame_payload = (uint8_t *)malloc(8);
	serialize_hello_packet(packet, frame_payload, 8);
	prepareSendData(0, 1, frame_payload, 8);
	appSendData();
	
	APP_WriteString("\r\n[INFO] Hello packet sent!\r\n");
}
```    

*Hello ACK packet*:
``` c
static void createSendHelloAck(NWK_DataInd_t *ind, uint16_t sleep_time, uint8_t command_id) {
	APP_WriteString("[DEBUG] Sending ACK command: ");
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
```    

General *ACK* packet:
``` c
static void createSendAck(NWK_DataInd_t *ind) {
	prepareSendData(ind->srcAddr, ind->srcEndpoint, ind->data, ind->size);
	appSendData();
}
```    
## Files
| Option | Description |
| ------ | ----------- |
| device.h  | Definition of devices & device state machine. |
| packet_parser.c | Functions for serialization & parsing packets. |
| packet_parser.h | Definition of functions for serialization & parsing packets. |
| packets.h | Definition of packet structures. |

## How to use (sample communication)
For this example gateway uses address 0x00 and keyboard uses 0x01. Code from *gateway* branch can be ran on both devices, only address needs to be changed.
Devices use *HelloPacket* to connect to gateway. This packet needs to be the first one sent to gateway, otherwise gateway ignores any other packets received from unknown devices.

Example communication (Hello):

 - Device initializes

``` c
[INFO] APP initiated!
------------------------------------------------ 
``` 
 - Device sends HelloPacket to gateway using "hello" command (input from user using keyboard)
``` c
hello    
[INFO] Hello packet sent!    
[INFO] Send complete!
[DEBUG] Received packet: 2    // HelloAck
[DEBUG] Sending ACK command: 1 // ACK
[DEBUG] ACK to Hello ACK sent!
[INFO] Send complete! // SUCCESS

//gateway should have the following output:
[INFO]  New device connected!
[DEBUG] Received packet: 1 //Hello packet
[DEBUG] Sending ACK command: 2 //HelloACK
[DEBUG] Hello ACK sent!
[INFO]  Send complete!
[DEBUG] Received packet: 3 //ACK
[DEBUG] ACK to Hello ACK received! //ACK to HelloACK received, connection successful
``` 

- After *Hello* procedure is finished, gateway should be in state *Connect* in *Node* list ("nodes" command). Device should also be in *Connect* state on gateway.
``` c
nodes //keyboard
| DEVICE | ADDRESS | STATE |
|   0    |   0     | CONNECT |

nodes //gateway
| DEVICE | ADDRESS | STATE |
|   0    |   1     | CONNECT |
``` 
- This output means successful connection of both devices. Connected device can send *data* packets to gateway. Our program contains test/debug method for sending data ("data" command). This command sends preconfigured data packet to gateway and gateway will print out data received.
``` c
//device
data 
[INFO]  Send complete!
[DEBUG] Received packet: 3 // ACK

//output from gateway
[DEBUG] Received packet: 6 //DataPacket
[DEBUG] Periodical data packet received!
[DEBUG] Data packet:0x01 0x01 0x01 0x00 0x00 0x00 0x01 0x00 0x00 0x00 //printout of data received
---End Of Data---
[INFO]  Send complete! //Sent ACK
``` 
