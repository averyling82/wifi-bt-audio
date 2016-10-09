/*
********************************************************************************************
*
*                  Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                               All rights reserved.
*
* FileName: System\VirtualFile\VirtualFile.h
* Owner: Aaron.sun
* Date: 2014.6.11
* Time: 16:55:46
* Desc: Virtual File For Simulator
* History:
*    <author>     <date>       <time>     <version>       <Desc>
*    Aaron.sun     2014.6.11     16:55:46   1.0
********************************************************************************************
*/

#ifndef __SYSTEM_VIRTUALFILE_VIRTUALFILE_H__
#define __SYSTEM_VIRTUALFILE_VIRTUALFILE_H__

//#define APE_TEST
//#define FLAC_TEST
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
//#include <stdio.h>


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable declare
*
*---------------------------------------------------------------------------------------------------------------------
*/


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API Declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
extern size_t fwrite(const void * buf, size_t nb_Byte, size_t Block, FILE * File);
extern size_t fread(void * buf, size_t nb_Byte, size_t Block, FILE * File);
extern int fseek(FILE * file, long offset, int pos);
extern FILE * fopen(const char * FileName, const char * mode);
extern unsigned char data[1220000] ;


#endif

