#ifndef __SD_H__
#define __SD_H__
#include "reg52.h"
sbit SD_SCL=P3^0; //SD卡同步时钟  输入
sbit SD_SI =P3^1; //SD卡同步数据  输入
sbit SD_CS =P3^2; //SD卡片选 	  输入
sbit SD_SO =P3^3; //SD卡同步数据  输出
#define DELAY_TIME 2000 //SD卡的复位与初始化时SPI的延时参数，根据实际速率修改其值，否则会造成SD卡复位或初始化失败
#define TRY_TIME 200    //向SD卡写入命令之后，读取SD卡的回应次数，即读TRY_TIME次，如果在TRY_TIME次中读不到回应，产生超时错误，命令写入失败
#define INIT_CMD0_ERROR     0x01 //CMD0错误
#define INIT_CMD1_ERROR     0x02 //CMD1错误
#define WRITE_BLOCK_ERROR   0x03 //写块错误
#define READ_BLOCK_ERROR    0x04 //读块错误
unsigned char sd_write_cmd(unsigned char *cmd);
unsigned char sd_reset();
unsigned char sd_init();
unsigned char sd_write_sector(unsigned long addr,unsigned char *buffer);
unsigned char sd_read_sector(unsigned long addr,unsigned char *buffer);
void sd_spi_write(unsigned char x);
unsigned char sd_spi_read();
#endif