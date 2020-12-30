/*************************************************************************
 文件： FAT.h
 描述：	FAT32/16文件系统写入读出，未完成写入部分。例程见最下注释
 更改： 2016 by nzy
 *************************************************************************/
#ifndef __FAT_H__
#define __FAT_H__
#include "LCD_12864.h"

//#define _DEBUG_    //调试请去掉注释

#define BYTE	unsigned char
#define WORD	unsigned int
#define LPBYTE	unsigned char *
#define LPWORD	unsigned int *
#define DWORD   unsigned long
#define LPDWORD unsigned long *
#define BOOL	bit
#define TRUE	1
#define FALSE	0
#define NULL    0
#define DWORDLE(a,b,c,d) (((DWORD)a)|((DWORD)b)<<8|((DWORD)c)<<16|((DWORD)d)<<24)
#define WORDLE(a,b)		 (((WORD)a)|((WORD)b)<<8)
#define BYTELE(a)		 ((BYTE)a)
#define LE(a,b)			 ((BYTE)((a)>>(8*b)))
#ifdef _DEBUG_
#define RETURN(a); FATFSDebugErrLvlMsg(a);return a;
#else
#define RETURN(a); return a;
#endif

//define _FATFSWRITE_

#ifndef __FATFs_ERROR_LEVEL__
#define __FATFs_ERROR_LEVEL__
#define FATFS_OPERATION_SUCCESS			0x00
#define FATFS_DEVICE_RESET_ERROR		0x01
#define FATFS_DEVICE_INITIAL_ERROR		0x02
#define FATFS_READ_BPB_SEC_ERROR		0x03
#define FATFS_FATFS_NOT_SUPPORT			0x04
#define FATFS_FS_NOT_SUPPORT			0x05
#define FATFS_OPERATION_MODE_ERROR		0x06
#define FATFS_READ_FSI_SEC_ERROR		0x07 
#define FATFS_WRITE_FSI_SEC_ERROR		0x08
#define FATFS_READ_DIR_ERROR			0x09
#define FATFS_GETNXTCLUS_ERROR			0x0A 
#define FATFS_FAT_REQUIRE_INITIAL		0x0F
#define FATFS_FILENAME_ERROR			0x10
#define FATFS_FILE_NOT_FOUND			0x11
#define FATFS_DIRECTORY_NOT_FOUND		0x12
#define FATFS_END_OF_FATDIR				0x13
#define FATFS_CACHE_NOMATCH				0x14
#define FATFS_READ_FAT_ERROR			0x20
#define FATFS_WRITE_FAT_ERROR			0x25  
#define FATFS_UPDATE_FSI_ERROR			0x28  
#define FATFS_READ_SEC_ERROR			0x21
#define FATFS_WRITE_SEC_ERROR			0x26  
#define FATFS_CLUS_SEC_ERROR			0x22
#define FATFS_RETAINED_CLUSTER			0x23
#define FATFS_BAD_CLUSTER				0x24
#define FATFS_READ_BLOCK_OVERRANG		0x2A
#define FATFS_WRITE_BLOCK_OVERRANG		0x29
#define FATFS_END_OF_FILE				0x2E
#define FATFS_REQUIRE_EMPTYCLUS_ERROR	0x27  
#define FATFS_LAST_CLUSTER				0x2F
#define FATFS_EMPTYCLUS_NOT_FOUND		0x31
#define FATFS_CACHE_NOEMPTYDIR			0x32
#define FATFS_FOUND_EMPTYCLUS_ERROR		0x33
#define FATFS_NAME_EXT_TOO_LONG			0xE1
#define FATFS_NAME_NAME_TOO_LONG		0xE2
#define FATFS_OPEN_FILE_FAILED			0xFF
#endif

#ifndef __DIR_ATTR__
#define __DIR_ATTR__
#define ATTR_FILE						0x00
#define ATTR_READ_ONLY					0x01
#define ATTR_HIDDEN						0x02
#define ATTR_SYSTEM						0x04
#define ATTR_VOLUME_ID					0x08
#define ATTR_DIRECTORY					0x10
#define ATTR_ARCHIVE					0x20
#define ATTR_LONG_NAME					(ATTR_READ_ONLY|ATTR_HIDDEN|ATTR_SYSTEM|ATTR_VOLUME_ID)
#define ATTR_LONG_NAME_MASK				(ATTR_READ_ONLY|ATTR_HIDDEN|ATTR_SYSTEM|ATTR_VOLUME_ID|ATTR_DIRECTORY|ATTR_ARCHIVE)
#define ATTR_NOTHING					0xFF
#endif

#ifndef __FATFS_OPERATION_MODE__
#define __FATFS_OPERATION_MODE__
#define FATFS_MODE_READ					0x01
#define FATFS_MODE_WRITE				0x02
#define FATFS_MODE_ADD					0x04
#define FATFS_MODE_DELETE				0x08
#endif



BYTE FATFSShortName(BYTE NameLen);
BYTE FATFSInitial(LPBYTE BPB_POINT);
BYTE FATFSReadNextBlock(LPBYTE FileCache, LPWORD BlockLen);
BYTE FATFSMatchDir(LPBYTE DirCache, BOOL IsFile);
BYTE FATFSDirOperation(LPBYTE DataCache, BOOL IsFile);
BYTE FATFSOpen(LPBYTE DataCache, LPBYTE szPath);

#ifdef _FATFSWRITE_
BYTE FATFSFindEmptyClus(LPBYTE DataCache, LPDWORD ClusAddress);
BYTE FATFSWriteNextBlock(LPBYTE FileCache, WORD BlockLen);
BYTE FATFSCreateDir(LPBYTE DirCache, BOOL IsFile);
#endif

#ifdef _DEBUG_
void FATFSDebugErrLvlMsg(BYTE ErrLvl);
#endif

extern DWORD data g_dwFileFirstClus;
extern DWORD data g_dwFileSize;
extern DWORD data g_dwTotalClus;
extern BYTE  data g_bFileAttrib;
extern WORD  data g_wRsvdSecCnt;
extern BYTE  data g_bSecPerClus;
extern BYTE  data g_bSecPerClusBin;
extern DWORD data g_dwFirstRootDirNum;
extern DWORD data g_dwFirstDataSector;
extern BYTE  data g_bNextBlock;
extern DWORD data g_dwFATSz;

#endif

