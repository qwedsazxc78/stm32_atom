/**
 * @file main.c
 * @author ISENTEK
 * @date 2016-12-01 v2.0.3
 * @brief How to use Isentek eCompass sensor driver and API
 */

// include essential header file
#include "ist8310_driver.h" // Magnetometer driver
#include "IST_LIB.h"		// ISentek eCompass library API
#include "config.h"			// ISentek eCompass library API
// Accelerometer driver, you can use your acc driver
// #include "kionix_driver.h"

int main()
{
	// A : isentek compass variable definition :
	// mag variable
	int16_t MagXYZ[3];
	float Mag_LSBtoUT = 0.3;
	float Mag_RawData[3];
	float Mag_CalibratedData[3];
	float Acc_RawData[3];
	float AccXYZ[3] = {0, 0, 9.8};

	// mag calibration parameter
	float Mag_IST8310_Bias[4] =  {3.578220 , 13.635200 , -12.570942 , 41.839500};
	float Mag_GetIST8310_Bias[4];
	int Mag_CalibrationStatus = 0;
	// mag heading angle
	float Mag_HeadingAngle = 0, Mag_HeadingAngle_Int100 = 0;
	// mag accurancy - return 0:unreliable  3:High  2:Medium 1: Low
	int Mag_Accurancy = 0;

	// B : isentek compass initialization stage :
	ist8310_Init(); 						// init driver
	Set_HeadingAngleEnable(IST_TRUE);		// if return Heading angle, it need acc and mag sensor
	Set_MagAccuracyEnable(IST_TRUE);		// if enable eCompass accuracy report - return 0:unreliable  3:High  2:Medium 1: Low
	Set_SingleCalibration(IST_TRUE);		// if enable single calibration right now
	// Set_DynamicCalibration(IST_TRUE) ;		// if enable continuous calibration

	// if you have last calibrated value and don't calibrate again, you can set by this "Set_MagBias" funciton
	// note : this setting dependent on your application, please check if you applciation is suitable
	// Set_MagBias(Mag_IST8310_Bias);

	while (1)
	{
		
		// C : running process stage :
		// C1. Get data from driver and change the snesor value from LSB to uT
		// C2. Dependent on PCB layout, change the mag and acc diraction to ENU (x = east, y = west, z = up)
		ist8310_GetXYZ(MagXYZ);
		Mag_RawData[0] =  -((float)MagXYZ[1]) * Mag_LSBtoUT;
		Mag_RawData[1] =  -((float)MagXYZ[0]) * Mag_LSBtoUT;
		Mag_RawData[2] =  ((float)MagXYZ[2]) * Mag_LSBtoUT;
		// we use KXTF9 accelerometer as example, you can change this function by your acc driver
		// KXTF9_GetXYZ(AccXYZ);
		Acc_RawData[0] = AccXYZ[1];
		Acc_RawData[1] = AccXYZ[0];
		Acc_RawData[2] = -AccXYZ[2];

		// C3. Run Compass Process to get Calibrated Data
		// Here, we use constant acc data input [0,0,9.8], also, acc data [0,0,1] is ok.
		Process_RunCompass(Mag_RawData, Acc_RawData, Mag_CalibratedData);

		// C4. Check Calibration Status
		// Mag_CalibrationStatus : In calibration = 1, no = 0
		// Get_MagBias : BiasX, BiasY, BiasZ, Radius
		// Mag_GetIST8310_Bias element will have valid value after calibration,
		// you can check the bias value to know process finish calibration.
		Mag_CalibrationStatus = Get_CalibrationStatus();
		Get_MagBias(Mag_GetIST8310_Bias);

		// C5. Get HeadingAngle
		// Get_HeadingAngle return int type angle. 0 - 360 degree
		// Get_HeadingAngle100 return int type angle . 0 - 36000 degree
		// Therefore, we need to change angle type
		Mag_HeadingAngle = (float)Get_HeadingAngle();
		Mag_HeadingAngle_Int100 = 0.01f * ((float)Get_HeadingAngle100());

		// C6. Get Mag Accurancy Status
		// 0:unreliable  3:High  2:medum  1: low
		Mag_Accurancy = Get_MagAccuracyStatus();

		// C7. print for debugging
		printf("Mag: %f ,%f , %f \r\n", Mag_CalibratedData[0], Mag_CalibratedData[1], Mag_CalibratedData[2]);
		printf("Bias: %f ,%f , %f , %f\r\n", Mag_GetIST8310_Bias[0], Mag_GetIST8310_Bias[1], Mag_GetIST8310_Bias[2], Mag_GetIST8310_Bias[3]);
		printf("Cal: %d \r\n", Mag_CalibrationStatus);
		printf("HeadingAngle: %f \r\n", Mag_HeadingAngle_Int100);
		printf("Accurancy: %d \r\n", Mag_Accurancy);
	}
