#pragma once
#ifndef DEVICE_H_
#define DEVICE_H_

#include "packets.h"

enum DeviceState {
	Disconnected, Connected, InSleep
};

typedef struct Device {
	uint16_t id;
	enum DeviceState state;
	enum PacketType last_packet;
} Device;

#endif  // DEVICE_H_