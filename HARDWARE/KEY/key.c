#include "stm32f10x.h"
#include "key.h"
#include "sys.h" 
#include "delay.h"
#include "oled.h"
#include "stdio.h"	
#include "usart.h"	
#include "esp8266.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK战舰STM32开发板
//按键驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/9/3
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////  
								    
//按键初始化函数
void KEY_Init(void) //IO初始化
{ 
 	GPIO_InitTypeDef GPIO_InitStructure;
 
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOE,ENABLE);//使能PORTA,PORTE时钟

	/*GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4;//KEY0-KEY2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	GPIO_Init(GPIOE, &GPIO_InitStructure);//初始化GPIOE2,3,4*/
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_3;//只初始化KEY1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	GPIO_Init(GPIOE, &GPIO_InitStructure);//初始化GPIOE2,3,4

	//初始化 WK_UP-->GPIOA.0	  下拉输入
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //PA0设置成输入，默认下拉	  
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.0

}

extern float sinFre;//正弦波频率50Hz
extern int N;//正弦波周期内采样点
extern int U;//正弦波电压幅度
extern float a;//调制比

extern int outputstate;//标记是否进入持续输出模式
extern int is_impulse;//标记是否进入脉冲模式
extern int impulse_i;//脉冲个数 1个单位为一个正弦波周期 即50个为产生一秒的脉冲
extern int impulse_num;//脉冲个数 1个单位为一个正弦波周期 即50个为产生一秒的脉冲
extern int impulse_step;//脉冲个数调整步长

//矩阵键盘初始化
void MatrixKey_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE|RCC_APB2Periph_GPIOF,ENABLE);
	//F为输出线  E为输入线
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6 ;			//F
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
 	GPIO_Init(GPIOF, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0 |GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6;			//E
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 		 //上拉输入
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
 	GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	
	GPIO_ResetBits(GPIOF, GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6);
}
/*
	//矩阵键盘朝外放
	1-->D  2-->#  3-->0  4-->*
	5-->C  6-->9  7-->8  8-->7
	9-->B  10->6  11->5  12->4
	13->A  14->3  15->2  16->1
	*/
int MatrixKey_Scan(void)
{
	
	
	if( 
			GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_0)==0 ||
			GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_2)==0 ||
			GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4)==0 ||
			GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_6)==0
		)
	{
		delay_ms(10);//去抖动 
		if( 
				GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_0)==0 ||
				GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_2)==0 ||
				GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4)==0 ||
				GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_6)==0
		)
			{
				
				GPIO_SetBits(GPIOF, GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6);
				GPIO_ResetBits(GPIOF, GPIO_Pin_0);//第一组
				if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_0)==0){while(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_0)==0);GPIO_ResetBits(GPIOF, GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6);	return 1;}
				else if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_2)==0){while(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_2)==0);GPIO_ResetBits(GPIOF, GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6);	return 2;}
				else if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4)==0){while(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4)==0);GPIO_ResetBits(GPIOF, GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6);	return 3;}
				else if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_6)==0){while(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_6)==0);GPIO_ResetBits(GPIOF, GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6);	return 4;}
				GPIO_SetBits(GPIOF, GPIO_Pin_0 );
				
				GPIO_ResetBits(GPIOF, GPIO_Pin_2);//第二组
				if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_0)==0){while(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_0)==0);GPIO_ResetBits(GPIOF, GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6);	return 5;}
				else if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_2)==0){while(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_2)==0);GPIO_ResetBits(GPIOF, GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6);	return 6;}
				else if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4)==0){while(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4)==0);GPIO_ResetBits(GPIOF, GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6);	return 7;}
				else if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_6)==0){while(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_6)==0);GPIO_ResetBits(GPIOF, GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6);	return 8;}
				GPIO_SetBits(GPIOF, GPIO_Pin_2 );
				
				GPIO_ResetBits(GPIOF, GPIO_Pin_4);//第三组
				if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_0)==0){while(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_0)==0);GPIO_ResetBits(GPIOF, GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6);	return 9;}
				else if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_2)==0){while(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_2)==0);GPIO_ResetBits(GPIOF, GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6);	return 10;}
				else if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4)==0){while(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4)==0);GPIO_ResetBits(GPIOF, GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6);	return 11;}
				else if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_6)==0){while(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_6)==0);GPIO_ResetBits(GPIOF, GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6);	return 12;}
				GPIO_SetBits(GPIOF, GPIO_Pin_4 );
			
				GPIO_ResetBits(GPIOF, GPIO_Pin_6);//第四组
				if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_0)==0){while(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_0)==0);GPIO_ResetBits(GPIOF, GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6);	return 13;}
				else if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_2)==0){while(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_2)==0);GPIO_ResetBits(GPIOF, GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6);	return 14;}
				else if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4)==0){while(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4)==0);GPIO_ResetBits(GPIOF, GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6);	return 15;}
				else if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_6)==0){while(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_6)==0);GPIO_ResetBits(GPIOF, GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6);	return 16;}

				GPIO_ResetBits(GPIOF, GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6);
			}
			
	}   
	GPIO_ResetBits(GPIOF, GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6);
	return 0;
}

/*
矩阵键盘按下之后执行的函数
*/
void keyfunc(int key)
{
	switch(key)
	{
		case  1:{/*step-*/impulse_step--;if(impulse_step<=1){impulse_step=1;}}break;
		case  2:{/*step+*/impulse_step++;if(impulse_step>=50){impulse_step=50;}}break;
		case  3:{/*num_impulse-*/impulse_num-=impulse_step;if(impulse_num<=1){impulse_num=1;}}break;
		case  4:{/*num_impulse+*/impulse_num+=impulse_step;if(impulse_num>=500){impulse_num=500;}}break;
		case  5:{/**/}break;
		case  6:{/**/}break;
		case  7:{/**/}break;
		case  8:{/*启动脉冲模式*/if(outputstate==0){is_impulse=1;LCD_Print(35,0,"脉冲");TIM_CtrlPWMOutputs(TIM1, ENABLE);TIM_ITConfig(TIM1,TIM_IT_Update,ENABLE);}}break;
		case  9:{/*初始化WIFI*/Net_Init();}break;
		case 10:{/*开始定时接收数据*/TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE );}break;
		case 11:{/*关闭输出*/TIM_CtrlPWMOutputs(TIM1, DISABLE);TIM_ITConfig(TIM1,TIM_IT_Update,DISABLE );outputstate=0;LCD_Print(35,0,"停止");}break;
		case 12:{/*启动输出*/TIM_CtrlPWMOutputs(TIM1, ENABLE);TIM_ITConfig(TIM1,TIM_IT_Update,ENABLE );outputstate=1;LCD_Print(35,0,"启动");}break;
		case 13:{/*f-*/sinFre-=0.5;if(sinFre<=10){sinFre=0.5;}}break;
		case 14:{/*f+*/sinFre+=0.5;if(sinFre<=60){sinFre=60;}}break;
		case 15:{/*U-*/U-=5;if(U<=10){U=10;}}break;
		case 16:{/*U+*/U+=5;if(U<=250){U=250;}}break;		
	}
}








//按键处理函数
//返回按键值
//mode:0,不支持连续按;1,支持连续按;
//0，没有任何按键按下
//1，KEY0按下
//2，KEY1按下
//3，KEY2按下 
//4，KEY3按下 WK_UP
//注意此函数有响应优先级,KEY0>KEY1>KEY2>KEY3!!
u8 KEY_Scan(u8 mode)
{	 
	static u8 key_up=1;//按键按松开标志
	if(mode)key_up=1;  //支持连按		  
	if(key_up&&(KEY1==0||WK_UP==1))
	{
		delay_ms(10);//去抖动 
		key_up=0;
		if(KEY1==0)return KEY1_PRES;
		else if(WK_UP==1)return WKUP_PRES;
	}else if(KEY1==1&&WK_UP==0)key_up=1; 	    
 	return 0;// 无按键按下
}
