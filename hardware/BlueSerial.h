#ifndef __BLUE_SERIAL_H
#define __BLUE_SERIAL_H
#include "stm32f10x.h"                  // Device header

#include <stdio.h>

extern uint16_t BlueSerial_DistanceAvg;
extern uint8_t BlueSerial_RxFlag;

void BlueSerial_Init(void);
void BlueSerial_SendByte(uint8_t Byte);
void BlueSerial_SendArray(uint8_t *Array, uint16_t Length);
void BlueSerial_SendString(char *String);
void BlueSerial_SendNumber(uint32_t Number, uint8_t Length);
void BlueSerial_Printf(char *format, ...);

#endif
