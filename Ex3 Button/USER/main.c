#include "stm32f4xx.h"
#include "sys.h"
#include "usart.h"
#include "delay.h"
#include "led.h"
#include "beep.h"
#include "key.h"

int main(void)
{
	u8 key;
	delay_init(168);
	LED_Init();
	Beep_Init();
	Key_Init();
	LED0 = 0;
	
	while(1){
		key = Key_Scan(0);
		if (key){
			switch (key)
			{
				case WK_UP_PRES:
					BEEP = !BEEP;
					break;
				case KEY0_PRES:
					LED0 = !LED0;
					break;
				case KEY1_PRES:
					LED1 = !LED1;
					break;
				case KEY2_PRES:
					LED0 = !LED0;
					LED1 = !LED1;
					break;
			}
		} else delay_ms(10);
	}
}

