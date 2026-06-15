#ifndef __X_BOARD_H
#define __X_BOARD_H

#include "stm32f10x.h"

/**********************************************************
***	Emm_V5.0步进闭环控制例程
***	编写作者：ZHANGDATOU
***	技术支持：张大头闭环伺服
***	淘宝店铺：https://zhangdatou.taobao.com
***	CSDN博客：http s://blog.csdn.net/zhangdatou666
***	qq交流群：262438510
**********************************************************/

void X_nvic_init(void);
void X_clock_init(void);
void X_usart_init(void);
void X_board_init(void);

#endif
