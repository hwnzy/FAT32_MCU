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
void Lcd12864Init();                           //��ʼ������;
void Lcd12864DisplayImage(LPBYTE p1,LPBYTE p2);//ͼƬ��ʾ����
void Lcd12864Printf(BYTE x,BYTE y,LPBYTE p);   //������(x,y)����ʾ�ַ�������
void Lcd12864Play(LPBYTE p);
void Lcd12864ClearDDRAM();                     //���DDRAM
void Lcd12864ClearGDRAM();                     //���GDRAM
void WriteCommand(BYTE dat);
void Lcd12864PrintfOne(LPBYTE p);
#endif
