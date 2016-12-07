#ifndef __BEEP_H
#define __BEEP_H

#include "sys.h"

#define BEEP PFout(8);

void Beep_init(void);

#endif
