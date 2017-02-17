#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "lcd.h"
#include "usmart.h"
#include "rtc.h"

int main(void)
{

	RTC_TimeTypeDef RTC_TimeStruct;
	RTC_DateTypeDef RTC_DateStruct;

	u8 tbuf[40];
	u8 t = 0;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	delay_init(168);
	uart_init(115200);

	usmart_dev.init(84);
	LED_Init();
	LCD_Init();
	My_RTC_Init();

	RTC_Set_WakeUp(RTC_WakeUpClock_CK_SPRE_16bits, 0);		//配置WAKE UP中断,1秒钟中断一次

	POINT_COLOR = RED;
	LCD_ShowString(30, 50, 200, 16, 16, "Explorer STM32F4");
	LCD_ShowString(30, 70, 200, 16, 16, "RTC TEST");
	while (1)
	{
		t++;
		if ((t % 10) == 0)
		{
			;
			RTC_GetTime(RTC_Format_BIN, &RTC_TimeStruct);

			sprintf((char*)tbuf, "Time:%02d:%02d:%02d", RTC_TimeStruct.RTC_Hours, RTC_TimeStruct.RTC_Minutes, RTC_TimeStruct.RTC_Seconds);
			LCD_ShowString(30, 140, 210, 16, 16, tbuf);

			RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct);

			sprintf((char*)tbuf, "Date:20%02d-%02d-%02d", RTC_DateStruct.RTC_Year, RTC_DateStruct.RTC_Month, RTC_DateStruct.RTC_Date);
			LCD_ShowString(30, 160, 210, 16, 16, tbuf);
			sprintf((char*)tbuf, "Week:%d", RTC_DateStruct.RTC_WeekDay);
			LCD_ShowString(30, 180, 210, 16, 16, tbuf);
		}
		if ((t % 20) == 0)LED0 = !LED0;
		delay_ms(10);
	}
}
