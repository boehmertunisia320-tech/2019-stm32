#ifndef __TIMER_H
#define __TIMER_H
#include "stm32f10x.h"                  // Device headere;

void Timer_Init(void);
void TIM2_IRQHandler(void);
	
#endif
