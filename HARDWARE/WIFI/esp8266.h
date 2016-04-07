#ifndef __ESP8266_H
#define __ESP8266_H

#include "stdio.h"	
#include "sys.h" 
#include "usart.h"
#include "oled.h"	  
#include "delay.h"

void Net_Init(void);
int Receive(void);
int Re(int seconds);

extern int is_con_slave;

#endif
