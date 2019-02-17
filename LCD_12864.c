#include "LCD_12864.h"

BYTE lcd_x,lcd_y;
static BOOL num = 0;
sbit LCD_DATA_COMMAND_SELECTION = P0^7; //液晶数据/命令选择端(1/0)
sbit LCD_READ_WRITE_SELECTION=P0^6;     //读/写选择端(1/0)
sbit LCD_DATA_ENABLE=P0^5;              //数据使能端
sbit LCD_PARALLEL_SERIE_SELECTION=P0^4; //并/串选择：H并行 L串行
sbit LCD_RESET=P0^3;                    //复位 ????? P0^2
	 
/******************************************
名称：CheckBusy
功能：判断LCD数据传输引脚是否处在忙状态
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
名称：WriteCommand
功能：向LCD写入命令
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
名称：WriteData
功能：向LCD写入数据
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
名称：Lcd12864Init
功能：对LCD执行初始化命令
******************************************/
void Lcd12864Init()
{
	LCD_PARALLEL_SERIE_SELECTION = 1;
	WriteCommand(0x30); //回到基本指令集
	Delayms(1);
	WriteCommand(0x01); //清屏
	Delayms(1);
	WriteCommand(0x06);//光标右移整体显示不移动
	Delayms(1);
	WriteCommand(0x0c); //退出睡眠模式
}

/******************************************
名称：Lcd12864Play
功能：播放一组点阵
******************************************/
void Lcd12864Play(LPBYTE p)
{
	BYTE j,k;
	lcd_x = 0x80;
	lcd_y = 0x80;
	if(!num) {
		for(j = 0;j < 32;j++)
		{
			WriteCommand(lcd_y + j);//水平地址可以自动加1，而垂直地址不会
			WriteCommand(lcd_x);//上半屏也就是上面32个垂直坐标对应的水平坐标为0x80-0x87
			for(k = 0;k < 16;k++)//水平地址和垂直地址确定2个字节的数据点亮几个点
			{
				WriteData(*p++);//一次传一个字节，也就是8个点
			}			
		}
		num = 1;		
	}
	else {
		lcd_x = 0x88;//下半屏也就是下面32个垂直坐标对应的水平坐标为0x88-0x8f
		for(j = 0;j < 32;j++)
		{
			WriteCommand(lcd_y + j);//水平地址可以自动加1，而垂直地址不会
			WriteCommand(lcd_x);//上半屏也就是上面32个垂直坐标对应的水平坐标为0x80-0x87
			for(k = 0;k < 16;k++)//水平地址和垂直地址确定2个字节的数据点亮几个点
			{
				WriteData(*p++);//一次传一个字节，也就是8个点
			}			
		}
		num = 0;
	}
}

//图片显示函数
/******************************************
名称：Lcd12864DisplayImage
功能：在坐标(x,y)处显示字符串，y取值只能为1\2\3\4
******************************************/
void Lcd12864DisplayImage(LPBYTE p1,LPBYTE p2)
{
	BYTE j,k;
	lcd_x = 0x80;
	lcd_y = 0x80;
	Lcd12864Init();
	WriteCommand(0x34);//打开扩展功能模式，绘图显示关闭
		for(j = 0;j < 32;j++)
		{
			WriteCommand(lcd_y + j);//水平地址可以自动加1，而垂直地址不会
			WriteCommand(lcd_x);//上半屏也就是上面32个垂直坐标对应的水平坐标为0x80-0x87
			for(k = 0;k < 16;k++)//水平地址和垂直地址确定2个字节的数据点亮几个点
			{
				WriteData(*p1++);//一次传一个字节，也就是8个点
			}			
		}
		lcd_x = 0x88;//下半屏也就是下面32个垂直坐标对应的水平坐标为0x88-0x8f
		for(j = 0;j < 32;j++)
		{
			WriteCommand(lcd_y + j);//水平地址可以自动加1，而垂直地址不会
			WriteCommand(lcd_x);//上半屏也就是上面32个垂直坐标对应的水平坐标为0x80-0x87
			for(k = 0;k < 16;k++)//水平地址和垂直地址确定2个字节的数据点亮几个点
			{
				WriteData(*p2++);//一次传一个字节，也就是8个点
			}			
		}
	WriteCommand(0x36);//打开扩展功能模式，打开绘图显示
}
//绘图显示RAM提供128X8个字节的记忆空间，在更改绘图RAM时先连续写入水平与垂直的坐标值，
//再写入两个字节的数据到绘图RAM，而地址计数器会自动加1，再写入绘图RAM的期间，绘图显示必须关闭
//步骤如下
//1.关闭绘图显示功能。
//2.先将水平的位元组坐标（X）写入绘图RAM地址，再将垂直的坐标（Y）写入绘图RAM地址
//3.打开绘图显示功能

/******************************************
名称：Lcd12864Printf
功能：在坐标(x,y)处显示字符串，y取值只能为1\2\3\4
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
名称：Lcd12864PrintfOne
功能：字符串第一行显示,停留1秒后清空
******************************************/
void Lcd12864PrintfOne(LPBYTE p)
{
	Lcd12864Printf(0,1,p);
	Delay1s(1);
	Lcd12864ClearDDRAM();
}


/******************************************
名称：Lcd12864ClearDDRAM
功能：清空DDRAM
******************************************/
void Lcd12864ClearDDRAM()
{
	WriteCommand(0x01);	
}


/******************************************
名称：Lcd12864ClearGDRAM
功能：清空GDRAM
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
	WriteCommand(0x36);//打开扩展功能模式，打开绘图显示
}

