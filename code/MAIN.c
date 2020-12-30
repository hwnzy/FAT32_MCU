#include "FAT.h"
#include "LCD_12864.h"
unsigned char * data file_cache;
unsigned char xdata Cache0[512];
unsigned char xdata Cache1[512];
unsigned char data num[6];
unsigned int times = 0;

void NumToString5(unsigned int num_int,unsigned char *p)
{	   //65535
	if(num_int) {
			p[0] = ('0'+(num_int/10000));
			p[1] = ('0'+(num_int%10000/1000));
			p[2] = ('0'+(num_int%10000%1000/100));
			p[3] = ('0'+(num_int%10000%1000%100/10));	
			p[4] = ('0'+(num_int%10000%1000%100%10));
			p[5] = '\0';
	}
}
	 
void NumToString(unsigned char num_char,unsigned char *p)
{
	if(num_char) {
			p[0] = ('0'+(num_char/100));
			p[1] = ('0'+((num_char%100)/10));
			p[2] = ('0'+((num_char%100)%10));
			p[3] = '\0';		
	}
}

void main()
{
	unsigned int Len = 0;
	unsigned char code File[] = "VEDIO.BIN";
	file_cache = Cache0;
	Lcd12864Init();
	Lcd12864Printf(0,1,"FAT32 by nzy");
	Delay1s(1);
	Lcd12864Printf(0,2,"������...");
	switch(FATFSInitial(file_cache))
	{
		case FATFS_FS_NOT_SUPPORT :	
			Lcd12864Printf(0,3,"����FATϵͳ"); break;

		case FATFS_FATFS_NOT_SUPPORT :
			Lcd12864Printf(0,3,"��֧��FATFS");	break;

		case FATFS_OPERATION_SUCCESS :
			Lcd12864Printf(0,3,"�����ɹ�"); break;

		case FATFS_READ_BPB_SEC_ERROR :
			Lcd12864Printf(0,3,"��BPB����"); break;
		
		case FATFS_DEVICE_INITIAL_ERROR	:
			Lcd12864Printf(0,3,"��ʼ������"); break;

		case FATFS_DEVICE_RESET_ERROR :
			Lcd12864Printf(0,3,"��λ����"); break;

		default :
			Lcd12864Printf(0,3,"δ֪����"); break;
	}
	Delay1s(1);
	Lcd12864ClearDDRAM();	
	Lcd12864Printf(0,1,"��������");
	NumToString5(g_wRsvdSecCnt,num);  //����ת��Ϊ�ַ���
	Lcd12864Printf(0,2,num);

	Lcd12864Printf(0,3,"ÿ������");
	NumToString(g_bSecPerClus,num);
	Lcd12864Printf(0,4,num);

	Delay1s(1);
	Lcd12864ClearDDRAM();

    Lcd12864Printf(0,1,"���ļ�����");
	Delay1s(1);
	Lcd12864Printf(0,2,"������...");


	switch(FATFSOpen(file_cache,File))
	{
		case FATFS_FILENAME_ERROR :	
			Lcd12864Printf(0,3,"�ļ�����ʧ��"); break;

		case FATFS_OPERATION_SUCCESS :
			Lcd12864Printf(0,3,"�����ɹ�");	break;

		case FATFS_OPEN_FILE_FAILED :
			Lcd12864Printf(0,3,"���ļ�ʧ��"); break;

		case FATFS_FAT_REQUIRE_INITIAL :
			Lcd12864Printf(0,3,"FATû�г�ʼ��"); break;

		default :
			Lcd12864Printf(0,3,"δ֪����"); break;
	}

	Delay1s(1);
	Lcd12864ClearDDRAM();

   	Lcd12864Printf(0,1,"��ʾ�ļ�����");
	Delay1s(1);
	Lcd12864Printf(0,2,"������...");
	Delay1s(1);



	switch(FATFSReadNextBlock(file_cache,&Len))
	{
		case FATFS_READ_FAT_ERROR :	
			Lcd12864Printf(0,3,"FAT������"); while(1);break;
	
		case FATFS_RETAINED_CLUSTER :
			Lcd12864Printf(0,3,"���Ǳ�����");	while(1);break;

		case FATFS_BAD_CLUSTER :
			Lcd12864Printf(0,3,"ֻ�ǻ���"); while(1);break;

		case FATFS_LAST_CLUSTER :
			Lcd12864Printf(0,3,"��������"); while(1);break;

		case FATFS_END_OF_FILE :
			Lcd12864Printf(0,3,"�ļ�ĩβ"); while(1);break;

		case FATFS_READ_SEC_ERROR :
			Lcd12864Printf(0,3,"����������"); while(1);break;
	
		case FATFS_OPERATION_SUCCESS :
			Lcd12864Printf(0,3,"�����ɹ�"); times++;break;
	
		case FATFS_READ_BLOCK_OVERRANG :		   
			Lcd12864Printf(0,3,"���������"); while(1);break;
	
		default :
			Lcd12864Printf(0,3,"δ֪����"); while(1);break;
	}
	NumToString5(times,num);
	Lcd12864Printf(0,4,num);					 
	

	Lcd12864ClearDDRAM();
	Lcd12864ClearGDRAM();
	WriteCommand(0x36);//����չ����ģʽ���򿪻�ͼ��ʾ
	while(1) {
		Lcd12864Play(file_cache);
		FATFSReadNextBlock(file_cache,&Len);		
	}
	while(1);
	/*	
	lcd_12864_display_words(0,1,"sprintf����");
	NumToString(abcd,num);
	lcd_12864_display_words(0,2,num);
	while(1);
	*/
}

