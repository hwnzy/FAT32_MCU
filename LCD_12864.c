#include "LCD_12864.h"

BYTE lcd_x,lcd_y;
static BOOL num = 0;
sbit LCD_DATA_COMMAND_SELECTION = P0^7; //Һ������/����ѡ���(1/0)
sbit LCD_READ_WRITE_SELECTION=P0^6;     //��/дѡ���(1/0)
sbit LCD_DATA_ENABLE=P0^5;              //����ʹ�ܶ�
sbit LCD_PARALLEL_SERIE_SELECTION=P0^4; //��/��ѡ��H���� L����
sbit LCD_RESET=P0^3;                    //��λ ????? P0^2
	 
/******************************************
���ƣ�CheckBusy
���ܣ��ж�LCD���ݴ��������Ƿ���æ״̬
******************************************/
void CheckBusy()
{
	LCD_DATA_COMMAND_SELECTION = 0;
	LCD_READ_WRITE_SELECTION = 1;
	LCD_DATA_ENABLE = 1;
	LCD_PORT = 0xff;
	while(LCD_PORT & 0x80);
	LCD_DATA_ENABLE = 0;	
}

/******************************************
���ƣ�WriteCommand
���ܣ���LCDд������
******************************************/
void WriteCommand(BYTE dat)
{
	CheckBusy();
	LCD_DATA_COMMAND_SELECTION = 0;
	LCD_READ_WRITE_SELECTION = 0;
	LCD_PORT = dat;
	LCD_DATA_ENABLE = 1;
	Delay10us(1);
	//Delayms(1);
	LCD_DATA_ENABLE = 0;
}

/******************************************
���ƣ�WriteData
���ܣ���LCDд������
******************************************/
void WriteData(BYTE dat) 
{
	CheckBusy();
	LCD_DATA_COMMAND_SELECTION = 1;
	LCD_READ_WRITE_SELECTION = 0;
	LCD_PORT = dat;
	LCD_DATA_ENABLE = 1;
	Delay10us(1);
	//Delayms(1);
	LCD_DATA_ENABLE = 0;
}


/******************************************
���ƣ�Lcd12864Init
���ܣ���LCDִ�г�ʼ������
******************************************/
void Lcd12864Init()
{
	LCD_PARALLEL_SERIE_SELECTION = 1;
	WriteCommand(0x30); //�ص�����ָ�
	Delayms(1);
	WriteCommand(0x01); //����
	Delayms(1);
	WriteCommand(0x06);//�������������ʾ���ƶ�
	Delayms(1);
	WriteCommand(0x0c); //�˳�˯��ģʽ
}

/******************************************
���ƣ�Lcd12864Play
���ܣ�����һ�����
******************************************/
void Lcd12864Play(LPBYTE p)
{
	BYTE j,k;
	lcd_x = 0x80;
	lcd_y = 0x80;
	if(!num) {
		for(j = 0;j < 32;j++)
		{
			WriteCommand(lcd_y + j);//ˮƽ��ַ�����Զ���1������ֱ��ַ����
			WriteCommand(lcd_x);//�ϰ���Ҳ��������32����ֱ�����Ӧ��ˮƽ����Ϊ0x80-0x87
			for(k = 0;k < 16;k++)//ˮƽ��ַ�ʹ�ֱ��ַȷ��2���ֽڵ����ݵ���������
			{
				WriteData(*p++);//һ�δ�һ���ֽڣ�Ҳ����8����
			}			
		}
		num = 1;		
	}
	else {
		lcd_x = 0x88;//�°���Ҳ��������32����ֱ�����Ӧ��ˮƽ����Ϊ0x88-0x8f
		for(j = 0;j < 32;j++)
		{
			WriteCommand(lcd_y + j);//ˮƽ��ַ�����Զ���1������ֱ��ַ����
			WriteCommand(lcd_x);//�ϰ���Ҳ��������32����ֱ�����Ӧ��ˮƽ����Ϊ0x80-0x87
			for(k = 0;k < 16;k++)//ˮƽ��ַ�ʹ�ֱ��ַȷ��2���ֽڵ����ݵ���������
			{
				WriteData(*p++);//һ�δ�һ���ֽڣ�Ҳ����8����
			}			
		}
		num = 0;
	}
}

//ͼƬ��ʾ����
/******************************************
���ƣ�Lcd12864DisplayImage
���ܣ�������(x,y)����ʾ�ַ�����yȡֵֻ��Ϊ1\2\3\4
******************************************/
void Lcd12864DisplayImage(LPBYTE p1,LPBYTE p2)
{
	BYTE j,k;
	lcd_x = 0x80;
	lcd_y = 0x80;
	Lcd12864Init();
	WriteCommand(0x34);//����չ����ģʽ����ͼ��ʾ�ر�
		for(j = 0;j < 32;j++)
		{
			WriteCommand(lcd_y + j);//ˮƽ��ַ�����Զ���1������ֱ��ַ����
			WriteCommand(lcd_x);//�ϰ���Ҳ��������32����ֱ�����Ӧ��ˮƽ����Ϊ0x80-0x87
			for(k = 0;k < 16;k++)//ˮƽ��ַ�ʹ�ֱ��ַȷ��2���ֽڵ����ݵ���������
			{
				WriteData(*p1++);//һ�δ�һ���ֽڣ�Ҳ����8����
			}			
		}
		lcd_x = 0x88;//�°���Ҳ��������32����ֱ�����Ӧ��ˮƽ����Ϊ0x88-0x8f
		for(j = 0;j < 32;j++)
		{
			WriteCommand(lcd_y + j);//ˮƽ��ַ�����Զ���1������ֱ��ַ����
			WriteCommand(lcd_x);//�ϰ���Ҳ��������32����ֱ�����Ӧ��ˮƽ����Ϊ0x80-0x87
			for(k = 0;k < 16;k++)//ˮƽ��ַ�ʹ�ֱ��ַȷ��2���ֽڵ����ݵ���������
			{
				WriteData(*p2++);//һ�δ�һ���ֽڣ�Ҳ����8����
			}			
		}
	WriteCommand(0x36);//����չ����ģʽ���򿪻�ͼ��ʾ
}
//��ͼ��ʾRAM�ṩ128X8���ֽڵļ���ռ䣬�ڸ��Ļ�ͼRAMʱ������д��ˮƽ�봹ֱ������ֵ��
//��д�������ֽڵ����ݵ���ͼRAM������ַ���������Զ���1����д���ͼRAM���ڼ䣬��ͼ��ʾ����ر�
//��������
//1.�رջ�ͼ��ʾ���ܡ�
//2.�Ƚ�ˮƽ��λԪ�����꣨X��д���ͼRAM��ַ���ٽ���ֱ�����꣨Y��д���ͼRAM��ַ
//3.�򿪻�ͼ��ʾ����

/******************************************
���ƣ�Lcd12864Printf
���ܣ�������(x,y)����ʾ�ַ�����yȡֵֻ��Ϊ1\2\3\4
******************************************/
void Lcd12864Printf(BYTE x,BYTE y,LPBYTE p)
{
	BYTE string_address;
	WriteCommand(0x30);
	switch(y){
		case 1 : string_address = 0x80+x;break;
		case 2 : string_address = 0x90+x;break;
		case 3 : string_address = 0x88+x;break;
		case 4 : string_address = 0x98+x;break;
		default : break;
	}
	WriteCommand(string_address);
	while(*p!='\0')
	{
		WriteData(*p++);
	}
}

/******************************************
���ƣ�Lcd12864PrintfOne
���ܣ��ַ�����һ����ʾ,ͣ��1������
******************************************/
void Lcd12864PrintfOne(LPBYTE p)
{
	Lcd12864Printf(0,1,p);
	Delay1s(1);
	Lcd12864ClearDDRAM();
}


/******************************************
���ƣ�Lcd12864ClearDDRAM
���ܣ����DDRAM
******************************************/
void Lcd12864ClearDDRAM()
{
	WriteCommand(0x01);	
}


/******************************************
���ƣ�Lcd12864ClearGDRAM
���ܣ����GDRAM
******************************************/
void Lcd12864ClearGDRAM()
{
	BYTE i,j,k;
	WriteCommand(0x34);
	i = 0x80;
	for(j = 0;j < 32;j++){
		WriteCommand(i++);
		WriteCommand(0x80);
		for(k = 0;k < 16;k++){
			WriteData(0x00);	
		}	
	}
	i = 0x80;
	for(j = 0;j < 32;j++){
		WriteCommand(i++);
		WriteCommand(0x88);
		for(k = 0;k < 16;k++){
			WriteData(0x00);	
		}	
	}
	WriteCommand(0x36);//����չ����ģʽ���򿪻�ͼ��ʾ
}

