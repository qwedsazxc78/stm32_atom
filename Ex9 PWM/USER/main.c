#include "stm32f4xx.h"
#include "sys.h"
#include "led.h"
#include "delay.h"
#include "pwm.h"

int main(void)
{
	u16 led0pwmval = 0;
	u8 dir = 1;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	LED_Init();
	delay_init(168);
	TIM14_PWM_Init(500 - 1, 84 - 1);
	// clock = 84M -> counter = 84M/84 = 1M
	// relaod = 500 -> PWM = 1M/500 = 2kHz

	while (1) {
		delay_ms(10);
		if (dir)led0pwmval++;
		else led0pwmval--;

		if (led0pwmval > 300) dir = 0;
		if (led0pwmval == 0) dir = 1;

		TIM_SetCompare1(TIM14, led0pwmval);
	}
}

