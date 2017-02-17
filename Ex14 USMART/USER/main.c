#include "stm32f4xx.h"
#include "sys.h"
#include "led.h"
#include "delay.h"
#include "usart.h"
#include "lcd.h"
#include "usmart.h"

int main(void)
{
	u8 tbuf[40];
	u8 t = 0;

	RTC_TimeTypeDef RTC_TimeStruct;
	RTC_DateTypeDef RTC_DateStruct;

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	delay_init(168);
	uart_init(115200);
	usmart_dev.init(84);

	LED_Init();
	LCD_Init();
	My_RTC_Init();

	RTC_Set_WakeUp(RTC_WakeUpClock_CK_SPRE_16bits, 0);
	POINT_COLOR = RED;
	LCD_ShowString(30, 50, 210, 16, 16, "STM32F4");
	LCD_ShowString(30, 70, 200, 16, 16, "RTC test");

	while (1) {
		t++;
		if ((t % 10) == 0) {	// 100ms
			RTC_GetTime(RTC_Format_BIN, &RTC_TimeStruct);
			sprintf((char*)tbuf, "Time:%02d,%02d,%02d", RTC_TimeStruct.RTC_Hours, \
			        RTC_TimeStruct.RTC_Minutes, RTC_TimeStruct.RTC_Seconds );
			LCD_ShowString(30, 140, 210, 16, 16, tbuf);

			RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct);
			sprintf((char*)tbuf, "Date:20%02d-%02d-%02d", RTC_DateStruct.RTC_Year, \
			        RTC_DateStruct.RTC_Month, RTC_DateStruct.RTC_Date );
			LCD_ShowString(30, 160, 210, 16, 16, tbuf);
			sprintf((char*)tbuf, "Week:20%02d-%02d-%02d", RTC_DateStruct.RTC_WeekDay);
			LCD_ShowString(30, 180, 210, 16, 16, tbuf);

		}


		if ((t % 20) == 0) {	// 100ms
			LED0 = !LED0;
		}

		delay_ms(10);
	}

}

void led_set(u8 sta) {
	LED1 = sta;
}

void test_fun(void(*ledset)(u8), u8 sta) {
	ledset(sta);
}
