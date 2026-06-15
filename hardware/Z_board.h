#ifndef __Z_BOARD_H
#define __Z_BOARD_H

#include "stm32f10x.h"

/**********************************************************
***	Emm_V5.0步进闭环控制例程
***	编写作者：ZHANGDATOU
***	技术支持：张大头闭环伺服
***	淘宝店铺：https://zhangdatou.taobao.com
***	CSDN博客：http s://blog.csdn.net/zhangdatou666
***	qq交流群：262438510
**********************************************************/

void Z_nvic_init(void);
void Z_clock_init(void);
void Z_usart_init(void);
void Z_board_init(void);

#endif
