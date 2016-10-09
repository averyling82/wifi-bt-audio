/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Cpu\NanoD\lib\hw_usb.h
* Owner: aaron.sun
* Date: 2015.5.13
* Time: 17:06:31
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2015.5.13     17:06:31   1.0
********************************************************************************************
*/


#ifndef __CPU_NANOD_LIB_HW_USB_H__
#define __CPU_NANOD_LIB_HW_USB_H__

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
#define _CPU_NANOD_LIB_HW_USB_COMMON_  __attribute__((section("cpu_nanod_lib_hw_usb_common")))
#define _CPU_NANOD_LIB_HW_USB_INIT_  __attribute__((section("cpu_nanod_lib_hw_usb_init")))
#define _CPU_NANOD_LIB_HW_USB_SHELL_  __attribute__((section("cpu_nanod_lib_hw_usb_shell")))

#define       USB_REG_BASE              USB_BASE
#define       USB_HOST_BASE             (USB_REG_BASE+0x400)
#define       USB_DEV_BASE              (USB_REG_BASE+0x800)

#define       USB_EP_FIFO(ep)           (USB_REG_BASE +((ep+1)<<12))

typedef  struct _CORE_REG
{
    uint32 gotgctl;
    uint32 gotgint;
    uint32 gahbcfg;
    uint32 gusbcfg;
    uint32 grstctl;
    uint32 gintsts;
    uint32 gintmsk;
    uint32 grxstsr;
    uint32 grxstsp;
    uint32 grxfsiz;
    uint32 gnptxfsiz;
    uint32 gnptxsts;
    uint32 gi2cctl;
    uint32 gpvndctl;
    uint32 ggpio;
    uint32 guid;
    uint32 gsnpsid;
    uint32 ghwcfg1;
    uint32 ghwcfg2;
    uint32 ghwcfg3;
    uint32 ghwcfg4;
    uint32 RESERVED1[(0x100-0x54)/4];
    uint32 hptxfsiz;
    uint32 dptxfsiz_dieptxf[15];
    uint32 RESERVED2[(0x400-0x140)/4];
}CORE_REG;


typedef  struct _HOST_CHANNEL_REG
{
    uint32 hccharn;
    uint32 hcspltn;
    uint32 hcintn;
    uint32 hcintmaskn;
    uint32 hctsizn;
    uint32 hcdman;
    uint32 reserved[2];
}HOST_CHANNEL_REG;


typedef  struct _HOST_REG
{
    uint32 hcfg;
    uint32 hfir;
    uint32 hfnum;
    uint32 reserved40C;
    uint32 hptxsts;
    uint32 haint;
    uint32 haintmsk;
    uint32 reserved1[(0x440-0x41c)/4];
    uint32 hprt;
    uint32 reserved2[(0x500-0x444)/4];
    HOST_CHANNEL_REG hchn[4];
    uint32 reserved3[(0x800-0x580)/4];
}HOST_REG;

typedef  struct _IN_EP_REG
{
    /** Device IN Endpoint Control Register. <i>Offset:900h +
     * (ep_num * 20h) + 00h</i> */
    uint32 diepctl;
    /** Reserved. <i>Offset:900h + (ep_num * 20h) + 04h</i> */
    uint32 reserved04;
    /** Device IN Endpoint Interrupt Register. <i>Offset:900h +
    * (ep_num * 20h) + 08h</i> */
    uint32 diepint;
    /** Reserved. <i>Offset:900h + (ep_num * 20h) + 0Ch</i> */
    uint32 reserved0C;
    /** Device IN Endpoint Transfer Size
    * Register. <i>Offset:900h + (ep_num * 20h) + 10h</i> */
    uint32 dieptsiz;
    /** Device IN Endpoint DMA Address Register. <i>Offset:900h +
    * (ep_num * 20h) + 14h</i> */
    uint32 diepdma;
    /** Device IN Endpoint Transmit FIFO Status Register. <i>Offset:900h +
    * (ep_num * 20h) + 18h</i> */
    uint32 dtxfsts;
    /** Reserved. <i>Offset:900h + (ep_num * 20h) + 1Ch - 900h +
    * (ep_num * 20h) + 1Ch</i>*/
    uint32 reserved18;
}IN_EP_REG;

typedef  struct _OUT_EP_REG
{
    /** Device OUT Endpoint Control Register. <i>Offset:B00h +
     * (ep_num * 20h) + 00h</i> */
    uint32 doepctl;
    /** Device OUT Endpoint Frame number Register.  <i>Offset:
     * B00h + (ep_num * 20h) + 04h</i> */
    uint32 doepfn;
    /** Device OUT Endpoint Interrupt Register. <i>Offset:B00h +
     * (ep_num * 20h) + 08h</i> */
    uint32 doepint;
    /** Reserved. <i>Offset:B00h + (ep_num * 20h) + 0Ch</i> */
    uint32 reserved0C;
    /** Device OUT Endpoint Transfer Size Register. <i>Offset:
     * B00h + (ep_num * 20h) + 10h</i> */
    uint32 doeptsiz;
    /** Device OUT Endpoint DMA Address Register. <i>Offset:B00h
     * + (ep_num * 20h) + 14h</i> */
    uint32 doepdma;
    /** Reserved. <i>Offset:B00h + (ep_num * 20h) + 18h - B00h +
     * (ep_num * 20h) + 1Ch</i> */
    uint32 unused[2];
}OUT_EP_REG;


typedef  struct _DEVICE_REG
{
    /** Device Configuration Register. <i>Offset 800h</i> */
    uint32 dcfg;
    /** Device Control Register. <i>Offset: 804h</i> */
    uint32 dctl;
    /** Device Status Register (Read Only). <i>Offset: 808h</i> */
    uint32 dsts;
    /** Reserved. <i>Offset: 80Ch</i> */
    uint32 unused;
    /** Device IN Endpoint Common Interrupt Mask
    * Register. <i>Offset: 810h</i> */
    uint32 diepmsk;
    /** Device OUT Endpoint Common Interrupt Mask
    * Register. <i>Offset: 814h</i> */
    uint32 doepmsk;
    /** Device All Endpoints Interrupt Register.  <i>Offset: 818h</i> */
    uint32 daint;
    /** Device All Endpoints Interrupt Mask Register.  <i>Offset:
    * 81Ch</i> */
    uint32 daintmsk;
    /** Device IN Token Queue Read Register-1 (Read Only).
    * <i>Offset: 820h</i> */
    uint32 dtknqr1;
    /** Device IN Token Queue Read Register-2 (Read Only).
    * <i>Offset: 824h</i> */
    uint32 dtknqr2;
    /** Device VBUS  discharge Register.  <i>Offset: 828h</i> */
    uint32 dvbusdis;
    /** Device VBUS Pulse Register.  <i>Offset: 82Ch</i> */
    uint32 dvbuspulse;
    /** Device IN Token Queue Read Register-3 (Read Only). /
    *  Device Thresholding control register (Read/Write)
    * <i>Offset: 830h</i> */
    uint32 dtknqr3_dthrctl;
    /** Device IN Token Queue Read Register-4 (Read Only). /
    *  Device IN EPs empty Inr. Mask Register (Read/Write)
    * <i>Offset: 834h</i> */
    uint32 dtknqr4_fifoemptymsk;

    uint32 reserved[(0x900-0x838)/4];

    /* Device Logical IN Endpoint-Specific Registers 900h-AFCh */
    IN_EP_REG in_ep[16];

    /** Device Logical OUT Endpoint-Specific Registers B00h-CFCh */
    OUT_EP_REG out_ep[16];

    uint32 reserved2[(0xe00-0xd00)/4];
}DEVICE_REG;


typedef  struct _FIFO_REG
{
    uint32 DataPort;
    uint32 reserved[(0x1000-0x004)/4];
}FIFO_REG;

typedef  struct _CLOCK_GATE_REG
{
    uint32 pcgcctl;
    uint32 reserved[(0x1000-0xe04)/4];
}CLOCK_GATE_REG;


typedef  struct _USB_OTG_REG
{
    CORE_REG Core;
    HOST_REG Host;
    DEVICE_REG Device;
    CLOCK_GATE_REG ClkGate;
    FIFO_REG Fifo[16];
    uint32 reserved[(0x40000-0x11000)/4];
}USB_OTG_REG;



typedef union _DEVICE_GRXSTS_DATA
{
    /** raw register data */
    uint32 d32;
    /** register bits */
    struct
    {
unsigned epnum :
        4;
unsigned bcnt :
        11;
unsigned dpid :
        2;

#define DWC_STS_DATA_UPDT        0x2                  // OUT Data Packet
#define DWC_STS_XFER_COMP        0x3                  // OUT Data Transfer Complete

#define DWC_DSTS_GOUT_NAK        0x1                  // Global OUT NAK
#define DWC_DSTS_SETUP_COMP        0x4                  // Setup Phase Complete
#define DWC_DSTS_SETUP_UPDT 0x6// SETUP Packet
unsigned pktsts :
        4;
unsigned fn :
        4;
unsigned reserved :
        7;
    } b;
} DEVICE_GRXSTS_DATA;

typedef union tagDEPCTL_DATA
{
    /** raw register data */
    uint32 d32;
    /** register bits */
    struct
    {
        /** Maximum Packet Size
         * IN/OUT EPn
         * IN/OUT EP0 - 2 bits
         *     2'b00: 64 Bytes
         *     2'b01: 32
         *     2'b10: 16
         *     2'b11: 8 */
        unsigned mps : 11;
#define DWC_DEP0CTL_MPS_64     0
#define DWC_DEP0CTL_MPS_32     1
#define DWC_DEP0CTL_MPS_16     2
#define DWC_DEP0CTL_MPS_8     3

        /** Next Endpoint
         * IN EPn/IN EP0
         * OUT EPn/OUT EP0 - reserved */
        unsigned nextep : 4;

        /** USB Active Endpoint */
        unsigned usbactep : 1;

        /** Endpoint DPID (INTR/Bulk IN and OUT endpoints)
         * This field contains the PID of the packet going to
         * be received or transmitted on this endpoint. The
         * application should program the PID of the first
         * packet going to be received or transmitted on this
         * endpoint , after the endpoint is
         * activated. Application use the SetD1PID and
         * SetD0PID fields of this register to program either
         * D0 or D1 PID.
         *
         * The encoding for this field is
         *     - 0: D0
         *     - 1: D1
         */
        unsigned dpid : 1;

        /** NAK Status */
        unsigned naksts : 1;

        /** Endpoint Type
         *    2'b00: Control
         *    2'b01: Isochronous
         *    2'b10: Bulk
         *    2'b11: Interrupt */
        unsigned eptype : 2;

        /** Snoop Mode
         * OUT EPn/OUT EP0
         * IN EPn/IN EP0 - reserved */
        unsigned snp : 1;

        /** Stall Handshake */
        unsigned stall : 1;

        /** Tx Fifo Number
         * IN EPn/IN EP0
         * OUT EPn/OUT EP0 - reserved */
        unsigned txfnum : 4;

        /** Clear NAK */
        unsigned cnak : 1;
        /** Set NAK */
        unsigned snak : 1;
        /** Set DATA0 PID (INTR/Bulk IN and OUT endpoints)
         * Writing to this field sets the Endpoint DPID (DPID)
         * field in this register to DATA0. Set Even
         * (micro)frame (SetEvenFr) (ISO IN and OUT Endpoints)
         * Writing to this field sets the Even/Odd
         * (micro)frame (EO_FrNum) field to even (micro)
         * frame.
         */
        unsigned setd0pid : 1;
        /** Set DATA1 PID (INTR/Bulk IN and OUT endpoints)
         * Writing to this field sets the Endpoint DPID (DPID)
         * field in this register to DATA1 Set Odd
         * (micro)frame (SetOddFr) (ISO IN and OUT Endpoints)
         * Writing to this field sets the Even/Odd
         * (micro)frame (EO_FrNum) field to odd (micro) frame.
         */
        unsigned setd1pid : 1;

        /** Endpoint Disable */
        unsigned epdis : 1;
        /** Endpoint Enable */
        unsigned epena : 1;
        } b;
}DEPCTL_DATA;


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



#endif
