#include "main.h"
/*************************************************************************************************************

**************************************************************************************************************/
 /*
 ��ʼ������
 */
 
 void INIT_ALL(void)
 {
	delay_init();	    	 //��ʱ������ʼ��	  
	uart_init(38400);	 	//���ڳ�ʼ��Ϊ9600
 	exfuns_init();		//Ϊfatfs��ر��������ڴ�				 
	LCD_Init();			//��ʼ��Һ�� 
	LED_Init();         //LED��ʼ��
	KEY_Init(); 	//��ʼ������
 	mem_init();			//��ʼ���ڴ��
	EXTIX_Init();
	Adc_Init();
 }
 
 int main(void)
 { 
	u16 adcx;
	u8 key=0XFF;
	u16 i,rxlen;
 	u32 total,free;
	u8 t=0;
	INIT_ALL(); 
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// �����ж����ȼ�����2 
	

	while(SD_Initialize())					//���SD��
	{
		delay_ms(200);	  
		LED0=!LED0;//DS0��˸
	}		
	f_mount(fs[0],"0:",1); 					//����SD�� 
 	f_mount(fs[1],"1:",1); 					//����FLASH.	  
	USART2_Init(38400);	
	if(Ublox_Cfg_Rate(1000,1)!=0)	//���ö�λ��Ϣ�����ٶ�Ϊ1000ms,˳���ж�GPSģ���Ƿ���λ. 
	{
   		LCD_ShowString(30,120,200,16,16,"NEO-6M Setting...");
		while((Ublox_Cfg_Rate(1000,1)!=0)&&key)	//�����ж�,ֱ�����Լ�鵽NEO-6M,�����ݱ���ɹ�
		{
			USART2_Init(9600);			//��ʼ������2������Ϊ9600(EEPROMû�б������ݵ�ʱ��,������Ϊ9600.)
	  	Ublox_Cfg_Prt(38400);			//��������ģ��Ĳ�����Ϊ38400
			Ublox_Cfg_Tp(1000000,100000,1);	//����PPSΪ1�������1��,������Ϊ100ms	    
			key=Ublox_Cfg_Cfg_Save();		//��������  
		}	  					 
	   	LCD_ShowString(30,120,200,16,16,"NEO-6M Set Done!!");
		delay_ms(500);
		LCD_Fill(30,120,30+200,120+16,WHITE);//�����ʾ 
	}	 	

	//���֮ǰ�м�¼�������ϵ������֮ǰ��¼���������ǳ���Ҫ
	f_open(file, "0:/124.txt", FA_READ );
	r=(*file).fsize/64;
	f_close(file);			 //�ر��ļ�
	
	while(sim808_send_cmd("AT","OK",500)!=0)	//�ж�sim808�Ƿ�ɹ�����
	{
		LCD_ShowString(30,90,200,16,16,"SIM808 unconnected");
		delay_ms(800);
		LCD_ShowString(30,90,200,16,16,"connect  again");
		delay_ms(400);
		LCD_Fill(30,90,30+200,90+16,WHITE);//�����ʾ 
	}
	LCD_ShowString(30,90,200,16,16,"Sim808 connected!!");
	//��ѯ�Ƿ�����GSM/GPRS����,ȷ���Ƿ��Ѿ�������GPRS����
	
	delay_ms(500);
	while(sim808_send_cmd("AT+CPIN?","READY",200)!=0)	//�ж�SIM��״̬
	{
		LCD_Fill(30,90,30+200,90+16,WHITE);//�����ʾ 
		LCD_ShowString(30,90,200,16,16," Not Ready");
		delay_ms(500);
	}
	LCD_ShowString(30,90,200,16,16,"Ready!!");
	
	delay_ms(1000);

	while(sim808_send_cmd("AT+CREG?","CREG",500)!=0)	
	{
		LCD_Fill(30,90,30+200,90+16,WHITE);//�����ʾ 
		delay_ms(500);
		LCD_ShowString(30,90,200,16,16," Not registered");
	}
	LCD_Fill(30,90,30+200,90+16,WHITE);//�����ʾ 
	LCD_ShowString(30,90,200,16,16,"  Registered!! ");
	
	//��ʾGPRS����״̬
	delay_ms(500);
	while(sim808_send_cmd("AT+CGATT?","CGATT",500)!=0)	
	{
		LCD_Fill(30,90,30+200,90+16,WHITE);//�����ʾ 
		LCD_ShowString(30,90,200,16,16," GPRS ERROR ");
		delay_ms(500);
	}
	LCD_Fill(30,90,30+200,90+16,WHITE);//�����ʾ 
	LCD_ShowString(30,90,200,16,16," GPRS Ready���� ");
	delay_ms(500);
	LCD_Fill(30,90,30+200,90+16,WHITE);//�����ʾ 
	
	delay_ms(500);
	while(sim808_send_cmd("AT+CIPMODE=1","OK",500)!=0)	
	{
		LCD_Fill(30,90,30+200,90+16,WHITE);//�����ʾ 
		LCD_ShowString(30,90,200,16,16," TOUCHUAN ERROR ");
		delay_ms(500);
	}
	LCD_Fill(30,90,30+200,90+16,WHITE);//�����ʾ 
	LCD_ShowString(30,90,200,16,16,"TRANSPARENT SENDING");
	delay_ms(500);
	LCD_Fill(30,90,30+200,90+16,WHITE);//�����ʾ 	
		
	t=0;
	while(1)
	{
		
		delay_ms(10);
		if(USART2_RX_STA&0X8000)		//���յ�һ�������ˣ��ж�USART2_RX���λΪ1��������׼����ɣ�
		{
			rxlen=USART2_RX_STA&0X7FFF;	//�õ����ݳ���
			for(i=0;i<rxlen;i++)
			USART1_TX_BUF[i]=USART2_RX_BUF[i];	   
 			USART2_RX_STA=0;		   	//������һ�ν���
			USART1_TX_BUF[i]=0;			//�Զ���ӽ�����
			GPS_Analysis(&gpsx,(u8*)USART1_TX_BUF);//�����ַ�
			sprintf((char *)dtbuf,"UTC Date:%04d/%02d/%02d   ",gpsx.utc.year,gpsx.utc.month,gpsx.utc.date);	//��ʾʵʱʱ�� 
			LCD_ShowString(30,270,200,16,16,dtbuf);		    
			sprintf((char *)dtbuf,"UTC Time:%02d:%02d:%02d   ",gpsx.utc.hour,gpsx.utc.min,gpsx.utc.sec);	//��ʾUTCʱ��
			LCD_ShowString(30,290,200,16,16,dtbuf);		  			
		}
		
		
			//�ϴ���Ϣ
		if(t>1)	//��һ�β�����+++
		{
		delay_ms(1500);
		printf("+++");
		delay_ms(1500);
		sim808_send_cmd((u8*)IP_command,"OK",1000);	
		delay_ms(1000);		
		printf("%03d,",ID);//����ID
		
		printf("%03.11f,",gpsx.longitude/100000.00000000000);	//���ȣ��Զ�Ϊ��λ��С�����11λС��
		printf("%02.12f,",gpsx.latitude/100000.000000000000);	//γ�ȣ��Զ�Ϊ��λ��С�����12λС��
		printf("0,0,");//��־λ
		printf("%d",randNum);
		LCD_ShowString(30,90,200,16,16,"Send OK!!!!!!");
		delay_ms(1000);
		LCD_Fill(30,90,30+200,90+16,WHITE);//�����ʾ 
		//delay_ms(5000);
		}
		delay_ms(5000);
		t++;
	} 
}
 

void SD()
{
			res=f_open (file, "124.txt", FA_CREATE_ALWAYS|FA_WRITE);
			res = f_lseek(file,64*r); //�ƶ�ָ��
			f_printf(file,"��¼%03d �Զ� ",r);	  //д���
			sprintf(buf,"%04d-%02d-%02d ",gpsx.utc.year,gpsx.utc.month,gpsx.utc.date);	//����
			f_write(file,buf,11,&br);  //����
			sprintf(buf,"%02d:%02d:%02d ",gpsx.utc.hour,gpsx.utc.min,gpsx.utc.sec);	//ʱ��
			f_write(file,buf,9,&br);   //ʱ��	
			sprintf(buf,"%1c%02d��%02.4f',",gpsx.nshemi,gpsx.latitude/100000,gpsx.latitude%100000/100000.0*60.0);	//�õ�γ���ַ���	
			f_write(file,buf,14,&br);   //γ��		
			sprintf(buf,"%1c%03d��%02.4f' ",gpsx.ewhemi,gpsx.longitude/100000,gpsx.longitude%100000/100000.0*60.0);	//�õ�γ���ַ���	
			f_write(file,buf,15,&br);   //����		
			f_printf(file,"\r\n");	 //�س�����
			f_close (file);
			LCD_ShowString(30,210,200,16,16,"record number:     ");
			LCD_ShowNum(142,210,r,5,16);	
			delay_ms(200);
			LED0=!LED0;
			emagnet = 0;	//���Ƶ����
			delay_ms(50000);
			emagnet = 1;	//���Ƶ����
			r++;
			t++;
}
void send_mark()	//�жϵ���ʽ���ͱ��λ����
	
{
		delay_ms(1500);
		printf("+++");
		delay_ms(1500);
		sim808_send_cmd((u8*)IP_command,"OK",1000);	
		delay_ms(1000);
		printf("%03d,",ID);//����ID
		printf("%03.11f,",gpsx.longitude/100000.00000000000);	//���ȣ��Զ�Ϊ��λ��С�����11λС��
		printf("%02.12f,",gpsx.latitude/100000.000000000000);	//γ�ȣ��Զ�Ϊ��λ��С�����12λС��
		printf("1,0,");//��־λ
		printf("%d",randNum);
		LCD_ShowString(30,90,200,16,16,"Mark OK!!!!");
		delay_ms(1000);
		LCD_Fill(30,90,30+200,90+16,WHITE);//�����ʾ
}

void SendEnd()
{
		delay_ms(1500);
		printf("+++");
		delay_ms(1500);
		sim808_send_cmd((u8*)IP_command,"OK",1000);	
		delay_ms(1500);
		printf("%03d,",ID);//����ID
		printf("%03.11f,",gpsx.longitude/100000.00000000000);	//���ȣ��Զ�Ϊ��λ��С�����11λС��
		printf("%02.12f,",gpsx.latitude/100000.000000000000);	//γ�ȣ��Զ�Ϊ��λ��С�����12λС��
		printf("1,1,");//��־λ
		printf("%d",randNum);
		LCD_ShowString(30,90,200,16,16,"Work Finished");
		delay_ms(1000);
		t++;
}

