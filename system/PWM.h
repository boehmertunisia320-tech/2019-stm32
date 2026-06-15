#ifndef __PWM_H
#define __PWM_H

#include "stm32f10x.h" 

void PWM_Init(void);
void TIM2_IRQHandler(void);
//void PWM_SetCompare(uint16_t Compare1,uint16_t Compare2);
	
#endif
