#include "stm32f4xx.h"
#include "sys.h"
#include "led.h"
#include "delay.h"
#include "usart.h"
#include "tpad.h"

extern u8 TIM5CH1_CAPTURE_STA;	
extern u32 TIM5CH1_CAPTURE_VAL;	
	
int main(void)
{
	u8 t = 0;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	LED_Init();
	delay_init(168);
	uart_init(115200);
	TPAD_Init(8);
	

	while (1) {
		if (TPAD_Scan(0)){
			LED1 =!LED1;
		}
		t++;

		if (t==15)
		{
			t=0;
			LED0 = !LED0;
		}
		delay_ms(10);
	}
}

