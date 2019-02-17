#ifndef __LCD_12864_H__
#define __LCD_12864_H__
#include "reg52.h"
#include "Delay.h"
#define BYTE	unsigned char
#define WORD	unsigned int
#define LPBYTE	unsigned char *
#define LPWORD	unsigned int *
#define BOOL	bit
#define LCD_PORT P2
void Lcd12864Init();                           //初始化函数;
void Lcd12864DisplayImage(LPBYTE p1,LPBYTE p2);//图片显示函数
void Lcd12864Printf(BYTE x,BYTE y,LPBYTE p);   //在坐标(x,y)处显示字符串函数
void Lcd12864Play(LPBYTE p);
void Lcd12864ClearDDRAM();                     //清空DDRAM
void Lcd12864ClearGDRAM();                     //清空GDRAM
void WriteCommand(BYTE dat);
void Lcd12864PrintfOne(LPBYTE p);
#endif
