/*
********************************************************************************************
*
*        Copyright (c):Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: App\MediaLibrary\FileInfoSort.c
* Owner: ctf
* Date: 2015.8.20
* Time: 17:29:00
* Version: 1.0
* Desc: media library browser
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*      ctf      2016.1.26    17:29:00      1.0
********************************************************************************************
*/

#include "BspConfig.h"
#ifdef MEDIA_UPDATE

#define NOT_INCLUDE_OTHER
#include "typedef.h"
#include "RKOS.h"
#include "Bsp.h"
#include "global.h"
#include "SysInfoSave.h"
#include "device.h"
#include "AddrSaveMacro.h"
#include "FileInfo.h"

#define   SORT_TYPE_ITEM_NUM  4   /* 定义排序类型个数 */


#define   SORT_FILENAME_LEN   4  /* 从文件名中截取的参与排序字符的个数，目前的16k的排序空间最大可以保存8*1024个文件的排序信息*/


#define   CHILD_CHAIN_NUM   72 /* 确定排序子链表的个数 */

//#define   FILE_NAME_TYPE    0       //这个顺序必须和AddrSaveMacro.h中定义的存储顺序一致
#define   ID3_TITLE_TYPE    0
#define   ID3_ALBUM_TYPE    1
#define   ID3_ARTIST_TYPE   2
#define   ID3_GENRE_TYPE    3

typedef struct _FILE_INFO_ADD_STRUCT
{

    UINT32  add1;//第一层排序信息写Flash地址
    UINT32  add2;//第二层排序信息写Flash地址
    UINT32  add3;//第三层排序信息写Flash地址
    UINT32  add4;//第四层排序信息写Flash地址

}FILE_INFO_ADD_STRUCT; /* 文件待排序信息,define by phc*/

typedef __packed struct _FILE_INFO_INDEX_STRUCT
{

    struct  _FILE_INFO_INDEX_STRUCT   *pNext;
    UINT16  SortFileName[SORT_FILENAME_LEN + 1];

}FILE_INFO_INDEX_STRUCT; /* 文件待排序信息,define by phc*/


UINT16 gChildChainDivValue[CHILD_CHAIN_NUM];//72*2字节  存放各个子链表的首个元属的第一个字母ˇ要比较的unicode 通过与该数组元属对比找个对应的子链表
FILE_INFO_INDEX_STRUCT  *pChildChainHead[CHILD_CHAIN_NUM]; //72*4字节 各排序个子链表的头节点的指针
FILE_INFO_INDEX_STRUCT  gChildChainHead[CHILD_CHAIN_NUM];  //72*8字节 各排序个子链表的头结点

FILE_INFO_INDEX_STRUCT  *pChildChainLast;  //72*8字节 各排序个子链表的头结点
FILE_INFO_INDEX_STRUCT  *pChildChainLast2;  //72*8字节 各排序个子链表的头结点

/* 注意ˇ以下两个数组物理地址必须ˇ连ˇ具体实ˇ可将其紧挨在一起定义 */
FILE_INFO_INDEX_STRUCT  gSortNameBuffer0[SORT_FILENUM_DEFINE];//1536*8字节 保存截取的文件名信息ˇ常驻于内存ˇ排序时调用, Buffer0
UINT32 gFileSortBufferIndex;
UINT8  gFileSortBuffer[MEDIAINFO_PAGE_SIZE];

UINT16 TempBuf1[MEDIA_ID3_SAVE_CHAR_NUM];
UINT16 TempBuf2[MEDIA_ID3_SAVE_CHAR_NUM];
UINT32 gBkMDReadAdrs1;
UINT32 gBkMDReadAdrs2;
UINT32 gwSectorOffset;
UINT32 gwDataBaseAddr;


UINT8   SingerBuffer[SORT_FILENUM_DEFINE * sizeof(SORTINFO_STRUCT)]; //16k 字节 歌手分类信息buf 因为歌手数ˇ制为2048所以这个buf开16k
UINT8   GerneAblumBuffer[SORT_FILENUM_DEFINE * sizeof(SORTINFO_STRUCT)]; //8k  流派专辑共用buf 记录分类信息ˇ因为专辑数ˇ制为1024所以这个buf开8k
UINT8   FileIDBuffer[2 * SORT_FILENUM_DEFINE]; //记录排序文件号 2010.05.17 由于28时发ˇ媒体库排序buff空间被写flash冲掉ˇ修改脚本后引起空间不足而修改
UINT8   FileIDBuffer1[2 * SORT_FILENUM_DEFINE]; //记录排序文件号


UINT32  MediaInfoSaveAdd[4];
UINT32  MediaInfoReadAdd[4];

extern UINT32 SaveSortInfo(UINT16 Deep, UINT8* SubInfoBuffer,UINT16 StartID,UINT8* FileIDBuf,UINT16 Flag);
/*
--------------------------------------------------------------------------------
  Function name : UINT32 GetPYCode(UINT32 wch)
  Author        : anzhiguo
  Description   : 获取汉字字符的排序号

  Input         : wchˇ汉字字符的unicode值
  Return        : pinCode:字符拼音排序号

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
#ifdef  PYSORT_ENABLE
UINT32 GetPYCode(UINT16 wch)
{
    UINT32 pinCode;

    extern UINT32 ReadDataFromIRAM(UINT32 addr);

    pinCode = ReadDataFromIRAM(wch- UNICODE_BEGIN+BASE_PYTABLE_ADDR_IN_IRM); // 从调入IRAM的拼音排序表得到汉字的拼音排序值
    pinCode += UNICODE_BEGIN;

    return (pinCode);
}

#endif
/*
--------------------------------------------------------------------------------
  Function name : UINT32 GetCmpResult(UINT32 wch)
  Author        : anzhiguo
  Description   : 获取汉字字符的排序号

  Input         : wchˇ汉字字符的unicode值
  Return        : pinCode:字符拼音排序号

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
UINT32 GetCmpResult(UINT16 wch)
{
    UINT32 cmpCode;

#ifdef  PYSORT_ENABLE
    if ((wch >= UNICODE_BEGIN) && (wch <= UNICODE_END))
    {
        cmpCode = GetPYCode(wch);
    }
    else
#endif

        if (wch>=97&&wch<=122) // 97 = 'a', 122 = 'z'  小写字母全部转换成大写字母
        {
            cmpCode = wch-32;
        }
        else
        {
            cmpCode = wch;
        }

    return (cmpCode);
}
/*
--------------------------------------------------------------------------------
  Function name : int PinyinCharCmp(UINT32 wch1, UINT32 wch2)
  Author        : anzhiguo
  Description   : 按拼音排序ˇ比较两个字符的码大小

  Input         : wch1ˇ字符1的unicode值   wch2ˇ字符2的unicode值
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
int PinyinCharCmp(UINT16 wch1, UINT16 wch2)
{
    return (GetCmpResult(wch1) - GetCmpResult(wch2));
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 PinyinStrnCmp(UINT16 *str1, UINT16 *str2, UINT32 length)
  Author        : anzhiguo
  Description   : 按拼音排序ˇ比较两个字符串大小

  Input         : str1ˇ字符串1   str2ˇ字符串2
  Return        : 0 str1 < str2
                  1 str1 = str2
                  2 str1 > str2
  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
UINT32 PinyinStrnCmp(UINT16 *str1, UINT16 *str2/*, UINT32 length*/)
{
    UINT32  i = 0;
    UINT16 * pBkMDReadAdrs1, * pBkMDReadAdrs2;
    UINT16 * longStr1, *longStr2;

    UINT32  BkMDReadAdrs1, BkMDReadAdrs2;


//    if (str1 == NULL)
//    {
//        return 0;
//    }
//
//    if (str2 == NULL)
//    {
//          return 2;
//    }
//
//    for (i = 0; i < length; i++)
//    {
//          if ((str1[i] == NULL) || (str2[i] == NULL) || (/*GetCmpResult*/(str1[i]) != /*GetCmpResult*/(str2[i])))
//          {
//          break;
//          }
//    }
//
//    if(i==length)  return 1;

//      if((str1[i]) > (str2[i]))
//         return 2;
//      else if((str1[i]) == (str2[i]))
//         return 1;  //ˇ等
//      else
//         return 0;



    //if(NULL == (str1[0] | str2[0]))   return 1;

    if (str1[0] > str2[0])   return 2;
    if (str1[0] < str2[0])   return 0;
    if (str1[1] > str2[1])   return 2;
    if (str1[1] < str2[1])   return 0;
    if (str1[2] > str2[2])   return 2;
    if (str1[2] < str2[2])   return 0;
    if (str1[3] > str2[3])   return 2;
    if (str1[3] < str2[3])   return 0;
    //if(str1[4] > str2[4]) return 2;
    //if(str1[4] < str2[4]) return 0;
    //if(str1[5] > str2[5]) return 2;
    //if(str1[5] < str2[5]) return 0;

#if 1
    if (str1[3] == NULL)
    {
        //printf("error2\n");
        return 1;
    }
    pBkMDReadAdrs1 = (uint16 *)(&str1[SORT_FILENAME_LEN]);
    pBkMDReadAdrs2 = (uint16 *)(&str2[SORT_FILENAME_LEN]);

    BkMDReadAdrs1 = ((UINT32)(gSysConfig.MediaLibrayStartLBA+gwDataBaseAddr)<<9) + BYTE_NUM_SAVE_PER_FILE * (UINT32)(*pBkMDReadAdrs1)+gwSectorOffset;
    BkMDReadAdrs2 = ((UINT32)(gSysConfig.MediaLibrayStartLBA+gwDataBaseAddr)<<9) + BYTE_NUM_SAVE_PER_FILE * (UINT32)(*pBkMDReadAdrs2)+gwSectorOffset;

#if 1
    if (BkMDReadAdrs1 == gBkMDReadAdrs1)
    {
        longStr1 = TempBuf1;
    }
    else
    {
        FW_ReadDataBaseByByte(BkMDReadAdrs1, (uint8*)TempBuf1, MEDIA_ID3_SAVE_CHAR_NUM*2);
        longStr1 = TempBuf1;
        gBkMDReadAdrs1 = BkMDReadAdrs1;
#if 0
        {
            uint32 i;
            for (i = 0; i < 41; i++)
            {
                printf("%c ", TempBuf1[i]);
            }
            printf("\n");
        }
#endif
    }
#else
    MDReadData(DataDiskID,*pBkMDReadAdrs1, MEDIA_ID3_SAVE_CHAR_NUM*2, TempBuf1);
    longStr1 = TempBuf1;
#if 0
    {
        uint32 i;
        for (i = 0; i < 41; i++)
        {
            printf("%x ", TempBuf1[i]);
        }
        printf("\n");
    }
#endif
#endif

#if 1

    if (BkMDReadAdrs2 == gBkMDReadAdrs2)
    {
        longStr2 = TempBuf2;
    }
    else
    {
        FW_ReadDataBaseByByte(BkMDReadAdrs2, (uint8*)TempBuf2, MEDIA_ID3_SAVE_CHAR_NUM*2);
        longStr2 = TempBuf2;
        gBkMDReadAdrs2 = BkMDReadAdrs2;
    }
#else
    MDReadData(DataDiskID,*pBkMDReadAdrs2, MEDIA_ID3_SAVE_CHAR_NUM*2, TempBuf2);
    longStr2 = TempBuf2;
#endif


    for (i = 4; i < MEDIA_ID3_SAVE_CHAR_NUM; i++)
    {
        if (longStr1[i] > longStr2[i])   return 2;
        if (longStr1[i] < longStr2[i])   return 0;
        if (longStr1[i] == NULL)
        {
            //printf("i = %d\n", i);
            return 1;
        }
    }
#endif

    //printf("errror\n");
    return 1;



}
/*
--------------------------------------------------------------------------------
  Function name : UINT8 BrowserListInsertBySort(FILE_INFO_INDEX_STRUCT *head, FILE_INFO_INDEX_STRUCT *pNode)
  Author        : anzhiguo
  Description   : 初始化双ˇ链表

  Input         : *head   插入链表头指针
                  *pFileSaveTemp   待插入结点结构
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
UINT8 BrowserListInsertBySort(FILE_INFO_INDEX_STRUCT *head, FILE_INFO_INDEX_STRUCT *pNode)
{
    FILE_INFO_INDEX_STRUCT *p,*q;
    UINT32 i;

    p = head;

    //顺序扫描链表,将新结点插入到比它大的结点前
    if ((pChildChainLast->pNext!=NULL) || (PinyinStrnCmp((uint16*)(pChildChainLast->SortFileName), (uint16*)(pNode->SortFileName)/*, SORT_FILENAME_LEN*/)==2))
    {
        if ((pChildChainLast2->pNext!=NULL) && (PinyinStrnCmp((uint16*)(pChildChainLast2->SortFileName), (uint16*)(pNode->SortFileName)/*, SORT_FILENAME_LEN*/)==2))
        {
            //printf(" bk reset : pChildChainLast2 = %p  \n", p );
        }
        else
        {
            p = pChildChainLast2;

        }

        while (p->pNext!=NULL)
        {
            q = p->pNext;
            // 2 表示 q->SortFileName > pNode->SortFileName
            if (PinyinStrnCmp((uint16*)(q->SortFileName), (uint16*)(pNode->SortFileName)/*, SORT_FILENAME_LEN*/)==2) //顺序扫描链表,将新结点插入到比它大的结点前
            {
                pNode->pNext = q;
                p->pNext = pNode;
                pChildChainLast2 = pNode;
                return 1;
            }
            p = q;
        }
    }
    else
    {
        p = pChildChainLast;
    }

    if (p->pNext==NULL) // 若到了链尾,说明插入的结点为当前链表最大值,将其插入到链尾
    {
        pNode->pNext = NULL;
        p->pNext = pNode;
        pChildChainLast = pNode;
        return 1;
    }

    return 0;
}
/*
--------------------------------------------------------------------------------
  Function name : void SortPageBufferInit(UINT8 *pBuffer)
  Author        : anzhiguo
  Description   :

  Input         : *pBuffer   要初始化的buf指针

  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
void SortPageBufferInit(UINT8 *pBuffer)
{
    UINT32  i;

    if (pBuffer == GerneAblumBuffer)
    {
        //memset(GerneAblumBuffer,0,8*1024);
        memset(GerneAblumBuffer,0,sizeof(GerneAblumBuffer));
    }

    if (pBuffer == SingerBuffer)
    {
        //memset(SingerBuffer,0,16*1024);
        memset(SingerBuffer,0,sizeof(SingerBuffer));
    }

}

/*
--------------------------------------------------------------------------------
  Function name : void ChildChainInit(void)
  Author        : anzhiguo
  Description   : 初始化双ˇ链表

  Input         :

  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:         gChildChainDivValue[i] 与 pChildChainHead[i] 一一对应
--------------------------------------------------------------------------------
*/
void ChildChainInit(UINT16 Flag)
{
    UINT32 i,j;
    for (i=0;i<CHILD_CHAIN_NUM;i++) // 初始化各链表头节点
    {

        //pChildChainHead 是一个指针数组ˇ它的每个元素均为一个FILE_INFO_INDEX_STRUCT类型的指针ˇ
        //gChildChainHead 是一个结构体数组ˇ它的每个元素均为一个FILE_INFO_INDEX_STRUCT类型的结构体变量ˇ
        pChildChainHead[i] = &gChildChainHead[i];
        pChildChainHead[i]->pNext = NULL;
    }

    //多次调度时ˇ此处下面的代码只需在进入排序模块时执行一次就可以ˇ因此可以考虑传一个参数用于判断是不是第一次进入
    if (Flag)
    {
        gChildChainDivValue[0]=  0x0000; // 排序字符为空的放置于此
        gChildChainDivValue[1]=   0x41; // 小于英文字母的字符ˇ包括数字及其它符号  0x41 A字母的ascall 码

        j = 2;
        for (i='A';i<'Z'+1;i++)
        {
            gChildChainDivValue[j] = i+1; // 英文字符,A~Z
            j++;
        }

        gChildChainDivValue[j++] = 0x4DFF; // 英文字符到汉字之间的unicode字符

        j = 29;
        for (i=j;i<CHILD_CHAIN_NUM-1;i++)
        {
            gChildChainDivValue[i] = gChildChainDivValue[i-1]+(0x51a5)/(CHILD_CHAIN_NUM-30); // 对汉字部分字符进行42等分
        }

        gChildChainDivValue[CHILD_CHAIN_NUM-1] = 0xffff; // 大于汉字的其它字符全部放至此链表
    }
    /*for(i=0;i<CHILD_CHAIN_NUM;i++)
    {
       DisplayTestDecNum((i%6)*50,(i/6)*15,gChildChainDivValue[i]);
    }
    while(1);*/
    /* 以下设置是为了在拼音排序时ˇ按拼音字母分隔排序链表 */
    /*gChildChainDivValue[0]=   0x0000;
    gChildChainDivValue[1]=   0x4DFF; //0x0000~0x4DFF; //  0~9,字母,其它符号
    gChildChainDivValue[2] =  0x4EC3; //0x4E00~0x4EC3; //  A, 吖
    gChildChainDivValue[3] =  0x5235; //0x4EC4~0x5235; //  Bˇ八
    gChildChainDivValue[4] =  0x576a; //0x5236~0x576a; //  C, 嚓
    gChildChainDivValue[5] =  0x5b40; //0x576b~0x5b40; //  D, ˇ
    gChildChainDivValue[6] =  0x5bf0; //0x5b41~0x5bf0; //  Eˇ酬
    gChildChainDivValue[7] =  0x5e55; //0x5bf1~0x5e55; //  F, 发
    gChildChainDivValue[8] =  0x6198; //0x5e56~0x6198; //  G, 旮
    gChildChainDivValue[9] =  0x65b3; //0x6199~0x65b3; //  H, 承,ha
    gChildChainDivValue[10] = 0x6c00; //0x65b4~0x6c00; //  J, 丌
    gChildChainDivValue[11] = 0x6e30; //0x6c01~0x6e30; //  K, 咔
    gChildChainDivValue[12] = 0x7448; //0x6e31~0x7448; //  L, 垃
    gChildChainDivValue[13] = 0x7810; //0x7449~0x7810; //  M, 
    gChildChainDivValue[14] = 0x7953; //0x7811~0x7953; //  N, ˇ
    gChildChainDivValue[15] = 0x7978; //0x7954~0x7978; //  O, 噢
    gChildChainDivValue[16] = 0x7be2; //0x7979~0x7be2; //  P, 秤
    gChildChainDivValue[17] = 0x7fe9; //0x7be3~0x7fe9; //  q, 七,qi
    gChildChainDivValue[18] = 0x8131; //0x7fea~0x8131; //  r, ◎,ra
    gChildChainDivValue[19] = 0x8650; //0x8132~0x8650; //  s, 仨,sa
    gChildChainDivValue[20] = 0x89d0; //0x8651~0x89d0; //  t, 他
    gChildChainDivValue[21] = 0x8c43; //0x89d1~0x8c43; //  w, 服
    gChildChainDivValue[22] = 0x9169; //0x8c44~0x9169; //  x, 夕
    gChildChainDivValue[23] = 0x9904; //0x916a~0x9904; //  y, 丫
    gChildChainDivValue[24] = 0x9fa4; //0x9905~0x9fa4; //  z, */
}

/*
--------------------------------------------------------------------------------
  Function name : void GetSortName(UINT32 SectorOffset)
  Author        : anzhiguo
  Description   : 从flash中读取所有文件的长文件名ˇ并存放到 gSortNameBuffer0和 gSortNameBuffer1 中
                  这两块空间是连续的ˇ每个可以存放的文件数是系统允许的做大文件数的一半

  Input         : SectorOffsetˇ待排序字符在文件信息中的偏移地址

  Return        : Flag 判断读取文件的顺序,Flag =0ˇ按FileIDBuffer中的顺序读取文件

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:         从保存在Flash中的文件信息读取需要排序的字符(文件名、ID3信息)ˇ并且存与gSortNameBuffer0[].SortFileName中
                将所有文件的信息(需要排序的字符信息)读取
--------------------------------------------------------------------------------
*/
void GetSortName(UINT32 SectorOffset,UINT16 FileNum,UINT16 Flag)
{
    UINT16  i,j;
    UINT16  FileCount;
    UINT8  TempBuffer[SORT_FILENAME_LEN*2];
    FILE_INFO_INDEX_STRUCT *pTemp;
    UINT16  *FileID;
    uint16 * BkMDReadAdrs;
    pTemp = gSortNameBuffer0;
    FileCount = FileNum;


    FileID = (UINT16 *)FileIDBuffer;

    BkMDReadAdrs = (uint16 *)(pTemp->SortFileName + SORT_FILENAME_LEN);

    gwSectorOffset = SectorOffset;
    gwDataBaseAddr = MUSIC_SAVE_INFO_SECTOR_START;

    for (i=0;i<FileCount;i++)
    {
        //从flash中读取截取的长文件名信息  //这里地址的计算在4k page的flash中会出问题
        if (Flag)
        {
            FW_ReadDataBaseByByte(((UINT32)(gSysConfig.MediaLibrayStartLBA+MUSIC_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(i)+SectorOffset, TempBuffer, SORT_FILENAME_LEN*2);
            //*BkMDReadAdrs = ((UINT32)(gSysConfig.MediaLibrayStartLBA+MUSIC_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(i)+SectorOffset;
            *BkMDReadAdrs = i;
            //printf("*BkMDReadAdrs = %d\n", *BkMDReadAdrs);
        }
        else
        {
            FW_ReadDataBaseByByte(((UINT32)(gSysConfig.MediaLibrayStartLBA+MUSIC_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(FileID[i])+SectorOffset, TempBuffer, SORT_FILENAME_LEN*2);
            //*BkMDReadAdrs = ((UINT32)(gSysConfig.MediaLibrayStartLBA+MUSIC_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(FileID[i])+SectorOffset;
            *BkMDReadAdrs = FileID[i];
            //printf("*BkMDReadAdrs = %d\n", *BkMDReadAdrs);
        }

        for (j=0;j<SORT_FILENAME_LEN;j++)
        {
            pTemp->SortFileName[j] = (TempBuffer[j*2]&0xff)+((TempBuffer[j*2+1]&0xff)<<8);
            if (
                (97 <=pTemp->SortFileName[j])
                && (122 >=pTemp->SortFileName[j])
            )
            {
                pTemp->SortFileName[j] = pTemp->SortFileName[j] - 32;
            }
        }

        pTemp++;
        BkMDReadAdrs = (uint16 *)(pTemp->SortFileName + SORT_FILENAME_LEN);
    }

}

void GetDirSortName(UINT32 SectorOffset,UINT32 StartID, UINT16 * FileNum, UINT16 Flag, UINT8 DirSort)
{
    UINT16  i,j;
    UINT16  FileCount;
    UINT8  TempBuffer[SORT_FILENAME_LEN*2];
    FILE_TREE_BASIC stFileTreeBasic;
    uint32 TreeSectStart, SaveInfoStart;

    FILE_INFO_INDEX_STRUCT *pTemp;
    UINT16  *FileID;
    uint16 * BkMDReadAdrs;

    pTemp = gSortNameBuffer0;
    FileID = (UINT16 *)FileIDBuffer;

    BkMDReadAdrs = (uint16 *)(pTemp->SortFileName + SORT_FILENAME_LEN);

    if (DirSort == 1)
    {
        TreeSectStart = MUSIC_TREE_INFO_SECTOR_START;
        SaveInfoStart = MUSIC_SAVE_INFO_SECTOR_START;
    }
    else if (DirSort == 2)
    {
        TreeSectStart = RECORD_TREE_INFO_SECTOR_START;
        SaveInfoStart = RECORD_SAVE_INFO_SECTOR_START;
    }
#ifdef PIC_MEDIA
    else if(DirSort == 4)
    {
        TreeSectStart = JPEG_TREE_INFO_SECTOR_START;
        SaveInfoStart = JPEG_SAVE_INFO_SECTOR_START;
    }
#endif
    else
    {
        *FileNum = 0;
        return;
    }

    gwSectorOffset = SectorOffset;
    gwDataBaseAddr = SaveInfoStart;

    if (Flag)
    {
        //first get info need count totalnum
        FileCount = 0;
        do
        {
            FW_ReadDataBaseByByte(((UINT32)(gSysConfig.MediaLibrayStartLBA + TreeSectStart)<<9) + sizeof(FILE_TREE_BASIC)*(UINT32)(StartID + FileCount), (uint8 *)&stFileTreeBasic, sizeof(FILE_TREE_BASIC));
            FileCount++;

            FW_ReadDataBaseByByte(((UINT32)(gSysConfig.MediaLibrayStartLBA+SaveInfoStart)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(stFileTreeBasic.dwBasicInfoID)+SectorOffset, TempBuffer, SORT_FILENAME_LEN*2);
            //*BkMDReadAdrs = ((UINT32)(gSysConfig.MediaLibrayStartLBA+SaveInfoStart)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(stFileTreeBasic.dwBasicInfoID)+SectorOffset;
            *BkMDReadAdrs = stFileTreeBasic.dwBasicInfoID;

            for (j=0;j<SORT_FILENAME_LEN;j++)
            {
                pTemp->SortFileName[j] = (TempBuffer[j*2]&0xff)+((TempBuffer[j*2+1]&0xff)<<8);
                if (
                    (97 <=pTemp->SortFileName[j])
                    && (122 >=pTemp->SortFileName[j])
                )
                {
                    pTemp->SortFileName[j] = pTemp->SortFileName[j] - 32;
                }
            }

            pTemp++;
            BkMDReadAdrs = (uint16 *)(pTemp->SortFileName + SORT_FILENAME_LEN);
        }
        while (stFileTreeBasic.dwNextBrotherID != 0xffffffff);

        *FileNum = FileCount;

    }
    else
    {
        FileCount = *FileNum;

        for (i=0;i<FileCount;i++)
        {
            FW_ReadDataBaseByByte(((UINT32)(gSysConfig.MediaLibrayStartLBA + TreeSectStart)<<9) + sizeof(FILE_TREE_BASIC)*(UINT32)(StartID + FileID[i]), (uint8 *)&stFileTreeBasic, sizeof(FILE_TREE_BASIC));

            FW_ReadDataBaseByByte(((UINT32)(gSysConfig.MediaLibrayStartLBA+SaveInfoStart)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(stFileTreeBasic.dwBasicInfoID)+SectorOffset, TempBuffer, SORT_FILENAME_LEN*2);
            *BkMDReadAdrs = stFileTreeBasic.dwBasicInfoID;

            for (j=0;j<SORT_FILENAME_LEN;j++)
            {
                pTemp->SortFileName[j] = (TempBuffer[j*2]&0xff)+((TempBuffer[j*2+1]&0xff)<<8);
                if (
                    (97 <=pTemp->SortFileName[j])
                    && (122 >=pTemp->SortFileName[j])
                )
                {
                    pTemp->SortFileName[j] = pTemp->SortFileName[j] - 32;
                }
            }

            pTemp++;
            BkMDReadAdrs = (uint16 *)(pTemp->SortFileName + SORT_FILENAME_LEN);
        }
    }
}

#ifdef PIC_MEDIA
void GetSortNameJpeg(UINT32 SectorOffset,UINT16 FileNum,UINT16 Flag)
{
    UINT16  i,j;
    UINT16  FileCount;
    UINT8  TempBuffer[SORT_FILENAME_LEN*2];
    FILE_INFO_INDEX_STRUCT *pTemp;
    UINT16  *FileID;
    uint16 * BkMDReadAdrs;
    pTemp = gSortNameBuffer0;
    FileCount = FileNum;


    FileID = (UINT16 *)FileIDBuffer;

    BkMDReadAdrs = (uint16 *)(pTemp->SortFileName + SORT_FILENAME_LEN);

    gwSectorOffset = SectorOffset;
    gwDataBaseAddr = JPEG_SAVE_INFO_SECTOR_START;

    for (i=0;i<FileCount;i++)
    {
        if (Flag)
        {
            FW_ReadDataBaseByByte(((UINT32)(gSysConfig.MediaLibrayStartLBA+JPEG_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(i)+SectorOffset, TempBuffer, SORT_FILENAME_LEN*2);
            *BkMDReadAdrs = i;
        }
        else
        {
            FW_ReadDataBaseByByte(((UINT32)(gSysConfig.MediaLibrayStartLBA+JPEG_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(FileID[i])+SectorOffset, TempBuffer, SORT_FILENAME_LEN*2);
            *BkMDReadAdrs = FileID[i];
        }

        for (j=0;j<SORT_FILENAME_LEN;j++)
        {
            pTemp->SortFileName[j] = (TempBuffer[j*2]&0xff)+((TempBuffer[j*2+1]&0xff)<<8);
            if ((97 <=pTemp->SortFileName[j])&& (122 >=pTemp->SortFileName[j]))
            {
                pTemp->SortFileName[j] = pTemp->SortFileName[j] - 32;
            }
        }

        pTemp++;
        BkMDReadAdrs = (uint16 *)(pTemp->SortFileName + SORT_FILENAME_LEN);
    }

}
#endif

#ifdef _M3U_
void GetSortNameM3u(UINT32 SectorOffset,UINT16 FileNum,UINT16 Flag)
{
    UINT16  i,j;
    UINT16  FileCount;
    UINT8  TempBuffer[SORT_FILENAME_LEN*2];
    FILE_INFO_INDEX_STRUCT *pTemp;
    UINT16  *FileID;
    uint16 * BkMDReadAdrs;
    pTemp = gSortNameBuffer0;
    FileCount = FileNum;

    FileID = (UINT16 *)FileIDBuffer;

    BkMDReadAdrs = (uint16 *)(pTemp->SortFileName + SORT_FILENAME_LEN);

    gwSectorOffset = SectorOffset;
    gwDataBaseAddr = M3U_SAVE_INFO_SECTOR_START;

    for(i=0;i<FileCount;i++)
    {
        if(Flag)
        {
            FW_ReadDataBaseByByte(((UINT32)(gSysConfig.MediaLibrayStartLBA+M3U_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(i)+SectorOffset, TempBuffer, SORT_FILENAME_LEN*2);
            *BkMDReadAdrs = i;
        }
        else
        {
            FW_ReadDataBaseByByte(((UINT32)(gSysConfig.MediaLibrayStartLBA+M3U_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(FileID[i])+SectorOffset, TempBuffer, SORT_FILENAME_LEN*2);
            *BkMDReadAdrs = FileID[i];
        }

        for(j=0;j<SORT_FILENAME_LEN;j++)
        {
            pTemp->SortFileName[j] = (TempBuffer[j*2]&0xff)+((TempBuffer[j*2+1]&0xff)<<8);
            if(
                   (97 <=pTemp->SortFileName[j])
                && (122 >=pTemp->SortFileName[j])
            )
            {
                pTemp->SortFileName[j] = pTemp->SortFileName[j] - 32;
            }
        }
        pTemp++;
        BkMDReadAdrs = (uint16 *)(pTemp->SortFileName + SORT_FILENAME_LEN);
    }
}
#endif

/*
--------------------------------------------------------------------------------
  Function name : UINT32 SaveSortInfo(UINT32 uiSaveType, UINT32 ulSortSectorAddr)
  Author        : anzhiguo
  Description   : 保存分类信息

  Input         : uiSaveType 保存信息的类型 : ID3_TITLE_TYPE :FILE_NAME_TYPE:ID3_ARTIST_TYPE:ID3_ALBUM_TYPE 四种类型
                  ulSortSectorAddr 信息保存的地址ˇ物理地址还是sec地址 ?
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
UINT32 SaveSortInfo(UINT16 Deep, UINT8* SubInfoBuffer,UINT16 StartID,UINT8* FileIDBuf,UINT16 Flag)
{
    UINT32  i,j;

    UINT32 uiFileCount = 0;
    UINT32 uiFileSubCount = 0;  // 文件个数计数
    UINT32 uiSameCount = 0;     // ID3信息归类时ˇˇ同类型计数
    UINT32 uiFileIndex = 0;     // Flash Page控制计数
    UINT16 uiFileSortTemp = 0;  // 文件排序号临时变量

    UINT32 uiSubCount = 0;      // 用于每条子链的文件ID3信息分类计数
    UINT32 uiSubCountTotal = 0; // 用于ID3信息分类的所有文件计数

    UINT32 ulID3SubSectorAddr = 0; // ID3信息归类信息保存地址

    UINT16 StartfileID = StartID; // 分类信息起始文件号
    UINT16 PreSortFileName[SORT_FILENAME_LEN + 2];

    FILE_INFO_INDEX_STRUCT  *pTemp = NULL; // 排序链表头指针
    if (!Flag)
    {
        //memcpy(FileIDBuf,FileIDBuffer,4096);
        memcpy(FileIDBuf, FileIDBuffer, 2 * SORT_FILENUM_DEFINE);
    }

    switch (Deep)
    {
        case 1:
            // case FILE_NAME_TYPE:
            // case ID3_TITLE_TYPE:
            i = 1;
            break;
        case 2:
            // case ID3_ARTIST_TYPE:
            i = 0;
            break;
            // case ID3_ALBUM_TYPE:
        case 3:
            i = 0;
            break;
            // case ID3_GENRE_TYPE:
        case 4:
            i = 0;
            break;
        default:
            break;

    }

    for (;i<CHILD_CHAIN_NUM;i++)
    {
        pTemp = pChildChainHead[i];

        while (pTemp->pNext!=NULL)
        {
            pTemp = pTemp->pNext;
            uiFileSortTemp = (UINT16)(pTemp - &gSortNameBuffer0[0]); //计算当前的文件在所有文件中的排序号

            //计算当前要保存的信息与gSortNameBuffer0[0]的偏移地址ˇ因为pChildChainHead链表的信息来源于gSortNameBuffer0
            if (Flag)
            {
                FileIDBuffer[StartID*2 +uiFileIndex++] =  uiFileSortTemp & 0xFF;     //保存文件排序后的序号
                FileIDBuffer[StartID*2 +uiFileIndex++] = (uiFileSortTemp>>8) & 0xFF;//保存文件排序后的序号

                //printf("uiFileSortTemp = %d = %d\n", uiFileSortTemp, StartID);
            }
            else
            {

                FileIDBuffer[StartID*2 +uiFileIndex++] =  FileIDBuf[uiFileSortTemp*2] & 0xFF;     //保存文件排序后的序号
                FileIDBuffer[StartID*2 +uiFileIndex++] =  FileIDBuf[uiFileSortTemp*2+1] & 0xFF;//保存文件排序后的序号

                //printf("Start = %d\n", StartID);
            }

            //printf("id = %d\n", *(uint16 *)(FileIDBuffer + StartID*2 + uiFileIndex - 2));
            switch (Deep)
            {
                case 1://一层深度时ˇ只要最终的文件号顺序链表ˇ不需要sub子ˇ的信息
                    // case FILE_NAME_TYPE:
                    // case ID3_TITLE_TYPE:
                    uiFileSubCount++;
                    break;
                    // case ID3_ARTIST_TYPE://同一个艺术家 或专辑有几个文件ˇ保存其第一个文件的偏移地址ˇ并保存该专辑下的文件个数
                    //  case ID3_ALBUM_TYPE :
                    // case ID3_GENRE_TYPE   :
                case 2:
                case 3:
                case 4:
                    {
                        if (0 == uiFileSubCount)
                        {
                            SubInfoBuffer[uiSubCount++] = StartfileID&0xFF; // 起始文件号
                            SubInfoBuffer[uiSubCount++] = (StartfileID>>8)&0xFF;// 起始文件号

                            SubInfoBuffer[uiSubCount++] = 0;//子ˇ的起始位置ˇ更新完下一级时才写入数据
                            SubInfoBuffer[uiSubCount++] = 0;
                            // printf("uiFileSubCount = %d\n", uiFileSubCount);
                            uiFileSubCount++;
                        }
                        //PinyinStrnCmp 返回 1 表示 两个字符窜ˇ等ˇ同一个艺术家或同一个专辑
                        else if (1==PinyinStrnCmp((uint16*)(pTemp->SortFileName), PreSortFileName/*, SORT_FILENAME_LEN*/)) //(pTemp->SortFileName)等于(pNodePrev->SortFileName)
                        {
                            //printf("same name\n");
                            uiSameCount++;
                        }
                        else // (pTemp->SortFileName)大于(pNodePrev->SortFileName),不会有小于的情况ˇ因为已经排过序的了
                        {
                            StartfileID += (uiSameCount+1);
                            SubInfoBuffer[uiSubCount++] = 0; // 保留空间, 使每次保存大小为2指数倍
                            SubInfoBuffer[uiSubCount++] = 0; // 保留空间, 使每次保存大小为2指数倍

                            //当前这个流派 ˇ专辑ˇ歌手下有多少个文件
                            SubInfoBuffer[uiSubCount++] = (uiSameCount+1)&0xFF; // 记录上一个Item包含的元素个数
                            SubInfoBuffer[uiSubCount++] = ((uiSameCount+1)>>8)& 0xFF;


                            //记录下一个分类ˇ的起始文件位置
                            SubInfoBuffer[uiSubCount++] = StartfileID&0xFF;
                            SubInfoBuffer[uiSubCount++] = (StartfileID>>8)&0xFF;

                            SubInfoBuffer[uiSubCount++] = 0; // //子ˇ的起始位置ˇ更新完下一级时才写入数据
                            SubInfoBuffer[uiSubCount++] = 0; // //子ˇ的起始位置ˇ更新完下一级时才写入数据


                            uiFileSubCount++;

                            uiSameCount = 0;
                        }
                    }
                    break;

                default:
                    break;

            }

            for (j=0;j<(SORT_FILENAME_LEN + 2);j++)
            {
                PreSortFileName[j] = pTemp->SortFileName[j];
            }
            uiFileCount++;
        }

    }
#if 1
    //  if (uiSubCount)//// 保存排序信息不足2K的部分
    //  {

    SubInfoBuffer[uiSubCount++] = 0; // 保留空间, 使每次保存大小为2指数倍
    SubInfoBuffer[uiSubCount++] = 0; // 保留空间, 使每次保存大小为2指数倍

    SubInfoBuffer[uiSubCount++] = (uiSameCount+1)&0xFF; // 统计最后一个Item的个数
    SubInfoBuffer[uiSubCount++] = ((uiSameCount+1)>>8)&0xFF;

    // }
    //printf("uiFileSubCount = %d\n", uiFileSubCount);
#endif

    return uiFileSubCount;


}

/*
--------------------------------------------------------------------------------
  Function name : void SortUpdateFun(void)
  Author        : anzhiguo
  Description   : 对指定范围内的文件进行排序

  Input         : StartNum--排序文件的起始文件号
                  FileNum --排序的文件个数
                  SubInfoBuffer  --存放本次排序后的sub信息
                  PreSubInfoBuffer -- 存放上一级排序的sbu信息ˇ为本级排序提供一些必要的参数
                  Deep    -- 判断当前排序的内容
  Return        : 返回当前范围内排序后分ˇ个数

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
UINT32 SingleTypeSortFunction(UINT16 StartNum,UINT16 FileNum,UINT16 *SubInfoBuffer,UINT16 Deep,UINT16 Flag)
{
    UINT32  i,j,SortSubNum;
    UINT16  filenum;
    UINT32 uiFirsCharSortVal=0; // 当前插入结点排序名字的首字符的拼音排序值

    FILE_INFO_INDEX_STRUCT *pHeadBk = NULL;
    FILE_INFO_INDEX_STRUCT *pTemp;

    pTemp = &gSortNameBuffer0[StartNum];

    filenum = FileNum;

    for (i=0;i<filenum;i++)
    {
        uiFirsCharSortVal = /*GetCmpResult*/(pTemp->SortFileName[0]);
        j = 0;
        while (uiFirsCharSortVal>gChildChainDivValue[j]) // 通过长文件的第一个字符来判断需要在哪个子链表中排序
            j++;

        if (pHeadBk != pChildChainHead[j])
        {
            pHeadBk = pChildChainHead[j];                                       /////////////
            pChildChainLast = pChildChainHead[j];
            pChildChainLast2 = pChildChainHead[j];
        }
        BrowserListInsertBySort(pChildChainHead[j], pTemp); // 将gSortNameBuffer0中的文件信息逐个插入链表pChildChainHead中
        pTemp++;
    }

    //SortSubNum 带回排序后的分类sub数目
    SortSubNum = SaveSortInfo(Deep, (UINT8*)SubInfoBuffer, StartNum, FileIDBuffer1, Flag); //保存文件的排序号ˇ以及分类信息ˇ并返回该类文件的数量

    return SortSubNum;
}

/*
--------------------------------------------------------------------------------
  Function name : void GetSortName(UINT32 SectorOffset)
  Author        : anzhiguo
  Description   : 文件排序功能函数

  Input         : uiSaveAddrOffsetˇ待排序字符(文件名ˇid3titleˇid3singerˇid3ablum)在文件信息中的偏移地址
                  FileNum---排序的文件总数
                  Deep  --- 排序的深度(目录层次)
                 // uiSortType ---排序的类型
                  SortSubNum ---排序后生成的子ˇ个数
                  buffer1   --- 排序信息存放buff
                  buffer2   --- 排序中用到上级排序的信息buf
                  Flag  --- 读取文件信息的方式
                  SubNum --- 用于分级排序时ˇ确定外层循环次数
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:         将gSortNameBuffer0和gSortNameBuffer1 中的信息进行分类排列ˇ即分派到对应子链表pChildChainHead的适当的位子上
--------------------------------------------------------------------------------
*/
void SortFunction(UINT32* uiSaveAddrOffset,UINT16 * FileNum,UINT16 Deep,UINT32 *SortSubNum,UINT16 Flag,UINT16*buffer1,UINT16*buffer2,UINT16 SubNum, UINT32 StartID, UINT8 DirSort)
{
    UINT32  i,j;

    UINT16* subbuf1,subbuf2;
    UINT32 uiFirsCharSortVal=0; // 当前插入结点排序名字的首字符的拼音排序值
    UINT16  filenum = 0;
    SORTINFO_STRUCT * pSortInfo1;
    SORTINFO_STRUCT * pSortInfo2;

    FILE_INFO_INDEX_STRUCT *pTemp;

    pTemp = gSortNameBuffer0;

    memset(&gSortNameBuffer0[0], 0, sizeof(FILE_INFO_INDEX_STRUCT) * SORT_FILENUM_DEFINE);

    //DEBUG("FileNum = %d", FileNum);
    gBkMDReadAdrs1 = 0xffffffff;
    gBkMDReadAdrs2 = 0xffffffff;

#ifdef PIC_MEDIA
    if (DirSort == 3)
    {
        GetSortNameJpeg(uiSaveAddrOffset[Deep-1], *FileNum, Flag);
    }
    else if(DirSort == 5)
    {
        GetSortNameM3u(uiSaveAddrOffset[Deep-1], *FileNum, Flag);
    }
    else
#endif
    {
        if (DirSort)
        {
            GetDirSortName(uiSaveAddrOffset[Deep-1],StartID, FileNum, Flag, DirSort);
        }
        else
        {
            GetSortName(uiSaveAddrOffset[Deep-1], *FileNum, Flag);     // 根据当前的排序深度ˇ获取需要的排序信息(所有文件)到gSortNameBuffer0中
        }
    }

    if (Flag)
    {
        ChildChainInit(1);  // 对各子链表的头节点进行初始化ˇ并给定义每个链表的分段关键字

        //SortSubNum获取最外一层排序后的分类子ˇ个数
        //从gSortNameBuffer0的第0个位置的文件开始ˇ排序FileNum个文件ˇ排序后的分类信息以结构存放在buffer1中
        //buffer2 存放上一级排序的信息ˇ在内层排序时ˇ提供一些参数
        filenum = SingleTypeSortFunction(0,*FileNum,buffer1,Deep,Flag);
        *SortSubNum  = filenum;
    }
    else
    {
        UINT16 IDnum ,basenum;
        pSortInfo1 = (SORTINFO_STRUCT *)buffer2 ;
        pSortInfo1->ItemBaseID = 0 ;
        pSortInfo1->ItemNum = 0;
        basenum = 0;
        pSortInfo2 = (SORTINFO_STRUCT *)buffer1 ;

        for (i=0; i < SubNum; i++)
        {   //获取分类子ˇ个数
            ChildChainInit(0);
            IDnum = SingleTypeSortFunction(pSortInfo1->BaseID,pSortInfo1->FileNum,(UINT16*)pSortInfo2,Deep,Flag);
            //偏移存放排序后sub的头指针ˇ为下次排序准备,这里是否需要加保护措施?????

            pSortInfo2 += IDnum;

            //numtemp = pSortInfo1->ItemNum;
            //保存分类子ˇ个数
            basenum += IDnum ;
            pSortInfo1->ItemNum = IDnum;
            //结构指针自加ˇ指ˇ一个sub块的信息结构ˇ并保存其起始id
            if (i < (SORT_FILENUM_DEFINE - 1))
            {
                pSortInfo1 ++;

                pSortInfo1->ItemBaseID = basenum ;
            }
            filenum +=IDnum;

        }
        //在这里ˇ上层排序完成ˇ需要写buffer2到flash中以保存信息

        if (DirSort == 0)
        {
            FW_WriteDataBaseBySector(MediaInfoSaveAdd[Deep], (uint8*)buffer2, SORT_FILENUM_DEFINE * sizeof(SORTINFO_STRUCT) / SECTOR_BYTE_SIZE);
        }
        SortPageBufferInit((UINT8*)buffer2);
    }

    Deep --;

    if (Deep == 0)
    {
        //将fileidbuffer 写flashˇ以及排序sub信息buffer

        if (DirSort == 0)
        {
            FW_WriteDataBaseBySector(MediaInfoSaveAdd[0], FileIDBuffer, (2 * SORT_FILENUM_DEFINE) / SECTOR_BYTE_SIZE);
        }
        else
        {

            //printf("xxx\n");
            for (i = 0; i < (*FileNum * 2); i++)
            {
                gFileSortBuffer[gFileSortBufferIndex++] = FileIDBuffer[i];
                if (gFileSortBufferIndex == MEDIAINFO_PAGE_SIZE)
                {
                    FW_WriteDataBaseBySector(MediaInfoSaveAdd[0], gFileSortBuffer, MEDIAINFO_PAGE_SIZE/SECTOR_BYTE_SIZE);
                    gFileSortBufferIndex = 0;
                    memset(gFileSortBuffer, 0, MEDIAINFO_PAGE_SIZE);
                    MediaInfoSaveAdd[0] += MEDIAINFO_PAGE_SIZE/SECTOR_BYTE_SIZE;
                }
            }
        }
        return;
    }
    else
    {
        SortFunction(uiSaveAddrOffset, FileNum, Deep, SortSubNum,0,buffer2,buffer1,filenum, StartID, DirSort);
    }

}

/*
--------------------------------------------------------------------------------
  Function name : void SortUpdateFun(void)
  Author        : anzhiguo
  Description   : 对文件名信息进行排序ˇ并在指定Flash中存储排序信息

  Input         :

  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
void SortUpdateFun(MEDIALIB_CONFIG * Sysfilenum ,UINT32 MediaInfoAddress)
{
    UINT16   PathDeep;//媒体库排序深的(目录层数)
    UINT16   *buffer1;
    UINT16   *buffer2;
    UINT32   uiSortTypeCount = 0;            // 排序类型选择

    UINT32   uiCountTemp[SORT_TYPE_ITEM_NUM];
    UINT16   FileNum;
    UINT32   StartID;

    if (Sysfilenum->gMusicFileNum != 0)
    {
        buffer1 = (UINT16*)GerneAblumBuffer;
        buffer2 = (UINT16*)SingerBuffer;

        for (uiSortTypeCount=0;uiSortTypeCount<SORT_TYPE_ITEM_NUM;uiSortTypeCount++)
            //uiSortTypeCount = ID3_ARTIST_TYPE;
        {
            switch (uiSortTypeCount)//设置ˇ应的地址信息
            {
                    /*
                    case FILE_NAME_TYPE:
                        //排序结束后ˇ排序结果保存到flash中的地址
                        MediaInfoSaveAdd[0] = MediaInfoAddress + FILENAME_SORT_INFO_SECTOR_START;
                        //排序开始前ˇ需要从该地址处读取需要的排序信息
                        MediaInfoReadAdd[0] = FILE_NAME_SAVE_ADDR_OFFSET;

                        PathDeep = 1;
                        uiSortType = SORT_TYPE1;
                        break;
                    */
                case ID3_ALBUM_TYPE:
                    MediaInfoReadAdd[0] = TRACKID_SAVE_ADDR_OFFSET;
                    MediaInfoReadAdd[1] = ID3_ALBUM_SAVE_ADDR_OFFSET;

                    MediaInfoSaveAdd[0] = MediaInfoAddress + ID3ALBUM_SORT_INFO_SECTOR_START;
                    MediaInfoSaveAdd[1] = MediaInfoAddress + ID3ALBUM_SORT_SUB_SECTOR_START;

                    PathDeep = 2;
                    break;


                case ID3_TITLE_TYPE:
                    MediaInfoReadAdd[0] = ID3_TITLE_SAVE_ADDR_OFFSET;//保存的文件信息的偏移地址(ˇ对于保存文件信息起始地址)
                    MediaInfoSaveAdd[0] = MediaInfoAddress + ID3TITLE_SORT_INFO_SECTOR_START;

                    PathDeep = 1;
                    break;

                case ID3_ARTIST_TYPE:
                    MediaInfoReadAdd[0] = TRACKID_SAVE_ADDR_OFFSET;
                    MediaInfoReadAdd[1] = ID3_ALBUM_SAVE_ADDR_OFFSET;
                    MediaInfoReadAdd[2] = ID3_SINGLE_SAVE_ADDR_OFFSET;

                    MediaInfoSaveAdd[0] = MediaInfoAddress + ID3ARTIST_SORT_INFO_SECTOR_START;
                    MediaInfoSaveAdd[1] = MediaInfoAddress + ID3ARTIST_ALBUM_SORT_SUB_SECTOR_START;
                    MediaInfoSaveAdd[2] = MediaInfoAddress + ID3ARTIST_SORT_SUB_SECTOR_START;
                    //printf("adddr = %d\n", MediaInfoAddress + ID3ARTIST_SORT_INFO_SECTOR_START);
                    PathDeep = 3;

                    buffer2 = (UINT16*)GerneAblumBuffer;
                    buffer1 = (UINT16*)SingerBuffer;
                    break;


                case ID3_GENRE_TYPE:
                    MediaInfoReadAdd[0] = TRACKID_SAVE_ADDR_OFFSET;
                    MediaInfoReadAdd[1] = ID3_ALBUM_SAVE_ADDR_OFFSET;
                    MediaInfoReadAdd[2] = ID3_SINGLE_SAVE_ADDR_OFFSET;
                    MediaInfoReadAdd[3] = ID3_GENRE_SAVE_ADDR_OFFSET;

                    MediaInfoSaveAdd[0] = MediaInfoAddress + ID3GENRE_SORT_INFO_SECTOR_START;
                    MediaInfoSaveAdd[1] = MediaInfoAddress + ID3GENRE_ALBUM_SORT_SUB_SECTOR_START;
                    MediaInfoSaveAdd[2] = MediaInfoAddress + ID3GENRE_ARTIST_SORT_SUB_SECTOR_START;
                    MediaInfoSaveAdd[3] = MediaInfoAddress + ID3GENRE_SORT_SUB_SECTOR_START;

                    buffer1 = (UINT16*)GerneAblumBuffer;
                    buffer2 = (UINT16*)SingerBuffer;
                    PathDeep = 4;
                    break;

            }
            //开始一类(流派ˇ专辑ˇ歌手ˇ文件名)信息的排序
            //memset(FileIDBuffer,0,4096);
            //memset(GerneAblumBuffer,0,8*1024);
            //memset(SingerBuffer,0,16*1024);
            memset(FileIDBuffer,0,sizeof(FileIDBuffer));           // 4 * SORT_FILE_NAME_BUF_SIZE
            memset(GerneAblumBuffer,0,sizeof(GerneAblumBuffer));   // 8*SORT_FILE_NAME_BUF_SIZE
            memset(SingerBuffer,0,sizeof(SingerBuffer));           // 16*SORT_FILE_NAME_BUF_SIZE


            SortFunction(MediaInfoReadAdd,(uint16 *)&(Sysfilenum->gMusicFileNum),PathDeep,&uiCountTemp[uiSortTypeCount],1,buffer1,buffer2,0, 0, 0);//获取对应的文件信息ˇ并排序ˇ一次只能是一种信息

        }

        Sysfilenum->gID3TitleFileNum =  uiCountTemp[0]; // 得到具有ID3 Title信息的文件个数   uiCountTemp[0] 是所有的音乐文件个数
        Sysfilenum->gID3AlbumFileNum =  uiCountTemp[1]; // 得到具有ID3 Album信息的文件个数
        Sysfilenum->gID3ArtistFileNum = uiCountTemp[2]; // 得到具有ID3 Artist信息的文件个数
        Sysfilenum->gID3GenreFileNum =  uiCountTemp[3]; // 得到具有ID3 Genre信息的文件个数
    }

    if (Sysfilenum->gTotalFileNum != 0)
    {
        buffer1 = (UINT16*)GerneAblumBuffer;
        buffer2 = (UINT16*)SingerBuffer;
        MediaInfoReadAdd[0] = FILE_NAME_SAVE_ADDR_OFFSET;
        MediaInfoReadAdd[1] = ATTR_SAVE_ADDR_OFFSET;
        MediaInfoSaveAdd[0] = MediaInfoAddress + MUSIC_TREE_SORT_INFO_SECTOR_START;
        gFileSortBufferIndex = 0;
        memset(gFileSortBuffer, 0, MEDIAINFO_PAGE_SIZE);

        StartID = 0;
        FileNum = 0;

        //printf("totalnum = %d\n", Sysfilenum->gTotalFileAndDirNum);
        while (StartID < Sysfilenum->gTotalFileNum)
        {
            //MedialibUpdateDisplayHook();//PAGE, tiantian
            memset(FileIDBuffer,0,sizeof(FileIDBuffer));           // 4 * SORT_FILE_NAME_BUF_SIZE
            memset(GerneAblumBuffer,0,sizeof(GerneAblumBuffer));   // 8*SORT_FILE_NAME_BUF_SIZE
            memset(SingerBuffer,0,sizeof(SingerBuffer));           // 16*SORT_FILE_NAME_BUF_SIZE
            PathDeep = 2;
            //printf("StartID = %d\n", StartID);
            SortFunction(MediaInfoReadAdd,&FileNum,PathDeep,&uiCountTemp[0],1,buffer1,buffer2,0, StartID, 1);
            StartID += FileNum;
            FileNum = 0;
        }

        if (gFileSortBufferIndex)
        {
            FW_WriteDataBaseBySector(MediaInfoSaveAdd[0], gFileSortBuffer, MEDIAINFO_PAGE_SIZE/SECTOR_BYTE_SIZE);
        }

    }

    if (Sysfilenum->gRecordFmFileNum != 0)
    {
        buffer1 = (UINT16*)GerneAblumBuffer;
        buffer2 = (UINT16*)SingerBuffer;
        MediaInfoReadAdd[0] = FILE_NAME_SAVE_ADDR_OFFSET;
        MediaInfoReadAdd[1] = ATTR_SAVE_ADDR_OFFSET;
        MediaInfoSaveAdd[0] = MediaInfoAddress + RECORD_TREE_SORT_INFO_SECTOR_START;
        gFileSortBufferIndex = 0;
        memset(gFileSortBuffer, 0, MEDIAINFO_PAGE_SIZE);

        StartID = 0;
        FileNum = 0;

        //printf("totalnum = %d\n", Sysfilenum->gRecordFmFileNum);
        while (StartID < Sysfilenum->gRecordFmFileNum)
        {
            memset(FileIDBuffer,0,sizeof(FileIDBuffer));           // 4 * SORT_FILE_NAME_BUF_SIZE
            memset(GerneAblumBuffer,0,sizeof(GerneAblumBuffer));   // 8*SORT_FILE_NAME_BUF_SIZE
            memset(SingerBuffer,0,sizeof(SingerBuffer));           // 16*SORT_FILE_NAME_BUF_SIZE
            PathDeep = 2;
            //printf("StartID = %d\n", StartID);
            SortFunction(MediaInfoReadAdd,&FileNum,PathDeep,&uiCountTemp[0],1,buffer1,buffer2,0, StartID, 2);
            StartID += FileNum;
            //printf("FileNum = %d\n", FileNum);
            FileNum = 0;
        }

        if (gFileSortBufferIndex)
        {
            FW_WriteDataBaseBySector(MediaInfoSaveAdd[0], gFileSortBuffer, MEDIAINFO_PAGE_SIZE/SECTOR_BYTE_SIZE);
        }

    }

#ifdef PIC_MEDIA

    if (Sysfilenum->gJpegFileNum/*gJpegTotalFileNum*/ != 0)
    {
        buffer1 = (UINT16*)GerneAblumBuffer;
        buffer2 = (UINT16*)SingerBuffer;
        MediaInfoReadAdd[0] = FILE_NAME_SAVE_ADDR_OFFSET;
        MediaInfoReadAdd[1] = ATTR_SAVE_ADDR_OFFSET;
        MediaInfoSaveAdd[0] = MediaInfoAddress + JPEG_TREE_ALL_SORT_INFO_SECTOR_START;
        gFileSortBufferIndex = 0;
        memset(gFileSortBuffer, 0, MEDIAINFO_PAGE_SIZE);

        StartID = 0;
        FileNum = Sysfilenum->gJpegFileNum;
        while (StartID < Sysfilenum->gJpegFileNum/*gJpegTotalFileNum*/)
        {
            memset(FileIDBuffer,0,sizeof(FileIDBuffer));           // 4 * SORT_FILE_NAME_BUF_SIZE
            memset(GerneAblumBuffer,0,sizeof(GerneAblumBuffer));   // 8*SORT_FILE_NAME_BUF_SIZE
            memset(SingerBuffer,0,sizeof(SingerBuffer));           // 16*SORT_FILE_NAME_BUF_SIZE
            PathDeep = 1;
            SortFunction(MediaInfoReadAdd,&FileNum,PathDeep,&uiCountTemp[0],1,buffer1,buffer2,0, StartID, 3);

            StartID += FileNum;
            FileNum = 0;
        }

        if (gFileSortBufferIndex)
        {
            FW_WriteDataBaseBySector(MediaInfoSaveAdd[0], gFileSortBuffer, MEDIAINFO_PAGE_SIZE/SECTOR_BYTE_SIZE);
        }

    }
    //
    // folder
    //

       buffer1 = (UINT16*)GerneAblumBuffer;
       buffer2 = (UINT16*)SingerBuffer;
       MediaInfoReadAdd[0] = FILE_NAME_SAVE_ADDR_OFFSET;
       MediaInfoReadAdd[1] = ATTR_SAVE_ADDR_OFFSET;
       MediaInfoSaveAdd[0] = MediaInfoAddress + JPEG_TREE_SORT_INFO_SECTOR_START;
       gFileSortBufferIndex = 0;
       memset(gFileSortBuffer, 0, MEDIAINFO_PAGE_SIZE);

       StartID = 0;
       while(StartID < Sysfilenum->gJpegTotalFileNum)
       {
            memset(FileIDBuffer,0,sizeof(FileIDBuffer));           // 4 * SORT_FILE_NAME_BUF_SIZE
            memset(GerneAblumBuffer,0,sizeof(GerneAblumBuffer));   // 8*SORT_FILE_NAME_BUF_SIZE
            memset(SingerBuffer,0,sizeof(SingerBuffer));           // 16*SORT_FILE_NAME_BUF_SIZE
            PathDeep = 2;
            SortFunction(MediaInfoReadAdd,&FileNum,PathDeep,&uiCountTemp[0],1,buffer1,buffer2,0, StartID, 4);

            StartID += FileNum;
            FileNum = 0;
       }

       if(gFileSortBufferIndex)
       {
           FW_WriteDataBaseBySector(MediaInfoSaveAdd[0], gFileSortBuffer, MEDIAINFO_PAGE_SIZE/SECTOR_BYTE_SIZE);
       }

#endif

#ifdef _M3U_
    if(Sysfilenum->gM3uFileNum != 0)
    {
        buffer1 = (UINT16*)GerneAblumBuffer;
        buffer2 = (UINT16*)SingerBuffer;
        MediaInfoReadAdd[0] = FILE_NAME_SAVE_ADDR_OFFSET;
        MediaInfoReadAdd[1] = ATTR_SAVE_ADDR_OFFSET;
        MediaInfoSaveAdd[0] = MediaInfoAddress + M3U_TREE_SORT_INFO_SECTOR_START;
        gFileSortBufferIndex = 0;
        memset(gFileSortBuffer, 0, MEDIAINFO_PAGE_SIZE);

        StartID = 0;
        FileNum = Sysfilenum->gM3uFileNum;
        while(StartID < Sysfilenum->gM3uFileNum)
        {
            memset(FileIDBuffer,0,sizeof(FileIDBuffer));           // 4 * SORT_FILE_NAME_BUF_SIZE
            memset(GerneAblumBuffer,0,sizeof(GerneAblumBuffer));   // 8*SORT_FILE_NAME_BUF_SIZE
            memset(SingerBuffer,0,sizeof(SingerBuffer));           // 16*SORT_FILE_NAME_BUF_SIZE
            PathDeep = 1;
            SortFunction(MediaInfoReadAdd,&FileNum,PathDeep,&uiCountTemp[0],1,buffer1,buffer2,0, StartID, 5);

            StartID += FileNum;
            FileNum = 0;
        }

        if(gFileSortBufferIndex)
        {
            FW_WriteDataBaseBySector(MediaInfoSaveAdd[0], gFileSortBuffer, MEDIAINFO_PAGE_SIZE/SECTOR_BYTE_SIZE);
        }
   }
#endif
}
#endif

/*
********************************************************************************
*
*                         End of FileInfoSort.c
********************************************************************************
*/
