
#include "DELAY.h"

#define FOSC 22158684L

void DelayCycle(unsigned char delay){
	while(--delay);			   //4T
}

void Delay10us(unsigned char delay){
	delay++;
	while(--delay)
		DelayCycle(50);
}

void Delay25us(BYTE delay){
	delay++;
	while(--delay)
		DelayCycle(132);
}

void Delayms(unsigned char delay){
	delay++;
	while(--delay)
		Delay10us(100);
}


void Delay10ms(BYTE delay){
	delay++;
	while(--delay)
		Delayms(10);
}

void Delay1s(BYTE delay){
	delay++;
	while(--delay)
		Delay10ms(100);
}

