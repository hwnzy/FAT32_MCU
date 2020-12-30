#ifndef __DELAY_H__
#define BYTE unsigned char
#define WORD unsigned int
//#define FOSC 22158684L

#define Delay183ns(a) DelayCycle(a)


void DelayCycle(BYTE delay);
void Delay10us(BYTE delay);
void Delay25us(BYTE delay);
void Delayms(BYTE delay);
void Delay10ms(BYTE delay);
void Delay1s(BYTE delay);
#endif