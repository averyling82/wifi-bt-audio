/*
********************************************************************************************
*
*                  Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Cpu\NanoC\lib\usb.h
* Owner: aaron.sun
* Date: 2014.9.24
* Time: 16:16:25
* Desc: usb bsp
* History:
*    <author>     <date>       <time>     <version>       <Desc>
*    aaron.sun     2014.9.24     16:16:25   1.0
********************************************************************************************
*/

#ifndef __CPU_NANOD_LIB_USB_H__
#define __CPU_NANOD_LIB_USB_H__

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
#define _CPU_NANOD_LIB_USB_COMMON_  __attribute__((section("cpu_nanod_lib_usb_common")))
#define _CPU_NANOD_LIB_USB_INIT_    __attribute__((section("cpu_nanod_lib_usb_init")))
#define _CPU_NANOD_LIB_USB_SHELL_   __attribute__((section("cpu_nanod_lib_usb_shell")))


#define USB_INT_TYPE_EARLY_SUSPEND             (1 << 0)
#define USB_INT_TYPE_SUSPEND                   (1 << 1)
#define USB_INT_TYPE_RESET                     (1 << 2)
#define USB_INT_TYPE_E_DONE                    (1 << 3)
#define USB_INT_TYPE_VBUS                      (1 << 4)
#define USB_INT_TYPE_RESUME                    (1 << 5)
#define USB_INT_TYPE_EP_IN                     (1 << 6)
#define USB_INT_TYPE_EP_OUT                    (1 << 7)
#define USB_INT_TYPE_RXFIFO_N_EMPTY            (1 << 8)
#define USB_INT_TYPE_NP_TXFIFO_EMPTY           (1 << 9)


#define USB_RX_INT_TYPE_GLOBAL_OUT_NAK         (1 << 0)
#define USB_RX_INT_TYPE_OUT_RECIVE_PACKAGE     (1 << 1)
#define USB_RX_INT_TYPE_SETUP_RECIVE_PACKAGE   (1 << 2)
#define USB_RX_INT_TYPE_OUT_TRANS_CMPL         (1 << 3)
#define USB_RX_INT_TYPE_SETUP_TRANS_CMPL       (1 << 4)


#define USB_OUT_EP_INT_TYPE_TRANS_OK           (1 << 0)
#define USB_OUT_EP_INT_TYPE_EP_DISABLE         (1 << 1)
#define USB_OUT_EP_INT_TYPE_AHB_ERR            (1 << 2)
#define USB_OUT_EP_INT_TYPE_SETUP_PHASE_OVER   (1 << 3)

#define USB_IN_EP_INT_TYPE_TRANS_OK            (1 << 0)
#define USB_IN_EP_INT_TYPE_TIME_OUT            (1 << 1)
#define USB_IN_EP_INT_TYPE_EP_DISABLE          (1 << 2)
#define USB_IN_EP_INT_TYPE_AHB_ERR             (1 << 3)
#define USB_IN_EP_INT_TYPE_SETUP_PHASE_OVER    (1 << 4)

/*********************Host****************************/
#define NORMAL_MODE                            0
#define FORCE_DEVICE_MODE                      1
#define FORCE_HOST_MODE                        2
#if 1
#define     MAX_HOST_CHN                       1
#define     HC_CHN_FIFO(chn)                   (USB_REG_BASE +((chn+1)<<12))

#define     DWC_OTG_HC_PID_DATA0               0
#define     DWC_OTG_HC_PID_DATA2               1
#define     DWC_OTG_HC_PID_DATA1               2
#define     DWC_OTG_HC_PID_MDATA               3
#define     DWC_OTG_HC_PID_SETUP               3

#define     DWC_OTG_EP_TYPE_CONTROL               0
#define     DWC_OTG_EP_TYPE_ISOC               1
#define     DWC_OTG_EP_TYPE_BULK               2
#define     DWC_OTG_EP_TYPE_INTR               3

#define     DWC_OTG_EP_SPEED_LOW               0
#define     DWC_OTG_EP_SPEED_FULL               1
#define     DWC_OTG_EP_SPEED_HIGH               2

#define     HOST_DISCONNECT                    1
#define     HOST_PORT_INT                      2


typedef union tagGINTMSK_DATA
{
    /** raw register data */
    uint32 d32;
    /** register bits     */
    struct
    {
        unsigned curmod : 1;
        unsigned modemis : 1;
        unsigned otgint : 1;
        unsigned sof : 1;
        unsigned rxflvl : 1;
        unsigned nptxfemp : 1;
        unsigned ginnakeff : 1;
        unsigned goutnakeff : 1;
        unsigned reserved8 : 1;
        unsigned i2cint : 1;
        unsigned erlysusp : 1;
        unsigned usbsusp : 1;
        unsigned usbrst : 1;
        unsigned enumdone : 1;
        unsigned isooutdrop : 1;
        unsigned eopf : 1;
        unsigned reserved16_20 : 5;
        unsigned incompip : 1;
        unsigned reserved22_23 : 2;
        unsigned prtint : 1;
        unsigned hchint : 1;
        unsigned ptxfemp : 1;
        unsigned reserved27 : 1;
        unsigned conidstschng : 1;
        unsigned disconnint : 1;
        unsigned sessreqint : 1;
        unsigned wkupint : 1;
    } b;
} GINTMSK_DATA;

typedef union tagGAHBCFG_DATA
{
    /** raw register data */
    uint32 d32;
    /** register bits     */
    struct
    {
        unsigned glblintrmsk : 1;
#define DWC_GAHBCFG_GLBINT_ENABLE        1

        unsigned hbstlen : 4;
#define DWC_GAHBCFG_INT_DMA_BURST_SINGLE    0
#define DWC_GAHBCFG_INT_DMA_BURST_INCR        1
#define DWC_GAHBCFG_INT_DMA_BURST_INCR4        3
#define DWC_GAHBCFG_INT_DMA_BURST_INCR8        5
#define DWC_GAHBCFG_INT_DMA_BURST_INCR16    7

        unsigned dmaen : 1;
#define DWC_GAHBCFG_DMAENABLE            1
        unsigned reserved : 1;
        unsigned nptxfemplvl : 1;
        unsigned ptxfemplvl : 1;
#define DWC_GAHBCFG_TXFEMPTYLVL_EMPTY        1
#define DWC_GAHBCFG_TXFEMPTYLVL_HALFEMPTY    0
        unsigned reserved9_31 : 23;
    } b;
} GAHBCFG_DATA;

typedef union tagGINTSTS_DATA
{
    /** raw register data */
    uint32 d32;
#define DWC_SOF_INTR_MASK 0x0008
    /** register bits     */
    struct
    {
#define DWC_DEVICE_MODE 0
#define DWC_HOST_MODE 1
        unsigned curmod : 1;
        unsigned modemis : 1;
        unsigned otgint : 1;
        unsigned sof : 1;
        unsigned rxflvl : 1;
        unsigned nptxfemp : 1;
        unsigned ginnakeff : 1;
        unsigned goutnakeff : 1;
        unsigned reserved8 : 1;
        unsigned i2cint : 1;
        unsigned erlysusp : 1;
        unsigned usbsusp : 1;
        unsigned usbrst : 1;
        unsigned enumdone : 1;
        unsigned isooutdrop : 1;
        unsigned eopf : 1;
        unsigned reserved16_20 : 5;
        unsigned incompip : 1;
        unsigned reserved22_23 : 2;
        unsigned prtint : 1;
        unsigned hchint : 1;
        unsigned ptxfemp : 1;
        unsigned reserved27 : 1;
        unsigned conidstschng : 1;
        unsigned disconnint : 1;
        unsigned sessreqint : 1;
        unsigned wkupint : 1;
    } b;
} GINTSTS_DATA;

typedef union tagHPRT0_DATA
{
        /** raw register data */
        uint32 d32;
        /** register bits     */
        struct {
        unsigned prtconnsts : 1;
        unsigned prtconndet : 1;
        unsigned prtena : 1;
        unsigned prtenchng : 1;
        unsigned prtovrcurract : 1;
        unsigned prtovrcurrchng : 1;
        unsigned prtres : 1;
        unsigned prtsusp : 1;
        unsigned prtrst : 1;
        unsigned reserved9 : 1;
        unsigned prtlnsts : 2;
        unsigned prtpwr : 1;
        unsigned prttstctl : 4;
        unsigned prtspd : 2;
#define DWC_HPRT0_PRTSPD_HIGH_SPEED 0
#define DWC_HPRT0_PRTSPD_FULL_SPEED 1
#define DWC_HPRT0_PRTSPD_LOW_SPEED    2
        unsigned reserved19_31 : 13;
        } b;
} HPRT0_DATA;

typedef union tagGRXSTSH_DATA
{
    /** raw register data */
    uint32 d32;
    /** register bits     */
    struct {
        unsigned chnum : 4;
        unsigned bcnt : 11;
        unsigned dpid : 2;

        unsigned pktsts : 4;
#define DWC_GRXSTS_PKTSTS_IN              0x2
#define DWC_GRXSTS_PKTSTS_IN_XFER_COMP      0x3
#define DWC_GRXSTS_PKTSTS_DATA_TOGGLE_ERR 0x5
#define DWC_GRXSTS_PKTSTS_CH_HALTED          0x7

        unsigned reserved : 11;
    } b;
} GRXSTSH_DATA;

typedef union tagGNPTXSTS_DATA
{
    /** raw register data */
    uint32 d32;
    /** register bits     */
    struct {
        unsigned nptxfspcavail : 16;
        unsigned nptxqspcavail : 8;
        /** Top of the Non-Periodic Transmit Request Queue
         *    - bit 24 - Terminate (Last entry for the selected
         *      channel/EP)
         *    - bits 26:25 - Token Type
         *      - 2'b00 - IN/OUT
         *      - 2'b01 - Zero Length OUT
         *      - 2'b10 - PING/Complete Split
         *      - 2'b11 - Channel Halt
         *    - bits 30:27 - Channel/EP Number
         */
        unsigned nptxqtop_terminate : 1;
        unsigned nptxqtop_token : 2;
        unsigned nptxqtop_chnep : 4;
        unsigned reserved : 1;
        } b;
} GNPTXSTS_DATA;

typedef union tagHCCHAR_DATA
{
        /** raw register data */
        uint32 d32;

        /** register bits     */
    struct {
        /** Maximum packet size in bytes */
        unsigned mps : 11;

        /** Endpoint number   */
        unsigned epnum : 4;

        /** 0: OUT, 1: IN     */
        unsigned epdir : 1;

        unsigned reserved : 1;

        /** 0: Full/high speed device, 1: Low speed device */
        unsigned lspddev : 1;

        /** 0: Control, 1: Isoc, 2: Bulk, 3: Intr          */
        unsigned eptype : 2;

        /** Packets per frame for periodic transfers. 0 is reserved. */
        unsigned multicnt : 2;

        /** Device address */
        unsigned devaddr : 7;

        /**
         * Frame to transmit periodic transaction.
         * 0: even, 1: odd
         */
        unsigned oddfrm : 1;

        /** Channel disable */
        unsigned chdis : 1;

        /** Channel enable  */
        unsigned chen : 1;
    } b;
} HCCHAR_DATA;

typedef union tagHOST_PIPE
{
    /** raw register data */
    uint32 d32;

        /** register bits */
    struct
    {
        /** Maximum packet size in bytes */
        unsigned mps : 11;

        /** Endpoint number */
        unsigned epnum : 4;

        /** 0: OUT, 1: IN */
        unsigned epdir : 1;

        /** 0: Control, 1: Isoc, 2: Bulk, 3: Intr */
        unsigned eptype : 2;

        /** Device address */
        unsigned devaddr : 7;

        //unsigned pid : 2;

        unsigned reserved25_31 : 7;

    } b;
} HOST_PIPE;

typedef union tagHCINTMSK_DATA
{
        /** raw register data */
        uint32 d32;

        /** register bits */
        struct {
        unsigned xfercompl : 1;
        unsigned chhltd : 1;
        unsigned ahberr : 1;
        unsigned stall : 1;
        unsigned nak : 1;
        unsigned ack : 1;
        unsigned nyet : 1;
        unsigned xacterr : 1;
        unsigned bblerr : 1;
        unsigned frmovrun : 1;
        unsigned datatglerr : 1;
        unsigned reserved : 21;
    } b;
} HCINTMSK_DATA;

typedef union tagHCTSIZ_DATA
{
        /** raw register data */
        uint32 d32;

        /** register bits     */
        struct {
        /** Total transfer size in bytes */
        unsigned xfersize : 19;

        /** Data packets to transfer     */
        unsigned pktcnt : 10;

        /**
         * Packet ID for next data packet
         * 0: DATA0
         * 1: DATA2
         * 2: DATA1
         * 3: MDATA (non-Control), SETUP (Control)
         */
        unsigned pid : 2;
#define DWC_HCTSIZ_DATA0 0
#define DWC_HCTSIZ_DATA1 2
#define DWC_HCTSIZ_DATA2 1
#define DWC_HCTSIZ_MDATA 3
#define DWC_HCTSIZ_SETUP 3

        /** Do PING protocol when 1 */
        unsigned dopng : 1;
        } b;
} HCTSIZ_DATA;

typedef union tagHCINT_DATA
{
        /** raw register data  */
        uint32 d32;
        /** register bits      */
    struct {
        /** Transfer Complete  */
        unsigned xfercomp : 1;
        /** Channel Halted     */
        unsigned chhltd : 1;
        /** AHB Error */
        unsigned ahberr : 1;
        /** STALL Response Received */
        unsigned stall : 1;
        /** NAK Response Received   */
        unsigned nak : 1;
        /** ACK Response Received   */
        unsigned ack : 1;
        /** NYET Response Received  */
        unsigned nyet : 1;
        /** Transaction Err         */
        unsigned xacterr : 1;
        /** Babble Error            */
        unsigned bblerr : 1;
        /** Frame Overrun           */
        unsigned frmovrun : 1;
        /** Data Toggle Error       */
        unsigned datatglerr : 1;
        /** Reserved                */
        unsigned reserved : 21;
    } b;
} HCINT_DATA;


typedef struct _USB_CTRL_REQUEST
{
    uint8   bRequestType;
    uint8   bRequest;
    uint16  wValue;
    uint16  wIndex;
    uint16  wLength;

} USB_CTRL_REQUEST;

typedef __packed struct _USB_DEVICE_DESCRIPTOR
{

    uint8       bLength;
    uint8       bDescriptorType;
    uint16      bcdUSB;
    uint8       bDeviceClass;
    uint8       bDeviceSubClass;
    uint8       bDeviceProtocol;
    uint8       bMaxPacketSize0;
    uint16      idVendor;
    uint16      idProduct;
    uint16      bcdDevice;
    uint8       iiManufacturer;
    uint8       iiProduct;
    uint8       iiSerialNumber;
    uint8       bNumConfigurations;

}USB_DEVICE_DESCRIPTOR;

typedef __packed  struct _USB_CONFIGURATION_DESCRIPTOR
{
    uint8       bLength;
    uint8       bDescriptorType;
    uint16      wTotalLength;
    uint8       bNumInterfaces;
    uint8       bConfigurationValue;
    uint8       iiConfiguration;
    uint8       bmAttributes;
    uint8       MaxPower;
}USB_CONFIGURATION_DESCRIPTOR;

typedef __packed struct _USB_INTERFACE_DESCRIPTOR
{
    uint8       bLength;
    uint8       bDescriptorType;
    uint8       bInterfaceNumber;
    uint8       bAlternateSetting;
    uint8       bNumEndpoints;
    uint8       bInterfaceClass;
    uint8       bInterfaceSubClass;
    uint8       bInterfaceProtocol;
    uint8       iiInterface;
}USB_INTERFACE_DESCRIPTOR;


typedef __packed struct _USB_ENDPOINT_DESCRIPTOR
{
    uint8       bLength;
    uint8       bDescriptorType;
    uint8       bEndpointAddress;
    uint8       bmAttributes;
    uint16      wMaxPacketSize;
    uint8       bInterval;
}USB_ENDPOINT_DESCRIPTOR;

typedef    struct tagHOST_INTERFACE
{
    USB_INTERFACE_DESCRIPTOR        IfDesc;
    USB_ENDPOINT_DESCRIPTOR         EpDesc[4];
    struct tagHOST_DEV  *pDev;
}HOST_INTERFACE, *pHOST_INTERFACE;


typedef    struct tagHOST_DEV
{
    uint8               valid;
    uint8               DevNum;
    uint8               speed;
    uint8               state;
    uint16              toggle[2];  /*one bit for each endpoint   [0] = OUT, [1] = IN*/
    USB_ENDPOINT_DESCRIPTOR        ep0;
    USB_DEVICE_DESCRIPTOR          DevDesc;
    USB_CONFIGURATION_DESCRIPTOR   CfgDesc;
    HOST_INTERFACE       intf;

    USB_ENDPOINT_DESCRIPTOR        *EpIn[16];
    USB_ENDPOINT_DESCRIPTOR        *EpOut[16];

}HOST_DEV, *pHOST_DEV;

typedef    struct tagUSB_REQ
{
    uint8               valid;
    HOST_DEV            *pDev;
    HOST_PIPE           pipe;
    USB_CTRL_REQUEST    *SetupPkt;
    uint8               CtrlPhase;
    void                *TransBuf;
    uint32              BufLen;
    uint32              ActualLen;
    uint32              StatusBuf[1];
    uint8               DataToggle;
    /** Ping state if 1. */
    uint8               PingState;
    uint8                 ErrCnt;
    int32               status;
    uint8               completed;
}USB_REQ, *pUSB_REQ;


typedef    volatile struct tagUHC_CHN_INFO
{
    uint8           chn;
    uint8           valid;

    /** Device to access            */
    uint8           DevAddr;
    /** EP to access                */
    uint8           EpNum;
    /** EP direction. 0: OUT, 1: IN */
    uint8           EpIsIn;
    /**
     * EP speed.
     * One of the following values:
     *    - DWC_OTG_EP_SPEED_LOW
     *    - DWC_OTG_EP_SPEED_FULL
     *    - DWC_OTG_EP_SPEED_HIGH
     */
    uint8           speed;
    /**
     * Endpoint type.
     * One of the following values:
     *    - DWC_OTG_EP_TYPE_CONTROL: 0
     *    - DWC_OTG_EP_TYPE_ISOC: 1
     *    - DWC_OTG_EP_TYPE_BULK: 2
     *    - DWC_OTG_EP_TYPE_INTR: 3
     */
    uint8           EpType;
    /** Max packet size in bytes */
    uint16          MaxPkt;
    /**
     * PID for initial transaction.
     * 0: DATA0,<br>
     * 1: DATA2,<br>
     * 2: DATA1,<br>
     * 3: MDATA (non-Control EP),
     *      SETUP (Control EP)
     */
    uint8           PidStart;

    uint8           *XferBuf;              //中断数据接收或发送数据用到的buf地址
    uint32          XferLen;               //需要传输的数据长度
    uint32          XferCnt;               //已经发送或接收的数据长度
    /** Packet count at start of transfer.*/
    uint16          StartPktCnt;
    /**
    * Flag to indicate whether the transfer has been started. Set to 1 if
    * it has been started, 0 otherwise.
    */
    uint8           XferStarted;
    /**
     * Set to 1 to indicate that a PING request should be issued on this
     * channel. If 0, process normally.
     */
    uint8           DoPing;
    /**
     * Set to 1 to indicate that the error count for this transaction is
     * non-zero. Set to 0 if the error count is 0.
     */
    uint8           ErrorState;

    uint32          status;
    /**
    * Set to 1 if the host channel has been halted, but the core is not
    * finished flushing queued requests. Otherwise 0.
    */
    uint8           HaltPending;
    uint8           HaltOnQueue;

    USB_REQ         *urb;

    /**wrm add**/
    uint8           DisStatus;
    uint8           ConnectStatus;

}UHC_CHN_INFO, *pUHC_CHN_INFO;

/* Macro used to clear one channel interrupt */
#define CLEAN_HC_INT(_hc_regs_, _intr_) \
do { \
    HCINT_DATA hcint_clear; \
    hcint_clear.d32 = 0;    \
    hcint_clear.b._intr_ = 1; \
    _hc_regs_->hcintn = hcint_clear.d32; \
} while (0)

#define DISABLE_HC_INT(_hc_regs_, _intr_) \
do { \
    HCINTMSK_DATA hcintmsk; \
    hcintmsk.d32 = _hc_regs_->hcintmaskn;  \
    hcintmsk.b._intr_ = 0; \
    _hc_regs_->hcintmaskn =  hcintmsk.d32; \
} while (0)




#define     HOST_OK                     0
#define     HOST_ERR                    1
#define     HOST_STALL                  2
#define     HOST_NAK                    3
#define     HOST_NYET                   4
#define     HOST_XERR                   5
#define     HOST_TGLERR                 6
#define     HOST_BABBLE                 7
#define     HOST_NO_QUEUE               8
#define     HOST_NO_SPACE               9
#define     HOST_NO_DATA                10
#define     HOST_TIMEOUT                11
#define     HOST_NO_READY               12
#define     HOST_NO_DEV                 13

#define     USB_SPEED_LS                0x00
#define     USB_SPEED_FS                0x01
#define     USB_SPEED_HS                0x02

typedef enum HC_XFER_STATUS
{
    HC_XFER_IDLE,
    HC_XFER_COMPL,
    HC_XFER_URB_COMPL,
    HC_XFER_ACK,
    HC_XFER_NAK,
    HC_XFER_NYET,
    HC_XFER_XERR,
    HC_XFER_STALL,
    HC_XFER_BABBLE,
    HC_XFER_TIMEOUT,
} HC_XFER_STATUS_e;


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

extern rk_err_t USBWriteEp(uint32 epnum, uint32 len, uint8 * buf);
extern rk_err_t USBReadEp(uint32 epnum, uint32 len, uint8 * buf);
extern rk_err_t USBEnumDone(uint32 * speed);
extern void     USBSetInEpStall(uint8 epnum);
extern void     USBSetOutEpStall(uint8 epnum);
extern uint8    USBCheckOutEpStall(uint8 epnum);
extern uint8    USBCheckInEpStall(uint8 epnum);
extern void     USBSetOutEpStatus(uint8 epnum, uint8 status);
extern void     USBSetInEpStatus(uint8 epnum, uint8 status);
extern void     USBReconnect(uint32 restart, uint32 fullspeed);
extern void     USBDisconnect(void);
extern void     USBConnect(bool FullSpeed);
extern void     USBReset(void);
extern uint32   USBGetRxFIFOIntType(uint32 * epnum, uint32 * datalen);
extern uint32   USBGetOutEpIntType(uint32 * ep);
extern uint32   USBGetInEpIntType(uint32 * ep);
extern void     USBSetAddr(uint32 addr);
extern void     USBEnableOutEp(uint32 epnum, uint32 bmAttributes, uint32 wMaxPacketSize);
extern void     USBEnalbeInEp(uint32 epnum, uint32 EnType, uint32 wMaxPacketSize);
extern uint32   UsbGetIntType(void);
extern void     UsbIntDisalbe(void);
extern void     UsbIntEnable(void);
extern void     USBSetEpSts(uint8 dir, uint8 epnum, uint8 status);
extern void     USBEnableEp (uint8 epnum, uint32 bmAttributes, uint32 wMaxPacketSize);
extern void     USBResetToggle(uint8 dir, uint8 epnum);
extern uint32   UsbSetIntMsk(void);

/*Host*/
extern void     USBSetMode(uint8 mode);
extern void     USBHostConnect(bool FullSpeed, UHC_CHN_INFO * pUsbHC);
extern uint32   UsbGetHostIntType(UHC_CHN_INFO * pUsbHC);
extern int32    HostResetPort(HOST_DEV *pDev);
extern int32    UHCPortStatus(uint8 *status, uint8 *change);
extern int32    HostStartWaitXfer(USB_REQ *urb, int32 timeout, UHC_CHN_INFO * pUsbHC);
extern void     Hostnptxfemp (int32 ret);
extern void     HostRegDeinit(void);

#endif

