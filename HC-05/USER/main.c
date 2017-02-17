#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "lcd.h"
// #include "hc05.h"
#include "usart2.h"			 	 
#include "string.h"	 
#include "key.h"
//ALIENTEKminiSTM32开发板扩展实验 
//ATK-HC05蓝牙串口模块实验-库函数版本  
//技术支持：www.openedv.com
//广州市星翼电子科技有限公司 
//ALIENTEK战舰STM32开发板实验13
//TFTLCD显示实验  
//技术支持：www.openedv.com
//广州市星翼电子科技有限公司 
 		  
int main(void)
 {	 
	u8 t;
	u8 key;
	u8 sendmask=0;
	u8 sendcnt=0;
	u8 sendbuf[20];	  
	u8 reclen=0;  
	delay_init();	    	 //延时函数初始化	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	uart_init(115200);	 	//串口初始化为9600
	LED_Init();				//初始化与LED连接的硬件接口
	KEY_Init();				//初始化按键
	LCD_Init();				//初始化LCD
	USART2_Init(38400);	//初始化串口2为:38400 for HC-05
   
	POINT_COLOR=RED;
	LCD_ShowString(30,30,200,16,16,"ALIENTEK STM32 ^_^");	
	LCD_ShowString(30,50,200,16,16,"HC05 BLUETOOTH COM TEST");	
	LCD_ShowString(30,70,200,16,16,"ATOM@ALIENTEK");  
	LCD_ShowString(30,110,200,16,16,"ATK-HC05 Standby!");  
  LCD_ShowString(30,160,200,16,16,"Send:");	
	LCD_ShowString(30,180,200,16,16,"Receive:");	

	POINT_COLOR=BLUE;	 
	LCD_ShowString(30,140,200,16,16,"ROLE:Slave ");

 	while(1) 
	{		
		key=KEY_Scan(0);
		if(key==KEY0_PRES)
		{
			sendmask=!sendmask;				//发送/停止发送  	 
			if(sendmask==0)LCD_Fill(30+40,160,240,160+16,WHITE);//清除显示
		}else delay_ms(10);	   
		if(t==50)
		{
			if(sendmask)					//定时发送
			{
				sprintf((char*)sendbuf,"ALIENTEK HC05 %d\r\n",sendcnt);
	  			LCD_ShowString(30+40,160,200,16,16,sendbuf);	//显示发送数据	
				u2_printf("ALIENTEK HC05 %d\r\n",sendcnt);		//发送到蓝牙模块
				sendcnt++;
				if(sendcnt>99)sendcnt=0;
			}
			// HC05_Sta_Show();  	  
			t=0;
			LED0=!LED0; 	     
		}	  
		if(USART2_RX_STA&0X8000)			//接收到一次数据了
		{
			LCD_Fill(30,200,240,320,WHITE);	//清除显示
 			reclen=USART2_RX_STA&0X7FFF;	//得到数据长度
		  	USART2_RX_BUF[reclen]=0;	 	//加入结束符
			if(reclen==9||reclen==8) 		//控制DS1检测
			{
				if(strcmp((const char*)USART2_RX_BUF,"+LED1 ON")==0)LED1=0;	//打开LED1
				if(strcmp((const char*)USART2_RX_BUF,"+LED1 OFF")==0)LED1=1;//关闭LED1
			}
 			LCD_ShowString(30,200,209,119,16,USART2_RX_BUF);//显示接收到的数据
 			USART2_RX_STA=0;	 
		}	 															     				   
		t++;	
	}											    
}
