#include "main.h"
/*************************************************************************************************************

**************************************************************************************************************/
 /*
 初始化外设
 */
 
 void INIT_ALL(void)
 {
	delay_init();	    	 //延时函数初始化	  
	uart_init(38400);	 	//串口初始化为9600
 	exfuns_init();		//为fatfs相关变量申请内存				 
	LCD_Init();			//初始化液晶 
	LED_Init();         //LED初始化
	KEY_Init(); 	//初始化按键
 	mem_init();			//初始化内存池
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
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// 设置中断优先级分组2 
	

	while(SD_Initialize())					//检测SD卡
	{
		delay_ms(200);	  
		LED0=!LED0;//DS0闪烁
	}		
	f_mount(fs[0],"0:",1); 					//挂载SD卡 
 	f_mount(fs[1],"1:",1); 					//挂载FLASH.	  
	USART2_Init(38400);	
	if(Ublox_Cfg_Rate(1000,1)!=0)	//设置定位信息更新速度为1000ms,顺便判断GPS模块是否在位. 
	{
   		LCD_ShowString(30,120,200,16,16,"NEO-6M Setting...");
		while((Ublox_Cfg_Rate(1000,1)!=0)&&key)	//持续判断,直到可以检查到NEO-6M,且数据保存成功
		{
			USART2_Init(9600);			//初始化串口2波特率为9600(EEPROM没有保存数据的时候,波特率为9600.)
	  	Ublox_Cfg_Prt(38400);			//重新设置模块的波特率为38400
			Ublox_Cfg_Tp(1000000,100000,1);	//设置PPS为1秒钟输出1次,脉冲宽度为100ms	    
			key=Ublox_Cfg_Cfg_Save();		//保存配置  
		}	  					 
	   	LCD_ShowString(30,120,200,16,16,"NEO-6M Set Done!!");
		delay_ms(500);
		LCD_Fill(30,120,30+200,120+16,WHITE);//清除显示 
	}	 	

	//如果之前有记录，重新上电先算出之前记录的条数，非常重要
	f_open(file, "0:/124.txt", FA_READ );
	r=(*file).fsize/64;
	f_close(file);			 //关闭文件
	
	while(sim808_send_cmd("AT","OK",500)!=0)	//判断sim808是否成功连接
	{
		LCD_ShowString(30,90,200,16,16,"SIM808 unconnected");
		delay_ms(800);
		LCD_ShowString(30,90,200,16,16,"connect  again");
		delay_ms(400);
		LCD_Fill(30,90,30+200,90+16,WHITE);//清除显示 
	}
	LCD_ShowString(30,90,200,16,16,"Sim808 connected!!");
	//查询是否连上GSM/GPRS网络,确认是否已经附着上GPRS服务
	
	delay_ms(500);
	while(sim808_send_cmd("AT+CPIN?","READY",200)!=0)	//判断SIM卡状态
	{
		LCD_Fill(30,90,30+200,90+16,WHITE);//清除显示 
		LCD_ShowString(30,90,200,16,16," Not Ready");
		delay_ms(500);
	}
	LCD_ShowString(30,90,200,16,16,"Ready!!");
	
	delay_ms(1000);

	while(sim808_send_cmd("AT+CREG?","CREG",500)!=0)	
	{
		LCD_Fill(30,90,30+200,90+16,WHITE);//清除显示 
		delay_ms(500);
		LCD_ShowString(30,90,200,16,16," Not registered");
	}
	LCD_Fill(30,90,30+200,90+16,WHITE);//清除显示 
	LCD_ShowString(30,90,200,16,16,"  Registered!! ");
	
	//显示GPRS附着状态
	delay_ms(500);
	while(sim808_send_cmd("AT+CGATT?","CGATT",500)!=0)	
	{
		LCD_Fill(30,90,30+200,90+16,WHITE);//清除显示 
		LCD_ShowString(30,90,200,16,16," GPRS ERROR ");
		delay_ms(500);
	}
	LCD_Fill(30,90,30+200,90+16,WHITE);//清除显示 
	LCD_ShowString(30,90,200,16,16," GPRS Ready！！ ");
	delay_ms(500);
	LCD_Fill(30,90,30+200,90+16,WHITE);//清除显示 
	
	delay_ms(500);
	while(sim808_send_cmd("AT+CIPMODE=1","OK",500)!=0)	
	{
		LCD_Fill(30,90,30+200,90+16,WHITE);//清除显示 
		LCD_ShowString(30,90,200,16,16," TOUCHUAN ERROR ");
		delay_ms(500);
	}
	LCD_Fill(30,90,30+200,90+16,WHITE);//清除显示 
	LCD_ShowString(30,90,200,16,16,"TRANSPARENT SENDING");
	delay_ms(500);
	LCD_Fill(30,90,30+200,90+16,WHITE);//清除显示 	
		
	t=0;
	while(1)
	{
		
		delay_ms(10);
		if(USART2_RX_STA&0X8000)		//接收到一次数据了（判断USART2_RX最高位为1，即发送准备完成）
		{
			rxlen=USART2_RX_STA&0X7FFF;	//得到数据长度
			for(i=0;i<rxlen;i++)
			USART1_TX_BUF[i]=USART2_RX_BUF[i];	   
 			USART2_RX_STA=0;		   	//启动下一次接收
			USART1_TX_BUF[i]=0;			//自动添加结束符
			GPS_Analysis(&gpsx,(u8*)USART1_TX_BUF);//分析字符
			sprintf((char *)dtbuf,"UTC Date:%04d/%02d/%02d   ",gpsx.utc.year,gpsx.utc.month,gpsx.utc.date);	//显示实时时间 
			LCD_ShowString(30,270,200,16,16,dtbuf);		    
			sprintf((char *)dtbuf,"UTC Time:%02d:%02d:%02d   ",gpsx.utc.hour,gpsx.utc.min,gpsx.utc.sec);	//显示UTC时间
			LCD_ShowString(30,290,200,16,16,dtbuf);		  			
		}
		
		
			//上传信息
		if(t>1)	//第一次不发送+++
		{
		delay_ms(1500);
		printf("+++");
		delay_ms(1500);
		sim808_send_cmd((u8*)IP_command,"OK",1000);	
		delay_ms(1000);		
		printf("%03d,",ID);//发送ID
		
		printf("%03.11f,",gpsx.longitude/100000.00000000000);	//经度，以度为单位，小数点后11位小数
		printf("%02.12f,",gpsx.latitude/100000.000000000000);	//纬度，以度为单位，小数点后12位小数
		printf("0,0,");//标志位
		printf("%d",randNum);
		LCD_ShowString(30,90,200,16,16,"Send OK!!!!!!");
		delay_ms(1000);
		LCD_Fill(30,90,30+200,90+16,WHITE);//清除显示 
		//delay_ms(5000);
		}
		delay_ms(5000);
		t++;
	} 
}
 

void SD()
{
			res=f_open (file, "124.txt", FA_CREATE_ALWAYS|FA_WRITE);
			res = f_lseek(file,64*r); //移动指针
			f_printf(file,"记录%03d 自动 ",r);	  //写序号
			sprintf(buf,"%04d-%02d-%02d ",gpsx.utc.year,gpsx.utc.month,gpsx.utc.date);	//日期
			f_write(file,buf,11,&br);  //日期
			sprintf(buf,"%02d:%02d:%02d ",gpsx.utc.hour,gpsx.utc.min,gpsx.utc.sec);	//时间
			f_write(file,buf,9,&br);   //时间	
			sprintf(buf,"%1c%02d°%02.4f',",gpsx.nshemi,gpsx.latitude/100000,gpsx.latitude%100000/100000.0*60.0);	//得到纬度字符串	
			f_write(file,buf,14,&br);   //纬度		
			sprintf(buf,"%1c%03d°%02.4f' ",gpsx.ewhemi,gpsx.longitude/100000,gpsx.longitude%100000/100000.0*60.0);	//得到纬度字符串	
			f_write(file,buf,15,&br);   //经度		
			f_printf(file,"\r\n");	 //回车换行
			f_close (file);
			LCD_ShowString(30,210,200,16,16,"record number:     ");
			LCD_ShowNum(142,210,r,5,16);	
			delay_ms(200);
			LED0=!LED0;
			emagnet = 0;	//控制电磁铁
			delay_ms(50000);
			emagnet = 1;	//控制电磁铁
			r++;
			t++;
}
void send_mark()	//中断的形式发送标记位数据
	
{
		delay_ms(1500);
		printf("+++");
		delay_ms(1500);
		sim808_send_cmd((u8*)IP_command,"OK",1000);	
		delay_ms(1000);
		printf("%03d,",ID);//发送ID
		printf("%03.11f,",gpsx.longitude/100000.00000000000);	//经度，以度为单位，小数点后11位小数
		printf("%02.12f,",gpsx.latitude/100000.000000000000);	//纬度，以度为单位，小数点后12位小数
		printf("1,0,");//标志位
		printf("%d",randNum);
		LCD_ShowString(30,90,200,16,16,"Mark OK!!!!");
		delay_ms(1000);
		LCD_Fill(30,90,30+200,90+16,WHITE);//清除显示
}

void SendEnd()
{
		delay_ms(1500);
		printf("+++");
		delay_ms(1500);
		sim808_send_cmd((u8*)IP_command,"OK",1000);	
		delay_ms(1500);
		printf("%03d,",ID);//发送ID
		printf("%03.11f,",gpsx.longitude/100000.00000000000);	//经度，以度为单位，小数点后11位小数
		printf("%02.12f,",gpsx.latitude/100000.000000000000);	//纬度，以度为单位，小数点后12位小数
		printf("1,1,");//标志位
		printf("%d",randNum);
		LCD_ShowString(30,90,200,16,16,"Work Finished");
		delay_ms(1000);
		t++;
}

