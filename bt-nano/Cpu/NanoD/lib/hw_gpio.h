/*
********************************************************************************
*                   Copyright (c) 2015 Rockchips
*                         All rights reserved.
*
* File Name£º  Hw_Gpio.h
*
* Description:
*
* History:      <author>          <time>        <version>
*    desc:    ORG.
********************************************************************************
*/

#ifndef _HW_GPIO_H_
#define _HW_GPIO_H_

#undef  EXT
#ifdef  _IN_GPIO_
#define EXT
#else
#define EXT extern
#endif

/*
--------------------------------------------------------------------------------

      GPIO register define

--------------------------------------------------------------------------------
*/

typedef volatile struct {

    UINT32 PortData;
    UINT32 PortDir;
    UINT32 Pad0[(0x30 - 0x04)/4 -1];
    UINT32 PortIntEnable;
    UINT32 PortIntMask;
    UINT32 PortIntType;    //edge or level trigger.
    UINT32 PortIntPolarity;//high level is valid or low level valid.
    UINT32 PortIntStatus;
    UINT32 PortIntRawStatus;

    UINT32 DebounceEnable;
    UINT32 PortIntClear;
    UINT32 ExtPort;
    UINT32 Pad1[(0x60 -0x50)/4 -1];
    UINT32 LevelSensSync;

} GPIO_t,*gGPIO_t;

/*
********************************************************************************
*
*                         End of hw_gpio.h
*
********************************************************************************
*/
#endif
