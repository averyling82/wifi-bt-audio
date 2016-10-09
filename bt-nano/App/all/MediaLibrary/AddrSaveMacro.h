/*
********************************************************************************
*
*     Copyright (c):Fuzhou Rockchip Electronics Co., Ltd
*                         All rights reserved.
*
* File Name  AddrSaveMacro.h
*
* Description:
*
* History:      <author>          <time>        <version>
*               anzhiguo         2008-9-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef ADDR_SAVE_MACRO_H
#define ADDR_SAVE_MACRO_H

#define  SORT_FILENUM_DEFINE        (2048)//2048      /* 限定文件最大数,文件信息保存占用4个block，每个block拥有(SecPerBlock[0] / SecPerPage[0])个page ，一个page可以保存4个文件信息 */
#define  MEDIA_ID3_SAVE_CHAR_NUM    (SYS_SUPPROT_STRING_MAX_LEN + 1)   //256 /* 保存的ID3_TITLE字符数 */

//每个媒体文件信息大小
#define  BYTE_NUM_SAVE_PER_FILE     (MEDIA_ID3_SAVE_CHAR_NUM * 2 * 8) //4096 /*occupy byte number each file information*/


//system reserved area(sector) size.
#define     MEDIAINFO_PAGE_SIZE         (4*1024) //page 大小
#define     MEDIAINFO_BLOCK_SIZE        4096  // sector count
#define     SECTOR_BYTE_SIZE            512     /*byte number in one sector */

#define     FILE_SAVE_NUM_PER_PAGE      (MEDIAINFO_PAGE_SIZE / BYTE_NUM_SAVE_PER_FILE)// 1 /*save file information,the number that one page can save in each page*/

/*
--------------------------------------------------------------------------------
 the memory allocate of one secector to save file deteiled information;
    offset address          save information        occupy size(unit : byte)
        0                   long file namge         SYS_SUPPROT_STRING_MAX_LEN*2 (80)
        80                  ID3_TITLE               CHAR_NUM_PER_ID3_TITLE*2  (60)
     80+60=140              ID3_SINGLE              CHAR_NUM_PER_ID3_SINGLE*2 (60)
    80+60+60=200            ID3_ALBUM               CHAR_NUM_PER_ID3_ALBUM*2  (60)
   80+60+60+60=260          file path info          (3 + (MAX_DIR_DEPTH - 1) * 12 + 1)(40)
  80+60+60+60+40=300        short file name           11

                                                    total : 311 byte
History:     <author>         <time>         <version>
            anzhiguo     2009-6-4         Ver1.0
--------------------------------------------------------------------------------
*/
#define   CUE_START_SAVE_ADDR_OFFSET 0
#define   CUE_START_SAVE_SIZE       4

#define   CUE_END_SAVE_ADDR_OFFSET (CUE_START_SAVE_ADDR_OFFSET + CUE_START_SAVE_SIZE)
#define   CUE_END_SAVE_SIZE         4

#define   TRACKID_SAVE_ADDR_OFFSET   (CUE_END_SAVE_ADDR_OFFSET + CUE_END_SAVE_SIZE) /*keeping file number offset in file information section*/
#define   TRACKID_SAVE_SIZE         8

#define   ATTR_SAVE_ADDR_OFFSET     (TRACKID_SAVE_ADDR_OFFSET  + TRACKID_SAVE_SIZE)
#define   ATTR_SAVE_SIZE            8

////////////////////////////////////////////////////////////////////////////////////////////

#define   MTP_RESERVE1_SAVE_ADDR_OFFSET  (MEDIA_ID3_SAVE_CHAR_NUM*2)
#define   MTP_RESERVE1_SIZE              (MEDIA_ID3_SAVE_CHAR_NUM*2)

///////////////////////////////////////////////////////////////////////////////////////////////

#define   FILE_NAME_SAVE_ADDR_OFFSET    (MTP_RESERVE1_SAVE_ADDR_OFFSET + MTP_RESERVE1_SIZE) /*In each file info keeping space, file name offset is the start postion in each keeping file infomation section.*/
#define   FILE_NAME_SAVE_SIZE           (MEDIA_ID3_SAVE_CHAR_NUM*2)   /*occupy byte number to save one file name */

#define   DIR_PATH_SAVE_ADDR_OFFSET     (FILE_NAME_SAVE_ADDR_OFFSET+FILE_NAME_SAVE_SIZE) /*keeping dir path offset in file information section */
#define   DIR_PATH_SAVE_SIZE            (MEDIA_ID3_SAVE_CHAR_NUM*2)            /*the byte number of one save dir path */

#define   ID3_TITLE_SAVE_ADDR_OFFSET    (DIR_PATH_SAVE_ADDR_OFFSET+DIR_PATH_SAVE_SIZE) /*keeping ID3_TITLE offset in file information section */
#define   ID3_TITLE_SAVE_SIZE           (MEDIA_ID3_SAVE_CHAR_NUM*2)            /*the byte number of one save ID3_TITLE */


#define   ID3_SINGLE_SAVE_ADDR_OFFSET   (ID3_TITLE_SAVE_ADDR_OFFSET+ID3_TITLE_SAVE_SIZE) /*keeping ID3_SINGLE offset in file information section */
#define   ID3_SINGLE_SAVE_SIZE          (MEDIA_ID3_SAVE_CHAR_NUM*2)          /*the byte number of one save ID3_SINGLE*/


#define   ID3_ALBUM_SAVE_ADDR_OFFSET    (ID3_SINGLE_SAVE_ADDR_OFFSET+ID3_SINGLE_SAVE_SIZE) /*keeping ID3_ALBUM offset in file information section */
#define   ID3_ALBUM_SAVE_SIZE           (MEDIA_ID3_SAVE_CHAR_NUM*2)            /*the byte number of one save ID3_ALBUM*/


#define   ID3_GENRE_SAVE_ADDR_OFFSET    (ID3_ALBUM_SAVE_ADDR_OFFSET+ID3_ALBUM_SAVE_SIZE) /*keeping ID3_GENRE offset in file information section*/
#define   ID3_GENRE_SAVE_SIZE           (MEDIA_ID3_SAVE_CHAR_NUM*2)            /*the byte number of one save ID3_GENRE*/


/*
*************************************************************************************************************
    Meida Lib Address map
   StartAddress: 0000   -----------------------------------
                        |                                 |
                        |   FILE_SAVE_INFO(2048*4)        |
                        |                                 |
   StartAddress: 2048*4 -----------------------------------
                        |                                 |
                        |   FILE_SORT_INFO(2048*4)        |
                        |                                 |
   StartAddress: 2048*8 -----------------------------------
                        |                                 |
                        |   FAVORITE_INFO(2048*4*2)       |
                        |                                 |
   StartAddress: 2048*16-----------------------------------
                        |                                 |
                        |   PLAYLIST_INFO(2048*4*2)       |
                        |                                 |
   StartAddress: 2048*32-----------------------------------

*************************************************************************************************************
*/
//------------------------------------------------------------------------
//File Save Info (Addr: 0)
#define     MUSIC_HEADER_INFO_SECTOR_START      0 /*基础表每一个信息块是否有效标志位*/
#define     MUSIC_HEADER_SECTOR_SIZE            (MEDIAINFO_PAGE_SIZE) /*16KB*/

#define     MUSIC_SAVE_INFO_SECTOR_START        (MUSIC_HEADER_INFO_SECTOR_START + MUSIC_HEADER_SECTOR_SIZE) /*the start address to save file detailed information */
#define     MUSIC_SAVE_SECTOR_SIZE              (MEDIAINFO_BLOCK_SIZE * 32) /*file information occupy memory, 4096k is 4M */

#define     RECORD_SAVE_INFO_SECTOR_START       (MUSIC_SAVE_INFO_SECTOR_START + MUSIC_SAVE_SECTOR_SIZE)
#define     RECORD_SAVE_SECTOR_SIZE             (MEDIAINFO_BLOCK_SIZE * 32)

#if(defined PIC_MEDIA) || (defined _M3U_)

#define     JPEG_SAVE_INFO_SECTOR_START         (RECORD_SAVE_INFO_SECTOR_START + RECORD_SAVE_SECTOR_SIZE)
#define     JPEG_SAVE_SECTOR_SIZE               (MEDIAINFO_BLOCK_SIZE * 32)

#define     M3U_SAVE_INFO_SECTOR_START          (JPEG_SAVE_INFO_SECTOR_START + JPEG_SAVE_SECTOR_SIZE)
#define     M3U_SAVE_SECTOR_SIZE                (MEDIAINFO_BLOCK_SIZE * 32)

#define     MUSIC_TREE_INFO_SECTOR_START        (M3U_SAVE_INFO_SECTOR_START + M3U_SAVE_SECTOR_SIZE)
#define     MUSIC_TREE_SECTOR_SIZE              (MEDIAINFO_BLOCK_SIZE * 4)

#define     RECORD_TREE_INFO_SECTOR_START       (MUSIC_TREE_INFO_SECTOR_START + MUSIC_TREE_SECTOR_SIZE)
#define     RECORD_TREE_SECTOR_SIZE             (MEDIAINFO_BLOCK_SIZE * 12)

#define     JPEG_TREE_INFO_SECTOR_START         (RECORD_TREE_INFO_SECTOR_START + RECORD_TREE_SECTOR_SIZE)
#define     JPEG_TREE_SECTOR_SIZE               (MEDIAINFO_BLOCK_SIZE * 12)

#define     M3U_TREE_INFO_SECTOR_START          (JPEG_TREE_INFO_SECTOR_START + JPEG_TREE_SECTOR_SIZE)
#define     M3U_TREE_SECTOR_SIZE                (MEDIAINFO_BLOCK_SIZE * 12)

#define     FAVORITE_MUSIC_INFO_SECTOR_START    (M3U_TREE_INFO_SECTOR_START +  M3U_TREE_SECTOR_SIZE)   /*start address of the keeping favorite infomation*/

#else

#define     MUSIC_TREE_INFO_SECTOR_START        (RECORD_SAVE_INFO_SECTOR_START + RECORD_SAVE_SECTOR_SIZE)
#define     MUSIC_TREE_SECTOR_SIZE              (MEDIAINFO_BLOCK_SIZE * 4)

#define     RECORD_TREE_INFO_SECTOR_START       (MUSIC_TREE_INFO_SECTOR_START + MUSIC_TREE_SECTOR_SIZE)
#define     RECORD_TREE_SECTOR_SIZE             (MEDIAINFO_BLOCK_SIZE * 12)

#define     M3U_SAVE_INFO_SECTOR_START          (RECORD_TREE_INFO_SECTOR_START + RECORD_TREE_SECTOR_SIZE)
#define     M3U_SAVE_SECTOR_SIZE                (MEDIAINFO_BLOCK_SIZE * 32)

#define     M3U_TREE_INFO_SECTOR_START          (M3U_SAVE_INFO_SECTOR_START + M3U_SAVE_SECTOR_SIZE)
#define     M3U_TREE_SECTOR_SIZE                (MEDIAINFO_BLOCK_SIZE * 12)

#define     FAVORITE_MUSIC_INFO_SECTOR_START    (M3U_TREE_INFO_SECTOR_START +  M3U_TREE_SECTOR_SIZE)   /*start address of the keeping favorite infomation*/

#endif

#define     FAVORITE_MUSIC_SECTOR_SIZE           MEDIAINFO_BLOCK_SIZE    /*the memory size of favorite 2000 sectoin is equal with 1M*/
#define     FAVORITE_BLOCK_SECTOR_START1        (FAVORITE_MUSIC_INFO_SECTOR_START)
#define     FAVORITE_BLOCK_SECTOR_START2        (FAVORITE_MUSIC_INFO_SECTOR_START + FAVORITE_MUSIC_SECTOR_SIZE)


//------------------------------------------------------------------------
//File Sort info (Addr: 2048*4)
//#define       FILENAME_SORT_INFO_SECTOR_START         (MUSIC_SAVE_INFO_SECTOR_START+MUSIC_SAVE_SECTOR_SIZE) /*the saved start address of file name sort information */
//#define     FILENAME_SORT_INFO_SIZE                   32  /*file name sort information occupy memory,16 sec that is 8k */

#define     ID3TITLE_SORT_INFO_SECTOR_START         (FAVORITE_BLOCK_SECTOR_START2 + FAVORITE_MUSIC_SECTOR_SIZE) /*the saved start address of file name sort information */
//#define       ID3TITLE_SORT_INFO_SECTOR_START         (FILENAME_SORT_INFO_SECTOR_START+FILENAME_SORT_INFO_SIZE) /*the start sector address of ID3_TITLE sort information*/
#define     ID3TITLE_SORT_INFO_SIZE                 32

#define     ID3ARTIST_SORT_INFO_SECTOR_START        (ID3TITLE_SORT_INFO_SECTOR_START+ID3TITLE_SORT_INFO_SIZE) /*the start sector address of ID3_ARTIST sort information*/
#define     ID3ARTIST_SORT_INFO_SIZE                32

#define     ID3ARTIST_SORT_SUB_SECTOR_START         (ID3ARTIST_SORT_INFO_SECTOR_START+ID3ARTIST_SORT_INFO_SIZE) /*the start sector address of ID3_ARTIST classified information*/
#define     ID3ARTIST_SORT_SUB_SIZE                 128  /* ID3_ARTIST classified information occupy 64 sec that is 32k*/

#define     ID3ARTIST_ALBUM_SORT_SUB_SECTOR_START   (ID3ARTIST_SORT_SUB_SECTOR_START+ID3ARTIST_SORT_SUB_SIZE) /* ID3 ALBUM归类信息保存起始sector地址 */
#define     IID3ARTIST_ALBUM_SORT_SUB_SIZE          128

#define     ID3ALBUM_SORT_INFO_SECTOR_START         (ID3ARTIST_ALBUM_SORT_SUB_SECTOR_START+IID3ARTIST_ALBUM_SORT_SUB_SIZE) /*the start sector address of ID3_ALBUM sort information*/
#define     ID3ALBUM_SORT_INFO_SIZE                 32

#define     ID3ALBUM_SORT_SUB_SECTOR_START          (ID3ALBUM_SORT_INFO_SECTOR_START+ID3ALBUM_SORT_INFO_SIZE) /*the start sector address of ID3_ALBUM classified information*/
#define     ID3ALBUM_SORT_SUB_SIZE                  128

#define     ID3GENRE_SORT_INFO_SECTOR_START         (ID3ALBUM_SORT_SUB_SECTOR_START+ID3ALBUM_SORT_SUB_SIZE) /*the start sector address of ID3_GENRE sort information*/
#define     ID3GENRE_SORT_INFO_SIZE                 32

#define     ID3GENRE_SORT_SUB_SECTOR_START          (ID3GENRE_SORT_INFO_SECTOR_START+ID3GENRE_SORT_INFO_SIZE) /*the start sector address of ID3_GENRE classified information*/
#define     ID3GENRE_SORT_SUB_SIZE                  128

#define     ID3GENRE_ARTIST_SORT_SUB_SECTOR_START   (ID3GENRE_SORT_SUB_SECTOR_START + ID3GENRE_SORT_SUB_SIZE) /* ID3 ARTIST归类信息保存起始sector地址 */
#define     ID3GENRE_ARTIST_SORT_SUB_SIZE           128  /* ID3 ARTIST归类信息占用空间 64个sec 32k */

#define     ID3GENRE_ALBUM_SORT_SUB_SECTOR_START    (ID3GENRE_ARTIST_SORT_SUB_SECTOR_START+ID3GENRE_ARTIST_SORT_SUB_SIZE) /* ID3 ALBUM归类信息保存起始sector地址 */
#define     ID3GENRE_ALBUM_SORT_SUB_SIZE            128

#define     MUSIC_TREE_SORT_INFO_SECTOR_START       (ID3TITLE_SORT_INFO_SECTOR_START + MEDIAINFO_BLOCK_SIZE)
#define     MUSIC_TREE_SORT_INFO_SIZE               128

#define     RECORD_TREE_SORT_INFO_SECTOR_START      (MUSIC_TREE_SORT_INFO_SECTOR_START + MEDIAINFO_BLOCK_SIZE)
#define     RECORD_TREE_SORT_INFO_SIZE              128

#if(defined PIC_MEDIA) || (defined _M3U_)

#define     JPEG_TREE_ALL_SORT_INFO_SECTOR_START    (RECORD_TREE_SORT_INFO_SECTOR_START + MEDIAINFO_BLOCK_SIZE)

#define     JPEG_TREE_ALL_SORT_INFO_SIZE            128

#define     JPEG_TREE_SORT_INFO_SECTOR_START        (JPEG_TREE_ALL_SORT_INFO_SECTOR_START + MEDIAINFO_BLOCK_SIZE)

#define     JPEG_TREE_SORT_INFO_SIZE                128

#define     M3U_TREE_SORT_INFO_SECTOR_START         (JPEG_TREE_SORT_INFO_SECTOR_START + MEDIAINFO_BLOCK_SIZE)
#define     M3U_TREE_SORT_INFO_SIZE                 128

#else
#define     M3U_TREE_SORT_INFO_SECTOR_START         (RECORD_TREE_SORT_INFO_SECTOR_START + MEDIAINFO_BLOCK_SIZE)
#define     M3U_TREE_SORT_INFO_SIZE                 128
#endif

/*
*************************************************************************************************************

    Meida Lib Address map End

*************************************************************************************************************
*/
/*internal offset definition in favorite info */
#define   FAVORITE_MUSIC_SAVE_SIZE              8   /*the occupy byte number of each favourite */
#define   FAVORITE_NUM_PER_PAGE                 (MEDIAINFO_PAGE_SIZE / 8)  /*one page can save 128 file informations,as the size of buffer that write to flash is 8k, so write 128 file path informatin to flahs every time */

#define   FAVORITE_MUSIC_DIRCLUS_OFFSET         0
#define   FAVORITE_MUSIC_DIRCLUS_SIZE           4
#define   FAVORITE_MUSIC_DIRINDEX_OFFSET        (FAVORITE_MUSIC_DIRCLUS_OFFSET + FAVORITE_MUSIC_DIRCLUS_SIZE)
#define   FAVORITE_MUSIC_DIRINDEX_SIZE          4


typedef __packed struct _SORTINFO_STRUCT
{
    UINT16 BaseID;          // 排序时有效文件在文件表中的序号（从0开始计数）
    UINT16 ItemBaseID;      // UI获取条目显示信息时用，系统从文件列表中对应的
                            // ItemBaseID个文件中获取需要的显示信息（如ItemBaseID这个文件对应的专辑，歌手，文件名等）
    UINT16 ItemNum;         // UI获取条目显示信息时用，获取显示条目的总数
    UINT16 FileNum;         //排序时参与排序的文件总数

} SORTINFO_STRUCT;

#endif/*ADDR_SAVE_MACRO_H*/



