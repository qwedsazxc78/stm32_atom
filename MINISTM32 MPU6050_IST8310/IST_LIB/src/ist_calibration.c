#include "math.h"
#include "float.h"
#include "Config.h"
#include "ist_calibration.h"
#include "IST_LIB.h"
#include "ist_math.h"

#include "stdio.h"

#define IST_SPHERE_DATAMAX_NUM 60
#define IST_FILTER_DATA 4
#define IST_SPHERE_DATAMAX 4
#define IST_SPHERE_DATAMIN 40
#define IST_SPHERE_RADIUS_MIN 10
#define IST_SPHERE_RADIUS_MAX 100

// plane and sphere calibration parameter
// if data is around collected data, it will be discard
#define FILTER_SPHERE_RADII (15.0f)
#define FILTER_PLANE_RADII (3.14159f)
#define FILTER_MAX_MIN_DISTANCE (2 * IST_SPHERE_RADIUS_MIN)
#define FIT_DATA_LEN 9
#define FIT_MAG_FILED_BUFFER 4
#define FIT_MAG_FILED_CHANGE 5
#define FIT_ERROR (80.0f)


// Internal fucniton
ISTBOOL Set_InitedVector(float v[3]);
ISTBOOL Get_IsClosedData(float *new_data, float* mag_collectData, int num);
ISTBOOL Process_LeastSquarMethod(float* rawdata, int raw_num, float *biasnRad);
void Get_MinMaxMagData(float *minMagXYZ, float *maxMagXYZ, float *new_data, int data_num);
ISTBOOL Get_IsFitMinMaxSituation(float *minMagXYZ, float *maxMagXYZ);
void Get_ResidualErr(float *ResidualErr, float DataSetsX, float DataSetsY, float DataSetsZ , float Databias[4]);
// 0123 collect
// float soft_mat[9] = {
//     1.01022, 0.00805615, 0.0137228,
//     0.00869712, 1.01476, 0.0215581,
//     0.0136976, 0.0227572, 0.976219
// };
float soft_mat[9] = {
    0.991286, 0.00467987, -0.0138997,
    0.0047037, 0.968636, 0.0066469,
    -0.0139064, 0.00670957, 1.04172
};

// float bias[3] = {-16.0305299186431,2.27481262926708,0.232155534602557};
float bias[4] = {0};

int data_num = 0, filter_num = 0;
float* collectData;
float maxMagXYZ[3] = {0}, minMagXYZ[3] = {0}, distanceMagXYZ[3] = {0};
float FitErrorData[15] = {0}, FitError = 0;
float sphere_data_dot_sphere_dataT[16] = {0};
float sphere_least_right[4] = {0};

float collect_radius[4];
int radius_num = 0;

void Reset_CollectData(void)
{
    data_num = 0;
}

void Get_Bias(float *mBias)
{
    int i = 0;
    for (i = 0; i < 4; i++) {
        mBias[i] = bias[i];
    }
}


void Set_Bias(float *mBias)
{
    int i = 0;
    for (i = 0; i < 4; i++) {
        bias[i] = mBias[i];
    }
}

void Get_SoftMatrix(float *mSoftMatrix)
{
    int i = 0;
    for (i = 0; i < 9; i++) {
        mSoftMatrix[i] = soft_mat[i];
    }
}

ISTBOOL Set_InitedVector(float v[3])
{
    int i = 0;
    for (i = 0; i < 3; i++) {
        if ((FLT_MAX <= v[i])) {
            return IST_TRUE;
        }
    }
    return IST_FALSE;
}

ISTBOOL Process_CollectData(float *new_data, ISTBOOL* isCollected)
{
    ISTBOOL res;
    float mbias[4] = {0};
    int i = 0;
    *isCollected = IST_FALSE;

    // init collectData
    if (collectData == NULL)
        collectData = (float*)calloc(3 * IST_FILTER_DATA, sizeof(float));

    // if not closed data, add new data to colloected datasets
    if (Get_IsClosedData(new_data, collectData, filter_num) == IST_TRUE)
    {
        //Add new data to collectData to filter the next data
        filter_num = (filter_num + 1) % IST_FILTER_DATA;
        collectData[3 * filter_num + 0] = new_data[0];
        collectData[3 * filter_num + 1] = new_data[1];
        collectData[3 * filter_num + 2] = new_data[2];

        // add a constrait on colleact data, max distance of x,y,z datasets need up to 40uT
        // data_num = 0 , init min,max value
        // data_num = !0, compare if this new point is lageer or smaller than max,min magXYZ
        Get_MinMaxMagData(minMagXYZ, maxMagXYZ, new_data, data_num);
        data_num++;
        res = Process_LeastSquarMethod(new_data, data_num, mbias);

        //printf("~~ data_num:%d\r\n", data_num);
        // collect num = 10, 20, 30, 40, 50 data for checking fit error
        switch (data_num) {
        case 10:
            for (i = 0; i < 3; ++i) {
                FitErrorData[i] = new_data[i];
            } break;
        case 20:
            for (i = 0; i < 3; ++i) {
                FitErrorData[i + 3] = new_data[i];
            } break;
        case 30:
            for (i = 0; i < 3; ++i) {
                FitErrorData[i + 6] = new_data[i];
            } break;
        case 40:
            for (i = 0; i < 3; ++i) {
                FitErrorData[i + 9] = new_data[i];
            } break;
        case 50:
            for (i = 0; i < 3; ++i) {
                FitErrorData[i + 12] = new_data[i];
            } break;
        }


        // get hard iron, release collectData
        if (res == IST_TRUE) {
            // Calibration Fit Err Cost fuction 
            // Residual = x ^ 2 + y ^ 2 + z ^ 2 - x * 2 * HardX - y * 2 * HardY - z * 2 * HardZ + HardX ^ 2 + HardY ^ 2 + HardZ ^ 2 - Radius ^ 2;
            // ResidualErr = Residual * Residual / data_length;
            // Err_cost = 0.5 / (earth_filed ^ 2) * sqrt( Err ^ 2 / data_length);            
            float ResidualErr = 0, tmpFitError = 0;
            for (i = 0; i < IST_FILTER_DATA; ++i) {
                Get_ResidualErr(&ResidualErr, collectData[3 * i], collectData[3 * i + 1], collectData[3 * i + 2], mbias);
            }
            for (i = 0; i < 5; ++i) {
                Get_ResidualErr(&ResidualErr, FitErrorData[3 * i], FitErrorData[3 * i + 1], FitErrorData[3 * i + 2], mbias);
            }
            ResidualErr = ResidualErr * ResidualErr / FIT_DATA_LEN;
            tmpFitError = 0.5 * 1000 * sqrt(ResidualErr) / mbias[3] / mbias[3];
            // printf("~~ FitErr:%f\r\n", tmpFitError);
            
            // Calibration Fit Err Cost fuction Step1 : 
            // Initilize FitError or update new bias if new bias error is small than original one, .
            // Otherwise, current FIT_ERROR would increase such as a decay effect by new calibrated input
            if (FitError == 0 || FitError > tmpFitError) {
                FitError = tmpFitError;
                // printf("~~ Sucess: current %f \r\n", FitError);
                if (FitError < FIT_ERROR) {
                    for (i = 0; i < 4; ++i)
                        bias[i] = mbias[i];
                }
            } else {
                FitError += FitError / tmpFitError * 10;
                // printf("~~ FitErr: new  %f \r\n", FitError);
            }

            // Calibration Fit Err Cost fuction Step2 : 
            // Check if environment radius change, the fourth element will calcuate average
            // If new environment mag field is differnet over FIT_MAG_FILED_CHANGE, update bias and re-fitting
            collect_radius[radius_num] = mbias[3];
            radius_num ++;
            if (radius_num == FIT_MAG_FILED_BUFFER) {
                radius_num = 0;
                // get average value
                for (i = 0; i < FIT_MAG_FILED_BUFFER - 1; ++i) {
                    collect_radius[3] += collect_radius[i];
                }
                collect_radius[3] /= FIT_MAG_FILED_BUFFER;

                // check if current bias is different with environment mag field
                if (abs(bias[3] - collect_radius[3]) > FIT_MAG_FILED_CHANGE) {
                    FitError = tmpFitError;
                    for (i = 0; i < 4; ++i)
                        bias[i] = mbias[i];
                    printf("~~ Sucess: mag field change %f \r\n", FitError);
                }
            }

            // release memory
            free(collectData);
            collectData = NULL;
        }

        // data_num reach the max value, release collectData
        if (data_num == IST_SPHERE_DATAMAX_NUM) {
            data_num = 0;
            free(collectData);
            collectData = NULL;
        }

        return res;
    }
    else {
        return IST_FALSE;
    }
}

void Get_ResidualErr(float * ResidualErr, float DataSetsX, float DataSetsY, float DataSetsZ , float Databias[4])
{
    *ResidualErr += powf(DataSetsX , 2.0) + powf(DataSetsY, 2.0) + powf(DataSetsZ, 2.0);
    *ResidualErr += -2 * DataSetsX * Databias[0] - 2 * DataSetsY * Databias[1] - 2 * DataSetsZ  * Databias[2];
    *ResidualErr += powf(Databias[0] , 2) + powf(Databias[1] , 2)  + powf(Databias[2] , 2)  - powf(Databias[3] , 2);
}


ISTBOOL Get_IsClosedData(float * new_data, float * mag_collectData, int num)
{
    int i;
    double radii;

    for (i = 0; i < IST_FILTER_DATA; i++) {
        radii = Get_Distance(new_data, mag_collectData);
// define in istlib.h, ENABLE_CALIBRATION_TYPE = 0 for plane, 1 for sphere
#if ENABLE_CALIBRATION_TYPE
        if (radii < FILTER_SPHERE_RADII) {
            return IST_FALSE;
        }
#else
        if (radii < FILTER_PLANE_RADII) {
            return IST_FALSE;
        }
#endif
    }
    return IST_TRUE;
}

ISTBOOL Process_LeastSquarMethod(float * rawdata, int raw_num, float * biasnRad)
{
    ISTBOOL res = IST_FALSE;
    float inv_sphere_data_dot_sphere_dataT[4 * 4] = {0};
    float invmat[9];
    int i = 0, j = 0;
    float rad = 0;
    float raw_data[3] = {0};
    float temp_sphere_data[3] = {0.0f, 0.0f, 0.0f}, sphere_data[3] = {0};
    float temp_bias[3] = {0}, bias_without_soft[3] = {0};
    float xyz[3] = {0};
    float deltaA[16] = {0}, deltaB[4] = {0};

    if (Set_InitedVector(rawdata))
        goto EXIT;
    // trasnlate the raw data to the sphere data
    //    for(i = 0; i < ndata; i++){
    raw_data[0] = rawdata[0];
    raw_data[1] = rawdata[1];
    raw_data[2] = rawdata[2];

    DotMatrix_A_B_to_C(temp_sphere_data, soft_mat, 3, 3, raw_data, 1);

    sphere_data[0] = temp_sphere_data[0];
    sphere_data[1] = temp_sphere_data[1];
    sphere_data[2] = temp_sphere_data[2];

    //AX = B
    xyz[0] = (sphere_data[0] * 2);
    xyz[1] = (sphere_data[1] * 2);
    xyz[2] = (sphere_data[2] * 2);

    //do least squre to find the offset of X, Y, Z and radius R
    deltaA[0] = xyz[0] * xyz[0]; deltaA[4] = xyz[1] * xyz[0]; deltaA[8] = xyz[2] * xyz[0]; deltaA[12] = xyz[0];
    deltaA[1] = xyz[0] * xyz[1]; deltaA[5] = xyz[1] * xyz[1]; deltaA[9] = xyz[2] * xyz[1]; deltaA[13] = xyz[1];
    deltaA[2] = xyz[0] * xyz[2]; deltaA[6] = xyz[1] * xyz[2]; deltaA[10] = xyz[2] * xyz[2]; deltaA[14] = xyz[2];
    deltaA[3] = xyz[0]; deltaA[7] = xyz[1]; deltaA[11] = xyz[2]; deltaA[15] = 1;

    deltaB[0] = xyz[0] * (xyz[0] * xyz[0] + xyz[1] * xyz[1] + xyz[2] * xyz[2]) / 4;
    deltaB[1] = xyz[1] * (xyz[0] * xyz[0] + xyz[1] * xyz[1] + xyz[2] * xyz[2]) / 4;
    deltaB[2] = xyz[2] * (xyz[0] * xyz[0] + xyz[1] * xyz[1] + xyz[2] * xyz[2]) / 4;
    deltaB[3] = (xyz[0] * xyz[0] + xyz[1] * xyz[1] + xyz[2] * xyz[2]) / 4;

    for (j = 0; j < 16; j++) {
        sphere_data_dot_sphere_dataT[j] = sphere_data_dot_sphere_dataT[j] + deltaA[j];
    }
    for (j = 0; j < 4; j++) {
        sphere_least_right[j] = sphere_least_right[j] + deltaB[j];
    }

    //Collect IST_SPHERE_DATAMAX_NUM points to do calibration
    if (raw_num < IST_SPHERE_DATAMAX_NUM)
        goto EXIT;

    // for plane calibration, distance (X - Y) < 10
    // for sphere calibration, distance (X - Y),(X - Z),(Y - Z) < 10
    if (Get_IsFitMinMaxSituation(minMagXYZ, maxMagXYZ) == IST_FALSE) {
        goto FAULT;
    }
    //do least squre to find the offset of X, Y, Z and radius R , collect 60 data, inv(A^T * A) -> 4 x 4
    if (!(InvertMatrix4by4(sphere_data_dot_sphere_dataT, inv_sphere_data_dot_sphere_dataT))) {
        goto FAULT;
    }

    DotMatrix_A_B_to_C(biasnRad, inv_sphere_data_dot_sphere_dataT, 4, 4, sphere_least_right, 1);

    // calculate radius
    rad = biasnRad[3];
    rad = (rad + pow(biasnRad[0], 2) + pow(biasnRad[1], 2) + pow(biasnRad[2], 2));
    if (rad < 0)
        goto FAULT;

    rad = sqrt(rad);

    // check if too large or too small sphere radius
    if (rad < IST_SPHERE_RADIUS_MIN || rad > IST_SPHERE_RADIUS_MAX)
        goto FAULT;

    biasnRad[3] = rad;

    // calculate bias without softiron, inv(mat) -> 3 x 3
    if (!(InvertMatrix3by3(soft_mat, invmat))) {
        goto FAULT;
    }
    for (i = 0; i < 3; i++) {
        temp_bias[i] = biasnRad[i];
    }
    DotMatrix_A_B_to_C(bias_without_soft, invmat, 3, 3, temp_bias, 1);

    for (i = 0; i < 3; i++) {
        biasnRad[i] = bias_without_soft[i];
    }

    res = IST_TRUE;
FAULT:

    for (j = 0; j < 16; j++) {
        sphere_data_dot_sphere_dataT[j] = 0;
    }
    for (j = 0; j < 4; j++) {
        sphere_least_right[j] = 0;
    }
EXIT:

    for (i = 0; i < 3; i++) {
        sphere_data[i] = 0;
    }

    return res;
}

void Get_MinMaxMagData(float * minMagXYZ, float * maxMagXYZ, float * new_data, int data_num)
{   int i = 0;
    if (data_num == 0) {
        for (i = 0; i < 3; ++i) {
            minMagXYZ[i] = new_data[i];
            maxMagXYZ[i] = new_data[i];
        }
    }
    else {
        for (i = 0; i < 3; ++i) {
            if (new_data[i] > maxMagXYZ[i])
                maxMagXYZ[i] = new_data[i];
            if (new_data[i] < minMagXYZ[i])
                minMagXYZ[i] = new_data[i];
        }
    }
}

ISTBOOL Get_IsFitMinMaxSituation(float * minMagXYZ, float * maxMagXYZ) {
    int i = 0;
    float distanceMagXYZ[3];

    for (i = 0; i < 3; ++i) {
        distanceMagXYZ[i] = maxMagXYZ[i] - minMagXYZ[i];
    }

// max/min ditance need to be larger than FILTER_MAX_MIN_DISTANCE 20 (2* min radius, 2*15 = 30 -> buffer 10)
#if ENABLE_CALIBRATION_TYPE
    // Max Distance X, Y, Z < 30, Discard this calibration
    if ((distanceMagXYZ[0] < FILTER_MAX_MIN_DISTANCE) ||  (distanceMagXYZ[1] < FILTER_MAX_MIN_DISTANCE) ||  (distanceMagXYZ[2] < FILTER_MAX_MIN_DISTANCE)) {
        return IST_FALSE;
    }
#else
    if ((distanceMagXYZ[0] < FILTER_MAX_MIN_DISTANCE) || (distanceMagXYZ[1] < FILTER_MAX_MIN_DISTANCE)) {
        return IST_FALSE;
    }
#endif
    return IST_TRUE;
}
