#include "rtc.h"
#include "led.h"
#include "delay.h"
#include "usart.h"

NVIC_InitTypeDef NVIC_InitStructure;

/**
 * init rtc
 * @return  0:init ok / 1:LSE fail / 2: init fail
 */
u8 My_RTC_Init(void) {
	RTC_InitTypeDef RTC_InitStructure;
	u16 retry = 0x1FFF;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE); // enable PWR clock
	PWR_BackupAccessCmd(ENABLE); 

	if (RTC_ReadBackupRegister(RTC_BKP_DR0) != 0x5050) {
		RCC_LSEConfig(RCC_LSE_ON); // enable LSE
		while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) {
			retry++;
			delay_ms(10);
		}
		if (retry == 0) return 1;

		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE); // set LSE as RTC clock
		RCC_RTCCLKCmd(ENABLE);

		RTC_InitStructure.RTC_AsynchPrediv = 0x7F; // 1~0x7F
		RTC_InitStructure.RTC_SynchPrediv = 0xFF; // 0~0x7FFF
		RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
		RTC_Init(&RTC_InitStructure);

		RTC_Set_Time(15, 33, 50, RTC_H12_AM);
		RTC_Set_Date(16, 12, 23, 4);

		RTC_WriteBackupRegister(RTC_BKP_DR0, 0x5050);

	}
	return 0;

}

//RTC time config
//hour,min,sec
//ampm:@RTC_AM_PM_Definitions  :RTC_H12_AM/RTC_H12_PM
//return:SUCEE(1)
//       ERROR(0)
ErrorStatus RTC_Set_Time(u8 hour, u8 min, u8 sec, u8 ampm) {
	RTC_TimeTypeDef RTC_TimeTypeInitStructure;

	RTC_TimeTypeInitStructure.RTC_Hours = hour;
	RTC_TimeTypeInitStructure.RTC_Minutes = min;
	RTC_TimeTypeInitStructure.RTC_Seconds = sec;
	RTC_TimeTypeInitStructure.RTC_H12 = ampm;

	return RTC_SetTime(RTC_Format_BIN, &RTC_TimeTypeInitStructure);
}

//RTC daste config
//year,month,date:year(0~99),month(1~12),date(0~31)
//week:week(1~7,0,ilegal!)
//return:SUCEE(1)
//       ERROR(0)
ErrorStatus RTC_Set_Date(u8 year, u8 month, u8 date, u8 week) {
	RTC_DateTypeDef RTC_DateTypeInitStructure;
	RTC_DateTypeInitStructure.RTC_Date = date;
	RTC_DateTypeInitStructure.RTC_Month = month;
	RTC_DateTypeInitStructure.RTC_WeekDay = week;
	RTC_DateTypeInitStructure.RTC_Year = year;
	return RTC_SetDate(RTC_Format_BIN, &RTC_DateTypeInitStructure);
}

void RTC_Set_AlarmA(u8 week, u8 hour, u8 min, u8 sec) {
	EXTI_InitTypeDef EXTI_InitStructure;
	RTC_AlarmTypeDef RTC_AlarmTypeInitStructure;
	RTC_TimeTypeDef RTC_TimeTypeInitStructure;

	RTC_AlarmCmd(RTC_Alarm_A, DISABLE);

	RTC_TimeTypeInitStructure.RTC_Hours = hour;
	RTC_TimeTypeInitStructure.RTC_Minutes = min;
	RTC_TimeTypeInitStructure.RTC_Seconds = sec;
	RTC_TimeTypeInitStructure.RTC_H12 = RTC_H12_AM;

	RTC_AlarmTypeInitStructure.RTC_AlarmDateWeekDay = week;
	RTC_AlarmTypeInitStructure.RTC_AlarmDateWeekDaySel = RTC_AlarmDateWeekDaySel_WeekDay; // wake by week
	RTC_AlarmTypeInitStructure.RTC_AlarmMask = RTC_AlarmMask_None;
	RTC_AlarmTypeInitStructure.RTC_AlarmTime = RTC_TimeTypeInitStructure;
	RTC_SetAlarm(RTC_Format_BIN, RTC_Alarm_A, &RTC_AlarmTypeInitStructure);

	RTC_ClearITPendingBit(RTC_IT_ALRA); // clear RTC alarm flag
	EXTI_ClearITPendingBit(EXTI_Line17);

	RTC_ITConfig(RTC_IT_ALRA, ENABLE);
	RTC_AlarmCmd(RTC_Alarm_A, ENABLE);

	EXTI_InitStructure.EXTI_Line = EXTI_Line17;//LINE17
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = RTC_Alarm_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

}

/*wksel:  @ref RTC_Wakeup_Timer_Definitions
#define RTC_WakeUpClock_RTCCLK_Div16        ((uint32_t)0x00000000)
#define RTC_WakeUpClock_RTCCLK_Div8         ((uint32_t)0x00000001)
#define RTC_WakeUpClock_RTCCLK_Div4         ((uint32_t)0x00000002)
#define RTC_WakeUpClock_RTCCLK_Div2         ((uint32_t)0x00000003)
#define RTC_WakeUpClock_CK_SPRE_16bits      ((uint32_t)0x00000004)
#define RTC_WakeUpClock_CK_SPRE_17bits      ((uint32_t)0x00000006)
*/
//cnt:reload value. interrupt when the value is 0
void RTC_Set_WakeUp(u32 wksel, u16 cnt) {
	EXTI_InitTypeDef   EXTI_InitStructure;

	RTC_WakeUpCmd(DISABLE);

	RTC_WakeUpClockConfig(wksel); // wake clock selection

	RTC_SetWakeUpCounter(cnt); // reload registry

	RTC_ClearITPendingBit(RTC_IT_WUT); //clear RTC WAKE UP flag
	EXTI_ClearITPendingBit(EXTI_Line22);

	RTC_ITConfig(RTC_IT_WUT, ENABLE);
	RTC_WakeUpCmd( ENABLE);

	EXTI_InitStructure.EXTI_Line = EXTI_Line22;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);


	NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void RTC_Alarm_IRQHandler(void) {
	if (RTC_GetFlagStatus(RTC_FLAG_ALRAF) == SET) {
		RTC_ClearFlag(RTC_FLAG_ALRAF);
		printf("ALARM A\r\n");
	}
	EXTI_ClearITPendingBit(EXTI_Line17);
}

void RTC_WKUP_IRQHandler(void) {
	if (RTC_GetFlagStatus(RTC_FLAG_WUTF) == SET) {
		RTC_ClearFlag(RTC_FLAG_WUTF);
		LED1 != LED1;
	}
	EXTI_ClearITPendingBit(EXTI_Line22);
}
