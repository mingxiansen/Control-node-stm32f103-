#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "usart.h"
#include "timer.h"
#include "math.h"
#include "oled.h"
#include "esp8266.h"

/*
接线记录区
PA.8  (CH1)   PA.9 (CH2)   Tim1 SPWM  输出口 (输出相同)
PA.10 (CH3)  PA.11 (CH4)   Tim1 SPWM  输出口 (输出相同)

PA.8  (CH1)		-左上		-白 		 (-紫)
PA.9  (CH2)		-右下		-蓝			 (-灰)

PA.10 (CH3)		-左下		-灰		     (-蓝)
PA.11 (CH4)		-右上		-紫			 (-白)

板载按键： Up-开    Down-关  (默认为关)
矩阵键盘： A<->F-  3<->F+   2<->U-  1<->U+      最下面一个口接PC0 其他7条线(连着)接PE2（灰色线）~PF6（棕色线）
OLED VCC-3.3V GND  D0(PF12)~DS(PB13) (4条线连着)
************************************************/

//参数设定区
#define Pi 3.14159265
#define fre 5000 //载波频率
#define tim_Prescaler 2 //分频系数 需要同时调整上下两者的值才能使载波频率的变化生效
#define time(x,f) (3600*x*f/1000000)
#define deadtime  (time(20,fre))
#define transformer2 300 //自耦变压器二次边电压
//可以实现 1Khz 2Khz 5Khz
/***
 CCR=3600 即为0us
 CCR=0    即为1000us
 则每1		即为 1000 / 3600 = 0.2778 us
 则 100us<--> 360
***/
void calculetaTable(float frequence,int V);

//调制参数
float sinFre=50.0;//正弦波频率50Hz
int N;//正弦波周期内采样点
int U=220;//正弦波电压幅度
float a;//调制比

//记录各种状态的标志
int outputstate=0;//标记是否进入持续输出模式
int is_impulse=0;//标记是否进入脉冲模式
int impulse_i=0;//脉冲个数 1个单位为一个正弦波周期 即50个为产生一秒的脉冲
int impulse_num=25;//脉冲个数 1个单位为一个正弦波周期 即50个为产生一秒的脉冲
int impulse_step=5;//脉冲个数调整步长

//存放SPWM占空比的数组
unsigned int sintable_OC1[400],sintable_OC1_backup[400];
unsigned int sintable_OC2[400],sintable_OC2_backup[400];
char isUseBackup=0,flag_newdata=0;
int show_key;
short az;
int main(void)
{	
	int i;//用于循环计数
//	char keychar[4];//显示按键键位
	u8 key=0;//保存按键键位
	//初始化开发板键盘   只初始化上下按键！！  左右按键不起作用
	KEY_Init();
	//初始化矩阵键盘
	MatrixKey_Init();
	//延时函数初始化	
	delay_init();
//	LCD_All_Init(); //初始化LCD屏
	//初始化OLED屏
	OLED_Init();	
	//显示当前数据
	showdata_init();
	//设置NVIC中断分组2 : 2位抢占优先级(中断嵌套次数)，2位响应优先级(中断响应先后顺序)
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	//初始化 串口USART3  波特率921600 8位数据位 1位停止位 0位校验位
	UART_Init(921600);
	
	
	//初始化WIFI模块网络连接--可以通过按键B重新初始化
	//Net_Init();  
	
	//1000ms定时，获取传感器数据
	TIM3_Int_Init(30000,7200);//10Khz的计数频率，计数到5000为500ms
	//初始化 TIM1 SPWM输出  不分频  载波频率 5 Khz 
	TIM1_SPWM_Init(tim_Prescaler);
	for(i=0;i< 400 ;i++)//不能删除，用于把表置为全0占空比
	{
		sintable_OC1[i]=sintable_OC1_backup[i]=3600;
		sintable_OC2[i]=sintable_OC2_backup[i]=3600;
	}
	//计算sin表
	calculetaTable(sinFre,U);
	
	TIM_Cmd(TIM1, ENABLE);  //使能TIM1
	
	//printf("START\r\n");
	
	while(1)
	 {
		 key=KEY_Scan(0);	//得到键值
				if(key)
			{						   
				switch(key)
				{	
					//Key0  右
					//case 1:	 break;
					case 2:	//Key1  下
						//U+=5;
					TIM_CtrlPWMOutputs(TIM1, DISABLE);
					TIM_ITConfig(TIM1,TIM_IT_Update,DISABLE ); 
					outputstate=0;
					LCD_Print(35,0,"停止");
						break;
					//Key2   左
					//case 3:break;	
						
					case 4:	//Key_Up 上
					TIM_CtrlPWMOutputs(TIM1, ENABLE);
					TIM_ITConfig(TIM1,TIM_IT_Update,ENABLE ); 
					outputstate=1;
					LCD_Print(35,0,"启动");
						break;
				}
		}
			
			
			key = MatrixKey_Scan();
			if(key !=0)//LCD_ShowNum(250,450,key,2,24);
			{		
				show_key=key;				
				keyfunc(key);		
				calculetaTable(sinFre,U);
				showdata();
				
				/*
				//显示按键值
				intTo3chars(key,keychar);
				LCD_Print(100,6,keychar);
				*/
				
			}
			else delay_ms(10); 
						
	 }
	 
	
	
 }
int times=0;
 

void calculetaTable(float frequence,int V)
{
	int i=0;//,N_new;
		unsigned int temp;
	//N_new = (int)(10000/frequence);
	//if(N_new != N )
	//{
		  //N  = N_new;
		  //V = 2*frequence;
    N = (int)(fre/frequence);
		a = (float)V / transformer2 ;
		
		if(isUseBackup==0)
		{
			if(N%2==1)//要计算的点，个数为奇数
			{}
				for(i=0;i< N/2;i++)//生成sin数据表占空比变化部分
				{ 
					temp = (unsigned int)(a*sin(2*Pi/N *(i+1))*3600);
					if(temp < deadtime &&  temp!=0 ){temp = deadtime;}
					else if(temp > (3600 - deadtime)){temp = 3600;}
					
					sintable_OC1_backup[i]     = 3600  - temp;
					sintable_OC2_backup[N/2+i] = sintable_OC1_backup[i];
					
				//50hz方波(1)
				//sintable_OC1_backup[i]     = 0;
				//sintable_OC2_backup[N/2+i] = sintable_OC1_backup[i];
				
				}
				/*
				//50hz方波(2)
					sintable_OC1_backup[0]     = 3600;
				sintable_OC2_backup[N/2] = sintable_OC1_backup[0];
				i--;
				sintable_OC1_backup[i]     = 3600;
				sintable_OC2_backup[N/2+i] = sintable_OC1_backup[i];
				i=N-2;
				*/
				
				/*
				//下面是为了确保sin正负交替时占空比为0
				sintable_OC1_backup[i]     = 3600;
				sintable_OC2_backup[N/2+i] = 3600;
				i++;
				*/
				for(;i< N ;i++)//生成sin数据表为0部分
				{ 
					sintable_OC1_backup[i]     = 3600 ;
					sintable_OC2_backup[i-N/2] = 3600;
				}
				
			}
			else
			{
				for(i=0;i< N/2 ;i++)//生成sin数据表占空比变化部分
				{ 
					temp = (unsigned int)(a*sin(2*Pi/N *(i+1))*3600);
				if(temp < deadtime && temp!=0 ){temp = deadtime;}
				else if(temp > (3600 - deadtime)){temp = 3600;}
					
					sintable_OC1[i]     = 3600  -  temp;
					sintable_OC2[N/2+i] = sintable_OC1[i];
				//50hz方波(3)
				//sintable_OC1[i]     = 0;
				//sintable_OC2[N/2+i] = sintable_OC1[i];
				
				}
				//
				/*
				//50hz方波(4)
				sintable_OC1[0]     = 3600;
				sintable_OC2[N/2] = sintable_OC1[0];
				i--;
				sintable_OC1[i]     = 3600;
				sintable_OC2[N/2+i] = sintable_OC1[i];
				i=N-2;
				*/
				//
				/*
				//下面是为了确保sin正负交替时占空比为0
				sintable_OC1[i]     = 3600;
				sintable_OC2[N/2+i] = 3600;
				i++;
				*/
				for(;i< N ;i++)//生成sin数据表为0部分
				{ 
					sintable_OC1[i]     = 3600 ;
					sintable_OC2[i-N/2] = 3600;
				}
			}
	flag_newdata = 1;
	//}
}


u8 data_H,data_L,temp,data_1,data_2,data_3,data_4,ttt=0;
 //定时器3中断服务程序
void TIM3_IRQHandler(void)   //TIM3中断
{ 
	if (TIM_GetITStatus(TIM3,TIM_IT_Update) != RESET) //检查指定的TIM中断发生与否:TIM 中断源 
	{
		times++;
		if(is_con_slave==1)
		{
			//失能定时中断
			TIM_ITConfig(TIM3,TIM_IT_Update,DISABLE );
			
			//准备向从机发送数据请求信息
			LCD_Print(0,6,"Send-Request----");
			//设置请求信息长度
			printf("AT+CIPSEND=1,3\r\n");
			if(Re(10)==1)
			{
				while(1)//等待允许发送的指令
				{
					if( USART_GetFlagStatus(USART3, USART_FLAG_RXNE) == SET)
					{if( USART_ReceiveData(USART3) == '>'){break;}}
				}
				//发送数据请求信息
				printf("R\r\n");
				if(Re(10)==1)//接收发送成功标志
				{
					//LCD_Print(0,6,"Send-Request--OK");
					if(Re(10)==1)//接收从机的数据起始标志-OK
					{
						//开始正式接收数据
						while( USART_GetFlagStatus(USART3, USART_FLAG_RXNE) != SET){;}
						{data_H = USART_ReceiveData(USART3);}
						while( USART_GetFlagStatus(USART3, USART_FLAG_RXNE) != SET){;}
						{data_L = USART_ReceiveData(USART3);}
						
						//接收\r\n
						while( USART_GetFlagStatus(USART3, USART_FLAG_RXNE) != SET){;}
						{temp = USART_ReceiveData(USART3);}
						while( USART_GetFlagStatus(USART3, USART_FLAG_RXNE) != SET){;}
						{temp = USART_ReceiveData(USART3);}
						
						az=(((short)data_H)<<8)  + (short)data_L;
						
						LCD_Print(0,6,"Data-Recevie--OK");
					
					}
					
				}
				else{LCD_Print(0,6,"SendRequest-Fail");}
			}
			else{LCD_Print(0,6,"SendRequest-Fail");}
			
					
			//使能定时中断
			TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE );
		}
	}
	TIM_ClearITPendingBit(TIM3, TIM_IT_Update);  //清除TIMx的中断待处理位:TIM 中断源 
		//	TIM1->SR = (uint16_t)~TIM_IT_Update;
}


//定时器1中断服务程序
void TIM1_UP_IRQHandler(void)   //TIM3中断
{
	static int point=0;
	if (TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET) //检查指定的TIM中断发生与否:TIM 中断源 
		{
			if(point>=N)
			{
				//脉冲模式
				if(is_impulse==1)
				{
					impulse_i++;
					if(impulse_i==impulse_num)
					{
						TIM_CtrlPWMOutputs(TIM1, DISABLE);
						TIM_ITConfig(TIM1,TIM_IT_Update,DISABLE );
						is_impulse=0;impulse_i=0;
						LCD_Print(35,0,"停止");
					}
				}
				//
					
				point=0;	
				if(flag_newdata == 1)//若需要输出新的sin表
				{
					 isUseBackup = ~isUseBackup;flag_newdata=0;
				}
			}
			if(isUseBackup==0)
			{
				TIM1->CCR1 = sintable_OC1[point];
				TIM1->CCR2 = sintable_OC1[point];
				TIM1->CCR3 = sintable_OC2[point];
				TIM1->CCR4 = sintable_OC2[point];
			}
			else
			{
				TIM1->CCR1 = sintable_OC1_backup[point];
				TIM1->CCR2 = sintable_OC1_backup[point];
				TIM1->CCR3 = sintable_OC2_backup[point];
				TIM1->CCR4 = sintable_OC2_backup[point];
			}
			point++;	
		}
	   TIM_ClearITPendingBit(TIM1, TIM_IT_Update);  //清除TIMx的中断待处理位:TIM 中断源 
		//	TIM1->SR = (uint16_t)~TIM_IT_Update;
}



