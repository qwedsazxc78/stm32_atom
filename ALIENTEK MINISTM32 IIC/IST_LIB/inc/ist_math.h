#ifndef IST_MATH_H
#define IST_MATH_H

#include "config.h"

void SmoothFliter(float new_value, float *pre_value, float *pre_value2, float theta);
void DotMatrix_A_B_to_C(float *C, float *A, int nrows, int ncols, float *B, int mcols);
ISTBOOL InvertMatrix4by4(const float B[], float C[]);
ISTBOOL InvertMatrix3by3(const float invIn[], float invOut[]);
float Get_Distance(const float x[3], const float y[3]);

#endif
