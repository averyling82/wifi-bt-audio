/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Driver\USBMsc\UsbMscDevice.h
* Owner: wrm
* Date: 2015.5.25
* Time: 10:28:52
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    wrm     2015.5.25     10:28:52   1.0
********************************************************************************************
*/


#ifndef __DRIVER_USBMSC_USBMSCDEVICE_H__
#define __DRIVER_USBMSC_USBMSCDEVICE_H__

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
#define _DRIVER_USBMSC_USBMSCDEVICE_COMMON_  __attribute__((section("driver_usbmsc_usbmscdevice_common")))
#define _DRIVER_USBMSC_USBMSCDEVICE_INIT_    __attribute__((section("driver_usbmsc_usbmscdevice_init")))
#define _DRIVER_USBMSC_USBMSCDEVICE_SHELL_   __attribute__((section("driver_usbmsc_usbmscdevice_shell")))
#if defined(__arm__) && (__ARMCC_VERSION)
#define _DRIVER_USBMSC_USBMSCDEVICE_SHELL_DATA_      _DRIVER_USBMSC_USBMSCDEVICE_SHELL_
#elif defined(__arm__) && (__GNUC__)
#define _DRIVER_USBMSC_USBMSCDEVICE_SHELL_DATA_      __attribute__((section("driver_usbmsc_usbmscdevice_shell_data")))
#else
#error Unknown compiling tools.
#endif

#ifndef USBMSC_LUN
#define    USBMSC_LUN  10
#endif

typedef  struct _USBMSC_DEV_ARG
{
    //USBMSC_DEVICE_CLASS usbMscDev;
    HDC hUsbOtgDev;
    HDC hLun[USBMSC_LUN];
    uint8 valid_lun;
    uint usbmode;

}USBMSC_DEV_ARG;

#define    USBMSC_DEV_NUM                              2

#define    BULK_IN_EP                                  0x01
#define    BULK_OUT_EP                                 0x01

#define    EP0_PACKET_SIZE                             64


#define    USB_TYPE_MASK                               (0x03 << 5)
#define    USB_TYPE_STANDARD                           (0x00 << 5)
#define    USB_TYPE_CLASS                              (0x01 << 5)
#define    USB_TYPE_VENDOR                             (0x02 << 5)
#define    USB_TYPE_RESERVED                           (0x03 << 5)

//define    USB subclass
#define    USB_SUBCLASS_CODE_RBC                       0x01
#define    USB_SUBCLASS_CODE_SFF8020I                  0x02
#define    USB_SUBCLASS_CODE_QIC157                    0x03
#define    USB_SUBCLASS_CODE_UFI                       0x04
#define    USB_SUBCLASS_CODE_SFF8070I                  0x05
#define    USB_SUBCLASS_CODE_SCSI                      0x06

// defined USB device classes
#define    USB_DEVICE_CLASS_RESERVED                   0x00
#define    USB_DEVICE_CLASS_AUDIO                      0x01    //音频设备
#define    USB_DEVICE_CLASS_COMMUNICATIONS             0x02    //通讯设备
#define    USB_DEVICE_CLASS_HUMAN_INTERFACE            0x03    //人机接口
#define    USB_DEVICE_CLASS_MONITOR                    0x04    //显示器
#define    USB_DEVICE_CLASS_PHYSICAL_INTERFACE         0x05    //物理接口
#define    USB_DEVICE_CLASS_POWER                      0x06    //电源
#define    USB_DEVICE_CLASS_PRINTER                    0x07    //打印机
#define    USB_DEVICE_CLASS_STORAGE                    0x08    //海量存储
#define    USB_DEVICE_CLASS_HUB                        0x09    //HUB
#define    USB_DEVICE_CLASS_VENDOR_SPECIFIC            0xFF    //

//define    USB protocol
#define    USB_PROTOCOL_CODE_CBI0                      0x00
#define    USB_PROTOCOL_CODE_CBI1                      0x01
#define    USB_PROTOCOL_CODE_BULK                      0x50

#define    FS_BULK_RX_SIZE                             64
#define    FS_BULK_TX_SIZE                             64
#define    HS_BULK_RX_SIZE                             512
#define    HS_BULK_TX_SIZE                             512

/*******************************************************************
CSW返回状态值
*******************************************************************/
#define    CSW_GOOD                                    0x00        //命令通过
#define    CSW_FAIL                                    0x01        //命令失败
#define    CSW_PHASE_ERROR                             0x02        //命令有误

/*******************************************************************
Bulk Only命令集
*******************************************************************/
#define    K_SCSICMD_TEST_UNIT_READY                   0x00
#define    K_SCSICMD_REZERO_UNIT                       0x01
#define    K_SCSICMD_REQUEST_SENSE                     0x03
#define    K_SCSICMD_FORMAT_UNIT                       0x04
#define    K_SCSICMD_INQUIRY                           0x12
#define    K_SCSICMD_MODE_SELECT_06                    0x15
#define    K_SCSICMD_MODE_SENSE_06                     0x1a
#define    K_SCSICMD_START_STOP_UNIT                   0x1b
#define    K_SCSICMD_SEND_DIAGNOSTIC                   0x1d
#define    K_SCSICMD_PREVENT_ALLOW_MEDIUM_REMOVAL      0x1e
#define    K_SCSICMD_READ_FORMAT_CAPACITIES            0x23
#define    K_SCSICMD_READ_CAPACITY                     0x25
#define    K_SCSICMD_READ_10                           0x28
#define    K_SCSICMD_WRITE_10                          0x2a
#define    K_SCSICMD_SEEK_10                           0x2b
#define    K_SCSICMD_WRITE_AND_VERIFY_10               0x2e
#define    K_SCSICMD_VERIFY_10                         0x2f
#define    K_SCSICMD_SYNCHRONIZE_CACHE_10              0x35
#define    K_SCSICMD_MODE_SELECT_10                    0x55
#define    K_SCSICMD_MODE_SENSE_10                     0x5a
#define    K_SCSICMD_PERSISTENT_RESERVE_IN             0x5e
#define    K_SCSICMD_READ_12                           0xa8
#define    K_SCSICMD_WRITE_12                          0xaa

/*******************************************************************
Sense codes
*******************************************************************/
#define    SCSI_SENSE_NO_SENSE                         0x00
#define    SCSI_SENSE_RECOVERED_ERROR                  0x01
#define    SCSI_SENSE_NOT_READY                        0x02
#define    SCSI_SENSE_MEDIUM_ERROR                     0x03
#define    SCSI_SENSE_HARDWARE_ERROR                   0x04
#define    SCSI_SENSE_ILLEGAL_REQUEST                  0x05
#define    SCSI_SENSE_UNIT_ATTENTION                   0x06
#define    SCSI_SENSE_DATA_PROTECT                     0x07
#define    SCSI_SENSE_BLANK_CHECK                      0x08
#define    SCSI_SENSE_UNIQUE                           0x09
#define    SCSI_SENSE_COPY_ABORTED                     0x0A
#define    SCSI_SENSE_ABORTED_COMMAND                  0x0B
#define    SCSI_SENSE_EQUAL                            0x0C
#define    SCSI_SENSE_VOL_OVERFLOW                     0x0D
#define    SCSI_SENSE_MISCOMPARE                       0x0E
#define    SCSI_SENSE_RESERVED                         0x0F
/*******************************************************************
Additional Sense Codes(ASC)
*******************************************************************/
#define    SCSI_ADSENSE_NO_SENSE                       0x00
#define    SCSI_ADSENSE_LUN_NOT_READY                  0x04
#define    SCSI_ADSENSE_ILLEGAL_COMMAND                0x20
#define    SCSI_ADSENSE_ILLEGAL_BLOCK                  0x21
#define    SCSI_ADSENSE_INVALID_PARAMETER              0x26
#define    SCSI_ADSENSE_INVALID_LUN                    0x25
#define    SCSI_ADSENSE_INVALID_CDB                    0x24
#define    SCSI_ADSENSE_MUSIC_AREA                     0xA0
#define    SCSI_ADSENSE_DATA_AREA                      0xA1
#define    SCSI_ADSENSE_VOLUME_OVERFLOW                0xA7
#define    SCSI_ADSENSE_NO_MEDIA_IN_DEVICE             0x3A
#define    SCSI_ADSENSE_FORMAT_ERROR                   0x31
#define    SCSI_ADSENSE_CMDSEQ_ERROR                   0x2C
#define    SCSI_ADSENSE_MEDIUM_CHANGED                 0x28
#define    SCSI_ADSENSE_BUS_RESET                      0x29
#define    SCSI_ADWRITE_PROTECT                        0x27
#define    SCSI_ADSENSE_TRACK_ERROR                    0x14
#define    SCSI_ADSENSE_SAVE_ERROR                     0x39
#define    SCSI_ADSENSE_SEEK_ERROR                     0x15
#define    SCSI_ADSENSE_REC_DATA_NOECC                 0x17
#define    SCSI_ADSENSE_REC_DATA_ECC                   0x18
/*******************************************************************
 Additional sense code qualifier(ASCQ)
*******************************************************************/
#define    SCSI_SENSEQ_FORMAT_IN_PROGRESS              0x04
#define    SCSI_SENSEQ_INIT_COMMAND_REQUIRED           0x02
#define    SCSI_SENSEQ_MANUAL_INTERVENTION_REQUIRED    0x03
#define    SCSI_SENSEQ_BECOMING_READY                  0x01
#define    SCSI_SENSEQ_FILEMARK_DETECTED               0x01
#define    SCSI_SENSEQ_SETMARK_DETECTED                0x03
#define    SCSI_SENSEQ_END_OF_MEDIA_DETECTED           0x02
#define    SCSI_SENSEQ_BEGINNING_OF_MEDIA_DETECTED     0x04
/*******************************************************************
Mode Sense/Select page constants.
*******************************************************************/
#define    MODE_PAGE_RBC_DEVICE_PARAMETERS             0x06

#define    PAGECTRL_CURRENT                            0x00
#define    PAGECTRL_CHANGEABLE                         0x01
#define    PAGECTRL_DEFAULT                            0x02
#define    PAGECTRL_SAVED                              0x03

/*******************************************************************
Page field for Inquiry vital product data parameters
*******************************************************************/
#define    SUPPORTED_PAGES                             0x00
#define    SERIAL_NUMBER_PAGE                          0x80
#define    DEVICE_INDENIFICATION_PAGE                  0x83


#define    K_RKCMD_SONY_FWUPDATE                       0xfc
#define    K_RKCMD_FIRMWAVE_UPGRADE                    0xff


/* Bulk-only class specific requests */
#define    USB_BULK_RESET_REQUEST                      0xff
#define    USB_BULK_GET_MAX_LUN_REQUEST                0xfe

#define    CBWCDBLENGTH                                16

#define    FSG_STRING_MANUFACTURER                     1
#define    FSG_STRING_PRODUCT                          2
#define    FSG_STRING_SERIAL                           3
#define    FSG_STRING_CONFIG                           0
#define    FSG_STRING_INTERFACE                        0

#define    USB_BULK_CB_WRAP_LEN                        31
#define    USB_BULK_CB_SIG                             0x43425355 /*CBW FLAG*/
#define    USB_BULK_IN_FLAG                            0x80

#define    USB_BULK_CS_WRAP_LEN                        13
#define    USB_BULK_CS_SIG                             0x53425355 /*CSW FLAG*/
#define    USB_STATUS_PASS                             0
#define    USB_STATUS_FAIL                             1
#define    USB_STATUS_PHASE_ERROR                      2

#define    USB_MAX_TRANS_SEC                           0Xf0

#define    MIN(x,y) ((x) < (y) ? (x) : (y))

#define    __get_unaligned_be32(p)    (uint32)(p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3])
#define    __get_unaligned_be16(p)    (uint16)(p[0] << 8 | p[1])


#if 1

#define    MAX_LUN_NUM             3

#define    USB_BBB_RESET           0xff
#define    USB_BBB_GET_MAX_LUN     0xfe

#define    SCSI_CHANGE_DEF         0x40    /* Change Definition (Optional) */
#define    SCSI_COMPARE            0x39    /* Compare (O) */
#define    SCSI_COPY               0x18    /* Copy (O) */
#define    SCSI_COP_VERIFY         0x3A    /* Copy and Verify (O) */
#define    SCSI_INQUIRY_CMD        0x12    /* Inquiry (MANDATORY) */
#define    SCSI_LOG_SELECT         0x4C    /* Log Select (O) */
#define    SCSI_LOG_SENSE          0x4D    /* Log Sense (O) */
#define    SCSI_MODE_SEL6          0x15    /* Mode Select 6-byte (Device Specific) */
#define    SCSI_MODE_SEL10         0x55    /* Mode Select 10-byte (Device Specific) */
#define    SCSI_MODE_SEN6          0x1A    /* Mode Sense 6-byte (Device Specific) */
#define    SCSI_MODE_SEN10         0x5A    /* Mode Sense 10-byte (Device Specific) */
#define    SCSI_READ_BUFF          0x3C    /* Read Buffer (O) */
#define    SCSI_REQ_SENSE          0x03    /* Request Sense (MANDATORY) */
#define    SCSI_SEND_DIAG          0x1D    /* Send Diagnostic (O) */
#define    SCSI_TST_U_RDY          0x00    /* Test Unit Ready (MANDATORY) */
#define    SCSI_WRITE_BUFF         0x3B    /* Write Buffer (O) */

#define    CBWSIGNATURE            0x43425355
#define    CBWFLAGS_OUT            0x00
#define    CBWFLAGS_IN             0x80
#define    CBWCDBLENGTH            16

#define    UMASS_BBB_CBW_SIZE      31

#define    CSWSIGNATURE            0x53425355
#define    CSWSTATUS_GOOD          0x0
#define    CSWSTATUS_FAILED        0x1
#define    CSWSTATUS_PHASE         0x2

#define    UMASS_BBB_CSW_SIZE       13


#define    SCSI_COMPARE            0x39    /* Compare (O)                      */
#define    SCSI_FORMAT             0x04    /* Format Unit (MANDATORY)          */
#define    SCSI_LCK_UN_CAC         0x36    /* Lock Unlock Cache (O)            */
#define    SCSI_PREFETCH           0x34    /* Prefetch (O)                     */
#define    SCSI_MED_REMOVL         0x1E    /* Prevent/Allow medium Removal (O) */
#define    SCSI_READ6              0x08    /* Read 6-byte (MANDATORY)          */
#define    SCSI_READ10             0x28    /* Read 10-byte (MANDATORY)         */
#define    SCSI_RD_FORMAT_CAPAC    0x23
#define    SCSI_RD_CAPAC           0x25    /* Read Capacity (MANDATORY)        */
#define    SCSI_RD_DEFECT          0x37    /* Read Defect Data (O)             */
#define    SCSI_READ_LONG          0x3E    /* Read Long (O)                    */
#define    SCSI_REASS_BLK          0x07    /* Reassign Blocks (O)              */
#define    SCSI_RCV_DIAG           0x1C    /* Receive Diagnostic Results (O)   */
#define    SCSI_RELEASE            0x17    /* Release Unit (MANDATORY)         */
#define    SCSI_REZERO             0x01    /* Rezero Unit (O)                  */
#define    SCSI_SRCH_DAT_E         0x31    /* Search Data Equal (O)            */
#define    SCSI_SRCH_DAT_H         0x30    /* Search Data High (O)             */
#define    SCSI_SRCH_DAT_L         0x32    /* Search Data Low (O)              */
#define    SCSI_SEEK6              0x0B    /* Seek 6-Byte (O)                  */
#define    SCSI_SEEK10             0x2B    /* Seek 10-Byte (O)                 */
#define    SCSI_SEND_DIAG          0x1D    /* Send Diagnostics (MANDATORY)     */
#define    SCSI_SET_LIMIT          0x33    /* Set Limits (O)                   */
#define    SCSI_START_STP          0x1B    /* Start/Stop Unit (O)              */
#define    SCSI_SYNC_CACHE         0x35    /* Synchronize Cache (O)            */
#define    SCSI_VERIFY             0x2F    /* Verify (O)                       */
#define    SCSI_WRITE6             0x0A    /* Write 6-Byte (MANDATORY)         */
#define    SCSI_WRITE10            0x2A    /* Write 10-Byte (MANDATORY)        */
#define    SCSI_WRT_VERIFY         0x2E    /* Write and Verify (O)             */
#define    SCSI_WRITE_LONG         0x3F    /* Write Long (O)                   */
#define    SCSI_WRITE_SAME         0x41    /* Write Same (O)                   */



typedef struct tagSCSI_CMD
{
    uint8       cmd[16];            /* command           */
    uint32      sense[64/4];        /* for request sense */
    uint8       status;             /* SCSI Status       */
    uint8       lun;                /* Target LUN        */
    uint8       cmdlen;             /* command len       */
    uint8       dirin;              /* Is dir in         */
    uint8       *pdata;             /* pointer to data   */
    uint32      datalen;            /* Total data length */
    //uint32    actlen;             /* tranfered bytes   */
} SCSI_CMD, pSCSI_CMD;



typedef    struct tagMSC_HOST
{
    uint8       valid;
    uint8       BulkOut;
    uint8       BulkIn;
    uint8       LunNum;

    uint32      SendCtrlPipe;
    uint32      RecvCtrlPipe;
    uint32      SendBulkPipe;
    uint32      RecvBulkPipe;

    uint8       LunMap[MAX_LUN_NUM];
    uint32      capacity[MAX_LUN_NUM];   //容量

    HOST_DEV    *pDev;
}MSC_HOST, *pMSC_HOST;

#endif


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
extern rk_err_t USBMSCDev_Shell(HDC dev, uint8 * pstr);
extern rk_err_t USBMSCDev_Task_DeInit(void *pvParameters);
extern rk_err_t USBMSCDev_Task_Init(void *pvParameters);
extern void     USBMSCDev_Task_Enter(void);
extern rk_err_t USBMSCHost_Write(HDC dev, uint32 start, void *pBuf, uint32 blks);
extern rk_err_t USBMSCHost_Read(HDC dev, uint32 start, void *pBuf, uint32 blks);
extern rk_err_t UsbMscDev_IntDisalbe(void);

extern rk_err_t USBMSCDev_Delete(uint32 DevID, void * arg);
extern HDC      USBMSCDev_Create(uint32 DevID, void * arg);

extern void     MscScsiCmdHandle(uint8 ep, uint8* buf, uint32 size);
extern rk_err_t UsbMscDev_Connect(HDC dev, uint32 Speed);

/**Host Part**/
extern int32    MscHost(HOST_INTERFACE* interfaceDes);




#endif
