#include "pwm.h"
#include "led.h"
#include "usart.h"

/**
 * [TIM3_Int_Init : Tout=((arr+1)*(psc+1))/Ft us.]
 * @param arr [auto reload]
 * @param psc [clock pre-frequency]
 */
void TIM14_PWM_Init(u16 arr, u16 psc) {

	// init NVIC and TIMER
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;

	// enable RCC_APB2 timer 14 & RCC_AHB1 GPIOF
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE); 

	GPIO_PinAFConfig(GPIOF,GPIO_PinSource9,GPIO_AF_TIM14);

	// init GPIO
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;           
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;       
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;     
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;       
	GPIO_Init(GPIOF,&GPIO_InitStructure); 

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
  	TIM_OC1Init(TIM14,&TIM_OCInitStructure);                         
	TIM_OC1PreloadConfig(TIM14, TIM_OCPreload_Enable);

	TIM_ARRPreloadConfig(TIM14, ENABLE);

	TIM_Cmd(TIM14, ENABLE);


}

