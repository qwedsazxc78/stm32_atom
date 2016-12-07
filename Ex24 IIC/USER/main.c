#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "lcd.h"
#include "24cxx.h"
#include "key.h"
#include "ist8310_driver.h"

//ALIENTEK 探索者STM32F407开发板 实验24
//IIC 实验 --库函数版本
//技术支持：www.openedv.com
//淘宝店铺：http://eboard.taobao.com
//广州市星翼电子科技有限公司
//作者：正点原子 @ALIENTEK



//要写入到24c02的字符串数组
const u8 TEXT_Buffer[] = {"Explorer STM32F4 IIC TEST"};
#define SIZE sizeof(TEXT_Buffer)

int main(void)
{
	u8 key;
	u16 i = 0;
	u16 j = 0;
	u8 datatemp[SIZE];
	u8 datacheck[1];
	short temp;
	// compass
	int16_t MagXYZ[3];
	float magx, magy, magz;
	float IST8310_bias[4] =  {5, 0, 0, 0};
	float getIST8310_bias[4];
	float yaw;

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init(168);    //初始化延时函数
	uart_init(115200);	//初始化串口波特率为115200

	LED_Init();					//初始化LED
	LCD_Init();					//LCD初始化
	KEY_Init(); 				//按键初始化
	AT24CXX_Init();			//IIC初始化
	ist8310_Init();

	POINT_COLOR = RED;
	LCD_ShowString(30, 50, 200, 16, 16, "Explorer STM32F4");
	LCD_ShowString(30, 70, 200, 16, 16, "IIC TEST");
	LCD_ShowString(30, 90, 200, 16, 16, "ATOM@ALIENTEK");
	LCD_ShowString(30, 110, 200, 16, 16, "2014/5/6");
	LCD_ShowString(30, 130, 200, 16, 16, "KEY1:Write  KEY0:Read");	//显示提示信息
	LCD_ShowString(30, 260, 200, 16, 16, " Yaw :    . C");
	LCD_ShowString(30, 280, 200, 16, 16, " Mx  :    . mG");
	LCD_ShowString(30, 300, 200, 16, 16, " My  :    . mG");
	LCD_ShowString(30, 320, 200, 16, 16, " Mz  :    . mG");
	LCD_ShowString(30, 340, 200, 16, 16, "Debug:    .   ");


	while (AT24CXX_Check()) //检测不到24c02
	{
		LCD_ShowString(30, 150, 200, 16, 16, "24C02 Check Failed!");
		delay_ms(500);
		LCD_ShowString(30, 150, 200, 16, 16, "Please Check!      ");
		delay_ms(500);
		LED0 = !LED0; //DS0闪烁
	}
	LCD_ShowString(30, 150, 200, 16, 16, "24C02 Ready!");
	POINT_COLOR = BLUE; //设置字体为蓝色
	while (1)
	{
		key = KEY_Scan(0);
		if (key == KEY1_PRES) //KEY1按下,写入24C02
		{
			LCD_Fill(0, 170, 239, 319, WHITE); //清除半屏
			LCD_ShowString(30, 170, 200, 16, 16, "Start Write 24C02....");
			AT24CXX_Write(0, (u8*)TEXT_Buffer, SIZE);
			LCD_ShowString(30, 170, 200, 16, 16, "24C02 Write Finished!"); //提示传送完成
		}
		if (key == KEY0_PRES) //KEY0按下,读取字符串并显示
		{
			LCD_ShowString(30, 170, 200, 16, 16, "Start Read 24C02.... ");
			AT24CXX_Read(0, datatemp, SIZE);
			LCD_ShowString(30, 170, 200, 16, 16, "The Data Readed Is:  "); //提示传送完成
			LCD_ShowString(30, 190, 200, 16, 16, datatemp); //显示读到的字符串
		}

		// scan i2c bus
//		if (j<255){
//			//ist_i2c_read_byte(j, IST8310_REG_WIA, datacheck);
//		    ist_i2c_read(j,IST8310_REG_WIA,1,datacheck);
//	//		LCD_ShowString(30,210,200,16,16,datacheck);//显示读到的字符串
//			printf("ID: %d , Value: %d\r\n",j ,datacheck[0]);
//
//		}else{
//			j=0;
//		}

		ist8310_GetXYZ(MagXYZ);
		magx = (float)MagXYZ[0];
		magy = (float)MagXYZ[1];
		magz = (float)MagXYZ[2];
		// for test lib
		setBias(IST8310_bias);
		getBias(getIST8310_bias);
		// printf("Mag: %f ,%f , %f \r\n",magx,magy,magz);


		i++;
		j++;
		delay_ms(10);

		if (i == 10)
		{
			temp = yaw * 10;
			if (temp < 0)
			{
				LCD_ShowChar(30 + 48, 260, '-', 16, 0);		//显示负号
				temp = -temp;		//转为正数
			} else LCD_ShowChar(30 + 48, 260, ' ', 16, 0);		//去掉负号
			LCD_ShowNum(30 + 48 + 8, 260, temp / 10, 3, 16);		//显示整数部分
			LCD_ShowNum(30 + 48 + 40, 260, temp % 10, 1, 16);		//显示小数部分


			temp = magx * 10;
			if (temp < 0)
			{
				LCD_ShowChar(30 + 48, 280, '-', 16, 0);		//显示负号
				temp = -temp;		//转为正数
			} else LCD_ShowChar(30 + 48, 280, ' ', 16, 0);		//去掉负号
			LCD_ShowNum(30 + 48 + 8, 280, temp / 10, 3, 16);		//显示整数部分
			LCD_ShowNum(30 + 48 + 40, 280, temp % 10, 1, 16);		//显示小数部分

			temp = magy * 10;
			if (temp < 0)
			{
				LCD_ShowChar(30 + 48, 300, '-', 16, 0);		//显示负号
				temp = -temp;		//转为正数
			} else LCD_ShowChar(30 + 48, 300, ' ', 16, 0);		//去掉负号
			LCD_ShowNum(30 + 48 + 8, 300, temp / 10, 3, 16);		//显示整数部分
			LCD_ShowNum(30 + 48 + 40, 300, temp % 10, 1, 16);		//显示小数部分

			temp = magz * 10;
			if (temp < 0)
			{
				LCD_ShowChar(30 + 48, 320, '-', 16, 0);		//显示负号
				temp = -temp;		//转为正数
			} else LCD_ShowChar(30 + 48, 320, ' ', 16, 0);		//去掉负号
			LCD_ShowNum(30 + 48 + 8, 320, temp / 10, 3, 16);		//显示整数部分
			LCD_ShowNum(30 + 48 + 40, 320, temp % 10, 1, 16);		//显示小数部分


			temp = getIST8310_bias[0] * 10;
			if (temp < 0)
			{
				LCD_ShowChar(30 + 48, 340, '-', 16, 0);		//显示负号
				temp = -temp;		//转为正数
			} else LCD_ShowChar(30 + 48, 340, ' ', 16, 0);		//去掉负号
			LCD_ShowNum(30 + 48 + 8, 340, temp / 10, 3, 16);		//显示整数部分
			LCD_ShowNum(30 + 48 + 40, 340, temp % 10, 1, 16);		//显示小数部分

			printf("Mag: %f ,%f , %f \r\n", magx, magy, magz);

			LED0 = !LED0; //提示系统正在运行
			i = 0;
		}
	}
}
