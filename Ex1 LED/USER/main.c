/**
  ******************************************************************************
  * @file    Project/STM32F4xx_StdPeriph_Templates/main.c 
  * @author  MCD Application Team
  * @version V1.4.0
  * @date    04-August-2014
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "sys.h"
#include "usart.h"
#include "delay.h"
#include "led.h"

int main(void)
{
	delay_init(168);
	LED_init();
	
	while(1){
		GPIO_ResetBits(GPIOF, GPIO_Pin_9);
		GPIO_SetBits(GPIOF, GPIO_Pin_10);
		delay_ms(500);
		GPIO_ResetBits(GPIOF, GPIO_Pin_10);
		GPIO_SetBits(GPIOF, GPIO_Pin_9);
		delay_ms(1000);
		
		LED0 = 0;
		LED1 = 1;
		delay_ms(500);
		LED0 = 1;
		LED1 = 0;
		delay_ms(1000);
		
		GPIOF->BSRRH = GPIO_Pin_9;
		GPIOF->BSRRL = GPIO_Pin_10;
		delay_ms(500);
		GPIOF->BSRRH = GPIO_Pin_10;
		GPIOF->BSRRL = GPIO_Pin_9;
		delay_ms(1000);
	}
}

