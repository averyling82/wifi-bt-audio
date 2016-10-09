/*
********************************************************************************
*                   Copyright (c) 2008,ZhengYongzhi
*                         All rights reserved.
*
* File Name£º   nvic.c
*
* Description:
*
* History:      <author>          <time>        <version>
*             ZhengYongzhi      2008-8-13          1.0
*    desc:    ORG.
********************************************************************************
*/
#define _IN_NVIC_
#include "typedef.h"
#include "DriverInclude.h"

/*
--------------------------------------------------------------------------------
  Function name : void NVICSystemReset(void)
  Author        : ZHengYongzhi
  Description   : Intended to force a large system reset of all major components
                  except for debug.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
void NvicSystemReset(void)
{
    nvic->APIntRst = NVIC_APINTRST_VECTKEY | NVIC_APINTRST_SYSRESETREQ;
}

/*
--------------------------------------------------------------------------------
  Function name : void NVICCpuReset(void)
  Author        : ZHengYongzhi
  Description   : CPU reset.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
void NvicCpuReset(void)
{
    nvic->APIntRst = NVIC_APINTRST_VECTKEY | NVIC_APINTRST_VECTRESET;
}

/*
--------------------------------------------------------------------------------
  Function name : void NVICClearActive(void)
  Author        : ZHengYongzhi
  Description   : Clear all state information for active NMI, fault, and interrupts
                  It is the responsibility of the application to reinitialize the
                  stack.
  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
void NvicClearActive(void)
{
    nvic->APIntRst = NVIC_APINTRST_VECTKEY | NVIC_APINTRST_VECTCLRACTIVE;
}

/*
--------------------------------------------------------------------------------
  Function name : void NVICClearActive(void)
  Author        : ZHengYongzhi
  Description   : Clear all state information for active NMI, fault, and interrupts
                  It is the responsibility of the application to reinitialize the
                  stack.
  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
void NvicVectorTableSet(uint32 vectortaboffset)
{
    nvic->VectorTableOffset = vectortaboffset;
}

/*
********************************************************************************
*
*                         End of nvic.c
*
********************************************************************************
*/
