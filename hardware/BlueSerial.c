#include "stm32f10x.h"                  // Device header
#include <stdio.h>
#include <stdarg.h>

uint16_t BlueSerial_DistanceAvg;	//定义12个距离数据的平均值
uint8_t BlueSerial_RxFlag;			//定义蓝牙串口接收的标志位变量

/**
  * 函    数：蓝牙串口初始化
  * 参    数：无
  * 返 回 值：无
  */
void BlueSerial_Init(void)
{
	/*开启时钟*/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);	//开启USART3的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//开启GPIOB的时钟
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);					//将PA10引脚初始化为复用推挽输出
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);					//将PA11引脚初始化为上拉输入
	
	/*USART初始化*/
	USART_InitTypeDef USART_InitStructure;					//定义结构体变量
	USART_InitStructure.USART_BaudRate = 230400;              //波特率
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//硬件流控制，不需要rdwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;	//模式，发送模式和接收模式均选择ode_Rx;
	USART_InitStructure.USART_Parity = USART_Parity_No;     //奇偶校验，不需要
	USART_InitStructure.USART_StopBits = USART_StopBits_1;  //停止位，选择1位
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;		//字长，选择8位8b;
	USART_Init(USART3, &USART_InitStructure);               //将结构体变量交给USART_Init，配置USART3
	
	/*中断输出配置*/
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);			//开启串口接收数据的中断
	
	/*NVIC中断分组*/
//	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);			//配置NVIC为分组2
//	
//	NVIC_InitTypeDef NVIC_InitStructure;					//定义结构体变量
//	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;       //选择配置NVIC的USART3线
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;         //指定NVIC线路使能
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;		//指定NVIC线路的抢占优先级为1;
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;      //指定NVIC线路的响应优先级为1
//	NVIC_Init(&NVIC_InitStructure);                         //将结构体变量交给NVIC_Init，配置NVIC外设
	/*USART使能*/
	USART_Cmd(USART3, ENABLE);								//使能USART3，串口开始运行
}

/**
  * 函    数：蓝牙串口发送一个字节
  * 参    数：Byte 要发送的一个字节
  * 返 回 值：无
  */
void BlueSerial_SendByte(uint8_t Byte)
{
	USART_SendData(USART3, Byte);		//将字节数据写入数据寄存器，写入后USART自动生成时序波形
	while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);	//等待发送完成
	/*下次写入数据寄存器会自动清除发送完成标志位，故此循环后，无需清除标志位*/
}

/**
  * 函    数：蓝牙串口发送一个数组
  * 参    数：Array 要发送数组的首地址
  * 参    数：Length 要发送数组的长度
  * 返 回 值：无
  */
void BlueSerial_SendArray(uint8_t *Array, uint16_t Length)
{
	uint16_t i;
	for (i = 0; i < Length; i ++)			//遍历数组
	{
		BlueSerial_SendByte(Array[i]);		//依次调用BlueSerial_SendByte发送每个字节数据
	}
}

/**
  * 函    数：蓝牙串口发送一个字符串
  * 参    数：String 要发送字符串的首地址
  * 返 回 值：无
  */
void BlueSerial_SendString(char *String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i ++)	//遍历字符数组（字符串），遇到字符串结束标志位后停止
	{
		BlueSerial_SendByte(String[i]);		//依次调用BlueSerial_SendByte发送每个字节数据
	}
}

/**
  * 函    数：次方函数（内部使用）
  * 返 回 值：返回值等于X的Y次方
  */
uint32_t BlueSerial_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;	//设置结果初值为1
	while (Y --)			//执行Y次
	{
		Result *= X;		//将X累乘到结果
	}
	return Result;
}

/**
  * 函    数：蓝牙串口发送数字
  * 参    数：Number 要发送的数字，范围：0~4294967295
  * 参    数：Length 要发送数字的长度，范围：0~10
  * 返 回 值：无
  */
void BlueSerial_SendNumber(uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i ++)		//根据数字长度遍历数字的每一位
	{
		BlueSerial_SendByte(Number / BlueSerial_Pow(10, Length - i - 1) % 10 + '0');	//依次调用BlueSerial_SendByte发送每位数字
	}
}

/**
  * 函    数：自己封装的prinf函数
  * 参    数：format 格式化字符串
  * 参    数：... 可变的参数列表
  * 返 回 值：无
  */
void BlueSerial_Printf(char *format, ...)
{
	char String[100];				//定义字符数组
	va_list arg;					//定义可变参数列表数据类型的变量arg
	va_start(arg, format);			//从format开始，接收参数列表到arg变量
	vsprintf(String, format, arg);	//使用vsprintf打印格式化字符串和参数列表到字符数组中
	va_end(arg);					//结束变量arg
	BlueSerial_SendString(String);	//蓝牙串口发送字符数组（字符串）
}

/**
  * 函    数：USART3中断函数
  * 参    数：无
  * 返 回 值：无
  * 注意事项：此函数为中断函数，无需调用，中断触发后自动执行
  *           函数名为预留的指定名称，可以从启动文件复制
  *           请确保函数名正确，不能有任何差异，否则中断函数将不能进入
  */
void USART3_IRQHandler(void)
{
	static uint8_t RxState = 0;		//定义表示当前状态机状态的静态变量
	static uint8_t RxBuffer[195];	//定义接收数据缓冲区，存储帧头后的195字节数据
	static uint8_t RxCount = 0;		//定义表示当前接收数据位置的静态变量
	static uint8_t HeaderCount = 0;	//定义帧头计数器，统计连续收到的0xAA字节数
	
	if (USART_GetITStatus(USART3, USART_IT_RXNE) == SET)	//判断是否是USART3的接收事件触发的中断
	{
		uint8_t RxData = USART_ReceiveData(USART3);			//读取数据寄存器，存放在接收的数据变量
		
		/*使用状态机的思路，依次处理数据包的不同部分*/
		
		/*当前状态为0，接收帧头*/
		if (RxState == 0)
		{
			if (RxData == 0xAA && BlueSerial_RxFlag == 0)	//如果收到帧头字节，并且上一个数据包已处理完毕
			{
				HeaderCount++;								//帧头计数器自增
				if (HeaderCount >= 4)						//如果连续收到4个0xAA
				{
					RxState = 1;							//置下一个状态
					RxCount = 0;							//数据包的位置归零
					HeaderCount = 0;						//帧头计数器归零
				}
			}
			else											//帧头错误
			{
				HeaderCount = 0;							//帧头计数器归零
			}
		}
		/*当前状态为1，接收数据部分*/
		else if (RxState == 1)
		{
			RxBuffer[RxCount] = RxData;	//将数据存入数据缓冲区的指定位置
			RxCount ++;				//数据包的位置自增
			
			if (RxCount >= 195)		//如果接收完195字节数据
			{
				RxState = 0;			//状态归0
				
				/*计算12个距离数据的平均值*/
				uint32_t distanceSum = 0;
				for (uint8_t i = 0; i < 12; i++)
				{
					uint8_t offset = 6 + i * 15;	//每个点数据偏移15字节，距离在点数据偏移6字节处
					uint16_t distance = RxBuffer[offset] | (RxBuffer[offset + 1] << 8);	//小端格式读取距离
					distanceSum += distance;																						//单位为毫米
				}
				BlueSerial_DistanceAvg = distanceSum / 12;	//计算平均值
				BlueSerial_RxFlag = 1;	//接收数据包标志位置1，成功接收一个数据包
			}
		}
		
		USART_ClearITPendingBit(USART3, USART_IT_RXNE);		//清除标志位
	}
}
