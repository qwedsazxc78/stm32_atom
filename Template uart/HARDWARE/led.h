#ifndef __LED_H
#define __LED_H

#include "sys.h"

#define LED0 PFout(9);
#define LED1 PFout(9);

void LED_init(void);
#endif
