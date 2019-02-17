#include "FAT.h"
#include "SD.h"

#define PRINTF			 Lcd12864PrintfOne		 //��ӡ		�������ַ���ָ��

#define FATFSDEVICERESET	!sd_reset			 //�豸��λ	�޲�	���أ�BOOL �ɹ���TRUE
#define FATFSDEVICEINITIAL	!sd_init		 	 //�豸��ʼ��	�޲�	���أ�BOOL �ɹ���TRUE
#define FATFSDEVICEREADBLOCK	!sd_read_sector			 //�豸����	������DWORD������ַ����������(512B)	���أ�BOOL �ɹ���TRUE
#define FATFSDEVICEWRITEBLOCK	!sd_write_sector		 //�豸д��	������DWORD������ַ����������(512B)	���أ�BOOL �ɹ���TRUE

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

#define SHORTNAMELENCACHELEN 13		 	 //�����ռ��С
#define FAT16MAXPERCHECKCLUS 4			 //FAT16���Ԥ�����
#define FAT32MAXPERCHECKCLUS 4			 //FAT32���Ԥ�����
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
#ifndef g_wBytePerSec			  		//��ʱֻ֧��512byte/�����Ĵ������
WORD  data g_wBytePerSec;
#endif

union FATFSStringDataUnion g_unTemp;						  
#define g_szFileShortName g_unTemp.String		//�����ռ临��
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
WORD  data g_wFSInfoNxtFree;    //FAT32���� FSInfoָ�� FAT16����������������
#endif

/*************************************************************************
������AnalysisMBR
����������MBR���ҳ�BPB�������ַ
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
 ������FATFSShortName
 ������8.3�ļ���ת��	
*************************************************************************/
BYTE FATFSShortName(BYTE NameLen){
	BYTE ExtLen;
	BYTE Cache[3]={0x20,0x20,0x20}; //ASCII��ֵ����ո�
	for(NameLen=0;g_szFileShortName[NameLen]!='.'&&g_szFileShortName[NameLen]!='\0';++NameLen); //ͣ��'.'��λ��
	if(g_szFileShortName[NameLen]!='\0'){ 
		++NameLen;
		for(ExtLen=0;g_szFileShortName[NameLen + ExtLen]!='\0';++ExtLen){
				Cache[ExtLen] = g_szFileShortName[NameLen + ExtLen];
		}
		--NameLen;//�ص�'.'��λ��
	}
	if(NameLen<9){
		for(;NameLen<11;++NameLen){
			g_szFileShortName[NameLen] = ' ';
		}
		NameLen = 8;
		for(;NameLen<11;++NameLen){
			g_szFileShortName[NameLen] = Cache[NameLen-8]; //�����׺��
		}
		g_szFileShortName[11] = '\0';  //�ٴβ����ַ���������־
		RETURN(FATFS_OPERATION_SUCCESS);
	}else{
		RETURN(FATFS_NAME_NAME_TOO_LONG);
	}
}

/*************************************************************************
 ������	InitialFATFS
 ������	��ʼ��FATFS����ø�FAT��Ϣ
*************************************************************************/
BYTE FATFSInitial(LPBYTE BPB_POINT){								 //g_dwTotalClus ����ʱ����
	if(FATFSDEVICERESET()){
		if(FATFSDEVICEINITIAL()){
		   	if(FATFSDEVICEREADBLOCK(AnalysisMBR(BPB_POINT), BPB_POINT)){ 
			//if(FATFSDEVICEREADBLOCK(8192, BPB_POINT)){				//���0�����ϵ�BPBֻ֧��һ�������˲�ȻҪ����MBR
				if(BPB_POINT[510]!=0x55 && BPB_POINT[510]!=0xAA){		//AA55��55AA�����ԣ�AA55һ��ΪӲ������������ĩβ
					g_IsFATFS = FALSE;
					RETURN(FATFS_FS_NOT_SUPPORT);
				}
				g_dHddsec = BPB_HIDDSEC; //����������
				g_wRsvdSecCnt = BPB_RSVDSECCNT; //����������
				g_dwFATSz = BPB_FATSZ32; //FAT����ռ������
				g_dwFirstDataSector = g_wRsvdSecCnt + BPB_NUMFATS * g_dwFATSz + g_dwTotalClus + g_dHddsec; //��������һ���������ǵü�������������
				g_dwTotalClus = BPB_TOTSEC32 - g_dwFirstDataSector + g_dHddsec; //��BPB�л�õ�����������������������
				g_bSecPerClus = BPB_SECPERCLUS;	//ÿ��������
				g_dwTotalClus = (g_dwTotalClus + (g_bSecPerClus/2)) / g_bSecPerClus; //���������� = ���������� / ÿ��������
				if(g_dwTotalClus > 65525UL){
					g_dwFirstRootDirNum = BPB_ROOTCLUS;	//FAT32���У���Ŀ¼���ڵ�һ���شغţ�ͨ��Ϊ2
					g_IsFAT16 = FALSE;
				}else{
					g_IsFATFS = FALSE;
					RETURN(FATFS_FATFS_NOT_SUPPORT);
				}
#ifndef g_wBytePerSec    
				g_wBytePerSec = BPB_BYTEPERSEC;
#else			//��ʱֻ֧��512byte/�����Ĵ������
				if(g_wBytePerSec != BPB_BYTEPERSEC){
					RETURN(FATFS_FATFS_NOT_SUPPORT);
				}
#endif				
				g_bSecPerClusBin = 0; //ȡ������
				while(g_bSecPerClus > 1){
					 ++g_bSecPerClusBin;
					 g_bSecPerClus >>= 1;  //ÿ��������/2
				}
				g_dwFileFirstClus = g_dwFirstRootDirNum; //�ļ���ʼ�� = ��Ŀ¼�����״� / ������
				g_bSecPerClus = BPB_SECPERCLUS;	//ֵ�ѱ����ģ��������¸�ֵ
				g_bNextBlock = 0; //��һ��Ҫ��������
				g_IsFATFS = TRUE; //�ж�ΪFAT32ϵͳ
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
 ������	FATFSReadNextBlock
 ������	��˳����������ϵ�����
*************************************************************************/
BYTE FATFSReadNextBlock(LPBYTE FileCache, LPWORD BlockLen){
	if(g_bNextBlock>=g_bSecPerClus){ //�Ѿ�����������>=ÿ��������
		//������һ����
			//FAT32
			if(g_unTemp.Data.bytea == 0){	//���������Ĵ���
				g_dwFileCurrentClus.Data<<=2;  //��FAT32���ԣ�ÿ����ռ��4���ֽ�
				g_unTemp.Data.dworda.Data = (g_dwFileCurrentClus.Data / g_wBytePerSec) + g_wRsvdSecCnt + g_dHddsec;//���ļ����ڵ�FAT��������������������
				if(!FATFSDEVICEREADBLOCK(g_unTemp.Data.dworda.Data, FileCache)){ //����Ӧ��FAT�������ж���
					*BlockLen = 0; //����Ϊ0
					RETURN(FATFS_READ_FAT_ERROR); //���ض�ȡFAT��ʧ��
				}
				g_unTemp.Data.worda = g_dwFileCurrentClus.Data%g_wBytePerSec; //�ļ�����FAT��ı�����
				g_dwFileCurrentClus.byte.a = FileCache[g_unTemp.Data.worda+3];	//��ȡ�ļ���һ�شغ�	C51:���
				g_dwFileCurrentClus.byte.b = FileCache[g_unTemp.Data.worda+2];	//��ȡ�ļ���һ�شغ�	X86:С��
				g_dwFileCurrentClus.byte.c = FileCache[g_unTemp.Data.worda+1];  //��ȡ�ļ���һ�شغ�
				g_dwFileCurrentClus.byte.d = FileCache[g_unTemp.Data.worda];	//��ȡ�ļ���һ�شغ�
				g_unTemp.Data.dwordb.Data = g_dwFileCurrentClus.Data;  //�õ��ļ���һ�شغ�
				while(g_unTemp.Data.bytea < FAT32MAXPERCHECKCLUS){	   //FATFS���Ԥ�����
					g_dwFileCurrentSec.Data = g_unTemp.Data.dwordb.Data << 2;  //�ļ���һ�غ� X ÿ�غ�ռ��4�ֽ�
					if(g_unTemp.Data.dworda.Data == ((g_dwFileCurrentSec.Data / g_wBytePerSec) + g_wRsvdSecCnt)){  //����ļ�����һ�ص�FAT����������ԭ���������� 
						g_unTemp.Data.worda = g_dwFileCurrentSec.Data % g_wBytePerSec; //��ȡ������
						g_dwFileCurrentSec.byte.a = FileCache[g_unTemp.Data.worda+3];  //��ȡ�ļ�����һ�شغ�
						g_dwFileCurrentSec.byte.b = FileCache[g_unTemp.Data.worda+2];  //��ȡ�ļ�����һ�شغ�
						g_dwFileCurrentSec.byte.c = FileCache[g_unTemp.Data.worda+1];  //��ȡ�ļ�����һ�شغ�
						g_dwFileCurrentSec.byte.d = FileCache[g_unTemp.Data.worda];	   //��ȡ�ļ�����һ�شغ�
						if(g_dwFileCurrentSec.Data == g_unTemp.Data.dwordb.Data + 1){ //�������һ�ظպý�����һ��
							g_unTemp.Data.bytea++;
							g_unTemp.Data.dwordb.Data = g_dwFileCurrentSec.Data;  //��һ�ض���Ϊ����һ��
						}else{
							break;
						}
					}else{
						break;
					}
				}
				if((g_dwFileCurrentClus.Data&=0x0FFFFFFFUL) > 0x0FFFFFEFUL){ //��ʼ�أ������ϣ��������ص��״�	����ע1������ĩβ
					*BlockLen = 0;					  //������ ���� �ļ���
					if(g_dwFileCurrentClus.Data < 0x0FFFFFF6UL){
						RETURN(FATFS_RETAINED_CLUSTER); //������
					}else if(g_dwFileCurrentClus.Data == 0x0FFFFFF7UL){
						RETURN(FATFS_BAD_CLUSTER); //����
					}else{
						RETURN(FATFS_LAST_CLUSTER); //���һ��
					}
				}
			}else{ //�������Ĵ���
				g_dwFileCurrentClus.Data++; //�ļ���ʼ���仯���غ�   ������һ��
				g_unTemp.Data.bytea--; //����������1
			}	
		g_dwFileCurrentSec.Data = ((g_dwFileCurrentClus.Data - 2)<<g_bSecPerClusBin) + g_dwFirstDataSector;	//�����ļ���ʼ������
		g_bNextBlock = 0; //�Ѿ����������� = 0
	}

	//��ǰ��
	if(g_bNextBlock<g_bSecPerClus){	//���Ҫ���������� < ÿ��������
		if(BlockLen){ 	 //�Գ������ԣ���ַΪ0
			if(g_dwFileSize){ //�ļ���С��Ϊ0
				if(g_dwFileSize >= g_wBytePerSec){  //�ļ���С >= ÿ�������ֽ���
					*BlockLen = g_wBytePerSec;		//ʵ�ʶ������� = ÿ�������ֽ���
					g_dwFileSize -= g_wBytePerSec;	//�ļ���С����һ������
				}else{
					*BlockLen = g_dwFileSize;  //ʵ�ʶ������� = �ļ���С
					g_dwFileSize = 0; //�ļ���С����Ϊ0
				}
			}else{
				*BlockLen = 0; //ʵ�ʶ����ĳ���Ϊ0
				RETURN(FATFS_END_OF_FILE);	//�����ļ�����
			}
		}
		if(!FATFSDEVICEREADBLOCK(g_dwFileCurrentSec.Data, FileCache)){  //������ļ���ʼ���仯�����鲻�ɹ�
			*BlockLen = 0;	 //ʵ�ʳ���Ϊ0
			RETURN(FATFS_READ_SEC_ERROR); //���ض�����ʧ��
		}
		++g_dwFileCurrentSec.Data;	//��1Ϊ�ļ���һ������
		++g_bNextBlock;	//�Ѿ�����������1
		RETURN(FATFS_OPERATION_SUCCESS);  //���ز����ɹ�
	}else{
		RETURN(FATFS_READ_BLOCK_OVERRANG);	//�Ѿ���ɶ���
	}		
}

/*************************************************************************
 ������	FATFSMatchAttrib
 ������	ȷ��Ŀ¼������
*************************************************************************/
BYTE FATFSValidateAttrib(BYTE Attr){
	if((Attr&ATTR_LONG_NAME_MASK)==ATTR_LONG_NAME){
		return ATTR_LONG_NAME;
	}else{
		if((Attr&(ATTR_DIRECTORY|ATTR_VOLUME_ID))==0x00){
			//�ļ�
			return ATTR_FILE;
		}else if((Attr&(ATTR_DIRECTORY|ATTR_VOLUME_ID))==ATTR_DIRECTORY){
			//Ŀ¼
			return ATTR_DIRECTORY;
		}else if((Attr&(ATTR_DIRECTORY|ATTR_VOLUME_ID))==ATTR_VOLUME_ID){
			//���
			return ATTR_VOLUME_ID;
		}else{
			//ʲô������[���Ҿ�ʶ���⼸���ļ�����]
			return ATTR_NOTHING;
		}
	}
}

/*************************************************************************
 ������	FATFSMatchDir 
 ������	ƥ��FATĿ¼���е����Ʋ���ȡ�й�����
*************************************************************************/
BYTE FATFSMatchDir(LPBYTE DirCache, BOOL IsFile){
	WORD j;
	BYTE k;
	for(j=0;j<g_wBytePerSec;j+=32){	//ÿ�������ֽ�����ÿ��Ŀ¼��Ĵ�СΪ32�ֽ�
		if(DirCache[j] == 0xE5){	//˵����Ŀ¼�ʹ�ù������ѱ�ɾ��
			continue;	//��������
		}
		if(DirCache[j] == 0x00){	//˵����Ŀ¼���δ��ʹ�ù�������Ŀ¼��ķ����ǰ�˳�򣬹�˵�����⿪ʼ���涼Ϊ0X00
			RETURN(FATFS_END_OF_FATDIR);
		}
		for(k=0;k<11;++k){
			if(DirCache[j+k] == g_szFileShortName[k]){  //����Ŀ¼���ǰ11��ASCII����������жԱ�
				continue;  //�����Ƚ������ַ�
			}else{
				break; //����
			}
		}
		if(k==11){
			g_bFileAttrib = BYTELE(DirCache[j+11]);	 //�ļ�����	 0x01-�ļ� 0x01-ֻ�� 0x02-���� 0x04-ϵͳ�ļ� 0x08-��� 0x0f-��ʾ��Ŀ¼��Ϊ���ļ���Ŀ¼�� 0x10-Ŀ¼ 0x20-�浵													
			if(IsFile){	//�ļ�							 													
				if(FATFSValidateAttrib(g_bFileAttrib)!=ATTR_FILE){ //�鿴�ǲ����ļ�
					RETURN(FATFS_FILE_NOT_FOUND);
				}
			}else{
				if(FATFSValidateAttrib(g_bFileAttrib)!=ATTR_DIRECTORY){//�鿴�ǲ���Ŀ¼
					RETURN(FATFS_DIRECTORY_NOT_FOUND);
				}
			}
			g_dwFileFirstClus = DWORDLE(DirCache[j+26], DirCache[j+27], DirCache[j+20], DirCache[j+21]); //�ļ���ʼ��
			g_dwFileSize = DWORDLE(DirCache[j+28], DirCache[j+29], DirCache[j+30], DirCache[j+31]);	//�ļ����ݴ�С�ֽ���
			RETURN(FATFS_OPERATION_SUCCESS);
		}
	}
	RETURN(FATFS_CACHE_NOMATCH);  
}


/*************************************************************************
 ������	FATFSOpenDir
 ������	�򿪵�ǰĿ¼ ��ƥ���Ŀ¼���ļ�
*************************************************************************/
BYTE FATFSDirOperation(LPBYTE DataCache, BOOL IsFile){
	BYTE k;
	do{	//ִֻ��һ�εġ�ѭ���� ���Ż�����
		if(g_dwFileFirstClus == g_dwFirstRootDirNum){ //����Ǹ�Ŀ¼
				g_dwFileCurrentClus.Data = g_dwFirstRootDirNum;	//FAT32��Ŀ¼����ļ��򿪣���ֵ��Ŀ¼���ڵĴغ�
		}else{ //���Ǹ�Ŀ¼
			g_dwFileCurrentClus.Data = 	g_dwFileFirstClus; 
		}
		g_dwFileCurrentSec.Data = ((g_dwFileCurrentClus.Data - 2)<<g_bSecPerClusBin) + g_dwFirstDataSector;	 //�ļ���ʼ�����ţ���2����ΪFAT32�Ǵ�2��ʼ�ģ�
		g_bNextBlock = 0; //Ҫ����������Ϊ0
		while(FATFSReadNextBlock(DataCache,0)==FATFS_OPERATION_SUCCESS){  //��˳����������ϵ���������һ��
			k = FATFSMatchDir(DataCache, IsFile); //��������������������бȽϣ�����ƥ�����
			if(k == FATFS_CACHE_NOMATCH){  //û���ҵ�
				continue; //�ٴ�ѭ��
			}else{
				break; //����ѭ��
			}
		}
	}while(0);
	if(k != FATFS_OPERATION_SUCCESS){ //���û��ƥ��ɹ�
		if(IsFile){	//���ļ�
			RETURN(FATFS_FILE_NOT_FOUND);
		}else{ //���ļ��У�Ŀ¼��
			RETURN(FATFS_DIRECTORY_NOT_FOUND);
		}
	}
	g_dwFileCurrentClus.Data = g_dwFileFirstClus; //�ļ����ݿ�ʼ��
	g_dwFileCurrentSec.Data = ((g_dwFileCurrentClus.Data - 2)<<g_bSecPerClusBin) + g_dwFirstDataSector;	//�ļ���ʼ������
	g_bNextBlock = 0; //Ҫ����������
	g_unTemp.Data.bytea = 0; //��������������Ϊ0
	RETURN(FATFS_OPERATION_SUCCESS);
}

/*************************************************************************
 ������	FATFSOpen
 ������	��ָ��·����FAT�ļ�
*************************************************************************/
BYTE FATFSOpen(LPBYTE DataCache, LPBYTE szPath){
//e.g.	"\ABC\DEF\BAD.BIN" "ABC\DEF\BAD.BIN" "\BAD.BIN" "BAD.BIN"
	BYTE i;
	if(g_IsFATFS){
		i = 0;
		if(szPath[0] == '\\'){	//�鿴Ŀ��·�������ַ��Ƿ���'\'
			szPath += 1; //����'\'
		}
		while(1){
			szPath = szPath + i;
			for(i=0;szPath[i]!='\0'&&szPath[i]!='\\'&&i<SHORTNAMELENCACHELEN-1;++i){ //������ת��������g_szFileShortName
				g_szFileShortName[i] = szPath[i];
			}
			if(i==SHORTNAMELENCACHELEN-1){
				RETURN(FATFS_FILENAME_ERROR);
			}else{
				g_szFileShortName[i] = '\0';  //�����ַ���������־
				if(FATFSShortName(i) != FATFS_OPERATION_SUCCESS){ //��8.3�ļ�������ת��				 
					break;
				}
				if(szPath[i]=='\0'){ //�Ƿ��ļ�ĩβ
					if(FATFSDirOperation(DataCache, TRUE) == FATFS_OPERATION_SUCCESS){  //��Ŀ¼����ƥ���ļ�
						RETURN(FATFS_OPERATION_SUCCESS);
					}else{
						break;
					}
				}else{ //�ļ���ĩβ
					++i;	//����"\"
					if(FATFSDirOperation(DataCache, FALSE) == FATFS_OPERATION_SUCCESS){ //��Ŀ¼����ƥ���ļ���
						continue;  //������һ�ε��ļ�/�ļ���������
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
 ������	FATFSDebugErrLvlMsg
 ������	���FAT_ERROR_LEVEL���嵽��Ļ ����_DEBUG_��Ч RETURN ���Զ�����
*************************************************************************/
#ifdef _DEBUG_
void FATFSDebugErrLvlMsg(BYTE ErrLvl){
	switch(ErrLvl){
		case FATFS_OPERATION_SUCCESS:
			PRINTF("�����ɹ�");
			break;
		case FATFS_DEVICE_RESET_ERROR:
			PRINTF("SD��λ����");
			break;
		case FATFS_DEVICE_INITIAL_ERROR:
			PRINTF("��ʼ���ɹ�");
			break;
		case FATFS_READ_BPB_SEC_ERROR:
			PRINTF("��ȡBPB��������");
			break;
		case FATFS_FATFS_NOT_SUPPORT:
			PRINTF("��֧��FATFS");
			break;
		case FATFS_FS_NOT_SUPPORT:
			PRINTF("FATFS_FS_NOT_SUPPORT");
			break;
		case FATFS_OPERATION_MODE_ERROR:
			PRINTF("ģʽѡ�����");
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
			PRINTF("��׺̫��");
			break;
		case FATFS_NAME_NAME_TOO_LONG:
			PRINTF("����̫��");
			break;
		case FATFS_OPEN_FILE_FAILED:
			PRINTF("���ļ�����");
			break;
	}
}
#endif