#ifndef IST_CALIBRATION_H
#define IST_CALIBRATION_H

#include "config.h"
#include <stdlib.h>

void Get_Bias(float *mBias);
void Set_Bias(float *mBias);
void Get_SoftMatrix(float *mSoftMatrix);
void Reset_CollectData(void);
ISTBOOL Process_CollectData(float *new_data, ISTBOOL* isCollected);


#endif
