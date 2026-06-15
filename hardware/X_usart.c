#include "X_usart.h"

/**********************************************************
***	Emm_V5.0步进电机库函数
***	编写作者：ZHANGDATOU
***	技术支持：张大头步进电机
***	淘宝店铺：https://zhangdatou.taobao.com
***	CSDN博客：http s://blog.csdn.net/zhangdatou666
***	qq交流群：262438510
**********************************************************/

__IO bool X_rxFrameFlag = false;
__IO uint8_t X_rxCmd[FIFO_SIZE] = {0};
__IO uint8_t X_rxCount = 0;

/**
	* @brief   USART1中断函数
	* @param   无
	* @retval  无
	*/
void USART1_IRQHandler(void)
{
	__IO uint16_t i = 0;

/**********************************************************
***	串口接收中断
**********************************************************/
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		// 未完成一帧数据接收，数据存入缓冲队列
		fifo_enQueue((uint8_t)USART1->DR);

		// 清除串口接收中断
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	}

/**********************************************************
***	串口空闲中断
**********************************************************/
	else if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)
	{
		// 先读SR再读DR清除IDLE中断
		USART1->SR; USART1->DR;

		// 获取一帧的接收数据
		X_rxCount = fifo_queueLength(); for(i=0; i < X_rxCount; i++) { X_rxCmd[i] = fifo_deQueue(); }

		// 一帧数据接收完成，置位帧标志位
		X_rxFrameFlag = true;
	}
}

/**
	* @brief   USART发送多个字节
	* @param   无
	* @retval  无
	*/
void X_usart_SendCmd(__IO uint8_t *cmd, uint8_t len)
{
	__IO uint8_t i = 0;
	
	for(i=0; i < len; i++) { X_usart_SendByte(cmd[i]); }
}

/**
	* @brief   USART发送一个字节
	* @param   无
	* @retval  无
	*/
void X_usart_SendByte(uint16_t data)
{
	__IO uint16_t t0 = 0;
	
	USART1->DR = (data & (uint16_t)0x01FF);

	while(!(USART1->SR & USART_FLAG_TXE))
	{
		++t0; if(t0 > 8000)	{	return; }
	}
}
