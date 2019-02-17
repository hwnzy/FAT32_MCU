#ifndef __SD_H__
#define __SD_H__
#include "reg52.h"
sbit SD_SCL=P3^0; //SD��ͬ��ʱ��  ����
sbit SD_SI =P3^1; //SD��ͬ������  ����
sbit SD_CS =P3^2; //SD��Ƭѡ 	  ����
sbit SD_SO =P3^3; //SD��ͬ������  ���
#define DELAY_TIME 2000 //SD���ĸ�λ���ʼ��ʱSPI����ʱ����������ʵ�������޸���ֵ����������SD����λ���ʼ��ʧ��
#define TRY_TIME 200    //��SD��д������֮�󣬶�ȡSD���Ļ�Ӧ����������TRY_TIME�Σ������TRY_TIME���ж�������Ӧ��������ʱ��������д��ʧ��
#define INIT_CMD0_ERROR     0x01 //CMD0����
#define INIT_CMD1_ERROR     0x02 //CMD1����
#define WRITE_BLOCK_ERROR   0x03 //д�����
#define READ_BLOCK_ERROR    0x04 //�������
unsigned char sd_write_cmd(unsigned char *cmd);
unsigned char sd_reset();
unsigned char sd_init();
unsigned char sd_write_sector(unsigned long addr,unsigned char *buffer);
unsigned char sd_read_sector(unsigned long addr,unsigned char *buffer);
void sd_spi_write(unsigned char x);
unsigned char sd_spi_read();
#endif