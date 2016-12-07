#include "iwdg.h"

void IWDG_Init(u8 prer, u16 rlr){

	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

	IWDG_SetPrescaler(prer);

	IWDG_SetReload(rlr);

	IWDG_ReloadCounter();

	IWDG_Enable();
}

void IWDG_Feed(void){
	IWDG_ReloadCounter();
}