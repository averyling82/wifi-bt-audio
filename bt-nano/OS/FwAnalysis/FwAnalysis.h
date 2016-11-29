/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: OS\FwAnalysis\FwAnalysis.h
* Owner: aaron.sun
* Date: 2015.10.28
* Time: 15:30:25
* Version: 1.0
* Desc: Firmwave driver
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2015.10.28     15:30:25   1.0
********************************************************************************************
*/


#ifndef __OS_FWANALYSIS_FWANALYSIS_H__
#define __OS_FWANALYSIS_FWANALYSIS_H__
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "ModuleInfoTab.h"


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/

#define _OS_FWANALYSIS_FWANALYSIS_COMMON_  __attribute__((section("os_fwanalysis_fwanalysis_common")))
#define _OS_FWANALYSIS_FWANALYSIS_INIT_  __attribute__((section("os_fwanalysis_fwanalysis_init")))
#define _OS_FWANALYSIS_FWANALYSIS_SHELL_  __attribute__((section("os_fwanalysis_fwanalysis_shell")))

#define     CHIP_SIGN           0x4F4E414E                  //NANO
#define     FLASH_MAGIC         0x4E414E44
#define     FLASH_INFO_VER      0x100

#ifdef _SPI_BOOT_
#define FW_IDB_NUM              1
#define FW_IDB_SIZE             (128)
#define FW_BOOT_SIZE            (128)   //64K
#define FW_SYS_OFFSET           FW_BOOT_SIZE
#define FW_BOOT_OFFSET          (0)
#define FW_ALIGN_SIZE           (64*1024) //must be multiple 64KB size
#endif

#ifdef _EMMC_BOOT_
#define  FW_IDB_NUM             5
#define  FW_IDB_SIZE            (512*1024/512)
#define  FW_BOOT_SIZE           (4*1024*1024/512)   //BOOT分区大小
#define  FW_SYS_OFFSET          FW_BOOT_SIZE      //固件从4M偏移开始
#define  FW_BOOT_OFFSET         (64)                //IDB 从32K偏移开始
#define FW_ALIGN_SIZE           (1024*1024) //must be 1MB size
#endif

//SegmentLoad Type define
#define SEGMENT_OVERLAY_CODE         0x01
#define SEGMENT_OVERLAY_DATA         0x02
#define SEGMENT_OVERLAY_BSS          0x04
#define SEGMENT_OVERLAY_ALL          (SEGMENT_OVERLAY_CODE | SEGMENT_OVERLAY_DATA | SEGMENT_OVERLAY_BSS)

#define MAX_CHAR_ENC_SUPPORT    5
#define MAX_FONT_SUPPORT        10
#define MAX_MENU_SUPPORT        10
#define MAX_UI_SUPPORT          10

//-----------HJL----------------
#if defined(__arm__) && defined(__ARMCC_VERSION)
#define PACK_STRUCT_BEGIN __packed
#define PACK_STRUCT_STRUCT
#define PACK_STRUCT_END
#define PACK_STRUCT_FIELD(x) x
#elif defined(__arm__) && defined(__GNUC__)
#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_STRUCT  __attribute__ ((__packed__))
#define PACK_STRUCT_END
#define PACK_STRUCT_FIELD(x) x
#else
#error Unsupported tools.
#endif

typedef enum _ENUM_CODE
{
    USB_BOOT,
    NAND_BOOT1,
    NAND_BOOT2,
    SYS_CODE,
    CODE_NUM

}ENUM_CODE;

typedef enum _ENUM_FONT
{
    FONT12,
    FONT16,
    FONT_NUM

}ENUM_FONT;

typedef enum _ENUM_CHAR_ENC
{
    GBK,
    CHAR_ENC_NUM
}ENUM_CHAR_ENC;

typedef enum _ENUM_UI
{
    UI1,
    UI_NUM
}ENUM_UI;


/***********模块*********/
typedef struct _MODULE_INFO
{

    unsigned int ModuleOffset;
    unsigned int ModuleLength;

} MODULE_INFO;


/***********代码信息***********/
typedef struct _CODE_INFO
{

    unsigned int    CodeNum;
    MODULE_INFO   CodeInfoTbl[CODE_NUM];

} CODE_INFO;

/*************字体**************/
typedef struct _FONT_INFO
{

    unsigned int    FontNum;
    MODULE_INFO   FontInfoTbl[MAX_FONT_SUPPORT];

} FONT_INFO;

/***********字符编码************/
typedef struct _CHAR_ENC_INFO
{

    unsigned int    CharEncNum;
    MODULE_INFO   CharEncInfoTbl[MAX_CHAR_ENC_SUPPORT];

} CHAR_ENC_INFO;

/************菜单***************/
typedef struct _MENU_INFO
{

    unsigned int    MenuNum;
    MODULE_INFO   MenuInfoTbl[MAX_MENU_SUPPORT];

} MENU_INFO;

/*************UI****************/
typedef struct _UI_INFO
{

    unsigned int    UiNum;
    MODULE_INFO   UiInfoTbl[MAX_UI_SUPPORT];

} UI_INFO;

/*ID BLOCK SECTOR 0 INFO*/
typedef PACK_STRUCT_BEGIN struct tagIDSEC0
{
    uint32  magic;              //0x0ff0aa55, MASKROM限定不能更改
    uint8   reserved[56-4];
    uint32  nandboot2offset;
    uint32  nandboot2len;
    uint32  nandboot1offset1;
    uint32  nandboot1len1;
    uint32  nandboot1offset2;
    uint32  nandboot1len2;
    uint8   CRC[16];
    uint8   reserved1[512-96];

}PACK_STRUCT_STRUCT IDSEC0, *pIDSEC0;
PACK_STRUCT_END

/*ID BLOCK SECTOR 1 INFO*/
typedef PACK_STRUCT_BEGIN struct tagIDSEC1
{
    uint16  SysAreaBlockRaw;        //系统保留块, 以原始Block为单位
    uint16  SysProgDiskCapacity;    //系统固件盘容量, 以MB为单位
    uint16  SysDataDiskCapacity;
    uint16  Disk2Size;
    uint16  Disk3Size;
    uint32  ChipTag;
    uint32  MachineId;
    uint16  LoaderYear;
    uint16  LoaderDate;
    uint16  LoaderVer;
    uint16  FirmMasterVer;      // (24-25)  BCD编码 固件主版本
    uint16  FirmSlaveVer;           // (26-27)  BCD编码 固件从版本
    uint16  FirmSmallVer;           // (28-29)  BCD编码 固件子版本
    uint16  LastLoaderVer;
    uint16  UpgradeTimes;
    uint16  MachineInfoLen;
    uint8   MachineInfo[30];
    uint16  ManufactoryInfoLen;
    uint8   ManufactoryInfo[30];
    uint16  FlashInfoOffset;
    uint16  FlashInfoLen;
    uint8   Reserved2[382];             // (102-483)保留
    uint32  FlashSize;                  //以sector为单位
    uint16  BlockSize;                  //以SEC表示的BLOCK SIZE
    uint8   PageSize;                  //以SEC表示的PAGE SIZE
    uint8   ECCBits;
    uint8   AccessTime;                //读写cycle时间, ns
    uint8   reserved2[5];
    uint16  FirstReservedBlock;     // (498-499)Idblock之后第一个保留块的的位置     (单位:block)
    uint16  LastReservedBlock;      // (500-501)Idblock之后最后一个保留块的的位置   (单位:block)
    uint16  IdBlock0;
    uint16  IdBlock1;
    uint16  IdBlock2;
    uint16  IdBlock3;
    uint16  IdBlock4;
}PACK_STRUCT_STRUCT IDSEC1, *pIDSEC1;
PACK_STRUCT_END

/***********RESOURCE_ADDR**********/
typedef struct _FIRMWARE_INF
{
    //Firmware Info addr
    uint32 Font12LogicAddress;     //char font 12
    uint32 Font16LogicAddress;     //char font 16
    uint32 IDBlockByteAddress;     //reserved
    uint32 CodeLogicAddress;       // code
    uint32 FontLogicAddress;       //char font 12/16
    uint32 GBKLogicAddress;        //GBK
    uint32 MenuLogicAddress;       //Ment
    uint32 ImageLogicAddress;      //Image
    uint32 LoadStartBase;
    uint32 SystemDefaultParaAddress;  //defalut para
    uint16 FirmwareYear;
    uint16 FirmwareDate;
    uint16 MasterVersion;
    uint16 SlaveVersion;
    uint16 SmallVersion;


}FIRMWARE_INF, *PFIRMWARE_INF;

/***********FIRMWARE_HEADER**********/
typedef struct _FIRMWARE_HEADER
{
    unsigned short  Year;               // BCD
    unsigned short  Date;               // BCD
    unsigned short  MasterVer;          // BCD
    unsigned short  SlaveVer;           // BCD
    unsigned short  SmallVer;           // BCD
    unsigned short  LoaderVer;          // BCD
    unsigned char   MachineID[4];       // 机器ID,与固件匹配
    unsigned char   VendorName[32];     // 厂商名
    unsigned char   ModelName[32];      // 型号名
    CODE_INFO       CodeInfo;           // 代码信息，包括UsbBoot, NandBoot1, NandBoot2, SysCode
    FONT_INFO       FontInfo;           // 字库信息
    CHAR_ENC_INFO   CharEncInfo;        // 字符编码信息，目前仅包含GBK编码
    MENU_INFO       MenuInfo;           // 菜单资源信息
    UI_INFO         UiInfo;             // UI资源信息
    unsigned char   Reserved[88];
    unsigned long   FwEndOffset;
    unsigned char   FwSign[8];          // 固件标记，"RkNanoFw"

}FIRMWARE_HEADER, *PFIRMWARE_HEADER;

typedef struct PictureInfoStruct
{
    UINT16  xSize;
    UINT16  ySize;
    UINT16  x;
    UINT16  y;
    UINT32  totalSize;                          /* it equal with xSize multiply ySize */
    UINT32  offsetAddr;                         /* the offset address that picture saved in flash */

} PICTURE_INFO_STRUCT;

/*menu text resource information structure,the order of members cannot be changed. */
typedef struct MenuTextInfoStruct
{
    UINT16  SelfID;                             /* menu item id */
    UINT16  FatherID;                           /* father menu id*/
    UINT16  CurItemInFatherID;                  /* the serial number in father menu */
    UINT16  ChildID ;                           /* frist menu item id */
    UINT16  PrevID;                             /* left brother menu item id.*/
    UINT16  NextID;                             /* right brother menu item id.*/
    UINT16  FunctionIndex;                      /* index of implement function.*/
    UINT16  ChildNum;                           /* total number of child items.*/
    UINT16  ChildIDArray[21];                   /* the id groud of child menu items*/

} MENU_TEXT_INFO_STRUCT;


typedef  struct _D_SEGMENT_INFO
{
    struct _D_SEGMENT_INFO * pstDsegment;
    uint32 SegmentID;
    uint32 CodeLoadBase;
    uint32 CodeImageBase;
    uint32 CodeImageLen;
    uint32 DataLoadBase;
    uint32 DataImageBase;
    uint32 DataImageLen;
    uint32 BssImageBase;
    uint32 BssImageLen;
    uint32 LoadCnt;

}D_SEGMENT_INFO;

typedef struct _DATABASE_INFO
{
    uint32  capacity;           //(Sector为单位)  4Byte
    uint32  BlockSize;          //(Sector为单位)  2Byte
    uint32  PageSize;           //(Sector为单位)  1Byte

} DATABASE_INFO;

typedef enum
{
#ifdef _USE_CODEPAGE_CP874_
    FONT_CODEPAGE_CP874 = 0 ,
#endif

#ifdef _USE_CODEPAGE_CP932_
    FONT_CODEPAGE_CP932 ,
#endif

#ifdef _USE_CODEPAGE_CP936_
    FONT_CODEPAGE_CP936 ,
#endif

#ifdef _USE_CODEPAGE_CP949_
    FONT_CODEPAGE_CP949 ,
#endif

#ifdef _USE_CODEPAGE_CP950_
    FONT_CODEPAGE_CP950 ,
#endif

#ifdef _USE_CODEPAGE_CP1250_
    FONT_CODEPAGE_CP1250 ,
#endif

#ifdef _USE_CODEPAGE_CP1251_
    FONT_CODEPAGE_CP1251 ,
#endif

#ifdef _USE_CODEPAGE_CP1252_
    FONT_CODEPAGE_CP1252 ,
#endif

#ifdef _USE_CODEPAGE_CP1253_
    FONT_CODEPAGE_CP1253 ,
#endif

#ifdef _USE_CODEPAGE_CP1254_
    FONT_CODEPAGE_CP1254 ,
#endif

#ifdef _USE_CODEPAGE_CP1255_
    FONT_CODEPAGE_CP1255 ,
#endif

#ifdef _USE_CODEPAGE_CP1256_
    FONT_CODEPAGE_CP1256 ,
#endif

#ifdef _USE_CODEPAGE_CP1257_
    FONT_CODEPAGE_CP1257 ,
#endif

#ifdef _USE_CODEPAGE_CP1258_
    FONT_CODEPAGE_CP1258 ,
#endif

#ifdef _USE_CODEPAGE_CP874_
    U2C_CODEPAGE_CP874 ,
#endif

#ifdef _USE_CODEPAGE_CP932_
    U2C_CODEPAGE_CP932 ,
#endif

#ifdef _USE_CODEPAGE_CP936_
    U2C_CODEPAGE_CP936 ,
#endif

#ifdef _USE_CODEPAGE_CP949_
    U2C_CODEPAGE_CP949 ,
#endif

#ifdef _USE_CODEPAGE_CP950_
    U2C_CODEPAGE_CP950 ,
#endif

#ifdef _USE_CODEPAGE_CP1250_
    U2C_CODEPAGE_CP1250 ,
#endif

#ifdef _USE_CODEPAGE_CP1251_
    U2C_CODEPAGE_CP1251 ,
#endif

#ifdef _USE_CODEPAGE_CP1252_
    U2C_CODEPAGE_CP1252 ,
#endif

#ifdef _USE_CODEPAGE_CP1253_
    U2C_CODEPAGE_CP1253 ,
#endif

#ifdef _USE_CODEPAGE_CP1254_
    U2C_CODEPAGE_CP1254 ,
#endif

#ifdef _USE_CODEPAGE_CP1255_
    U2C_CODEPAGE_CP1255 ,
#endif

#ifdef _USE_CODEPAGE_CP1256_
    U2C_CODEPAGE_CP1256 ,
#endif

#ifdef _USE_CODEPAGE_CP1257_
    U2C_CODEPAGE_CP1257 ,
#endif

#ifdef _USE_CODEPAGE_CP1258_
    U2C_CODEPAGE_CP1258 ,
#endif

    FONT_CODEPAGE_MAX_COUNT ,
} FONT_CODEPAGE_TYPE;
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
extern D_SEGMENT_INFO * gpstDsegment;
extern FIRMWARE_INF gstFwInf;
extern uint16  UpgradeTimes;
extern uint16  LoaderVer;
extern uint32  SysProgDiskCapacity;
extern uint32  SysProgRawDiskCapacity;
extern int32  FW1Valid, FW2Valid;
extern uint32 FwSysOffset;

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API Declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
extern rk_err_t FW_CodePageDeInit(void);
extern rk_err_t FwResume(void);
extern rk_err_t FwSuspend(void);
extern rk_err_t DBResume(void);
extern rk_err_t DBSuspend(void);
extern rk_err_t FW_DBInit(void);
extern rk_err_t FW_CodePageLoad(void);
extern rk_err_t FW_CodePageUnLoad(void);
extern rk_err_t FW_GetProductSn(void *pSn);
extern void FW_Resource_DeInit(void);
extern void FW_GetDBInf(DATABASE_INFO * pstDataBaseInf);
extern rk_err_t FW_ReadDataBaseByByte(uint32 Addr, uint8 *pData, uint32 length);
extern rk_err_t FW_ReadDataBaseBySector(uint32 LBA, uint8 * buf,  uint32 SecCnt);
extern rk_err_t FW_WriteDataBaseBySector(uint32 LBA, uint8 * buf, uint32 SecCnt);
extern void FW_Resource_Init(void);
extern rk_err_t FW_GetMenuInfoWithIDNum(UINT32 menuTextID,MENU_TEXT_INFO_STRUCT *pMenuTextInfo);
extern rk_err_t FW_GetPicResource(uint32 FlashAddr, uint8 *pData, uint16 length);
extern rk_err_t FW_GetMenuResource(UINT16 menuTextID , UINT16 *pMenuStr ,UINT16 StrLen);
extern rk_err_t FW_RemoveSegment(uint32 SegmentID);
extern rk_err_t FW_LoadSegment(uint32 SegmentID, uint8 Type);
extern rk_err_t FW_ReadFirmwaveByByte(uint32 Addr, uint8 *pData, uint32 length);
extern rk_err_t FW_GetSegmentInfo(uint32 SegmentID, SEGMENT_INFO_T * Segment);
extern rk_err_t FWShell(HDC dev,  uint8 * pstr);
extern rk_err_t FW_CodePageInit(void);
extern rk_err_t FW_Ansi2UnicodeStr(void * ansi, uint16 * unicode, uint8 EncodeMode);
extern uint32 FW_Ansi2UnicodeN(void *ansi, uint16 * unicode,int byteN, uint8 EncodeMode);
extern uint16 FW_Ansi2Unicode(uint16 ansi, uint8 EncodeMode);
extern uint16 FW_Unicode2Ansi(uint16 unicode, uint8 EncodeMode);
extern rk_err_t FW_Unicode2AnsiStr(uint16 *unicode, void * ansi,uint8 EncodeMode);
extern uint32 FW_Unicode2AnsiN(uint16 *unicode, void * ansi,int byteN, uint8 EncodeMode);
extern rk_err_t FW_GetOTAParameter(uint8 *product_cu,uint8 *product_vc,uint8 *product_sn,uint8 *product_id);

#endif
