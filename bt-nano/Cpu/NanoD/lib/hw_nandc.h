/*
********************************************************************************
*                   Copyright (c) 2008,ZhengYongzhi
*                         All rights reserved.
*
* File Name£º   hw_emctl.h
*
* Description:
*
*
* History:      <author>          <time>        <version>
*             ZhengYongzhi      2008-8-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _HW_NANDC_H_
#define _HW_NANDC_H_

/*
*-------------------------------------------------------------------------------
*
*                  external memory control memap register define
*
*-------------------------------------------------------------------------------
*/
//External Memory Control Register
//External Memory Control Register
typedef volatile struct {

    UINT32 EccCtl;
    UINT32 FMCtl;
    UINT32 Reserved1;
    UINT32 FMWait;
    UINT32 Reserved2;
    UINT32 EMICtl;
    //UINT32 SYSD_RREG_0;
    UINT32 BchEn0[28];
    UINT32 BchDe0[60];

    UINT32 BchStatus;
    UINT32 RandMC;
} NANDC_t;

#define NandCtl     ((NANDC_t *) NANDC_BASE)

//chip interface reg
typedef volatile struct tagNAND_IF
{
    uint32 data;
    uint32 addr;
    uint32 cmd;
}NAND_IF, *pNAND_IF;


#define NANDIF_BASE0    ((UINT32)(NANDC_BASE + 0x200))
#define NANDIF_BASE1    ((UINT32)(NANDC_BASE + 0x400))
#define NANDIF_BASE2    ((UINT32)(NANDC_BASE + 0x600))
#define NANDIF_BASE3    ((UINT32)(NANDC_BASE + 0x800))

#define NandIF0         ((NAND_IF *) NANDIF_BASE0)
#define NandIF1         ((NAND_IF *) NANDIF_BASE1)
#define NandIF2         ((NAND_IF *) NANDIF_BASE2)
#define NandIF3         ((NAND_IF *) NANDIF_BASE3)

/*
*-------------------------------------------------------------------------------
*
*  The following define the bit fields in the EccCtl register.
*
*-------------------------------------------------------------------------------
*/
#define     EMCTL_ECCCTL_BCHRST                 ((UINT32)0x00000001 << 0)
#define     EMCTL_ECCCTL_DATA_INPUT             ((UINT32)0x00000001 << 2)

#define     EMCTL_ECCCTL_ENABLE_MODE            ((UINT32)0x00000001 << 4)

#define     EMCTL_ECCCTL_ACTIVE_RANGE           ((UINT32)0x00000001 << 5)

#define     EMCTL_ECCCTL_ACTIVE_FLASH_0_2       ((UINT32)0x00000001 << 12)
#define     EMCTL_ECCCTL_ACTIVE_FLASH_1_3       ((UINT32)0x00000002 << 12)
#define     EMCTL_ECCCTL_BCH_POWDN              ((UINT32)0x00000001 << 14)
#define     EMCTL_ECCCTL_BCH_16BIT              ((UINT32)0x00000000 << 15)
#define     EMCTL_ECCCTL_BCH_24BIT              ((UINT32)0x00000001 << 15)
#define     EMCTL_ECCCTL_BCH_40BIT              ((UINT32)0x00000002 << 15)
#define     EMCTL_ECCCTL_BCH_60BIT              ((UINT32)0x00000003 << 15)

#define     EMCTL_ECCCTL_BCHMODE                15

#define     EMCTL_RANDMC_ENABLE             ((UINT32)0x00000001 << 24)
#define     EMCTL_RANDMC_RESET              ((UINT32)0x00000001 << 23)
#define     EMCTL_RANDMC_SEED               ((UINT32)0x00567800)

/*
*-------------------------------------------------------------------------------
*
*  The following define the bit fields in the FMCtl register.
*
*-------------------------------------------------------------------------------
*/
#define     EMCTL_FMCTL_SEL_FLASH0          ((UINT32)0x00000001 << 0)
#define     EMCTL_FMCTL_SEL_FLASH1          ((UINT32)0x00000001 << 1)
#define     EMCTL_FMCTL_FLASH_WE            ((UINT32)0x00000001 << 2)
#define     EMCTL_FMCTL_FLASH_RYBY          ((UINT32)0x00000001 << 3)
#define     EMCTL_FMCTL_SEL_FLASH2          ((UINT32)0x00000001 << 4)
#define     EMCTL_FMCTL_SEL_FLASH3          ((UINT32)0x00000001 << 5)


/*
*-------------------------------------------------------------------------------
*
*  The following define the bit fields in the FMWait register.
*
*-------------------------------------------------------------------------------
*/
#define     EMCTL_FMWAIT_CSRW_CYCLE         ((UINT32)0x00000001 << 0)
#define     EMCTL_FMWAIT_RWPW_CYCLE         ((UINT32)0x00000001 << 5)
#define     EMCTL_FMWAIT_EXT_RDY            ((UINT32)0x00000001 << 11)
#define     EMCTL_FMWAIT_RWCS_CYCLE         ((UINT32)0x00000001 << 12)


/*
*-------------------------------------------------------------------------------
*
*  The following define the bit fields in the EMICtl register.
*
*-------------------------------------------------------------------------------
*/
#define     EMCTL_EMICTL_DATAOUT_VALID      ((UINT32)0x00000001 << 0)

/*
********************************************************************************
*
*                         End of hw_emctl.h
*
********************************************************************************
*/
#endif
