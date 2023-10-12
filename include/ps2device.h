#ifndef PS2DEVICE_H_H
#define PS2DEVICE_H_H
#include "stdint.h"

extern void wait60PortToread();
extern void wait60_64PortTowirte();
uint32 ps2DeviceInit();


#endif

