#ifndef __Screen_SERIAL_H
#define __Screen_SERIAL_H
#include "stm32f10x.h"                  // Device header

#include <stdio.h>

extern signed char ScreenSerial_RxPacket[];
extern uint8_t ScreenSerial_RxFlag;

void ScreenSerial_Init(void);
void ScreenSerial_SendByte(uint8_t Byte);
void ScreenSerial_SendArray(uint8_t *Array, uint16_t Length);
void ScreenSerial_SendString(char *String);
void ScreenSerial_SendNumber(int32_t Number, uint8_t Length);
void ScreenSerial_Printf(char *format, ...);

#endif
