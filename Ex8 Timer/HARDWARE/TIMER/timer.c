#include "timer.h"
#include "led.h"

/**
 * [TIM3_Int_Init : Tout=((arr+1)*(psc+1))/Ft us.]
 * @param arr [auto reload]
 * @param psc [clock pre-frequency]
 */
void TIM3_Int_Init(u16 arr, u16 psc) {

	// init NVIC and TIMER
	NVIC_InitTypeDef   NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;

	// enable RCC_APB2
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	// init Timer
	TIM_TimeBaseInitStructure.TIM_Period = arr;
	TIM_TimeBaseInitStructure.TIM_Prescaler = psc;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;

	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);

	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM3, ENABLE);


	// config NVIC to TIM3
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

}


void TIM3_IRQHandler(void) {

	if (TIM_GetITStatus(TIM3, TIM_IT_Update == SET)) {
		LED1 = !LED1;
	}
	TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
}
