#include "math.h"
#include "Config.h"
#include "IST_LIB.h"
#include "ist_calibration.h"
#include "ist_math.h"

#include "stdio.h"

#define ENABLE_CALIBRATION_NUM      20
#define CALIBRATION_THETA_THRESHOLD 0.5f
#define RADII_TO_DEGREE             57.295827f
#define ENABLE_ACCURACY_REPORT

// ACCURACY_REPORT Part
#ifdef ENABLE_ACCURACY_REPORT
#define IST_HIRON_RAD_MIN 10  // uT
#define IST_HIRON_RAD_MAX 100  // uT
#define MAX_ACCURACY_SIZE   36      //allow 20 data entries
#define ACCURACY_SIZE_SPACE (360/MAX_ACCURACY_SIZE)
#define IST_PI 3.1415926f
#define IST_PI_4 0.785398f
#define ACCURACY_THRESHOLD_LOW      25
#define ACCURACY_THRESHOLD_MEDIUM   15
#define ACCURACY_THRESHOLD_HIGH     5
#define STABLE_COUNT                10

typedef  struct {
    int Max;
    int Min;
} Diff_item;

struct stable_yaw {
    int yaw;
    int count;
};

float *qlist36;
static Diff_item max_item = { -1, -1};
float x = 0.0f;
float fx = 0.0f;
static struct stable_yaw s_yaw = { -1, 0};
static int accuracy = 1;
#endif // End Define ENABLE_ACCURACY_REPORT 

float m_p1[3] = {0}, m_p2[3] = {0};
double pre_vec[3] = {0};
int launch_calibration_num = 0;
ISTBOOL enable_calibration = IST_FALSE;
ISTBOOL m_Get_IsHeadingAngleEnable = IST_FALSE;
ISTBOOL m_Get_AccuracyEnable = IST_FALSE;
ISTBOOL m_isDynamicCalibration = IST_FALSE;
int m_accuracy = 0;
volatile float m_yaw = 0;
float MagFilterCalibratedData[3];

void Get_CalibratedMagData(float out[3], float m[9], float bias[3], float data[3]);
ISTBOOL Get_DynamicCalibrationStartCondition(float* input, ISTBOOL status);
double Process_CalculateAngularVelocity(double v1[3], double v2[3]);
void Process_TiltCompensation(float *mData, float *aData, float *Angular, float *c_mdata);
int Process_CalculateAccuracy(float ist_mag_rad, float* ist_mag_data, float yaw);
void Get_MagLowPassFilterOutput(float input[3], float average[3], float factor);

/** @brief Get Mag sensor bais,
 *  @param bias[4] unit:uT / get magX bais, magY bais, magZ bais, Radii
 */
void Get_MagBias(float bias[4])
{
    Get_Bias(bias);
}

/** @brief Set Mag sensor bais, you can read old bias from EEPROM, and set bias when system reboot or reset process.
 *  @param bias[4]  unit:uT / set magX bais, magY bais, magZ bais, Radii
 */
void Set_MagBias(float bias[4])
{
    Set_Bias(bias);
}

/** @brief Get if heading angle (yaw angle) calculated process enable or not
 *  @return ISTBOOL Enable = 1 , Disable = 0
 */
ISTBOOL Get_IsHeadingAngleEnable(void)
{
    return m_Get_IsHeadingAngleEnable;
}


/** @brief Set heading angle (yaw angle) calculated process enable
 *  @param enable Enable = 1 , Disable = 0
 */
void Set_HeadingAngleEnable(ISTBOOL enable)
{
    m_Get_IsHeadingAngleEnable = enable;
}

/** @brief Get heading angle with int type (more 2 point accurancy)
 *  @return int heading angle * 100 --- ex 180.50 degree -> return 18050
 */
int Get_HeadingAngle100(void)
{
    return (int)(m_yaw * 100);
}

/** @brief Get heading angle with int type
 *  @return int heading angle --- ex 180 degree  -> return 180
 */
int Get_HeadingAngle(void)
{
    return (int)m_yaw;
}

/** @brief Get if Accuracy report enable or not
 *  @return ISTBOOL Enable = 1 , Disable = 0
 */
ISTBOOL Get_AccuracyEnable(void)
{
    return m_Get_AccuracyEnable;
}

/** @brief Set Accuracy report enable
 *  @param enable Enable = 1 , Disable = 0
 */
void Set_MagAccuracyEnable(ISTBOOL enable)
{
    if (enable == IST_TRUE) {
        if (qlist36 == NULL)
            qlist36 = calloc(MAX_ACCURACY_SIZE, sizeof(float));
    }
    else {
        free(qlist36);
        qlist36 = NULL;
    }

    m_Get_AccuracyEnable = enable;
}

/** @brief Get Accuracy report value
 *  @return int Accurancy level : High : 3 - Medium : 2 - Low : 1 - unreliable : 0
 */
int Get_MagAccuracyStatus(void)
{
    return m_accuracy;
}

/** @brief Set Dynamic Calibration enable, if enable this process, ecompass will automatically calibrate when reaching specific condition.
 *         This process
 *  @param enable Enable = 1 , Disable = 0
 */
void Set_DynamicCalibration(ISTBOOL enable)
{
    m_isDynamicCalibration = enable;
}

/** @brief Set Dynamic Calibration enable, if enable this process, ecompass will automatically calibrate when reaching specific condition.
 *  @param enable Enable = 1 , Disable = 0
 */
void Set_SingleCalibration(ISTBOOL enable)
{
    enable_calibration = enable;
}

/** @brief Get if calibration process finish
 *  @return ISTBOOL In calibration process = 1, Finish = 0
 */
ISTBOOL Get_CalibrationStatus(void)
{
    return enable_calibration;
}

/** @brief eCompass runing process, please put the mag and acc data by the specific direction, or the return result will fail.
 *  @param MagRawData[3]    unit:uT / Axis direction : (x,y,z) = (North, East, Up) / Magnetometer sensor data.
 *                          +++ ex : MagRawData[3] = { 40uT, -40uT, 10uT}.
 *  @param *AccData         unit:g or 9.8 / Axis direction : (x,y,z) = (North, East, Up) / Accelerometer sensor data.
 *                          +++ ex : AccData[3] = { 0, 0, 1g} or AccData[3] = { 0, 0, 9.8}
 *  @param *MagCalibratedData unit:uT / Axis direction : (x,y,z) = (North, East, Up) / Calibrated mag data
 */
void Process_RunCompass(float MagRawData[3], float *AccData, float *MagCalibratedData)
{
    int i;
    float soft_matrix[9], bias[4] = {0};
    float c_mdata[2] = {0};
    float angular[3] = {0};

    ISTBOOL isCollected;    //for customer
    if ( m_isDynamicCalibration == IST_TRUE ) {
        //Smooth mdata
        for (i = 0; i < 3; i++) {
            SmoothFliter(MagRawData[i], m_p1 + i, m_p2 + i, 0.1);
        }
        enable_calibration = Get_DynamicCalibrationStartCondition(m_p2, enable_calibration);
    }
    else {
        for (i = 0; i < 3; i++)
            m_p2[i] = MagRawData[i];
    }

    if ( enable_calibration == IST_TRUE) {
        if (Process_CollectData(MagRawData, &isCollected) == IST_TRUE) {
            if ( m_Get_AccuracyEnable == IST_TRUE) {
                for ( i = 0; i < MAX_ACCURACY_SIZE; i++)
                    qlist36[i] = 0;
            }
            enable_calibration = IST_FALSE;
            m_accuracy = 1;
            accuracy = 3;
        }
    }
    else {
        Reset_CollectData();  //set collect data num = 0
    }

    Get_Bias(bias);
    Get_SoftMatrix(soft_matrix);
    Get_CalibratedMagData(MagCalibratedData, soft_matrix, bias, m_p2);
    Get_MagLowPassFilterOutput(MagCalibratedData, MagFilterCalibratedData, ANGLE_FILTER_FACTOR);

    if ( m_Get_IsHeadingAngleEnable == IST_TRUE) {
        Process_TiltCompensation(MagFilterCalibratedData, AccData, angular, c_mdata);
        m_yaw = angular[2] * RADII_TO_DEGREE;

        if (m_yaw < 0)
            m_yaw += 360;

        if ( m_Get_AccuracyEnable == IST_TRUE) {
            m_accuracy = Process_CalculateAccuracy( bias[3], MagFilterCalibratedData, m_yaw);
        }
    }
}

double Process_CalculateAngularVelocity(double v1[3], double v2[3])
{
    double a_dot_b;
    double aa, bb;
    double cosine;
    a_dot_b = (v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2]);
    aa = sqrt(pow(v1[0], 2) + pow(v1[1], 2) + pow(v1[2], 2)) * sqrt(pow(v2[0], 2) + pow(v2[1], 2) + pow(v2[2], 2));
    bb = a_dot_b / aa;
    cosine = acos(bb) * RADII_TO_DEGREE;
    return cosine;
}

void Get_CalibratedMagData(float out[3], float m[9], float bias[3], float data[3])
{
    int i, j;
    float temp = 0;
    for (i = 0; i < 3; i++) {
        out[i] = 0;
        for (j = 0; j < 3; j++) {
            temp = (m[i * 3 + j] * (data[j] - bias[j]));
            out[i] = (out[i] + temp);
        }
    }
}

// Device direction - andoird phone defination - ENU (y,x,z)
// MAG:x:east , y:north, z:up ACC:x:west,y:south, z:down in API 1.0.0
// MAG and Acc :x:east , y:north, z:up in API 2.0.0
// here, algo use aerospace coordinate NED defination - NED (x,y,z)
// therefore, change the device andoird direction to aerospace/NED
void Process_TiltCompensation(float *mData, float *aData, float *Angular, float *c_mdata)
{

    float ax = 0, ay = 0, az = 0;
    float mx = 0, my = 0, mz = 0;
    float dx = 0, dy = 0, dz = 0;
    float sx = 0, sy = 0;
    float cx = 0, cy = 0;

    //adjust axis
    ax = aData[1];
    ay = aData[0];
    az = -aData[2];
    mx = mData[1];
    my = mData[0];
    mz = -mData[2];

    dx = atan2(ay, az);
    sx = sin(dx);
    cx = cos(dx);
    dy = atan(-((ax / ((ay * sx) + (az * cx)))));
    sy = sin(dy);
    cy = cos(dy);
    dz = atan2(((mz * sx) - (my * cx)), ((mx * cy) + (my * sy * sx) + (mz * sy * cx)));

    c_mdata[0] = ((mz * sx * cy) - (my * cx) - (mx * cy * cx));
    c_mdata[1] = ((mx * cy) + (my * sy * sx) + (mz * sy * cx));

    Angular[0] = dx;
    Angular[1] = dy;
    Angular[2] = dz;
}

ISTBOOL Get_DynamicCalibrationStartCondition(float* input, ISTBOOL status)
{
    double thetas;
    double vec[3] = {0};

    vec[0] = (double)input[0];
    vec[1] = (double)input[1];
    vec[2] = (double)input[2];

    thetas = Process_CalculateAngularVelocity(vec, pre_vec);

    pre_vec[0] = vec[0];
    pre_vec[1] = vec[1];
    pre_vec[2] = vec[2];

    // launch_calibration_num determinte if enabling starting condiftion of dynamic calibration
    // > 20, start to calibrate
    if (fabs(thetas) > CALIBRATION_THETA_THRESHOLD && launch_calibration_num < ENABLE_CALIBRATION_NUM)
        launch_calibration_num++;
    else if (fabs(thetas) <= CALIBRATION_THETA_THRESHOLD && launch_calibration_num > 0)
        launch_calibration_num--;

    if (status == IST_FALSE) {
        if (launch_calibration_num >= ENABLE_CALIBRATION_NUM)
            return IST_TRUE;
    }
    else {
        if (launch_calibration_num <= 0)
            return IST_FALSE;
    }

    return status;
}

void Get_MagLowPassFilterOutput(float input[3], float average[3], float factor)
{
    int i;
    for (i = 0; i < 3; ++i)
    {
        average[i] = factor * input[i] + (1 - factor) * average[i];
    }
}

#ifdef ENABLE_ACCURACY_REPORT

int Process_CalculateAccuracy(float ist_mag_rad, float* ist_mag_data, float yaw)
{
    float mag_distance = 0.0f;
    int bFindMinMax = 0;
    //int bCheckMinAccuracy = 0, bCheckMaxAccuracy = 0;
    int location = 0;
    int i = 0;
    float min = 0, max = 0;
    if (ist_mag_rad <= 0) {
        accuracy = 0; // unreliable
        goto RETURN;
    }

    if (ist_mag_rad > 0) {
        if (ist_mag_rad < IST_HIRON_RAD_MIN || ist_mag_rad > IST_HIRON_RAD_MAX) {
            // earth magnetic intensity is about 25 ~ 65 uT
            // if hard-iron sphere radius is smaller than 20 or larger than 200, it seems this caculation is wrong
            // so we report unreliable
            accuracy = 0; // unreliable
            goto RETURN;
        }
    }

    // earth magnetic intensity is about 25 ~ 65 uT
    // if radius is smaller than 20 or larger than 200, it seems something wrong
    // so we report unreliable

    // define in istlib.h, ENABLE_CALIBRATION_TYPE = 0 for plane, 1 for sphere
#if ENABLE_CALIBRATION_TYPE
    mag_distance = sqrt(pow(ist_mag_data[0], 2) + pow(ist_mag_data[1], 2) + pow(ist_mag_data[2], 2));
#else
    mag_distance = sqrt(pow(ist_mag_data[0], 2) + pow(ist_mag_data[1], 2) );
#endif

    if (mag_distance < IST_HIRON_RAD_MIN || mag_distance > IST_HIRON_RAD_MAX) {
        accuracy = 0; // unreliable
        goto RETURN;
    }

    location = (int)yaw / ACCURACY_SIZE_SPACE;
    if (s_yaw.yaw == location && s_yaw.count < STABLE_COUNT) {
        s_yaw.count++;
        goto RETURN;
    }
    else if (s_yaw.yaw != location) {
        s_yaw.yaw = location;
        s_yaw.count = 0;
        goto RETURN;
    }

    // Store mag_distance into array 36

    if (qlist36[location] == 0)
        qlist36[location] = mag_distance;
    else
        qlist36[location] = (qlist36[location] + mag_distance) / 2;
    if (max_item.Max == -1 || max_item.Min == -1 ||
            location == max_item.Max || location == max_item.Min)
    {
        bFindMinMax = 1;
    }

    if (max_item.Max == -1)
        max = 0;
    else
        max = qlist36[max_item.Max];

    if (max_item.Min == -1)
        min = 0;
    else
        min = qlist36[max_item.Min];

    if (bFindMinMax) {
        for (i = 0; i < MAX_ACCURACY_SIZE; i++) {
            if (qlist36[i] == 0)
                continue;

            if ( max == 0 || qlist36[i] > max) {
                max = qlist36[i];
                max_item.Max = i;
                //bCheckMaxAccuracy = 1;
            }

            if ( min == 0 || qlist36[i] < min) {
                min = qlist36[i];
                max_item.Min = i;
                //bCheckMinAccuracy = 1;
            }
        }
    }
    else {
        max = qlist36[max_item.Max];
        min = qlist36[max_item.Min];

        if ( qlist36[location] > max ) {
            max = qlist36[location];
            max_item.Max = location;
            //bCheckMaxAccuracy = 1;
        }

        if ( qlist36[location] < min ) {
            min = qlist36[location];
            max_item.Min = location;
            //bCheckMinAccuracy = 1;
        }
    }

    if (max_item.Max != -1 && max_item.Min != -1){
        max = qlist36[max_item.Max];
        min = qlist36[max_item.Min];

        x = (max - min) / min;
        if (x > 2){
            accuracy = 0; // unreliable
            goto RETURN;
        }
        fx = -3.8571f * x * x + 22.2966f * x + 0.7381f;
    }

    // fx(low) > ACCURACY_THRESHOLD_LOW > fx(medium)> ACCURACY_THRESHOLD_MEDIUM > fx(high) > ACCURACY_THRESHOLD_HIGH > fx(high)
    //                   25                                    15                                      10
    if (fx > ACCURACY_THRESHOLD_LOW){
        accuracy = 1; // low
        goto RETURN;
    }
    else if (fx <= ACCURACY_THRESHOLD_LOW && fx > ACCURACY_THRESHOLD_MEDIUM){
        accuracy = 2; // medium
        goto RETURN;
    }
    else if (fx <= ACCURACY_THRESHOLD_MEDIUM && fx > ACCURACY_THRESHOLD_HIGH){
        accuracy = 3; // high
        goto RETURN;
    }
    else{
        accuracy = 3; // high
    }

RETURN:
    return accuracy;
}
#endif //Define ENABLE_ACCURACY_REPORT
