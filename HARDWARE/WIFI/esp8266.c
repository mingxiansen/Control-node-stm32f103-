#include "esp8266.h"
#define INITFAIL {LCD_Print(0,6,"WIFI-初始化-失败 ");LCD_Print(105,0,"NO");}

int is_con_slave=0;//标记是否连接到从机
int rn_count = 0;//对接收消息中的换行符计数，收到3个\r\n即为正常结束
char wifi_feedback[200];//用于存放接收数据的数组
int data_i=0;//接收数据数组的索引
int time_wait=3;//接收数据等待的时间(3秒)
int receive_state=0;//为0表示还没有收到数据  为1表示正在接收数据 为2表示接收数据的过程正常结束
u8 fac_us=72000000/8000000;	
u16 fac_ms=72000000/8000000*1000;
	
void Net_Init()
{
	LCD_Print(105,0,"AT");//进入初始化模式
	LCD_Print(0,6,"初始化WIFI模块中 ");//可以显示9个汉字或者16个半角字符
	
	//等待模块自身启动完成
	delay_ms(1000);
	
	delay_ms(1000);
	delay_ms(1000);
	
	
	//关闭命令回显
	printf("ATE0\r\n");
	if(Re(10)==1){LCD_Print(0,6,"Close--show---OK");}
	else{LCD_Print(0,6,"WiFi-Init--ERROR");INITFAIL return ;}
	delay_ms(1000);
	
	
	//作为主机连接WiFi热点
	LCD_Print(0,6,"正在连接WiFi...  ");
	
	printf("AT+CWJAP=\"Mon-PC\",\"317000000\"\r\n");
	
	if(Re(10)==1){LCD_Print(0,6,"WiFi--Access--OK");}
	else{LCD_Print(0,6,"WIFI-------ERROR");INITFAIL return ;}
	delay_ms(1000);
	
		
	//作为主机-设置AP热点供从机接入
	printf("AT+CWSAP=\"lift-project\",\"labcat127\",1,4,4\r\n");
	
	if(Re(10)==1){LCD_Print(0,6,"Ad hoc--Init--OK");}
	else{LCD_Print(0,6,"Adhoc-Init-ERROR");INITFAIL return ;}
	delay_ms(1000);

	//设置主机AP模式下的IP
	printf("AT+CIPAP=\"192.168.4.1\"\r\n");
	
	if(Re(10)==1){LCD_Print(0,6,"Ad hoc--Init--OK");}
	else{LCD_Print(0,6,"Adhoc-Init-ERROR");INITFAIL return ;}
	delay_ms(1000);

	//主机设置为多连接
	printf("AT+CIPMUX=1\r\n");
	
	if(Re(10)==1){LCD_Print(0,6,"Ad hoc--Init--OK");}
	else{LCD_Print(0,6,"Adhoc-Init-ERROR");INITFAIL return ;}
	delay_ms(1000);
	

	
	//主机开启服务器模式，端口为8088
	printf("AT+CIPSERVER=1,8088\r\n");
	
	if(Re(10)==1){LCD_Print(0,6,"Server--Init--OK");}
	else{LCD_Print(0,6,"Server-Init-ERROR");INITFAIL return ;}
	delay_ms(1000);
		
	//测试与SensorWeb服务器的链接
	printf("AT+PING=\"115.28.147.177\"\r\n");
	if(Re(10)==1){LCD_Print(0,6,"Ping--Server--OK");}
	else{LCD_Print(0,6,"PingServer-ERROR");INITFAIL return ;}
	delay_ms(1000);
	
	//与服务器建立连接
	printf("AT+CIPSTART=0,\"TCP\",\"115.28.147.177\",80\r\n");
	if(Re(10)==1){LCD_Print(0,6,"Connect-Sever-OK");}
	else{LCD_Print(0,6,"Con-Sever--ERROR");return ;}
	delay_ms(1000);
	
	
	
	/*
	//发送数据部分
	
	//与服务器建立连接
	printf("AT+CIPSTART=0,\"TCP\",\"115.28.147.177\",80\r\n");
	if(Re(10)==1){LCD_Print(0,6,"Connect-Sever-OK");}
	else{LCD_Print(0,6,"Con-Sever--ERROR");return ;}
	delay_ms(1000);
	
	//发送数据
	LCD_Print(0,6,"Send-Data--READY");
	printf("AT+CIPSEND=0,205\r\n");
	if(Re(10)==1){LCD_Print(0,6,"Send--Ready---OK");}
	else{LCD_Print(0,6,"Send-Ready-ERROR");}
	delay_ms(1000);
	
	LCD_Print(0,6,"Send-Data----ing");
	printf("POST /server/submit.php HTTP/1.1\r\n");
	printf("Host: 115.28.147.177\r\n");
	printf("Content-Length: 78\r\n");
	printf("Content-Type: application/x-www-form-urlencoded\r\n");
	printf("\r\n");
	printf("sensor_id=0001&acceleration=1.897&speed=1.462&frequence=54.364&voltage=218.386\r\n");
	
	
	if(Re(10)==1)//确认是否成功发送
	{	
		if(Re(10)==1){LCD_Print(0,6,"Send-Data-OK-ACK");}
		else{LCD_Print(0,6,"Send-Data-NO-ACK");}
	}
	else{LCD_Print(0,6,"Send-Data--ERROR");return;}
	delay_ms(1000);
	
	*/
	
	
	LCD_Print(105,0,"OK");//WiFi模块初始化完成，测试全部成功
	LCD_Print(0,6,"WIFI - 初始化成功");
	
	
	//开启串口接受中断--接收从机的连接信息
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
	
	
	//断开TCP/IP连接
	//printf("AT+CIPCLOSE");
	
	
	//ReceiveData();
	//LCD_Print(0,6,wifi_feedback);
	/*while(1)
	{
		net_flag++;
	delay_ms(1000);
	}*/
	
	//if(strncmp("OK","OK",2) == 0) {net_flag=1;}
	
	//printf("AT+CWJAP=\"Mon-PC\",\"317000000\"\r\n");
	//ReceiveData();
	
	
	//printf("AT+CWQAP\r\n");
	//ReceiveData();
	//连接WIFI
	//printf("AT+CWJAP=\"Mon-PC\",\"317000000\"\r\n");
	//返回 WIFI CONNECTED\r\nWIFI GOT IP\r\n\r\nOK\r\n
	
	//断开WIFI连接
	//printf("AT+CWQAP\r\n");
	//返回 ---成功退出 \r\nOK\r\nWIFI DISCONNECT\r\n
	//     ---推出前就已无连接  \r\nOK\r\n\r\n
	
	//查询是否连接了WIFI
	//printf("AT+CWJAP?");
	//返回 ---有连接  +CWJAP:"Mon-PC","46:fd:52:3f:f8:8c",1,-49\r\n\r\nOK\r\n
	//     ---无连接  No AP\r\n\r\nOK\r\n
	
	//测试与远程服务器的连接(测试是否有接入网络)
	//printf("AT+PING=\"115.28.147.177\"");
	//返回 ---有链接  +45\r\n\r\nOK\r\n   (+45为延迟，大小不一定每次一样)
	//     ---无连接  +timeout\r\n\r\nERROR\r\n
	
	//建立到SensorWeb的连接
	//printf("AT+CIPSTART=\"TCP\",\"115.28.147.177\",80");
	//返回 ---成功  CONNECT\r\n\r\nOK\r\n
	//     ---失败  no ip\r\n\r\nERROR\r\n
	
	//断开TCP/IP连接
	//printf("AT+CIPCLOSE");
	//返回 ---成功  CLOSED\r\n\r\nOK\r\n
	//     ---无连接  \r\nERROR\r\n

	 
	/*
	LCD_Print(0,0,"输出:");
		LCD_Print(35,0,"停止");
		intTo3chars(U,u_chars);
		floatTo5chars(sinFre,fre_chars);
		LCD_Print(0,2,"电压:");
		LCD_Print(36,2,u_chars);
		LCD_Print(62,2,"频率:");
		LCD_Print(96,2,fre_chars);
		
		//脉冲个数显示
		intTo3chars(impulse_num,num_chars);
		intTo3chars(impulse_step,step_chars);
		LCD_Print(0,4,"脉冲数");
		LCD_Print(43,4,num_chars);
		LCD_Print(72,4,"步长");
		LCD_Print(100,4,step_chars);
		
		//网络连接部分
		LCD_Print(68,0,"网络:");
		LCD_Print(105,0,"无");
		LCD_Print(0,6,"检测");
		LCD_Print(30,6,"WiFi");
		LCD_Print(60,6,"模块中");
		*/
		
}

int Re(int seconds)
{
	u32 temp;
	u8 data;
	u16 nms=1000;
	int times=seconds;
	data_i=0;
	
	while( USART_GetFlagStatus(USART3, USART_FLAG_RXNE) == RESET );//等待第一个字节
	
	while(times--)
	{
		SysTick->LOAD=(u32)nms*fac_ms;				//时间加载(SysTick->LOAD为24bit)
		SysTick->VAL =0x00;							//清空计数器
		SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;	//开始倒数  
		do
		{
			if( USART_GetFlagStatus(USART3, USART_FLAG_RXNE) == SET)
			{
				data = USART_ReceiveData(USART3);
				wifi_feedback[data_i]=data ;
				//OK
				if( wifi_feedback[data_i] == 'K' && data_i>=1 && 
					wifi_feedback[data_i-1] == 'O' )
				{
					SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	//关闭计数器
					SysTick->VAL =0X00;       					//清空计数器
					//返回成功信息
					return 1;
				}
				//ERROR
				if( wifi_feedback[data_i] == 'R' && data_i>=4 && 
					wifi_feedback[data_i-1] == 'O' &&
					wifi_feedback[data_i-2] == 'R' &&
					wifi_feedback[data_i-3] == 'R' &&
					wifi_feedback[data_i-4] == 'E' 
				)
				{
					SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	//关闭计数器
					SysTick->VAL =0X00;       					//清空计数器
					//返回失败信息
					return 0;
				}
				
				//接收成功-重新装载倒计时秒数
				times=seconds;
				data_i++;
			}
			
			
			temp=SysTick->CTRL;
		}while((temp&0x01)&&!(temp&(1<<16)));		//等待时间到达   
		SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	//关闭计数器
		SysTick->VAL =0X00;       					//清空计数器	
	}
	return 0;
}


void USART3_IRQHandler(void)                	//串口3中断服务程序
{
	u32 temp;
	u8 data;
	u16 nms=1000;
	int times=10;
	data_i=0;
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
		//失能接收中断--准备处理连续字符
		USART_ITConfig(USART3, USART_IT_RXNE, DISABLE);
		while(times--)
		{
			SysTick->LOAD=(u32)nms*fac_ms;				//时间加载(SysTick->LOAD为24bit)
			SysTick->VAL =0x00;							//清空计数器
			SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;	//开始倒数  
			do
			{
				if( USART_GetFlagStatus(USART3, USART_FLAG_RXNE) == SET)
				{
					data = USART_ReceiveData(USART3);
					wifi_feedback[data_i]=data ;
					//CONNECT
					if( wifi_feedback[data_i] == 'T' && data_i>=6 && 
						wifi_feedback[data_i-1] == 'C' &&
						wifi_feedback[data_i-2] == 'E' &&
						wifi_feedback[data_i-3] == 'N' &&
						wifi_feedback[data_i-4] == 'N' &&
						wifi_feedback[data_i-5] == 'O' &&
						wifi_feedback[data_i-6] == 'C' )
					{
						SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	//关闭计数器
						SysTick->VAL =0X00;       					//清空计数器	
						//返回成功信息
						is_con_slave=1;
						LCD_Print(0,6,"WIFI-从机连接成功");
						return ;
					}
					//ERROR
					if( wifi_feedback[data_i] == 'R' && data_i>=4 && 
						wifi_feedback[data_i-1] == 'O' &&
						wifi_feedback[data_i-2] == 'R' &&
						wifi_feedback[data_i-3] == 'R' &&
						wifi_feedback[data_i-4] == 'E' 
					)
					{
						SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	//关闭计数器
						SysTick->VAL =0X00;       					//清空计数器	
						//返回失败信息
						is_con_slave=0;
						LCD_Print(0,6,"WIFI-从机连接失败");
						//连接从机失败--重新使能接受中断--准备下次连接
						USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
						return ;
					}
					
					//接收成功-重新装载倒计时秒数
					times=10;
					data_i++;
				}
				
				
				temp=SysTick->CTRL;
			}while((temp&0x01)&&!(temp&(1<<16)));		//等待时间到达   
			SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	//关闭计数器
			SysTick->VAL =0X00;       					//清空计数器	
		}
		
     } 
	is_con_slave=0;
	//连接从机失败--重新使能接受中断--准备下次连接
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
} 

int Receive()
{
	time_wait=6;
	//receive_state=0;
	data_i=0;
	rn_count = 0;
	while(receive_state==0);//等待数据发来
	while(receive_state==1)//从接收第一个字节的数据开始，超过6秒钟则认为接受失败
	{
		delay_ms(1000);
		time_wait--;
		if(time_wait==0){return 0;}
		if(receive_state==2){receive_state=0;return 1;}
	}
	return 1;
}

