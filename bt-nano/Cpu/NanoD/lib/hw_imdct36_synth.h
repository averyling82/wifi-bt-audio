/*
********************************************************************************
*                   Copyright (c) 2008,Huweiguo
*                         All rights reserved.
*
* File Name£º   Hw_Imdct36_Synth.h
*
* Description:
*
*
* History:      <author>          <time>        <version>
*               HuWeiGuo        2009-01-05         1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _HW_IMDCT36_SYNTH_H_
#define _HW_IMDCT36_SYNTH_H_

#undef  EXT
#ifdef _IN_IMDCT36_SYNTH_
#define EXT
#else
#define EXT extern
#endif

/*
*-------------------------------------------------------------------------------
*
*                          Imdct & Synth memap register define
*
*-------------------------------------------------------------------------------
*/
#define IMDCT_ENABLE            ((uint32)1 << 0)
#define IMDCT_RIGHTSHIFT_MODE	((uint32)1 << 1)

#define SYNTH_ENABLE            ((uint32)1 << 0)

typedef volatile struct tagIMDCT_REG
{
    uint32 Ctrl;
    uint32 Stat;
    uint32 interrupt;
    uint32 eoi;

}IMDCT_REG, *pIMDCT_REG;
#define Imdct              ((IMDCT_REG*)(IMDCT_BASE + 0x800))
#define Imdct_Ram0         ((IMDCT_REG*)(IMDCT_BASE))
#define Imdct_Ram1         ((IMDCT_REG*)(IMDCT_BASE + 0x48))
#define IMDCT_RAM0_SIZE     18
#define IMDCT_RAM1_SIZE     36

typedef volatile struct tagSYNTH_REG
{
    uint32 Ctrl;
    uint32 Cfg;
    uint32 Stat;
    uint32 interrupt;
    uint32 eoi;

}SYNTH_REG, *pSYNTH_REG;
#define Synth              ((SYNTH_REG*)(SYNTH_BASE + 0x800))
#define Synth_Ram0         ((IMDCT_REG*)(SYNTH_BASE))
#define Synth_Ram1         ((IMDCT_REG*)(SYNTH_BASE + 0x1000))
#define SYNTH_RAM0_SIZE     32
#define SYNTH_RAM1_SIZE     1024

/*
********************************************************************************
*
*                         End of Hw_Imdct36_Synth.h
*
********************************************************************************
*/

#endif
