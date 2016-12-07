#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "lcd.h"
#include "24cxx.h"
#include "key.h"
#include "ist8310_driver.h"

//ALIENTEK ̽����STM32F407������ ʵ��24
//IIC ʵ�� --�⺯���汾
//����֧�֣�www.openedv.com
//�Ա����̣�http://eboard.taobao.com
//������������ӿƼ����޹�˾
//���ߣ�����ԭ�� @ALIENTEK



//Ҫд�뵽24c02���ַ�������
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

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//����ϵͳ�ж����ȼ�����2
	delay_init(168);    //��ʼ����ʱ����
	uart_init(115200);	//��ʼ�����ڲ�����Ϊ115200

	LED_Init();					//��ʼ��LED
	LCD_Init();					//LCD��ʼ��
	KEY_Init(); 				//������ʼ��
	AT24CXX_Init();			//IIC��ʼ��
	ist8310_Init();

	POINT_COLOR = RED;
	LCD_ShowString(30, 50, 200, 16, 16, "Explorer STM32F4");
	LCD_ShowString(30, 70, 200, 16, 16, "IIC TEST");
	LCD_ShowString(30, 90, 200, 16, 16, "ATOM@ALIENTEK");
	LCD_ShowString(30, 110, 200, 16, 16, "2014/5/6");
	LCD_ShowString(30, 130, 200, 16, 16, "KEY1:Write  KEY0:Read");	//��ʾ��ʾ��Ϣ
	LCD_ShowString(30, 260, 200, 16, 16, " Yaw :    . C");
	LCD_ShowString(30, 280, 200, 16, 16, " Mx  :    . mG");
	LCD_ShowString(30, 300, 200, 16, 16, " My  :    . mG");
	LCD_ShowString(30, 320, 200, 16, 16, " Mz  :    . mG");
	LCD_ShowString(30, 340, 200, 16, 16, "Debug:    .   ");


	while (AT24CXX_Check()) //��ⲻ��24c02
	{
		LCD_ShowString(30, 150, 200, 16, 16, "24C02 Check Failed!");
		delay_ms(500);
		LCD_ShowString(30, 150, 200, 16, 16, "Please Check!      ");
		delay_ms(500);
		LED0 = !LED0; //DS0��˸
	}
	LCD_ShowString(30, 150, 200, 16, 16, "24C02 Ready!");
	POINT_COLOR = BLUE; //��������Ϊ��ɫ
	while (1)
	{
		key = KEY_Scan(0);
		if (key == KEY1_PRES) //KEY1����,д��24C02
		{
			LCD_Fill(0, 170, 239, 319, WHITE); //�������
			LCD_ShowString(30, 170, 200, 16, 16, "Start Write 24C02....");
			AT24CXX_Write(0, (u8*)TEXT_Buffer, SIZE);
			LCD_ShowString(30, 170, 200, 16, 16, "24C02 Write Finished!"); //��ʾ�������
		}
		if (key == KEY0_PRES) //KEY0����,��ȡ�ַ�������ʾ
		{
			LCD_ShowString(30, 170, 200, 16, 16, "Start Read 24C02.... ");
			AT24CXX_Read(0, datatemp, SIZE);
			LCD_ShowString(30, 170, 200, 16, 16, "The Data Readed Is:  "); //��ʾ�������
			LCD_ShowString(30, 190, 200, 16, 16, datatemp); //��ʾ�������ַ���
		}

		// scan i2c bus
//		if (j<255){
//			//ist_i2c_read_byte(j, IST8310_REG_WIA, datacheck);
//		    ist_i2c_read(j,IST8310_REG_WIA,1,datacheck);
//	//		LCD_ShowString(30,210,200,16,16,datacheck);//��ʾ�������ַ���
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
				LCD_ShowChar(30 + 48, 260, '-', 16, 0);		//��ʾ����
				temp = -temp;		//תΪ����
			} else LCD_ShowChar(30 + 48, 260, ' ', 16, 0);		//ȥ������
			LCD_ShowNum(30 + 48 + 8, 260, temp / 10, 3, 16);		//��ʾ��������
			LCD_ShowNum(30 + 48 + 40, 260, temp % 10, 1, 16);		//��ʾС������


			temp = magx * 10;
			if (temp < 0)
			{
				LCD_ShowChar(30 + 48, 280, '-', 16, 0);		//��ʾ����
				temp = -temp;		//תΪ����
			} else LCD_ShowChar(30 + 48, 280, ' ', 16, 0);		//ȥ������
			LCD_ShowNum(30 + 48 + 8, 280, temp / 10, 3, 16);		//��ʾ��������
			LCD_ShowNum(30 + 48 + 40, 280, temp % 10, 1, 16);		//��ʾС������

			temp = magy * 10;
			if (temp < 0)
			{
				LCD_ShowChar(30 + 48, 300, '-', 16, 0);		//��ʾ����
				temp = -temp;		//תΪ����
			} else LCD_ShowChar(30 + 48, 300, ' ', 16, 0);		//ȥ������
			LCD_ShowNum(30 + 48 + 8, 300, temp / 10, 3, 16);		//��ʾ��������
			LCD_ShowNum(30 + 48 + 40, 300, temp % 10, 1, 16);		//��ʾС������

			temp = magz * 10;
			if (temp < 0)
			{
				LCD_ShowChar(30 + 48, 320, '-', 16, 0);		//��ʾ����
				temp = -temp;		//תΪ����
			} else LCD_ShowChar(30 + 48, 320, ' ', 16, 0);		//ȥ������
			LCD_ShowNum(30 + 48 + 8, 320, temp / 10, 3, 16);		//��ʾ��������
			LCD_ShowNum(30 + 48 + 40, 320, temp % 10, 1, 16);		//��ʾС������


			temp = getIST8310_bias[0] * 10;
			if (temp < 0)
			{
				LCD_ShowChar(30 + 48, 340, '-', 16, 0);		//��ʾ����
				temp = -temp;		//תΪ����
			} else LCD_ShowChar(30 + 48, 340, ' ', 16, 0);		//ȥ������
			LCD_ShowNum(30 + 48 + 8, 340, temp / 10, 3, 16);		//��ʾ��������
			LCD_ShowNum(30 + 48 + 40, 340, temp % 10, 1, 16);		//��ʾС������

			printf("Mag: %f ,%f , %f \r\n", magx, magy, magz);

			LED0 = !LED0; //��ʾϵͳ��������
			i = 0;
		}
	}
}
