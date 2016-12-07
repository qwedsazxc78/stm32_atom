#include "stm32f4xx.h"
#include "sys.h"
#include "usart.h"
#include "delay.h"
#include "led.h"
#include "key.h"

int main(void)
{
	u8 t, len;
	u16 times = 0;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	delay_init(168);
	LED_Init();
	uart_init(115200);
	LED0 = 0;

	while (1) {
		// bit 15 is high, uart receive already
		if (USART_RX_STA & 0x8000) {
			// bit 0-13 are uart received len
			len = USART_RX_STA & 0x3fff;
			printf("\r\nthe msg is : \r\n");
			for (t = 0; t < len; t++) {
				USART1->DR = USART_RX_BUF[t];
				while ((USART1->SR & 0x40) == 0);
			}
			printf("\r\n\r\n");
			USART_RX_STA = 0;
		} else {
			times++;
			if (times % 5000 == 0)
			{
				printf("\r\nALIENTEK STM32F407 UART experiemnt\r\n");
				times = 0;
			}
			if (times % 200 == 0)printf("please enter the msg and type enter\r\n");
			if (times % 30 == 0)LED0 = !LED0; 
			delay_ms(10);
		}

	}
}

  