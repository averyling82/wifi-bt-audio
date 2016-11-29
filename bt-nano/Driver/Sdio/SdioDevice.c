/*
********************************************************************************************
*
*          Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Driver\Sdio\SdioDevice.c
* Owner: Aaron.sun
* Date: 2014.6.14
* Time: 9:10:31
* Desc: sdio device class
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    Aaron.sun     2014.6.14     9:10:31   1.0
********************************************************************************************
*/

#include "BspConfig.h"
#ifdef __DRIVER_SDIO_SDIODEVICE_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "RKOS.h"
#include "BSP.h"

#define SDIO_FUNC_MAX 3




/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
typedef struct _SDIO_FUNC_TUPLE
{
    struct _SDIO_FUNC_TUPLE *next;
    uint8 code;
    uint8 size;
    uint8 data[1];

}SDIO_FUNC_TUPLE;


typedef struct _SDIO_FUNC
{
    HDC     hSdio;  /*SDM_CARD_INFO the card this device belongs to */

    sdio_irq_handler_t * irq_handler;    /* IRQ callback */
    uint        max_blksize;    /* maximum block size */
    uint        cur_blksize;    /* current block size */
    uint        enable_timeout;    /* max enable timeout in msec */
    uint        num;        /* function number */
    uint16        vendor;        /* vendor id */
    uint16        device;        /* device id */
    uint        num_info;    /* number of info strings */
    const char    **info;        /* info strings */
    uint8        func_class;        /* standard interface class */
    uint        state;            /* function state */
    uint8        tmpbuf[4];    /* DMA:able scratch buffer */
    void *driver_data;   /* struct if_sdio_card */
    SDIO_FUNC_TUPLE * tuples;


}SDIO_FUNC;


typedef struct _SDIO_CCCR
{
    unsigned int        sdio_vsn;
    unsigned int        sd_vsn;
unsigned int        multi_block:
    1,
low_speed:
    1,
wide_bus:
    1,
high_power:
    1,
high_speed:
    1,
disable_cd:
    1;
}SDIO_CCCR;

typedef struct _SDIO_CIS
{
    unsigned short        vendor;
    unsigned short        device;
    unsigned short        blksize;
    unsigned int        max_dtr;

}SDIO_CIS;


typedef int (tpl_parse_t)(void *, SDIO_FUNC *, const unsigned char *, unsigned);

typedef struct _CIS_TPL
{
    unsigned char code;
    unsigned char min_size;
    tpl_parse_t *parse;

}CIS_TPL;

typedef  struct _SDIO_DEVICE_HOST
{

    pSemaphore osSdioIrqSem;
    uint32       sdio_irqs;
    uint32       devCnt;
}SDIO_DEVICE_HOST;


typedef  struct _SDIO_DEVICE_CLASS
{
    DEVICE_CLASS stSdioDevice;
    pSemaphore osSdioOperReqSem;
    SDIO_DEVICE_HOST *host;
    SDIO_FUNC Func[SDIO_FUNC_MAX];
    SDIO_FUNC *sdio_single_irq; /* SDIO function when only one IRQ active */
    HDC hSdc;
    uint32           type;             //Card type
    uint32           rca;              //Relative Card Address
    uint32           ocr;        /* the current OCR setting */
    SDIO_CCCR        cccr;
    SDIO_CIS         cis;
    SDIO_FUNC_TUPLE * tuples;
    uint32        num_info;    /* number of info strings */
    const uint8        **info;        /* info strings */
    uint32        state;        /* (our) card state */
    uint32       TranSpeed;       //卡的最大数据传输速度，也就是卡的最大工作频率，单位KHz
    uint32       irqcnt;
    uint32      sdio_irqs;
    uint32      sdio_funcs; //number of funcs;
}SDIO_DEVICE_CLASS;



/* SDIO commands                         type  argument     response */
#define SD_IO_SEND_OP_COND         SD_CMD5 /* bcr  [23:0] OCR         R4  */
#define SD_IO_RW_DIRECT            SD_CMD52 /* ac   [31:0] See below   R5  */
#define SD_IO_RW_EXTENDED          SD_CMD53 /* adtc [31:0] See below   R5  */

/*
 * SD_IO_SEND_OP_COND argument format:
 *
 *      [24] S18R
 *      [23:0] I/O OCR
 */
#define SD_IO_OCR_20_21          (1 << 8)
#define SD_IO_OCR_21_22          (1 << 9)
#define SD_IO_OCR_22_23          (1 << 10)
#define SD_IO_OCR_23_24          (1 << 11)
#define SD_IO_OCR_24_25          (1 << 12)
#define SD_IO_OCR_25_26          (1 << 13)
#define SD_IO_OCR_26_27          (1 << 14)
#define SD_IO_OCR_27_28          (1 << 15)
#define SD_IO_OCR_28_29          (1 << 16)
#define SD_IO_OCR_29_30          (1 << 17)
#define SD_IO_OCR_30_31          (1 << 18)
#define SD_IO_OCR_31_32          (1 << 19)
#define SD_IO_OCR_32_33          (1 << 20)
#define SD_IO_OCR_33_34          (1 << 21)
#define SD_IO_OCR_34_35          (1 << 22)
#define SD_IO_OCR_35_36          (1 << 23)

#define SD_IO_OCR_S18R           (1 << 24)

/*
 * SD_IO_RW_DIRECT argument format:
 *
 *      [31] R/W flag
 *      [30:28] Function number
 *      [27] RAW flag
 *      [25:9] Register address
 *      [7:0] Data
 */
#define SD_IO_W   0x80000000
#define SD_IO_R   0x00000000

#define SD_IO_RAW_YES 0x08000000
#define SD_IO_RAW_NO  0x00000000

#define SD_IO_INC_ADDR 0x04000000
#define SD_IO_FIX_ADDR 0x00000000

#define SD_IO_BLK_MOD 0x08000000

/*
  SDIO status in R5
  Type
    e : error bit
    s : status bit
    r : detected and set for the actual command response
    x : detected and set during command execution. the host must poll
            the card by sending status command in order to read these bits.
  Clear condition
    a : according to the card state
    b : always related to the previous command. Reception of
            a valid command will clear it (with a delay of one command)
    c : clear by read
 */

#define R5_COM_CRC_ERROR    (1 << 15)    /* er, b */
#define R5_ILLEGAL_COMMAND    (1 << 14)    /* er, b */
#define R5_ERROR        (1 << 11)    /* erx, c */
#define R5_FUNCTION_NUMBER    (1 << 9)    /* er, c */
#define R5_OUT_OF_RANGE        (1 << 8)    /* er, c */
#define R5_STATUS(x)        (x & 0xCB00)
#define R5_IO_CURRENT_STATE(x)    ((x & 0x3000) >> 12) /* s, b */


#define SDIO_R    0
#define SDIO_W    1

/*
 * SD_IO_RW_EXTENDED argument format:
 *
 *      [31] R/W flag
 *      [30:28] Function number
 *      [27] Block mode
 *      [26] Increment address
 *      [25:9] Register address
 *      [8:0] Byte/block count
 */

#define R4_MEMORY_PRESENT (1u << 27)
#define R4_MEMORY_READY   (1u << 31)

#define USER_SUPPORT_VOLTAGE     (SD_IO_OCR_27_28 | SD_IO_OCR_28_29 | SD_IO_OCR_29_30 | SD_IO_OCR_30_31 | SD_IO_OCR_31_32 | SD_IO_OCR_32_33 | SD_IO_OCR_33_34)


#define SDIO_CCCR_CAPS        0x08

#define  SDIO_CCCR_CAP_SDC    0x01    /* can do CMD52 while data transfer */
#define  SDIO_CCCR_CAP_SMB    0x02    /* can do multi-block xfers (CMD53) */
#define  SDIO_CCCR_CAP_SRW    0x04    /* supports read-wait protocol */
#define  SDIO_CCCR_CAP_SBS    0x08    /* supports suspend/resume */
#define  SDIO_CCCR_CAP_S4MI    0x10    /* interrupt during 4-bit CMD53 */
#define  SDIO_CCCR_CAP_E4MI    0x20    /* enable ints during 4-bit CMD53 */
#define  SDIO_CCCR_CAP_LSC    0x40    /* low speed card */
#define  SDIO_CCCR_CAP_4BLS    0x80    /* 4 bit low speed card */

#define SDIO_CCCR_CIS        0x09    /* common CIS pointer (3 bytes) */

/* Following 4 regs are valid only if SBS is set */
#define SDIO_CCCR_SUSPEND    0x0c
#define SDIO_CCCR_SELx        0x0d
#define SDIO_CCCR_EXECx        0x0e
#define SDIO_CCCR_READYx    0x0f

#define SDIO_CCCR_BLKSIZE    0x10

#define SDIO_CCCR_POWER        0x12

#define SDIO_POWER_SMPC        0x01    /* Supports Master Power Control */
#define SDIO_POWER_EMPC        0x02    /* Enable Master Power Control */

#define SDIO_CCCR_SPEED        0x13

#define SDIO_SPEED_SHS        0x01    /* Supports High-Speed mode */
#define SDIO_SPEED_EHS        0x02    /* Enable High-Speed mode */

#define SDIO_FBR_STD_IF        0x00

#define  SDIO_FBR_SUPPORTS_CSA    0x40    /* supports Code Storage Area */
#define  SDIO_FBR_ENABLE_CSA    0x80    /* enable Code Storage Area */

#define SDIO_FBR_STD_IF_EXT    0x01

#define SDIO_FBR_POWER        0x02

#define  SDIO_FBR_POWER_SPS    0x01    /* Supports Power Selection */
#define  SDIO_FBR_POWER_EPS    0x02    /* Enable (low) Power Selection */

#define SDIO_FBR_CIS        0x09    /* CIS pointer (3 bytes) */


#define SDIO_FBR_CSA        0x0C    /* CSA pointer (3 bytes) */

#define SDIO_FBR_CSA_DATA    0x0F

#define SDIO_FBR_BLKSIZE    0x10    /* block size (2 bytes) */

/*
 * Function Basic Registers (FBR)
 */
/* SDIO Device FBR Start Address  */
#define SDIOD_FBR_STARTADDR        0x100

/* SDIO Device FBR Size */
#define SDIOD_FBR_SIZE            0x100


#define SDIO_FBR_BASE(f)    ((f) * 0x100) /* base of function f's FBRs */

/* Function register offsets */
#define SDIOD_FBR_DEVCTR        0x00    /* basic info for function */
#define SDIOD_FBR_EXT_DEV        0x01    /* extended I/O device code */
#define SDIOD_FBR_PWR_SEL        0x02    /* power selection bits */

/* SDIO Function CIS ptr offset */
#define SDIOD_FBR_CISPTR_0        0x09
#define SDIOD_FBR_CISPTR_1        0x0A
#define SDIOD_FBR_CISPTR_2        0x0B

/* Code Storage Area pointer */
#define SDIOD_FBR_CSA_ADDR_0        0x0C
#define SDIOD_FBR_CSA_ADDR_1        0x0D
#define SDIOD_FBR_CSA_ADDR_2        0x0E
#define SDIOD_FBR_CSA_DATA        0x0F

/* SDIO Function I/O Block Size */
#define SDIOD_FBR_BLKSIZE_0        0x10
#define SDIOD_FBR_BLKSIZE_1        0x11

/* devctr */
#define SDIOD_FBR_DEVCTR_DIC    0x0f    /* device interface code */
#define SDIOD_FBR_DECVTR_CSA    0x40    /* CSA support flag */
#define SDIOD_FBR_DEVCTR_CSA_EN    0x80    /* CSA enabled */

#define CISTPL_NULL            0x00 //Null
#define CISTPL_CHECKSUM        0x10 //Checksum control
#define CISTPL_VERS_1          0x15 //Level 1 version/product-information
#define CISTPL_ALTSTR          0x16 //The Alternate Language String Tuple
#define CISTPL_MANFID          0x20 //Manufacturer Identification String Tuple
#define CISTPL_FUNCID          0x21 //Function Identification Tuple
#define CISTPL_FUNCE           0x22 //Function Extensions
//80h-8Fh Vendor Unique Tuples
#define CISTPL_SDIO_STD        0x91 //Additional information for functions built to support application specifications for standard SDIO functions.
#define CISTPL_SDIO_EXT        0x92 //Reserved for future use with SDIO devices.
#define CISTPL_END             0xFF //The End-of-chain Tuple

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define  SDIO_BUS_WIDTH_1BIT    0x00
#define  SDIO_BUS_WIDTH_4BIT    0x02
#define  SDIO_BUS_ECSI            0x20    /* Enable continuous SPI interrupt */
#define  SDIO_BUS_SCSI            0x40    /* Support continuous SPI interrupt */
#define  SDIO_BUS_ASYNC_INT        0x20
#define  SDIO_BUS_CD_DISABLE    0x80    /* disable pull-up on DAT3 (pin 1) */

#define MMC_STATE_PRESENT        (1<<0)        /* present in sysfs */
#define MMC_STATE_READONLY        (1<<1)        /* card is read-only */
#define MMC_STATE_HIGHSPEED        (1<<2)        /* card is in high speed mode */
#define MMC_STATE_BLOCKADDR        (1<<3)        /* card uses block-addressing */
#define MMC_STATE_HIGHSPEED_DDR (1<<4)        /* card is in high speed mode */
#define MMC_STATE_ULTRAHIGHSPEED (1<<5)        /* card is in ultra high speed mode */
#define MMC_CARD_SDXC            (1<<6)        /* card is SDXC */


#define mmc_card_set_highspeed(c) ((c)->state |= MMC_STATE_HIGHSPEED)

#define SDIO_CLASS_NONE        0x00    /* Not a SDIO standard interface */
#define SDIO_CLASS_UART        0x01    /* standard UART interface */
#define SDIO_CLASS_BT_A        0x02    /* Type-A BlueTooth std interface */
#define SDIO_CLASS_BT_B        0x03    /* Type-B BlueTooth std interface */
#define SDIO_CLASS_GPS        0x04    /* GPS standard interface */
#define SDIO_CLASS_CAMERA    0x05    /* Camera standard interface */
#define SDIO_CLASS_PHS        0x06    /* PHS standard interface */
#define SDIO_CLASS_WLAN        0x07    /* WLAN interface */
#define SDIO_CLASS_ATA        0x08    /* Embedded SDIO-ATA std interface */
#define SDIO_CLASS_BT_AMP    0x09    /* Type-A Bluetooth AMP interface */


/* Card type */
#define UNKNOW_CARD             (0)         //无法识别或不支持的卡，不可用
#define SDIO                    (0x1 << 1)
#define SDHC                    (0x1 << 2)  //Ver2.00 High Capacity SD Memory Card
#define SD20                    (0x1 << 3)  //Ver2.00 Standard Capacity SD Memory Card
#define SD1X                    (0x1 << 4)  //Ver1.xx Standard Capacity SD Memory Card
#define MMC4                    (0x1 << 5)  //Ver4.xx MMC
#define MMC                     (0x1 << 6)  //Ver3.xx MMC
#define eMMC2G                  (0x1 << 7)  //Ver4.2 larter eMMC and densities greater than 2GB
#define SDXC                    (0x1 << 8)
#define UHS1                    (0x1 << 9)

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
rk_err_t SdioDevShellHelp(HDC dev,  uint8 * pstr);
rk_err_t sdio_memcpy_fromio(SDIO_FUNC *func, void *dst, uint32 addr, uint32 count);
rk_err_t sdio_memcpy_toio(SDIO_FUNC *func, uint32 addr, void *src, uint32 count);
uint32 sdio_max_byte_size(SDIO_FUNC * func);
rk_err_t sdio_io_rw_ext_helper(SDIO_FUNC *func, uint32 write, uint32 addr,
                               uint32 incr_addr, uint8 *buf, uint32 size);
rk_err_t mmc_io_rw_extended(SDIO_DEVICE_CLASS * pstSdioDev, uint32 write,
                            uint32 fn, uint32 addr, uint32 incr_addr, uint8 *buf, uint32 blocks, uint32 blksz, uint8 * out);
rk_err_t sdio_read_func_cis(SDIO_FUNC * func);
rk_err_t sdio_read_fbr(SDIO_FUNC * func);
rk_err_t sdio_init_func(SDIO_DEVICE_CLASS * pstSdioDev, uint32 fn);
rk_err_t sdio_enable_4bit_bus(SDIO_DEVICE_CLASS * pstSdioDev);
rk_err_t mmc_sdio_switch_hs(SDIO_DEVICE_CLASS * pstSdioDev, uint32 enable);
rk_err_t sdio_enable_hs(SDIO_DEVICE_CLASS * pstSdioDev);
rk_err_t sdio_disable_cd(SDIO_DEVICE_CLASS * pstSdioDev);
rk_err_t cistpl_funce_func(SDIO_DEVICE_CLASS * pstSdioDev, SDIO_FUNC *func, const uint8 *buf, uint32 size);
rk_err_t cistpl_funce_common(SDIO_DEVICE_CLASS * pstSdioDev, SDIO_FUNC *func, const uint8 *buf, uint32 size);
rk_err_t cistpl_funce(SDIO_DEVICE_CLASS * pstSdioDev, SDIO_FUNC *func, const uint8 *buf, uint32 size);
rk_err_t cistpl_manfid(SDIO_DEVICE_CLASS * pstSdioDev, SDIO_FUNC *func, const uint8 *buf, uint32 size);
rk_err_t cistpl_vers_1(SDIO_DEVICE_CLASS * pstSdioDev, SDIO_FUNC *func, const uint8 *buf, uint32 size);
rk_err_t cis_tpl_parse(SDIO_DEVICE_CLASS * pstSdioDev, SDIO_FUNC *func,
                       const char *tpl_descr,
                       const CIS_TPL *tpl, int tpl_count,//cis_tpl_list
                       unsigned char code,
                       const unsigned char *buf, unsigned size);
rk_err_t sdio_read_cis(SDIO_DEVICE_CLASS * pstSdioDev, SDIO_FUNC * func);
rk_err_t sdio_read_common_cis(SDIO_DEVICE_CLASS * pstSdioDev);
rk_err_t sdio_read_cccr(SDIO_DEVICE_CLASS * pstSdioDev);
rk_err_t mmc_select_card(SDIO_DEVICE_CLASS * pstSdioDev, uint32 * select);
rk_err_t mmc_send_relative_addr(SDIO_DEVICE_CLASS * pstSdioDev, uint32 * rRCA);
rk_err_t mmc_send_io_op_cond(SDIO_DEVICE_CLASS * pstSdioDev, uint32 ocr, uint32 *rocr);
rk_err_t SdioInitCard(SDIO_DEVICE_CLASS * pstSdioDev);
rk_err_t SdioReset(SDIO_DEVICE_CLASS * pstSdioDev);
rk_err_t SdioSendCmd(HDC hSdc, uint32 cmd, uint32 CmdArg, uint32 *RespBuf);
rk_err_t mmc_io_rw_direct(SDIO_DEVICE_CLASS * pstSdioDev, int32 write, uint32 fn, uint32 addr, uint8 in, uint8 *out);
rk_err_t SdioDevShellTest(HDC dev, uint8 * pstr);
rk_err_t SdioDevShellDel(HDC dev, uint8 * pstr);
rk_err_t SdioDevShellCreate(HDC dev, uint8 * pstr);
rk_err_t SdioDevShellPcb(HDC dev, uint8 * pstr);
void SdioDevIntIsr(void);
rk_err_t SdioDevDeInit(SDIO_DEVICE_CLASS * pstSdioDev);
rk_err_t SdioDevInit(SDIO_DEVICE_CLASS * pstSdioDev);
rk_err_t SdioDevResume(HDC dev);
rk_err_t SdioDevSuspend(HDC dev);


int process_sdio_pending_irqs(SDIO_DEVICE_CLASS* card);
static int sdio_card_irq_get(SDIO_DEVICE_CLASS * card);
static int sdio_card_irq_put(SDIO_DEVICE_CLASS * card);
int SdioDev_Claim_irq(void *_func, sdio_irq_handler_t * handler);
void SdioIrqTask(void);
HDC SdioIntIrq(HDC hSdc, void *arg);
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static SDIO_DEVICE_CLASS * gpstSdioDevISR[SDIO_DEV_MAX_CNT];
static SDIO_DEVICE_HOST *gSdioDeviceHost = NULL;

//_DRIVER_SDIO_SDIODEVICE_COMMON_DATA_
_DRIVER_SDIO_SDIODEVICE_INIT_
static const CIS_TPL cis_tpl_list[] =
{
    {    CISTPL_VERS_1,    3,    cistpl_vers_1        },//0x15
    {    CISTPL_MANFID,    4,    cistpl_manfid        },//0x20
    {    CISTPL_FUNCID,    2,    /* cistpl_funcid */    },//0x21
    {    CISTPL_FUNCE ,    0,    cistpl_funce        },//0x22
};

//_DRIVER_SDIO_SDIODEVICE_COMMON_DATA_
_DRIVER_SDIO_SDIODEVICE_INIT_
static const CIS_TPL cis_tpl_funce_list[] =
{
    {    0x00,    4,    cistpl_funce_common        },
    {    0x01,    0,    cistpl_funce_func        },
    {    0x04,    1+1+6,    /* CISTPL_FUNCE_LAN_NODE_ID */    },
};


//_DRIVER_SDIO_SDIODEVICE_COMMON_DATA_
_DRIVER_SDIO_SDIODEVICE_INIT_
static const unsigned char speed_val[16] =
    { 0, 10, 12, 13, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 70, 80 };

//_DRIVER_SDIO_SDIODEVICE_COMMON_DATA_
_DRIVER_SDIO_SDIODEVICE_INIT_
static const unsigned int speed_unit[8] =
    { 10000, 100000, 1000000, 10000000, 0, 0, 0, 0 };


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/





/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#ifdef _REALTEK_
/*******************************************************************************
** Name: SdioDev_Memcpy_FromIo
** Input:HDC hSdioFun, void *dst, uint32 addr, uint32 count
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.10.26
** Time: 18:22:03
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_COMMON_
COMMON API rk_err_t SdioDev_Memcpy_FromIo(HDC hSdioFun, void *dst, uint32 addr, uint32 count)
{
    SDIO_FUNC * pstFunc = (SDIO_FUNC *)hSdioFun;

    if (pstFunc == NULL)
    {
        return RK_ERROR;
    }

    return sdio_memcpy_fromio(pstFunc, dst, addr, count);
}
/*******************************************************************************
** Name: SdioDev_Memcpy_ToIo
** Input:HDC hSdioFun, uint32 addr, void *src, uint32 count
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.10.26
** Time: 18:19:30
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_COMMON_
COMMON API rk_err_t SdioDev_Memcpy_ToIo(HDC hSdioFun, uint32 addr, void *src, uint32 count)
{
    SDIO_FUNC * pstFunc = (SDIO_FUNC *)hSdioFun;

    if (pstFunc == NULL)
    {
        return RK_ERROR;
    }

    return sdio_memcpy_toio(pstFunc, addr, src, count);
}

/*******************************************************************************
** Name: SdioDev_Writew
** Input:HDC hSdioFun, uint32 b, uint32 addr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.17
** Time: 10:39:50
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_COMMON_
COMMON API rk_err_t SdioDev_Writew(HDC hSdioFun, uint32 b, uint32 addr)
{

    SDIO_FUNC * pstFunc = (SDIO_FUNC *)hSdioFun;

    if (pstFunc == NULL)
    {
        return RK_ERROR;
    }

    *(uint16 *)pstFunc->tmpbuf = b;

    return sdio_memcpy_toio(hSdioFun, addr, pstFunc->tmpbuf, 2);

}

/*******************************************************************************
** Name: SdioDev_Writel
** Input:HDC hSdioFun, uint32 b, uint32 addr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.14
** Time: 17:36:12
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_COMMON_
COMMON API rk_err_t SdioDev_Writel(HDC hSdioFun, uint32 b, uint32 addr)
{
    SDIO_FUNC * pstFunc = (SDIO_FUNC *)hSdioFun;
    if (pstFunc == NULL)
    {
        return RK_ERROR;
    }

    *(uint32 *)pstFunc->tmpbuf = b;

    return sdio_memcpy_toio(pstFunc, addr, pstFunc->tmpbuf, 4);
}

/*******************************************************************************
** Name: SdioDev_WriteSb
** Input:HDC hSdioFun, void *src, uint32 addr, uint32 count
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.14
** Time: 17:21:56
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_COMMON_
COMMON API rk_err_t SdioDev_WriteSb(HDC hSdioFun,uint32 addr, void *src, uint32 count)
{
    SDIO_FUNC * pstFunc = (SDIO_FUNC *)hSdioFun;
    if (pstFunc == NULL)
    {
        return RK_ERROR;
    }

    return sdio_io_rw_ext_helper(pstFunc, SDIO_W, addr, 0, src, count);
}


/*******************************************************************************
** Name: SdioDev_Writeb
** Input:HDC hSdioFunc, uint8 b, uint32 addr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.14
** Time: 16:22:16
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_COMMON_
COMMON API rk_err_t SdioDev_Writeb(HDC hSdioFunc, uint8 b, uint32 addr)
{
    SDIO_FUNC * pstFunc = (SDIO_FUNC *)hSdioFunc;
    SDIO_DEVICE_CLASS * pstSdioDev;
    rk_err_t ret;

    if (pstFunc == NULL)
    {
        return RK_ERROR;
    }

    pstSdioDev = pstFunc->hSdio;

    ret = mmc_io_rw_direct(pstSdioDev, SDIO_W, pstFunc->num, addr, b, NULL);
    return ret;
}
/*******************************************************************************
** Name: SdioDev_ReadW
** Input:HDC hSdioFunc, uint32 addr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.17
** Time: 9:02:55
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_COMMON_
COMMON API rk_err_t SdioDev_Readw(HDC hSdioFunc, uint32 addr)
{
    SDIO_FUNC * pstFunc = (SDIO_FUNC *)hSdioFunc;
    rk_err_t ret;
    if (pstFunc == NULL)
    {
        return RK_ERROR;
    }

    ret = sdio_memcpy_fromio(hSdioFunc, pstFunc->tmpbuf, addr, 2);
    if (ret)
    {
        return RK_ERROR;
    }

    return (*(uint16 *)pstFunc->tmpbuf);
}

/*******************************************************************************
** Name: SdioDev_Readl
** Input:HDC hSdioFun, uint32 addr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.14
** Time: 17:32:42
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_COMMON_
COMMON API rk_err_t SdioDev_Readl(HDC hSdioFun, uint32 addr)
{
    SDIO_FUNC * pstFunc = (SDIO_FUNC *)hSdioFun;
    rk_err_t ret;
    if (pstFunc == NULL)
    {
        return RK_ERROR;
    }

    ret = sdio_memcpy_fromio(pstFunc, (void *)pstFunc->tmpbuf, addr, 4);

    if (ret != RK_SUCCESS)
    {
        return RK_ERROR;
    }

    return (*(uint32*)pstFunc->tmpbuf);
}

/*******************************************************************************
** Name: SdioDev_ReadSb
** Input:HDC hSdioFun, void *dst, uint32 addr, uint32 count
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.14
** Time: 17:20:05
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_COMMON_
COMMON API rk_err_t SdioDev_ReadSb(HDC hSdioFun, void *dst, uint32 addr, uint32 count)
{
    SDIO_FUNC * pstFunc = (SDIO_FUNC *)hSdioFun;
    SDIO_DEVICE_CLASS * pstSdioDev;
    if (pstFunc == NULL)
    {
        return RK_ERROR;
    }

    return sdio_io_rw_ext_helper(pstFunc, SDIO_R, addr, 0, dst, count);
}

/*******************************************************************************
** Name: SdioDev_Readb
** Input:HDC hSdioFunc, uint32 addr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.14
** Time: 16:28:59
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_COMMON_
COMMON API rk_err_t SdioDev_Readb(HDC hSdioFunc, uint32 addr)
{
    SDIO_FUNC * pstFunc = (SDIO_FUNC *)hSdioFunc;
    SDIO_DEVICE_CLASS * pstSdioDev;
    rk_err_t ret;
    uint8 val;

    if (pstFunc == NULL)
    {
        return RK_ERROR;
    }

    pstSdioDev = pstFunc->hSdio;

    ret = mmc_io_rw_direct(pstSdioDev, SDIO_R, pstFunc->num, addr, 0, &val);
    if (ret)
    {

        return RK_ERROR;
    }

    return (rk_err_t)val;

}


/*******************************************************************************
** Name: SdioDev_DisableFunc
** Input:HDC hSdioFunc
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.14
** Time: 16:18:24
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_COMMON_
COMMON API rk_err_t SdioDev_DisableFunc(HDC hSdioFunc)
{
    SDIO_FUNC * pstFunc = (SDIO_FUNC *)hSdioFunc;
    SDIO_DEVICE_CLASS * pstSdioDev;
    rk_err_t ret;
    uint8 reg;

    if (pstFunc == NULL)
    {
        return RK_ERROR;
    }

    pstSdioDev = pstFunc->hSdio;

    ret = mmc_io_rw_direct(pstSdioDev, SDIO_R, SDIO_FUN_0, SDIO_CCCR_IOEx, 0, &reg);
    if (ret)
        goto err;

    reg &= ~(1 << pstFunc->num);

    ret = mmc_io_rw_direct(pstSdioDev, SDIO_W, SDIO_FUN_0, SDIO_CCCR_IOEx, reg, NULL);
    if (ret)
        goto err;

    return 0;

err:
    return ret;

}

/*******************************************************************************
** Name: SdioDev_EnalbeFunc
** Input:HDC hSdioFunc
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.14
** Time: 16:04:21
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_COMMON_
COMMON API rk_err_t SdioDev_EnalbeFunc(HDC hSdioFunc)
{
    SDIO_FUNC * pstFunc = (SDIO_FUNC *)hSdioFunc;
    SDIO_DEVICE_CLASS * pstSdioDev;
    rk_err_t ret;
    uint8 reg, i;

    if (pstFunc == NULL)
    {
        return RK_ERROR;
    }

    pstSdioDev = pstFunc->hSdio;

    ret = mmc_io_rw_direct(pstSdioDev, SDIO_R, SDIO_FUN_0, SDIO_CCCR_IOEx, 0, &reg);
    if (ret)
        goto err;

    reg |= 1 << pstFunc->num;

    ret = mmc_io_rw_direct(pstSdioDev, SDIO_W, SDIO_FUN_0, SDIO_CCCR_IOEx, reg, NULL);
    if (ret)
        goto err;

    i = 0;
    while (1)
    {
        ret = mmc_io_rw_direct(pstSdioDev, SDIO_R, SDIO_FUN_0, SDIO_CCCR_IORx, 0, &reg);
        if (ret)
            goto err;

        if (reg & (1 << pstFunc->num))
            break;

        ret = -ETIME;

        if (i++ >= 200)
            break;

        DelayUs(1000);
    }

    return 0;

err:
    return ret;

}
#endif
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(commom) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: sdio_memcpy_toio
** Input:SDIO_FUNC *func, uint32 addr, void *src, uint32 count
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.14
** Time: 17:14:29
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_COMMON_
COMMON FUN rk_err_t sdio_memcpy_toio(SDIO_FUNC *func, uint32 addr, void *src, uint32 count)
{
    return sdio_io_rw_ext_helper(func, SDIO_W, addr, 1, src, count);
}


/*******************************************************************************
** Name: sdio_memcpy_fromio
** Input:(SDIO_FUNC *func, uint32 addr, void *src, uint32 count)
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.14
** Time: 17:16:19
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_COMMON_
COMMON FUN rk_err_t sdio_memcpy_fromio(SDIO_FUNC *func, void *dst, uint32 addr, uint32 count)
{
    return sdio_io_rw_ext_helper(func, SDIO_R, addr, 1, dst, count);
}

/*******************************************************************************
** Name: sdio_max_byte_size
** Input:SDIO_FUNC * func
** Return: uint32
** Owner:Aaron.sun
** Date: 2014.6.14
** Time: 17:10:31
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_COMMON_
COMMON FUN uint32 sdio_max_byte_size(SDIO_FUNC * func)
{
    uint32 mval =    min(func->cur_blksize, func->max_blksize);

    //rk_printf("func %d, %d", func->cur_blksize, func->max_blksize);

    return min(mval, 512u); /* maximum size for byte mode */
}

/*******************************************************************************
** Name: sdio_io_rw_ext_helper
** Input:void
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.14
** Time: 17:04:47
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_COMMON_
COMMON FUN rk_err_t sdio_io_rw_ext_helper(SDIO_FUNC *func, uint32 write, uint32 addr,
        uint32 incr_addr, uint8 *buf, uint32 size)
{
    uint32 remainder = size;
    uint32 max_blocks;
    rk_err_t ret;

    SDIO_DEVICE_CLASS * pstSdioDev = (SDIO_DEVICE_CLASS *)func->hSdio;

    /* Do the bulk of the transfer using block mode (if supported). */
    if (pstSdioDev->cccr.multi_block && (size > sdio_max_byte_size(func)))  /* maximum size for byte mode */
    {
        /* Blocks per command is limited by host count, host transfer
         * size (we only use a single sg entry) and the maximum for
         * IO_RW_EXTENDED of 511 blocks. */
        max_blocks = 511u;

        while (remainder > func->cur_blksize)
        {
            uint blocks;

            blocks = remainder / func->cur_blksize;
            if (blocks > max_blocks)
                blocks = max_blocks;
            size = blocks * func->cur_blksize;

            ret = mmc_io_rw_extended((SDIO_DEVICE_CLASS *)func->hSdio, write,
                                     func->num, addr, incr_addr, buf,
                                     blocks, func->cur_blksize, NULL);
            if (ret)
            {
                return ret;
            }
            remainder -= size;
            buf += size;
            if (incr_addr)
                addr += size;
        }
    }

    /* Write the remainder using byte mode. */
    while (remainder > 0)
    {
        size = min(remainder, sdio_max_byte_size(func));

        ret = mmc_io_rw_extended((SDIO_DEVICE_CLASS *)func->hSdio, write, func->num, addr, incr_addr, buf, 1, size, NULL);
        if (ret)
        {
            return ret;
        }
        remainder -= size;
        buf += size;
        if (incr_addr)
            addr += size;
    }

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: mmc_io_rw_extended
** Input:void
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.14
** Time: 16:36:48
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_COMMON_
COMMON FUN rk_err_t mmc_io_rw_extended(SDIO_DEVICE_CLASS * pstSdioDev, uint32 write,
                                       uint32 fn, uint32 addr, uint32 incr_addr, uint8 *buf, uint32 blocks, uint32 blksz, uint8 * out)
{
    rk_err_t         ret;
    uint32           status;
    uint32           cmd_opcode = 0;
    uint32           cmd_arg = 0;
    int              sdioc_try = 2;

    SDC_CMD_ARG stCmdArg;

    //rk_printf("%d, %d, %d", fn,blocks, blksz);

    if ((fn > 7) || (blocks == 1 && blksz > 512) || (blocks == 0) || (blksz == 0))
        return -EINVAL;

    /* sanity check */
    if (addr & ~0x1FFFF)
        return -EINVAL;

    cmd_arg = write ? SD_IO_W : SD_IO_R;
    cmd_arg |= fn << 28;     // function number.
    cmd_arg |= incr_addr ? SD_IO_INC_ADDR : SD_IO_FIX_ADDR;// set address.
    cmd_arg |= addr << 9;    // r/w function insided address.

    if (blocks == 1 && blksz <= 512)
        cmd_arg |= (blksz == 512) ? 0 : blksz;    /* byte mode, set transfer bytes number.*/
    else
        cmd_arg |= SD_IO_BLK_MOD | blocks;        /* block mode,set block flag and block number.*/

    cmd_opcode = SD_IO_RW_EXTENDED;
    cmd_opcode |= (write ? SD_WRITE_OP : SD_READ_OP) | SD_RSP_R5 | WAIT_PREV;

    stCmdArg.Cmd = cmd_opcode;
    stCmdArg.CmdArg = cmd_arg;
    stCmdArg.Buf = buf;
    stCmdArg.BlkSize = blksz;
    stCmdArg.Size = blocks * blksz;
    stCmdArg.RespBuf = &status;

    while (sdioc_try)
    {
        if (write)
        {
            ret = SdcDev_Write(pstSdioDev->hSdc, &stCmdArg);
            SdcDev_DataTransIdle(pstSdioDev->hSdc);
        }
        else
        {
            ret = SdcDev_Read(pstSdioDev->hSdc, &stCmdArg);
        }

        if (stCmdArg.Size == ret)
            break;

        sdioc_try--;
    }

    if (stCmdArg.Size != ret)
    {
        printf("sdc err\n");
        return RK_ERROR;
    }

    if (out != NULL)
        *out = status;

    if (status & R5_ERROR)
    {
        return -EIO;
    }
    if (status & R5_FUNCTION_NUMBER)
    {
        return -EINVAL;
    }
    if (status & R5_OUT_OF_RANGE)
    {
        return -ERANGE;
    }
    return RK_SUCCESS;

}


/*******************************************************************************
** Name: SdioSendCmd
** Input:HDC hSdc, uint32 cmd, uint32 CmdArg, uint32 *RespBuf
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.14
** Time: 11:24:36
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_COMMON_
COMMON FUN rk_err_t SdioSendCmd(HDC hSdc, uint32 cmd, uint32 CmdArg, uint32 *RespBuf)
{
    SDC_CMD_ARG DevReqArg;
    DevReqArg.Cmd = cmd;
    DevReqArg.CmdArg = CmdArg;
    DevReqArg.RespBuf = RespBuf;
    return SdcDev_SendCmd(hSdc, &DevReqArg);
}

/*******************************************************************************
** Name: mmc_io_rw_direct
** Input:void *pCardInfo, int32 write, uint32 fn, uint32 addr, uint8 in, uint8 *out
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.14
** Time: 11:12:29
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_COMMON_
COMMON FUN rk_err_t mmc_io_rw_direct(SDIO_DEVICE_CLASS * pstSdioDev, int32 write, uint32 fn,
                                     uint32 addr, uint8 in, uint8 *out)
{
    rk_err_t         ret;
    UINT32           status;
    UINT32           cmd_arg = 0;


    /* sanity check */
    if (addr & ~0x1FFFF)
        return -EINVAL;

    cmd_arg = write ? SD_IO_W : SD_IO_R;
    cmd_arg |= fn << 28;     // function number.
    cmd_arg |= (write && out) ? SD_IO_RAW_YES : SD_IO_RAW_NO;// read after write.
    cmd_arg |= addr << 9;    // r/w function insided address.
    cmd_arg |= in;           // if this handle is writing data,the in is the data that will be write to sdio.

    ret = SdioSendCmd(pstSdioDev->hSdc, (SD_IO_RW_DIRECT | SD_NODATA_OP | SD_RSP_R5 | WAIT_PREV), cmd_arg, &status);

    if (RK_SUCCESS != ret)
    {
        return RK_ERROR;
    }

    //rk_printf("write = %d status = %d, addr = %d in = %d", write, status, addr, in);

    if (status & R5_ERROR)
        return -EIO;
    if (status & R5_FUNCTION_NUMBER)
        return -EINVAL;
    if (status & R5_OUT_OF_RANGE)
        return -ERANGE;

    if (out)
        *out = status & 0xFF;

    return RK_SUCCESS;

}

/*******************************************************************************
** Name: SdioDevIntIsr
** Input:void
** Return: void
** Owner:Aaron.sun
** Date: 2014.6.14
** Time: 9:40:35
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_COMMON_
COMMON FUN void SdioDevIntIsr(void)
{

}

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: SdidDevDel
** Input:void
** Return: rk_err_t
** Owner:cjh
** Date: 2015.8.13
** Time: 19:26:18
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_INIT_
INIT API rk_err_t SdioDevDelete(uint32 DevID ,void * arg)
{
    SDIO_DEV_ARG * pstSdioArg = (SDIO_DEV_ARG *)arg;

    //SDIO_DEVICE_CLASS * pstSdioDev = (SDIO_DEVICE_CLASS *)dev;
    //Check SdDev is not exist...
    rk_printf("enter SdioDevDelet ID%d\n",DevID);
    if (gpstSdioDevISR[DevID] == NULL)
    {
        return RK_ERROR;
    }
    //SdDev deinit...
    if((pstSdioArg != NULL) && (gpstSdioDevISR[DevID]->hSdc != NULL))
    {
        pstSdioArg->hSdc = gpstSdioDevISR[DevID]->hSdc;
    }

    SdioDevDeInit(gpstSdioDevISR[DevID]);
    rk_printf("del free gpstSdioDevISR id =%d\n",DevID);
    //Free SdDev memory...

    rkos_semaphore_delete(gpstSdioDevISR[DevID]->osSdioOperReqSem);
    if(gpstSdioDevISR[DevID]->info != NULL)
    {
        rk_printf("free info\n");
        rkos_memory_free(gpstSdioDevISR[DevID]->info);
    }
    rkos_memory_free(gpstSdioDevISR[DevID]);
    rk_printf("free gpstSdioDevISR\n");

    SdioDevHwDeInit(DevID, 0);
    //Delete SdDev...
    gpstSdioDevISR[DevID] = NULL;
    rk_printf("FW_RemoveSegment SdidDevDelet\n");
    //Delete SdDev Read and Write Module...
#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    FW_RemoveSegment(SEGMENT_ID_SDIO_DEV);
#endif

    return RK_SUCCESS;

}
/*******************************************************************************
** Name: SdioDev_DisalbeInt
** Input:HDC dev, uint32 FuncNum
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.7.10
** Time: 17:40:45
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_INIT_
INIT API rk_err_t SdioDev_DisalbeInt(HDC dev, uint32 FuncNum)
{

}

/*******************************************************************************
** Name: SdioDev_EnableInt
** Input:HDC dev, uint32 FunNum
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.7.10
** Time: 17:39:36
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_INIT_
INIT API rk_err_t SdioDev_EnableInt(HDC dev, uint32 FunNum)
{
    SDIO_DEVICE_CLASS * pstSdioDev = (SDIO_DEVICE_CLASS *)dev;
    unsigned char reg;
    unsigned char r_a_w;
    rk_err_t ret;

    if (pstSdioDev == NULL)
    {
        return RK_PARA_ERR;
    }

    ret = mmc_io_rw_direct(pstSdioDev, SDIO_R, SDIO_FUN_0, SDIO_CCCR_IENx, 0, &reg);
    if (ret)
        return ret;

    reg |= 1 << 1;
    reg |= 1 << 2;

    reg |= 1; /* Master interrupt enable */

    r_a_w = 0;

    ret = mmc_io_rw_direct(pstSdioDev, SDIO_W, SDIO_FUN_0, SDIO_CCCR_IENx, reg, &r_a_w);
    if (ret)
        return ret;


}

/*******************************************************************************
** Name: SdioDev_GetFuncHandle
** Input:uint32 FuncNum
** Return: HDC
** Owner:Aaron.sun
** Date: 2014.6.23
** Time: 15:33:40
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_COMMON_
INIT API HDC SdioDev_GetFuncHandle(HDC dev, uint32 FuncNum)
{
    SDIO_DEVICE_CLASS * pstSdioDev = (SDIO_DEVICE_CLASS *)dev;

    if (pstSdioDev == NULL)
    {
        return (HDC)RK_PARA_ERR;
    }

    if (FuncNum >=  SDIO_FUNC_MAX)
    {
        return (HDC)RK_PARA_ERR;
    }

    return (HDC)&pstSdioDev->Func[FuncNum];
}

/*******************************************************************************
** Name: SdioDev_SetBlockSize
** Input:HDC hSdioFun
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.14
** Time: 17:26:29
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_COMMON_
INIT API rk_err_t SdioDev_SetBlockSize(HDC hSdioFun, uint32 BlockSize)
{
    SDIO_FUNC * pstFunc = (SDIO_FUNC *)hSdioFun;
    SDIO_DEVICE_CLASS * pstSdioDev;
    rk_err_t ret;

    if (pstFunc == NULL)
    {
        return RK_ERROR;
    }

    pstSdioDev = pstFunc->hSdio;

    if (BlockSize == 0)
    {
        BlockSize = min(pstFunc->max_blksize, 4095u);
        BlockSize = min(BlockSize, 512u);
    }
    ret = mmc_io_rw_direct(pstSdioDev, SDIO_W, SDIO_FUN_0,
                           SDIO_FBR_BASE(pstFunc->num) + SDIO_FBR_BLKSIZE,
                           BlockSize & 0xff, NULL);
    if (ret)
        return ret;

    ret = mmc_io_rw_direct(pstSdioDev, SDIO_W, SDIO_FUN_0,
                           SDIO_FBR_BASE(pstFunc->num) + SDIO_FBR_BLKSIZE + 1,
                           (BlockSize >> 8) & 0xff, NULL);
    if (ret)
        return ret;

    pstFunc->cur_blksize = BlockSize;
    return 0;

}


/*******************************************************************************
** Name: SdioDev_Create
** Input:void * arg
** Return: HDC
** Owner:Aaron.sun
** Date: 2014.6.14
** Time: 9:40:35
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_INIT_
INIT API HDC SdioDev_Create(uint32 DevID, void * arg)
{

    DEVICE_CLASS * pstDev;
    SDIO_DEVICE_CLASS * pstSdioDev;
    SDIO_DEV_ARG * pstSdioArg = (SDIO_DEV_ARG *)arg;
    if (arg == NULL)
    {
        return (HDC)(RK_PARA_ERR);
    }

    pstSdioDev =  rkos_memory_malloc(sizeof(SDIO_DEVICE_CLASS));

    if (pstSdioDev == NULL)
    {
        return pstSdioDev;
    }

    memset(pstSdioDev, 0x00, sizeof(SDIO_DEVICE_CLASS));

    pstSdioDev->osSdioOperReqSem  = rkos_semaphore_create(1,1);

    if (pstSdioDev->osSdioOperReqSem == 0)
    {
        rkos_semaphore_delete(pstSdioDev->osSdioOperReqSem);
        rkos_memory_free(pstSdioDev);
        return (HDC) RK_ERROR;
    }

    pstDev = (DEVICE_CLASS *)pstSdioDev;

    pstDev->suspend = SdioDevSuspend;
    pstDev->resume  = SdioDevResume;

    if(pstSdioArg->hSdc != NULL)
    {
        pstSdioDev->hSdc = pstSdioArg->hSdc;
        rk_printf("pstSdioArg->hSdc !NULL");
    }

    rk_printf("cjh sdio id=%d",DevID);
    pstSdioDev->stSdioDevice.DevID = DevID;
    gpstSdioDevISR[DevID] = NULL;
    SdioDevHwInit(DevID, 0);

#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    FW_LoadSegment(SEGMENT_ID_SDIO_DEV, SEGMENT_OVERLAY_CODE);
#endif

    rk_printf("SdioDevInit");
    if (SdioDevInit(pstSdioDev) != RK_SUCCESS)
    {
        rk_printf("SdioDevInit != RK_SUCCESS");
        rkos_semaphore_delete(pstSdioDev->osSdioOperReqSem);
        rkos_memory_free(pstSdioDev);
        return (HDC) RK_ERROR;
    }
    SdcRegisterIoIntIsr(pstSdioDev->hSdc, SdioIntIrq);
    //pstSdioDev->osSdioIrqSem = rkos_semaphore_create(1,0);

    if (gSdioDeviceHost == NULL )
    {
        gSdioDeviceHost =  rkos_memory_malloc(sizeof(SDIO_DEVICE_HOST));
    }
    memset(gSdioDeviceHost, 0x00, sizeof(SDIO_DEVICE_HOST));


    gpstSdioDevISR[DevID] = pstSdioDev;

    pstSdioDev->host = gSdioDeviceHost;

    if (gSdioDeviceHost->devCnt == 0)
    {
        gSdioDeviceHost->osSdioIrqSem = rkos_semaphore_create(0xffff,0);

        RKTaskCreate(TASK_ID_SDIO_IRQ_TASK,0,NULL, ASYNC_MODE);
    }
    gSdioDeviceHost->devCnt++;
    return pstDev;

}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: sdio_read_func_cis
** Input:SDIO_FUNC * func
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.14
** Time: 15:38:56
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_INIT_
INIT FUN rk_err_t sdio_read_func_cis(SDIO_FUNC * func)
{
    rk_err_t ret;

    ret = sdio_read_cis((SDIO_DEVICE_CLASS *)func->hSdio, func);
    if (ret)
        return ret;

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: sdio_read_fbr
** Input:SDIO_FUNC * func
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.14
** Time: 15:31:03
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_INIT_
INIT FUN rk_err_t sdio_read_fbr(SDIO_FUNC * func)
{
    rk_err_t ret;
    uint8 data;

    ret = mmc_io_rw_direct((SDIO_DEVICE_CLASS *)func->hSdio, SDIO_R, SDIO_FUN_0, SDIO_FBR_BASE(func->num) + SDIO_FBR_STD_IF, 0, &data);
    if (ret)
        goto out;

    data &= 0x0f;// The SDIO Standard Function code identifies those I/O functions

    if (data == SDIO_CLASS_NONE)
        return 0;

    if (data == 0x0f)
    {
        ret = mmc_io_rw_direct((SDIO_DEVICE_CLASS *)func->hSdio, SDIO_R, SDIO_FUN_0, SDIO_FBR_BASE(func->num) + SDIO_FBR_STD_IF_EXT, 0, &data);
        if (ret)
            goto out;
    }

    func->func_class = data;

out:
    return ret;
}

/*******************************************************************************
** Name: sdio_init_func
** Input:SDIO_DEVICE_CLASS * pstSdioDev, uint32 fn
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.14
** Time: 15:21:15
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_INIT_
INIT FUN rk_err_t sdio_init_func(SDIO_DEVICE_CLASS * pstSdioDev, uint32 fn)
{
    rk_err_t ret;

    SDIO_FUNC *func;

    if (fn > SDIO_FUNC_MAX)
        return -EPERM;

    func = &pstSdioDev->Func[fn];
    memset(func, 0, sizeof(SDIO_FUNC));

    func->num = fn;
    func->hSdio = pstSdioDev;

    ret = sdio_read_fbr(func);
    if (ret)
        goto fail;

    ret = sdio_read_func_cis(func);
    if (ret)
        goto fail;

    return 0;

fail:
    /*
     * It is okay to remove the function here even though we hold
     * the host lock as we haven't registered the device yet.
     */
    return ret;
}

/*******************************************************************************
** Name: sdio_enable_4bit_bus
** Input:SDIO_DEVICE_CLASS * pstSdioDev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.14
** Time: 14:58:44
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_INIT_
INIT FUN rk_err_t sdio_enable_4bit_bus(SDIO_DEVICE_CLASS * pstSdioDev)
{
    rk_err_t ret;
    uint8 ctrl;
    uint32   wide = BUS_WIDTH_INVALID;

    /*
     * whether is card internal support wide bus
     */
    if (pstSdioDev->cccr.low_speed && !pstSdioDev->cccr.wide_bus)
        return 0;

    ret = mmc_io_rw_direct(pstSdioDev, SDIO_R, SDIO_FUN_0, SDIO_CCCR_IF, 0, &ctrl);
    if (ret)
        return ret;

    ctrl |= SDIO_BUS_WIDTH_4BIT;

    ret = mmc_io_rw_direct(pstSdioDev, SDIO_W, SDIO_FUN_0, SDIO_CCCR_IF, ctrl, NULL);
    if (ret)
        return ret;

    /*
      * whether is SDC iomux config wide bus.
      */
    ret =  SdcDev_SetBusWidth(pstSdioDev->hSdc, BUS_WIDTH_4_BIT);
    if (ret != RK_SUCCESS)
        return RK_ERROR;

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: mmc_sdio_switch_hs
** Input:SDIO_DEVICE_CLASS * pstSdioDev, uint32 enable
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.14
** Time: 14:49:26
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_INIT_
INIT FUN rk_err_t mmc_sdio_switch_hs(SDIO_DEVICE_CLASS * pstSdioDev, uint32 enable)
{
    rk_err_t ret;
    uint8 speed;

    if (!pstSdioDev->cccr.high_speed)
        return 0;

    ret = mmc_io_rw_direct(pstSdioDev, SDIO_R, SDIO_FUN_0, SDIO_CCCR_SPEED, 0, &speed);
    if (ret)
        return ret;

    if (enable)
        speed |= SDIO_SPEED_EHS;
    else
        speed &= ~SDIO_SPEED_EHS;

    ret = mmc_io_rw_direct(pstSdioDev, SDIO_W, SDIO_FUN_0, SDIO_CCCR_SPEED, speed, NULL);
    if (ret)
        return ret;

    return 1;
}

/*******************************************************************************
** Name: sdio_enable_hs
** Input:SDIO_DEVICE_CLASS * pstSdioDev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.14
** Time: 14:48:04
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_INIT_
INIT FUN rk_err_t sdio_enable_hs(SDIO_DEVICE_CLASS * pstSdioDev)
{
    return mmc_sdio_switch_hs(pstSdioDev, TRUE);
}

/*******************************************************************************
** Name: sdio_disable_cd
** Input:SDIO_DEVICE_CLASS * pstSdioDev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.14
** Time: 14:44:45
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_INIT_
INIT FUN rk_err_t sdio_disable_cd(SDIO_DEVICE_CLASS * pstSdioDev)
{
    rk_err_t ret;
    uint8 ctrl;

    ret = mmc_io_rw_direct(pstSdioDev, SDIO_R, SDIO_FUN_0, SDIO_CCCR_IF, 0, &ctrl);
    if (ret)
        return ret;

    ctrl |= SDIO_BUS_CD_DISABLE;

    return mmc_io_rw_direct(pstSdioDev, SDIO_W, SDIO_FUN_0, SDIO_CCCR_IF, ctrl, NULL);
}

/*******************************************************************************
** Name: cistpl_funce_func
** Input:SDIO_DEVICE_CLASS * pstSdioDev, SDIO_FUNC *func, const uint8 *buf, uint32 size
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.14
** Time: 14:27:43
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_INIT_
INIT FUN rk_err_t cistpl_funce_func(SDIO_DEVICE_CLASS * pstSdioDev, SDIO_FUNC *func, const uint8 *buf, uint32 size)
{
    uint32 vsn;
    uint32 min_size;

    /* Only valid for the individual function's CIS (1-7) */
    if (!func)
        return -EINVAL;

    /*
     * This tuple has a different length depending on the SDIO spec
     * version.
     */
    vsn = ((SDIO_DEVICE_CLASS *)(func->hSdio))->cccr.sdio_vsn;
    min_size = (vsn == SDIO_SDIO_REV_1_00) ? 28 : 42;

    if (size < min_size)
        return -EINVAL;

    /* TPLFE_MAX_BLK_SIZE */
    func->max_blksize = buf[12] | (buf[13] << 8);

    /* TPLFE_ENABLE_TIMEOUT_VAL, present in ver 1.1 and above */
    if (vsn > SDIO_SDIO_REV_1_00)
        func->enable_timeout = (buf[28] | (buf[29] << 8)) * 10;
    else
        func->enable_timeout = 1000;//dgl

    return 0;
}

/*******************************************************************************
** Name: cistpl_funce_common
** Input:SDIO_DEVICE_CLASS * pstSdioDev, SDIO_FUNC *func, const uint8 *buf, uint32 size
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.14
** Time: 14:25:09
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_INIT_
INIT FUN rk_err_t cistpl_funce_common(SDIO_DEVICE_CLASS * pstSdioDev, SDIO_FUNC *func, const uint8 *buf, uint32 size)
{

    /* Only valid for the common CIS (function 0) */
    if (func)
        return -EINVAL;

    /* TPLFE_FN0_BLK_SIZE */
    pstSdioDev->cis.blksize = buf[1] | (buf[2] << 8);

    /* TPLFE_MAX_TRAN_SPEED */
    pstSdioDev->cis.max_dtr = speed_val[(buf[3] >> 3) & 15] * speed_unit[buf[3] & 7];

    return 0;
}


/*******************************************************************************
** Name: cistpl_funce
** Input:SDIO_DEVICE_CLASS * pstSdioDev, SDIO_FUNC *func, const uint8 *buf, uint32 size
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.14
** Time: 14:20:31
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_INIT_
COMMON FUN rk_err_t cistpl_funce(SDIO_DEVICE_CLASS * pstSdioDev, SDIO_FUNC *func, const uint8 *buf, uint32 size)
{
    if (size < 1)
        return -EINVAL;

    return cis_tpl_parse(pstSdioDev, func, "CISTPL_FUNCE",
                         cis_tpl_funce_list, ARRAY_SIZE(cis_tpl_funce_list),
                         buf[0], buf, size);
}

/*******************************************************************************
** Name: cistpl_manfid
** Input:SDIO_DEVICE_CLASS * pstSdioDev, SDIO_FUNC *func, const uint8 *buf, uint32 size
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.14
** Time: 14:16:44
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_INIT_
COMMON FUN rk_err_t cistpl_manfid(SDIO_DEVICE_CLASS * pstSdioDev, SDIO_FUNC *func, const uint8 *buf, uint32 size)
{
    unsigned int vendor, device;

    /* TPLMID_MANF */
    vendor = buf[0] | (buf[1] << 8);//vendor id

    /* TPLMID_CARD */
    device = buf[2] | (buf[3] << 8);//device id : 0x4330

    printf("manfid:vendor:0x%x device:0x%x\n", vendor, device);

    if (func)
    {
        func->vendor = vendor;
        func->device = device;
    }
    else
    {
        pstSdioDev->cis.vendor = vendor;
        pstSdioDev->cis.device = device;
    }

    return 0;
}

/*******************************************************************************
** Name: cistpl_vers_1
** Input:SDIO_DEVICE_CLASS * pstSdioDev, SDIO_FUNC *func, const uint8 *buf, uint32 size
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.14
** Time: 14:10:56
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_INIT_
INIT FUN rk_err_t cistpl_vers_1(SDIO_DEVICE_CLASS * pstSdioDev, SDIO_FUNC *func, const uint8 *buf, uint32 size)
{
    unsigned i, nr_strings;
    char **buffer, *string;
    int malloc_siz = 0;

    /* Find all null-terminated (including zero length) strings in
       the TPLLV1_INFO field. Trailing garbage is ignored. */
    buf += 2;
    size -= 2;

    nr_strings = 0;
    for (i = 0; i < size; i++)
    {
        if (buf[i] == 0xff)
            break;
        if (buf[i] == 0)
            nr_strings++;
    }
    if (nr_strings == 0)
        return 0;

    size = i;
    malloc_siz = sizeof(char*) * nr_strings + size;
    buffer = rkos_memory_malloc(malloc_siz);
    if (!buffer)
        return -ENOMEM;
    memset(buffer, 0, malloc_siz);

    string = (char*)(buffer + nr_strings);

    for (i = 0; i < nr_strings; i++)
    {
        buffer[i] = string;
        memcpy(string, (char *)buf, StrLenA((uint8 *)buf));
        string += StrLenA(string) + 1;
        buf += StrLenA((uint8 *)buf) + 1;
    }

    if (func)
    {
        func->num_info = nr_strings;
        func->info = (const char**)buffer;
    }
    else
    {
        pstSdioDev->num_info = nr_strings;
        pstSdioDev->info = (const char**)buffer;
    }

    return 0;
}

/*******************************************************************************
** Name: cis_tpl_parse
** Input:void *card, SDIO_FUNC *func
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.14
** Time: 14:03:05
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_INIT_
INIT FUN rk_err_t cis_tpl_parse(SDIO_DEVICE_CLASS * pstSdioDev, SDIO_FUNC *func,
                                const char *tpl_descr,
                                const CIS_TPL *tpl, int tpl_count,//cis_tpl_list
                                unsigned char code,
                                const unsigned char *buf, unsigned size)
{
    int i, ret;

    /* look for a matching code in the table */
    for (i = 0; i < tpl_count; i++, tpl++)
    {
        if (tpl->code == code)
            break;
    }
    if (i < tpl_count)
    {
        if (size >= tpl->min_size)
        {
            if (tpl->parse)
                ret = tpl->parse(pstSdioDev, func, buf, size);//dgl cis_tpl_list
            else
                ret = -EILSEQ;    /* known tuple, not parsed */
        }
        else
        {
            /* invalid tuple */
            ret = -EINVAL;
        }
    }
    else
    {
        /* unknown tuple */
        ret = -ENOENT;
    }

    return ret;
}

/*******************************************************************************
** Name: sdio_read_cis
** Input:SDIO_DEVICE_CLASS * pstSdioDev, SDIO_FUNC * func
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.14
** Time: 13:54:20
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_INIT_
INIT FUN rk_err_t sdio_read_cis(SDIO_DEVICE_CLASS * pstSdioDev, SDIO_FUNC * func)
{
    rk_err_t ret;
    SDIO_FUNC_TUPLE *this, **prev;
    uint i, ptr = 0;
    uint malloc_siz = 0;

    /*
     * Note that this works for the common CIS (function number 0) as
     * well as a function's CIS * since SDIO_CCCR_CIS and SDIO_FBR_CIS
     * have the same offset.
     */
    for (i = 0; i < 3; i++)
    {
        uint8 x, fn;

        if (func)
            fn = func->num;
        else
            fn = 0;

        ret = mmc_io_rw_direct(pstSdioDev, SDIO_R, SDIO_FUN_0, SDIO_FBR_BASE(fn) + SDIO_FBR_CIS + i, 0, &x);
        if (ret)
            return ret;
        ptr |= x << (i * 8); //make up the 24bits address.
    }

    if (func)
        prev = &func->tuples;
    else
        prev = &pstSdioDev->tuples;

    if (*prev != NULL)
        while (1);

    do
    {
        unsigned char tpl_code, tpl_link;

        ret = mmc_io_rw_direct(pstSdioDev, SDIO_R, SDIO_FUN_0, ptr++, 0, &tpl_code);
        if (ret)
            break;

        /* 0xff means we're done */
        if (tpl_code == 0xff)
            break;

        /* null entries have no link field or data */
        if (tpl_code == 0x00)
            continue;

        ret = mmc_io_rw_direct(pstSdioDev, SDIO_R, SDIO_FUN_0, ptr++, 0, &tpl_link);
        if (ret)
            break;

        /* a size of 0xff also means we're done */
        if (tpl_link == 0xff)
            break;

        malloc_siz = sizeof(*this) + tpl_link;//tpl_link : This is the number of bytes in the tuple body. (n)
        this = rkos_memory_malloc(malloc_siz);
        if (!this)
            return -ENOMEM;
        memset((void *)this, 0, malloc_siz);

        for (i = 0; i < tpl_link; i++)
        {
            ret = mmc_io_rw_direct(pstSdioDev, SDIO_R, SDIO_FUN_0, ptr + i, 0, &this->data[i]);//ptr:cis address pointer.
            if (ret)
            {
                rkos_memory_free(this);
                break;
            }
        }

        if (ret)
        {
            rkos_memory_free(this);
            break;
        }

        /* Try to parse the CIS tuple */
        ret = cis_tpl_parse(pstSdioDev, func, "CIS",
                            cis_tpl_list, ARRAY_SIZE(cis_tpl_list),
                            tpl_code, this->data, tpl_link);
        if (ret == -EILSEQ || ret == -ENOENT)
        {
            /*
             * The tuple is unknown or known but not parsed.
             * Queue the tuple for the function driver.
             */
            this->next = NULL;
            this->code = tpl_code;
            this->size = tpl_link;
            *prev = this;
            prev = &this->next;

            if (ret == -ENOENT)
            {
                /* warn about unknown tuples */
                // This is the normal exit procedure,rather than an error
                ;
            }
            /* keep on analyzing tuples */
            ret = 0;
            rkos_memory_free(this);
        }
        else
        {
            /*
             * We don't need the tuple anymore if it was
             * successfully parsed by the SDIO core or if it is
             * not going to be queued for a driver.
             */
            rkos_memory_free(this);
        }
        ptr += tpl_link; //ptr: cis address pointer.
    }
    while (!ret);
    /*
     * Link in all unknown tuples found in the common CIS so that
     * drivers don't have to go digging in two places.
     */
    if (func)
        *prev = pstSdioDev->tuples;

    return ret;
}

/*******************************************************************************
** Name: sdio_read_common_cis
** Input:SDIO_DEVICE_CLASS * pstSdioDev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.14
** Time: 13:53:23
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_INIT_
INIT FUN rk_err_t sdio_read_common_cis(SDIO_DEVICE_CLASS * pstSdioDev)
{
    return sdio_read_cis(pstSdioDev, NULL);
}

/*******************************************************************************
** Name: sdio_read_cccr
** Input:SDIO_DEVICE_CLASS * pstSdioDev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.14
** Time: 13:45:28
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_INIT_
INIT FUN rk_err_t sdio_read_cccr(SDIO_DEVICE_CLASS * pstSdioDev)
{
    rk_err_t ret;
    int cccr_vsn;
    uint8 data = 0;

    ret = mmc_io_rw_direct(pstSdioDev, SDIO_R, SDIO_FUN_0, SDIO_CCCR_CCCR, 0, &data);
    if (ret)
        goto out;

    cccr_vsn = data & 0x0f;//02h : CCCR/FBR defined in SDIO Version 2.00

    #ifdef _WIFI_5G_ap6234
    //if (cccr_vsn > SDIO_CCCR_REV_1_20)
       // return -EINVAL;
    #endif

    pstSdioDev->cccr.sdio_vsn = (data & 0xf0) >> 4;// 03h : SDIO Specification Version 2.00

    ret = mmc_io_rw_direct(pstSdioDev, SDIO_R, SDIO_FUN_0, SDIO_CCCR_CAPS, 0, &data);
    if (ret)
        goto out;

    if (data & SDIO_CCCR_CAP_SMB)   // Support Multiple Block Transfer CMD53
        pstSdioDev->cccr.multi_block = 1;
    if (data & SDIO_CCCR_CAP_LSC)   // Low-Speed Card
        pstSdioDev->cccr.low_speed = 1;
    if (data & SDIO_CCCR_CAP_4BLS)  // 4-bit Mode Support for Low-Speed Card
        pstSdioDev->cccr.wide_bus = 1;

    if (cccr_vsn >= SDIO_CCCR_REV_1_10)
    {
        ret = mmc_io_rw_direct(pstSdioDev, SDIO_R, SDIO_FUN_0, SDIO_CCCR_POWER, 0, &data);
        if (ret)
            goto out;

        /*
         Support Master Power Control.
         SMPC=1: The total card power may exceed 720mW.
                 Controls of EMPC, SPS and EPS are available.
        */
        if (data & SDIO_POWER_SMPC)
            pstSdioDev->cccr.high_power = 1;
    }

    if (cccr_vsn >= SDIO_CCCR_REV_1_20)
    {
        ret = mmc_io_rw_direct(pstSdioDev, SDIO_R, SDIO_FUN_0, SDIO_CCCR_SPEED, 0, &data);
        if (ret)
            goto out;

        if (data & SDIO_SPEED_SHS)//Support High-Speed : SHS=1: The card supports High-Speed mode...
            pstSdioDev->cccr.high_speed = 1;
    }

out:
    return ret;
}

/*******************************************************************************
** Name: mmc_select_card
** Input:SDIO_DEVICE_CLASS * pstSdioDev, uint32 * select
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.14
** Time: 13:42:08
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_INIT_
INIT FUN rk_err_t mmc_select_card(SDIO_DEVICE_CLASS * pstSdioDev, uint32 * select)
{
    rk_err_t         ret;
    UINT32           status;

    /**************************************************/
    // make card entry into Cmd State
    // I/O State : stby => cmd By CMD7;
    /**************************************************/
    ret = SdioSendCmd(pstSdioDev->hSdc, (SD_SELECT_DESELECT_CARD | SD_NODATA_OP | SD_RSP_R1B | WAIT_PREV), (pstSdioDev->rca << 16), &status);
    if (RK_SUCCESS != ret)
    {
        return ret;
    }

    if (select)
        *select = status;

    return ret;

}

/*******************************************************************************
** Name: mmc_send_relative_addr
** Input:SDIO_DEVICE_CLASS * pstSdioDev, uint32 * rRCA
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.14
** Time: 13:37:25
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_INIT_
INIT FUN rk_err_t mmc_send_relative_addr(SDIO_DEVICE_CLASS * pstSdioDev, uint32 * rRCA)
{

    rk_err_t         err;
    UINT32           status;

    err = SdioSendCmd(pstSdioDev->hSdc, (SD_SEND_RELATIVE_ADDR | SD_NODATA_OP | SD_RSP_R6 | WAIT_PREV), 0, &status);
    if (err != RK_SUCCESS)
    {
        return RK_ERROR;
    }
    pstSdioDev->rca = (uint16)(status >> 16);//RCA:Relative Card Address register,set by arm when host send cmd3;

    if (rRCA)
        *rRCA = status;

    return err;

}

/*******************************************************************************
** Name: mmc_send_io_op_cond
** Input:SDIO_DEV_CLASS * pstSdioDev, uint32 ocr, uint32 *rocr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.14
** Time: 13:31:49
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_INIT_
INIT FUN rk_err_t mmc_send_io_op_cond(SDIO_DEVICE_CLASS * pstSdioDev, uint32 ocr, uint32 *rocr)
{

    UINT32           i = 0;
    rk_err_t         ret;
    uint32           status;

    /**************************************************/
    // make card entery to Ready State
    // It define the longest time is one second,so in here, turn to number of cycles is 100, once is 10 ms almost.
    /**************************************************/

    for (i=0; i<100; i++)
    {
        ret = SdioSendCmd(pstSdioDev->hSdc, (SD_IO_SEND_OP_COND | SD_NODATA_OP | SD_RSP_R4 | WAIT_PREV), ocr, &status);
        if (ret)
            break;

        /* if we're just probing, do a single pass */
        if (ocr == 0)
            break;

        /* otherwise wait until reset completes */
        if (status & R4_MEMORY_READY)
            break;

        DelayMs(10);
    }

    if (rocr)
        *rocr = status;

    return ret;

}

/*******************************************************************************
** Name: SdioInitCard
** Input:SDIO_DEVICE_CLASS * pstSdioDev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.14
** Time: 11:59:47
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_INIT_
INIT FUN rk_err_t SdioInitCard(SDIO_DEVICE_CLASS * pstSdioDev)
{

    rk_err_t ret;

    /*
     * Sanity check the voltages that the card claims to support.
     */
    if (pstSdioDev->ocr & 0xFF)
        pstSdioDev->ocr &= ~0xFF;

    /*
     * Inform the card of the voltage
     */
    ret = mmc_send_io_op_cond(pstSdioDev, pstSdioDev->ocr&USER_SUPPORT_VOLTAGE, NULL);
    if (ret)
        goto remove;

    /*
     * For native busses:  set card RCA and quit open drain mode.
     */
    ret = mmc_send_relative_addr(pstSdioDev, NULL);
    if (ret)
        goto remove;

    /*
     * Select card, as all following commands rely on that.
     */
    ret = mmc_select_card(pstSdioDev, NULL);
    if (ret)
        goto remove;

    /*
    * Read the common registers.
    */
    ret = sdio_read_cccr(pstSdioDev);
    if (ret)
        goto remove;

    /*
     * Read the common CIS tuples.
     */
    ret= sdio_read_common_cis(pstSdioDev);
    if (ret)
        goto remove;

    /*
     * If needed, disconnect card detection pull-up resistor.
     */
    ret = sdio_disable_cd(pstSdioDev);
    if (ret)
        goto remove;

    /*
     * Switch to high-speed (if supported).
     * err == 0/1 [not]support high speed.
     */
    ret = sdio_enable_hs(pstSdioDev);
    if (ret > 0)
        mmc_card_set_highspeed(pstSdioDev); // success to enable high speed.
    else if (ret)
        goto remove;

    /*
     * Change to the card's maximum speed.
     */
    ret = SdcDev_SetBusFreq(pstSdioDev->hSdc, SD_FPP_FREQ);//SD_FPP_FREQ SDHC_FPP_FREQ
    if (RK_SUCCESS != ret)
        goto remove;

    pstSdioDev->TranSpeed = SD_FPP_FREQ;

    /*
    * Switch to wider bus (if supported).
    */
    ret = sdio_enable_4bit_bus(pstSdioDev);
    if (ret > 0)
        ; //success to switch 4 bits wide.
    else if (ret)
        goto remove;

    return 0;

remove:
    return ret;

}


/*******************************************************************************
** Name: SdioReset
** Input:SDIO_DEVICE_CLASS * pstSdioDev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.14
** Time: 11:48:04
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_INIT_
INIT FUN rk_err_t SdioReset(SDIO_DEVICE_CLASS * pstSdioDev)
{
    int ret;
    uint8 abort;

    /* SDIO Simplified Specification V2.0, 4.4 Reset for SDIO */

    ret = mmc_io_rw_direct(pstSdioDev, SDIO_R, SDIO_FUN_0, SDIO_CCCR_ABORT, 0, &abort);
    if (ret)
        abort = 0x08;
    else
        abort |= 0x08;

    ret = mmc_io_rw_direct(pstSdioDev, SDIO_W, SDIO_FUN_0, SDIO_CCCR_ABORT, abort, NULL);

    return ret;
}

/*******************************************************************************
** Name: SdioDevDeInit
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.14
** Time: 9:40:35
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_INIT_
INIT FUN rk_err_t SdioDevDeInit(SDIO_DEVICE_CLASS * pstSdioDev)
{
    if (gSdioDeviceHost->devCnt)
    {
        gSdioDeviceHost->devCnt--;
        if (gSdioDeviceHost->devCnt == 0)
        {
            //if (gSdioDeviceHost->osSdioIrqSem)
            {

                rkos_semaphore_delete(gSdioDeviceHost->osSdioIrqSem);
            }

            RKTaskDelete(TASK_ID_SDIO_IRQ_TASK, 0, SYNC_MODE);
            rk_printf("RKTaskDelete free gSdioDeviceHost\n");
            rkos_memory_free(gSdioDeviceHost);

            gSdioDeviceHost = NULL;
        }

    }

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: SdioDevInit
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.14
** Time: 9:40:35
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_INIT_
INIT FUN rk_err_t SdioDevInit(SDIO_DEVICE_CLASS * pstSdioDev)
{
    rk_err_t ret;
    uint32 status = 0;
    uint32 nf;
    uint32 mp;
    uint32 funcs, i;

    rk_printf("enter SdioDevInit");
    SdcDev_SetBusWidth(pstSdioDev->hSdc, BUS_WIDTH_1_BIT);
    SdcDev_SetBusFreq(pstSdioDev->hSdc, FOD_FREQ);

    DelayUs(1000);  //等待电源和时钟稳定

    //SdioReset(pstSdioDev);

    ret = SdioSendCmd(pstSdioDev->hSdc, (SD_GO_IDLE_STATE | SD_NODATA_OP | SD_RSP_NONE | NO_WAIT_PREV | SEND_INIT), 0, NULL);
    if (RK_SUCCESS != ret)
    {
        rk_printf("SdioSendCmd fail");
        return RK_ERROR;
    }

    DelayUs(100);  // 27有发现CMD0发送完以后延时一下再发其他命令能提高卡的识别率

    ret = SdioSendCmd(pstSdioDev->hSdc, (SDIO_IO_SEND_OP_COND | SD_NODATA_OP | SD_RSP_R4 | WAIT_PREV), 0, &status);
    if (RK_SUCCESS != ret)
    {
        printf("22SdioSendCmd fail");
        return RK_ERROR;
    }

    nf = (status >> 28) & 0x7;//bit31-bit29 : number of I/O functions...
    mp = (status >> 27) & 0x1;//bit28 : memory present...
    if ((mp == 0) && (nf > 0) && (status & 0xFFFF00))
    {
        //bit27-bit26 : stuff bits
        //bit25 : S18A
        //bit24-bit0 : The supported minimum and maximum values for VDD
        //bit0-bit7 : reserved bits no use.
        //bit8 : 2.0-2.1
        //bit9 : 2.1-2.2
        //.......
        //bit23: 3.5-3.6
        /* SDIO-only Card */
        pstSdioDev->ocr = status;

        ret = SdioInitCard(pstSdioDev);
        if (ret)
        {
            rk_printf("SdioInitCard fail");
            return ret;
        }
        /*
        * The number of functions on the card is encoded inside
        * the ocr.
        */
        funcs = (pstSdioDev->ocr & 0x70000000) >> 28;

        #if (defined  _WIFI_5G_AP6234) ||  (defined _WIFI_5G_AP6255)
        funcs = 2;
        #endif

        rk_printf("sdio funcs = %d", funcs);
        /*
         * Initialize (but don't add) all present functions.
         */
        pstSdioDev->sdio_funcs = funcs;
        {
            SDIO_FUNC *func;

            func = &pstSdioDev->Func[0];
            memset(func, 0, sizeof(SDIO_FUNC));

            func->num = 0;
            func->hSdio = pstSdioDev;
        }

        for (i = 0; i < funcs; i++)
        {
            ret = sdio_init_func(pstSdioDev, i + 1);
            if (ret)
            {
                rk_printf("sdio_init_func fail");
                return RK_ERROR;
            }
        }

        pstSdioDev->type = SDIO;

    }

    return RK_SUCCESS;

}


/******************************************************************************
 * SdioIrqTask -
 * DESCRIPTION: -
 *
 * Input:
 * Output:
 * Returns:
 *
 * modification history
 * --------------------
 * Owner: wangping
 * Date : 2014.7.15  written
 * Time : 11:46:21
 * --------------------
 ******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_COMMON_
void SdioIrqTask(void)
{

    int i;
    //printf("SdioOK 01 \n");
    while (1)
    {
        // printf("SdioIrqTask 01\r\n");
        rkos_semaphore_take(gSdioDeviceHost->osSdioIrqSem, MAX_DELAY);

        //rk_printf("SdioIrqTask");
        for (i=0; i<SDIO_DEV_MAX_CNT; i++ )
        {
            if (gpstSdioDevISR[i])
            {
               // if(gpstSdioDevISR[i]->irqcnt)
                {
                   //rk_printf("SdioIrqTask 01");
                   process_sdio_pending_irqs(gpstSdioDevISR[i]) ;
                   //rk_printf("SdioIrqTask 02");
                   //gpstSdioDevISR[i]->irqcnt--;

                }
            }
        }

    }

}

/******************************************************************************
 * SdioIntIrq -
 * DESCRIPTION: -
 *
 * Input:
 * Output:
 * Returns:
 *
 * modification history
 * --------------------
 * Owner: wangping
 * Date : 2014.7.15  written
 * Time : 11:46:11
 * --------------------
 ******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_COMMON_
API HDC SdioIntIrq(HDC hSdc, void *arg)
{

    int i;

    for (i=0; i<SDIO_DEV_MAX_CNT; i++ )
    {
        if (gpstSdioDevISR[i])
        {
            //if (gpstSdioDevISR[i]->hSdc == hSdc)
            {
                //gpstSdioDevISR[i]->irqcnt++;
                //printf("SdioIntirq\r\n");
                rkos_semaphore_give_fromisr(gSdioDeviceHost->osSdioIrqSem);

            }
        }
    }

}

_DRIVER_SDIO_SDIODEVICE_COMMON_
rk_err_t SdioIntIrqInit(void *pvParameters)
{
    return RK_SUCCESS;
}

_DRIVER_SDIO_SDIODEVICE_COMMON_
rk_err_t SdioIntIrqDeInit(void *pvParameters)
{
    return RK_SUCCESS;
}

/******************************************************************************
 * process_sdio_pending_irqs -
 * DESCRIPTION: -
 *
 * Input:
 * Output:
 * Returns:
 *
 * modification history
 * --------------------
 * Owner: wangping
 * Date : 2014.7.15  written
 * Time : 11:46:04
 * --------------------
 ******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_COMMON_
int process_sdio_pending_irqs(SDIO_DEVICE_CLASS* card)
{

    int i, ret = 0, count;
    uint8 pending;
    SDIO_FUNC *func;
    //SDIO_DEVICE_CLASS* card;

    //card = (SDM_CARD_INFO*)SDIOM_GetCardInfo(0);
    /*
     * Optimization, if there is only 1 function interrupt registered
     * call irq handler directly
     */
    func = card->sdio_single_irq;
    if (func)
    {
        func->irq_handler(func);
        return 1;
    }

    ret = mmc_io_rw_direct(card, SDIO_R, SDIO_FUN_0, SDIO_CCCR_INTx, 0, &pending);
    if (ret)
    {
        printf("error %d reading SDIO_CCCR_INTx", ret);
        return ret;
    }
    //printf("pending = %d\n", pending);
    count = 0;
    for (i = 1; i <= 7; i++)
    {
        if (pending & (1 << i))
        {
            func = &card->Func[i];
            if (!func)
            {
                //RKDEBUG("no func");
                ret = -EINVAL;
            }
            else if (func->irq_handler)
            {
                /* IRQHandlerF2 IRQHandler->dhdsdio_isr->dhdsdio_dpc->dhdsdio_readframes */
                //rk_printf("func = %x", func->irq_handler);
                func->irq_handler(func);
                //rk_printf("y");
                count++;
            }
            else
            {
                //RKDEBUG("func %d: pending IRQ with no handler\n", func->num);
                ret = -EINVAL;
            }
        }

    }

    if (count)
        return count;

    return ret;

}

/******************************************************************************
 * sdio_card_irq_get -
 * DESCRIPTION: -
 *
 * Input:
 * Output:
 * Returns:
 *
 * modification history
 * --------------------
 * Owner: wangping
 * Date : 2014.7.15  written
 * Time : 9:54:11
 * --------------------
 ******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_COMMON_
static int sdio_card_irq_get(SDIO_DEVICE_CLASS * card)
{
    card->sdio_irqs++;
    return 0;
}

/******************************************************************************
 * sdio_card_irq_put -
 * DESCRIPTION: -
 *
 * Input:
 * Output:
 * Returns:
 *
 * modification history
 * --------------------
 * Owner: wangping
 * Date : 2014.7.15  written
 * Time : 9:54:06
 * --------------------
 ******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_COMMON_
static int sdio_card_irq_put(SDIO_DEVICE_CLASS * card)
{
    if (card->sdio_irqs < 1)
        return -EPERM;

    --card->sdio_irqs;

    return 0;
}


/******************************************************************************
 * sdio_single_irq_set - If there is only 1 function registered set sdio_single_irq
 * DESCRIPTION: -
 *
 * Input:
 * Output:
 * Returns:
 *
 * modification history
 * --------------------
 * Owner: wangping
 * Date : 2014.7.15  written
 * Time : 9:53:40
 * --------------------
 ******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_COMMON_
static void sdio_single_irq_set(SDIO_DEVICE_CLASS* card)
{
    SDIO_FUNC *func;
    int i;

    card->sdio_single_irq = NULL;
    if (card->sdio_irqs == 1)
    {
        for (i = 0; i < card->sdio_funcs; i++)
        {
            func = &card->Func[i];
            if (func && func->irq_handler)
            {
                card->sdio_single_irq = func;
                break;
            }
        }
    }
}


/******************************************************************************
 * SdioDev_Claim_irq - claim the IRQ for a SDIO function
 * DESCRIPTION: -
 *    Claim and activate the IRQ for the given SDIO function. The provided
 *    handler will be called when that IRQ is asserted.  The host is always
 *    claimed already when the handler is called so the handler must not
 *    call sdio_claim_host() nor sdio_release_host().
 *
 * Input:  @func: SDIO function
 *           @handler: IRQ handler callback
 * Output:
 * Returns:
 *
 * modification history
 * --------------------
 * Owner: wangping
 * Date : 2014.7.15  written
 * Time : 9:48:55
 * --------------------
 ******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_COMMON_
int SdioDev_Claim_irq(void *_func, sdio_irq_handler_t * handler)
{
    int ret;
    unsigned char reg;
    unsigned char r_a_w;
    SDIO_FUNC * func = _func;
    if (!func || !handler)
        return -EPERM;
    if (func->irq_handler)
    {
        return -EBUSY;
    }

    ret = mmc_io_rw_direct(func->hSdio, SDIO_R, SDIO_FUN_0, SDIO_CCCR_IENx, 0, &reg);
    if (ret)
        return ret;

    reg |= 1 << func->num;

    reg |= 1; /* Master interrupt enable */

    r_a_w = 0;

    ret = mmc_io_rw_direct(func->hSdio, SDIO_W, SDIO_FUN_0, SDIO_CCCR_IENx, reg, &r_a_w);
    if (ret)
        return ret;

    func->irq_handler = handler;

    ret = sdio_card_irq_get(func->hSdio);
    if (ret)
        func->irq_handler = NULL;
    sdio_single_irq_set(func->hSdio);

    return ret;
}


/******************************************************************************
 * sdio_release_irq -  release the IRQ for a SDIO function
 * DESCRIPTION: - Disable and release the IRQ for the given SDIO function.
 *
 * Input: SDIO function
 * Output:
 * Returns:
 *
 * modification history
 * --------------------
 * Owner: wangping
 * Date : 2014.7.15  written
 * Time : 9:53:02
 * --------------------
 ******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_COMMON_
int sdio_release_irq(void *_func)
{
    int ret;
    uint8 reg;

    SDIO_FUNC * func = _func;

    if (!func || !func->hSdio)
        return -EPERM;

    if (func->irq_handler)
    {
        func->irq_handler = NULL;
        sdio_card_irq_put(func->hSdio);
        sdio_single_irq_set(func->hSdio);
    }

    ret = mmc_io_rw_direct(func->hSdio, SDIO_R, SDIO_FUN_0, SDIO_CCCR_IENx, 0, &reg);
    if (ret)
        return ret;

    reg &= ~(1 << func->num);

    /* Disable master interrupt with the last function interrupt */
    if (!(reg & 0xFE))
        reg = 0;

    ret = mmc_io_rw_direct(func->hSdio, SDIO_W, SDIO_FUN_0, SDIO_CCCR_IENx, reg, NULL);
    if (ret)
        return ret;

    return 0;
}



/*******************************************************************************
** Name: SdioDevResume
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.14
** Time: 9:40:35
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_INIT_
INIT FUN rk_err_t SdioDevResume(HDC dev)
{

}

/*******************************************************************************
** Name: SdioDevSuspend
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.14
** Time: 9:40:35
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_INIT_
INIT FUN rk_err_t SdioDevSuspend(HDC dev)
{

}


#ifdef _SDIO_DEV_SHELL_
_DRIVER_SDIO_SDIODEVICE_SHELL_DATA_
static SHELL_CMD ShellSdioName[] =
{
    "pcb",SdioDevShellPcb,"list sdio device pcb inf","sdio.pcb [object id]",
    "create",SdioDevShellCreate,"create a sdio device","sdio.create",
    "delete",SdioDevShellDel,"delete a sdio device","sdio.delete",
    "test",SdioDevShellTest,"test sdio device","sdio.test",
    "\b",NULL,"NULL","NULL",
};

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: SdioDev_Shell
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.14
** Time: 9:40:35
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_SHELL_
SHELL API rk_err_t SdioDev_Shell(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret = RK_SUCCESS;

    uint8 Space;

    if(ShellHelpSampleDesDisplay(dev, ShellSdioName, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }


    StrCnt = ShellItemExtract(pstr,&pItem, &Space);

    if((StrCnt == 0) || (*(pstr - 1) != '.'))
    {
        return RK_ERROR;
    }

    ret = ShellCheckCmd(ShellSdioName, pItem, StrCnt);
    if (ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;                                            //remove '.',the point is the useful item

    ShellHelpDesDisplay(dev, ShellSdioName[i].CmdDes, pItem);
    if(ShellSdioName[i].ShellCmdParaseFun != NULL)
    {
        ret = ShellSdioName[i].ShellCmdParaseFun(dev, pItem);
    }

    return ret;
}

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: SdioDevShellTest
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.14
** Time: 9:40:35
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_SHELL_
SHELL FUN rk_err_t SdioDevShellTest(HDC dev, uint8 * pstr)
{
    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }
    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }
    // TODO:
    //add other code below:
    //...

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: SdioDevShellDel
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.14
** Time: 9:40:35
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_SHELL_
SHELL FUN rk_err_t SdioDevShellDel(HDC dev, uint8 * pstr)
{
    SDIO_DEVICE_CLASS * pstSdioDev = (SDIO_DEVICE_CLASS *)dev;
    SDIO_DEV_ARG pstSdioArg;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }

    #ifdef _WICE_
    Grf_GpioMuxSet(AP6181_POWER_ON_GPIO_CH, AP6181_POWER_ON_GPIO_PIN, Type_Gpio);
    Gpio_SetPinDirection(AP6181_POWER_ON_GPIO_CH, AP6181_POWER_ON_GPIO_PIN, GPIO_OUT);
    Gpio_SetPinLevel(AP6181_POWER_ON_GPIO_CH, AP6181_POWER_ON_GPIO_PIN, GPIO_LOW);
    DelayMs(2);
    Grf_GpioMuxSet(AP6181_POWER_ON_GPIO_CH, AP6181_POWER_ON_GPIO_PIN, Type_Gpio);
    Gpio_SetPinDirection(AP6181_POWER_ON_GPIO_CH, AP6181_POWER_ON_GPIO_PIN, GPIO_OUT);
    Gpio_SetPinLevel(AP6181_POWER_ON_GPIO_CH, AP6181_POWER_ON_GPIO_PIN, GPIO_HIGH);
    #endif

    if (RKDev_Delete(DEV_CLASS_SDIO, 0, &pstSdioArg) != RK_SUCCESS)
    {
        printf("SDDev delete failureDevID=%d\n",pstSdioDev->stSdioDevice.DevID);
        return RK_SUCCESS;
    }

    if (pstSdioArg.hSdc != NULL)
    {
        rk_printf("pstSdioArg.hSdc close\n");
        if (RKDev_Close(pstSdioArg.hSdc) != RK_SUCCESS)
        {
            rk_printf("hsdc close failure\n");
            return RK_SUCCESS;
        }
    }

    printf("RKDev_Delete over\n");

    return RK_SUCCESS;
}


/*******************************************************************************
** Name: SdioDevShellMc
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.14
** Time: 9:40:35
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_SHELL_
SHELL FUN rk_err_t SdioDevShellCreate(HDC dev, uint8 * pstr)
{
    HDC hSdc;
    SDIO_DEV_ARG stSdioArg;
    rk_err_t ret;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }
    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }

    #ifdef _WICE_
    Grf_GpioMuxSet(AP6181_POWER_ON_GPIO_CH, AP6181_POWER_ON_GPIO_PIN, Type_Gpio);
    Gpio_SetPinDirection(AP6181_POWER_ON_GPIO_CH, AP6181_POWER_ON_GPIO_PIN, GPIO_OUT);
    Gpio_SetPinLevel(AP6181_POWER_ON_GPIO_CH, AP6181_POWER_ON_GPIO_PIN, GPIO_LOW);
    DelayMs(2);
    Grf_GpioMuxSet(AP6181_POWER_ON_GPIO_CH, AP6181_POWER_ON_GPIO_PIN, Type_Gpio);
    Gpio_SetPinDirection(AP6181_POWER_ON_GPIO_CH, AP6181_POWER_ON_GPIO_PIN, GPIO_OUT);
    Gpio_SetPinLevel(AP6181_POWER_ON_GPIO_CH, AP6181_POWER_ON_GPIO_PIN, GPIO_HIGH);
    #endif

    hSdc = RKDev_Open(DEV_CLASS_SDC, 1, NOT_CARE);

    if ((hSdc == NULL) || (hSdc == (HDC)RK_ERROR) || (hSdc == (HDC)RK_PARA_ERR))
    {
        rk_print_string("\nERROR:Sdc Device Open Failure\n");
        return RK_SUCCESS;
    }

    stSdioArg.hSdc = hSdc;
    ret = RKDev_Create(DEV_CLASS_SDIO, 0, &stSdioArg);
    if (ret != RK_SUCCESS)
    {
        rk_print_string("\nERROR:sdio Device Create Failure\n");
        return RK_ERROR;
    }
    rk_print_string("sdio0 Device Create Success\n");

    return RK_SUCCESS;

}


/*******************************************************************************
** Name: SdioDevShellPcb
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.14
** Time: 9:40:35
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_SHELL_
SHELL FUN rk_err_t SdioDevShellPcb(HDC dev, uint8 * pstr)
{
    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }
    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }
//  TODO:
    //add other code below:
    //...

    return RK_SUCCESS;
}
#endif
#endif


