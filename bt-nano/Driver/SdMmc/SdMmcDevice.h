/*
********************************************************************************************
*
*              Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                                  All rights reserved.
*
* FileName: ..\Driver\SdMmc\SdMmcDevice.h
* Owner: Aaron.sun
* Date: 2014.2.19
* Time: 13:39:43
* Desc: sd/mmc control device class
* History:
*     <author>     <date>       <time>     <version>       <Desc>
* Aaron.sun     2014.2.19     13:39:43   1.0
********************************************************************************************
*/

#ifndef __DRIVER_SDMMC_SDMMCDEVICE_H__
#define __DRIVER_SDMMC_SDMMCDEVICE_H__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#define _DRIVER_SDMMC_SDMMCDEVICE_COMMON_  __attribute__((section("driver_sdmmc_sdmmcdevice_common")))
#define _DRIVER_SDMMC_SDMMCDEVICE_INIT_  __attribute__((section("driver_sdmmc_sdmmcdevice_init")))
#define _DRIVER_SDMMC_SDMMCDEVICE_SHELL_  __attribute__((section("driver_sdmmc_sdmmcdevice_shell")))
#if defined(__arm__) && (__ARMCC_VERSION)
#define _DRIVER_SDMMC_SDMMCDEVICE_SHELL_DATA_      _DRIVER_SDMMC_SDMMCDEVICE_SHELL_
#elif defined(__arm__) && (__GNUC__)
#define _DRIVER_SDMMC_SDMMCDEVICE_SHELL_DATA_      __attribute__((section("driver_sdmmc_sdmmcdevice_shell_data")))
#else
#error Unknown compiling tools.
#endif

/*------------------------------ Global Defines ------------------------------*/
/* Class 0 (basic command) */
#define SD_CMD0    (0)    // [31:0] stuff bits
#define SD_CMD1    (1)    // [31:0] stuff, MMC
#define SD_CMD2    (2)    // [31:0] stuff
#define SD_CMD3    (3)    // [31:0] stuff
#define SD_CMD4    (4)    // [31:16] DSR [15:0] stuff
#define SD_CMD5    (5)    // [31:24] stuff [23:0] I/O OCR, SDIO
#define SD_CMD6    (6)    // ACMD6:[31:2] stuff [1:0] bus width
// SD2.0 CMD6
// [31] Mode 0:Check function, 1:Switch function
// [30:24] reserved
// [23:20] reserved for function group 6
// [19:16] reserved for function group 5
// [15:12] reserved for function group 4
// [11:8]  reserved for function group 3
// [7:4]   function group 2 for command system
// [3:0]   function group 1 for access mode
// MMC4.0 CMD6
// [31:26] Set to 0
// [25:24] Access
// [23:16] Index
// [15:8] Value
// [7:3] Set to 0
// [2:0] Cmd Set
#define SD_CMD7    (7)    // [31:16] RCA [15:0] stuff
#define SD_CMD8    (8)    // SD 2.0
// [31:12] reserved [11:8] supply voltage(VHS) [7:0] check pattern
// MMC4.0
// [31:0] stuff bits
#define SD_CMD9    (9)    // [31:16] RCA [15:0] stuff
#define SD_CMD10   (10)   // [31:16] RCA [15:0] stuff
#define SD_CMD11   (11)
#define SD_CMD12   (12)   // [31:0] stuff
#define SD_CMD13   (13)   // [31:16] RCA [15:0] stuff, ACMD13:[31:0] stuff
#define SD_CMD14   (14)   // MMC4.0
// [31:0] stuff bits
#define SD_CMD15   (15)   // [31:16] RCA [15:0] stuff
/* Class 2 */
#define SD_CMD16   (16)   // [31:0] block length
#define SD_CMD17   (17)   // [31:0] data address
#define SD_CMD18   (18)   // [31:0] data address
#define SD_CMD19   (19)   // MMC4.0
// [31:0] stuff bits
#define SD_CMD20   (20)
#define SD_CMD21   (21)
#define SD_CMD22   (22)   // ACMD22:[31:0] stuff
#define SD_CMD23   (23)   // [31:16] stuff [15:0] number of blocks, MMC
// ACMD23:[31:23] stuff [22:0] Number of blocks
/* Class 4 */
#define SD_CMD24   (24)   // [31:0] data address
#define SD_CMD25   (25)   // [31:0] data address
#define SD_CMD26   (26)
#define SD_CMD27   (27)   // [31:0] stuff
/* Class 6 */
#define SD_CMD28   (28)   // [31:0] data address
#define SD_CMD29   (29)   // [31:0] data address
#define SD_CMD30   (30)   // [31:0] write protect data address
#define SD_CMD31   (31)
/* Class 5 */
#define SD_CMD32   (32)   // [31:0] data address
#define SD_CMD33   (33)   // [31:0] data address
#define SD_CMD34   (34)
#define SD_CMD35   (35)   // [31:0] data address,MMC
#define SD_CMD36   (36)   // [31:0] data address,MMC
#define SD_CMD37   (37)
#define SD_CMD38   (38)   // [31:0] stuff
#define SD_CMD39   (39)
#define SD_CMD40   (40)
#define SD_CMD41   (41)   // ACMD41:[31:0] OCR without busy,
// ACMD41:[31] reserved [30] HCS(OCR[30]) [29:24] reserved [23:0] Vdd Voltage window(OCR[23:0]), SD2.0
/* Class 7 */
#define SD_CMD42   (42)   // [31:0] stuff, ACMD42:[31:1] stuff [0] set_cd
#define SD_CMD43   (43)
#define SD_CMD44   (44)
#define SD_CMD45   (45)
#define SD_CMD46   (46)
#define SD_CMD47   (47)
#define SD_CMD48   (48)
#define SD_CMD49   (49)
#define SD_CMD50   (50)
#define SD_CMD51   (51)   // ACMD51:[31:0] stuff
#define SD_CMD52   (52)   // [31] R/W flag [30:28] Function Number [27] RAW flag [26] stuff [25:9] Register Address [8] stuff [7:0] Write Data or stuff, SDIO
#define SD_CMD53   (53)   // [31] R/W flag [30:28] Function Number [27] Block Mode [26] OP Code [25:9] Register Address [8:0] Byte/Block Count
#define SD_CMD54   (54)
/* Class 8 */
#define SD_CMD55   (55)   // [31:16] RCA [15:0] stuff
#define SD_CMD56   (56)   // [31:1] stuff [0] RD/WRn
#define SD_CMD57   (57)
#define SD_CMD58   (58)
#define SD_CMD59   (59)   // [31:1] stuff [0] CRC option,MMC
#define SD_CMD60   (60)
#define SD_CMD61   (61)
#define SD_CMD62   (62)
#define SD_CMD63   (63)

/* SD 1.0 command */
/* Class 0 */
#define SD_GO_IDLE_STATE        SD_CMD0
#define SD_ALL_SEND_CID         SD_CMD2
#define SD_SEND_RELATIVE_ADDR   SD_CMD3
#define SD_SET_DSR              SD_CMD4
#define SD_SELECT_DESELECT_CARD SD_CMD7
#define SD_SEND_CSD             SD_CMD9
#define SD_SEND_CID             SD_CMD10
#define SD_VOLT_SWITCH          SD_CMD11

#define SD_STOP_TRANSMISSION    SD_CMD12
#define SD_SEND_STATUS          SD_CMD13
#define SD_GO_INACTIVE_STATE    SD_CMD15
/* Class 2 */
#define SD_SET_BLOCKLEN         SD_CMD16
#define SD_READ_SINGLE_BLOCK    SD_CMD17
#define SD_READ_MULTIPLE_BLOCK  SD_CMD18
#define SD_SET_BLOCKCNT         SD_CMD23

/* Class 4 */
#define SD_WRITE_BLOCK          SD_CMD24
#define SD_WRITE_MULTIPLE_BLOCK SD_CMD25
#define SD_PROGRAM_CSD          SD_CMD27
/* Class 6 */
#define SD_SET_WRITE_PROT       SD_CMD28
#define SD_CLR_WRITE_PROT       SD_CMD29
#define SD_SEND_WRITE_PROT      SD_CMD30
/* Class 5 */
#define SD_ERASE_WR_BLK_START   SD_CMD32
#define SD_ERASE_WR_BLK_END     SD_CMD33
#define SD_ERASE                SD_CMD38
/* Class 7 */
#define SD_LOCK_UNLOCK          SD_CMD42
/* Class 8 */
#define SD_APP_CMD              SD_CMD55
#define SD_GEN_CMD              SD_CMD56
/* Application cmd */
#define SDA_SET_BUS_WIDTH       SD_CMD6
#define SDA_SD_STATUS           SD_CMD13
#define SDA_SEND_NUM_WR_BLOCKS  SD_CMD22
#define SDA_SET_WR_BLK_ERASE_COUNT SD_CMD23
#define SDA_SD_APP_OP_COND      SD_CMD41
#define SDA_SET_CLR_CARD_DETECT SD_CMD42
#define SDA_SEND_SCR            SD_CMD51

/* SD 2.0 addition command */
#define SD2_SEND_IF_COND        SD_CMD8
#define SD2_SWITCH_FUNC         SD_CMD6

/* SDIO addition command */
#define SDIO_IO_SEND_OP_COND    SD_CMD5
#define SDIO_IO_RW_DIRECT       SD_CMD52
#define SDIO_IO_RW_EXTENDED     SD_CMD53

/* MMC addition command */
#define MMC_SEND_OP_COND        SD_CMD1
#define MMC_SET_RELATIVE_ADDR   SD_CMD3

/* MMC4.0 addition command */
#define MMC4_SWITCH_FUNC        SD_CMD6
#define MMC4_SEND_EXT_CSD       SD_CMD8
#define MMC4_BUSTEST_R          SD_CMD14
#define MMC4_BUSTEST_W          SD_CMD19


/* Command Registe bit */  //直接把Command Register的位定义拿过来当command flag，这样控制器就不用再分析flag，而可以直接把flag设到寄存器中
#define BLOCK_TRANS             (0 << 11)    //block data transfer command
#define STREAM_TRANS            (1 << 11)    //stream data transfer command
#define READ_CARD               (0 << 10)    //read from card
#define WRITE_CARD              (1 << 10)    //write to card
#define NOCARE_RW               (0 << 10)    //not care read or write
#define NO_DATA_EXPECT          (0 << 9)     //no data transfer expected
#define DATA_EXPECT             (1 << 9)     //data transfer expected
#define NO_CHECK_R_CRC          (0 << 8)     //do not check response crc
#define CHECK_R_CRC             (1 << 8)     //check response crc
#define NOCARE_R_CRC            CHECK_R_CRC  //not care response crc
#define SHORT_R                 (0 << 7)     //short response expected from card
#define LONG_R                  (1 << 7)     //long response expected from card
#define NOCARE_R                SHORT_R      //not care response length
#define NO_R_EXPECT             (0 << 6)     //no response expected from card
#define R_EXPECT                (1 << 6)     //response expected from card
#define RSP_BUSY                (1 << 16)    // card may send busy，这是我们自己加的一位，用于R1b的，寄存器并没有这位，因此硬件控制器使用时必须把这位清掉


/* command flag */
#define SD_RSP_NONE             (NOCARE_R_CRC | NOCARE_R | NO_R_EXPECT)
#define SD_RSP_R1               (CHECK_R_CRC | SHORT_R | R_EXPECT)
#define SD_RSP_R1B              (CHECK_R_CRC | SHORT_R | R_EXPECT | RSP_BUSY)
#define SD_RSP_R2               (CHECK_R_CRC | LONG_R | R_EXPECT)
#define SD_RSP_R3               (NO_CHECK_R_CRC | SHORT_R | R_EXPECT)
#define SD_RSP_R4               (NO_CHECK_R_CRC | SHORT_R | R_EXPECT)
#define SD_RSP_R5               (CHECK_R_CRC | SHORT_R | R_EXPECT)
#define SD_RSP_R6               (CHECK_R_CRC | SHORT_R | R_EXPECT)
#define SD_RSP_R6M              (CHECK_R_CRC | SHORT_R | R_EXPECT)
#define SD_RSP_R7               (CHECK_R_CRC | SHORT_R | R_EXPECT)


//读写操作的flag
#define SD_OP_MASK              ((1<<11)|(1<<10)|(1<<9))
#define SD_NODATA_OP            (NOCARE_RW | NO_DATA_EXPECT | BLOCK_TRANS)
#define SD_READ_OP              (READ_CARD | DATA_EXPECT | BLOCK_TRANS)
#define SD_WRITE_OP             (WRITE_CARD | DATA_EXPECT | BLOCK_TRANS)

typedef enum _SDC_DEVICE_CONTROL_CMD
{
    SDC_DEVICE_SET_MODE,
    SDC_DEVICE_SET_BUS_FREQ,
    SDC_DEVICE_SET_BUS_WIDTH,
    SDC_DEVICE_CMD_REQ,
    SDC_DEVICE_NUM

}SDC_DEVICE_CONTROL_CMD;


typedef struct _SDC_CMD_ARG
{
    uint32  Cmd;
    uint32  CmdArg;
    uint32 *RespBuf;
    uint32  BlkSize;
    uint8  *Buf;
    uint32  Size;

}SDC_CMD_ARG;



typedef  struct _SDC_DEV_ARG
{
    HDC hDma;
    uint32 Channel;

}SDC_DEV_ARG;

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
extern void SdcIntIRQ1(void);
extern void SdcIntIRQ0(void);
extern rk_err_t SdcDev_SetPower(HDC dev, BOOL enable);
extern rk_err_t SdcDev_DataTransIdle(HDC dev);
extern rk_err_t SdcDev_SetBusWidth(HDC dev, uint32 BusWidth);
extern rk_err_t SdcDev_SetBusFreq(HDC dev, uint32 BusFreq);
extern rk_err_t SdcDev_SendCmd(HDC dev, SDC_CMD_ARG * pstCmdArg);
extern HDC SdcDev_Create(uint32 DevID, void *arg);
extern rk_err_t SdcDevDelete(uint32 DevID, void * arg);
extern rk_err_t SdcDev_Shell(HDC dev, uint8 * pstr);
extern rk_size_t SdcDev_Read(HDC dev, SDC_CMD_ARG * pstCmdArg);
extern rk_size_t SdcDev_Write(HDC dev, SDC_CMD_ARG * pstCmdArg);
extern rk_err_t SdcRegisterIoIntIsr(HDC dev, void(*isr_hook)(HDC hSdc, void *arg));

#endif

