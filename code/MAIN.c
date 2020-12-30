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
	Lcd12864Printf(0,2,"测试中...");
	switch(FATFSInitial(file_cache))
	{
		case FATFS_FS_NOT_SUPPORT :	
			Lcd12864Printf(0,3,"不是FAT系统"); break;

		case FATFS_FATFS_NOT_SUPPORT :
			Lcd12864Printf(0,3,"不支持FATFS");	break;

		case FATFS_OPERATION_SUCCESS :
			Lcd12864Printf(0,3,"操作成功"); break;

		case FATFS_READ_BPB_SEC_ERROR :
			Lcd12864Printf(0,3,"读BPB错误"); break;
		
		case FATFS_DEVICE_INITIAL_ERROR	:
			Lcd12864Printf(0,3,"初始化错误"); break;

		case FATFS_DEVICE_RESET_ERROR :
			Lcd12864Printf(0,3,"复位错误"); break;

		default :
			Lcd12864Printf(0,3,"未知错误"); break;
	}
	Delay1s(1);
	Lcd12864ClearDDRAM();	
	Lcd12864Printf(0,1,"保留扇区");
	NumToString5(g_wRsvdSecCnt,num);  //数字转换为字符串
	Lcd12864Printf(0,2,num);

	Lcd12864Printf(0,3,"每簇扇区");
	NumToString(g_bSecPerClus,num);
	Lcd12864Printf(0,4,num);

	Delay1s(1);
	Lcd12864ClearDDRAM();

    Lcd12864Printf(0,1,"打开文件测试");
	Delay1s(1);
	Lcd12864Printf(0,2,"测试中...");


	switch(FATFSOpen(file_cache,File))
	{
		case FATFS_FILENAME_ERROR :	
			Lcd12864Printf(0,3,"文件命名失败"); break;

		case FATFS_OPERATION_SUCCESS :
			Lcd12864Printf(0,3,"操作成功");	break;

		case FATFS_OPEN_FILE_FAILED :
			Lcd12864Printf(0,3,"打开文件失败"); break;

		case FATFS_FAT_REQUIRE_INITIAL :
			Lcd12864Printf(0,3,"FAT没有初始化"); break;

		default :
			Lcd12864Printf(0,3,"未知错误"); break;
	}

	Delay1s(1);
	Lcd12864ClearDDRAM();

   	Lcd12864Printf(0,1,"显示文件测试");
	Delay1s(1);
	Lcd12864Printf(0,2,"测试中...");
	Delay1s(1);



	switch(FATFSReadNextBlock(file_cache,&Len))
	{
		case FATFS_READ_FAT_ERROR :	
			Lcd12864Printf(0,3,"FAT读错误"); while(1);break;
	
		case FATFS_RETAINED_CLUSTER :
			Lcd12864Printf(0,3,"这是保留簇");	while(1);break;

		case FATFS_BAD_CLUSTER :
			Lcd12864Printf(0,3,"只是坏簇"); while(1);break;

		case FATFS_LAST_CLUSTER :
			Lcd12864Printf(0,3,"这是最后簇"); while(1);break;

		case FATFS_END_OF_FILE :
			Lcd12864Printf(0,3,"文件末尾"); while(1);break;

		case FATFS_READ_SEC_ERROR :
			Lcd12864Printf(0,3,"读扇区错误"); while(1);break;
	
		case FATFS_OPERATION_SUCCESS :
			Lcd12864Printf(0,3,"操作成功"); times++;break;
	
		case FATFS_READ_BLOCK_OVERRANG :		   
			Lcd12864Printf(0,3,"读最后扇区"); while(1);break;
	
		default :
			Lcd12864Printf(0,3,"未知错误"); while(1);break;
	}
	NumToString5(times,num);
	Lcd12864Printf(0,4,num);					 
	

	Lcd12864ClearDDRAM();
	Lcd12864ClearGDRAM();
	WriteCommand(0x36);//打开扩展功能模式，打开绘图显示
	while(1) {
		Lcd12864Play(file_cache);
		FATFSReadNextBlock(file_cache,&Len);		
	}
	while(1);
	/*	
	lcd_12864_display_words(0,1,"sprintf检验");
	NumToString(abcd,num);
	lcd_12864_display_words(0,2,num);
	while(1);
	*/
}

