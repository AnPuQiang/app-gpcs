#include "sim808.h"
#include "usart.h"
#include "delay.h"	
#include "led.h"   	 
#include "key.h"	 	 	 	 	 
#include "lcd.h" 	  
#include "flash.h" 	 	 
#include "malloc.h"
#include "string.h"    	
#include "usart2.h" 
#include "usart.h"
#include "ff.h"

//检测接收到的命令回复，str为正确的回复
//回复错误返回0，回复正确返回str的位置
u8* sim808_check_cmd(u8 *str)
{
	char *strx=0;
	if(USART_RX_STA&0X8000)	//
	{
		USART_RX_BUF[USART_RX_STA&0X7FFF]=0;//添加结束符
		strx=strstr((const char*)USART_RX_BUF,(const char*)str);
	}
	return (u8*)strx;
}
//发送命令
//cmd：发送的命令字符串，不需要回车，cmd<0XFF发送数字，否则发送字符串
//返回值：0，成功；1，失败
u8 sim808_send_cmd(u8 *cmd,u8 *ack,u16 wait_time)
{	
	u8 res=0;
	USART_RX_STA=0;
	printf("%s\r\n",cmd);
	if(ack&&wait_time)
	{
		while(--wait_time)
		{
			delay_ms(10);
			if(USART_RX_STA&0X8000)
				{
			if(sim808_check_cmd(ack)) break;
				USART_RX_STA=0;
				}
		}
		if(wait_time==0)res=1;
	}
	return res;
	
}


