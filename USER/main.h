#ifndef __MAIN_H
#define __MAIN_H

#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "lcd.h"
#include "usart2.h"
#include "gps.h"
#include "string.h"
#include "key.h"
#include "malloc.h"  
#include "MMC_SD.h" 
#include "ff.h"  
#include "exfuns.h"
#include "sim808.h"
#include "exti.h"
//#include <time.h>
#include "adc.h"


#define MAX 1000000;


 void CIPSEND(void);
 void sendbeat(void);
 void SD(void);
 void send_mark(void);
 void SendEnd(void);
 
 FIL fil;
 FRESULT res;
 UINT bww;
 char buf[100];
 u8 t;	//while�����ڼ���
 __align(4) u8 dtbuf[50];   								//��ӡ������
 
 u8 USART1_TX_BUF[USART2_MAX_RECV_LEN];
 nmea_msg gpsx;
 const char *IP_command = "AT+CIPSTART=\"TCP\",\"123.206.41.146\",\"8080\"";
 const int ID=001;
 int heartbeat;
 char ch = 0X1A;	
 u32 r=0;
 int randNum;	//�����
#endif