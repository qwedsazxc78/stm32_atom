#include "timer.h"
#include "led.h"
#include "usart.h"

/**
 * [TIM3_Int_Init : Tout=((arr+1)*(psc+1))/Ft us.]
 * @param arr [auto reload]
 * @param psc [clock pre-frequency]
 */
void TIM14_PWM_Init(u16 arr, u16 psc) {

	// init GPIO and TIMER
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;

	// enable RCC_APB2 timer 14 & RCC_AHB1 GPIOF
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource9, GPIO_AF_TIM14);

	// init GPIO
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOF, &GPIO_InitStructure);

	// init timer
	TIM_TimeBaseInitStructure.TIM_Period = arr;
	TIM_TimeBaseInitStructure.TIM_Prescaler = psc;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM14, &TIM_TimeBaseInitStructure);

	// init timer14 channel1 PWM mode
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
	TIM_OCInitStructure.TIM_Pulse = 0;
	TIM_OC1Init(TIM14, &TIM_OCInitStructure);
	TIM_OC1PreloadConfig(TIM14, TIM_OCPreload_Enable);
	TIM_ARRPreloadConfig(TIM14, ENABLE);

	TIM_Cmd(TIM14, ENABLE);
}

TIM_ICInitTypeDef TIM5_ICInitStructure;

void TIM5_CH1_Cap_Init(u32 arr, u16 psc) {

	// init GPIO and TIMER
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;

	// enable RCC_APB2 timer 5 & RCC_AHB1 GPIOA
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);


	// init GPIO
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_TIM5);

	// init timer
	TIM_TimeBaseInitStructure.TIM_Period = arr;
	TIM_TimeBaseInitStructure.TIM_Prescaler = psc;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseInitStructure);

	// init timer5 channel0 input compare mode
	TIM5_ICInitStructure.TIM_Channel = TIM_Channel_1; //CC1S=01 input IC map to TI1
	TIM5_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
	TIM5_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; // mapping into TI1
	TIM5_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM5_ICInitStructure.TIM_ICFilter = 0x00; //IC1F=0000

	TIM_ICInit(TIM5, &TIM5_ICInitStructure);
	TIM_ITConfig(TIM5, TIM_IT_Update | TIM_IT_CC1, ENABLE); // allow update &  interrupt
	TIM_Cmd(TIM5, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

}
// capture status
//[7]: 0, no capture;1, capture.
//[6]: 0, no capture low level; 1,capture low level.
//[5:0]: capture overflow counter after low level
//   	(for 32 bit timer, 1us counter +1 , overflow time:4294s)
u8 TIM5CH1_CAPTURE_STA = 0;
u32 TIM5CH1_CAPTURE_VAL;

void TIM5_IRQHandler(void)
{

	if ((TIM5CH1_CAPTURE_STA & 0X80) == 0) //还未成功捕获
	{
		if (TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET) //溢出
		{
			if (TIM5CH1_CAPTURE_STA & 0X40) //已经捕获到高电平了
			{
				if ((TIM5CH1_CAPTURE_STA & 0X3F) == 0X3F) //高电平太长了
				{
					TIM5CH1_CAPTURE_STA |= 0X80;		//标记成功捕获了一次
					TIM5CH1_CAPTURE_VAL = 0XFFFFFFFF;
				} else TIM5CH1_CAPTURE_STA++;
			}
		}
		if (TIM_GetITStatus(TIM5, TIM_IT_CC1) != RESET) //捕获1发生捕获事件
		{
			if (TIM5CH1_CAPTURE_STA & 0X40)		//捕获到一个下降沿
			{
				TIM5CH1_CAPTURE_STA |= 0X80;		//标记成功捕获到一次高电平脉宽
				TIM5CH1_CAPTURE_VAL = TIM_GetCapture1(TIM5); //获取当前的捕获值.
				TIM_OC1PolarityConfig(TIM5, TIM_ICPolarity_Rising); //CC1P=0 设置为上升沿捕获
			} else  								//还未开始,第一次捕获上升沿
			{
				TIM5CH1_CAPTURE_STA = 0;			//清空
				TIM5CH1_CAPTURE_VAL = 0;
				TIM5CH1_CAPTURE_STA |= 0X40;		//标记捕获到了上升沿
				TIM_Cmd(TIM5, DISABLE ); 	//关闭定时器5
				TIM_SetCounter(TIM5, 0);
				TIM_OC1PolarityConfig(TIM5, TIM_ICPolarity_Falling);		//CC1P=1 设置为下降沿捕获
				TIM_Cmd(TIM5, ENABLE ); 	//使能定时器5
			}
		}
	}
	TIM_ClearITPendingBit(TIM5, TIM_IT_CC1 | TIM_IT_Update); //清除中断标志位
}

