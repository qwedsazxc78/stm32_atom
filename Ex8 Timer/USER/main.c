#include "stm32f4xx.h"
#include "sys.h"
#include "led.h"
#include "delay.h"
#include "timer.h"

int main(void)
{

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	TIM3_Int_Init(5000-1,8400-1); // Tout=((arr+1)*(psc+1))/Ft us. = 5000*8400/84M = 500ms
	LED_Init();
	delay_init(168);
	

	while (1) {
		LED0 = !LED0;
		delay_ms(200);
		
	}
}

