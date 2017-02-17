#include "stm32f4xx.h"
#include "sys.h"
#include "led.h"
#include "delay.h"
#include "usart.h"
#include "timer.h"

extern u8 TIM5CH1_CAPTURE_STA;	
extern u32 TIM5CH1_CAPTURE_VAL;	
	
int main(void)
{
	long long temp = 0;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	LED_Init();
	delay_init(168);
	uart_init(115200);
	TIM14_PWM_Init(500 - 1, 84 - 1);
	// clock = 84M -> counter = 84M/84 = 1M
	// relaod = 500 -> PWM = 1M/500 = 2kHz
	TIM5_CH1_Cap_Init(0xFFFFFFFF, 84-1);
	

	while (1) {
		delay_ms(10);
		TIM_SetCompare1(TIM14, TIM_GetCapture1(TIM14)+1);
		
		if (TIM_GetCapture1(TIM14) == 300) TIM_SetCompare1(TIM14, 0); 
		if(TIM5CH1_CAPTURE_STA & 0x80){
			temp = TIM5CH1_CAPTURE_STA & 0x3F;
			temp *= 0xFFFFFFFF;
			temp+= TIM5CH1_CAPTURE_VAL;
			printf("HIGH:%lld us\r\n", temp);
			TIM5CH1_CAPTURE_STA = 0;
		}
	}
}

