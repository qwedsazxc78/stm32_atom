#include "stm32f4xx.h"
#include "sys.h"
#include "usart.h"
#include "delay.h"
#include "led.h"
#include "key.h"
#include "beep.h"
#include "exti.h"
#include "iwdg.h"

int main(void)
{

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	delay_init(168);
	LED_Init();
	KEY_Init();
	delay_ms(100);
	IWDG_Init(4,625);
	
	LED0 = 0;

	while (1) {
		if (KEY_Scan(0) == WK_UP_PRES){
			IWDG_Feed();
		}
		delay_ms(10);
	}
}

