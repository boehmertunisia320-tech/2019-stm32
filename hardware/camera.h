#ifndef __CAMERA_H
#define __CAMERA_H
#include "stm32f10x.h"                  // Device header

#include <stdio.h>

extern signed char CameraSerial_RxPacket[];
extern uint8_t CameraSerial_RxFlag;

void CameraSerial_Init(void);
void CameraSerial_SendByte(uint8_t Byte);
void CameraSerial_SendArray(uint8_t *Array, uint16_t Length);
void CameraSerial_SendString(char *String);
void CameraSerial_SendNumber(int32_t Number, uint8_t Length);
void CameraSerial_Printf(char *format, ...);

#endif
