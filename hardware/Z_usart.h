#ifndef __Z_USART_H
#define __Z_USART_H

#include "Z_board.h"
#include "fifo.h"

/**********************************************************
***	Emm_V5.0步进电机库函数
***	编写作者：ZHANGDATOU
***	技术支持：张大头步进电机
***	淘宝店铺：https://zhangdatou.taobao.com
***	CSDN博客：http s://blog.csdn.net/zhangdatou666
***	qq交流群：262438510
**********************************************************/

extern __IO bool Z_rxFrameFlag;
extern __IO uint8_t Z_rxCmd[FIFO_SIZE];
extern __IO uint8_t Z_rxCount;

void Z_usart_SendCmd(__IO uint8_t *cmd, uint8_t len);
void Z_usart_SendByte(uint16_t data);

#endif
