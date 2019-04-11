/*
 * packet_parser.h
 *
 * Created: 22. 3. 2019 21:37:49
 *  Author: Maros
 */ 


#ifndef PACKET_PARSER_H_
#define PACKET_PARSER_H_

#include "packets.h"
#include "device.h"

// from main.h
extern void APP_WriteString(char *string);

//incoming
PacketType get_packet_type(Device device, uint8_t endpoint, uint8_t *frame, uint8_t packet_length);
PacketType process_packet(Device device, uint8_t endpoint, uint8_t *frame, uint8_t packet_length, void *packet_struct);
void detect_data_packet_arrays_size(uint16_t data, uint8_t *item_count);

//outgoing
void debug_packet(uint8_t *packet, uint8_t packet_length);
void serialize_fixed_size_packet(void* packet, uint8_t *frame_payload, uint8_t packet_length);
void serialize_hello_packet(HelloPacket_t *hello_packet, uint8_t *frame_payload, uint8_t packet_length);
void serialize_hello_ack_packet(HelloAckPacket_t *hello_ack_packet, uint8_t *frame_payload);
void serialize_sleep_packet(SleepPacket_t *sleep_packet, uint8_t *frame_payload);
void serialize_reconnect_packet(ReconnectAckPacket_t *reconnect_packet, uint8_t *frame_payload);
void serialize_get_value_packet(GetValuePacket_t *get_value_packet, uint8_t *frame_payload, uint8_t packet_length);
void serialize_set_value_packet(SetValuePacket_t *set_value_packet, uint8_t *frame_payload, uint8_t packet_length, uint8_t item_count);
void serialize_data_packet(DataPacket_t *data_packet, uint8_t *frame_payload, uint8_t packet_length, uint8_t item_count);

#endif /* PACKET_PARSER_H_ */