#include "key.h"
#include "delay.h"

void Key_Init(void){

	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOE, ENABLE);
	// init config on GPIOE pin2,3,and 4 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2| GPIO_Pin_3 |GPIO_Pin_4 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

u8 Key_Scan(u8 mode) {
	static u8 key_up = 1;
	if (mode) key_up = 1;

	if (key_up && (KEY0 == 0 || KEY1 == 0 || KEY2 == 0 || WK_UP == 1)) {
		delay_ms(10);
		key_up = 0;
		if (KEY0 == 0) return 1;
		else if (KEY1 == 0) return 2;
		else if (KEY2 == 0) return 3;
		else if (WK_UP == 1) return 4;
	} else if (KEY0 == 1 || KEY1 == 1 || KEY2 == 1 || WK_UP == 0) key_up = 1;
	return 0;


}
