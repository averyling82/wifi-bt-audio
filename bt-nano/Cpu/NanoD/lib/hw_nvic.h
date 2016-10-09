/*
********************************************************************************
*                   Copyright (c) 2008,ZhengYongzhi
*                         All rights reserved.
*
* File Name£º   hw_nvic.h
*
* Description:
*
*
* History:      <author>          <time>        <version>
*             ZhengYongzhi      2008-8-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _HW_NVIC_H_
#define _HW_NVIC_H_

/*
*-------------------------------------------------------------------------------
*
*                           nvic memap register define
*
*-------------------------------------------------------------------------------
*/
typedef volatile struct {

    UINT32 MasterCtrl;

    UINT32 IntCtrlType;                                         //interrupt control type register(abbreviation)

    UINT32 zReserved0x008_0x010[(0x010 - 0x008) / 4];

    struct {
        UINT32 Ctrl;                                            //SysTick control and status R
        UINT32 Reload;                                          //SysTick reload value R
        UINT32 Value;                                           //SysTick current value R.
        UINT32 Calibration;                                     //SysTick clibration R.
    }SysTick;

    UINT32 zReserved0x020_0x100[(0x100 - 0x020) / 4];

    /* 0~239 Configurable interrupts */
    struct {
        UINT32 Enable[32];                                      //interrupt enable set R        8~31 Reserved
        UINT32 Disable[32];                                     //interrupt enable bit clear R. 8~31 Reserved
        UINT32 SetPend[32];                                     //interrupt pend set R.      8~31 Reserved
        UINT32 ClearPend[32];                                   //interrupt pend bit clear R.8~31 Reserved
        UINT32 Active[64];                                      //active bit R                  8~63 Reserved
        UINT32 Priority[64];                                    //interrupt priority R.    64 * 4 = 256 every priority be set by one byte.
    }Irq;

    UINT32 zReserved0x500_0xd00[(0xd00 - 0x500) / 4];

    UINT32 CPUID;                                               //CPUID Base Register

    UINT32 INTcontrolState;                                     //Interrupt Control State Register(contain CortexM3 Fixed interrupts pending flag,status and cortrol bit.)
    UINT32 VectorTableOffset;                                   //vector table offset R

    UINT32 APIntRst;                                            //application interrupt/reset control R.
    UINT32 SysCtrl;                                             //System Control R
    UINT32 ConfigCtrl;                                          //System Control R
    UINT32 SystemPriority[3];                                   //System Handlers Priority R
    UINT32 SystemHandlerCtrlAndState;                           //system cpu control and status R
    UINT32 ConfigurableFaultStatus;                             //fault configuration R.
    UINT32 HardFaultStatus;                                     //hardware fault status R
    UINT32 DebugFaultStatus;                                    //debug fault status R.
    UINT32 MemManageAddress;                                    //memory manage fault address R
    UINT32 BusFaultAddress;                                     //bus fault address R
    UINT32 AuxFaultStatus;                                      //Auxiliary Fault Status Register

//------------------------------------------------------------------------------
//Read Only
    UINT32 PFR0;                                                //cpu function R0
    UINT32 PFR1;                                                //cpu function R1
    UINT32 DFR0;                                                //debug function R0
    UINT32 AFR0;                                                //auxiliary function R0
    UINT32 MMFR0;                                               //memory moudel function R0
    UINT32 MMFR1;                                               //memory moudel function R1
    UINT32 MMFR2;                                               //memory moudel function R2
    UINT32 MMFR3;                                               //memory moudel function R3
    UINT32 ISAR0;                                               //ISA function R0
    UINT32 ISAR1;                                               //ISA function R1
    UINT32 ISAR2;                                               //ISA function R2
    UINT32 ISAR3;                                               //ISA function R3
    UINT32 ISAR4;                                               //ISA function R4 ED70
//------------------------------------------------------------------------------

    UINT32 zReserved0xd74_0xd90[(0xd90 - 0xd74) / 4];

    struct {
        UINT32 Type;                                            //MPU type R
        UINT32 Ctrl;                                            //MPU control R
        UINT32 RegionNumber;                                    //MPU area number R
        UINT32 RegionBaseAddr;                                  //MPU district base R
        UINT32 RegionAttrSize;                                  //MPU district property and size R

        UINT32 RegionBaseAddrAlias1;                            //MPU alias1 district base address R
        UINT32 RegionAttrSizeAlias1;                            //MPU alias1 district property and size R
        UINT32 RegionBaseAddrAlias2;                            //MPU alias2 district base address R
        UINT32 RegionAttrSizeAlias2;                            //MPU alias2 district property and size R
        UINT32 RegionBaseAddrAlias3;                            //MPU alias3 district base address R
        UINT32 RegionAttrSizeAlias3;                            //MPU alias4 district property and size R db8

    } MPU;

    UINT32 zReserved0xdbc_0xf00[(0xf00 - 0xdbc) / 4];

    UINT32 SoftTriInt;                                          //software trigger interrupt R

    UINT32 zReserved0xf04_0xfd0[(0xfd0 - 0xf04) / 4];

    UINT32 PID4;                                                //peripheral distinguish R£¨PERIPHID4£©
    UINT32 PID5;                                                //peripheral distinguish R£¨PERIPHID5£©
    UINT32 PID6;                                                //peripheral distinguish R (PERIPHID6£©
    UINT32 PID7;                                                //peripheral distinguish R£¨PERIPHID7£©

    UINT32 PID0;                                                //peripheral distinguish R bit7:0£¨PERIPHID0£©
    UINT32 PID1;                                                //peripheral distinguish R bit15:8£¨PERIPHID1£©
    UINT32 PID2;                                                //peripheral distinguish R bit23:16£¨PERIPHID2£©
    UINT32 PID3;                                                //peripheral distinguish R bit31:24£¨PERIPHID3£©

    UINT32 CID0;                                                //component distinguish R bit7:0£¨PCELLID0£©
    UINT32 CID1;                                                //component distinguish R bit15:8£¨PCELLID1£©
    UINT32 CID2;                                                //component distinguish R bit23:16£¨PCELLID2£©
    UINT32 CID3;                                                //component distinguish R bit31:24£¨PCELLID3£©

} NVIC_t;

#define NVIC_BASE               ((UINT32)0xE000E000)
#define nvic                    ((NVIC_t *) NVIC_BASE)

/*
*-------------------------------------------------------------------------------
*
*  The following define the bit fields in the IntCtrlType register.
*
*-------------------------------------------------------------------------------
*/
#define     NVIC_INTCTRLTYPE_INTSUM0_32      ((UINT32)0x00000000 << 0)
#define     NVIC_INTCTRLTYPE_INTSUM33_64     ((UINT32)0x00000001 << 0)
#define     NVIC_INTCTRLTYPE_INTSUM65_96     ((UINT32)0x00000002 << 0)
#define     NVIC_INTCTRLTYPE_INTSUM97_128    ((UINT32)0x00000003 << 0)
#define     NVIC_INTCTRLTYPE_INTSUM129_160   ((UINT32)0x00000004 << 0)
#define     NVIC_INTCTRLTYPE_INTSUM161_192   ((UINT32)0x00000005 << 0)
#define     NVIC_INTCTRLTYPE_INTSUM193_224   ((UINT32)0x00000006 << 0)
#define     NVIC_INTCTRLTYPE_INTSUM225_256   ((UINT32)0x00000007 << 0)

/*
*-------------------------------------------------------------------------------
*
*  The following define the bit fields in the SysTick Control and Status Register.
*  (SysTick.Ctrl)
*
*-------------------------------------------------------------------------------
*/
#define     NVIC_SYSTICKCTRL_ENABLE         ((UINT32)0x00000001 << 0)
#define     NVIC_SYSTICKCTRL_TICKINT        ((UINT32)0x00000001 << 1)

#define     NVIC_SYSTICKCTRL_CLKEX          ((UINT32)0x00000000 << 2)
#define     NVIC_SYSTICKCTRL_CLKIN          ((UINT32)0x00000001 << 2)

#define     NVIC_SYSTICKCTRL_FLAG           ((UINT32)0x00000001 << 16)

/*
*-------------------------------------------------------------------------------
*
*  The following define the bit fields in the SysTick Calibration Value Register.
*  (SysTick.Calibration)
*
*-------------------------------------------------------------------------------
*/
#define     NVIC_SYSTICKCALIB_TEMMS_MASK    ((UINT32)0x00ffffff)

#define     NVIC_SYSTICKCALIB_SKEW          ((UINT32)0x00000001 << 30)
#define     NVIC_SYSTICKCALIB_NOREF         ((UINT32)0x00000001 << 31)

/*
*-------------------------------------------------------------------------------
*
*  The following define the bit fields in the CPU ID Base Register.
*  (CPUID)
*
*-------------------------------------------------------------------------------
*/
#define     NVIC_CPUID_REVISION_MASK        ((UINT32)0x0000000f)
#define     NVIC_CPUID_PARTNO_MASK          ((UINT32)0x0000fff0)
#define     NVIC_CPUID_CONSTANT_MASK        ((UINT32)0x000f0000)
#define     NVIC_CPUID_VARIANT_MASK         ((UINT32)0x00f00000)
#define     NVIC_CPUID_IMPLEMENTER_MASK     ((UINT32)0xff000000)

/*
*-------------------------------------------------------------------------------
*
*  The following define the bit fields in the Interrupt Control State Register.
*  (INTcontrolState)
*
*-------------------------------------------------------------------------------
*/
#define     NVIC_INTCTRLSTA_VECTACTIVE_MASK ((UINT32)0x000001ff)
#define     NVIC_INTCTRLSTA_VECTPEND_MASK   ((UINT32)0x003ff000)

#define     NVIC_INTCTRLSTA_RETTOBASE       ((UINT32)0x00000001 << 11)
#define     NVIC_INTCTRLSTA_ISRPENDING      ((UINT32)0x00000001 << 22)
#define     NVIC_INTCTRLSTA_ISRPREEMPT      ((UINT32)0x00000001 << 23)
#define     NVIC_INTCTRLSTA_PENDSTCLR       ((UINT32)0x00000001 << 25)
#define     NVIC_INTCTRLSTA_PENDSTSET       ((UINT32)0x00000001 << 26)
#define     NVIC_INTCTRLSTA_PENDSVCLR       ((UINT32)0x00000001 << 27)
#define     NVIC_INTCTRLSTA_PENDSVSET       ((UINT32)0x00000001 << 28)
#define     NVIC_INTCTRLSTA_NMIPENDSET      ((UINT32)0x00000001 << 31)

/*
*-------------------------------------------------------------------------------
*
*  The following define the bit fields in the Vector Table Offset Register.
*  (VectorTableOffset)
*
*-------------------------------------------------------------------------------
*/
#define     NVIC_VECTTABLOFFSET_MASK        ((UINT32)0x1fffff80)

#define     NVIC_VECTTABLOFFSET_TBLBASE     ((UINT32)0x00000001 << 29)

/*
*-------------------------------------------------------------------------------
*
*  The following define the bit fields in the Application Interrupt and Reset
*  Control Register.(APIntRst)
*
*-------------------------------------------------------------------------------
*/
#define     NVIC_APINTRST_VECTKEY           ((UINT32)0x05FA0000)

#define     NVIC_APINTRST_VECTRESET         ((UINT32)0x00000001 << 0)
#define     NVIC_APINTRST_VECTCLRACTIVE     ((UINT32)0x00000001 << 1)
#define     NVIC_APINTRST_SYSRESETREQ       ((UINT32)0x00000001 << 2)

#define     NVIC_APINTRST_PRIGROUP_MASK     ((UINT32)0x00000700)
#define     NVIC_APINTRST_PRIGROUP7_1       ((UINT32)0x00000000 << 8)
#define     NVIC_APINTRST_PRIGROUP6_2       ((UINT32)0x00000001 << 8)
#define     NVIC_APINTRST_PRIGROUP5_3       ((UINT32)0x00000002 << 8)
#define     NVIC_APINTRST_PRIGROUP4_4       ((UINT32)0x00000003 << 8)
#define     NVIC_APINTRST_PRIGROUP3_5       ((UINT32)0x00000004 << 8)
#define     NVIC_APINTRST_PRIGROUP2_6       ((UINT32)0x00000005 << 8)
#define     NVIC_APINTRST_PRIGROUP1_7       ((UINT32)0x00000006 << 8)
#define     NVIC_APINTRST_PRIGROUP0_8       ((UINT32)0x00000007 << 8)

#define     NVIC_APINTRST_ENDIANESS_LITT    ((UINT32)0x00000000 << 15)
#define     NVIC_APINTRST_ENDIANESS_BIG     ((UINT32)0x00000001 << 15)

/*
*-------------------------------------------------------------------------------
*
*  The following define the bit fields in the System Control Register.
*  (SysCtrl)
*
*-------------------------------------------------------------------------------
*/
#define     NVIC_SYSCTRL_SLEEPONEXIT        ((UINT32)0x00000001 << 1)
#define     NVIC_SYSCTRL_SLEEPDEEP          ((UINT32)0x00000001 << 2)
#define     NVIC_SYSCTRL_SEVONPEND          ((UINT32)0x00000001 << 4)

/*
*-------------------------------------------------------------------------------
*
*  The following define the bit fields in the Configuration Control Register.
*  (ConfigCtrl)
*
*-------------------------------------------------------------------------------
*/
#define     NVIC_CONFIGCTRL_NONEBASETHRDENA ((UINT32)0x00000001 << 0)
#define     NVIC_CONFIGCTRL_USERSETMPEND    ((UINT32)0x00000001 << 1)
#define     NVIC_CONFIGCTRL_UNALIGN_TRP     ((UINT32)0x00000001 << 3)
#define     NVIC_CONFIGCTRL_DIV_0_TRP       ((UINT32)0x00000001 << 4)
#define     NVIC_CONFIGCTRL_BFHFNMIGN       ((UINT32)0x00000001 << 8)
#define     NVIC_CONFIGCTRL_STKALIGN        ((UINT32)0x00000001 << 9)

/*
*-------------------------------------------------------------------------------
*
*  The following define the bit fields in the System Handler Control and State Register.
*  (SystemHandlerCtrlAndState)
*
*-------------------------------------------------------------------------------
*/
#define     NVIC_SYSHANDCTRL_MEMFAULTACT    ((UINT32)0x00000001 << 0)
#define     NVIC_SYSHANDCTRL_BUSFAULTACT    ((UINT32)0x00000001 << 1)
#define     NVIC_SYSHANDCTRL_USGFAULTACT    ((UINT32)0x00000001 << 3)
#define     NVIC_SYSHANDCTRL_SVCALLACT      ((UINT32)0x00000001 << 7)
#define     NVIC_SYSHANDCTRL_MONITORACT     ((UINT32)0x00000001 << 8)
#define     NVIC_SYSHANDCTRL_PENDSVACT      ((UINT32)0x00000001 << 10)
#define     NVIC_SYSHANDCTRL_SYSTICKACT     ((UINT32)0x00000001 << 11)
#define     NVIC_SYSHANDCTRL_USGFAULTPENDED ((UINT32)0x00000001 << 12)
#define     NVIC_SYSHANDCTRL_MEMFAULTPENDED ((UINT32)0x00000001 << 13)
#define     NVIC_SYSHANDCTRL_BUSFAULTPENDED ((UINT32)0x00000001 << 14)
#define     NVIC_SYSHANDCTRL_SVCALLPENDED   ((UINT32)0x00000001 << 15)
#define     NVIC_SYSHANDCTRL_MEMFAULTENA    ((UINT32)0x00000001 << 16)
#define     NVIC_SYSHANDCTRL_BUSFAULTENA    ((UINT32)0x00000001 << 17)
#define     NVIC_SYSHANDCTRL_USGFAULTENA    ((UINT32)0x00000001 << 18)

/*
*-------------------------------------------------------------------------------
*
*  The following define the bit fields in the Configurable Fault Status Registers.
*  (ConfigurableFaultStatus)
*
*-------------------------------------------------------------------------------
*/
#define     NVIC_CONFIGFAULT_IACCVIOL       ((UINT32)0x00000001 << 0)
#define     NVIC_CONFIGFAULT_DACCVIOL       ((UINT32)0x00000001 << 1)
#define     NVIC_CONFIGFAULT_MUNSTKERR      ((UINT32)0x00000001 << 3)
#define     NVIC_CONFIGFAULT_MSTKERR        ((UINT32)0x00000001 << 4)
#define     NVIC_CONFIGFAULT_MMARVALID      ((UINT32)0x00000001 << 7)

#define     NVIC_CONFIGFAULT_IBUSERR        ((UINT32)0x00000001 << 8)
#define     NVIC_CONFIGFAULT_PRECISERR      ((UINT32)0x00000001 << 9)
#define     NVIC_CONFIGFAULT_IMPRECISERR    ((UINT32)0x00000001 << 10)
#define     NVIC_CONFIGFAULT_UNSTKERR       ((UINT32)0x00000001 << 11)
#define     NVIC_CONFIGFAULT_STKERR         ((UINT32)0x00000001 << 12)
#define     NVIC_CONFIGFAULT_BFARVALID      ((UINT32)0x00000001 << 15)

#define     NVIC_CONFIGFAULT_UNDEFINSTR     ((UINT32)0x00000001 << 0)
#define     NVIC_CONFIGFAULT_INVSTATE       ((UINT32)0x00000001 << 1)
#define     NVIC_CONFIGFAULT_INVPC          ((UINT32)0x00000001 << 2)
#define     NVIC_CONFIGFAULT_NOCP           ((UINT32)0x00000001 << 3)
#define     NVIC_CONFIGFAULT_UNALIGNED      ((UINT32)0x00000001 << 8)
#define     NVIC_CONFIGFAULT_DIVBYZERO      ((UINT32)0x00000001 << 9)

/*
*-------------------------------------------------------------------------------
*
*  The following define the bit fields in the Hard Fault Status Register.
*  (HardFaultStatus)
*
*-------------------------------------------------------------------------------
*/
#define     NVIC_HARDFAULT_VECTTBL          ((UINT32)0x00000001 << 0)
#define     NVIC_HARDFAULT_FORCED           ((UINT32)0x00000001 << 30)
#define     NVIC_HARDFAULT_DEBUGEVT         ((UINT32)0x00000001 << 31)

/*
*-------------------------------------------------------------------------------
*
*  The following define the bit fields in the Debug Fault Status Register.
*  (DebugFaultStatus)
*
*-------------------------------------------------------------------------------
*/
#define     NVIC_DEBUGFAULT_HALTED          ((UINT32)0x00000001 << 0)
#define     NVIC_DEBUGFAULT_BKPT            ((UINT32)0x00000001 << 1)
#define     NVIC_DEBUGFAULT_DWTTRAP         ((UINT32)0x00000001 << 2)
#define     NVIC_DEBUGFAULT_VCATCH          ((UINT32)0x00000001 << 3)
#define     NVIC_DEBUGFAULT_EXTERNAL        ((UINT32)0x00000001 << 4)

/*
*-------------------------------------------------------------------------------
*
*  The following define the bit fields in the MPU Type Register.
*  (MPU.type)
*
*-------------------------------------------------------------------------------
*/
#define     NVIC_MPUTYPE_DREGION_MASK       ((UINT32)0x0000ff00)
#define     NVIC_MPUTYPE_IREGION_MASK       ((UINT32)0x00ff0000)

#define     NVIC_MPUTYPE_SEPARATE           ((UINT32)0x00000001 << 0)

/*
*-------------------------------------------------------------------------------
*
*  The following define the bit fields in the MPU Control Register.
*  (MPU.Ctrl)
*
*-------------------------------------------------------------------------------
*/
#define     NVIC_MPUCTRL_ENABLE             ((UINT32)0x00000001 << 0)
#define     NVIC_MPUCTRL_HFNMIENA           ((UINT32)0x00000001 << 1)
#define     NVIC_MPUCTRL_PRIVDEFENA         ((UINT32)0x00000001 << 2)

/*
*-------------------------------------------------------------------------------
*
*  The following define the bit fields in the MPU Region Number Register.
*  (MPU.RegionNumber)
*
*-------------------------------------------------------------------------------
*/
#define     NVIC_MPUREGIONNUM_MASK          ((UINT32)0x000000ff)

/*
*-------------------------------------------------------------------------------
*
*  The following define the bit fields in the MPU Region Base Address Register.
*  (MPU.RegionBaseAddr)
*
*-------------------------------------------------------------------------------
*/
#define     NVIC_MPUREGIONBASE_REGION_MASK  ((UINT32)0x0000000f)
#define     NVIC_MPUREGIONBASE_ADDR_MASK    ((UINT32)0xffffffe0)

#define     NVIC_MPUREGIONBASE_VALID        ((UINT32)0x00000001 << 4)

/*
*-------------------------------------------------------------------------------
*
*  The following define the bit fields in the MPU Region Attribute and Size Register.
*  (MPU.RegionAttrSize)
*
*-------------------------------------------------------------------------------
*/
#define     NVIC_MPUATTRSIZE_REGION_MASK    ((UINT32)0x0000000f)

#define     NVIC_MPUATTRSIZE_ENABLE         ((UINT32)0x00000001 << 0)

#define     NVIC_MPUATTRSIZE_SIZE_32B       ((UINT32)0x00000004 << 1)
#define     NVIC_MPUATTRSIZE_SIZE_64B       ((UINT32)0x00000005 << 1)
#define     NVIC_MPUATTRSIZE_SIZE_128B      ((UINT32)0x00000006 << 1)
#define     NVIC_MPUATTRSIZE_SIZE_256B      ((UINT32)0x00000007 << 1)
#define     NVIC_MPUATTRSIZE_SIZE_512B      ((UINT32)0x00000008 << 1)
#define     NVIC_MPUATTRSIZE_SIZE_1K        ((UINT32)0x00000009 << 1)
#define     NVIC_MPUATTRSIZE_SIZE_2K        ((UINT32)0x0000000A << 1)
#define     NVIC_MPUATTRSIZE_SIZE_4K        ((UINT32)0x0000000B << 1)
#define     NVIC_MPUATTRSIZE_SIZE_8K        ((UINT32)0x0000000C << 1)
#define     NVIC_MPUATTRSIZE_SIZE_16K       ((UINT32)0x0000000D << 1)
#define     NVIC_MPUATTRSIZE_SIZE_32K       ((UINT32)0x0000000E << 1)
#define     NVIC_MPUATTRSIZE_SIZE_64K       ((UINT32)0x0000000F << 1)
#define     NVIC_MPUATTRSIZE_SIZE_128K      ((UINT32)0x00000010 << 1)
#define     NVIC_MPUATTRSIZE_SIZE_256K      ((UINT32)0x00000011 << 1)
#define     NVIC_MPUATTRSIZE_SIZE_512K      ((UINT32)0x00000012 << 1)
#define     NVIC_MPUATTRSIZE_SIZE_1M        ((UINT32)0x00000013 << 1)
#define     NVIC_MPUATTRSIZE_SIZE_2M        ((UINT32)0x00000014 << 1)
#define     NVIC_MPUATTRSIZE_SIZE_4M        ((UINT32)0x00000015 << 1)
#define     NVIC_MPUATTRSIZE_SIZE_8M        ((UINT32)0x00000016 << 1)
#define     NVIC_MPUATTRSIZE_SIZE_16M       ((UINT32)0x00000017 << 1)
#define     NVIC_MPUATTRSIZE_SIZE_32M       ((UINT32)0x00000018 << 1)
#define     NVIC_MPUATTRSIZE_SIZE_64M       ((UINT32)0x00000019 << 1)
#define     NVIC_MPUATTRSIZE_SIZE_128M      ((UINT32)0x0000001A << 1)
#define     NVIC_MPUATTRSIZE_SIZE_256M      ((UINT32)0x0000001B << 1)
#define     NVIC_MPUATTRSIZE_SIZE_512M      ((UINT32)0x0000001C << 1)
#define     NVIC_MPUATTRSIZE_SIZE_1G        ((UINT32)0x0000001D << 1)
#define     NVIC_MPUATTRSIZE_SIZE_2G        ((UINT32)0x0000001E << 1)
#define     NVIC_MPUATTRSIZE_SIZE_4G        ((UINT32)0x0000001F << 1)

#define     NVIC_MPUATTRSIZE_SRD0           ((UINT32)0x00000001 << 8)
#define     NVIC_MPUATTRSIZE_SRD1           ((UINT32)0x00000001 << 9)
#define     NVIC_MPUATTRSIZE_SRD2           ((UINT32)0x00000001 << 10)
#define     NVIC_MPUATTRSIZE_SRD3           ((UINT32)0x00000001 << 11)
#define     NVIC_MPUATTRSIZE_SRD4           ((UINT32)0x00000001 << 12)
#define     NVIC_MPUATTRSIZE_SRD5           ((UINT32)0x00000001 << 13)
#define     NVIC_MPUATTRSIZE_SRD6           ((UINT32)0x00000001 << 14)
#define     NVIC_MPUATTRSIZE_SRD7           ((UINT32)0x00000001 << 15)

#define     NVIC_MPUATTRSIZE_B              ((UINT32)0x00000001 << 16)
#define     NVIC_MPUATTRSIZE_C              ((UINT32)0x00000001 << 17)
#define     NVIC_MPUATTRSIZE_S              ((UINT32)0x00000001 << 18)

#define     NVIC_MPUATTRSIZE_TEX            ((UINT32)0x00000001 << 19)

#define     NVIC_MPUATTRSIZE_P_NA_U_NA      ((UINT32)0x00000000 << 24)
#define     NVIC_MPUATTRSIZE_P_RW_U_NA      ((UINT32)0x00000001 << 24)
#define     NVIC_MPUATTRSIZE_P_RW_U_RO      ((UINT32)0x00000002 << 24)
#define     NVIC_MPUATTRSIZE_P_RW_U_RW      ((UINT32)0x00000003 << 24)
#define     NVIC_MPUATTRSIZE_P_RO_U_NA      ((UINT32)0x00000005 << 24)
#define     NVIC_MPUATTRSIZE_P_RO_U_RO      ((UINT32)0x00000006 << 24)

#define     NVIC_MPUATTRSIZE_XN             ((UINT32)0x00000001 << 28)

/*
*-------------------------------------------------------------------------------
*
*  The following define the bit fields in the Software Trigger Interrupt Register.
*  (SoftTriInt)
*
*-------------------------------------------------------------------------------
*/
#define     NVIC_SFTRINT_INTID_MASK         ((UINT32)0x000001ff)

/*
********************************************************************************
*
*                         End of hw_nvic.h
*
********************************************************************************
*/
#endif
