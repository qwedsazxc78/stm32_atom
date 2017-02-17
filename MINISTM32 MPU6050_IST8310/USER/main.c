#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "lcd.h"
#include "key.h"  
#include "myiic.h"

#include "IST8310_driver.h"
#include "IST_LIB.h"
   	
void showLCD (int height, short temp );

 int main(void)
 { 
	u8 key;
	u16 ii = 0;
	u16 j = 0;
//	u8 datatemp[SIZE];
	//u8 datacheck[1];
	short temp;
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
	 
	 
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// 设置中断优先级分组2
	delay_init();	    	 //延时函数初始化	  
	uart_init(115200);	 	//串口初始化为9600
	LED_Init();		  		//初始化与LED连接的硬件接口
 	LCD_Init();	
	KEY_Init();				//按键初始化		

 // B : isentek compass initialization stage :
	ist8310_Init(); 						// init driver
	Set_HeadingAngleEnable(IST_TRUE);		// if return Heading angle, it need acc and mag sensor
	Set_MagAccuracyEnable(IST_TRUE);		// if enable eCompass accuracy report - return 0:unreliable  3:High  2:medum  1: low
	Set_SingleCalibration(IST_TRUE);		// if enable single calibration right now
	// Set_DynamicCalibration(IST_TRUE) ;		// if enable continous calibration

	// if you have last calibrated value and don't calibrate again, you can set by this "Set_MagBias" funciton
	// note : this setting dependent on your application, please check if you applciation is suitable
	// Set_MagBias(Mag_IST8310_Bias);


	POINT_COLOR = RED;
	LCD_ShowString(30, 30, 200, 16, 16, "Explorer STM32F4");
	LCD_ShowString(30, 50, 200, 16, 16, "Algorithm Dev.");
	LCD_ShowString(30, 80, 200, 16, 16, " Yaw :    . degree");
	LCD_ShowString(30, 100, 200, 16, 16, " Mx  :    . uT");
	LCD_ShowString(30, 120, 200, 16, 16, " My  :    . uT");
	LCD_ShowString(30, 140, 200, 16, 16, " Mz  :    . uT");
	LCD_ShowString(30, 160, 200, 16, 16, "BiasX:    . uT");
	LCD_ShowString(30, 180, 200, 16, 16, "BiasY:    . uT");
	LCD_ShowString(30, 200, 200, 16, 16, "BiasZ:    . uT");
	LCD_ShowString(30, 220, 200, 16, 16, "Cal  :    . ");
	LCD_ShowString(30, 240, 200, 16, 16, "Accur:    . ");

	POINT_COLOR = BLUE; //设置字体为蓝色

	while (1)
	{
		if (KEY_Scan(1) == 2) {
			Set_SingleCalibration(IST_TRUE);
		}

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

		// C3. Run Compass Process to get CalibratedData
		//	   Here, we use constant acc data [0,0,9.8], also, acc data [0,0,1] is ok.
		Process_RunCompass(Mag_RawData, Acc_RawData, Mag_CalibratedData);

		// C4. Check Calibration Status
		// Mag_CalibrationStatus : In calibrattion = 1, no = 0
		// Get_MagBias element will have valid value after calibration, you can check the bias value
		// to know process finish calibraiton.
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

		// print for debugging
		// printf("Mag: %f ,%f , %f \r\n", Mag_CalibratedData[0], Mag_CalibratedData[1], Mag_CalibratedData[2]);
		// printf("Bias: %f ,%f , %f , %f\r\n", Mag_GetIST8310_Bias[0], Mag_GetIST8310_Bias[1], Mag_GetIST8310_Bias[2], Mag_GetIST8310_Bias[3]);
		// printf("Cal: %d \r\n", Mag_CalibrationStatus);
		// printf("HeadingAngle: %f \r\n", Mag_HeadingAngle_Int100);
		// printf("Accurancy: %d \r\n", Mag_Accurancy);

		ii++;
		j++;

		delay_ms(10);

		if (ii == 20)
		{
			temp = Mag_HeadingAngle_Int100 * 10;
			showLCD(80, temp);

			temp = Mag_RawData[0] * 10;
			showLCD(100, temp);
			temp = Mag_RawData[1] * 10;
			showLCD(120, temp);
			temp = Mag_RawData[2] * 10;
			showLCD(140, temp);

			temp = Mag_GetIST8310_Bias[0] * 10;
			showLCD(160, temp);
			temp = Mag_GetIST8310_Bias[1] * 10;
			showLCD(180, temp);
			temp = Mag_GetIST8310_Bias[2] * 10;
			showLCD(200, temp);
			
			temp = Mag_CalibrationStatus * 10;
			showLCD(220, temp); 
			temp = Mag_Accurancy * 10;
			showLCD(240, temp);


			printf("Mag: %f ,%f , %f \r\n", Mag_CalibratedData[0], Mag_CalibratedData[1], Mag_CalibratedData[2]);
			printf("Bias: %f ,%f , %f , %f\r\n", Mag_GetIST8310_Bias[0], Mag_GetIST8310_Bias[1], Mag_GetIST8310_Bias[2], Mag_GetIST8310_Bias[3]);
			printf("Cal: %d \r\n", Mag_CalibrationStatus);
			printf("HeadingAngle: %f \r\n", Mag_HeadingAngle_Int100);
			printf("Accurancy: %d \r\n", Mag_Accurancy);


			LED0 = !LED0; //提示系统正在运行
			ii = 0;
		}
	}
}
 

void showLCD (int height, short temp ) {
	if (temp < 0)
	{
		LCD_ShowChar(30 + 48, height, '-', 16, 0);		//显示负号
		temp = -temp;		//转为正数
	} else LCD_ShowChar(30 + 48, height, ' ', 16, 0);		//去掉负号
	LCD_ShowNum(30 + 48 + 8, height, temp / 10, 3, 16);		//显示整数部分
	LCD_ShowNum(30 + 48 + 40, height, temp % 10, 1, 16);		//显示小数部分
}

