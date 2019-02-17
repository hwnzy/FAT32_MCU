#include "SD.h"

unsigned char init_shift_down;//在初始化的时候，设置此变量为1，如此可在同步数据传输（SPI）降低速度保持稳定
unsigned char bdata sd_data;
sbit dat0=sd_data^0;
sbit dat1=sd_data^1;
sbit dat2=sd_data^2;
sbit dat3=sd_data^3;
sbit dat4=sd_data^4;
sbit dat5=sd_data^5;
sbit dat6=sd_data^6;
sbit dat7=sd_data^7;

void NZYdelay(unsigned int time) 
{
 while(time--);
}
/******************************************************************
 函数：sd_write_cmd
 功能：向SD卡写命令
 ******************************************************************/
unsigned char sd_write_cmd(unsigned char *cmd)//向SD卡写命令，pcmd是命令字节序列的首地址
{
	unsigned char temp,time=0;

	SD_CS=1;
	sd_spi_write(0xff);//提高兼容性如果没有这里，有些SD卡可能不支持
	SD_CS=0;

	sd_spi_write(cmd[0]);
	sd_spi_write(cmd[1]);
	sd_spi_write(cmd[2]);
	sd_spi_write(cmd[3]);
	sd_spi_write(cmd[4]);
	sd_spi_write(cmd[5]);

	do{
		temp=sd_spi_read();//一直读，知道读到的不是0xff或超时
		time++;
	}while((temp==0xff)&&(time<TRY_TIME));

	return (temp);
}
/******************************************************************
 函数：sd_reset
 功能：复位SD卡，用到CMD0，使用SD卡切换到SPI模式
 ******************************************************************/

unsigned char sd_reset()//SD卡复位，进入SPI模式，使用SMD0（命令0）
{
	unsigned char time,temp,i;
	unsigned char cmd[]={0x40,0x00,0x00,0x00,0x00,0x95};//命令0的字节序列
	init_shift_down=1;//将is_init置为1
	SD_CS=1;//关闭片选
	for(i=0;i<0x0f;i++)//复位时，首先必须要发送最少74个时钟信号
	{
		sd_spi_write(0xff);//120个时钟
	}
	SD_CS=0;//打开片选
	time=0;
	do{
		temp=sd_write_cmd(cmd);//写入CMD0
		time++;
		if(time==TRY_TIME)
		{
			SD_CS=1;//关闭片选
			return (INIT_CMD0_ERROR);//CMDO写入失败
		}
	}while(temp!=0x01);
	SD_CS=1;//关闭片选
	sd_spi_write(0xff);//按照SD卡的操作时序在这里补8个时钟
	return 0;//返回0，说明复位操作成功
}
/******************************************************************
 函数：sd_reset
 功能：初始化SD卡，使用CMD1
 ******************************************************************/
unsigned char sd_init()//初始化，使用CMD1（命令1）
{
	unsigned char time,temp;
	unsigned char cmd0[]={0x40,0x00,0x00,0x00,0x00,0x95};//命令0的字节序列
	unsigned char cmd8[]={0x48,0x00,0x00,0x01,0xaa,0x87};//命令8的字节序列
	unsigned char cmd41[]={0x69,0x40,0x00,0x00,0x00,0xff};//命令41的字节序列
	unsigned char cmd55[]={0x77,0x00,0x00,0x00,0x00,0xff};//命令55的字节序列
	for(time=0;time<0x1f;time++)
	{
		sd_spi_write(0xff);
	}
	SD_CS=0;//打开片选
	time=0;
	do{
		temp=sd_write_cmd(cmd0);
		time++;
		if(time>=TRY_TIME)
		if(time>=TRY_TIME)
		{
			SD_CS=1;//关闭片选
			return (INIT_CMD1_ERROR);//CMD1写入失败
		}
	}while(temp!=0x01);
	time=0;
	do{
		temp=sd_write_cmd(cmd8);
		time++;
		if(time>=TRY_TIME)
		{
			SD_CS=1;
			return (INIT_CMD1_ERROR);
		}
	}while(temp!=0x01);
	time=0;
	do{
		temp=sd_write_cmd(cmd55);
		time++;
		if(time>=TRY_TIME)
		{
			SD_CS=1;
			return (INIT_CMD1_ERROR);
		}
	}while(sd_write_cmd(cmd41)!=0x00);

	init_shift_down=0;//初始化完毕，可以尽量提高数据传输速率
	
	SD_CS=1;//关闭片选
	sd_spi_write(0xff);//按照SD卡的操作时序在这里补8个时钟
	return (0);//返回0，说明初始化操作成功	
}
/****************************************************************************
 函数：sd_reset
 功能：将buffer指向的512个字节的数据写入到SD卡的addr扇区中
 ****************************************************************************/
unsigned charsd_write_sector(unsigned long addr,unsigned char *buffer) //向SD卡中的指定地址的扇区写入512个字节，使用CMD24（命令24）
{  
 	unsigned char temp,time;
 	unsigned int i;
 	unsigned char cmd[]={0x58,0x00,0x00,0x00,0x00,0xff}; //向SD卡中单个块（512字节，一个扇区）写入数据，用CMD24
 	//addr<<=9; //addr = addr * 512 将块地址（扇区地址）转为字节地址 ［这里就限制了SD卡的最大容量为4G］
 
 	cmd[1]=((addr&0xff000000)>>24); //将字节地址写入到CMD24字节序列中
 	cmd[2]=((addr&0x00ff0000)>>16);
 	cmd[3]=((addr&0x0000ff00)>>8);
 	cmd[4]=(addr&0x000000ff);
 	SD_CS=0;//打开SD卡片选
 	time=0;
 	do
 	{  
 	 	temp=sd_write_cmd(cmd);
 		time++;
  	if(time==TRY_TIME) 
  	{ 
   		SD_CS=1; //关闭片选
   		return(temp); //命令写入失败
  	}
 	}while(temp!=0);

 	for(i=0;i<100;i++) //这里要插入若干时钟信号
 	{
  		sd_spi_write(0xff);
 	} 

 	sd_spi_write(0xfe);//写入开始字节 0xfe，后面就是要写入的512个字节的数据 

 	for(i=0;i<512;i++) //将缓冲区中要写入的512个字节写入SD卡
 	{
  		sd_spi_write(buffer[i]);
 	}

	sd_spi_write(0xff); 
 	sd_spi_write(0xff); //两个字节的CRC校验码，不用关心 
 	temp=sd_spi_read();   //读取返回值

 	if((temp&0x1F)!=0x05) //如果返回值是 XXX00101说明数据已经被SD卡接受了
 	{
  		SD_CS=1;
  		return(WRITE_BLOCK_ERROR); //写块数据失败
 	}
  	while(sd_spi_read()!=0xff);//等到SD卡不忙（数据被接受以后，SD卡要将这些数据写入到自身的FLASH中，需要一个时间）
          //忙时，读回来的值为0x00,不忙时，为0xff
	SD_CS=1; //关闭片选
 	sd_spi_write(0xff);//按照SD卡的操作时序在这里补8个时钟
 	return(0);   //返回0,说明写扇区操作成功
}
/****************************************************************************
 函数：sd_reset
 功能：读取addr扇区的512个字节到buffer指向的数据缓冲区
 ****************************************************************************/
unsigned char sd_read_sector(unsigned long addr,unsigned char *buffer)//从SD卡的指定扇区中读出512个字节，使用CMD17（17号命令）
{
 	unsigned int j;
 	unsigned char time,temp;
 	unsigned char cmd[]={0x51,0x00,0x00,0x00,0x00,0xff}; //CMD17的字节序列
   
 	//addr<<=9; //addr=addr*512    将块地址（扇区地址）转为字节地址
 	cmd[1]=((addr&0xff000000)>>24);//将字节地址写入到CMD17字节序列中
 	cmd[2]=((addr&0x00FF0000)>>16);
 	cmd[3]=((addr&0x0000FF00)>>8);
 	cmd[4]=(addr&0x000000ff);
 	SD_CS=0;//打开片选
 	time=0;
 	do
 	{  
  		temp=sd_write_cmd(cmd); //写入CMD17
  		time++;
  		if(time==TRY_TIME) 
  	{
   		return(READ_BLOCK_ERROR); //读块失败
  	}
 	}while(temp!=0); 
      
 	while (sd_spi_read()!= 0xfe); //一直读，当读到0xfe时，说明后面的是512字节的数据了
	for(j=0;j<512;j++)  //将数据写入到数据缓冲区中
 	{/*512这里由于没有外部的FLASH储存器，故只是在RAM中创建一个pcmb1[5]，用于检验*/ 
  		buffer[j]=sd_spi_read();
 	}
 	sd_spi_read();
 	sd_spi_read();//读取两个字节的CRC校验码，不用关心它们
 	SD_CS=1;  //SD卡关闭片选
 	sd_spi_write(0xff);//按照SD卡的操作时序在这里补8个时钟
 	return 0;
}
/******************************************************************
 函数：sd_reset
 功能：IO模拟SPI，发送一个字节
 ******************************************************************/

void sd_spi_write(unsigned char x) 
{
 	sd_data=x;
 
 	SD_SI=dat7;
 	SD_SCL=0; 
 	if(init_shift_down) NZYdelay(DELAY_TIME); 
 	SD_SCL=1;
 	if(init_shift_down) NZYdelay(DELAY_TIME);

 	SD_SI=dat6;
 	SD_SCL=0; 
	if(init_shift_down) NZYdelay(DELAY_TIME); 
 	SD_SCL=1;
 	if(init_shift_down) NZYdelay(DELAY_TIME);

 	SD_SI=dat5;
 	SD_SCL=0;
 	if(init_shift_down) NZYdelay(DELAY_TIME);
 	SD_SCL=1;
 	if(init_shift_down) NZYdelay(DELAY_TIME);

 	SD_SI=dat4;
 	SD_SCL=0;
 	if(init_shift_down) NZYdelay(DELAY_TIME); 
 	SD_SCL=1;
 	if(init_shift_down) NZYdelay(DELAY_TIME);

 	SD_SI=dat3;
 	SD_SCL=0; 
	if(init_shift_down) NZYdelay(DELAY_TIME); 
 	SD_SCL=1;
 	if(init_shift_down) NZYdelay(DELAY_TIME);

 	SD_SI=dat2;
 	SD_SCL=0;
 	if(init_shift_down) NZYdelay(DELAY_TIME); 
 	SD_SCL=1;
 	if(init_shift_down) NZYdelay(DELAY_TIME);

 	SD_SI=dat1;
 	SD_SCL=0; 
 	if(init_shift_down) NZYdelay(DELAY_TIME); 
 	SD_SCL=1;
 	if(init_shift_down) NZYdelay(DELAY_TIME);

 	SD_SI=dat0;
 	SD_SCL=0;
 	if(init_shift_down) NZYdelay(DELAY_TIME);  
 	SD_SCL=1;
 	if(init_shift_down) NZYdelay(DELAY_TIME);
}

/******************************************************************
 函数：sd_reset
 功能：IO模拟SPI，读取一个字节
******************************************************************/

unsigned char sd_spi_read() //SPI读一个字节
{  
 	SD_SO=1;

 	SD_SCL=1;
	if(init_shift_down) NZYdelay(DELAY_TIME);
	SD_SCL=0;
 	if(init_shift_down) NZYdelay(DELAY_TIME); 
	dat7=SD_SO; 

 	SD_SCL=1;
 	if(init_shift_down) NZYdelay(DELAY_TIME);
 	SD_SCL=0;
 	if(init_shift_down) NZYdelay(DELAY_TIME);
 	dat6=SD_SO; 
	
 	SD_SCL=1;
 	if(init_shift_down) NZYdelay(DELAY_TIME);
 	SD_SCL=0;
 	if(init_shift_down) NZYdelay(DELAY_TIME);
 	dat5=SD_SO; 

 	SD_SCL=1;
	if(init_shift_down) NZYdelay(DELAY_TIME);
 	SD_SCL=0;
 	if(init_shift_down) NZYdelay(DELAY_TIME);
 	dat4=SD_SO; 

 	SD_SCL=1;
	if(init_shift_down) NZYdelay(DELAY_TIME);
 	SD_SCL=0;
 	if(init_shift_down) NZYdelay(DELAY_TIME); 
 	dat3=SD_SO; 

 	SD_SCL=1;
 	if(init_shift_down) NZYdelay(DELAY_TIME);
 	SD_SCL=0;
 	if(init_shift_down) NZYdelay(DELAY_TIME);
 	dat2=SD_SO; 
 	
 	SD_SCL=1;
 	if(init_shift_down) NZYdelay(DELAY_TIME);
 	SD_SCL=0;
 	if(init_shift_down) NZYdelay(DELAY_TIME);
 	dat1=SD_SO;

 	SD_SCL=1;
 	if(init_shift_down) NZYdelay(DELAY_TIME);
 	SD_SCL=0;
 	if(init_shift_down) NZYdelay(DELAY_TIME);
 	dat0=SD_SO; 
 
 	return (sd_data);
}

