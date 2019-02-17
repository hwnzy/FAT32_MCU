#include "SD.h"

unsigned char init_shift_down;//�ڳ�ʼ����ʱ�����ô˱���Ϊ1����˿���ͬ�����ݴ��䣨SPI�������ٶȱ����ȶ�
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
 ������sd_write_cmd
 ���ܣ���SD��д����
 ******************************************************************/
unsigned char sd_write_cmd(unsigned char *cmd)//��SD��д���pcmd�������ֽ����е��׵�ַ
{
	unsigned char temp,time=0;

	SD_CS=1;
	sd_spi_write(0xff);//��߼��������û�������ЩSD�����ܲ�֧��
	SD_CS=0;

	sd_spi_write(cmd[0]);
	sd_spi_write(cmd[1]);
	sd_spi_write(cmd[2]);
	sd_spi_write(cmd[3]);
	sd_spi_write(cmd[4]);
	sd_spi_write(cmd[5]);

	do{
		temp=sd_spi_read();//һֱ����֪�������Ĳ���0xff��ʱ
		time++;
	}while((temp==0xff)&&(time<TRY_TIME));

	return (temp);
}
/******************************************************************
 ������sd_reset
 ���ܣ���λSD�����õ�CMD0��ʹ��SD���л���SPIģʽ
 ******************************************************************/

unsigned char sd_reset()//SD����λ������SPIģʽ��ʹ��SMD0������0��
{
	unsigned char time,temp,i;
	unsigned char cmd[]={0x40,0x00,0x00,0x00,0x00,0x95};//����0���ֽ�����
	init_shift_down=1;//��is_init��Ϊ1
	SD_CS=1;//�ر�Ƭѡ
	for(i=0;i<0x0f;i++)//��λʱ�����ȱ���Ҫ��������74��ʱ���ź�
	{
		sd_spi_write(0xff);//120��ʱ��
	}
	SD_CS=0;//��Ƭѡ
	time=0;
	do{
		temp=sd_write_cmd(cmd);//д��CMD0
		time++;
		if(time==TRY_TIME)
		{
			SD_CS=1;//�ر�Ƭѡ
			return (INIT_CMD0_ERROR);//CMDOд��ʧ��
		}
	}while(temp!=0x01);
	SD_CS=1;//�ر�Ƭѡ
	sd_spi_write(0xff);//����SD���Ĳ���ʱ�������ﲹ8��ʱ��
	return 0;//����0��˵����λ�����ɹ�
}
/******************************************************************
 ������sd_reset
 ���ܣ���ʼ��SD����ʹ��CMD1
 ******************************************************************/
unsigned char sd_init()//��ʼ����ʹ��CMD1������1��
{
	unsigned char time,temp;
	unsigned char cmd0[]={0x40,0x00,0x00,0x00,0x00,0x95};//����0���ֽ�����
	unsigned char cmd8[]={0x48,0x00,0x00,0x01,0xaa,0x87};//����8���ֽ�����
	unsigned char cmd41[]={0x69,0x40,0x00,0x00,0x00,0xff};//����41���ֽ�����
	unsigned char cmd55[]={0x77,0x00,0x00,0x00,0x00,0xff};//����55���ֽ�����
	for(time=0;time<0x1f;time++)
	{
		sd_spi_write(0xff);
	}
	SD_CS=0;//��Ƭѡ
	time=0;
	do{
		temp=sd_write_cmd(cmd0);
		time++;
		if(time>=TRY_TIME)
		if(time>=TRY_TIME)
		{
			SD_CS=1;//�ر�Ƭѡ
			return (INIT_CMD1_ERROR);//CMD1д��ʧ��
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

	init_shift_down=0;//��ʼ����ϣ����Ծ���������ݴ�������
	
	SD_CS=1;//�ر�Ƭѡ
	sd_spi_write(0xff);//����SD���Ĳ���ʱ�������ﲹ8��ʱ��
	return (0);//����0��˵����ʼ�������ɹ�	
}
/****************************************************************************
 ������sd_reset
 ���ܣ���bufferָ���512���ֽڵ�����д�뵽SD����addr������
 ****************************************************************************/
unsigned charsd_write_sector(unsigned long addr,unsigned char *buffer) //��SD���е�ָ����ַ������д��512���ֽڣ�ʹ��CMD24������24��
{  
 	unsigned char temp,time;
 	unsigned int i;
 	unsigned char cmd[]={0x58,0x00,0x00,0x00,0x00,0xff}; //��SD���е����飨512�ֽڣ�һ��������д�����ݣ���CMD24
 	//addr<<=9; //addr = addr * 512 �����ַ��������ַ��תΪ�ֽڵ�ַ �������������SD�����������Ϊ4G��
 
 	cmd[1]=((addr&0xff000000)>>24); //���ֽڵ�ַд�뵽CMD24�ֽ�������
 	cmd[2]=((addr&0x00ff0000)>>16);
 	cmd[3]=((addr&0x0000ff00)>>8);
 	cmd[4]=(addr&0x000000ff);
 	SD_CS=0;//��SD��Ƭѡ
 	time=0;
 	do
 	{  
 	 	temp=sd_write_cmd(cmd);
 		time++;
  	if(time==TRY_TIME) 
  	{ 
   		SD_CS=1; //�ر�Ƭѡ
   		return(temp); //����д��ʧ��
  	}
 	}while(temp!=0);

 	for(i=0;i<100;i++) //����Ҫ��������ʱ���ź�
 	{
  		sd_spi_write(0xff);
 	} 

 	sd_spi_write(0xfe);//д�뿪ʼ�ֽ� 0xfe���������Ҫд���512���ֽڵ����� 

 	for(i=0;i<512;i++) //����������Ҫд���512���ֽ�д��SD��
 	{
  		sd_spi_write(buffer[i]);
 	}

	sd_spi_write(0xff); 
 	sd_spi_write(0xff); //�����ֽڵ�CRCУ���룬���ù��� 
 	temp=sd_spi_read();   //��ȡ����ֵ

 	if((temp&0x1F)!=0x05) //�������ֵ�� XXX00101˵�������Ѿ���SD��������
 	{
  		SD_CS=1;
  		return(WRITE_BLOCK_ERROR); //д������ʧ��
 	}
  	while(sd_spi_read()!=0xff);//�ȵ�SD����æ�����ݱ������Ժ�SD��Ҫ����Щ����д�뵽�����FLASH�У���Ҫһ��ʱ�䣩
          //æʱ����������ֵΪ0x00,��æʱ��Ϊ0xff
	SD_CS=1; //�ر�Ƭѡ
 	sd_spi_write(0xff);//����SD���Ĳ���ʱ�������ﲹ8��ʱ��
 	return(0);   //����0,˵��д���������ɹ�
}
/****************************************************************************
 ������sd_reset
 ���ܣ���ȡaddr������512���ֽڵ�bufferָ������ݻ�����
 ****************************************************************************/
unsigned char sd_read_sector(unsigned long addr,unsigned char *buffer)//��SD����ָ�������ж���512���ֽڣ�ʹ��CMD17��17�����
{
 	unsigned int j;
 	unsigned char time,temp;
 	unsigned char cmd[]={0x51,0x00,0x00,0x00,0x00,0xff}; //CMD17���ֽ�����
   
 	//addr<<=9; //addr=addr*512    �����ַ��������ַ��תΪ�ֽڵ�ַ
 	cmd[1]=((addr&0xff000000)>>24);//���ֽڵ�ַд�뵽CMD17�ֽ�������
 	cmd[2]=((addr&0x00FF0000)>>16);
 	cmd[3]=((addr&0x0000FF00)>>8);
 	cmd[4]=(addr&0x000000ff);
 	SD_CS=0;//��Ƭѡ
 	time=0;
 	do
 	{  
  		temp=sd_write_cmd(cmd); //д��CMD17
  		time++;
  		if(time==TRY_TIME) 
  	{
   		return(READ_BLOCK_ERROR); //����ʧ��
  	}
 	}while(temp!=0); 
      
 	while (sd_spi_read()!= 0xfe); //һֱ����������0xfeʱ��˵���������512�ֽڵ�������
	for(j=0;j<512;j++)  //������д�뵽���ݻ�������
 	{/*512��������û���ⲿ��FLASH����������ֻ����RAM�д���һ��pcmb1[5]�����ڼ���*/ 
  		buffer[j]=sd_spi_read();
 	}
 	sd_spi_read();
 	sd_spi_read();//��ȡ�����ֽڵ�CRCУ���룬���ù�������
 	SD_CS=1;  //SD���ر�Ƭѡ
 	sd_spi_write(0xff);//����SD���Ĳ���ʱ�������ﲹ8��ʱ��
 	return 0;
}
/******************************************************************
 ������sd_reset
 ���ܣ�IOģ��SPI������һ���ֽ�
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
 ������sd_reset
 ���ܣ�IOģ��SPI����ȡһ���ֽ�
******************************************************************/

unsigned char sd_spi_read() //SPI��һ���ֽ�
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

