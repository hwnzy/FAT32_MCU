#include "FAT.h"
#include "SD.h"

#define PRINTF			 Lcd12864PrintfOne		 //打印		参数：字符串指针

#define FATFSDEVICERESET	!sd_reset			 //设备复位	无参	返回：BOOL 成功：TRUE
#define FATFSDEVICEINITIAL	!sd_init		 	 //设备初始化	无参	返回：BOOL 成功：TRUE
#define FATFSDEVICEREADBLOCK	!sd_read_sector			 //设备读块	参数：DWORD扇区地址，扇区缓存(512B)	返回：BOOL 成功：TRUE
#define FATFSDEVICEWRITEBLOCK	!sd_write_sector		 //设备写块	参数：DWORD扇区地址，扇区缓存(512B)	返回：BOOL 成功：TRUE

#define BPB_POINT			BPB_Point
#define BPB_BYTEPERSEC		WORDLE(BPB_POINT[11], BPB_POINT[12])
#define BPB_SECPERCLUS		BYTELE(BPB_POINT[13])
#define BPB_RSVDSECCNT		WORDLE(BPB_POINT[14], BPB_POINT[15])
#define BPB_NUMFATS			BYTELE(BPB_POINT[16])
#define BPB_ROOTENTCNT		WORDLE(BPB_POINT[17], BPB_POINT[18])
#define BPB_TOTSEC16		WORDLE(BPB_POINT[19], BPB_POINT[20])
#define BPB_MEDIA			BYTELE(BPB_POINT[21])
#define BPB_FATSZ16			WORDLE(BPB_POINT[22],BPB_POINT[23])
#define BPB_SECPERTRK		WORDLE(BPB_POINT[24], BPB_POINT[25])
#define BPB_NUMHEADS		WORDLE(BPB_POINT[26], BPB_POINT[27])
#define BPB_HIDDSEC			DWORDLE(BPB_POINT[28], BPB_POINT[29], BPB_POINT[30], BPB_POINT[31])
#define BPB_TOTSEC32		DWORDLE(BPB_POINT[32], BPB_POINT[33], BPB_POINT[34], BPB_POINT[35])
//FAT 32
#define BPB_FATSZ32			DWORDLE(BPB_POINT[36], BPB_POINT[37], BPB_POINT[38], BPB_POINT[39])
#define BPB_EXTFLAGS		WORDLE(BPB_POINT[40], BPB_POINT[41])
#define BPB_FSVER			WORDLE(BPB_POINT[42], BPB_POINT[43])
#define BPB_ROOTCLUS		DWORDLE(BPB_POINT[44], BPB_POINT[45], BPB_POINT[46], BPB_POINT[47])
#define BPB_FSINFO			WORDLE(BPB_POINT[48], BPB_POINT[49])
#define BPB_BKBOOTSEC		WORDLE(BPB_POINT[50], BPB_POINT[51])
//FSInfo
#define FSI_POINT			FSI_Point
#define FSI_LEADSIG			DWORDLE(FSI_POINT[0], FSI_POINT[1], FSI_POINT[2], FSI_POINT[3])
#define FSI_STRUCSIG		DWORDLE(FSI_POINT[4], FSI_POINT[5], FSI_POINT[6], FSI_POINT[7])
#define FSI_FREE_COUNT		DWORDLE(FSI_POINT[488], FSI_POINT[489], FSI_POINT[490], FSI_POINT[491])
#define FSI_NXT_FREE		DWORDLE(FSI_POINT[492], FSI_POINT[493], FSI_POINT[494], FSI_POINT[495])

//BPBSec
#define BPB_SEC             DWORDLE(BPB_POINT[454], BPB_POINT[455], BPB_POINT[456], BPB_POINT[457]);

#define SHORTNAMELENCACHELEN 13		 	 //命名空间大小
#define FAT16MAXPERCHECKCLUS 4			 //FAT16最大预处理簇
#define FAT32MAXPERCHECKCLUS 4			 //FAT32最大预处理簇
#define UNDWORD union dword

sbit TPIN = P1^0;



struct Byte4 {
	BYTE a;
	BYTE b;
	BYTE c;
	BYTE d;
};

struct Word2 {
	WORD a;
	WORD b;
};

union dword {
	DWORD Data;
	struct Word2 word;
	struct Byte4 byte;
};

struct FATFSTemp13 {
	union dword dworda;
	union dword dwordb;
	WORD worda;
	WORD wordb;
	BYTE bytea;
};

union FATFSStringDataUnion {
	struct FATFSTemp13 Data; 	
	BYTE String[SHORTNAMELENCACHELEN];
};

#define g_wBytePerSec 512
#ifndef g_wBytePerSec			  		//暂时只支持512byte/扇区的储存介质
WORD  data g_wBytePerSec;
#endif

union FATFSStringDataUnion g_unTemp;						  
#define g_szFileShortName g_unTemp.String		//命名空间复用
#ifndef g_szFileShortName
BYTE  data g_szFileShortName[SHORTNAMELENCACHELEN];
#endif

BOOL  g_IsFATFS = FALSE;
BOOL  g_IsFAT16 = TRUE;
DWORD data g_dwFileFirstClus;
UNDWORD data g_dwFileCurrentClus;
DWORD data g_dwFileSize;
UNDWORD data g_dwFileCurrentSec;
DWORD data g_dwTotalClus;
BYTE  data g_bFileAttrib;
WORD  data g_wRsvdSecCnt;
BYTE  data g_bSecPerClus;
BYTE  data g_bSecPerClusBin;
DWORD data g_dwFirstRootDirNum;
DWORD data g_dwFirstDataSector;
BYTE  data g_bNextBlock;
DWORD data g_dwFATSz;
DWORD data g_dHddsec;
#ifdef _FATFSWRITE_
BYTE  data g_bNumFATs;
WORD  data g_wFSInfoNxtFree;    //FAT32用作 FSInfo指针 FAT16用作最后空扇区搜索
#endif

/*************************************************************************
函数：AnalysisMBR
描述：分析MBR，找出BPB的物理地址
*************************************************************************/
WORD AnalysisMBR(LPBYTE BPB_POINT)
{
	if(FATFSDEVICEREADBLOCK(0, BPB_POINT)){
		return BPB_SEC;
	}else{
		g_IsFATFS = FALSE;
		RETURN(FATFS_READ_BPB_SEC_ERROR);
	}	
}


/*************************************************************************
 函数：FATFSShortName
 描述：8.3文件名转换	
*************************************************************************/
BYTE FATFSShortName(BYTE NameLen){
	BYTE ExtLen;
	BYTE Cache[3]={0x20,0x20,0x20}; //ASCII码值代表空格
	for(NameLen=0;g_szFileShortName[NameLen]!='.'&&g_szFileShortName[NameLen]!='\0';++NameLen); //停在'.'的位置
	if(g_szFileShortName[NameLen]!='\0'){ 
		++NameLen;
		for(ExtLen=0;g_szFileShortName[NameLen + ExtLen]!='\0';++ExtLen){
				Cache[ExtLen] = g_szFileShortName[NameLen + ExtLen];
		}
		--NameLen;//回到'.'的位置
	}
	if(NameLen<9){
		for(;NameLen<11;++NameLen){
			g_szFileShortName[NameLen] = ' ';
		}
		NameLen = 8;
		for(;NameLen<11;++NameLen){
			g_szFileShortName[NameLen] = Cache[NameLen-8]; //插入后缀名
		}
		g_szFileShortName[11] = '\0';  //再次插入字符串结束标志
		RETURN(FATFS_OPERATION_SUCCESS);
	}else{
		RETURN(FATFS_NAME_NAME_TOO_LONG);
	}
}

/*************************************************************************
 函数：	InitialFATFS
 描述：	初始化FATFS并获得该FAT信息
*************************************************************************/
BYTE FATFSInitial(LPBYTE BPB_POINT){								 //g_dwTotalClus 作临时变量
	if(FATFSDEVICERESET()){
		if(FATFSDEVICEINITIAL()){
		   	if(FATFSDEVICEREADBLOCK(AnalysisMBR(BPB_POINT), BPB_POINT)){ 
			//if(FATFSDEVICEREADBLOCK(8192, BPB_POINT)){				//获得0扇区上的BPB只支持一个分区了不然要分析MBR
				if(BPB_POINT[510]!=0x55 && BPB_POINT[510]!=0xAA){		//AA55和55AA都可以，AA55一般为硬盘主引导扇区末尾
					g_IsFATFS = FALSE;
					RETURN(FATFS_FS_NOT_SUPPORT);
				}
				g_dHddsec = BPB_HIDDSEC; //隐藏扇区数
				g_wRsvdSecCnt = BPB_RSVDSECCNT; //保留扇区数
				g_dwFATSz = BPB_FATSZ32; //FAT表所占扇区数
				g_dwFirstDataSector = g_wRsvdSecCnt + BPB_NUMFATS * g_dwFATSz + g_dwTotalClus + g_dHddsec; //数据区第一个扇区，记得加上隐藏扇区数
				g_dwTotalClus = BPB_TOTSEC32 - g_dwFirstDataSector + g_dHddsec; //从BPB中获得的总扇区数包括隐藏扇区数
				g_bSecPerClus = BPB_SECPERCLUS;	//每簇扇区数
				g_dwTotalClus = (g_dwTotalClus + (g_bSecPerClus/2)) / g_bSecPerClus; //数据区簇数 = 数据扇区数 / 每簇扇区数
				if(g_dwTotalClus > 65525UL){
					g_dwFirstRootDirNum = BPB_ROOTCLUS;	//FAT32特有，根目录所在第一个簇簇号，通常为2
					g_IsFAT16 = FALSE;
				}else{
					g_IsFATFS = FALSE;
					RETURN(FATFS_FATFS_NOT_SUPPORT);
				}
#ifndef g_wBytePerSec    
				g_wBytePerSec = BPB_BYTEPERSEC;
#else			//暂时只支持512byte/扇区的储存介质
				if(g_wBytePerSec != BPB_BYTEPERSEC){
					RETURN(FATFS_FATFS_NOT_SUPPORT);
				}
#endif				
				g_bSecPerClusBin = 0; //取二的幂
				while(g_bSecPerClus > 1){
					 ++g_bSecPerClusBin;
					 g_bSecPerClus >>= 1;  //每簇扇区数/2
				}
				g_dwFileFirstClus = g_dwFirstRootDirNum; //文件开始簇 = 根目录所在首簇 / 扇区号
				g_bSecPerClus = BPB_SECPERCLUS;	//值已被更改，所以重新赋值
				g_bNextBlock = 0; //下一块要读的扇区
				g_IsFATFS = TRUE; //判断为FAT32系统
				RETURN(FATFS_OPERATION_SUCCESS);
			}else{
				g_IsFATFS = FALSE;
				RETURN(FATFS_READ_BPB_SEC_ERROR);
			}
		}else{
			g_IsFATFS = FALSE;
			RETURN(FATFS_DEVICE_INITIAL_ERROR);
		}
	}else{ 
		g_IsFATFS = FALSE;
		RETURN(FATFS_DEVICE_RESET_ERROR);
	}
}

/*************************************************************************
 函数：	FATFSReadNextBlock
 描述：	按顺序读出簇链上的扇区
*************************************************************************/
BYTE FATFSReadNextBlock(LPBYTE FileCache, LPWORD BlockLen){
	if(g_bNextBlock>=g_bSecPerClus){ //已经读的扇区数>=每簇扇区数
		//跳到下一个簇
			//FAT32
			if(g_unTemp.Data.bytea == 0){	//不是连续的簇链
				g_dwFileCurrentClus.Data<<=2;  //对FAT32而言，每个簇占据4个字节
				g_unTemp.Data.dworda.Data = (g_dwFileCurrentClus.Data / g_wBytePerSec) + g_wRsvdSecCnt + g_dHddsec;//该文件所在的FAT扇区数，加上隐藏扇区
				if(!FATFSDEVICEREADBLOCK(g_unTemp.Data.dworda.Data, FileCache)){ //对相应的FAT扇区进行读块
					*BlockLen = 0; //长度为0
					RETURN(FATFS_READ_FAT_ERROR); //返回读取FAT表失败
				}
				g_unTemp.Data.worda = g_dwFileCurrentClus.Data%g_wBytePerSec; //文件所在FAT表的表项数
				g_dwFileCurrentClus.byte.a = FileCache[g_unTemp.Data.worda+3];	//获取文件下一簇簇号	C51:大端
				g_dwFileCurrentClus.byte.b = FileCache[g_unTemp.Data.worda+2];	//获取文件下一簇簇号	X86:小端
				g_dwFileCurrentClus.byte.c = FileCache[g_unTemp.Data.worda+1];  //获取文件下一簇簇号
				g_dwFileCurrentClus.byte.d = FileCache[g_unTemp.Data.worda];	//获取文件下一簇簇号
				g_unTemp.Data.dwordb.Data = g_dwFileCurrentClus.Data;  //得到文件下一簇簇号
				while(g_unTemp.Data.bytea < FAT32MAXPERCHECKCLUS){	   //FATFS最大预处理簇
					g_dwFileCurrentSec.Data = g_unTemp.Data.dwordb.Data << 2;  //文件下一簇号 X 每簇号占据4字节
					if(g_unTemp.Data.dworda.Data == ((g_dwFileCurrentSec.Data / g_wBytePerSec) + g_wRsvdSecCnt)){  //如果文件的下一簇的FAT表项数还在原来的扇区内 
						g_unTemp.Data.worda = g_dwFileCurrentSec.Data % g_wBytePerSec; //获取表项数
						g_dwFileCurrentSec.byte.a = FileCache[g_unTemp.Data.worda+3];  //获取文件下下一簇簇号
						g_dwFileCurrentSec.byte.b = FileCache[g_unTemp.Data.worda+2];  //获取文件下下一簇簇号
						g_dwFileCurrentSec.byte.c = FileCache[g_unTemp.Data.worda+1];  //获取文件下下一簇簇号
						g_dwFileCurrentSec.byte.d = FileCache[g_unTemp.Data.worda];	   //获取文件下下一簇簇号
						if(g_dwFileCurrentSec.Data == g_unTemp.Data.dwordb.Data + 1){ //如果下下一簇刚好接着下一簇
							g_unTemp.Data.bytea++;
							g_unTemp.Data.dwordb.Data = g_dwFileCurrentSec.Data;  //下一簇定义为下下一簇
						}else{
							break;
						}
					}else{
						break;
					}
				}
				if((g_dwFileCurrentClus.Data&=0x0FFFFFFFUL) > 0x0FFFFFEFUL){ //起始簇（名义上）或连续簇的首簇	【备注1】：见末尾
					*BlockLen = 0;					  //保留簇 坏簇 文件完
					if(g_dwFileCurrentClus.Data < 0x0FFFFFF6UL){
						RETURN(FATFS_RETAINED_CLUSTER); //保留簇
					}else if(g_dwFileCurrentClus.Data == 0x0FFFFFF7UL){
						RETURN(FATFS_BAD_CLUSTER); //坏簇
					}else{
						RETURN(FATFS_LAST_CLUSTER); //最后一簇
					}
				}
			}else{ //是连续的簇链
				g_dwFileCurrentClus.Data++; //文件起始（变化）簇号   接着下一簇
				g_unTemp.Data.bytea--; //连续簇数减1
			}	
		g_dwFileCurrentSec.Data = ((g_dwFileCurrentClus.Data - 2)<<g_bSecPerClusBin) + g_dwFirstDataSector;	//更新文件起始扇区号
		g_bNextBlock = 0; //已经读的扇区数 = 0
	}

	//当前簇
	if(g_bNextBlock<g_bSecPerClus){	//如果要读的扇区数 < 每簇扇区数
		if(BlockLen){ 	 //对常量而言，地址为0
			if(g_dwFileSize){ //文件大小不为0
				if(g_dwFileSize >= g_wBytePerSec){  //文件大小 >= 每扇区的字节数
					*BlockLen = g_wBytePerSec;		//实际读出长度 = 每扇区的字节数
					g_dwFileSize -= g_wBytePerSec;	//文件大小减少一个扇区
				}else{
					*BlockLen = g_dwFileSize;  //实际读出长度 = 文件大小
					g_dwFileSize = 0; //文件大小更新为0
				}
			}else{
				*BlockLen = 0; //实际读出的长度为0
				RETURN(FATFS_END_OF_FILE);	//返回文件结束
			}
		}
		if(!FATFSDEVICEREADBLOCK(g_dwFileCurrentSec.Data, FileCache)){  //如果对文件起始（变化）读块不成功
			*BlockLen = 0;	 //实际长度为0
			RETURN(FATFS_READ_SEC_ERROR); //返回读扇区失败
		}
		++g_dwFileCurrentSec.Data;	//加1为文件下一个扇区
		++g_bNextBlock;	//已经读的扇区加1
		RETURN(FATFS_OPERATION_SUCCESS);  //返回操作成功
	}else{
		RETURN(FATFS_READ_BLOCK_OVERRANG);	//已经完成读块
	}		
}

/*************************************************************************
 函数：	FATFSMatchAttrib
 描述：	确认目录的内容
*************************************************************************/
BYTE FATFSValidateAttrib(BYTE Attr){
	if((Attr&ATTR_LONG_NAME_MASK)==ATTR_LONG_NAME){
		return ATTR_LONG_NAME;
	}else{
		if((Attr&(ATTR_DIRECTORY|ATTR_VOLUME_ID))==0x00){
			//文件
			return ATTR_FILE;
		}else if((Attr&(ATTR_DIRECTORY|ATTR_VOLUME_ID))==ATTR_DIRECTORY){
			//目录
			return ATTR_DIRECTORY;
		}else if((Attr&(ATTR_DIRECTORY|ATTR_VOLUME_ID))==ATTR_VOLUME_ID){
			//卷标
			return ATTR_VOLUME_ID;
		}else{
			//什么都不是[暂且就识别这几个文件属性]
			return ATTR_NOTHING;
		}
	}
}

/*************************************************************************
 函数：	FATFSMatchDir 
 描述：	匹配FAT目录项中的名称并读取有关数据
*************************************************************************/
BYTE FATFSMatchDir(LPBYTE DirCache, BOOL IsFile){
	WORD j;
	BYTE k;
	for(j=0;j<g_wBytePerSec;j+=32){	//每个扇区字节数，每个目录项的大小为32字节
		if(DirCache[j] == 0xE5){	//说明该目录项被使用过，但已被删除
			continue;	//继续查找
		}
		if(DirCache[j] == 0x00){	//说明该目录项从未被使用过，由于目录项的分配是按顺序，故说明从这开始后面都为0X00
			RETURN(FATFS_END_OF_FATDIR);
		}
		for(k=0;k<11;++k){
			if(DirCache[j+k] == g_szFileShortName[k]){  //将该目录项的前11个ASCII码与数组进行对比
				continue;  //继续比较其余字符
			}else{
				break; //跳出
			}
		}
		if(k==11){
			g_bFileAttrib = BYTELE(DirCache[j+11]);	 //文件属性	 0x01-文件 0x01-只读 0x02-隐藏 0x04-系统文件 0x08-卷标 0x0f-表示该目录项为长文件名目录项 0x10-目录 0x20-存档													
			if(IsFile){	//文件							 													
				if(FATFSValidateAttrib(g_bFileAttrib)!=ATTR_FILE){ //查看是不是文件
					RETURN(FATFS_FILE_NOT_FOUND);
				}
			}else{
				if(FATFSValidateAttrib(g_bFileAttrib)!=ATTR_DIRECTORY){//查看是不是目录
					RETURN(FATFS_DIRECTORY_NOT_FOUND);
				}
			}
			g_dwFileFirstClus = DWORDLE(DirCache[j+26], DirCache[j+27], DirCache[j+20], DirCache[j+21]); //文件开始簇
			g_dwFileSize = DWORDLE(DirCache[j+28], DirCache[j+29], DirCache[j+30], DirCache[j+31]);	//文件内容大小字节数
			RETURN(FATFS_OPERATION_SUCCESS);
		}
	}
	RETURN(FATFS_CACHE_NOMATCH);  
}


/*************************************************************************
 函数：	FATFSOpenDir
 描述：	打开当前目录 并匹配该目录的文件
*************************************************************************/
BYTE FATFSDirOperation(LPBYTE DataCache, BOOL IsFile){
	BYTE k;
	do{	//只执行一次的“循环” 最优化代码
		if(g_dwFileFirstClus == g_dwFirstRootDirNum){ //如果是根目录
				g_dwFileCurrentClus.Data = g_dwFirstRootDirNum;	//FAT32根目录项表当文件打开，赋值根目录所在的簇号
		}else{ //不是根目录
			g_dwFileCurrentClus.Data = 	g_dwFileFirstClus; 
		}
		g_dwFileCurrentSec.Data = ((g_dwFileCurrentClus.Data - 2)<<g_bSecPerClusBin) + g_dwFirstDataSector;	 //文件开始扇区号（减2是因为FAT32是从2开始的）
		g_bNextBlock = 0; //要读的扇区块为0
		while(FATFSReadNextBlock(DataCache,0)==FATFS_OPERATION_SUCCESS){  //按顺序读出簇链上的扇区，读一块
			k = FATFSMatchDir(DataCache, IsFile); //将读到的数据与数组进行比较，进行匹配查找
			if(k == FATFS_CACHE_NOMATCH){  //没有找到
				continue; //再次循环
			}else{
				break; //跳出循环
			}
		}
	}while(0);
	if(k != FATFS_OPERATION_SUCCESS){ //如果没有匹配成功
		if(IsFile){	//是文件
			RETURN(FATFS_FILE_NOT_FOUND);
		}else{ //是文件夹（目录）
			RETURN(FATFS_DIRECTORY_NOT_FOUND);
		}
	}
	g_dwFileCurrentClus.Data = g_dwFileFirstClus; //文件数据开始簇
	g_dwFileCurrentSec.Data = ((g_dwFileCurrentClus.Data - 2)<<g_bSecPerClusBin) + g_dwFirstDataSector;	//文件起始扇区号
	g_bNextBlock = 0; //要读的扇区数
	g_unTemp.Data.bytea = 0; //簇链的连续簇数为0
	RETURN(FATFS_OPERATION_SUCCESS);
}

/*************************************************************************
 函数：	FATFSOpen
 描述：	打开指定路径的FAT文件
*************************************************************************/
BYTE FATFSOpen(LPBYTE DataCache, LPBYTE szPath){
//e.g.	"\ABC\DEF\BAD.BIN" "ABC\DEF\BAD.BIN" "\BAD.BIN" "BAD.BIN"
	BYTE i;
	if(g_IsFATFS){
		i = 0;
		if(szPath[0] == '\\'){	//查看目标路径的首字符是否是'\'
			szPath += 1; //跳过'\'
		}
		while(1){
			szPath = szPath + i;
			for(i=0;szPath[i]!='\0'&&szPath[i]!='\\'&&i<SHORTNAMELENCACHELEN-1;++i){ //将名称转移至数组g_szFileShortName
				g_szFileShortName[i] = szPath[i];
			}
			if(i==SHORTNAMELENCACHELEN-1){
				RETURN(FATFS_FILENAME_ERROR);
			}else{
				g_szFileShortName[i] = '\0';  //插入字符串结束标志
				if(FATFSShortName(i) != FATFS_OPERATION_SUCCESS){ //将8.3文件名进行转换				 
					break;
				}
				if(szPath[i]=='\0'){ //是否文件末尾
					if(FATFSDirOperation(DataCache, TRUE) == FATFS_OPERATION_SUCCESS){  //在目录项中匹配文件
						RETURN(FATFS_OPERATION_SUCCESS);
					}else{
						break;
					}
				}else{ //文件夹末尾
					++i;	//跳过"\"
					if(FATFSDirOperation(DataCache, FALSE) == FATFS_OPERATION_SUCCESS){ //在目录项中匹配文件夹
						continue;  //进行下一次的文件/文件夹名查找
					}else{
						break;
					}
				}
			}
		}
		RETURN(FATFS_OPEN_FILE_FAILED);
	}
	RETURN(FATFS_FAT_REQUIRE_INITIAL);
}




/*************************************************************************
 函数：	FATFSDebugErrLvlMsg
 描述：	输出FAT_ERROR_LEVEL定义到屏幕 定义_DEBUG_生效 RETURN 宏自动调用
*************************************************************************/
#ifdef _DEBUG_
void FATFSDebugErrLvlMsg(BYTE ErrLvl){
	switch(ErrLvl){
		case FATFS_OPERATION_SUCCESS:
			PRINTF("操作成功");
			break;
		case FATFS_DEVICE_RESET_ERROR:
			PRINTF("SD复位错误");
			break;
		case FATFS_DEVICE_INITIAL_ERROR:
			PRINTF("初始化成功");
			break;
		case FATFS_READ_BPB_SEC_ERROR:
			PRINTF("读取BPB扇区错误");
			break;
		case FATFS_FATFS_NOT_SUPPORT:
			PRINTF("不支持FATFS");
			break;
		case FATFS_FS_NOT_SUPPORT:
			PRINTF("FATFS_FS_NOT_SUPPORT");
			break;
		case FATFS_OPERATION_MODE_ERROR:
			PRINTF("模式选择错误");
			break;
		case FATFS_READ_FSI_SEC_ERROR:
			PRINTF("FATFS_READ_FSI_SEC_ERROR");
			break;
		case FATFS_WRITE_FSI_SEC_ERROR:
			PRINTF("FATFS_WRITE_FSI_SEC_ERROR");
			break;
		case FATFS_READ_DIR_ERROR:
			PRINTF("FATFS_READ_DIR_ERROR");
			break;
		case FATFS_FAT_REQUIRE_INITIAL:
			PRINTF("FATFS_FAT_REQUIRE_INITIAL");
			break;
		case FATFS_FILENAME_ERROR:
			PRINTF("FATFS_FILENAME_ERROR");
			break;
		case FATFS_FILE_NOT_FOUND:
			PRINTF("FATFS_FILE_NOT_FOUND");
			break;
		case FATFS_DIRECTORY_NOT_FOUND:
			PRINTF("FATFS_DIRECTORY_NOT_FOUND");
			break;
		case FATFS_END_OF_FATDIR:
			PRINTF("FATFS_END_OF_FATDIR");
			break;
		case FATFS_CACHE_NOMATCH:
			PRINTF("FATFS_CACHE_NOMATCH");
			break;
		case FATFS_READ_FAT_ERROR:
			PRINTF("FATFS_READ_FAT_ERROR");
			break;
		case FATFS_WRITE_FAT_ERROR:
			PRINTF("FATFS_WRITE_FAT_ERROR");
			break;
		case FATFS_UPDATE_FSI_ERROR:
			PRINTF("FATFS_UPDATE_FSI_ERROR");
			break;
		case FATFS_READ_SEC_ERROR:
			PRINTF("FATFS_READ_SEC_ERROR");
			break;
		case FATFS_WRITE_SEC_ERROR:
			PRINTF("FATFS_WRITE_SEC_ERROR");
			break;
		case FATFS_CLUS_SEC_ERROR:
			PRINTF("FATFS_CLUS_SEC_ERROR");
			break;
		case FATFS_RETAINED_CLUSTER:
			PRINTF("FATFS_RETAINED_CLUSTER");
			break;
		case FATFS_BAD_CLUSTER:
			PRINTF("FATFS_BAD_CLUSTER");
			break;
		case FATFS_READ_BLOCK_OVERRANG:
			PRINTF("FATFS_READ_BLOCK_OVERRANG");
			break;
		case FATFS_WRITE_BLOCK_OVERRANG:
			PRINTF("FATFS_WRITE_BLOCK_OVERRANG");
			break;
		case FATFS_END_OF_FILE:
			PRINTF("FATFS_END_OF_FILE");
			break;
		case FATFS_REQUIRE_EMPTYCLUS_ERROR:
			PRINTF("FATFS_REQUIRE_EMPTYCLUS_ERROR");
			break;
		case FATFS_LAST_CLUSTER:
			PRINTF("FATFS_LAST_CLUSTER");
			break;
		case FATFS_EMPTYCLUS_NOT_FOUND:
			PRINTF("FATFS_EMPTYCLUS_NOT_FOUND");
			break;
		case FATFS_CACHE_NOEMPTYDIR:
			PRINTF("FATFS_CACHE_NOEMPTYDIR");
			break;
		case FATFS_FOUND_EMPTYCLUS_ERROR:
			PRINTF("FATFS_FOUND_EMPTYCLUS_ERROR");
			break;
		case FATFS_NAME_EXT_TOO_LONG:
			PRINTF("后缀太长");
			break;
		case FATFS_NAME_NAME_TOO_LONG:
			PRINTF("命名太长");
			break;
		case FATFS_OPEN_FILE_FAILED:
			PRINTF("打开文件错误");
			break;
	}
}
#endif