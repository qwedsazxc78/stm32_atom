#ifndef __PWM_H
#define __PWM_H

#include "sys.h"

void TIM14_PWM_Init(u16 arr, u16 psc);
void TIM5_CH1_Cap_Init(u32 arr,u16 psc);

#endif
