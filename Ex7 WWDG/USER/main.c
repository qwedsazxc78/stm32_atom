#include "stm32f4xx.h"
#include "sys.h"
#include "usart.h"
#include "delay.h"
#include "led.h"
#include "key.h"
#include "beep.h"
#include "exti.h"
#include "wwdg.h"

int main(void)
{

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	delay_init(168);
	LED_Init();
	KEY_Init();
	LED0 = 0;
	delay_ms(300);
	WWDG_Init(0x7F, 0x5F, WWDG_Prescaler_8);

	while (1) {
		 LED0 = 1;
	}
}

