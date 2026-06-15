#include "stm32f10x.h"                  // Device header
#include "ScreenSerial.h"
#include "camera.h"
#include "BlueSerial.h"
#include "X_board.h"
#include "Z_board.h"
#include "Emm_V5.h"
#include "Delay.h"
#include "fire.h"

float distance=200;
float angle,pitch;
int8_t state,cur_angle;
int16_t X,Y;
float total_yaw;
uint8_t yaw_X;
uint8_t return_flag,fire_flag;

#include <math.h>

#define W 640        // 摄像头分辨率宽
#define H 480        // 摄像头分辨率高
#define FOV_X 60.0f  // 水平视场角度
#define FOV_Y 45.0f  // 垂直视场角度
//计算偏角公式
float calculateYaw(int cx) 
{
	// 水平偏移量（像素相对于中心）
	float dx = cx - W / 2.0f;
	// Yaw 角度（度）
	float yaw_deg = (dx / (W/2.0f)) * (FOV_X/2.0f);
	return yaw_deg;
}

float calculatePitch(int cx) 
{
	// Yaw 角度（度）
	float yaw_deg = cx;
	return yaw_deg;
}

int main(void)
{
	Fire_Init();										//初始化已经拉高充电引脚
	X_board_init();
	Z_board_init();
	ScreenSerial_Init();
	CameraSerial_Init();
	BlueSerial_Init();
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);			//配置NVIC为分组2
	/*NVIC配置*/
	NVIC_InitTypeDef NVIC_InitStructure;					//定义结构体变量
	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;       //选择配置NVIC的UART4线
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;         //指定NVIC线路使能
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;		//指定NVIC线路的抢占优先级为1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;      //指定NVIC线路的响应优先级为1
	NVIC_Init(&NVIC_InitStructure);                         //将结构体变量交给NVIC_Init，配置NVIC外设
	
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;       //选择配置NVIC的USART1线
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;         //指定NVIC线路使能
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;		//指定NVIC线路的抢占优先级为1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;      //指定NVIC线路的响应优先级为1
	NVIC_Init(&NVIC_InitStructure);                         //将结构体变量交给NVIC_Init，配置NVIC外设
	
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;       //选择配置NVIC的USART2线
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;         //指定NVIC线路使能
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;		//指定NVIC线路的抢占优先级为1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;      //指定NVIC线路的响应优先级为1
	NVIC_Init(&NVIC_InitStructure);                         //将结构体变量交给NVIC_Init，配置NVIC外设
	
	NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;       //选择配置NVIC的UART5线
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;         //指定NVIC线路使能
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;		//指定NVIC线路的抢占优先级为1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;      //指定NVIC线路的响应优先级为1
	NVIC_Init(&NVIC_InitStructure);                         //将结构体变量交给NVIC_Init，配置NVIC外设
	
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;       //选择配置NVIC的USART3线
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;         //指定NVIC线路使能
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;		//指定NVIC线路的抢占优先级为1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;      //指定NVIC线路的响应优先级为1
	NVIC_Init(&NVIC_InitStructure);                         //将结构体变量交给NVIC_Init，配置NVIC外设
	
	Delay_s(2);
	//上电角度回零
	while( CameraSerial_RxFlag == 0 );										
	angle = calculateYaw(CameraSerial_RxPacket[0]);										
	CameraSerial_RxFlag = 0;
	if(angle >= 0)
		Emm_V5_Pos_Control(MOTOR_X,1,1,(uint16_t)(angle*9+0.5),0);	//逆时针转动
	else
		Emm_V5_Pos_Control(MOTOR_X,1,0,(uint16_t)(-angle*9+0.5),0);	//顺时针转动
	Delay_ms(1670);
	Emm_V5_Origin_Set_O(MOTOR_X,1,0);			//设置上电位置为零点
	
	while(1)
	{
		if(ScreenSerial_RxFlag == 1)
		{
			if(ScreenSerial_RxPacket[0] == 0x01 || ScreenSerial_RxPacket[0] == 0x02)
			{
				distance += ScreenSerial_RxPacket[4];
				angle = ScreenSerial_RxPacket[8]+ScreenSerial_RxPacket[12]*0.1;					//键入旋转角度
				cur_angle+=angle;													//绝对角度
			}
			else if(ScreenSerial_RxPacket[0] == 0x03)		//切换状态
				state = ScreenSerial_RxPacket[4];
			//边界保护
			if(distance < 200)
				distance=200;
			else if(distance > 300)
				distance=300;
			if(state < 0)
				state=0;
			else if(state > 2)
				state=2;
			ScreenSerial_RxFlag = 0;
			if(state == 0)
			{
				if(angle >= 0)
					Emm_V5_Pos_Control(MOTOR_X,1,1,(uint16_t)(angle*9+0.5),0);	//逆时针转动
				else
					Emm_V5_Pos_Control(MOTOR_X,1,0,(uint16_t)(-angle*9+0.5),0);	//顺时针转动
				Delay_ms(1670);
				//缺少根据距离计算俯仰角的代码以及驱动电机转动的函数
				//pitch=calculate(distance);
				//if(pitch >= 0)
				//	Emm_V5_Pos_Control(MOTOR_Z,1,1,(uint16_t)(pitch*9+0.5),0);	//逆时针转动
				//else
				//	Emm_V5_Pos_Control(MOTOR_Z,1,0,(uint16_t)(-pitch*9+0.5),0);	//顺时针转动
				//Delay_ms(1670);
				while(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_11) == RESET);
				GPIO_ResetBits(GPIOA,GPIO_Pin_7);
				GPIO_SetBits(GPIOA,GPIO_Pin_12);															//发射				
				Delay_ms(100);																								//确保发射完毕
				GPIO_SetBits(GPIOA,GPIO_Pin_7);
				GPIO_ResetBits(GPIOA,GPIO_Pin_12);
			}
		}
		if(state == 1)																				//发挥部分1
		{
			if(return_flag == 0)
			{
				Emm_V5_Origin_Trigger_Return(MOTOR_X,1,0,0);			//回零
				Delay_s(1);
				return_flag = 1;
			}
			if(yaw_X < 10)														//收集偏角数据
			{
				if(CameraSerial_RxFlag == 1)
				{
					X = CameraSerial_RxPacket[0];					//获取靶心与镜头中心的X偏移量
					total_yaw += calculateYaw(X); 			
					yaw_X++;
					CameraSerial_RxFlag = 0;
				}
			}
			else																			//收集完毕-计算-转动-发射
			{
				yaw_X = 0;
				angle = total_yaw / 10.0f;
				
				if(angle >= 0)
					Emm_V5_Pos_Control(MOTOR_X,1,1,(uint16_t)(angle*9+0.5),0);	//逆时针转动
				else
					Emm_V5_Pos_Control(MOTOR_X,1,0,(uint16_t)(-angle*9+0.5),0);	//顺时针转动
				Delay_ms(1670);
				while(BlueSerial_RxFlag == 0);
				distance=BlueSerial_DistanceAvg*0.1;
				//缺少根据距离计算俯仰角的代码以及驱动电机转动的函数
				while(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_11) == RESET);
				GPIO_ResetBits(GPIOA,GPIO_Pin_7);
				GPIO_SetBits(GPIOA,GPIO_Pin_12);															//发射				
				Delay_ms(100);																								//确保发射完毕
				GPIO_SetBits(GPIOA,GPIO_Pin_7);
				GPIO_ResetBits(GPIOA,GPIO_Pin_12);
				
				CameraSerial_RxFlag = 0;
				total_yaw=0;
				return_flag = 0;
				state = 0;
			}
		}
		if(state == 2)																										//发挥部分2
		{
			//转到-30°位置开始
			distance=250;
			//缺少根据距离计算俯仰角的代码以及驱动电机转动的函数
			Emm_V5_Origin_Trigger_Return(MOTOR_X,1,0,0);
			Delay_s(1);
			cur_angle=-30;
			Emm_V5_Pos_Control(MOTOR_X,1,0,30*9,0);
			Delay_ms(1670);
			
			for(uint8_t x=0;x<120;x++)
			{
				if(x < 60)
				{
					Emm_V5_Pos_Control(MOTOR_X,1,1,9,0);
					if(CameraSerial_RxFlag == 1 && GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_11) == SET) //是否接收到新数据且是否充好电
					{
						if(calculateYaw(CameraSerial_RxPacket[0]) <= 2 && calculateYaw(CameraSerial_RxPacket[0]) >= 1 && fire_flag == 0) 	//2°且没发射过
						{
							GPIO_ResetBits(GPIOA,GPIO_Pin_7);
							GPIO_SetBits(GPIOA,GPIO_Pin_12);
							fire_flag = 1;
						}
						CameraSerial_RxFlag = 0;
					}
				}
				else
				{
					Emm_V5_Pos_Control(MOTOR_X,1,0,9,0);
					if(CameraSerial_RxFlag == 1 && GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_11) == SET)
					{
						if(calculateYaw(CameraSerial_RxPacket[0]) >= -2 && calculateYaw(CameraSerial_RxPacket[0]) <= -1 && fire_flag == 0)
						{
							GPIO_ResetBits(GPIOA,GPIO_Pin_7);
							GPIO_SetBits(GPIOA,GPIO_Pin_12);
							fire_flag = 1;
						}
						CameraSerial_RxFlag = 0;
					}
				}
				Delay_ms(60);
			}
			Delay_ms(100);
			GPIO_SetBits(GPIOA,GPIO_Pin_7);
			GPIO_ResetBits(GPIOA,GPIO_Pin_12);
			fire_flag = 0;
			state=0;
		}
		//显示电磁炮数据
		ScreenSerial_SendString("n3.val=");
		ScreenSerial_SendNumber(distance,3);
		for(int x=0;x<3;x++)
		{
			ScreenSerial_SendByte(0xFF);
		}
		ScreenSerial_SendString("n4.val=");
		ScreenSerial_SendNumber(cur_angle,2);
		for(int x=0;x<3;x++)
		{
			ScreenSerial_SendByte(0xFF);
		}
		ScreenSerial_SendString("n5.val=");
		ScreenSerial_SendNumber(state,1);
		for(int x=0;x<3;x++)
		{
			ScreenSerial_SendByte(0xFF);
		}
	}
} 	
