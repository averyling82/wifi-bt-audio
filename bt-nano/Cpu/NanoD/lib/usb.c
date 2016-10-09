/*
********************************************************************************************
*
*                Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Cpu\NanoD\lib\usb.c
* Owner: aaron.sun
* Date: 2014.9.24
* Time: 16:16:06
* Desc: usb bsp
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2014.9.24     16:16:06   1.0
********************************************************************************************
*/
#define __CPU_NANOD_LIB_USB_C__
#include "BspConfig.h"
#ifdef __CPU_NANOD_LIB_USB_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "RKOS.h"
#include "BSP.h"


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
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/



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
void  UsbControllerInit(bool FullSpeed);
void  UsbHostControllerInit(bool FullSpeed);
void  USBResetPhy(void);
static void  HostPortIsr(void);
void  HostRxPkt(UHC_CHN_INFO *hc, uint32 bytes);
void  HostCompleteUrb(USB_REQ *urb, int32 status);
int32 HostProcessXfer(UHC_CHN_INFO * pUsbHC);
int32 HostHaltChn(UHC_CHN_INFO* hc, uint32 status);
void  HostInitChn(UHC_CHN_INFO *hc, USB_REQ *urb);
int32 HostUpdateXfer(UHC_CHN_INFO* hc, USB_REQ *urb, uint32 ChnStatus);
void  HostCompleteXfer(UHC_CHN_INFO* hc, HOST_CHANNEL_REG *pChnReg, uint32 status);
void  HostReleaseChn(UHC_CHN_INFO * hc);
void  HostSaveToggle(UHC_CHN_INFO* hc, USB_REQ *urb);
static void  HostStartXfer(UHC_CHN_INFO *hc);
static void  HostDoPing(UHC_CHN_INFO* hc);
static void  HostTxPkt(UHC_CHN_INFO *hc);
static void  HostTxData(int8 chn, uint32 *pBuf, uint32 len);
static int32 HostContinueXfer(UHC_CHN_INFO *hc);
static void  HostRxData(int8 chn, uint32 *pBuf, uint32 len);
static void  HostRxflvlIsr(UHC_CHN_INFO * pUsbHC);
static void  HostFreeChn(UHC_CHN_INFO * hc);
static void  HostXfercompIsr(UHC_CHN_INFO* hc);
static void  HostChnIsr(int8 chn, UHC_CHN_INFO * pUsbHC);
static void  HostChhltdIsr(UHC_CHN_INFO* hc);
static void  HostStallIsr(UHC_CHN_INFO* hc);
static void  HostNakIsr(UHC_CHN_INFO* hc);
static void  HostAckIsr(UHC_CHN_INFO* hc);
static void  HostNyetIsr(UHC_CHN_INFO* hc);
static void  HostXacterrIsr(UHC_CHN_INFO* hc);
static void  HostBabbleIsr(UHC_CHN_INFO* hc);

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: USBClearOutfEpStall
** Input:uint8 epnum
** Return: void
** Owner:aaron.sun
** Date: 2015.5.25
** Time: 18:51:16
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON API void USBSetOutEpStatus(uint8 epnum, uint8 status)
{
    DEVICE_REG *dev_regs = (DEVICE_REG *)USB_DEV_BASE;
    if (status != 0)
            dev_regs->out_ep[epnum].doepctl |= 1<<21;  //Out endpoint send stall handshack
        else
            dev_regs->out_ep[epnum].doepctl &= ~(1<<21);

}

/*******************************************************************************
** Name: USBClearInEpStall
** Input:uint8 epnum
** Return: void
** Owner:aaron.sun
** Date: 2015.5.25
** Time: 18:48:35
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON API void USBSetInEpStatus(uint8 epnum, uint8 status)
{
    DEVICE_REG *dev_regs = (DEVICE_REG *)USB_DEV_BASE;
    if (status != 0)
    {
         volatile uint32 value = dev_regs->in_ep[epnum].diepctl;
         (value & (1ul<<31))? (value |= (1<<30 | 1<<21)) : (value |= (1<<21));
         dev_regs->in_ep[epnum].diepctl = value;  //In endpoint send stall handshack
    }
    else
         dev_regs->in_ep[epnum].diepctl &= ~(1<<21);
}

/*******************************************************************************
** Name: USBEnumDone
** Input:uint32 * speed
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.10.10
** Time: 18:15:15
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON API rk_err_t USBEnumDone(uint32 * speed)
{
    DEVICE_REG *dev_regs = (DEVICE_REG *)USB_DEV_BASE;
    /* Set the MPS of the IN EP based on the enumeration speed */
    *speed = USB_SPEED_FULL;

    switch ((dev_regs->dsts >> 1) & 0x03)
    {
        case 0: //HS_PHY_30MHZ_OR_60MHZ:
            *speed = USB_SPEED_HIGH;
            dev_regs->in_ep[0].diepctl &= ~0x03;  //64bytes MPS
            break;

        case 1: //FS_PHY_30MHZ_OR_60MHZ:
        case 3: //FS_PHY_48MHZ
            dev_regs->in_ep[0].diepctl &= ~0x03;  //64bytes MPS
            break;

        case 2: //LS_PHY_6MHZ
        default:
            dev_regs->in_ep[0].diepctl |= 0x03;   //8bytes MPS
            break;
    }

    //pDev->request(0, UDC_EV_ENUM_DONE, speed, pDev);
    /* setup EP0 to receive SETUP packets */
    dev_regs->out_ep[0].doeptsiz = (1ul << 29) | (1ul << 19) | 0x40; //supcnt = 1, pktcnt = 1, xfersize = 64*1
    /* Enable OUT EP for receive */
    dev_regs->out_ep[0].doepctl |= (1ul<<31);

    dev_regs->dctl |= 1<<8;         //clear global IN NAK
}

/*******************************************************************************
** Name: USBReconnect
** Input:uint32 param
** Return: void
** Owner:aaron.sun
** Date: 2014.9.29
** Time: 15:35:15
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON API void USBReconnect(uint32 restart, uint32 fullspeed)
{

    if (1 == restart)
    {
        USBConnect(fullspeed);
    }
    else
    {
        UsbIntEnable();
    }
    return;

}


/*******************************************************************************
** Name: USBDisconnect
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2014.9.29
** Time: 15:34:10
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON API void USBDisconnect(void)
{
    DEVICE_REG *dev_regs = (DEVICE_REG *)USB_DEV_BASE;

    dev_regs->dctl |= 0x02;         //soft disconnect
}


/*******************************************************************************
** Name: USBConnect
** Input:bool FullSpeed
** Return: void
** Owner:aaron.sun
** Date: 2014.9.29
** Time: 15:33:44
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON API void USBConnect(bool FullSpeed)
{
    DEVICE_REG *dev_regs = (DEVICE_REG *)USB_DEV_BASE;
    UsbIntDisalbe();
    dev_regs->dctl |= 0x02;         //soft disconnec
    UsbControllerInit(FullSpeed);
    dev_regs->dctl &= ~0x02;        //soft connect
    UsbIntEnable();
}


/*******************************************************************************
** Name: USBReset
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2014.9.29
** Time: 15:21:37
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON API void USBReset(void)
{
    uint32 count;
    DEVICE_REG *dev_regs = (DEVICE_REG *)USB_DEV_BASE;
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;

    /* Clear the Remote Wakeup Signalling */
    otg_core->Device.dctl &= ~0x01;

    /* Set NAK for all OUT EPs */

    /* Flush the NP Tx FIFO */
    otg_core->Core.grstctl |= ( 0x10<<6) | (1<<5);     //Flush all Txfifo
    for (count=0; count<10000; count++)
    {
        if ((otg_core->Core.grstctl & (1<<5))==0)
            break;
    }

    /* Flush the Learning Queue. */
    otg_core->Core.grstctl |= 1<<3;

    /* Reset Device Address */
    otg_core->Device.dcfg &= ~0x07f0;

    /* setup EP0 to receive SETUP packets */

    /* Clear interrupt */
    otg_core->Core.gintsts = 0xFFFFFFFF;
}


/*******************************************************************************
** Name: USBGetRxFIFOIntType
** Input:void
** Return: uint32
** Owner:aaron.sun
** Date: 2014.9.25
** Time: 14:06:05
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON API uint32 USBGetRxFIFOIntType(uint32 * epnum, uint32 * datalen)
{
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;
    DEVICE_GRXSTS_DATA status;
    uint32 inttype;

    /* Disable the Rx Status Queue Level interrupt */
    otg_core->Core.gintmsk &= ~(1<<4);

    /* Get the Status from the top of the FIFO */
    status.d32 = otg_core->Core.grxstsp;
    inttype = 0;
    switch (status.b.pktsts)
    {
        case 0x01: //Global OUT NAK
            inttype |= USB_RX_INT_TYPE_GLOBAL_OUT_NAK;
            break;

        case 0x02: // OUT Data packet recived
            *epnum = status.b.epnum;
            *datalen = status.b.bcnt;
            inttype |= USB_RX_INT_TYPE_OUT_RECIVE_PACKAGE;
            break;

        case 0x03://OUT transfer completed
            if(status.b.epnum == 0)
            {
                otg_core->Device.out_ep[status.b.epnum].doepctl = (1ul<<15) | (1ul<<26) | (1ul<<31);
            }
            else
            {
                otg_core->Device.out_ep[status.b.epnum].doepctl |= (1ul<<26) | (1ul<<31);
            }
            inttype |= USB_RX_INT_TYPE_OUT_TRANS_CMPL;
            break;


        case 0x04: // SETUP transfer completed
            otg_core->Device.out_ep[0].doepctl = (1ul<<15) | (1ul<<26) | (1ul<<31);
            inttype |= USB_RX_INT_TYPE_SETUP_TRANS_CMPL;
            break;

        case 0x06: //SETUP data packet received
            *epnum = status.b.epnum;
            *datalen = status.b.bcnt;
            inttype |= USB_RX_INT_TYPE_SETUP_RECIVE_PACKAGE;
            break;
        default:
            break;
    }


    /* Enable the Rx Status Queue Level interrupt */
    otg_core->Core.gintmsk |= (1<<4);
    /* Clear interrupt */
    otg_core->Core.gintsts = (1<<4);

    return inttype;

}

/*******************************************************************************
** Name: USBGetOutEpIntType
** Input:void
** Return: uint32
** Owner:aaron.sun
** Date: 2014.9.25
** Time: 14:00:04
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON API uint32 USBGetOutEpIntType(uint32 * ep)
{
    uint32 ep_intr;
    uint32 doepint;
    uint32 epnum = 0;
    uint32 inttype;
    DEVICE_REG *dev_regs = (DEVICE_REG *)USB_DEV_BASE;

    /* Read in the device interrupt bits */
    ep_intr = (dev_regs->daint & dev_regs->daintmsk) >> 16;

    inttype = 0;

    while(ep_intr)
    {
        if (ep_intr & 0x1)
        {

            doepint = dev_regs->out_ep[epnum].doepint & dev_regs->doepmsk;

            /* Transfer complete */
            if ( doepint & 0x01 )
            {
                /* Clear the bit in DOEPINTn for this interrupt */
                dev_regs->out_ep[epnum].doepint = 0x01;
                inttype |= USB_OUT_EP_INT_TYPE_TRANS_OK;
            }

            /* Endpoint disable  */
            if ( doepint & 0x02 )
            {
                /* Clear the bit in DOEPINTn for this interrupt */
                dev_regs->out_ep[epnum].doepint = 0x02;
                inttype |= USB_OUT_EP_INT_TYPE_EP_DISABLE;
            }

            /* AHB Error */
            if ( doepint & 0x04 )
            {
                dev_regs->out_ep[epnum].doepint = 0x04;
                inttype |= USB_OUT_EP_INT_TYPE_AHB_ERR;
            }

            /* Setup Phase Done (contorl EPs) */
            if ( doepint & 0x08 )
            {
                dev_regs->out_ep[epnum].doepint = 0x08;
                inttype |= USB_OUT_EP_INT_TYPE_SETUP_PHASE_OVER;
            }

            dev_regs->out_ep[epnum].doepint = 0xffffffff;
        }

        if(inttype)
        {
           *ep = epnum;
          break;
        }

        epnum++;
        ep_intr >>= 1;

    }

}

/*******************************************************************************
** Name: USBGetInEpIntType
** Input:void
** Return: uint32
** Owner:aaron.sun
** Date: 2014.9.25
** Time: 13:59:22
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON API uint32 USBGetInEpIntType(uint32 * ep)
{
    uint32 ep_intr;
    uint32 diepint, msk;
    uint32 epnum = 0;
    DEVICE_REG *dev_regs = (DEVICE_REG *)USB_DEV_BASE;

    uint32 inttype;

    /* Read in the device interrupt bits */
    ep_intr = (dev_regs->daint & dev_regs->daintmsk) & 0xFFFF;

    inttype = 0;
    /* Service the Device IN interrupts for each endpoint */
    while (ep_intr)
    {
        if (ep_intr&0x1)
        {
            msk = dev_regs->diepmsk | ((dev_regs->dtknqr4_fifoemptymsk & 0x01)<<7);   //because of the low 7bit being reserved
            diepint = dev_regs->in_ep[epnum].diepint & msk;

            /* Transfer complete */
            if ( diepint & 0x01 )
            {
                /* Clear the bit in DIEPINTn for this interrupt */
                 //dev_regs->in_ep[epnum].diepint = 0x01;
                 inttype |= USB_IN_EP_INT_TYPE_TRANS_OK;
            }

            /* Endpoint disable  */
            if (diepint & 0x02)
            {
                /* Clear the bit in DIEPINTn for this interrupt */
                //dev_regs->in_ep[epnum].diepint = 0x02;
                inttype |= USB_IN_EP_INT_TYPE_EP_DISABLE;
            }

            /* AHB Error */
            if ( diepint & 0x04 )
            {
                /* Clear the bit in DIEPINTn for this interrupt */
                //dev_regs->in_ep[epnum].diepint = 0x04;
                inttype |= USB_IN_EP_INT_TYPE_AHB_ERR;
            }

            /* TimeOUT Handshake (non-ISOC IN EPs) */
            if ( diepint & 0x08 )
            {
                //dev_regs->in_ep[epnum].diepint = 0x08;
                inttype |= USB_IN_EP_INT_TYPE_TIME_OUT;
            }

            dev_regs->in_ep[epnum].diepint = 0xffffffff;
        }

        if(inttype)
        {
          *ep = epnum;
          break;
        }

        epnum++;
        ep_intr >>=1;

    }
}

/*******************************************************************************
** Name: USBSetAddr
** Input:uint32 addr
** Return: void
** Owner:aaron.sun
** Date: 2014.9.25
** Time: 10:56:23
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON API void USBSetAddr(uint32 addr)
{
     DEVICE_REG *dev_regs = (DEVICE_REG *)USB_DEV_BASE;

     dev_regs->dcfg = (dev_regs->dcfg & (~0x07f0)) | (addr << 4);  //reset device addr
}

/*******************************************************************************
** Name: USBInEpSetData0
** Input:uint8 dir, uint8 epnum
** Return: void
** Owner:zhuzhe
** Date: 2014.2.27
** Time: 10:19:26
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
void USBInEpSetData0(uint8 epnum)
{
    DEVICE_REG *dev_regs = (DEVICE_REG *)USB_DEV_BASE;

    dev_regs->in_ep[epnum].diepctl  |= 1<<28;
}

/*******************************************************************************
** Name: USBOutEpSetData0
** Input:uint8 dir, uint8 epnum
** Return: void
** Owner:zhuzhe
** Date: 2014.2.27
** Time: 10:19:26
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
void USBOutEpSetData0(uint8 dir, uint8 epnum)
{
    DEVICE_REG *dev_regs = (DEVICE_REG *)USB_DEV_BASE;

    dev_regs->out_ep[epnum].doepctl |= 1<<28;
}

/*******************************************************************************
** Name: USBInEpSetNak
** Input:uint8 epnum
** Return: void
** Owner:zhuzhe
** Date: 2014.2.27
** Time: 10:23:37
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
void  USBInEpSetNak(uint8 epnum)
{
    DEVICE_REG *dev_regs = (DEVICE_REG *)USB_DEV_BASE;

    dev_regs->in_ep[(epnum & 0x0f)].diepctl  |= (1ul<<27) ; // set Nak
}

/*******************************************************************************
** Name: USBOutEpSetNak
** Input:uint8 epnum
** Return: void
** Owner:zhuzhe
** Date: 2014.2.27
** Time: 10:23:37
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
void  USBOutEpSetNak(uint8 epnum)
{
    DEVICE_REG *dev_regs = (DEVICE_REG *)USB_DEV_BASE;

    dev_regs->out_ep[epnum].doepctl |= (1ul<<27) ; // set Nak
}

/*******************************************************************************
** Name: USBInEpClearNak
** Input:uint8 epnum
** Return: void
** Owner:zhuzhe
** Date: 2014.2.27
** Time: 10:22:34
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
void USBInEpClearNak(uint8 epnum)
{
    DEVICE_REG *dev_regs = (DEVICE_REG *)USB_DEV_BASE;
    dev_regs->in_ep[(epnum & 0x0f)].diepctl |= (1ul<<26) | (1ul<<31);  //Active ep, Clr Nak, endpoint enable
}

/*******************************************************************************
** Name: USBOutEpClearNak
** Input:uint8 epnum
** Return: void
** Owner:zhuzhe
** Date: 2014.2.27
** Time: 10:22:34
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
void USBOutEpClearNak(uint8 epnum)
{
    DEVICE_REG *dev_regs = (DEVICE_REG *)USB_DEV_BASE;
    dev_regs->out_ep[epnum].doepctl |= (1ul<<26) | (1ul<<31); //Active ep, Clr Nak, endpoint enable
}

/*******************************************************************************
** Name: USBInEpStall
** Input:uint8 epnum
** Return: void'
** Owner:zhuzhe
** Date: 2014.2.27
** Time: 10:21:01
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
void  USBSetInEpStall(uint8 epnum)
{
    //write_XDATA(USB_CSR0, CSR0_OUT_SEVD | CSR0_SEND_STALL);     //send stall
    DEVICE_REG *dev_regs = (DEVICE_REG *)USB_DEV_BASE;

    dev_regs->in_ep[(epnum & 0x0f)].diepctl |= 1<<21;   //send IN0 stall handshack

}

/*******************************************************************************
** Name: USBOutEpStall
** Input:uint8 epnum
** Return: void'
** Owner:zhuzhe
** Date: 2014.2.27
** Time: 10:21:01
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
void  USBSetOutEpStall(uint8 epnum)
{
    //write_XDATA(USB_CSR0, CSR0_OUT_SEVD | CSR0_SEND_STALL);     //send stall
    DEVICE_REG *dev_regs = (DEVICE_REG *)USB_DEV_BASE;

    dev_regs->out_ep[epnum].doepctl |= 1<<21;  //send OUT0 stall handshack

}

/*******************************************************************************
** Name: USBCheckInEpStall
** Input:uint8 dir, uint8 epnum
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.9.25
** Time: 10:36:15
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON API uint8 USBCheckInEpStall(uint8 epnum)
{
    DEVICE_REG *dev_regs = (DEVICE_REG *)USB_DEV_BASE;
    return(dev_regs->in_ep[epnum].diepctl>>21) & 0x1;
}

/*******************************************************************************
** Name: USBCheckInEpStall
** Input:uint8 dir, uint8 epnum
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.9.25
** Time: 10:36:15
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON API uint8 USBCheckOutEpStall(uint8 epnum)
{
    DEVICE_REG *dev_regs = (DEVICE_REG *)USB_DEV_BASE;
    return (dev_regs->out_ep[epnum].doepctl>>21) & 0x1;  //Out endpoint send stall handshack

}

/*******************************************************************************
** Name: USBWriteEp
** Input:uint32 epnum, uint32 len, uint8 * buf
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.9.25
** Time: 10:07:30
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON API rk_err_t USBWriteEp(uint32 epnum, uint32 len, uint8 * buf)
{
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;

    uint32 i,j=0;
    DEPCTL_DATA diepctl;
    uint32 *fifo = (uint32 *)USB_EP_FIFO(epnum);
    IN_EP_REG * in_ep = (IN_EP_REG *)&otg_core->Device.in_ep[epnum];

LOOP:
    if (((otg_core->Core.gnptxsts & 0xFFFF) >= (len+3)/4) && (((otg_core->Core.gnptxsts>>16) & 0xFF) > 0))
    {
        otg_core->Device.in_ep[epnum].dieptsiz = len | (1<<19);
        diepctl.d32 = in_ep->diepctl;
        in_ep->diepctl = ((diepctl.b.dpid==0)? (1<<28):(1<<29))|(diepctl.b.eptype<<18)|(1<<15)|diepctl.b.mps;
        otg_core->Device.in_ep[epnum].diepctl |= (1ul<<26)|(1ul<<31);  //Clear NAK,Endpoint Enable

        uint32 *ptr = (uint32*)buf;//Endpoint Enable
        for (i = 0; i < len; i += 4)
        {
            *fifo = *ptr++;
        }

        return (len);
    }
    else
    {
        if (j > 2000)
        {
            printf ("Write EP%d fail gnptxsts=0x%x\n",epnum,otg_core->Core.gnptxsts);
            return RK_ERROR;
        }
        j++;
        //printf ("Write EP%d fail:BUF[1]=%x BUF[2]=%x BUF[3]=%x j=%d\n",epnum, buf[1] ,buf[2] ,buf[3],j);
        DelayUs(1);
        goto LOOP;
    }
}

/*******************************************************************************
** Name: USBReadEp
** Input:uint32 epnum, uint32 len, uint8 * buf
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.9.25
** Time: 9:31:44
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON API rk_err_t USBReadEp(uint32 epnum, uint32 len, uint8 * buf)
{
    uint32 i;
    uint32 *fifo = (uint32 *)USB_EP_FIFO(epnum);

    uint32 *ptr = (uint32*)buf;

    for(i=0; i<len; i+=4)
    {
        *ptr++ = *fifo;
    }

    if(0 == epnum)
    {
        //clear nak

        DEVICE_REG *dev_regs = (DEVICE_REG *)USB_DEV_BASE;
        dev_regs->out_ep[0].doeptsiz = 0x40 | (1<<19) | (1<<29);
    }

    return len;
}


/*******************************************************************************
** Name: USBEnableOutEp
** Input:uint32 epnum, uint32 bmAttributes, uint32 wMaxPacketSize
** Return: void
** Owner:aaron.sun
** Date: 2014.9.25
** Time: 9:51:20
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON API void USBEnableOutEp(uint32 epnum, uint32 bmAttributes, uint32 wMaxPacketSize)
{
    DEVICE_REG *dev_regs = (DEVICE_REG *)USB_DEV_BASE;

    dev_regs->daintmsk |= ((1<<epnum)<<16);
    dev_regs->out_ep[epnum].doeptsiz = (1ul << 19) | wMaxPacketSize;
    dev_regs->out_ep[epnum].doepint = 0xFF;
    dev_regs->out_ep[epnum].doepctl = (1ul<<31)|(1<<28)|(1<<26)|((bmAttributes & 0xf7)<<18)|(1<<15)|wMaxPacketSize;  //回复原先参数，与SynchronisationType值无关
}


/*******************************************************************************
** Name: USBEnalbeEp
** Input:uint32 epnum, uint32 bmAttributes, uint32 wMaxPacketSize
** Return: void
** Owner:aaron.sun
** Date: 2014.9.25
** Time: 9:24:28
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON API void USBEnalbeInEp(uint32 epnum, uint32 EnType, uint32 wMaxPacketSize)
{
    DEVICE_REG *dev_regs = (DEVICE_REG *)USB_DEV_BASE;

    dev_regs->daintmsk |= (1<<epnum);
    dev_regs->in_ep[epnum].diepctl = (1<<28)|(1<<27)|(EnType<<18)|(1<<15)|wMaxPacketSize;
    //if (3==epnum)
    //    dev_regs->in_ep[epnum].diepctl |= (1<<22);  //这个配置,还要进一步debug
    dev_regs->dctl |= 1<<8;
}

/*******************************************************************************
** Name: USBEnableEp
** Input:uint8 uint8 epnum, uint32 bmAttributes, uint32 wMaxPacketSize
** Return: void
** Owner:wrm
** Date: 2015.5.27
** Time: 9:28:57
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON API void USBEnableEp (uint8 epnum, uint32 bmAttributes, uint32 wMaxPacketSize)
{
    DEVICE_REG *dev_regs = (DEVICE_REG *)USB_DEV_BASE;
    if (epnum & 0x80)
    {
        epnum = 0x7F & epnum;
        dev_regs->daintmsk |= (1<<epnum);
        dev_regs->in_ep[epnum].diepctl = (1<<28)|(1<<27)|(bmAttributes<<18)|(1<<15)|wMaxPacketSize;
        dev_regs->dctl |= 1<<8;
    }
    else
    {
        dev_regs->daintmsk |= ((1<<epnum)<<16);
        dev_regs->out_ep[epnum].doeptsiz = (1ul << 19) | wMaxPacketSize;
        dev_regs->out_ep[epnum].doepctl = (1ul<<31)|(1<<28)|(1<<26)|(bmAttributes<<18)|(1<<15)|wMaxPacketSize;
        dev_regs->out_ep[epnum].doepint = 0xFF;
    }
}

/*******************************************************************************
** Name: USBResetToggle
** Input:uint8 dir, uint8 epnum
** Return: void
** Owner:wrm
** Date: 2015.5.27
** Time: 9:28:57
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON API void USBResetToggle(uint8 dir, uint8 epnum)
{
    DEVICE_REG *dev_regs = (DEVICE_REG *)USB_DEV_BASE;

    if (dir)
    {
        dev_regs->in_ep[epnum].diepctl  |= 1<<28;
    }
    else
    {
        dev_regs->out_ep[epnum].doepctl |= 1<<28;
    }
}


/*******************************************************************************
** Name: USBSetEpSts
** Input:uint8 dir, uint8 epnum, uint8 status
** Return: void
** Owner:wrm
** Date: 2015.5.27
** Time: 9:28:57
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON API void USBSetEpSts(uint8 dir, uint8 epnum, uint8 status)
{
    DEVICE_REG *dev_regs = (DEVICE_REG *)USB_DEV_BASE;

    if (dir!=0)
    {
        if (status != 0)
        {
            volatile uint32 value = dev_regs->in_ep[epnum].diepctl;
            (value & (1ul<<31))? (value |= (1<<30 | 1<<21)) : (value |= (1<<21));
            dev_regs->in_ep[epnum].diepctl = value;    //In endpoint send stall handshack
        }
        else
            dev_regs->in_ep[epnum].diepctl &= ~(1<<21);
    }
    else
    {
        if (status != 0)
            dev_regs->out_ep[epnum].doepctl |= 1<<21;  //Out endpoint send stall handshack
        else
            dev_regs->out_ep[epnum].doepctl &= ~(1<<21);
    }
}
/*******************************************************************************
** Name: UsbSetIntMsk
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2014.9.24
** Time: 16:21:56
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON API uint32 UsbSetIntMsk(void)
{
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;
    otg_core->Core.gintmsk |= (1<<5);
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: UsbGetIntType
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2014.9.24
** Time: 16:21:56
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON API uint32 UsbGetIntType(void)
{
    uint32 intr_status;
    uint32 intr_type;
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;

    intr_status = otg_core->Core.gintsts & otg_core->Core.gintmsk;
    //printf ("UsbGetIntType intr_status=0x%x\n",intr_status);
    intr_type = 0;

    if (!intr_status)
    {
        return intr_type;
    }

    if (intr_status & (1<<4))   //RxFIFO Non-Empty
    {
        intr_type |= USB_INT_TYPE_RXFIFO_N_EMPTY;
    }

    if (intr_status & (1<<5))   //Non-periodic TxFIFO Empty
    {
        otg_core->Core.gintmsk &= ~(1<<5);
        intr_type |= USB_INT_TYPE_NP_TXFIFO_EMPTY;

        //otg_core->Device.dtknqr4_fifoemptymsk = 0;
    }

    if (intr_status & (1<<10))   //Early Suspend
    {
        intr_type |= USB_INT_TYPE_EARLY_SUSPEND;
        otg_core->Core.gintsts = 1<<10;
    }

    if (intr_status & (1<<11))   //USB Suspend
    {
        intr_type |= USB_INT_TYPE_SUSPEND;
        otg_core->Core.gintsts = 1<<11;
    }

    if (intr_status & (1<<12)) //USB Reset
    {
        intr_type |= USB_INT_TYPE_RESET;
        otg_core->Core.gintsts = 1<<12;
    }

    if (intr_status & (1<<13)) //Enumeration Done
    {
        intr_type |= USB_INT_TYPE_E_DONE;
        otg_core->Core.gintsts = 1<<13;
    }

    if (intr_status & (1<<18))      //IN中断
    {
        intr_type |= USB_INT_TYPE_EP_IN;
    }

    if (intr_status & (1<<19))      //OUT中断
    {
       intr_type |= USB_INT_TYPE_EP_OUT;
    }

    if (intr_status & (1<<30))      //USB VBUS中断  this interrupt is asserted when the utmiotg_bvalid signal goes high.
    {
        intr_type |= USB_INT_TYPE_VBUS;
        otg_core->Core.gintsts = 1<<30;
    }

    if (intr_status & (1ul<<31))   //resume
    {
        intr_type |= USB_INT_TYPE_RESUME;
        otg_core->Core.gintsts = 1ul<<31;
    }

    if (intr_status & ((1<<22)|(1<<6)|(1<<7)|(1<<17)))    //ingore the other interrupt
    {
        otg_core->Core.gintsts = intr_status & ((1<<22)|(1<<6)|(1<<7)|(1<<17));
    }

    return intr_type;

}

/*******************************************************************************
** Name:  UsbIntDisalbe
** Input:void
** Return: void
** Owner:zhuzhe
** Date: 2014.3.6
** Time: 15:39:04
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON API void  UsbIntDisalbe(void)
{
     *((volatile unsigned long*)(0xE000E180)) = 0x00000020;
}
/*******************************************************************************
** Name: UsbIntEnable
** Input:void
** Return: uint32
** Owner:zhuzhe
** Date: 2014.3.6
** Time: 15:37:45
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON API void UsbIntEnable(void)
{
    *((volatile unsigned long*)(0xE000E100)) = 0x00000020;
}


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(read) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: UsbControllerInit
** Input:bool FullSpeed
** Return: void
** Owner:aaron.sun
** Date: 2014.9.29
** Time: 15:24:17
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON FUN void UsbControllerInit(bool FullSpeed)
{
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;
    int count = 0;

    /* Wait for AHB master IDLE state. */
    for (count=0; count<10000; count++)
    {
        if ((otg_core->Core.grstctl & (1ul<<31))!=0)
            break;
    }

    //core soft reset
    otg_core->Core.grstctl |= 1<<0;               //Core soft reset
    for (count=0; count<10000; count++)
    {
        if ((otg_core->Core.grstctl & (1<<0))==0)
            break;
    }

    otg_core->ClkGate.pcgcctl = 0x0F;             /* Restart the Phy Clock */

    DelayMs(10);
    otg_core->ClkGate.pcgcctl = 0x00;             /* Restart the Phy Clock */


    //Non-Periodic TxFIFO Empty Level interrupt indicates that the IN Endpoint TxFIFO is completely empty
    otg_core->Core.gahbcfg |= 1<<7;

    /* Do device intialization*/

    //usb_write_reg32(USB_PCGCCTL, 0);

    /* High speed PHY running at high speed */
    if (!FullSpeed)
    {
        otg_core->Device.dcfg &= ~0x03;
    }
    else
    {
        //#ifdef FORCE_FS
        otg_core->Device.dcfg |= 0x01;   //Force FS
        //#endif
    }

    /* Reset Device Address */
    otg_core->Device.dcfg &= ~0x07F0;

    /* Flush the FIFOs */
    otg_core->Core.grstctl |= ( 0x10<<6) | (1<<5);     //Flush all Txfifo
    for (count=0; count<10000; count++)
    {
        if ((otg_core->Core.grstctl & (1<<5))==0)
            break;
    }

    otg_core->Core.grstctl |= 1<<4;              //Flush all Rxfifo
    for (count=0; count<10000; count++)
    {
        if ((otg_core->Core.grstctl & (1<<4))==0)
            break;
    }

    /* Flush the Learning Queue. */
    otg_core->Core.grstctl |= 1<<3;

    /* Clear all pending Device Interrupts */
    otg_core->Device.in_ep[0].diepctl = (1<<27)|(1<<30);        //IN0 SetNAK & endpoint disable
    otg_core->Device.in_ep[0].dieptsiz = 0;
    otg_core->Device.in_ep[0].diepint = 0xFF;

    otg_core->Device.out_ep[0].doepctl = (1<<27)|(1<<30);        //OUT0 SetNAK & endpoint disable
    otg_core->Device.out_ep[0].doeptsiz = 0;
    otg_core->Device.out_ep[0].doepint = 0xFF;

    otg_core->Device.diepmsk = 0x2F;              //device IN interrutp mask
    otg_core->Device.doepmsk = 0x0F;              //device OUT interrutp mask
    otg_core->Device.daint = 0xFFFFFFFF;          //clear all pending intrrupt
    otg_core->Device.daintmsk = 0x00010001;         //device all ep interrtup mask(IN0 & OUT0)
    otg_core->Core.gintsts = 0xFFFFFFFF;
    otg_core->Core.gotgint = 0xFFFFFFFF;

    //otg_core->Core.gintmsk=(1<<4)|/*(1<<5)|*/(1<<10)|(1<<11)|(1<<12)|(1<<13)|(1<<18)|(1<<19)|(1ul<<30)|(1ul<<31);
    otg_core->Core.gintmsk=(1<<4)|(1<<5)|(1<<10)|(1<<11)|(1<<12)|(1<<13)|(1<<18)|(1<<19)|(1ul<<30)|(1ul<<31);
    otg_core->Core.gahbcfg |= 0x01;        //Global interrupt mask
}

#ifdef USB_HOST
/*******************************************************************************
** Name: HostResetPort
** Input:HOST_DEV *pDev
** Return: int32
** Owner:wrm
** Date: 2014.9.24
** Time: 16:21:56
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON API int32 HostResetPort(HOST_DEV *pHDev)
{
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;
    volatile uint32 status;
    uint32 i, j;
    int32 ret=-HOST_ERR;
    uint8 change;
    uint8 state;

    //for(i=0; i<3; i++)
    for(i=0; i<1; i++)
    {
        //reset the port
        status = otg_core->Host.hprt&(~0x0f);
        status |= (0x01<<8);
        otg_core->Host.hprt = status;

        DelayMs(50);  //hi-speed 50ms,F/L speed 10ms
        DelayMs(10);  //datasheet 建议在加10ms清除

        status = otg_core->Host.hprt&(~0x0f);
        status &= ~(0x01<<8);
        otg_core->Host.hprt = status;

        for (j=0; j<50; j++)
        {
            if (otg_core->Host.hprt & 0x04) //wait for port enable
                break;
            DelayMs(1);
        }
        printf ("HostResetPort:j=%d\n",j);
        if (j>=50)
            return ret;

        ret = HOST_OK;
        status=(otg_core->Host.hprt >> 17) & 0x03;
        switch (status)
        {
            case 0:     //high speed
                printf ("HostResetPort: high speed\n");
                pHDev->speed = USB_SPEED_HS ;
                break;
            case 1:     //full speed
                printf ("HostResetPort: full speed\n");
                pHDev->speed = USB_SPEED_FS;
                break;
            case 2:     //low speed
                printf ("HostResetPort: low speed\n");
                pHDev->speed = USB_SPEED_LS;
                break;
            case 3:
            default:
                ret = -HOST_ERR;
                break;
        }
    }
    return ret;
}


/*******************************************************************************
** Name: Hostnptxfemp
** Input:void
** Return: void
** Owner:wrm
** Date: 2014.9.24
** Time: 16:21:56
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON API void Hostnptxfemp (int32 ret)
{
    GINTMSK_DATA gintmsk;
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;
    gintmsk.d32 = otg_core->Core.gintmsk;
    gintmsk.b.nptxfemp = (ret != 0);
    otg_core->Core.gintmsk = gintmsk.d32;
}

/*******************************************************************************
** Name: HostStartWaitXfer
** Input:USB_REQ *urb, int32 timeout
** Return: int32
** Owner:wrm
** Date: 2014.9.24
** Time: 16:21:56
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON API int32 HostStartWaitXfer(USB_REQ *urb, int32 timeout, UHC_CHN_INFO * pUsbHC)
{
    int32 i;
    UHC_CHN_INFO *hc = pUsbHC;

    timeout *= 1000;

    HostInitChn(hc, urb);

    HostProcessXfer(pUsbHC);
    for (i=0; i<timeout; i++)
    {
        if (urb->completed)
            break;

        DelayUs(1);
    }
    if (i >= timeout)
    {
        HostCompleteUrb(urb, -HOST_TIMEOUT);
    }
    HostFreeChn(hc);
    if(urb->status < 0)
    {
    }
    return (urb->status<0)? urb->status : urb->ActualLen;
}


/*******************************************************************************
** Name: UsbGetHostIntType
** Input:UHC_CHN_INFO * pUsbHC
** Return: void
** Owner:wrm
** Date: 2014.9.24
** Time: 16:21:56
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON API uint32 UsbGetHostIntType(UHC_CHN_INFO * pUsbHC)
{
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;
    uint32 intType = 0;
    GINTSTS_DATA gintsts;
    GINTMSK_DATA gintmsk;
    GNPTXSTS_DATA txsts;
    HPRT0_DATA hprt;

    gintsts.d32 = otg_core->Core.gintsts;
    hprt.d32 = otg_core->Host.hprt;
    /* Check if HOST Mode */
    if (DWC_HOST_MODE == gintsts.b.curmod)
    {
        gintsts.d32 &= otg_core->Core.gintmsk;
        if (!gintsts.d32)
        {
            printf ("Host Mode error exit\n");
            return 0;
        }
    }
    else
    {
        return 0;
    }


    if(gintsts.b.disconnint)
    {
        printf ("\nHost disconnint\n");
        USBHostConnect(0,pUsbHC);
        return HOST_DISCONNECT;
    }

    if(gintsts.b.prtint)
    {
        printf ("Host Port Int---------------------------------------------------------------\n");
        if (hprt.b.prtconnsts == 1)
        {
            printf ("Host Port Int 1-----------------------------------------------------------------\n");
            HostPortIsr();
            pUsbHC->ConnectStatus = 1;
        }
    }
    if (gintsts.b.rxflvl)
    {
        //printf ("RxFIFO Non-Empty\n");
        HostRxflvlIsr(pUsbHC);
    }

    if (gintsts.b.nptxfemp)
    {
        //printf ("Non-Periodic TxFIFO Empty\n");
        HostProcessXfer(pUsbHC);
    }

    if (gintsts.b.hchint)
    {
        //printf ("Host Channel Interrupt\n");
        int8 chn;
        uint32 haint = otg_core->Host.haint & otg_core->Host.haintmsk;

        for(chn=0; chn<MAX_HOST_CHN; chn++)
        {
            if(haint & (1<<chn))
            {
                //printf ("Judge Host Interrupt Type\n");
                HostChnIsr(chn,pUsbHC);
            }
        }
    }

    UsbIntEnable();
    otg_core->Core.gintsts = gintsts.d32;//write clear

    return intType;
}


/*******************************************************************************
** Name: USBSetMode
** Input:uint8 mode
** Return: void
** Owner:wrm
** Date: 2015.6.25
** Time: 9:28:57
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON API void USBSetMode(uint8 mode)
{
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;
    if (mode == FORCE_DEVICE_MODE)
    {
        printf ("FORCE_DEVICE_MODE\n");
        //otg_core->Core.gusbcfg &= 0x9fffffff;   //恢复正常模式
        //DelayMs(25);
        otg_core->Core.gusbcfg |= 0x40000000;
        DelayMs(25);
    }
    if (mode == FORCE_HOST_MODE)
    {
        //otg_core->Core.gusbcfg &= 0x9fffffff;
        //DelayMs(25);
        otg_core->Core.gusbcfg |= 0x20000000;
        DelayMs(30);
        printf ("\n*******************************\n");
        printf ("\nSet Host Mode gusbcfg=0x%x\n",otg_core->Core.gusbcfg);
        printf ("\nHost Mode gintsts=0x%x\n",otg_core->Core.gintsts);
        printf ("\n*******************************\n");
    }
    if (mode == NORMAL_MODE)
    {
        printf ("Normal\n");
        otg_core->Core.gusbcfg &= 0x9fffffff;
        DelayMs(25);
    }

}

/*******************************************************************************
** Name: USBHostConnect
** Input:bool FullSpeed, USBOTG_DEVICE_CLASS * pUsbOtgDev
** Return: void
** Owner:wrm
** Date: 2014.9.29
** Time: 15:33:44
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON API void USBHostConnect(bool FullSpeed, UHC_CHN_INFO * pUsbHC)
{
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;
    DEVICE_REG *dev_regs = (DEVICE_REG *)USB_DEV_BASE;
    int i;
    int count = 0;
    GINTMSK_DATA gintmsk;
    GAHBCFG_DATA ahbcfg;

#if 1
    UsbIntDisalbe();
    dev_regs->dctl |= 0x02;                   //soft disconnec
    printf ("\nHOST CON1 is0x%08x\n",Grf->GRF_UOC_CON1);
    printf ("\nHOST CON2 is0x%08x\n",Grf->GRF_UOC_CON2);

    Grf->GRF_UOC_CON2 |= 0x00060006;          //DP, DM 下拉15K电阻
    Grf->GRF_UOC_CON1 = (0x00030001 << 9);    //选择为host

    printf ("\nHOST CON1 is0x%08x\n",Grf->GRF_UOC_CON1);
    printf ("\nHOST CON2 is0x%08x\n",Grf->GRF_UOC_CON2);

    USBResetPhy();
    UsbHostControllerInit(FullSpeed);
    UsbIntEnable();
#endif
}

/*******************************************************************************
** Name: HostRegDeinit
** Input:void
** Return: void
** Owner:wrm
** Date: 2014.9.29
** Time: 15:33:44
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON API void HostRegDeinit(void)
{
    UsbIntDisalbe();
    printf ("\nHOST CON1 is0x%08x\n",Grf->GRF_UOC_CON1);
    printf ("\nHOST CON2 is0x%08x\n",Grf->GRF_UOC_CON2);
    Grf->GRF_UOC_CON1 = 0x02000000;    //选择为Dev
    Grf->GRF_UOC_CON2 = 0x00060000;   //DP, DM 下拉15K电阻使不能
    printf ("\nHOST CON1 is0x%08x\n",Grf->GRF_UOC_CON1);
    printf ("\nHOST CON2 is0x%08x\n",Grf->GRF_UOC_CON2);
    USBResetPhy();
    UsbIntEnable();
    return ;
}
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#if 1
/*******************************************************************************
** Name: UsbHostControllerInit
** Input:bool FullSpeed
** Return: void
** Owner:wrm
** Date: 2014.9.29
** Time: 15:24:17
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON FUN void UsbHostControllerInit(bool FullSpeed)
{
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;
    int count = 0;
    GINTMSK_DATA gintmsk;
    GAHBCFG_DATA ahbcfg;

    /* Wait for AHB master IDLE state. */
    for (count=0; count<10000; count++)
    {
        if ((otg_core->Core.grstctl & (1ul<<31))!=0)
            break;
        DelayUs(10);
    }

    otg_core->ClkGate.pcgcctl = 0x00;    //Restart the Phy Clock
    printf ("\nClkGate.pcgcctl Adress=0x%x Value=0x%x\n",
              &otg_core->ClkGate.pcgcctl,
              otg_core->ClkGate.pcgcctl);

    //Core soft reset
    otg_core->Core.grstctl |= 1<<0;
    for (count=0; count<10000; count++)
    {
        if ((otg_core->Core.grstctl & (1<<0))==0)
            break;
    }

    for (count=0; count<5000; count++)
    {
        if ((otg_core->Core.gintsts & 0x1)!=0)
            break;

        DelayMs(1);
    }

    otg_core->Host.hprt |= (0x01<<12);          //power on the port
    otg_core->Core.gintsts=0xffffffff;
    otg_core->Core.gotgint=0xffffffff;


    gintmsk.d32 = 0;
    gintmsk.b.disconnint = 1;
    gintmsk.b.hchint = 1;
    gintmsk.b.prtint = 1;
    gintmsk.b.rxflvl = 1;
    otg_core->Core.gintmsk = gintmsk.d32;

    ahbcfg.d32 = otg_core->Core.gahbcfg;
    ahbcfg.b.glblintrmsk = 1;
    ahbcfg.b.nptxfemplvl = DWC_GAHBCFG_TXFEMPTYLVL_EMPTY;
    otg_core->Core.gahbcfg = ahbcfg.d32;      //Global Interrupt Mask, operates in Slave mode

}


/*******************************************************************************
** Name: HostInitChnReg
** Input:UHC_CHN_INFO *hc
** Return: int32
** Owner:wrm
** Date: 2014.9.24
** Time: 16:21:56
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON FUN static void HostInitChnReg(UHC_CHN_INFO *hc)
{
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;
    HOST_CHANNEL_REG *pChnReg;
    HCCHAR_DATA hcchar;
    HCINTMSK_DATA hcintmaskn;
    HOST_REG *pHostReg = &otg_core->Host;

    pChnReg = &pHostReg->hchn[hc->chn];

    /* Clear old interrupt conditions for this host channel. */
    hcintmaskn.d32 = 0xFFFFFFFF;
    hcintmaskn.b.reserved = 0;
    pChnReg->hcintn = hcintmaskn.d32;

    /* Enable channel interrupts required for this transfer. */
    hcintmaskn.d32 = 0;
    hcintmaskn.b.chhltd = 1;

    switch (hc->EpType)
    {
        case DWC_OTG_EP_TYPE_CONTROL:
        case DWC_OTG_EP_TYPE_BULK:
            hcintmaskn.b.xfercompl = 1;
            hcintmaskn.b.stall = 1;
            hcintmaskn.b.xacterr = 1;
            hcintmaskn.b.datatglerr = 1;
            if (hc->EpIsIn)
            {
                hcintmaskn.b.bblerr = 1;
            }
            else
            {
                hcintmaskn.b.nak = 1;
                hcintmaskn.b.nyet = 1;
                if (hc->DoPing)
                {
                    hcintmaskn.b.ack = 1;
                }
            }
            if (hc->ErrorState)
            {
                hcintmaskn.b.ack = 1;
            }
            break;
        default:
            break;
    }
    //printf ("HostInitChnReg:pChnReg->hcintmaskn=0x%x\n",hcintmaskn.d32);

    pChnReg->hcintmaskn = hcintmaskn.d32;
    pHostReg->haintmsk |= (0x1ul<<hc->chn);
    /* Make sure host channel interrupts are enabled. */
    //gintmsk.b.hchint = 1;

    hcchar.d32 = 0;
    hcchar.b.devaddr = hc->DevAddr;
    hcchar.b.epnum = hc->EpNum;
    hcchar.b.epdir = hc->EpIsIn;
    hcchar.b.lspddev = (hc->speed == DWC_OTG_EP_SPEED_LOW); //HIGH SPEED
    hcchar.b.eptype = hc->EpType;
    hcchar.b.mps = hc->MaxPkt;

    pChnReg->hccharn = hcchar.d32;
    //printf ("HostInitChnReg:pChnReg->hccharn=0x%x\n",pChnReg->hccharn);

}

/*******************************************************************************
** Name: HostInitChn
** Input:UHC_CHN_INFO * pUsbHC
** Return: int32
** Owner:wrm
** Date: 2014.9.24
** Time: 16:21:56
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON FUN void HostInitChn(UHC_CHN_INFO *hc, USB_REQ *urb)
{
    HOST_PIPE pipe = urb->pipe;

    hc->DevAddr = pipe.b.devaddr;
    hc->EpNum = pipe.b.epnum;
    hc->MaxPkt = pipe.b.mps;
    hc->EpType = pipe.b.eptype;
    hc->EpIsIn = pipe.b.epdir;

    hc->speed = urb->pDev->speed;

    hc->XferStarted = 0;
    hc->status = HC_XFER_IDLE;
    hc->ErrorState = (urb->ErrCnt > 0);
    hc->HaltPending = 0;
    hc->HaltOnQueue = 0;
    hc->valid = urb->valid;

    hc->DoPing = urb->PingState;
    hc->PidStart = (urb->DataToggle)? DWC_OTG_HC_PID_DATA1 : DWC_OTG_HC_PID_DATA0;

    hc->XferBuf = (uint8*)((uint32)urb->TransBuf + urb->ActualLen);
    hc->XferLen = urb->BufLen - urb->ActualLen;
    hc->XferCnt = 0;
#if 0
    printf ("*****************************\n");
    printf ("\nHostInitChn:hc->DevAddr=%d\n",hc->DevAddr);
    printf ("\nHostInitChn:hc->EpNum=%d\n",hc->EpNum);
    printf ("\nHostInitChn:hc->MaxPkt=%d\n",hc->MaxPkt);
    printf ("\nHostInitChn:hc->EpType=%d\n",hc->EpType);
    printf ("\nHostInitChn:hc->EpIsIn=%d\n",hc->EpIsIn);
    printf ("\nHostInitChn:hc->speed=%d\n",hc->speed);
    printf ("\nHostInitChn:hc->ErrorState=%d\n",hc->ErrorState);
    printf ("\nHostInitChn:hc->DoPing=%d\n",hc->DoPing);
    printf ("\nHostInitChn:hc->PidStart=%d\n",hc->PidStart);
    printf ("\nHostInitChn:urb->BufLen=%d urb->ActualLen=%d\n",
            urb->BufLen,
            urb->ActualLen);
    printf ("\nHostInitChn:hc->XferLen=%d\n",hc->XferLen);
    printf ("\nHostInitChn:hc->valid=%d\n",hc->valid);

    printf ("\nHostInitChn:urb->CtrlPhase=%d\n",urb->CtrlPhase);

    printf ("*****************************\n");
#endif
    switch (hc->EpType)
    {
        case DWC_OTG_EP_TYPE_CONTROL:
            switch (urb->CtrlPhase)
            {
                case DWC_OTG_CONTROL_SETUP:
                    hc->DoPing = 0;
                    hc->EpIsIn = 0;
                    hc->PidStart = DWC_OTG_HC_PID_SETUP;
                    hc->XferBuf = (uint8*)urb->SetupPkt;
                    hc->XferLen = 8;
                    break;

                 case DWC_OTG_CONTROL_DATA:
                    hc->PidStart = (urb->DataToggle)? DWC_OTG_HC_PID_DATA1 : DWC_OTG_HC_PID_DATA0;
                    break;

                 case DWC_OTG_CONTROL_STATUS:
                    /*
                    * Direction is opposite of data direction or IN if no
                    * data.
                    */
                    if (urb->BufLen == 0)
                        hc->EpIsIn = 1;
                    else
                        hc->EpIsIn = (pipe.b.epdir != 1);

                    if (hc->EpIsIn)
                        hc->DoPing = 0;

                    hc->PidStart = DWC_OTG_HC_PID_DATA1;
                    hc->XferLen = 0;
                    hc->XferBuf = (uint8*)urb->StatusBuf;
                    break;
                default:
                    break;
            }
            break;

        case DWC_OTG_EP_TYPE_BULK:
            break;
        default:
            break;
    }

    hc->urb = urb;
    HostInitChnReg(hc);
}


/*******************************************************************************
** Name: HostAllocChn
** Input:UHC_CHN_INFO * pUsbHC
** Return: int32
** Owner:wrm
** Date: 2014.9.24
** Time: 16:21:56
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON FUN static UHC_CHN_INFO * HostAllocChn(UHC_CHN_INFO * pUsbHC)
{
    uint32 i;
    UHC_CHN_INFO* hc = pUsbHC;

    for(i=0; i<MAX_HOST_CHN; i++, hc++)
    {
        if (0 == hc->valid)
        {
            memset((void*)hc, 0, sizeof(UHC_CHN_INFO));
            hc->valid = 1;
            hc->chn = i;
            return hc;
        }
    }
    return NULL;
}
/*******************************************************************************
** Name: HostCompleteUrb
** Input:USB_REQ *urb, int32 status
** Return: int32
** Owner:wrm
** Date: 2014.9.24
** Time: 16:21:56
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON FUN void HostCompleteUrb(USB_REQ *urb, int32 status)
{
    urb->status = status;
    urb->completed = 1;
}
/*******************************************************************************
** Name: HostFreeChn
** Input:UHC_CHN_INFO * hc
** Return: int32
** Owner:wrm
** Date: 2014.9.24
** Time: 16:21:56
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON FUN static void HostFreeChn(UHC_CHN_INFO * hc)
{
    if (hc->valid)
    {
        uint32 i;
        if(hc->XferStarted)
        {
            if (hc->HaltPending || hc->HaltOnQueue)
                DelayUs(1);
            else
                HostHaltChn(hc, HC_XFER_TIMEOUT);
        }

        for (i=0; i<1000; i++)
        {
            if (!hc->XferStarted)
                break;

            DelayUs(1);
        }

        hc->valid = 0;
    }
}

/*******************************************************************************
** Name: HostPortStatus
** Input:uint8 *status, uint8 *change
** Return: int32
** Owner:wrm
** Date: 2014.9.24
** Time: 16:21:56
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON FUN int32 HostPortStatus(uint8 *status, uint8 *change)
{
#if 0
    //DISABLE_USB_INT;
    if (change)
    {
        *change = gHostCtrl.PortChange;
        gHostCtrl.PortChange = 0;
    }

    if (status)
        *status = gHostCtrl.PortStatus;
    //ENABLE_USB_INT;
#endif
    return HOST_OK;
}

/*******************************************************************************
** Name: HostHaltChn
** Input:UHC_CHN_INFO * pUsbHC
** Return: int32
** Owner:wrm
** Date: 2014.9.24
** Time: 16:21:56
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON FUN int32 HostHaltChn(UHC_CHN_INFO* hc, uint32 status)
{
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;
    int32 ret = 1;
    HOST_CHANNEL_REG *pChnReg  = &otg_core->Host.hchn[hc->chn];
    HCCHAR_DATA hcchar;
    GNPTXSTS_DATA txsts;

    if (hc->HaltPending)
    {
        return ret;
    }

    hc->status = status;
    hcchar.d32 = pChnReg->hccharn;
    if (hcchar.b.chen == 0)
    {
        ret = 0;
        return ret;
    }

    txsts.d32 = otg_core->Core.gnptxsts;
    hcchar.b.chen = (txsts.b.nptxqspcavail)? 1 : 0;

    hcchar.b.chdis = 1;
    pChnReg->hccharn = hcchar.d32;

    if (hcchar.b.chen)
    {
        hc->HaltPending = 1;
        hc->HaltOnQueue = 0;
    }
    else
    {
        hc->HaltOnQueue = 1;
    }

    if (hc->HaltOnQueue)
    {
        GINTMSK_DATA gintmsk;
        gintmsk.d32 = otg_core->Core.gintmsk;
        gintmsk.b.nptxfemp = 1;
        otg_core->Core.gintmsk = gintmsk.d32;
    }

    return ret;
}
/*******************************************************************************
** Name: HostTxData
** Input:nt8 chn, uint32 *pBuf, uint32 len
** Return: int32
** Owner:wrm
** Date: 2014.9.24
** Time: 16:21:56
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON FUN static void HostTxData(int8 chn, uint32 *pBuf, uint32 len)
{
    uint32 * FIFO = (uint32 *)HC_CHN_FIFO(chn);
    //printf ("TX:chn=%d len=%d\n",chn,len);
    //printf ("TX:pBuf[0]=0x%x\n",pBuf[0]);
    //printf ("TX:pBuf[1]=0x%x\n",pBuf[1]);
    //printf ("TX:FIFO Adress=0x%x\n",FIFO);

    while(len--)
    {
        *FIFO = *pBuf++;
    }
    //printf ("TX Data Complete\n");
}


/*******************************************************************************
** Name: HostTxPkt
** Input:UHC_CHN_INFO * hc
** Return: int32
** Owner:wrm
** Date: 2014.9.24
** Time: 16:21:56
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON FUN static void HostTxPkt(UHC_CHN_INFO *hc)
{
    uint32 remain;
    uint32 byte;
    uint32 dword;

    remain = hc->XferLen - hc->XferCnt;
    byte =  (remain > hc->MaxPkt) ? hc->MaxPkt : remain;

    dword = (byte+3)>>2;

    /* xferbuf must be DWORD aligned. */
    HostTxData(hc->chn, (uint32 *)hc->XferBuf, dword);

    hc->XferCnt += byte;
    hc->XferBuf += byte;
}

/*******************************************************************************
** Name: HostDoPing
** Input:UHC_CHN_INFO * hc
** Return: int32
** Owner:wrm
** Date: 2014.9.24
** Time: 16:21:56
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON FUN static void HostDoPing(UHC_CHN_INFO* hc)
{
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;
    HCTSIZ_DATA hctsiz;
    HCCHAR_DATA hcchar;
    HOST_CHANNEL_REG *pChnReg  = &otg_core->Host.hchn[hc->chn];

    hctsiz.d32 = 0;
    hctsiz.b.dopng = 1;
    hctsiz.b.pktcnt = 1;
    pChnReg->hctsizn =  hctsiz.d32;

    hcchar.d32 = pChnReg->hccharn;
    hcchar.b.chen = 1;
    hcchar.b.chdis = 0;

    pChnReg->hccharn = hcchar.d32;
}
/*******************************************************************************
** Name: HostStartXfer
** Input:UHC_CHN_INFO * hc
** Return: int32
** Owner:wrm
** Date: 2014.9.24
** Time: 16:21:56
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON FUN static void HostStartXfer(UHC_CHN_INFO *hc)
{
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;
    HCCHAR_DATA hcchar;
    HCTSIZ_DATA hctsiz;
    uint16  NumPkt;
    HOST_CHANNEL_REG *pChnReg  = &otg_core->Host.hchn[hc->chn];

    hctsiz.d32 = 0;
    if (hc->DoPing)
    {
        HostDoPing(hc);
        hc->XferStarted = 1;
        return;
    }
    if (hc->XferLen > 0)
    {

        NumPkt = (hc->XferLen + hc->MaxPkt- 1) / hc->MaxPkt;
    }
    else
    {
        /* Need 1 packet for transfer length of 0. */
        NumPkt = 1;
    }

    if (hc->EpIsIn)
    {
        /* Always program an integral # of max packets for IN transfers. */
        hc->XferLen = NumPkt * hc->MaxPkt;
    }

    hctsiz.b.xfersize = hc->XferLen;

    hc->StartPktCnt = NumPkt;
    hctsiz.b.pktcnt = NumPkt;;
    hctsiz.b.pid = hc->PidStart;

    pChnReg->hctsizn = hctsiz.d32;

    hcchar.d32 = pChnReg->hccharn;
    /* Set host channel enable after all other setup is complete. */
    hcchar.b.chen = 1;
    hcchar.b.chdis = 0;
    pChnReg->hccharn = hcchar.d32;

    hc->XferStarted = 1;

    if (!hc->EpIsIn && hc->XferLen > 0)
    {
        /* Load OUT packet into the appropriate Tx FIFO. */
        HostTxPkt(hc);
    }
}
/*******************************************************************************
** Name: HostContinueXfer
** Input:UHC_CHN_INFO * hc
** Return: int32
** Owner:wrm
** Date: 2014.9.24
** Time: 16:21:56
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON FUN static int32 HostContinueXfer(UHC_CHN_INFO *hc)
{
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;
    if (hc->PidStart == DWC_OTG_HC_PID_SETUP)
    {
        /* SETUPs are queued only once since they can't be NAKed. */
        return 0;
    }
    else if (hc->EpIsIn)
    {
        HCCHAR_DATA hcchar;
        HOST_CHANNEL_REG *pChnReg  = &otg_core->Host.hchn[hc->chn];

        hcchar.d32 = pChnReg->hccharn;
        hcchar.b.chen = 1;
        hcchar.b.chdis = 0;
        pChnReg->hccharn = hcchar.d32;

        return 1;
    }
    else
    {
        /* OUT transfers. */
        if (hc->XferCnt < hc->XferLen)
        {
            HostTxPkt(hc);
            return 1;
        }
        else
        {
            return 0;
        }
    }
}

/*******************************************************************************
** Name: HostProcessXfer
** Input:UHC_CHN_INFO * pUsbHC
** Return: int32
** Owner:wrm
** Date: 2014.9.24
** Time: 16:21:56
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON FUN int32 HostProcessXfer(UHC_CHN_INFO * pUsbHC)
{
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;
    int32 i = 0;
    int32 ret = 0;
    GINTMSK_DATA gintmsk;
    GNPTXSTS_DATA txsts;
    UHC_CHN_INFO* hc = pUsbHC;
#if 0
    printf ("hc->valid=%d hc->urb=0x%x hc->urb->completed=%d\n",
            hc->valid,
            hc->urb,
            hc->urb->completed);
#endif
    for(i=0; i<MAX_HOST_CHN; i++, hc++)
    {
        if (hc->valid && hc->urb && !hc->urb->completed)
        {
            break;
        }
    }

    if (i >= MAX_HOST_CHN)
        {
         goto OUT;
        }

    txsts.d32 = otg_core->Core.gnptxsts;
    if (txsts.b.nptxqspcavail == 0)
    {
        ret = 1;
    }
    else if (hc->HaltPending)
    {
        ret = 0;
    }
    else if (hc->HaltOnQueue)
    {
        HostHaltChn(hc, hc->status);
        ret = 0;
    }
    else if (hc->DoPing)
    {
        if (!hc->XferStarted)
        {
            HostStartXfer(hc);
        }
        ret = 0;
    }
    else if (!hc->EpIsIn || hc->PidStart == DWC_OTG_HC_PID_SETUP)
    {
        if (txsts.b.nptxfspcavail*4 >= hc->MaxPkt)
        {
            if (!hc->XferStarted)
            {
                HostStartXfer(hc);
                ret = 1;
            }
            else
            {
                ret = HostContinueXfer(hc);
            }
        }
        else
        {
            ret = -1;
        }
    }
    else
    {

        if (!hc->XferStarted)
        {
            HostStartXfer(hc);
            ret = 1;
        }
        else
        {
            ret = HostContinueXfer(hc);
        }
    }
OUT:
    /*
    * Ensure NP Tx FIFO empty interrupt is disabled when
    * there are no non-periodic transfers to process.
    */
    //printf ("wrm_5_3 ret=%d\n",ret);
    gintmsk.d32 = otg_core->Core.gintmsk;
    gintmsk.b.nptxfemp = (ret != 0);
    otg_core->Core.gintmsk = gintmsk.d32;
    return ret;
}

/*******************************************************************************
** Name: HostRxData
** Input:int8 chn, uint32 *pBuf, uint32 len
** Return: void
** Owner:wrm
** Date: 2014.9.24
** Time: 16:21:56
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON FUN static void HostRxData(int8 chn, uint32 *pBuf, uint32 len)
{
    uint32 * FIFO = (uint32 *)HC_CHN_FIFO(chn);
    int i = 0;

    while(len--)
    {
        *pBuf++ = *FIFO;
    }
}


/*******************************************************************************
** Name: HostRxPkt
** Input:UHC_CHN_INFO *hc, uint32 bytes
** Return: void
** Owner:wrm
** Date: 2014.9.24
** Time: 16:21:56
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON FUN void HostRxPkt(UHC_CHN_INFO *hc, uint32 bytes)
{
    uint32 dword = (bytes + 3) >>2;

    /* xferbuf must be DWORD aligned. */
    HostRxData(hc->chn, (uint32 *)hc->XferBuf, dword);

    hc->XferCnt += bytes;
    hc->XferBuf += bytes;

}
/*******************************************************************************
** Name: HostGetChn
** Input:int8 chn, UHC_CHN_INFO * pUsbHC
** Return:UHC_CHN_INFO*
** Owner:wrm
** Date: 2014.9.24
** Time: 16:21:56
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON FUN UHC_CHN_INFO* HostGetChn(int8 chn, UHC_CHN_INFO * pUsbHC)
{
    if (chn >= MAX_HOST_CHN)
        return NULL;

    return pUsbHC;
}

/*******************************************************************************
** Name: HostRxflvlIsr
** Input:UHC_CHN_INFO * pUsbHC
** Return: void
** Owner:wrm
** Date: 2014.9.24
** Time: 16:21:56
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON FUN static void HostRxflvlIsr(UHC_CHN_INFO * pUsbHC)
{
    int8 chn;
    GRXSTSH_DATA grxsts;
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;
#if 1
    otg_core->Core.gintmsk |= (1<<4);
#endif
    grxsts.d32 = otg_core->Core.grxstsp;

    chn = grxsts.b.chnum;

    switch (grxsts.b.pktsts)
    {
        case DWC_GRXSTS_PKTSTS_IN:
        /* Read the data into the host buffer.    */
        if (grxsts.b.bcnt > 0)
        {
            HostRxPkt(HostGetChn(chn, pUsbHC), grxsts.b.bcnt);
        }

        case DWC_GRXSTS_PKTSTS_IN_XFER_COMP:
        case DWC_GRXSTS_PKTSTS_DATA_TOGGLE_ERR:
        case DWC_GRXSTS_PKTSTS_CH_HALTED:
        /* Handled in interrupt, just ignore data */
            break;
        default:
            otg_core->Core.gintmsk &= ~(1<<4);
            break;
    }
}


/*******************************************************************************
** Name: HostPortIsr
** Input:void
** Return: void
** Owner:wrm
** Date: 2014.9.24
** Time: 16:21:56
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON FUN static void HostPortIsr(void)
{
    HPRT0_DATA hprt;
    HPRT0_DATA clean_hprt;
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;

    hprt.d32 = otg_core->Host.hprt;
    clean_hprt.d32 = hprt.d32;

    /* Clear appropriate bits in HPRT0 to clear the interrupt bit in
     * GINTSTS */

    clean_hprt.b.prtena = 0;
    clean_hprt.b.prtconndet = 0;
    clean_hprt.b.prtenchng = 0;
    clean_hprt.b.prtovrcurrchng = 0;
    /* Port Connect Detected
     * Set flag and clear if detected */
    if(hprt.b.prtconndet)
    {
        clean_hprt.b.prtconndet = 1;
    }
    if (hprt.b.prtenchng)
    {
        clean_hprt.b.prtenchng = 1;
    }
    if(hprt.b.prtovrcurrchng)
    {
        /** Overcurrent Change Interrupt */
        clean_hprt.b.prtovrcurrchng = 1;
        //disable the port and power off
        clean_hprt.b.prtpwr = 0;
    }

    /* Clear Port Interrupts */
    otg_core->Host.hprt = clean_hprt.d32;
}


/*******************************************************************************
** Name: USBResetPhy
** Input:void
** Return: void
** Owner:wrm
** Date: 2014.9.24
** Time: 16:21:56
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON FUN void USBResetPhy(void)
{
    ScuSoftResetCtr(USBGLB_SRST, 1);
    ScuSoftResetCtr(USBOTG_SRST, 1);
    DelayUs(20);
    ScuSoftResetCtr(USBGLB_SRST, 0);
    ScuSoftResetCtr(USBOTG_SRST, 0);

    ScuSoftResetCtr(USBPHY_SRST, 1);  //usbphy_sft_rst
    DelayMs(10);
    ScuSoftResetCtr(USBPHY_SRST, 0);
    DelayMs(1);
}


/*******************************************************************************
** Name: HostUpdateXfer
** Input:UHC_CHN_INFO* hc, uint32 ChnStatus, int32 *ShortRead
** Return: void
** Owner:wrm
** Date: 2014.9.24
** Time: 16:21:56
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON FUN static int32 HostGetActualLen(UHC_CHN_INFO* hc, uint32 ChnStatus, int32 *ShortRead)
{
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;
    HCTSIZ_DATA hctsiz;
    uint32     length;
    HOST_CHANNEL_REG *pChnReg  = &otg_core->Host.hchn[hc->chn];

    hctsiz.d32 = pChnReg->hctsizn;

    if (ChnStatus == HC_XFER_COMPL)
    {
        if (hc->EpIsIn) //ep_is_in
        {
            length = hc->XferLen - hctsiz.b.xfersize;
            if (ShortRead != NULL)
            {
                *ShortRead = (hctsiz.b.xfersize != 0);
            }
        }
        else
        {
            length = hc->XferLen;
        }
    }
    else
    {
        /*
         * Must use the hctsiz.pktcnt field to determine how much data
         * has been transferred. This field reflects the number of
         * packets that have been transferred via the USB. This is
         * always an integral number of packets if the transfer was
         * halted before its normal completion. (Can't use the
         * hctsiz.xfersize field because that reflects the number of
         * bytes transferred via the AHB, not the USB).
         */
        length = (hc->StartPktCnt - hctsiz.b.pktcnt) * hc->MaxPkt;
    }

    return length;

}


/*******************************************************************************
** Name: HostUpdateXfer
** Input:UHC_CHN_INFO* hc, USB_REQ *urb, uint32 ChnStatus
** Return: void
** Owner:wrm
** Date: 2014.9.24
** Time: 16:21:56
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON FUN int32 HostUpdateXfer(UHC_CHN_INFO* hc, USB_REQ *urb, uint32 ChnStatus)
{
    //HCHN_REG *pChnReg  = &OTGReg->Host.hchn[pChn->chn];
    int32 XferDone = 0;

    if (HC_XFER_COMPL == ChnStatus)
    {
        int32 ShortRead = 0;

        urb->ActualLen += HostGetActualLen(hc, ChnStatus, &ShortRead);
        if (ShortRead || (urb->ActualLen >= urb->BufLen))
        {
           XferDone = 1;
        }
    }
    else
    {
        urb->ActualLen += HostGetActualLen(hc, ChnStatus, NULL);
    }

    return XferDone;
}
/*******************************************************************************
** Name: HostReleaseChn
** Input:UHC_CHN_INFO* hc
** Return: void
** Owner:wrm
** Date: 2014.9.24
** Time: 16:21:56
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON FUN void HostReleaseChn(UHC_CHN_INFO * hc)
{
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;
    HOST_REG *pHostReg = &otg_core->Host;
    HOST_CHANNEL_REG *pChnReg  = &otg_core->Host.hchn[hc->chn];
    USB_REQ  *urb = hc->urb;

    if (HC_XFER_XERR==hc->status)
    {
        if (urb->ErrCnt >= 3)
        {
            HostCompleteUrb(urb, -HOST_XERR);
        }
    }

    hc->HaltPending = 0;
    hc->XferStarted = 0;
    pChnReg->hcintmaskn = 0;
    pChnReg->hcintn = 0xFFFFFFFF;
    pHostReg->haintmsk &= ~(0x1ul<<hc->chn);

    if (!urb->completed)
    {
        HostInitChn(hc, hc->urb);
        HostProcessXfer(hc);
    }
}


/*******************************************************************************
** Name: HostCompleteXfer
** Input:UHC_CHN_INFO* hc, HOST_CHANNEL_REG *pChnReg, uint32 status
** Return: void
** Owner:wrm
** Date: 2014.9.24
** Time: 16:21:56
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON FUN void HostCompleteXfer(UHC_CHN_INFO* hc, HOST_CHANNEL_REG *pChnReg, uint32 status)
{
    HCINT_DATA hcint;
    int32 ret;

    hcint.d32 = pChnReg->hcintn;
    if (hcint.b.nyet)
    {
        //printf ("HostCompleteXfer_1\n");
        /*
         * Got a NYET on the last transaction of the transfer. This
         * means that the endpoint should be in the PING state at the
         * beginning of the next transfer.
         */
        hc->urb->PingState = 1;
        CLEAN_HC_INT(pChnReg, nyet);
    }

    if (hc->EpIsIn) //ep_is_in
     {
        /*
         * IN transfers in Slave mode require an explicit disable to
         * halt the channel. (In DMA mode, this call simply releases
         * the channel.)
         */
        ret = HostHaltChn(hc, status);
        //printf ("HostCompleteXfer_2 ret=%d\n",ret);
        if (ret == 0)
            HostReleaseChn(hc);
    }
    else
    {
        //printf ("HostCompleteXfer_3\n");
        /*
         * The channel is automatically disabled by the core for OUT
         * transfers in Slave mode.
         */
        hc->status = status;
        HostReleaseChn(hc);
    }
}
/*******************************************************************************
** Name: HostSaveToggle
** Input:void
** Return: void
** Owner:wrm
** Date: 2014.9.24
** Time: 16:21:56
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON FUN void HostSaveToggle(UHC_CHN_INFO* hc, USB_REQ *urb)
{
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;
    HCTSIZ_DATA hctsiz;
    HOST_CHANNEL_REG *pChnReg  = &otg_core->Host.hchn[hc->chn];

    hctsiz.d32 = pChnReg->hctsizn;
    //printf ("HostSaveToggle Data chn=%d\n",hc->chn);
    //printf ("HostSaveToggle Data pid=%d\n",hctsiz.b.pid);
    if (hctsiz.b.pid == DWC_HCTSIZ_DATA0)
    {
        urb->DataToggle = 0;
    }
    else//(hctsiz.b.pid == DWC_HCTSIZ_DATA1)
    {
        urb->DataToggle = 1;
    }
}

/*******************************************************************************
** Name: HostXfercompIsr
** Input:void
** Return: void
** Owner:wrm
** Date: 2014.9.24
** Time: 16:21:56
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON FUN static void HostXfercompIsr(UHC_CHN_INFO* hc)
{
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;
    HOST_CHANNEL_REG *pChnReg = &otg_core->Host.hchn[hc->chn];
    USB_REQ   *urb    = hc->urb;
    uint32 status = HC_XFER_COMPL;

    DISABLE_HC_INT(pChnReg, xfercompl);
    //printf ("HostXfercompIs EpType=%d\n",hc->EpType);
    switch (hc->EpType) /** 0: Control, 1: Isoc, 2: Bulk, 3: Intr */
    {
        case DWC_OTG_EP_TYPE_CONTROL:
            //printf ("HostXfercompIs CtrlPhase=%d\n",urb->CtrlPhase);
            switch (urb->CtrlPhase)
            {
                case DWC_OTG_CONTROL_SETUP:
                    if (urb->BufLen > 0)
                        urb->CtrlPhase = DWC_OTG_CONTROL_DATA;
                    else
                        urb->CtrlPhase = DWC_OTG_CONTROL_STATUS;;
                    break;
                case DWC_OTG_CONTROL_DATA:
                    HostUpdateXfer(hc, urb, HC_XFER_COMPL);
                    urb->CtrlPhase = DWC_OTG_CONTROL_STATUS;
                    break;
                case DWC_OTG_CONTROL_STATUS:
                    HostCompleteUrb(urb, 0);
                    status = HC_XFER_URB_COMPL;
                    break;
                default:
                    break;
            }

            HostCompleteXfer(hc, pChnReg, status);
            break;
        case DWC_OTG_EP_TYPE_BULK:
            HostUpdateXfer(hc, urb, HC_XFER_COMPL);
            HostSaveToggle(hc, urb);
            HostCompleteUrb(urb, 0);
            status = HC_XFER_URB_COMPL;
            HostCompleteXfer(hc, pChnReg, status);
            break;

        default:
            break;
    }
}

/*******************************************************************************
** Name: HostChhltdIsr
** Input:UHC_CHN_INFO * pUsbHC
** Return: void
** Owner:wrm
** Date: 2014.9.24
** Time: 16:21:56
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON FUN static void HostChhltdIsr(UHC_CHN_INFO* hc)
{
    HostReleaseChn(hc);

}
/*******************************************************************************
** Name: HostStallIsr
** Input:UHC_CHN_INFO * pUsbHC
** Return: void
** Owner:wrm
** Date: 2014.9.24
** Time: 16:21:56
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON FUN static void HostStallIsr(UHC_CHN_INFO* hc)
{
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;
    HOST_CHANNEL_REG *pChnReg  = &otg_core->Host.hchn[hc->chn];

    HostCompleteUrb(hc->urb, -HOST_STALL);

    if (hc->EpType == DWC_OTG_EP_TYPE_BULK)
    {
        /*
        * USB protocol requires resetting the data toggle for bulk
        * and interrupt endpoints when a CLEAR_FEATURE(ENDPOINT_HALT)
        * setup command is issued to the endpoint. Anticipate the
        * CLEAR_FEATURE command since a STALL has occurred and reset
        * the data toggle now.
        */
        hc->urb->DataToggle = 0;
    }

    HostHaltChn(hc, HC_XFER_STALL);                //Disable Channel

    DISABLE_HC_INT(pChnReg, stall);
}

/*******************************************************************************
** Name: HostNakIsr
** Input:UHC_CHN_INFO * hc
** Return: void
** Owner:wrm
** Date: 2014.9.24
** Time: 16:21:56
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON FUN static void HostNakIsr(UHC_CHN_INFO* hc)
{
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;
    HOST_CHANNEL_REG *pChnReg  = &otg_core->Host.hchn[hc->chn];
    USB_REQ     *urb = hc->urb;

    switch (hc->EpType) /** 0: Control, 1: Isoc, 2: Bulk, 3: Intr */
    {
        case DWC_OTG_EP_TYPE_CONTROL:
        case DWC_OTG_EP_TYPE_BULK:
            //Reset Error Count
            urb->ErrCnt = 0;

            if (!urb->PingState)
            {
                HostUpdateXfer(hc, urb, HC_XFER_NAK);
                HostSaveToggle(hc, urb);

                if (USB_SPEED_HS==hc->speed && !hc->EpIsIn)
                {
                    urb->PingState = 1;
                }
            }

            HostHaltChn(hc, HC_XFER_NAK);
            break;
        default:
            break;
    }

    CLEAN_HC_INT(pChnReg, nak);
    DISABLE_HC_INT(pChnReg, nak);
}

/*******************************************************************************
** Name: HostAckIsr
** Input:UHC_CHN_INFO * hc
** Return: void
** Owner:wrm
** Date: 2014.9.24
** Time: 16:21:56
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON FUN static void HostAckIsr(UHC_CHN_INFO* hc)
{
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;
    HOST_CHANNEL_REG *pChnReg  = &otg_core->Host.hchn[hc->chn];
    USB_REQ     *urb = hc->urb;

    //Reset Error Count
    urb->ErrCnt = 0;

    if (urb->PingState)
    {
        urb->PingState = 0;
        HostHaltChn(hc, HC_XFER_ACK);
    }

    DISABLE_HC_INT(pChnReg, ack);
}
/*******************************************************************************
** Name: HostNyetIsr
** Input:UHC_CHN_INFO * hc
** Return: void
** Owner:wrm
** Date: 2014.9.24
** Time: 16:21:56
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON FUN static void HostNyetIsr(UHC_CHN_INFO* hc)
{
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;
    HOST_CHANNEL_REG *pChnReg  = &otg_core->Host.hchn[hc->chn];
    USB_REQ     *urb = hc->urb;

    printf("NYET\n");
    //Reset Error Count
    urb->ErrCnt = 0;
    urb->PingState = 1;

    HostUpdateXfer(hc, urb, HC_XFER_NYET);
    HostSaveToggle(hc, urb);

    /*
    * Halt the channel and re-start the transfer so the PING
    * protocol will start.
    */
    HostHaltChn(hc, HC_XFER_NYET);

    DISABLE_HC_INT(pChnReg, nyet);
}

/*******************************************************************************
** Name: HostXacterrIsr
** Input:UHC_CHN_INFO * hc
** Return: void
** Owner:wrm
** Date: 2014.9.24
** Time: 16:21:56
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON FUN static void HostXacterrIsr(UHC_CHN_INFO* hc)
{
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;
    HOST_CHANNEL_REG *pChnReg  = &otg_core->Host.hchn[hc->chn];
    USB_REQ     *urb = hc->urb;

    //Reset Error Count
    urb->ErrCnt++;
    if (!urb->PingState)
    {
        HostUpdateXfer(hc, urb, HC_XFER_XERR);
        HostSaveToggle(hc, urb);

        if (USB_SPEED_HS==hc->speed && !hc->EpIsIn)
        {
            urb->PingState = 1;
        }
    }
    HostHaltChn(hc, HC_XFER_XERR);

    DISABLE_HC_INT(pChnReg, xacterr);
}

/*******************************************************************************
** Name: HostBabbleIsr
** Input:UHC_CHN_INFO * hc
** Return: void
** Owner:wrm
** Date: 2014.9.24
** Time: 16:21:56
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON FUN static void HostBabbleIsr(UHC_CHN_INFO* hc)
{
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;
    HOST_CHANNEL_REG *pChnReg  = &otg_core->Host.hchn[hc->chn];

    HostCompleteUrb(hc->urb, -HOST_BABBLE);
    HostHaltChn(hc, HC_XFER_BABBLE);

    DISABLE_HC_INT(pChnReg, bblerr);
}

/*******************************************************************************
** Name: HostTglerrIsr
** Input:UHC_CHN_INFO * hc
** Return: void
** Owner:wrm
** Date: 2014.9.24
** Time: 16:21:56
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON FUN static void HostTglerrIsr(UHC_CHN_INFO* hc)
{
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;
    HOST_CHANNEL_REG *pChnReg  = &otg_core->Host.hchn[hc->chn];

    if (hc->EpIsIn) //ep_is_in
    {
        hc->urb->ErrCnt = 0;
    }
    else
    {
        //printf("Data Toggle Error on OUT transfer,""channel %d\n", _hc->hc_num);
    }

    DISABLE_HC_INT(pChnReg, datatglerr);
}



/*******************************************************************************
** Name: int8 chn
** Input:UHC_CHN_INFO * pUsbHC
** Return: void
** Owner:wrm
** Date: 2014.9.24
** Time: 16:21:56
*******************************************************************************/
_CPU_NANOD_LIB_USB_COMMON_
COMMON FUN static void HostChnIsr(int8 chn, UHC_CHN_INFO * pUsbHC)
{
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;
    HCINT_DATA hcint;
    HCINTMSK_DATA hcintmsk;
    HOST_CHANNEL_REG *pChnReg  = &otg_core->Host.hchn[chn];
    UHC_CHN_INFO* hc = HostGetChn(chn, pUsbHC);

    hcint.d32 = pChnReg->hcintn;
    hcintmsk.d32 = pChnReg->hcintmaskn;

    hcint.d32 = hcint.d32 & hcintmsk.d32;

    if ((hcint.b.chhltd) && (hcint.d32 != 0x2))
    {
        hcint.b.chhltd = 0;
    }

    if (hcint.b.xfercomp)
    {
        //printf ("Xfer TX Complete\n");
        HostXfercompIsr(hc);
        /*
         * If NYET occurred at same time as Xfer Complete, the NYET is
         * handled by the Xfer Complete interrupt handler. Don't want
         * to call the NYET interrupt handler in this case.
         */
        hcint.b.nyet = 0;
    }
    if (hcint.b.chhltd)
    {
        //printf ("Host ReleaseChn\n");
        HostChhltdIsr(hc);
    }
    if (hcint.b.ahberr)
    {
        ;
    }
    if (hcint.b.stall)
    {
        //printf ("Host Stall\n");
        HostStallIsr(hc);
    }
    if (hcint.b.nak)
    {
        //printf ("Host Nak\n");
        HostNakIsr(hc);
    }
    if (hcint.b.ack)
    {
        //printf ("Host Ack\n");
        HostAckIsr(hc);
    }
    if (hcint.b.nyet)
    {
        //printf ("Host Nyet\n");
        HostNyetIsr(hc);
    }
    if (hcint.b.xacterr)
    {
        //printf ("Host Xacterr\n");
        HostXacterrIsr(hc);
    }
    if ( hcint.b.bblerr)
    {
        //printf ("Host Babblerr\n");
        HostBabbleIsr(hc);
    }

    if (hcint.b.frmovrun)
    {
        /**
        * Handles a host channel frame overrun interrupt. This handler may be called
        * in either DMA mode or Slave mode.
        */
    }
    if (hcint.b.datatglerr)
    {
        //printf ("Host TglerrIsr\n");
        HostTglerrIsr(hc);
    }
}

#endif

#endif


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



#endif

