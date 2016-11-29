/*
********************************************************************************************
*
*                  Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Driver\USB\UsbOtgDev.h
* Owner: aaron.sun
* Date: 2014.10.7
* Time: 15:42:21
* Desc: usb otg device
* History:
*    <author>     <date>       <time>     <version>       <Desc>
*    aaron.sun     2014.10.7     15:42:21   1.0
********************************************************************************************
*/

#ifndef __DRIVER_USB_USBOTGDEV_H__
#define __DRIVER_USB_USBOTGDEV_H__

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
#define _DRIVER_USB_USBOTGDEV_COMMON_  __attribute__((section("driver_usb_usbotgdev_common")))
#define _DRIVER_USB_USBOTGDEV_INIT_    __attribute__((section("driver_usb_usbotgdev_init")))
#define _DRIVER_USB_USBOTGDEV_SHELL_   __attribute__((section("driver_usb_usbotgdev_shell")))
#if defined(__arm__) && (__ARMCC_VERSION)
#define _DRIVER_USB_USBOTGDEV_SHELL_DATA_      _DRIVER_USB_USBOTGDEV_SHELL_
#elif defined(__arm__) && (__GNUC__)
#define _DRIVER_USB_USBOTGDEV_SHELL_DATA_      __attribute__((section("driver_usb_usbotgdev_shell_data")))
#else
#error Unknown compiling tools.
#endif

typedef enum _USBOTG_MODE
{
    USBOTG_MODE_DEVICE,
    USBOTG_MODE_HOST,
    USBOTG_MODE_OTG,
    USBOTG_MODE_NUM

}USBOTG_MODE;

//配置描述符集合描述符结构
typedef __packed struct _USB_FSG_CONFIGS_DESCRIPTOR
{
    USB_CONFIGURATION_DESCRIPTOR Config;
    USB_INTERFACE_DESCRIPTOR Interface;
    USB_ENDPOINT_DESCRIPTOR BulkIn;
    USB_ENDPOINT_DESCRIPTOR BulkOut;
} USB_FSG_CONFIGS_DESCRIPTOR, *PUSB_FSG_CONFIGS_DESCRIPTOR;

typedef __packed struct _OTHER_SPEED_CONFIG_DESCRIPTOR
{
    uint8       bLength;
    uint8       bDescriptorType;
    uint16      wTotalLength;
    uint8       bNumInterfaces;
    uint8       bConfigurationValue;
    uint8       iiConfiguration;
    uint8       bmAttributes;
    uint8       MaxPower;
}OTHER_SPEED_CONFIG_DESCRIPTOR;

typedef __packed struct _USB_STRING_DESCRIPTOR
{
    uint8       bLength;
    uint8       bDescriptorType;
    uint8       bString[1];

}USB_STRING_DESCRIPTOR;


typedef __packed struct _HS_DEVICE_QUALIFIER
{
    uint8       bLength;          //length of HS Device Descriptor
    uint8       bQualifier;       //HS Device Qualifier Type
    uint16      wVersion;         // USB 2.0 version
    uint8       bDeviceClass;     //Device class
    uint8       bDeviceSubClasss; //Device SubClass
    uint8       bProtocol;        //Device Protocol Code
    uint8       MaxPktSize;       //Maximum Packet SIze for other speed
    uint8       bOther_Config;    //Number of Other speed configurations
    uint8       Reserved;         //Reserved

}HS_DEVICE_QUALIFIER;

typedef __packed struct _USB_POWER_DESCRIPTOR
{
    uint8       bLength;
    uint8       bDescriptorType;
    uint8       bCapabilitiesFlags;
    uint16      EventNotification;
    uint16      D1LatencyTime;
    uint16      D2LatencyTime;
    uint16      D3LatencyTime;
    uint8       PowerUnit;
    uint16      D0PowerConsumption;
    uint16      D1PowerConsumption;
    uint16      D2PowerConsumption;

}USB_POWER_DESCRIPTOR;

typedef __packed struct _USB_HUB_DESCRIPTOR
{
    uint8       bDescriptorLength;        // Length of this descriptor
    uint8       bDescriptorType;          // Hub configuration type
    uint8       bNumberOfPorts;           // number of ports on this hub
    uint16      wHubCharacteristics;      // Hub Charateristics
    uint8       bPowerOnToPowerGood;      // port power on till power good in 2ms
    uint8       bHubControlCurrent;       // max current in mA
    // room for 255 ports power control and removable bitmask
    uint8       bRemoveAndPowerMask[64];

}USB_HUB_DESCRIPTOR;

typedef struct _VERIFY_PARAMS
{
    uint32 number;
    uint8 dir;
    uint8 bulk;
    uint8 status;
}VERIFY_PARAMS;

#if 1

typedef struct tagUSB_DESC_HEADER {
    uint8       bLength;
    uint8       bDescriptorType;
} USB_DESC_HEADER;


typedef enum DWC_OTG_CTRL_PHASE {
    DWC_OTG_CONTROL_SETUP,
    DWC_OTG_CONTROL_DATA,
    DWC_OTG_CONTROL_STATUS
} DWC_OTG_CTRL_PHASE_e;

enum USB_DEV_STATE
{
    /* NOTATTACHED isn't in the USB spec, and this state acts
     * the same as ATTACHED ... but it's clearer this way.
     */
    USB_STATE_NOTATTACHED = 0,

    /* chapter 9 and authentication (wireless) device states */
    USB_STATE_ATTACHED,
    USB_STATE_POWERED,            /* wired            */
    USB_STATE_RECONNECTING,        /* auth             */
    USB_STATE_UNAUTHENTICATED,  /* auth             */
    USB_STATE_DEFAULT,            /* limited function */
    USB_STATE_ADDRESS,
    USB_STATE_CONFIGURED,        /* most functions   */

    USB_STATE_SUSPENDED

    /* NOTE:  there are actually four different SUSPENDED
     * states, returning to POWERED, DEFAULT, ADDRESS, or
     * CONFIGURED respectively when SOF tokens flow again.
     * At this level there's no difference between L1 and L2
     * suspend states.  (L2 being original USB 1.1 suspend.)
     */
};

#endif

/*------------------------------ Global Defines ------------------------------*/
#if 1
#define    USB_CNTL_TIMEOUT            100    /* 100ms timeout */
#define    USB_CTRL_GET_TIMEOUT           5000
#define    USB_CTRL_SET_TIMEOUT           5000

#define    USB_MAXCONFIG               1
#define    USB_MAXINTERFACES           1

#define    MAX_USB_DEV                 1
#define    MAX_USB_DRV                 1
#define    MAX_HOST_EP                 4
#define    MAX_USB_REQ                 1

#define    USBPipeEpNum(pipe)          ((pipe>>11) & 0xf)

#define    USBPipeDir(pipe)            ((pipe>>15) & 0x1)

#define    USBIsCtrlPipe(pipe)         (!((pipe>>16) & 0x1))

#define    USBPipeType(pipe)           ((pipe>>16) & 0x3)

#define    USBPipeDev(pipe)            ((pipe>>18) & 0x7f)


#define    USBEpNum(ep)    (ep->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK)

#define    USBEpType(ep)   (ep->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK)

#define    USBEpDirIn(ep)  ((ep->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_IN)


#define    HostCleanEpStall(pDev, pipe, wIndex)    \
           MscHostClearFeature(pDev, pipe, USB_RECIP_ENDPOINT, USB_ENDPOINT_HALT, wIndex)


#define    USBGetToggle(dev, ep, out)   (((dev)->toggle[out] >> (ep)) & 1)

#define       USBDoToggle (dev, ep, out)   ((dev)->toggle[out] ^= (1 << (ep)))

#define    USBSetToggle(dev, ep, out, bit) \
           ((dev)->toggle[out] = ((dev)->toggle[out] & ~(1 << (ep))) | \
           ((bit) << (ep)))

#endif

#define VERIFY_NUMBER                           6824237
#define VERIFY_SIZE                             sizeof(VERIFY_PARAMS)


#define USB_DIR_OUT                             0       /* to device */
#define USB_DIR_IN                              0x80    /* to host   */

/* Sub Classes */
#define US_SC_RBC                               0x01    /* Typically, flash devices */
#define US_SC_8020                              0x02    /* CD-ROM                   */
#define US_SC_QIC                               0x03    /* QIC-157 Tapes            */
#define US_SC_UFI                               0x04    /* Floppy                   */
#define US_SC_8070                              0x05    /* Removable media          */
#define US_SC_SCSI                              0x06    /* Transparent              */
#define US_SC_LOCKABLE                          0x07    /* Password-protected       */

#define US_SC_ISD200                            0xf0    /* ISD200 ATA               */
#define US_SC_CYP_ATACB                         0xf1    /* Cypress ATACB            */
#define US_SC_DEVICE                            0xff    /* Use device's value       */

/* Protocols */
#define US_PR_CBI                               0x00    /* Control/Bulk/Interrupt     */
#define US_PR_CB                                0x01    /* Control/Bulk w/o interrupt */
#define US_PR_BULK                              0x50    /* bulk only                  */

/** 0: Control, 1: Isoc, 2: Bulk, 3: Intr */
#define USB_EPTYPE_CTRL                         0
#define USB_EPTYPE_ISOC                         1
#define USB_EPTYPE_BULK                         2
#define USB_EPTYPE_INTR                         3

#define USB_ENDPOINT_NUMBER_MASK                0x0f    /* in bEndpointAddress */
#define USB_ENDPOINT_DIR_MASK                   0x80

/*
 * USB types, the second of three bRequestType fields
 */
#define USB_TYPE_MASK                           (0x03 << 5)
#define USB_TYPE_STANDARD                       (0x00 << 5)
#define USB_TYPE_CLASS                          (0x01 << 5)
#define USB_TYPE_VENDOR                         (0x02 << 5)
#define USB_TYPE_RESERVED                       (0x03 << 5)

/*
 * USB recipients, the third of three bRequestType fields
 */
#define USB_RECIP_MASK                          0x1f
#define USB_RECIP_DEVICE                        0x00
#define USB_RECIP_INTERFACE                     0x01
#define USB_RECIP_ENDPOINT                      0x02
#define USB_RECIP_OTHER                         0x03


#define USB_DT_DEVICE_SIZE                      18

/*
 * Descriptor types ... USB 2.0 spec table 9.5
 */
#define USB_DT_DEVICE                           0x01
#define USB_DT_CONFIG                           0x02
#define USB_DT_STRING                           0x03
#define USB_DT_INTERFACE                        0x04
#define USB_DT_ENDPOINT                         0x05
#define USB_DT_DEVICE_QUALIFIER                 0x06
#define USB_DT_OTHER_SPEED_CONFIG               0x07
#define USB_DT_INTERFACE_POWER                  0x08
/* these are from a minor usb 2.0 revision (ECN) */
#define USB_DT_OTG                              0x09
#define USB_DT_DEBUG                            0x0a
#define USB_DT_INTERFACE_ASSOCIATION            0x0b
/* these are from the Wireless USB spec */
#define USB_DT_SECURITY                         0x0c
#define USB_DT_KEY                              0x0d
#define USB_DT_ENCRYPTION_TYPE                  0x0e
#define USB_DT_BOS                              0x0f
#define USB_DT_DEVICE_CAPABILITY                0x10
#define USB_DT_WIRELESS_ENDPOINT_COMP           0x11
#define USB_DT_WIRE_ADAPTER                     0x21
#define USB_DT_RPIPE                            0x22
#define USB_DT_CS_RADIO_CONTROL                 0x23
/* From the USB 3.0 spec */
#define    USB_DT_SS_ENDPOINT_COMP              0x30

/*
 * Standard requests, for the bRequest field of a SETUP packet.
 *
 * These are qualified by the bRequestType field, so that for example
 * TYPE_CLASS or TYPE_VENDOR specific feature flags could be retrieved
 * by a GET_STATUS request.
 */

#define USB_REQ_MASK                     (uint8)0x0F
#define USB_REQ_GET_STATUS                      0x00
#define USB_REQ_CLEAR_FEATURE                   0x01
#define USB_REQ_SET_FEATURE                     0x03
#define USB_REQ_SET_ADDRESS                     0x05
#define USB_REQ_GET_DESCRIPTOR                  0x06
#define USB_REQ_SET_DESCRIPTOR                  0x07
#define USB_REQ_GET_CONFIGURATION               0x08
#define USB_REQ_SET_CONFIGURATION               0x09
#define USB_REQ_GET_INTERFACE                   0x0A
#define USB_REQ_SET_INTERFACE                   0x0B
#define USB_REQ_SYNCH_FRAME                     0x0C

#define USB_REQ_SET_ENCRYPTION                  0x0D    /* Wireless USB */
#define USB_REQ_GET_ENCRYPTION                  0x0E
#define USB_REQ_RPIPE_ABORT                     0x0E
#define USB_REQ_SET_HANDSHAKE                   0x0F
#define USB_REQ_RPIPE_RESET                     0x0F
#define USB_REQ_GET_HANDSHAKE                   0x10
#define USB_REQ_SET_CONNECTION                  0x11
#define USB_REQ_SET_SECURITY_DATA               0x12
#define USB_REQ_GET_SECURITY_DATA               0x13
#define USB_REQ_SET_WUSB_DATA                   0x14
#define USB_REQ_LOOPBACK_DATA_WRITE             0x15
#define USB_REQ_LOOPBACK_DATA_READ              0x16
#define USB_REQ_SET_INTERFACE_DS                0x17

/*
 * USB feature flags are written using USB_REQ_{CLEAR,SET}_FEATURE, and
 * are read as a bit array returned by USB_REQ_GET_STATUS.  (So there
 * are at most sixteen features of each type.)  Hubs may also support a
 * new USB_REQ_TEST_AND_SET_FEATURE to put ports into L1 suspend.
 */
#define USB_DEVICE_SELF_POWERED                 0    /* (read only)                */
#define USB_DEVICE_REMOTE_WAKEUP                1    /* dev may initiate wakeup    */
#define USB_DEVICE_TEST_MODE                    2    /* (wired high speed only)    */
#define USB_DEVICE_BATTERY                      2    /* (wireless)                 */
#define USB_DEVICE_B_HNP_ENABLE                 3    /* (otg) dev may initiate HNP */
#define USB_DEVICE_WUSB_DEVICE                  3    /* (wireless)                 */
#define USB_DEVICE_A_HNP_SUPPORT                4    /* (otg) RH port supports HNP */
#define USB_DEVICE_A_ALT_HNP_SUPPORT            5    /* (otg) other RH port does   */
#define USB_DEVICE_DEBUG_MODE                   6    /* (special devices only)     */

#define USB_ENDPOINT_HALT                       0    /* IN/OUT will STALL          */

/*
 * Device and/or Interface Class codes
 * as found in bDeviceClass or bInterfaceClass
 * and defined by www.usb.org documents
 */
#define USB_CLASS_PER_INTERFACE                 0    /* for DeviceClass */
#define USB_CLASS_AUDIO                         1    //音频设备
#define USB_CLASS_COMM                          2    //通讯设备
#define USB_CLASS_HID                           3    //人机接口
#define USB_CLASS_PHYSICAL                      5    //物理接口
#define USB_CLASS_STILL_IMAGE                   6    //电源
#define USB_CLASS_PRINTER                       7    //打印机
#define USB_CLASS_MASS_STORAGE                  8    //海量存储
#define USB_CLASS_HUB                           9    //HUB

#define USB_CLASS_CDC_DATA                      0x0a
#define USB_CLASS_CSCID                         0x0b    /* chip+ smart card */
#define USB_CLASS_CONTENT_SEC                   0x0d    /* content security */
#define USB_CLASS_VIDEO                         0x0e
#define USB_CLASS_WIRELESS_CONTROLLER           0xe0
#define USB_CLASS_MISC                          0xef
#define USB_CLASS_APP_SPEC                      0xfe
#define USB_CLASS_VENDOR_SPEC                   0xff


#define USB_SUBCLASS_VENDOR_SPEC                0xff

#define USB_DT_ENDPOINT_SIZE                    7
#define USB_DT_ENDPOINT_AUDIO_SIZE              9       /* Audio extension */

#define USB_DT_CONFIG_SIZE                      9

/*
 * Endpoints
 */

#define USB_ENDPOINT_SYNCTYPE                   0x0c
#define USB_ENDPOINT_SYNC_NONE                  (0 << 2)
#define USB_ENDPOINT_SYNC_ASYNC                 (1 << 2)
#define USB_ENDPOINT_SYNC_ADAPTIVE              (2 << 2)
#define USB_ENDPOINT_SYNC_SYNC                  (3 << 2)

#define USB_ENDPOINT_XFERTYPE_MASK              0x03    /* in bmAttributes */
#define USB_ENDPOINT_XFER_CONTROL               0
#define USB_ENDPOINT_XFER_ISOC                  1
#define USB_ENDPOINT_XFER_BULK                  2
#define USB_ENDPOINT_XFER_INT                   3
#define USB_ENDPOINT_MAX_ADJUSTABLE             0x80

#define USB_DT_INTERFACE_SIZE                   9

#define USB_DT_CONFIG_SIZE                      9

/* from config descriptor bmAttributes */
#define USB_CONFIG_ATT_ONE                      (1 << 7) /* must be set     */
#define USB_CONFIG_ATT_SELFPOWER                (1 << 6) /* self powered    */
#define USB_CONFIG_ATT_WAKEUP                   (1 << 5) /* can wakeup      */
#define USB_CONFIG_ATT_BATTERY                  (1 << 4) /* battery powered */


#define USB_STRING_MANUFACTURER                 1
#define USB_STRING_PRODUCT                      2
#define USB_STRING_SERIAL                       3
#define USB_STRING_CONFIG                       4
#define USB_STRING_INTERFACE                    5


#define EP0_PACKET_SIZE                         64

#define DEVICE_MODE                             0
#define HOST_MODE                               1


typedef struct _USBOTG_DEV_ARG
{
    uint32 usbmode;
    uint32 usbspeed;

}USBOTG_DEV_ARG;


typedef  struct _USBOGT_DESCRIPTOR
{
    USB_DEVICE_DESCRIPTOR * pDevDes;
    USB_CONFIGURATION_DESCRIPTOR * pConfigDes;
    USB_INTERFACE_DESCRIPTOR * pInterfaceDes;
    USB_ENDPOINT_DESCRIPTOR * pEnDes[3];
    OTHER_SPEED_CONFIG_DESCRIPTOR * pOtherConfigDes;
    HS_DEVICE_QUALIFIER * pHsQualifier;
    USB_POWER_DESCRIPTOR * pUsbPowerDes;
    USB_HUB_DESCRIPTOR * pUsbHubDes;
    USB_STRING_DESCRIPTOR * pUsbLangId;
    USB_STRING_DESCRIPTOR * pDevString;
    USB_STRING_DESCRIPTOR * pMString;
    USB_STRING_DESCRIPTOR * pSerialNum;
    USB_STRING_DESCRIPTOR * pConfigString;
    USB_STRING_DESCRIPTOR * pInterfaceString;
    void (*pfReadEp)(uint32 ep, uint8 * buf, uint32 size);
    uint32 ConfigLen;
    /**Host Part**/
    int32 (*pMscHost)(HOST_INTERFACE* interfaceDes);

}USBOGT_DESCRIPTOR;


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
extern rk_err_t UsbOtgDev_SetDes(HDC dev, USBOGT_DESCRIPTOR * pDes);
extern rk_err_t UsbOtgDev_Connect(HDC dev, uint32 Speed);
extern rk_err_t UsbOtgDev_Shell(HDC dev, uint8 * pstr);
extern rk_err_t UsbOtgDev0_Task_DeInit(void *pvParameters);
extern rk_err_t UsbOtgDev0_Task_Init(void *pvParameters);
extern rk_err_t UsbOtgDev_Write(HDC dev, uint8 ep, uint8 * buf, uint32 size);
extern rk_err_t UsbOtgDev_Delete(uint32 DevID, void * arg);
extern rk_err_t UsbOtgDevResume(HDC dev);
extern rk_err_t UsbOtgDevSuspend(HDC dev);
extern rk_err_t UsbOtgDev_IntDisalbe(void);
extern uint8    UsbOtgDev_GetEnumStatus(HDC dev);
extern uint8    UsbOtgDev_SetEnumStatus(HDC dev, uint8 enum_status);

extern void     UsbOtgDev0_Task_Enter(void);
extern void     UsbOtgDevIntIsr0(void);
extern void     UsbOtgDevIntIsr1(void);

extern HDC      UsbOtgDev_Create(uint32 DevID, void * arg);



#if 1
//Host
extern rk_err_t UsbOtgDev_DisConnect(void);
extern rk_err_t UsbOtgHost_Task_DeInit(void *pvParameters);
extern rk_err_t UsbOtgHost_Task_Init(void *pvParameters);
extern void     UsbOtgHost_Task_Enter(void);
extern void     OtgHostnptxfemp(int32 ret);
extern int32    HostBulkMsg(HOST_DEV *pDev, uint32 pipe, void * data, int len, int timeout);
extern uint32   HostCreatePipe(HOST_DEV * dev, uint8 epnum, uint8 epdir);


static USB_REQ* HostAllocUrb(void);
static void     HostEp0ReInit(HOST_DEV *pDev, uint16 wMaxPacketSize);
static int32    HostGetDesc(HOST_DEV *pDev, uint8 type, uint8 index, void *buf, int size);
static void     HostFreeUrb(USB_REQ *urb);
static int32    HostSetAddr(HOST_DEV *pDev);
static int32    HostPortInit(HOST_DEV *pHDev);
static int32    HostNewDev(HOST_DEV *pHDev);
static int32    HostParseConfig(HOST_DEV *pHDev, uint8* buf, uint32 size);
static int32    HostParseInterface(HOST_DEV *pDev, uint8* buf, uint32 size);
static int32    HostSetConfig(HOST_DEV *pDev, uint16 config);

#endif


#endif

