#include "stm32f4xx.h"
#include "sys.h"
#include "usart.h"
#include "delay.h"
#include "led.h"
#include "key.h"
#include "beep.h"
#include "exti.h"

int main(void)
{
	u8 t, len;
	u16 times = 0;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	delay_init(168);
	LED_Init();
	Beep_Init();
	EXTIX_Init();
	uart_init(115200);
	LED0 = 0;

	while (1) {
		printf("show result \r\n");
		delay_ms(1000);
	}
}

