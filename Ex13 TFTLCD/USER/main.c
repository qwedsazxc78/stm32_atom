#include "stm32f4xx.h"
#include "sys.h"
#include "led.h"
#include "delay.h"
#include "usart.h"
#include "lcd.h"

int main(void)
{
	u8 x = 0;
	u8 lcd_id[12];
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	delay_init(168);
	uart_init(115200);

	LED_Init();
	LCD_Init();
	POINT_COLOR = RED;
	sprintf((char*)lcd_id, "LCD ID:%04X", lcddev.id);

	while (1) {
		switch (x) {
		case 0: LCD_Clear(WHITE); break;
		case 1: LCD_Clear(BLACK); break;
		case 2: LCD_Clear(BLUE); break;
		case 3: LCD_Clear(GRED); break;
		case 4: LCD_Clear(GBLUE); break;
		case 5: LCD_Clear(RED); break;
		case 6: LCD_Clear(GREEN); break;
		case 7: LCD_Clear(YELLOW); break;
		case 8: LCD_Clear(BRRED); break;
		case 9: LCD_Clear(GRAY); break;
		case 10: LCD_Clear(DARKBLUE); break;
		case 11: LCD_Clear(GRAYBLUE); break;
		}
		POINT_COLOR = RED;
		LCD_ShowString(30, 40, 210, 24, 24, "STM32F4");
		LCD_ShowString(30, 70, 200, 24, 24, "TFT LCD");
		LCD_ShowString(30, 100, 200, 24, 24, lcd_id );

		x++;

		if (x == 12) x = 0;
		LED0 = !LED0;
		delay_ms(1000);
	}

}

