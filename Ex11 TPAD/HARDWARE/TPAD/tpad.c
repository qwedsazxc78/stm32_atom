#include "tpad.h"
#include "delay.h"
#include "usart.h"

#define TPAD_ARR_MAX_VAL 0xFFFFFFFF
vu16 tpad_default_val = 0;

/**
 * init touch pad
 * @param  psc [frequency coefficient]
 * @return     [0: success, 1: fail]
 */
u8 TPAD_Init(u8 psc) {
	u16 buf[10];
	u16 temp;
	u8 i, j;
	TIM2_CH1_Cap_Init(TPAD_ARR_MAX_VAL , psc - 1);
	// get 10 reading data
	for ( i = 0; i < 10; ++i)
	{
		buf[i] = TPAD_Get_Val();
		delay_ms(10);
	}
	// arrange order
	for ( i = 0; i < 9; ++i)
	{
		for ( j = i + 1; j < 10; ++j)
		{
			if (buf[i] > buf[j])
			{
				temp = buf[i];
				buf[i] = buf[j];
				buf[i] = temp;
			}
		}
	}

	temp = 0;
	// average 6 element in array.
	for ( i = 2; i < 8; ++i) temp += buf[i];
	tpad_default_val = temp / 6;
	printf("tpad_default_val: %d\n", tpad_default_val);

	if (tpad_default_val > TPAD_ARR_MAX_VAL / 2) return 1;
	return 0;
}

/**
 * Reset and release capacity voltage and clear counter of timer.
 */
void TPAD_Reset(void) {
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;  //PA5
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_ResetBits(GPIOA, GPIO_Pin_5);

	delay_ms(5);
	TIM_ClearITPendingBit(TIM2, TIM_IT_CC1 | TIM_IT_Update);
	TIM_SetCounter(TIM2, 0);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;  //PA5
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

}

// #define TPAD_GATE_VAL 100; // touch value, valid touch need to larger than tpad_default_val+TPAD_GATE_VAL
/**
 * [TPAD_Scan description]
 * @param  mode [0: no support contious trigger, 1: suppot contious trigger]
 * @return      [0: no touch, 1: touch]
 */
u8 TPAD_Scan(u8 mode) {
	static u8 keyen = 0;
	u8 res = 0, sample = 3;
	u16 rval;
	u8 TPAD_GATE_VAL = 100;

	if (mode) {
		sample = 6;
		keyen = 0;
	}
	rval = TPAD_Get_MaxVal(sample);

	if (rval > (tpad_default_val + TPAD_GATE_VAL) && rval < (10 * tpad_default_val)) {
		if ((keyen == 0) && (rval > (tpad_default_val + TPAD_GATE_VAL))) {
			res = 1;
		}
		keyen = 3;
	}
	if (keyen != 0) keyen--;
	return res;
}
/**
 * get cpature value from timer, if timeout, return the current counter value
 * @return  [capture value]
 */
u16 TPAD_Get_Val(void) {
	TPAD_Reset();
	while (TIM_GetFlagStatus(TIM2, TIM_IT_CC1) == RESET) { // wait to get rising edge
		if (TIM_GetCounter(TIM2) > TPAD_ARR_MAX_VAL - 500) return TIM_GetCounter(TIM2);
	}
	return TIM_GetCapture1(TIM2);
}

/**
 * read n time, get the max
 * @param  n [time]
 * @return   [max value]
 */
u16 TPAD_Get_MaxVal(u8 n) {
	u16 temp = 0;
	u16 res = 0;
	while (n--) {
		temp = TPAD_Get_Val();
		if (temp > res) res = temp;
	}
	return res;
}

void TIM2_CH1_Cap_Init(u32 arr, u16 psc) {
	GPIO_InitTypeDef  GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_ICInitTypeDef  TIM2_ICInitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);  	 //TIM2时钟使能
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); 	//使能PORTA时钟

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_TIM2);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5; //GPIOA5
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	//速度100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;//不带上下拉
	GPIO_Init(GPIOA, &GPIO_InitStructure); //初始化PA5


	//初始化TIM2
	TIM_TimeBaseStructure.TIM_Period = arr; //设定计数器自动重装值
	TIM_TimeBaseStructure.TIM_Prescaler = psc; 	//预分频器
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位

	//初始化通道1
	TIM2_ICInitStructure.TIM_Channel = TIM_Channel_1; //CC1S=01 	选择输入端 IC1映射到TIM2上
	TIM2_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//上升沿捕获
	TIM2_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM2_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //配置输入分频,不分频
	TIM2_ICInitStructure.TIM_ICFilter = 0x00;//IC2F=0000 配置输入滤波器 不滤波
	TIM_ICInit(TIM2, &TIM2_ICInitStructure);//初始化TIM2 IC1

	TIM_Cmd(TIM2, ENABLE ); 	//使能定时器2
}

