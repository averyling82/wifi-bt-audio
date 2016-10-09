/*
********************************************************************************************
*
*                Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Driver\USB\UsbOtgDev.c
* Owner: aaron.sun
* Date: 2014.10.7
* Time: 15:42:08
* Desc: usb otg device
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun  2014.10.7     15:42:08   1.0
********************************************************************************************
*/
#include "BspConfig.h"
#ifdef __DRIVER_USB_USBOTGDEV_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "RKOS.h"
#include "BSP.h"
#include "DeviceInclude.h"



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#define USBOTG_DEV_NUM 2

typedef struct _USB_EP0_REQ
{
    uint16  NeedLen;
    uint16  XferLen;
    uint8   *buf;
    uint8   IsIn;   //0: host to device, 1: device to host

} USB_EP0_REQ;

typedef  struct _USBOTG_DEVICE_CLASS
{
    DEVICE_CLASS stUsbOtgDevice;
    pSemaphore osUsbOtgOperSem;
    pSemaphore osUsbOtgEpSem[3];
    uint8 * SendBuf[3];
    uint32 SendLen[3];
    uint8 EpData[512];
    uint32 CurEp;
    uint32 DataSize;
    uint32 usbmode;
    uint32 usbspeed;
    uint32 usbstatus;
    uint32 threadstatus;
    uint32 RemoteWakeup;
    uint32 connected;
    uint8  early_suspend_status;
    uint8  suspend_status;
    uint8  reset_status;
    uint8  enum_ok_status;
    uint8  getconfigure;
    uint8  AlterInterface;
    USB_EP0_REQ stEp0Req;
    uint32 Ep0State;
    void (*pfReadEp)(uint32 ep, uint8 * buf, uint32 size);
    uint32 ConfigLen;
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
    /**Host**/
    uint8 HostProcessing;
    UHC_CHN_INFO ChannelInfo[MAX_HOST_CHN];
    HOST_DEV pHDev;
    USB_REQ  gURBCache;
    int32 (*pMscHost)(HOST_INTERFACE* interfaceDes);
    int UsbHostMscDeviceList;

}USBOTG_DEVICE_CLASS;


#define UDC_EV_IDLE                                     0
#define UDC_EV_OUT_PKT_RCV                              (1<<0)
#define UDC_EV_OUT_XFER_COMPL                           (1<<1)
#define UDC_EV_IN_XFER_COMPL                            (1<<2)
#define UDC_EV_SETUP_DONE                               (1<<3)
#define UDC_EV_SETUP_XFER_COMPL                         (1<<4)
#define UDC_EV_ENUM_DONE                                (1<<5)
#define UDC_EV_CLEAN_EP                                 (1<<6)
#define UDC_EV_IN_XFER_REDO                             (1<<7)
#define UDC_EV_IN_XFER_TIMEOUT                          (1<<8)
#define UDC_EV_IN_PKT_OVER                              (1<<9)

#define MIN(x, y)  ((x) < (y) ? (x) : (y))


typedef enum _EP0_STATE
{
    EP0_DISCONNECT,        /* no host */
    EP0_IDLE,
    EP0_IN_DATA_PHASE,
    EP0_OUT_DATA_PHASE,
    EP0_IN_STATUS_PHASE,
    EP0_OUT_STATUS_PHASE,
    EP0_STALL
} EP0_STATE;


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static USBOTG_DEVICE_CLASS * gpstUsbOtgDevISR[USBOTG_DEV_NUM] = {(USBOTG_DEVICE_CLASS *)NULL,(USBOTG_DEVICE_CLASS *)NULL};


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
*                                                   local function declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
rk_err_t UsbOtgHostCheckIdle(HTC hTask);
rk_err_t UsbOtgDevCheckIdle(HTC hTask);
rk_err_t UsbOtgDevDeInit(USBOTG_DEVICE_CLASS * pstUsbOtgDev);
rk_err_t UsbOtgDevInit(USBOTG_DEVICE_CLASS * pstUsbOtgDev);
rk_err_t UsbOtgDevResume(HDC dev);
rk_err_t UsbOtgDevSuspend(HDC dev);
rk_err_t UsbOtgDevShellTest(HDC dev, uint8 * pstr);
rk_err_t UsbOtgDevShellDel(HDC dev, uint8 * pstr);
rk_err_t UsbOtgDevShellMc(HDC dev, uint8 * pstr);
rk_err_t UsbOtgDevShellPcb(HDC dev, uint8 * pstr);
rk_err_t UsbOtgDevShellHelp(HDC dev, uint8 * pstr);
rk_err_t UsbOtgDevShellSuspend(HDC dev, uint8 * pstr);
rk_err_t UsbOtgDevShellResume(HDC dev, uint8 * pstr);



void     USBInPacketHandle(USBOTG_DEVICE_CLASS * pstUsbOtgDev);
void     USBStartEp0Xfer(USBOTG_DEVICE_CLASS * pstUsbOtgDev);
uint32   USBGetString(USBOTG_DEVICE_CLASS * pstUsbOtgDev,uint8 index, uint32 size, void *string);
void     USBOutTransCompleteHandle(USBOTG_DEVICE_CLASS * pstUsbOtgDev);
void     USBOutPacketHandle(USBOTG_DEVICE_CLASS * pstUsbOtgDev);
void     USBGetInterface(USBOTG_DEVICE_CLASS * pstUsbOtgDev);
void     USBGetConfig(USBOTG_DEVICE_CLASS * pstUsbOtgDev);
void     USBSetInterface(USBOTG_DEVICE_CLASS * pstUsbOtgDev);
void     USBSetConfig(USBOTG_DEVICE_CLASS * pstUsbOtgDev);
void     USBGetDescriptor(USBOTG_DEVICE_CLASS * pstUsbOtgDev);
void     USBSetAddress(USBOTG_DEVICE_CLASS * pstUsbOtgDev);
void     USBSetFeature(USBOTG_DEVICE_CLASS * pstUsbOtgDev);
void     USBCleanFeature(USBOTG_DEVICE_CLASS * pstUsbOtgDev);
void     USBGetStatus(USBOTG_DEVICE_CLASS * pstUsbOtgDev);
void     UsbSetupHandle(USBOTG_DEVICE_CLASS * pstUsbOtgDev);
void     UsbOtgDevIntIsr(uint32 DevID);


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/

/*******************************************************************************
** Name: UsbOtgDevIntIsr
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2014.10.8
** Time: 14:37:09
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_COMMON_
COMMON API void UsbOtgDevIntIsr0(void)
{
    //Call total int service...
    UsbOtgDevIntIsr(0);
}

/*******************************************************************************
** Name: UsbOtgDevIntIsr
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2014.10.8
** Time: 14:37:09
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_COMMON_
COMMON API void UsbOtgDevIntIsr1(void)
{
    //Call total int service...
    UsbOtgDevIntIsr(1);
}


/*******************************************************************************
** Name: UsbOtgDev_Task_Enter
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2014.10.8
** Time: 14:37:09
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_COMMON_
COMMON API void UsbOtgDev0_Task_Enter(void)
{

    HTC hSelf;
    hSelf = RKTaskGetRunHandle();

    while (1)
    {
        rkos_semaphore_take(gpstUsbOtgDevISR[0]->osUsbOtgOperSem, MAX_DELAY);
        if(UsbOtgDevCheckIdle(hSelf) != RK_SUCCESS)
        {
            continue;
        }

        switch (gpstUsbOtgDevISR[0]->usbstatus)
        {
            case UDC_EV_SETUP_DONE:
                UsbSetupHandle(gpstUsbOtgDevISR[0]);
                break;


            case UDC_EV_OUT_PKT_RCV:
                if (gpstUsbOtgDevISR[0]->CurEp == 0)
                {
                    USBOutPacketHandle(gpstUsbOtgDevISR[0]);
                }
                else
                {
                    gpstUsbOtgDevISR[0]->pfReadEp(gpstUsbOtgDevISR[0]->CurEp, gpstUsbOtgDevISR[0]->EpData, gpstUsbOtgDevISR[0]->DataSize);
                }
                break;

            case UDC_EV_OUT_XFER_COMPL:
            #if 0
                if (gpstUsbOtgDevISR[0]->CurEp == 0)
                {
                    USBOutTransCompleteHandle(gpstUsbOtgDevISR[0]);
                }
                else
                {
                    gpstUsbOtgDevISR[0]->pfReadEp(gpstUsbOtgDevISR[0]->CurEp, gpstUsbOtgDevISR[0]->EpData, gpstUsbOtgDevISR[0]->DataSize);
                }
                break;
            #endif
            case UDC_EV_IN_PKT_OVER:
                ;
        }
        gpstUsbOtgDevISR[0]->usbstatus =  UDC_EV_IDLE;
        UsbIntEnable();
        //rk_printf("usb otg task serice...");
    }
}

#ifdef USB_HOST
/*******************************************************************************
** Name: HostBulkMsg
** Input:HOST_DEV *pDev, uint32 pipe, void * data, int len, int timeout
** Return: int32
** Owner:wrm
** Date: 2015.7.10
** Time: 14:28:54
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_COMMON_
COMMON API int32 HostBulkMsg(HOST_DEV *pDev, uint32 pipe, void * data, int len, int timeout)
{
    int32 ret;
    USB_REQ * urb = NULL;
    uint8 epdir = USBPipeDir(pipe);
    uint8 epnum = USBPipeEpNum(pipe);

    urb = HostAllocUrb();

    urb->pipe.d32 = pipe;
    urb->TransBuf = data;
    urb->BufLen = len;

    urb->pDev = pDev;
    urb->DataToggle = USBGetToggle(pDev, epnum, epdir);
#if 0
    printf ("\npDev->toggle[out]=%d ep=%d epdir=%d\n",
            pDev->toggle[epdir],epnum,epdir);
    printf ("\nwrm HostBulkMsg urb->DataToggle=%d\n",urb->DataToggle);
#endif
    ret =  HostStartWaitXfer(urb, timeout,&gpstUsbOtgDevISR[0]->ChannelInfo[0]);

    USBSetToggle(pDev, epnum, epdir, urb->DataToggle);

    HostFreeUrb(urb);

    return ret;
}


/*******************************************************************************
** Name: OtgHostnptxfemp
** Input:void
** Return: void
** Owner:wrm
** Date: 2015.7.10
** Time: 14:28:54
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_COMMON_
COMMON API void OtgHostnptxfemp(int32 ret)
{
    Hostnptxfemp(ret);
}

/*******************************************************************************
** Name: UsbOtgHost_Connect
** Input:HDC dev, uint32 Speed
** Return: rk_err_t
** Owner:wrm
** Date: 2014.10.10
** Time: 14:28:54
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_COMMON_
COMMON API rk_err_t UsbOtgHost_Connect(HDC dev, uint32 Speed)
{
    USBOTG_DEVICE_CLASS * pstUsbOtgDev = (USBOTG_DEVICE_CLASS *)dev;
    FREQ_EnterModule(FREQ_MAX);

    RKTaskCreate(TASK_ID_USBOTG1, 0, NULL, SYNC_MODE);


    gpstUsbOtgDevISR[0]->usbmode = HOST_MODE;
    gpstUsbOtgDevISR[0]->HostProcessing = 0;
    gpstUsbOtgDevISR[0]->ChannelInfo[0].DisStatus = 0;
    gpstUsbOtgDevISR[0]->ChannelInfo[0].ConnectStatus = 0;

    USBHostConnect(Speed,&gpstUsbOtgDevISR[0]->ChannelInfo[0]);
}

/*******************************************************************************
** Name: UsbOtgDev_SetMode
** Input:HDC dev, USBOGT_DESCRIPTOR * pDes
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.5.25
** Time: 10:13:16
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_COMMON_
COMMON API rk_err_t UsbOtgDev_SetMode(HDC dev, uint8 mode)
{
    USBOTG_DEVICE_CLASS * pstUsbOtgDev = (USBOTG_DEVICE_CLASS *)dev;

    USBSetMode(mode);

    return RK_SUCCESS;

}

/*******************************************************************************
** Name: HostCreatePipe
** Input:HOST_DEV * dev, uint8 epnum, uint8 epdir
** Return: int32
** Owner:wrm
** Date: 2014.10.8
** Time: 14:37:09
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_COMMON_
COMMON API uint32 HostCreatePipe(HOST_DEV * pHDev, uint8 epnum, uint8 epdir)
{
    USB_ENDPOINT_DESCRIPTOR *Ep;
    uint32 pipe;

    if (epdir)
        Ep = pHDev->EpIn[epnum];
    else
        Ep = pHDev->EpOut[epnum];

    pipe = (pHDev->DevNum<<18) |(Ep->bmAttributes<<16)|(epdir << 15)
            |(epnum << 11)| Ep->wMaxPacketSize;

    return pipe;
}

#endif


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: UsbOtgDevCheckIdle
** Input:HTC hTask
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.5
** Time: 14:03:42
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_COMMON_
COMMON FUN rk_err_t UsbOtgDevCheckIdle(HTC hTask)
{
    RK_TASK_CLASS*   pTask = (RK_TASK_CLASS*)hTask;

    if(pTask->State != TASK_STATE_WORKING)
    {
        pTask->State = TASK_STATE_WORKING;
    }

    pTask->IdleTick = 0;

    return RK_SUCCESS;
}

#ifdef USB_HOST
/*******************************************************************************
** Name: UsbOtgHostCheckIdle
** Input:hTask
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.5
** Time: 14:04:23
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_COMMON_
COMMON FUN rk_err_t UsbOtgHostCheckIdle(hTask)
{
    RK_TASK_CLASS*   pTask = (RK_TASK_CLASS*)hTask;

    if(pTask->State != TASK_STATE_WORKING)
    {
        pTask->State = TASK_STATE_WORKING;
    }

    pTask->IdleTick = 0;

    return RK_SUCCESS;
}

//Host
/*******************************************************************************
** Name: HostAllocUrb
** Input:
** Return: int32
** Owner:wrm
** Date: 2014.10.8
** Time: 14:37:09
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_COMMON_
COMMON FUN static USB_REQ * HostAllocUrb(void)
{
    USB_REQ *urb = &gpstUsbOtgDevISR[0]->gURBCache;

    //printf ("HostAllocUrb:urb->valid=%d\n",urb->valid);
    if (!urb->valid)
    {
        memset(urb, 0, sizeof(USB_REQ));

        urb->valid = 1;
        urb->completed = 0;
        return urb;
    }

    return NULL;
}
/*******************************************************************************
** Name: HostFreeUrb
** Input:USB_REQ *urb
** Return: int32
** Owner:wrm
** Date: 2014.10.8
** Time: 14:37:09
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_COMMON_
COMMON FUN static void HostFreeUrb(USB_REQ *urb)
{
    urb->valid = 0;
}

/*******************************************************************************
** Name: HostCtrlMsg
** Input:
** Return: int32
** Owner:wrm
** Date: 2014.10.8
** Time: 14:37:09
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_COMMON_
COMMON FUN int32 HostCtrlMsg (HOST_DEV *pHDev, uint32 pipe, uint8 bmRequestType, uint8 bRequest, uint16  wValue,
                                 uint16 wIndex, void *data, uint32 size, int timeout)
{
    int32 ret;
    USB_CTRL_REQUEST CtrlReq;
    USB_REQ * urb = NULL;

    CtrlReq.bRequestType = bmRequestType;
    CtrlReq.bRequest = bRequest;
    CtrlReq.wValue = wValue;
    CtrlReq.wIndex = wIndex;
    CtrlReq.wLength = size;

    urb = HostAllocUrb();
    urb->pipe.d32 = pipe;
    urb->SetupPkt = &CtrlReq;
    urb->TransBuf = data;
    urb->BufLen = size;

    urb->DataToggle = 1;
    urb->CtrlPhase = DWC_OTG_CONTROL_SETUP;
    urb->pDev = pHDev;

    ret =  HostStartWaitXfer(urb, timeout,&gpstUsbOtgDevISR[0]->ChannelInfo[0]);

    HostFreeUrb(urb);

    return ret;
}

/*******************************************************************************
** Name: HostGetDesc
** Input:HOST_DEV *pHDev, uint8 type, uint8 index, void *buf, int size
** Return: int32
** Owner:wrm
** Date: 2014.10.8
** Time: 14:37:09
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_COMMON_
COMMON FUN static int32 HostGetDesc(HOST_DEV *pHDev, uint8 type, uint8 index, void *buf, int size)
{
    int32 i, ret;

    for (i = 0; i < 3; i++)
    {
        ret = HostCtrlMsg(pHDev, HostCreatePipe(pHDev, 0, 1),  USB_DIR_IN, USB_REQ_GET_DESCRIPTOR,
                    (type << 8) + index, 0, buf, size, USB_CTRL_GET_TIMEOUT);
        if (ret <= 0)
        {
            continue;
        }
        if (ret>1 && ((uint8 *)buf)[1] != type)
        {
            ret = -HOST_ERR;
            continue;
        }
        break;
    }
    return ret;
}

/*******************************************************************************
** Name: HostPortInit
** Input:HOST_DEV *pDev, uint16 wMaxPacketSize
** Return: void
** Owner:wrm
** Date: 2014.10.8
** Time: 14:37:09
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_COMMON_
COMMON FUN static void HostEp0ReInit(HOST_DEV *pHDev, uint16 wMaxPacketSize)
{
    USB_ENDPOINT_DESCRIPTOR *ep0 = &pHDev->ep0;

    ep0->bmAttributes = USB_ENDPOINT_XFER_CONTROL;
    ep0->wMaxPacketSize = wMaxPacketSize;

    pHDev->EpOut[0] = ep0;
    pHDev->EpIn[0] = ep0;
}
/*******************************************************************************
** Name: HostSetAddr
** Input:HOST_DEV *pHDev
** Return: void
** Owner:wrm
** Date: 2014.10.8
** Time: 14:37:09
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_COMMON_
COMMON FUN static int32 HostSetAddr(HOST_DEV *pHDev)
{
    int32 ret, i;
    uint8 devnum = 1;

    for(i=0; i<2; i++)
    {
        ret = HostCtrlMsg(pHDev, HostCreatePipe(pHDev, 0, 0),
                 0, USB_REQ_SET_ADDRESS, devnum, 0,
                NULL, 0, USB_CTRL_SET_TIMEOUT);

        if (ret >= 0)
        {
            pHDev->DevNum = devnum;
            pHDev->state = USB_STATE_ADDRESS;
            break;
        }
        DelayMs(200);
    }

    return ret;
}

/*******************************************************************************
** Name: HostPortInit
** Input:HOST_DEV *pHDev
** Return: void
** Owner:wrm
** Date: 2014.10.8
** Time: 14:37:09
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_COMMON_
COMMON FUN static int32 HostPortInit(HOST_DEV *pHDev)
{
    int32 ret;
    uint16    wMaxPacketSize;
    USB_DEVICE_DESCRIPTOR buf[64/4];
    USB_DEVICE_DESCRIPTOR *pDesc;

    ret = HostResetPort(pHDev);
    if (ret < 0)
        return ret;

    //printf ("HostPortInit:speed=%d\n",pHDev->speed);
    switch (pHDev->speed)
    {
        case USB_SPEED_HS:     //high speed
            wMaxPacketSize = 64;
            break;
        case USB_SPEED_FS:     //full speed
            wMaxPacketSize = 64;
            break;
        case USB_SPEED_LS:     //low speed
            wMaxPacketSize = 8;
            break;

        default:
            break;
    }

    HostEp0ReInit(pHDev, wMaxPacketSize);

    DelayMs(10);
    ret = HostGetDesc(pHDev, USB_DT_DEVICE, 0, buf, 64);
    if (ret < 0)
        return ret;

    pDesc = (USB_DEVICE_DESCRIPTOR *)buf;
    if (wMaxPacketSize != pDesc->bMaxPacketSize0)
    {
        HostEp0ReInit(pHDev, pDesc->bMaxPacketSize0);
    }

    #if 0
    ret = UHCResetPort(pDev); // windows 和linux 会进行二次复位, 协议无此要求
    if (ret < 0)
        return ret;
    #endif

    ret = HostSetAddr(pHDev);
    if (ret < 0)
        return ret;

    DelayMs(5);

    ret = HostGetDesc(pHDev, USB_DT_DEVICE, 0, buf, USB_DT_DEVICE_SIZE);
    if (ret < 0)
        return ret;

    memcpy(&pHDev->DevDesc, buf, USB_DT_DEVICE_SIZE);

    return ret;
}

/*******************************************************************************
** Name: HostParseInterface
** Input:HOST_DEV *pHDev, uint8* buf, uint32 size
** Return: void
** Owner:wrm
** Date: 2014.10.8
** Time: 14:37:09
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_COMMON_
COMMON FUN static int32 HostParseInterface(HOST_DEV *pHDev, uint8* buf, uint32 size)
{
    int32 i, numep, ret = -HOST_ERR;
    USB_ENDPOINT_DESCRIPTOR *ep;
    uint8 epnum;
    uint8 epdir; /** 0: OUT, 1: IN */
    USB_INTERFACE_DESCRIPTOR* ifdesc;
    HOST_INTERFACE *intf = &pHDev->intf;

    ifdesc = &intf->IfDesc;
    memcpy(ifdesc, buf, USB_DT_INTERFACE_SIZE);
    numep = ifdesc->bNumEndpoints;
    if (numep > MAX_HOST_EP)
    {
        numep = MAX_HOST_EP;
        ifdesc->bNumEndpoints = numep;
    }

    buf += ifdesc->bLength;
    size -= ifdesc->bLength;

    if (numep > 0)
    {
        for (i=0; i<numep && size>0; i++)
        {
            ep = (USB_ENDPOINT_DESCRIPTOR*)buf;
            if (ep->bDescriptorType != USB_DT_ENDPOINT)
                return -HOST_ERR;

            epdir = ep->bEndpointAddress >> 7;
            epnum = ep->bEndpointAddress & 0x0F;

            ep = &intf->EpDesc[i];
            memcpy(ep, buf, USB_DT_ENDPOINT_SIZE);
            (epdir)? (pHDev->EpIn[epnum]=ep) : (pHDev->EpOut[epnum]=ep);

            buf += USB_DT_ENDPOINT_SIZE;
            size -=USB_DT_ENDPOINT_SIZE;
        }
        intf->pDev = pHDev;
        ret = HOST_OK;
    }

    return ret;
}

/*******************************************************************************
** Name: HostParseConfig
** Input:HOST_DEV *pHDev, uint8* buf, uint32 size
** Return: void
** Owner:wrm
** Date: 2014.10.8
** Time: 14:37:09
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_COMMON_
COMMON FUN static int32 HostParseConfig(HOST_DEV *pHDev, uint8* buf, uint32 size)
{
    int32 ret = -HOST_ERR;
    uint32 nintf;
    USB_DESC_HEADER *header;
    USB_CONFIGURATION_DESCRIPTOR *config = &pHDev->CfgDesc;

    memcpy(config, buf, USB_DT_CONFIG_SIZE);
    #if 1
    nintf = config->bNumInterfaces;
    if (nintf > USB_MAXINTERFACES)
    {
        nintf = USB_MAXINTERFACES;
        config->bNumInterfaces = nintf;
    }
    #endif
    buf += config->bLength;
    size -= config->bLength;

    header = (USB_DESC_HEADER *)buf;

    if (USB_DT_INTERFACE == header->bDescriptorType)
    {
        ret = HostParseInterface(pHDev, buf, size);
        if (ret < 0)
            return ret;
    }

    return ret;
}

/*******************************************************************************
** Name: HostSetConfig
** Input:HOST_DEV *pHDev, uint16 config
** Return: void
** Owner:wrm
** Date: 2014.10.8
** Time: 14:37:09
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_COMMON_
COMMON FUN static int32 HostSetConfig(HOST_DEV *pHDev, uint16 config)
{
    int32 ret;

    ret = HostCtrlMsg(pHDev, HostCreatePipe(pHDev, 0, 0),
             0, USB_REQ_SET_CONFIGURATION, config, 0,
            NULL, 0, USB_CTRL_SET_TIMEOUT);

    if (ret < 0)
        return ret;

    pHDev->state = USB_STATE_CONFIGURED;
    return HOST_OK;
}

/*******************************************************************************
** Name: HostNewDev
** Input:HOST_DEV *pHDev
** Return: void
** Owner:wrm
** Date: 2014.10.8
** Time: 14:37:09
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_COMMON_
COMMON FUN static int32 HostNewDev(HOST_DEV *pHDev)
{
    int32 ret = -HOST_ERR;
    uint16 length;
    uint32 buf[256/4];
    USB_CONFIGURATION_DESCRIPTOR *config;
    uint32 ncfg = pHDev->DevDesc.bNumConfigurations;

    if (ncfg > USB_MAXCONFIG)
    {
        ncfg = USB_MAXCONFIG;
    }

    if (ncfg < 1)
        return -HOST_ERR;

    config = (USB_CONFIGURATION_DESCRIPTOR *)buf;

    ret = HostGetDesc(pHDev, USB_DT_CONFIG, 0, buf, USB_DT_CONFIG_SIZE);
    if (ret < 0)
        goto FAIL;
    length = config->wTotalLength;
    if (length > 256)
    {
        ret = -HOST_ERR;
        goto FAIL;
    }

    ret = HostGetDesc(pHDev, USB_DT_CONFIG, 0, buf, length);
    if (ret < 0)
        goto FAIL;

    ret = HostParseConfig(pHDev, (uint8*)buf, length);
    if (ret < 0)
        goto FAIL;

    ret = HostSetConfig(pHDev, 1);
    if (ret < 0)
        goto FAIL;

FAIL:

    return ret;
}


#endif

/*******************************************************************************
** Name: USBInPacketHandle
** Input:USBOTG_DEVICE_CLASS * pstUsbOtgDev
** Return: void
** Owner:aaron.sun
** Date: 2015.5.26
** Time: 15:41:15
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_COMMON_
COMMON FUN void USBInPacketHandle(USBOTG_DEVICE_CLASS * pstUsbOtgDev)
{
    USB_EP0_REQ *req = &pstUsbOtgDev->stEp0Req;

    if(req->XferLen >= req->NeedLen)
    {
        pstUsbOtgDev->Ep0State = EP0_IDLE;
        req->NeedLen = 0;
    }
    else
    {
        uint16 size = MIN(EP0_PACKET_SIZE, (req->NeedLen-req->XferLen));
        USBWriteEp(0, size, req->buf+req->XferLen);
        req->XferLen += size;
    }
}

/*******************************************************************************
** Name: USBStartEp0Xfer
** Input:USB_EP0_REQ *req
** Return: void
** Owner:aaron.sun
** Date: 2015.5.26
** Time: 15:33:02
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_COMMON_
COMMON FUN void USBStartEp0Xfer(USBOTG_DEVICE_CLASS * pstUsbOtgDev)
{
    USB_EP0_REQ *req = &pstUsbOtgDev->stEp0Req;


    req->XferLen = 0;

    if (req->IsIn)
    {
        uint16 len = MIN(EP0_PACKET_SIZE, req->NeedLen);
        USBWriteEp(0, len, req->buf);
        req->XferLen += len;
        if (req->NeedLen > req->XferLen)
        {
           pstUsbOtgDev->Ep0State = EP0_IN_DATA_PHASE;
        }
    }
    else
    {
        pstUsbOtgDev->Ep0State = EP0_OUT_DATA_PHASE;
    }

    return;
}

/*******************************************************************************
** Name: USBGetString
** Input:uint8 index, uint32 size, void *string
** Return: void
** Owner:aaron.sun
** Date: 2015.5.26
** Time: 14:21:04
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_COMMON_
COMMON FUN uint32 USBGetString(USBOTG_DEVICE_CLASS * pstUsbOtgDev, uint8 index,
                               uint32 size, void *string)
{
    uint32 len = 0;

    switch (index)
    {
        case 0:
            len = MIN(((uint8 *)pstUsbOtgDev->pUsbLangId)[0], size);
            memcpy(string, (void*)pstUsbOtgDev->pUsbLangId, len);
            break;

        case USB_STRING_MANUFACTURER:
            len = MIN(((uint8 *)pstUsbOtgDev->pMString)[0], size);
            memcpy(string, (void*)pstUsbOtgDev->pMString, len);

        case USB_STRING_PRODUCT:
            len = MIN(((uint8 *)pstUsbOtgDev->pDevString)[0], size);
            memcpy(string, (void*)pstUsbOtgDev->pDevString, len);
            break;

        case USB_STRING_SERIAL:
            len = MIN(((uint8 *)pstUsbOtgDev->pSerialNum)[0], size);
            memcpy(string, (void*)pstUsbOtgDev->pSerialNum, len);
            break;

        case USB_STRING_CONFIG:
            len = MIN(((uint8 *)pstUsbOtgDev->pConfigString)[0], size);
            memcpy(string, (void*)pstUsbOtgDev->pConfigString, len);
            break;

        case USB_STRING_INTERFACE:
            len = MIN(((uint8 *)pstUsbOtgDev->pInterfaceString)[0], size);
            memcpy(string, (void*)pstUsbOtgDev->pInterfaceString, len);
            break;

        default:
            break;
    }

    return len;
}
/*******************************************************************************
** Name: USBOutTransCompleteHandle
** Input:USBOTG_DEVICE_CLASS * pstUsbOtgDev
** Return: void
** Owner:aaron.sun
** Date: 2015.5.26
** Time: 10:43:35
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_COMMON_
COMMON FUN void USBOutTransCompleteHandle(USBOTG_DEVICE_CLASS * pstUsbOtgDev)
{
    pstUsbOtgDev->pfReadEp(pstUsbOtgDev->CurEp, pstUsbOtgDev->EpData, pstUsbOtgDev->DataSize);
}

/*******************************************************************************
** Name: USBOutPacketHandle
** Input:USBOTG_DEVICE_CLASS * pstUsbOtgDev
** Return: void
** Owner:aaron.sun
** Date: 2015.5.26
** Time: 10:39:55
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_COMMON_
COMMON FUN void USBOutPacketHandle(USBOTG_DEVICE_CLASS * pstUsbOtgDev)
{
    if(pstUsbOtgDev->DataSize != 0)
        pstUsbOtgDev->pfReadEp(pstUsbOtgDev->CurEp, pstUsbOtgDev->EpData, pstUsbOtgDev->DataSize);
}
/*******************************************************************************
** Name: USBGetInterface
** Input:USBOTG_DEVICE_CLASS * pstUsbOtgDev
** Return: void
** Owner:aaron.sun
** Date: 2015.5.26
** Time: 9:29:40
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_COMMON_
COMMON FUN void USBGetInterface(USBOTG_DEVICE_CLASS * pstUsbOtgDev)
{
#if 1
    uint8 intf = pstUsbOtgDev->AlterInterface;
    printf("Get Interface = %d\n", pstUsbOtgDev->AlterInterface);
    USBWriteEp(0, 1, &intf);
#endif

}

/*******************************************************************************
** Name: USBGetConfig
** Input:USBOTG_DEVICE_CLASS * pstUsbOtgDev
** Return: void
** Owner:aaron.sun
** Date: 2015.5.26
** Time: 9:29:23
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_COMMON_
COMMON FUN void USBGetConfig(USBOTG_DEVICE_CLASS * pstUsbOtgDev)
{
#if 1
    uint8 config = pstUsbOtgDev->getconfigure;
    printf("Get Config = %d\n",config);
    USBWriteEp(0, 1, &config);
#endif
}
/*******************************************************************************
** Name: USBSetInterface
** Input:USBOTG_DEVICE_CLASS * pstUsbOtgDev
** Return: void
** Owner:aaron.sun
** Date: 2015.5.26
** Time: 9:28:57
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_COMMON_
COMMON FUN void USBSetInterface(USBOTG_DEVICE_CLASS * pstUsbOtgDev)
{
#if 1
    USB_CTRL_REQUEST * pstUsbCtrl =(USB_CTRL_REQUEST *)pstUsbOtgDev->EpData;
    pstUsbOtgDev->AlterInterface = pstUsbCtrl->wValue & 0x1;
    printf("\nSet Interface = %d", pstUsbCtrl->wValue);
    USBWriteEp(0, 0, NULL);
#endif
}

/*******************************************************************************
** Name: USBSetConfig
** Input:USBOTG_DEVICE_CLASS * pstUsbOtgDev
** Return: void
** Owner:aaron.sun
** Date: 2015.5.26
** Time: 9:21:29
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_COMMON_
COMMON FUN void USBSetConfig(USBOTG_DEVICE_CLASS * pstUsbOtgDev)
{
    USB_CTRL_REQUEST * pstUsbCtrl =(USB_CTRL_REQUEST *)pstUsbOtgDev->EpData;
    uint8 type = pstUsbCtrl->bRequestType & USB_TYPE_MASK;
    int32 ret = 0;
    uint8 config = pstUsbCtrl->wValue & 0x1;
    //printf("Set Config = %d\n",config);
    pstUsbOtgDev->getconfigure = config;
    USB_FSG_CONFIGS_DESCRIPTOR *ptr = (USB_FSG_CONFIGS_DESCRIPTOR *)(pstUsbOtgDev->pConfigDes);
    if (config)
    {
        USBEnableEp(0x01, ptr->BulkOut.bmAttributes, ptr->BulkOut.wMaxPacketSize);
        USBEnableEp(0x01|0x80, ptr->BulkIn.bmAttributes, ptr->BulkIn.wMaxPacketSize);

        if (0==pstUsbOtgDev->connected)
        {
            pstUsbOtgDev->connected = 1;
        }
    }
    USBWriteEp(0, 0, NULL);
}

/*******************************************************************************
** Name: USBGetDescriptor
** Input:USBOTG_DEVICE_CLASS * pstUsbOtgDev
** Return: void
** Owner:aaron.sun
** Date: 2015.5.26
** Time: 8:47:49
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_COMMON_
COMMON FUN void USBGetDescriptor(USBOTG_DEVICE_CLASS * pstUsbOtgDev)
{
    USB_CTRL_REQUEST * pstUsbCtrl =(USB_CTRL_REQUEST *)pstUsbOtgDev->EpData;
    uint32 len = 0;
    uint8 bDescriptor = (pstUsbCtrl->wValue >> 8) & 0xff;
    uint8 speed = pstUsbOtgDev->usbspeed;
    USB_EP0_REQ *ep0req = &pstUsbOtgDev->stEp0Req;


    ep0req->buf = pstUsbOtgDev->EpData;

    switch (bDescriptor)
    {
        case USB_DT_DEVICE:
            {
                len = MIN(sizeof(USB_DEVICE_DESCRIPTOR),pstUsbCtrl->wLength);
                memcpy(ep0req->buf, (uint8*)pstUsbOtgDev->pDevDes, len);
                break;
            }
        case USB_DT_DEVICE_QUALIFIER:
            {
                len = MIN(sizeof(HS_DEVICE_QUALIFIER), pstUsbCtrl->wLength);
                memcpy(ep0req->buf, (uint8*)pstUsbOtgDev->pHsQualifier, len);
                break;
            }

        case USB_DT_OTHER_SPEED_CONFIG:
        case USB_DT_CONFIG:
            {
                USB_CONFIGURATION_DESCRIPTOR *ptr = (USB_CONFIGURATION_DESCRIPTOR *)ep0req->buf;
                USB_FSG_CONFIGS_DESCRIPTOR *Fsg_tmp = (USB_FSG_CONFIGS_DESCRIPTOR *)(pstUsbOtgDev->pConfigDes);
                if (bDescriptor == USB_DT_OTHER_SPEED_CONFIG)
                {
                    speed = (USB_SPEED_FULL + USB_SPEED_HIGH) - speed;
                }
                len = MIN(pstUsbOtgDev->ConfigLen, pstUsbCtrl->wLength);
                if (speed == USB_SPEED_HIGH)
                {
                    Fsg_tmp->BulkIn.wMaxPacketSize = 512;
                    Fsg_tmp->BulkOut.wMaxPacketSize = 512;
                }
                else
                {
                    Fsg_tmp->BulkIn.wMaxPacketSize = 64;
                    Fsg_tmp->BulkOut.wMaxPacketSize = 64;
                }

                memcpy(ep0req->buf, pstUsbOtgDev->pConfigDes, len);
                ptr->bDescriptorType = bDescriptor;
                break;
            }
        case USB_DT_STRING:
            {
                len = USBGetString(pstUsbOtgDev, pstUsbCtrl->wValue & 0x00ff, pstUsbCtrl->wLength, ep0req->buf);
                if (len == 0)
                {
                    USBWriteEp(0, 0, NULL);
                }
                break;
            }
        default:
            USBSetInEpStall(0);
            break;
    }

    if (len)
    {
        ep0req->IsIn = 1;
        ep0req->NeedLen = len;
        USBStartEp0Xfer(pstUsbOtgDev);
    }

    return;
}

/*******************************************************************************
** Name: USBSetAddress
** Input:USBOTG_DEVICE_CLASS * pstUsbOtgDev
** Return: void
** Owner:aaron.sun
** Date: 2015.5.26
** Time: 8:47:17
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_COMMON_
COMMON FUN void USBSetAddress(USBOTG_DEVICE_CLASS * pstUsbOtgDev)
{
    USB_CTRL_REQUEST * pstUsbCtrl =(USB_CTRL_REQUEST *)pstUsbOtgDev->EpData;

    USBSetAddr (pstUsbCtrl->wValue);  //reset device addr
    printf ("\nSetAddress:0x%x",pstUsbCtrl->wValue);
    USBWriteEp(0,0, NULL);

}
/*******************************************************************************
** Name: USBSetFeature
** Input:USBOTG_DEVICE_CLASS * pstUsbOtgDev
** Return: void
** Owner:aaron.sun
** Date: 2015.5.26
** Time: 8:45:09
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_COMMON_
COMMON API void USBSetFeature(USBOTG_DEVICE_CLASS * pstUsbOtgDev)
{
#if 1
    uint8 bRecipient;
    USB_CTRL_REQUEST * pstUsbCtrl =(USB_CTRL_REQUEST *)pstUsbOtgDev->EpData;
    bRecipient = pstUsbCtrl->bRequestType & USB_RECIP_MASK;
    USBWriteEp(0, 0, NULL);

    if (bRecipient == USB_RECIP_DEVICE)
    {
        if (pstUsbCtrl->wValue == USB_DEVICE_REMOTE_WAKEUP)
        {
            pstUsbOtgDev->RemoteWakeup = 1;
        }
        else if (pstUsbCtrl->wValue == USB_DEVICE_TEST_MODE)
        {
            switch (pstUsbCtrl->wIndex>>8)
            {
                case 0x01:  //test J
                    //dev_regs->dctl = ((dev_regs->dctl|(1<<4))&(~(3<<5)));
                    break;
                case 0x02:  //test K
                    //dev_regs->dctl = ((dev_regs->dctl|(1<<5))&(~(1<<6))&(~(1<<4)));
                    break;
                case 0x03:  //test SE0
                    //dev_regs->dctl = ((dev_regs->dctl|(3<<4))&(~(1<<6)));
                    break;
                case 0x04:  //test packet
                    //dev_regs->dctl = ((dev_regs->dctl|(1<<6))&(~(3<<4)));
                    break;
                case 0x05:  //Test Force Enable
                    Delay100cyc(12);        //延时10us @100MHz
                    //dev_regs->dctl=(dev_regs->dctl & (~(0x07<<4))) | (((ctrl->wIndex>>8)%8)<<4);
                    break;
                default:
                    break;
            }
        }
    }
    else if (bRecipient == USB_RECIP_ENDPOINT       //清除端点stall
        && pstUsbCtrl->wValue == USB_ENDPOINT_HALT)
    {
        uint8 endp,dir;

        endp = (uint8)(pstUsbCtrl->wIndex & USB_ENDPOINT_NUMBER_MASK);
        dir = pstUsbCtrl->wIndex & (uint8)USB_ENDPOINT_DIR_MASK;
        printf ("Stall dir=%d endp=%d\n",dir,endp);
        USBSetEpSts(dir, endp, 1);
    }
#endif
}

/*******************************************************************************
** Name: USBCleanFeature
** Input:USBOTG_DEVICE_CLASS * pstUsbOtgDev
** Return: void
** Owner:aaron.sun
** Date: 2015.5.25
** Time: 17:21:25
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_COMMON_
COMMON FUN void USBCleanFeature(USBOTG_DEVICE_CLASS * pstUsbOtgDev)
{
    uint8 endp,dir;
    uint8 bRecipient;
    USB_CTRL_REQUEST * pstUsbCtrl =(USB_CTRL_REQUEST *)pstUsbOtgDev->EpData;
    //pUSB_DEVICE  pDev = pUDC->pDev;

    bRecipient = pstUsbCtrl->bRequestType & USB_RECIP_MASK;

    if (bRecipient == USB_RECIP_DEVICE              //清除远程唤醒功能
            && pstUsbCtrl->wValue == USB_DEVICE_REMOTE_WAKEUP)
    {
        pstUsbOtgDev->RemoteWakeup = 0;
        USBWriteEp(0, 0, NULL);
    }
    else if (bRecipient == USB_RECIP_ENDPOINT       //清除端点stall
             && pstUsbCtrl->wValue == USB_ENDPOINT_HALT)
    {
        endp = (uint8)(pstUsbCtrl->wIndex & USB_ENDPOINT_NUMBER_MASK);
        dir = pstUsbCtrl->wIndex & (uint8)USB_ENDPOINT_DIR_MASK;
        if (dir == 0)
        {
            USBSetOutEpStatus(endp, 0);     // clear RX/TX stall for OUT/IN on EPn.
        }
        else
        {
            USBSetInEpStatus(endp, 0);     // clear RX/TX stall for OUT/IN on EPn.
        }
        USBWriteEp(0, 0, NULL);


#if 1
        if (endp > 0)
        {
            USBResetToggle(dir, endp);
            USBSetEpSts(0, BULK_OUT_EP, 0);
            rkos_semaphore_give(gpstUsbOtgDevISR[0]->osUsbOtgEpSem[2]);
        }
#endif
    }
    else
    {
        //USBStallEp(0);
        USBSetOutEpStall(0);
    }
}

/*******************************************************************************
** Name: USBGetStatus
** Input:USBOTG_DEV_CLASS * pstUsbOtgDev
** Return: void
** Owner:aaron.sun
** Date: 2015.5.25
** Time: 15:52:09
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_COMMON_
COMMON FUN void USBGetStatus(USBOTG_DEVICE_CLASS * pstUsbOtgDev)
{
    uint8 bRecipient;
    uint16 status;
    USB_CTRL_REQUEST * pstUsbCtrl =(USB_CTRL_REQUEST *)pstUsbOtgDev->EpData;

    bRecipient = pstUsbCtrl->bRequestType & USB_RECIP_MASK;

    if (bRecipient == USB_RECIP_ENDPOINT)
    {

        uint8 dir = pstUsbCtrl->wIndex & (uint8)USB_ENDPOINT_DIR_MASK;
        uint8 EpNum = (uint8)(pstUsbCtrl->wIndex & USB_ENDPOINT_NUMBER_MASK);

        if (dir == 0)
        {
            status = USBCheckOutEpStall(EpNum);
        }
        else
        {
            status = USBCheckInEpStall(EpNum);
        }
        USBWriteEp(0, 2, (uint8 *)&status);
    }
    else if (bRecipient == USB_RECIP_DEVICE)
    {
        status = (uint16)(pstUsbOtgDev->RemoteWakeup<<1);
        USBWriteEp(0, 2, (uint8 *)&status);
    }
}

/*******************************************************************************
** Name: UsbSetupHandle
** Input:USBOTG_DEVICE_CLASS * pstUsbOtgDev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.10.14
** Time: 18:10:13
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_COMMON_
COMMON FUN void UsbSetupHandle(USBOTG_DEVICE_CLASS * pstUsbOtgDev)
{
    USB_CTRL_REQUEST * pstUsbCtrl =(USB_CTRL_REQUEST *)pstUsbOtgDev->EpData;

    uint8 type, req;

    type = pstUsbCtrl->bRequestType & USB_TYPE_MASK;
    req = pstUsbCtrl->bRequest & USB_REQ_MASK;

    /* handle non-standard (class/vendor) requests in the fun driver */
    if (type == USB_TYPE_CLASS || type == USB_TYPE_VENDOR)
    {
        pstUsbOtgDev->pfReadEp(pstUsbOtgDev->CurEp, pstUsbOtgDev->EpData, pstUsbOtgDev->DataSize);
        return;
    }
    else if (type == USB_TYPE_STANDARD)
    {
        //// --- Standard Request handling --- ////
        switch (req)
        {
            case USB_REQ_GET_STATUS:
                USBGetStatus(pstUsbOtgDev);
                break;

            case USB_REQ_CLEAR_FEATURE:
                USBCleanFeature(pstUsbOtgDev);
                break;

            case USB_REQ_SET_FEATURE:
                USBSetFeature(pstUsbOtgDev);
                break;

            case USB_REQ_SET_ADDRESS:
                USBSetAddress(pstUsbOtgDev);
                break;

            case USB_REQ_GET_DESCRIPTOR:
                USBGetDescriptor(pstUsbOtgDev);
                break;

            case USB_REQ_SET_CONFIGURATION:
                pstUsbOtgDev->connected = 1;
                USBSetConfig(pstUsbOtgDev);
                break;

            case USB_REQ_SET_INTERFACE:
                USBSetInterface(pstUsbOtgDev);
                break;

            case USB_REQ_GET_CONFIGURATION:
                USBGetConfig(pstUsbOtgDev);
                break;

            case USB_REQ_GET_INTERFACE:
                USBGetInterface(pstUsbOtgDev);
                break;

            case USB_REQ_SYNCH_FRAME:
                break;

            default:
                /* Call the fun Driver's setup functions */
                pstUsbOtgDev->pfReadEp(pstUsbOtgDev->CurEp, pstUsbOtgDev->EpData, pstUsbOtgDev->DataSize);
                break;
        }
    }
    else
    {
        USBSetOutEpStall(0);
    }


}



/*******************************************************************************
** Name: UsbOtgDevIntIsr
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2014.10.8
** Time: 14:37:09
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_COMMON_
COMMON FUN void UsbOtgDevIntIsr(uint32 DevID)
{
    uint32 UsbOtgDevIntType, UsbRxFIFOIntType, UsbInEpIntType;
    uint32 Ep, datalen;
    uint32 UsbHostIntType;
    UHC_CHN_INFO * pUsbHC = &gpstUsbOtgDevISR[0]->ChannelInfo[0];

    #ifdef USB_HOST
    if (gpstUsbOtgDevISR[0]->usbmode == HOST_MODE)
    {
        //printf ("Entry UsbHOST Interrupt\n");
        UsbGetHostIntType(&gpstUsbOtgDevISR[0]->ChannelInfo[0]);
        //if (UsbHostIntType == HOST_DISCONNECT)
        //{

        //}
        if ((pUsbHC->ConnectStatus == 1)&&(gpstUsbOtgDevISR[DevID]->HostProcessing==0))
        {
            //UsbIntDisalbe();
            gpstUsbOtgDevISR[DevID]->HostProcessing = 1;
            rkos_semaphore_give_fromisr(gpstUsbOtgDevISR[DevID]->osUsbOtgOperSem);
        }
        if ((pUsbHC->ConnectStatus == 1)&&(gpstUsbOtgDevISR[DevID]->HostProcessing==1))
        {
            pUsbHC->ConnectStatus = 0;
        }
    }
    #endif

    if (gpstUsbOtgDevISR[0]->usbmode == DEVICE_MODE)
    {
        //Get UsbOtgDev Int type...
        UsbOtgDevIntType = UsbGetIntType();

        if (gpstUsbOtgDevISR[DevID] != NULL)
        {
            if (UsbOtgDevIntType &USB_INT_TYPE_NP_TXFIFO_EMPTY)
            {
                //printf("\nep in TXFIFO_EMPTY!\n");
                rkos_semaphore_give_fromisr(gpstUsbOtgDevISR[0]->osUsbOtgEpSem[1]);
            }
            if (UsbOtgDevIntType & USB_INT_TYPE_EP_IN)
            {
                //write serice code...
                //printf("\nep in!");
                UsbInEpIntType = USBGetInEpIntType(&Ep);
                if (UsbInEpIntType & USB_IN_EP_INT_TYPE_TRANS_OK)
                {
                    gpstUsbOtgDevISR[DevID]->usbstatus = UDC_EV_IN_PKT_OVER;
                    if (Ep == 0)
                    {
                        UsbIntDisalbe();
                        rkos_semaphore_give_fromisr(gpstUsbOtgDevISR[DevID]->osUsbOtgOperSem);
                    }
                    else
                    {
                        UsbIntDisalbe();
                        rkos_semaphore_give_fromisr(gpstUsbOtgDevISR[DevID]->osUsbOtgEpSem[Ep]);
                    }
                }
                else if (UsbInEpIntType & USB_IN_EP_INT_TYPE_TIME_OUT)
                {
                    //UsbSetIntMsk();
                    //rkos_semaphore_take(gpstUsbOtgDevISR[0]->osUsbOtgEpSem[1],MAX_DELAY);
                    USBWriteEp(Ep, gpstUsbOtgDevISR[DevID]->SendLen[Ep], gpstUsbOtgDevISR[DevID]->SendBuf[Ep]);
                }

            }

            if (UsbOtgDevIntType & USB_INT_TYPE_EP_OUT)
            {
                //write serice code...
                //printf("\nep out!");
                USBGetOutEpIntType(&Ep);
            }

            if (UsbOtgDevIntType & USB_INT_TYPE_SUSPEND)
            {
                printf("\ndev_suspend!");
                gpstUsbOtgDevISR[DevID]->suspend_status = 1;
            }

            if (UsbOtgDevIntType & USB_INT_TYPE_EARLY_SUSPEND)
            {
                printf("\ndev_early suspend!");
                //after early suspend, should be reset.
            }

            if (UsbOtgDevIntType & USB_INT_TYPE_RESET)
            {
                printf("\ndev_reset");
                USBReset();
            }

            if (UsbOtgDevIntType & USB_INT_TYPE_E_DONE)
            {
                printf("\ndev_enum ok!");
                USBEnumDone(&gpstUsbOtgDevISR[DevID]->usbspeed);
                gpstUsbOtgDevISR[DevID]->enum_ok_status = 1;
                gpstUsbOtgDevISR[DevID]->suspend_status = 0;
            }

            if (gpstUsbOtgDevISR[0]->suspend_status == 1)
            {
                rkos_semaphore_give_fromisr(gpstUsbOtgDevISR[0]->osUsbOtgEpSem[1]);
            }


            if (UsbOtgDevIntType & USB_INT_TYPE_RXFIFO_N_EMPTY)
            {
                UsbRxFIFOIntType = USBGetRxFIFOIntType(&Ep, &datalen);
                if (UsbRxFIFOIntType & USB_RX_INT_TYPE_SETUP_RECIVE_PACKAGE)
                {
                    USBReadEp(Ep, datalen, gpstUsbOtgDevISR[DevID]->EpData);
                    gpstUsbOtgDevISR[DevID]->usbstatus = UDC_EV_SETUP_DONE;
                    gpstUsbOtgDevISR[DevID]->DataSize = datalen;
                }
                else if (UsbRxFIFOIntType & USB_RX_INT_TYPE_OUT_RECIVE_PACKAGE)
                {
                    USBReadEp(Ep, datalen, gpstUsbOtgDevISR[DevID]->EpData);
                    gpstUsbOtgDevISR[DevID]->usbstatus = UDC_EV_OUT_PKT_RCV;
                    gpstUsbOtgDevISR[DevID]->DataSize = datalen;
                }
                else if (UsbRxFIFOIntType & USB_RX_INT_TYPE_OUT_TRANS_CMPL)
                {
                    gpstUsbOtgDevISR[DevID]->usbstatus = UDC_EV_OUT_XFER_COMPL;
                }
                else if (UsbRxFIFOIntType & USB_RX_INT_TYPE_SETUP_TRANS_CMPL)
                {
                    gpstUsbOtgDevISR[DevID]->usbstatus = UDC_EV_SETUP_XFER_COMPL;
                }

                gpstUsbOtgDevISR[DevID]->CurEp = Ep;

                UsbIntDisalbe();
                rkos_semaphore_give_fromisr(gpstUsbOtgDevISR[DevID]->osUsbOtgOperSem);
            }
        //wirte other int service...
        }
    }
}

/*******************************************************************************
** Name: UsbOtgDev_GetEnumStatus
** Input:HDC dev
** Return: uint8
** Owner:wrm
** Date: 2014.10.8
** Time: 14:37:09
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_COMMON_
COMMON API uint8 UsbOtgDev_GetEnumStatus(HDC dev)
{
    //UsbOtgDev Resume...
    USBOTG_DEVICE_CLASS * pstUsbOtgDev =  (USBOTG_DEVICE_CLASS *)dev;
    if (pstUsbOtgDev == NULL)
    {
        //return RK_ERROR;
    }

    return gpstUsbOtgDevISR[0]->enum_ok_status;
}

/*******************************************************************************
** Name: UsbOtgDev_SetEnumStatus
** Input:HDC dev
** Return: uint8
** Owner:wrm
** Date: 2014.10.8
** Time: 14:37:09
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_COMMON_
COMMON API uint8 UsbOtgDev_SetEnumStatus(HDC dev, uint8 enum_status)
{
    //UsbOtgDev Resume...
    USBOTG_DEVICE_CLASS * pstUsbOtgDev =  (USBOTG_DEVICE_CLASS *)dev;
    if (pstUsbOtgDev == NULL)
    {
        //return RK_ERROR;
    }

    gpstUsbOtgDevISR[0]->enum_ok_status = enum_status;

    return 0;
}

/*******************************************************************************
** Name: UsbOtgDevResume
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.10.8
** Time: 14:37:09
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_COMMON_
COMMON API rk_err_t UsbOtgDevResume(HDC dev)
{
    //UsbOtgDev Resume...
    USBOTG_DEVICE_CLASS * pstUsbOtgDev =  (USBOTG_DEVICE_CLASS *)dev;
    if (pstUsbOtgDev == NULL)
    {
        //return RK_ERROR;
    }
    //CPU init
    ScuClockGateCtr(CLK_USBPHY_GATE, 1);
    ScuClockGateCtr(HCLK_USBC_GATE, 1);
    ScuSoftResetCtr(USBPHY_SRST, 0);
    ScuSoftResetCtr(USBGLB_SRST, 0);
    ScuSoftResetCtr(USBOTG_SRST, 0);
    DelayUs(5);
    Grf_otgphy_suspend(0);
    DelayUs(200);
}

/*******************************************************************************
** Name: UsbOtgDevSuspend
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.10.8
** Time: 14:37:09
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_COMMON_
COMMON API rk_err_t UsbOtgDevSuspend(HDC dev)
{
    //UsbOtgDev Suspend...
    USBOTG_DEVICE_CLASS * pstUsbOtgDev =  (USBOTG_DEVICE_CLASS *)dev;
    if (pstUsbOtgDev == NULL)
    {
        //return RK_ERROR;
    }
    //CPU init
    ScuClockGateCtr(CLK_USBPHY_GATE, 0);
    ScuClockGateCtr(HCLK_USBC_GATE, 0);
    DelayUs(5);
    Grf_otgphy_suspend(1);
    DelayUs(200);
}

/*******************************************************************************
** Name: UsbOtgDev_Write
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.10.8
** Time: 14:37:09
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_COMMON_
COMMON API rk_err_t UsbOtgDev_Write(HDC dev, uint8 ep, uint8 * buf, uint32 size)
{
    //UsbOtgDev Write...
    USBOTG_DEVICE_CLASS * pstUsbOtgDev =  (USBOTG_DEVICE_CLASS *)dev;
    rk_err_t ret = 0;
    if (pstUsbOtgDev == NULL)
    {
        return RK_ERROR;
    }
    if (ep == 0)
    {
        USBWriteEp(ep,size, buf);
    }
    else if (ep == 1)
    {
        VERIFY_PARAMS* params = (VERIFY_PARAMS*)buf;
        if ((params->number==VERIFY_NUMBER)&&(size == VERIFY_SIZE))
        {
            //使能EP
            printf ("\nusb:---Stall lock\n");
            USBSetEpSts(params->dir,params->bulk,params->status);

            rkos_semaphore_take(gpstUsbOtgDevISR[0]->osUsbOtgEpSem[2],MAX_DELAY);
            printf ("\nusb:---------Stall unlock\n");
        }
        else
        {
            UsbSetIntMsk();
            rkos_semaphore_take(gpstUsbOtgDevISR[0]->osUsbOtgEpSem[1],MAX_DELAY);
            ret = USBWriteEp(ep,size, buf);
            if (ret == RK_ERROR)
            {
                printf ("\nusbotg:write error\n");
                return RK_ERROR;
            }
        }
    }

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: UsbOtgDev_Connect
** Input:HDC dev, uint32 Speed
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.10.10
** Time: 14:28:54
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_COMMON_
COMMON API rk_err_t UsbOtgDev_Connect(HDC dev, uint32 Speed)
{
    USBOTG_DEVICE_CLASS * pstUsbOtgDev = (USBOTG_DEVICE_CLASS *)dev;

    RKTaskCreate(TASK_ID_USBOTG0, 0, NULL, SYNC_MODE);

    gpstUsbOtgDevISR[0]->usbmode = DEVICE_MODE;
    gpstUsbOtgDevISR[0]->threadstatus = 1;
    //High speed
    USBConnect(Speed);

}

/*******************************************************************************
** Name: UsbOtgDev_DisConnect
** Input:void
** Return: rk_err_t
** Owner:wrm
** Date: 2014.10.10
** Time: 14:28:54
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_COMMON_
COMMON API rk_err_t UsbOtgDev_DisConnect(void)
{
    USBDisconnect();
    return 0;
}

/*******************************************************************************
** Name: UsbOtgDev_IntDisalbe
** Input:void
** Return: rk_err_t
** Owner:wrm
** Date: 2014.10.10
** Time: 14:28:54
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_COMMON_
COMMON API rk_err_t UsbOtgDev_IntDisalbe(void)
{
    UsbIntDisalbe();
    return 0;
}




/*******************************************************************************
** Name: UsbOtgDev_SetDes
** Input:HDC dev, USBOGT_DESCRIPTOR * pDes
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.5.25
** Time: 10:13:16
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_COMMON_
COMMON API rk_err_t UsbOtgDev_SetDes(HDC dev, USBOGT_DESCRIPTOR * pDes)
{
    USBOTG_DEVICE_CLASS * pstUsbOtgDev = (USBOTG_DEVICE_CLASS *)dev;
    pstUsbOtgDev->pDevDes = pDes->pDevDes;
    pstUsbOtgDev->pConfigDes = pDes->pConfigDes;
    pstUsbOtgDev->pInterfaceDes = pDes->pInterfaceDes;
    pstUsbOtgDev->pEnDes[0] = pDes->pEnDes[0];
    pstUsbOtgDev->pEnDes[1] = pDes->pEnDes[1];
    pstUsbOtgDev->pEnDes[2] = pDes->pEnDes[2];

    pstUsbOtgDev->pOtherConfigDes = pDes->pOtherConfigDes;
    pstUsbOtgDev->pHsQualifier = pDes->pHsQualifier;
    pstUsbOtgDev->pUsbPowerDes = pDes->pUsbPowerDes;
    pstUsbOtgDev->pUsbHubDes = pDes->pUsbHubDes;
    pstUsbOtgDev->pUsbLangId = pDes->pUsbLangId;
    pstUsbOtgDev->pDevString = pDes->pDevString;
    pstUsbOtgDev->pMString = pDes->pMString;
    pstUsbOtgDev->pSerialNum = pDes->pSerialNum;
    pstUsbOtgDev->pConfigString = pDes->pConfigString;
    pstUsbOtgDev->pInterfaceString = pDes->pInterfaceString;
    pstUsbOtgDev->pfReadEp = pDes->pfReadEp;
    pstUsbOtgDev->ConfigLen = pDes->ConfigLen;

    /**Host Part**/
    pstUsbOtgDev->pMscHost = pDes->pMscHost;

    return RK_SUCCESS;

}

/*******************************************************************************
** Name: UsbOtgDev_Task_DeInit
** Input:void *pvParameters
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.10.8
** Time: 14:37:09
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_COMMON_
COMMON API rk_err_t UsbOtgDev0_Task_DeInit(void *pvParameters)
{
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: UsbOtgDev_Task_Init
** Input:void *pvParameters
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.10.8
** Time: 14:37:09
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_COMMON_
COMMON API rk_err_t UsbOtgDev0_Task_Init(void *pvParameters)
{
    return RK_SUCCESS;
}

#if  1
/*******************************************************************************
** Name: UsbOtgHost_Task_DeInit
** Input:void *pvParameters
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.10.8
** Time: 14:37:09
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_COMMON_
COMMON API rk_err_t UsbOtgHost_Task_DeInit(void *pvParameters)
{
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: UsbOtgHost_Task_Init
** Input:void *pvParameters
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.10.8
** Time: 14:37:09
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_COMMON_
COMMON API rk_err_t UsbOtgHost_Task_Init(void *pvParameters)
{
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: UsbOtgHost_Task_Enter
** Input:void
** Return: void
** Owner:wrm
** Date: 2014.10.8
** Time: 14:37:09
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_COMMON_
COMMON API void UsbOtgHost_Task_Enter(void)
{
#ifdef USB_HOST
    uint8 buf[512]={0};
    int32 ret = 0;

    HTC hSelf;
    hSelf = RKTaskGetRunHandle();

    while (1)
    {
        //printf ("Sever: UsbOtgHost_Task_Enter\n");
        rkos_semaphore_take(gpstUsbOtgDevISR[0]->osUsbOtgOperSem, MAX_DELAY);

        if(UsbOtgHostCheckIdle(hSelf) != RK_SUCCESS)
        {
            continue;
        }

        gpstUsbOtgDevISR[0]->gURBCache.valid = 0;
        //在枚举后可以正常热插拔,开始默认从0地址读写数据
        gpstUsbOtgDevISR[0]->pHDev.DevNum = 0;
        gpstUsbOtgDevISR[0]->pHDev.toggle[0] = 0;

        ret = HostPortInit(&gpstUsbOtgDevISR[0]->pHDev);
        if (ret < 0)
        {

        }

        ret = HostNewDev(&gpstUsbOtgDevISR[0]->pHDev);
        if (ret < 0)
        {

        }
        gpstUsbOtgDevISR[0]->HostProcessing = 0;
        gpstUsbOtgDevISR[0]->ChannelInfo[0].DisStatus = 1;
        printf ("\n---------------------------------------------\n");
        printf("Enum USB Device OK! \nUSB Device Class is:0x%x\n",
               gpstUsbOtgDevISR[0]->pHDev.intf.IfDesc.bInterfaceClass);
        printf ("0x01:USB Audio Device\n");
        printf ("0x02:USB Commucation Device\n");
        printf ("0x03:USB HID Device\n");
        printf (".....\n");
        printf ("0x08:USB Massage Storage Device\n");
        printf ("\n---------------------------------------------\n");
        switch (gpstUsbOtgDevISR[0]->pHDev.intf.IfDesc.bInterfaceClass)
        {
            case USB_CLASS_AUDIO:
                break;
            case USB_CLASS_COMM:
                break;
            case USB_CLASS_HID:
                break;
            case USB_CLASS_PHYSICAL:
                break;
            case USB_CLASS_STILL_IMAGE:
                break;
            case USB_CLASS_PRINTER:
                break;
            case USB_CLASS_MASS_STORAGE:
                if (gpstUsbOtgDevISR[0]->UsbHostMscDeviceList == 0)
                {
                    DeviceTask_CreateDeviceList(DEVICE_LIST_HOST_USBMSC, NULL, SYNC_MODE);
                    gpstUsbOtgDevISR[0]->UsbHostMscDeviceList = 1;
                }

                gpstUsbOtgDevISR[0]->pMscHost(&gpstUsbOtgDevISR[0]->pHDev.intf);
                break;
            case USB_CLASS_HUB:

                break;
            default:
                break;
        }

    }
#endif
}

#endif

/*******************************************************************************
** Name: UsbOtgDev_Delete
** Input:uint32 DevID
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.10.8
** Time: 14:37:09
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_INIT_
COMMON API rk_err_t UsbOtgDev_Delete(uint32 DevID ,void * arg)
{

#if 1
    DevID = 0;
    //printf ("\nUSB OTG Delete \n");
    //Check UsbOtgDev is not exist...
    if (gpstUsbOtgDevISR[DevID] == NULL)
    {
        return RK_ERROR;
    }

    //UsbOtgDev deinit...
    UsbOtgDevDeInit(gpstUsbOtgDevISR[DevID]);

    //Free UsbOtgDev memory...
    rkos_semaphore_delete(gpstUsbOtgDevISR[DevID]->osUsbOtgOperSem);
    rkos_semaphore_delete(gpstUsbOtgDevISR[DevID]->osUsbOtgEpSem[0]);
    rkos_semaphore_delete(gpstUsbOtgDevISR[DevID]->osUsbOtgEpSem[1]);
    rkos_semaphore_delete(gpstUsbOtgDevISR[DevID]->osUsbOtgEpSem[2]);
    rkos_memory_free(gpstUsbOtgDevISR[DevID]);

    //Delete UsbOtgDev...
    gpstUsbOtgDevISR[DevID] = NULL;
    //Delete UsbOtgDev Read and Write Module...
    FW_RemoveSegment(SEGMENT_ID_USBOTG_DEV);

#endif
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: UsbOtgDevDeInit
** Input:USBOTG_DEVICE_CLASS * pstUsbOtg
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.10.8
** Time: 14:37:09
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_INIT_
COMMON FUN rk_err_t UsbOtgDevDeInit(USBOTG_DEVICE_CLASS * pstUsbOtgDev)
{
    if (gpstUsbOtgDevISR[0]->usbmode == DEVICE_MODE)
    {
        printf ("\nOTG DEVICE Delete\n");
        if (gpstUsbOtgDevISR[0]->threadstatus == 1)
        {
            RKTaskDelete (TASK_ID_USBOTG0, 0, ASYNC_MODE);//DIRECT_MODE ASYNC_MODE
            gpstUsbOtgDevISR[0]->threadstatus = 0;
        }
    }
    #ifdef USB_HOST
    else if (gpstUsbOtgDevISR[0]->usbmode == HOST_MODE)
    {
        printf ("\nOTG HOST Delete\n");
        RKTaskDelete (TASK_ID_USBOTG1, 0, ASYNC_MODE);
        HostRegDeinit();
    }
    #endif
    return RK_SUCCESS;
}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/


/*******************************************************************************
** Name: UsbOtgDev_Create
** Input:uint32 DevID, void * arg
** Return: HDC
** Owner:aaron.sun
** Date: 2014.10.8
** Time: 14:37:09
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_INIT_
INIT API HDC UsbOtgDev_Create(uint32 DevID, void * arg)
{
    USBOTG_DEV_ARG * pstUsbOtgDevArg;
    DEVICE_CLASS* pstDev;
    USBOTG_DEVICE_CLASS * pstUsbOtgDev;

    if (arg == NULL)
    {
        return (HDC)RK_PARA_ERR;
    }

    // Create handler...
    pstUsbOtgDevArg = (USBOTG_DEV_ARG *)arg;
    pstUsbOtgDev =  rkos_memory_malloc(sizeof(USBOTG_DEVICE_CLASS));
    memset(pstUsbOtgDev, 0, sizeof(USBOTG_DEVICE_CLASS));
    if (pstUsbOtgDev == NULL)
    {
        return NULL;
    }

    //init handler...
    pstUsbOtgDev->osUsbOtgOperSem  = rkos_semaphore_create(1,0);

    pstUsbOtgDev->osUsbOtgEpSem[0]  = rkos_semaphore_create(1,0);
    pstUsbOtgDev->osUsbOtgEpSem[1]  = rkos_semaphore_create(1,0);
    pstUsbOtgDev->osUsbOtgEpSem[2]  = rkos_semaphore_create(1,0);


    if (((pstUsbOtgDev->osUsbOtgOperSem) == 0)
            || ((pstUsbOtgDev->osUsbOtgEpSem[0]) == 0)
            || ((pstUsbOtgDev->osUsbOtgEpSem[1]) == 0)
            || ((pstUsbOtgDev->osUsbOtgEpSem[2]) == 0))
    {
        rkos_semaphore_delete(pstUsbOtgDev->osUsbOtgOperSem);
        rkos_semaphore_delete(pstUsbOtgDev->osUsbOtgEpSem[0]);
        rkos_semaphore_delete(pstUsbOtgDev->osUsbOtgEpSem[1]);
        rkos_semaphore_delete(pstUsbOtgDev->osUsbOtgEpSem[2]);
        rkos_memory_free(pstUsbOtgDev);
        return (HDC) RK_ERROR;
    }

    pstDev = (DEVICE_CLASS *)pstUsbOtgDev;
    pstDev->suspend = UsbOtgDevSuspend;
    pstDev->resume  = UsbOtgDevResume;

    //init arg...
    pstUsbOtgDev->usbmode = pstUsbOtgDevArg->usbmode;

    //module overlay...
#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    FW_LoadSegment(SEGMENT_ID_USBOTG_DEV, SEGMENT_OVERLAY_CODE);
#endif

    gpstUsbOtgDevISR[DevID] = NULL;

    UsbOtgDevHwInit(DevID, 0);

    //device init...
    if (UsbOtgDevInit(pstUsbOtgDev) != RK_SUCCESS)
    {
        rkos_semaphore_delete(pstUsbOtgDev->osUsbOtgOperSem);
        rkos_semaphore_delete(pstUsbOtgDev->osUsbOtgEpSem[0]);
        rkos_semaphore_delete(pstUsbOtgDev->osUsbOtgEpSem[1]);
        rkos_semaphore_delete(pstUsbOtgDev->osUsbOtgEpSem[2]);
        rkos_memory_free(pstUsbOtgDev);
        return (HDC) RK_ERROR;
    }

    gpstUsbOtgDevISR[DevID] = pstUsbOtgDev;

    return (HDC)pstDev;

}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: UsbOtgDevInit
** Input:USBOTG_DEVICE_CLASS * pstUsbOtg
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.10.8
** Time: 14:37:09
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_INIT_
INIT FUN rk_err_t UsbOtgDevInit(USBOTG_DEVICE_CLASS * pstUsbOtgDev)
{

    //some ip device,such as uart ip device, only a read fifo and a write fifo,
    //we can use fifo as buffer for save some data when shut off Ip int

    //RKTaskCreate(TASK_ID_USBOTG0, 0, NULL, SYNC_MODE);
    pstUsbOtgDev->UsbHostMscDeviceList = 0;
    pstUsbOtgDev->early_suspend_status = 0;
    pstUsbOtgDev->suspend_status = 0;
    pstUsbOtgDev->reset_status = 0;
    pstUsbOtgDev->enum_ok_status = 0;

    return RK_SUCCESS;
}


#ifdef _USBOTG_DEV_SHELL_
_DRIVER_USB_USBOTGDEV_SHELL_DATA_
static SHELL_CMD ShellUsbOtgName[] =
{
    "pcb",NULL,"NULL","NULL",
    "create",NULL,"NULL","NULL",
    "delete",NULL,"NULL","NULL",
    "test",NULL,"NULL","NULL",
    "help",NULL,"NULL","NULL",
    "suspend",NULL,"NULL","NULL",
    "resume",NULL,"NULL","NULL",
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
** Name: UsbOtgDev_Shell
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.10.8
** Time: 14:37:09
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_SHELL_
SHELL API rk_err_t UsbOtgDev_Shell(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret;

    uint8 Space;

    StrCnt = ShellItemExtract(pstr,&pItem, &Space);

    if (StrCnt == 0)
    {
        return RK_ERROR;
    }

    ret = ShellCheckCmd(ShellUsbOtgName, pItem, StrCnt);
    if (ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;                                                 //remove '.',the point is the useful item

    switch (i)
    {
        case 0x00:
            ret = UsbOtgDevShellPcb(dev,pItem);
            break;

        case 0x01:
            ret = UsbOtgDevShellMc(dev,pItem);
            break;

        case 0x02:
            ret = UsbOtgDevShellDel(dev,pItem);
            break;

        case 0x03:
            ret = UsbOtgDevShellTest(dev,pItem);
            break;

        case 0x04:
            ret = UsbOtgDevShellHelp(dev,pItem);
            break;

        case 0x05:
            ret = UsbOtgDevShellSuspend(dev,pItem);
            break;

        case 0x06:
            ret = UsbOtgDevShellResume(dev,pItem);
            break;

        default:
            ret = RK_ERROR;
            break;
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
** Name: UsbOtgDevShellHelp
** Input:HDC dev, const uint8 * pstr
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.3
** Time: 17:00:42
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_SHELL_
SHELL FUN rk_err_t UsbOtgDevShellHelp(HDC dev, uint8 * pstr)
{
    pstr--;

    if ( StrLenA((uint8 *) pstr) != 0)
        return RK_ERROR;

    rk_print_string("usbotg命令集提供了一系列的命令对usbotg进行操作\r\n");
    rk_print_string("usbotg包含的子命令如下:           \r\n");
    rk_print_string("pcb       显示pcb信息             \r\n");
    rk_print_string("create    创建usbotg              \r\n");
    rk_print_string("del       删除usbotg              \r\n");
    rk_print_string("test      测试usbotg命令          \r\n");
    rk_print_string("help      显示usbotg命令帮助信息  \r\n");
    rk_print_string("suspend   显示suspend命令帮助信息 \r\n");
    rk_print_string("resume    显示resume命令帮助信息  \r\n");

    return RK_SUCCESS;
}
/*******************************************************************************
** Name: UsbOtgDevShellSuspend
** Input:HDC dev, const uint8 * pstr
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.3
** Time: 17:00:42
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_SHELL_
SHELL FUN rk_err_t UsbOtgDevShellSuspend(HDC dev, uint8 * pstr)
{
    UsbOtgDevSuspend(dev);

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: UsbOtgDevShellResume
** Input:HDC dev, const uint8 * pstr
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.3
** Time: 17:00:42
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_SHELL_
SHELL FUN rk_err_t UsbOtgDevShellResume(HDC dev, uint8 * pstr)
{
    UsbOtgDevResume(dev);

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: UsbOtgDevShellTest
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.10.8
** Time: 14:37:09
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_SHELL_
SHELL FUN rk_err_t UsbOtgDevShellTest(HDC dev, uint8 * pstr)
{
    HDC hUsbOtgDev;
    uint32 DevID;

#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            rk_print_string("usbotg.test : usbotgdev test cmd.\r\n");
            return RK_SUCCESS;
        }
    }
#endif

    //Get UsbOtgDev ID...
    if (StrCmpA(pstr, "0", 1) == 0)
    {
        DevID = 0;
    }
    else if (StrCmpA(pstr, "1", 1) == 0)
    {
        DevID = 1;
    }
    else
    {
        DevID = 0;
    }

    //Open UsbOtgDev...
    hUsbOtgDev = RKDev_Open(DEV_CLASS_USBOTG, 0, NOT_CARE);

    if ((hUsbOtgDev == NULL) || (hUsbOtgDev == (HDC)RK_ERROR) || (hUsbOtgDev == (HDC)RK_PARA_ERR))
    {
        rk_print_string("UsbOtgDev open failure");
        return RK_SUCCESS;
    }

    //do test otgdev....
    //UsbOtgDev_Connect(hUsbOtgDev, 0);
    //do test otgHost...
    #ifdef USB_HOST
    UsbOtgHost_Connect(hUsbOtgDev,0);
    #endif
    //close UsbOtgDev...

    RKDev_Close(hUsbOtgDev);
    return RK_SUCCESS;

}
/*******************************************************************************
** Name: UsbOtgDevShellDel
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.10.8
** Time: 14:37:09
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_SHELL_
SHELL FUN rk_err_t UsbOtgDevShellDel(HDC dev, uint8 * pstr)
{
    uint32 DevID;

#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            rk_print_string("usbotg.del : usbotgdev del cmd.\r\n");
            return RK_SUCCESS;
        }
    }
#endif

    //Get UsbOtgDev ID...
    if (StrCmpA(pstr, "0", 1) == 0)
    {
        DevID = 0;
    }
    else if (StrCmpA(pstr, "1", 1) == 0)
    {
        DevID = 1;
    }
    else
    {
        DevID = 0;
    }

    if (RKDev_Delete(DEV_CLASS_USBOTG, DevID, NULL) != RK_SUCCESS)
    {
        rk_print_string("UsbOtgDev delete failure");
    }

    return RK_SUCCESS;

}

/*******************************************************************************
** Name: UsbOtgDevShellMc
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.10.8
** Time: 14:37:09
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_SHELL_
SHELL FUN rk_err_t UsbOtgDevShellMc(HDC dev, uint8 * pstr)
{
    USBOTG_DEV_ARG stUsbOtgDevArg;
    rk_err_t ret;
    uint32 DevID;

#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            rk_print_string("usbotg.mc : usbotgdev mc cmd.\r\n");
            return RK_SUCCESS;
        }
    }
#endif

    if (StrCmpA(pstr, "0", 1) == 0)
    {
        DevID = 0;
    }
    else if (StrCmpA(pstr, "1", 1) == 0)
    {
        DevID = 1;
    }
    else
    {
        DevID = 0;
    }


    //Init UsbOtgDev arg...
    stUsbOtgDevArg.usbmode = USBOTG_MODE_DEVICE;

    //Create UsbOtgDev...
    ret = RKDev_Create(DEV_CLASS_USBOTG, DevID, &stUsbOtgDevArg);
    if (ret != RK_SUCCESS)
    {
        rk_print_string("UsbOtgDev create failure");
    }

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: UsbOtgDevShellPcb
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.10.8
** Time: 14:37:09
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_SHELL_
SHELL FUN rk_err_t UsbOtgDevShellPcb(HDC dev, uint8 * pstr)
{
    HDC hUsbOtgDev;
    uint32 DevID;

#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            rk_print_string("usbotg.pcb : usbotg pcb info cmd.\r\n");
            return RK_SUCCESS;
        }
    }
#endif

    //Get UsbOtgDev ID...
    if (StrCmpA(pstr, "0", 1) == 0)
    {
        DevID = 0;
    }
    else if (StrCmpA(pstr, "1", 1) == 0)
    {
        DevID = 1;
    }
    else
    {
        DevID = 0;
    }

    //Display pcb...
    if (gpstUsbOtgDevISR[DevID] != NULL)
    {
        rk_print_string("\r\n");
        rk_printf_no_time("UsbOtgDev ID = %d Pcb list as follow:", DevID);
        rk_printf_no_time("\t&stUsbOtgDevice---0x%08x", &gpstUsbOtgDevISR[DevID]->stUsbOtgDevice);
        rk_printf_no_time("\tosUsbOtgOperReqSem---0x%08x", gpstUsbOtgDevISR[DevID]->osUsbOtgOperSem);
        rk_printf_no_time("\tusbmode---0x%08x", gpstUsbOtgDevISR[DevID]->usbmode);
        rk_printf_no_time("\tusbspeed---0x%08x", gpstUsbOtgDevISR[DevID]->usbspeed);
    }
    else
    {
        rk_print_string("\r\n");
        rk_printf_no_time("UsbOtgDev ID = %d not exit", DevID);
    }
    return RK_SUCCESS;

}



#endif
#endif

