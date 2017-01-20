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

//�����յ�������ظ���strΪ��ȷ�Ļظ�
//�ظ����󷵻�0���ظ���ȷ����str��λ��
u8* sim808_check_cmd(u8 *str)
{
	char *strx=0;
	if(USART_RX_STA&0X8000)	//
	{
		USART_RX_BUF[USART_RX_STA&0X7FFF]=0;//��ӽ�����
		strx=strstr((const char*)USART_RX_BUF,(const char*)str);
	}
	return (u8*)strx;
}
//��������
//cmd�����͵������ַ���������Ҫ�س���cmd<0XFF�������֣��������ַ���
//����ֵ��0���ɹ���1��ʧ��
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


