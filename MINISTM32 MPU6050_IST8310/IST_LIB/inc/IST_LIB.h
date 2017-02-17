/**
 * @file IST_LIB.h
 * @author ISENTEK
 * @date 2017-01-20 v2.0.4
 * @brief Isentek eCompass API
 */

#ifndef IST_LIB_H
#define IST_LIB_H

#include "config.h"
#include <stdlib.h>

/** @brief  Customer setting for 2D plane or 3D sphere calibtion
 *  @param  int 2D plane calibtion = 0 , 3D sphere calibtion = 1
 */
#define ENABLE_CALIBRATION_TYPE (0)

/** @brief  Y[n] = factor * X[n] + (1 - factor) * X[n-1]
 *  @param  Filter factor , default = 0.1
 */
#define ANGLE_FILTER_FACTOR (0.1)

/** @brief  Accurancy report enable
 *  @param  Define disable : comment , enable : uncomment
 */
#define ENABLE_ACCURACY_REPORT

void Process_RunCompass(float MagRawData[3], float *AccData, float *MagCalibratedData);

void Get_MagBias(float bias[4]);
void Set_MagBias(float bias[4]);

ISTBOOL Get_IsHeadingAngleEnable(void);
void Set_HeadingAngleEnable(ISTBOOL);
int Get_HeadingAngle(void);
int Get_HeadingAngle100(void);

ISTBOOL Get_AccuracyEnable(void);
void Set_MagAccuracyEnable(ISTBOOL);
int Get_MagAccuracyStatus(void);

void Set_DynamicCalibration(ISTBOOL);
void Set_SingleCalibration(ISTBOOL);
ISTBOOL Get_CalibrationStatus(void);

#endif
