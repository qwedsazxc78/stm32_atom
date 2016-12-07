 #ifndef IST_LIB_H
#define IST_LIB_H

#include "config.h"

#define ENABLE_ACCURACY_REPORT
#ifdef ENABLE_ACCURACY_REPORT
typedef union _vec4 {
    struct {
        float x, y, z, w;
    } axis;
    float element[4];
} _vec_t;
#define ISTVEC _vec_t
#endif

//void IST_HandingCalculation(float *mData, float *aData, float *Angular, float *c_mdata);
//void IST_calibration(float input[3], float output[3], float *Radius, ISTBOOL *isCollected);
//int check_accuracy(float ist_mag_rad, float* ist_mag_data, float yaw);
void IST_Execute(float input[3], float *aData, float *output);
void getBias(float bias[4]);
void setBias(float bias[4]);
ISTBOOL isDmaEnable(void);
void setDmaEnable(ISTBOOL);
ISTBOOL isAngularEnable(void);
void setAngularEnable(ISTBOOL);
float getAngular(void);
ISTBOOL isAccuracyEnable(void);
void setAccuracyEnable(ISTBOOL);
int getAccuracy(void);
void setDynamicCalibration(ISTBOOL);
void setSingleCalibration(ISTBOOL);
ISTBOOL isCalibrationEnable(void);
int getAngularInt(void);
int getAngularInt100(void);

#endif
