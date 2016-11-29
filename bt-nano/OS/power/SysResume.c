/*
********************************************************************************************
*
*  Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                         All rights reserved.
*
* FileName: \Common\System\SysService\SysResume.c
* Owner: aaron.sun
* Date: 2014.8.29
* Time: 16:34:29
* Desc: system resume
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2014.8.29     16:34:29   1.0
********************************************************************************************
*/

#include "BspConfig.h"
#ifdef ___COMMON_SYSTEM_SYSSERVICE_SYSRESUME_C__

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
#include "FwAnalysis.h"


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#define _SYSTEM_SYSSEVER_SYSRESUME_READ_  __attribute__((section("system_syssever_sysresume_read")))
#define _SYSTEM_SYSSEVER_SYSRESUME_WRITE_ __attribute__((section("system_syssever_sysresume_write")))
#define _SYSTEM_SYSSEVER_SYSRESUME_INIT_  __attribute__((section("system_syssever_sysresume_init")))
#define _SYSTEM_SYSSEVER_SYSRESUME_SHELL_  __attribute__((section("system_syssever_sysresume_shell")))
#if defined(__arm__) && defined(__ARMCC_VERSION)
#define  _SYSTEM_SYSSEVER_SYSRESUME_SHELL_DATA_         _SYSTEM_SYSSEVER_SYSRESUME_SHELL_
#elif defined(__arm__) && defined(__GNUC__)
#define _SYSTEM_SYSSEVER_SYSRESUME_SHELL_DATA_  __attribute__((section("system_syssever_sysresume_shell_data")))
#else
#error Unknown compiling tools.
#endif

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
uint32 SysState;
uint32 SysWakeCnt;
uint32 PrevSysWakeCnt;
uint32 PrevSysTickCounter;


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
rk_err_t ShellSysResumeBspSysResume(HDC dev, uint8 * pstr);
rk_err_t ShellSysResumeBspElect(HDC dev, uint8 * pstr);
rk_err_t ShellSysResumeBspMaskRom(HDC dev, uint8 * pstr);
rk_err_t ShellSysResumeBspReset(HDC dev, uint8 * pstr);
rk_err_t ShellSysResumeBspHelp(HDC dev, uint8 * pstr);
rk_err_t SysResumeShellHelp(HDC dev, uint8 * pstr);
rk_err_t SysResumeShellBsp(HDC dev, uint8 * pstr);
rk_err_t SysResumeShellTest(HDC dev, uint8 * pstr);
rk_err_t SysResumeShellDel(HDC dev, uint8 * pstr);
rk_err_t SysResumeShellMc(HDC dev, uint8 * pstr);
rk_err_t SysResumeShellPcb(HDC dev, uint8 * pstr);
rk_err_t ReadResource(uint32 Addr, uint8 *pData, uint16 length);
rk_err_t DataResume(void);
rk_err_t CodeResume(void);
rk_err_t SpiNorRead(uint32 LBA,uint32 len, uint8 * buf);
rk_err_t SpiWrite(uint8 * buf, uint32 size);
rk_err_t SpiRead(uint8 * buf, uint32 size);
uint32 SpiClkResume(uint32 nMhz);
void GpioResume(eGPIO_CHANNEL gpio_port);
void MemResume(void);
rk_err_t SysTickPeriodResume(uint32 mstick);
rk_err_t PllResume(UINT32 nMHz);
rk_err_t SysFreqResume(uint32 nMhz);
rk_err_t PmuSysResume(uint64 appList);
rk_err_t SpiResume(void);



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(read) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: SysResume
** Input:void
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.8.29
** Time: 16:41:24
*******************************************************************************/
_SYSTEM_SYSSEVER_SYSRESUME_READ_

extern uint64  g_APPList;

COMMON API rk_err_t SysResume(uint32 Level)
{
    #ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    FW_RemoveSegment(SEGMENT_ID_RESUME);
    #endif

    __WFI();
    //PmuSysResume(g_APPList);
    //SpiResume();
    //CodeResume();
    SysTickEnable();

}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(read) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: ReadResource
** Input:uint32 Addr, uint8 *pData, uint16 length
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.9.1
** Time: 15:37:37
*******************************************************************************/
_SYSTEM_SYSSEVER_SYSRESUME_READ_
COMMON FUN rk_err_t ReadResource(uint32 Addr, uint8 *pData, uint16 length)
{
    uint32 sectorNum;
    uint32 sectorOffset;
    uint32 count;
    uint32 i;
    uint8  FlashBuf[512];
    uint8 *pbuf = pData;
    sectorNum = Addr >> 9;
    sectorOffset = Addr & 511;
    while(length)
    {
        if (sectorOffset != 0 || length < 512)
        {
            SpiNorRead(sectorNum,1,FlashBuf);

            count = 512-sectorOffset;
            count = (count>length)? length : count;
            for (i = sectorOffset; i< (count+sectorOffset); i++)
            {
                *pbuf++ = FlashBuf[i];
            }
            length -= count;
            sectorOffset = 0;
        }
        else
        {
            SpiNorRead(sectorNum, 1, pbuf); //read 512 byte
            pbuf +=  512;
            length -=  512;
        }
        sectorNum++;

    }

}

/*******************************************************************************
** Name: DataResume
** Input:void
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.9.1
** Time: 11:20:10
*******************************************************************************/
_SYSTEM_SYSSEVER_SYSRESUME_READ_
COMMON FUN rk_err_t DataResume(void)
{

}

#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
/*******************************************************************************
** Name: CodeResume
** Input:void
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.9.1
** Time: 11:19:27
*******************************************************************************/
_SYSTEM_SYSSEVER_SYSRESUME_READ_
COMMON FUN rk_err_t CodeResume(void)
{
    uint32 CodeInfoAddr;
    uint8  FlashBuf[512];
    SEGMENT_INFO_T * pModule;

    rk_err_t ret;
    FIRMWARE_INFO_T * pFirmwareModuleInfo;

    D_SEGMENT_INFO * pDmodule;


    uint32 LoadBase;
    uint32 ImageBase;
    uint32 ImageLength;

    pDmodule = gpstDsegment;

    while(pDmodule != NULL)
    {
        //printf("module ID = %d\n", pDmodule->SegmentID);

        LoadBase    = pDmodule->CodeLoadBase;
        ImageBase   = pDmodule->CodeImageBase;
        ImageLength = pDmodule->CodeImageLen;
        ReadResource(LoadBase, (uint8 *)ImageBase, ImageLength);

        pDmodule = pDmodule->pstDsegment;
    }

    return RK_SUCCESS;
}
#endif

/*******************************************************************************
** Name: SpiNorRead
** Input:uint32 LBA
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.9.1
** Time: 11:17:25
*******************************************************************************/
#define CMD_READ_DATA           (0x03)

_SYSTEM_SYSSEVER_SYSRESUME_READ_
COMMON FUN rk_err_t SpiNorRead(uint32 LBA,uint32 len, uint8 * buf)
{
    uint8 cmd[4];
    uint32 Addr;

    Addr = LBA << 9;
    cmd[0] = CMD_READ_DATA;
    cmd[1] = (Addr >> 16) & 0xFF;
    cmd[2] = (Addr >> 8) & 0xFF;
    cmd[3] = (Addr & 0xFF);

    SPICtl->SPI_SER  |= ((uint32)(0x01) << 1);

    SpiWrite(cmd, 4);

    SpiRead(buf, len * 512);

    SPICtl->SPI_SER  &= ~((uint32)(0x01) << 1);

    return RK_SUCCESS;

}

/*******************************************************************************
** Name: SpiWrite
** Input:uint8 * buf, uint32 size
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.9.1
** Time: 11:16:43
*******************************************************************************/
_SYSTEM_SYSSEVER_SYSRESUME_READ_
COMMON FUN rk_err_t SpiWrite(uint8 * buf, uint32 size)
{
    SPICtl->SPI_ENR = SPI_DISABLE;
    SPICtl->SPI_CTRLR0 &= ~(TRANSMIT_RECEIVE_MASK);
    SPICtl->SPI_CTRLR0 |= TRANSMIT_ONLY;
    SPICtl->SPI_ENR = SPI_ENABLE;

    while (size)
    {
       if ((SPICtl->SPI_SR & TRANSMIT_FIFO_FULL) != TRANSMIT_FIFO_FULL)
       {
           SPICtl->SPI_TXDR[0] = *buf++;
           size--;
       }
    }

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: SpiRead
** Input:uint8 * buf, uint32 size
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.9.1
** Time: 11:15:38
*******************************************************************************/
_SYSTEM_SYSSEVER_SYSRESUME_READ_
COMMON FUN rk_err_t SpiRead(uint8 * buf, uint32 size)
{
     SPICtl->SPI_ENR = SPI_DISABLE;
     SPICtl->SPI_CTRLR0 &= ~(TRANSMIT_RECEIVE_MASK);
     SPICtl->SPI_CTRLR0 |= RECEIVE_ONLY;
     SPICtl->SPI_CTRLR1 = size - 1;
     SPICtl->SPI_ENR = SPI_ENABLE;

     while (size)
     {
         if ((SPICtl->SPI_SR & RECEIVE_FIFO_EMPTY) != RECEIVE_FIFO_EMPTY)
         {
             *buf++ = (uint8)(SPICtl->SPI_RXDR[0] & 0xFF);
             size--;
         }
     }

    return RK_SUCCESS;


}


/*******************************************************************************
** Name: GpioResume
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2014.9.1
** Time: 10:45:21
*******************************************************************************/
_SYSTEM_SYSSEVER_SYSRESUME_READ_
COMMON FUN void GpioResume(eGPIO_CHANNEL gpio_port)
{
    Grf_GpioMuxSet(gpio_port,GPIOPortA_Pin0,IOMUX_GPIO0A0_IO);
    Grf_GpioMuxSet(gpio_port,GPIOPortA_Pin1,IOMUX_GPIO0A1_IO);
    Grf_GpioMuxSet(gpio_port,GPIOPortA_Pin2,IOMUX_GPIO0A2_IO);
    Grf_GpioMuxSet(gpio_port,GPIOPortA_Pin3,IOMUX_GPIO0A3_IO);
    Grf_GpioMuxSet(gpio_port,GPIOPortA_Pin4,IOMUX_GPIO0A4_IO);
    Grf_GpioMuxSet(gpio_port,GPIOPortA_Pin5,IOMUX_GPIO0A5_IO);
    Grf_GpioMuxSet(gpio_port,GPIOPortA_Pin6,IOMUX_GPIO0A6_IO);
    Grf_GpioMuxSet(gpio_port,GPIOPortA_Pin7,IOMUX_GPIO0A7_IO);

    Grf_GpioMuxSet(gpio_port,GPIOPortB_Pin0,IOMUX_GPIO0B0_IO);
    Grf_GpioMuxSet(gpio_port,GPIOPortB_Pin1,IOMUX_GPIO0B1_IO);
    Grf_GpioMuxSet(gpio_port,GPIOPortB_Pin2,IOMUX_GPIO0B2_IO);
    Grf_GpioMuxSet(gpio_port,GPIOPortB_Pin3,IOMUX_GPIO0B3_IO);
    Grf_GpioMuxSet(gpio_port,GPIOPortB_Pin4,IOMUX_GPIO0B4_IO);
    Grf_GpioMuxSet(gpio_port,GPIOPortB_Pin5,IOMUX_GPIO0B5_IO);
    Grf_GpioMuxSet(gpio_port,GPIOPortB_Pin6,IOMUX_GPIO0B6_IO);
    Grf_GpioMuxSet(gpio_port,GPIOPortB_Pin7,IOMUX_GPIO0B7_IO);

    Grf_GpioMuxSet(gpio_port,GPIOPortC_Pin0,IOMUX_GPIO0C0_IO);
    Grf_GpioMuxSet(gpio_port,GPIOPortC_Pin1,IOMUX_GPIO0C1_IO);
    Grf_GpioMuxSet(gpio_port,GPIOPortC_Pin2,IOMUX_GPIO0C2_IO);
    Grf_GpioMuxSet(gpio_port,GPIOPortC_Pin3,IOMUX_GPIO0C3_IO);
    Grf_GpioMuxSet(gpio_port,GPIOPortC_Pin4,IOMUX_GPIO0C4_IO);
    Grf_GpioMuxSet(gpio_port,GPIOPortC_Pin5,IOMUX_GPIO0C5_IO);
    Grf_GpioMuxSet(gpio_port,GPIOPortC_Pin6,IOMUX_GPIO0C6_IO);
    Grf_GpioMuxSet(gpio_port,GPIOPortC_Pin7,IOMUX_GPIO0C7_IO);

    Grf_GpioMuxSet(gpio_port,GPIOPortD_Pin0,IOMUX_GPIO0D0_IO);
    Grf_GpioMuxSet(gpio_port,GPIOPortD_Pin1,IOMUX_GPIO0D1_IO);


    Gpio_SetPinDirection(gpio_port,GPIOPortA_Pin0,GPIO_IN);
    Gpio_SetPinDirection(gpio_port,GPIOPortA_Pin1,GPIO_IN);
    Gpio_SetPinDirection(gpio_port,GPIOPortA_Pin2,GPIO_IN);
    Gpio_SetPinDirection(gpio_port,GPIOPortA_Pin3,GPIO_IN);
    Gpio_SetPinDirection(gpio_port,GPIOPortA_Pin4,GPIO_IN);
    Gpio_SetPinDirection(gpio_port,GPIOPortA_Pin5,GPIO_IN);
    Gpio_SetPinDirection(gpio_port,GPIOPortA_Pin6,GPIO_IN);
    Gpio_SetPinDirection(gpio_port,GPIOPortA_Pin7,GPIO_IN);

    Gpio_SetPinDirection(gpio_port,GPIOPortB_Pin0,GPIO_IN);
    Gpio_SetPinDirection(gpio_port,GPIOPortB_Pin1,GPIO_IN);
    Gpio_SetPinDirection(gpio_port,GPIOPortB_Pin2,GPIO_IN);
    Gpio_SetPinDirection(gpio_port,GPIOPortB_Pin3,GPIO_IN);
    Gpio_SetPinDirection(gpio_port,GPIOPortB_Pin4,GPIO_IN);
    Gpio_SetPinDirection(gpio_port,GPIOPortB_Pin5,GPIO_IN);
    Gpio_SetPinDirection(gpio_port,GPIOPortB_Pin6,GPIO_IN);
    Gpio_SetPinDirection(gpio_port,GPIOPortB_Pin7,GPIO_IN);

    Gpio_SetPinDirection(gpio_port,GPIOPortC_Pin0,GPIO_IN);
    Gpio_SetPinDirection(gpio_port,GPIOPortC_Pin1,GPIO_IN);
    Gpio_SetPinDirection(gpio_port,GPIOPortC_Pin2,GPIO_IN);
    Gpio_SetPinDirection(gpio_port,GPIOPortC_Pin3,GPIO_IN);
    Gpio_SetPinDirection(gpio_port,GPIOPortC_Pin4,GPIO_IN);
    Gpio_SetPinDirection(gpio_port,GPIOPortC_Pin5,GPIO_IN);
    Gpio_SetPinDirection(gpio_port,GPIOPortC_Pin6,GPIO_IN);
    Gpio_SetPinDirection(gpio_port,GPIOPortC_Pin7,GPIO_IN);

    Gpio_SetPinDirection(gpio_port,GPIOPortD_Pin0,GPIO_IN);
    Gpio_SetPinDirection(gpio_port,GPIOPortD_Pin1,GPIO_IN);
    Gpio_SetPinDirection(gpio_port,GPIOPortD_Pin2,GPIO_IN);
    Gpio_SetPinDirection(gpio_port,GPIOPortD_Pin3,GPIO_IN);

}


/*******************************************************************************
** Name: MemResume
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2014.9.1
** Time: 10:44:24
*******************************************************************************/
_SYSTEM_SYSSEVER_SYSRESUME_READ_
COMMON FUN void MemResume(void)
{

}
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(write) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(write) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



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

#ifdef _SYS_BOOT_SHELL_

_SYSTEM_SYSSEVER_SYSRESUME_SHELL_DATA_
static SHELL_CMD ShellSysResumeName[] =
{
    "pcb",SysResumeShellPcb,"NULL","NULL",
    "mc",SysResumeShellMc,"NULL","NULL",
    "del",SysResumeShellDel,"NULL","NULL",
    "test",SysResumeShellTest,"NULL","NULL",
#ifdef SHELL_BSP
    "bsp",SysResumeShellBsp,"NULL","NULL",
#endif
    "\b",NULL,"NULL","NULL",
};

#ifdef SHELL_BSP
_SYSTEM_SYSSEVER_SYSRESUME_SHELL_DATA_
static SHELL_CMD ShellSysResumeBspName[] =
{
    "reset",ShellSysResumeBspReset,"NULL","NULL",
    "maskrom",ShellSysResumeBspMaskRom,"NULL","NULL",
    "elect",ShellSysResumeBspElect,"NULL","NULL",
    "sysresume",ShellSysResumeBspSysResume,"NULL","NULL",
    "\b",NULL,"NULL","NULL",
};

#endif

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: SysResume_Shell
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.10
** Time: 9:26:47
*******************************************************************************/
_SYSTEM_SYSSEVER_SYSRESUME_SHELL_
SHELL API rk_err_t SysResume_Shell(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret = RK_SUCCESS;

    uint8 Space;

    if(ShellHelpSampleDesDisplay(dev, ShellSysResumeName, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }


    StrCnt = ShellItemExtract(pstr, &pItem, &Space);

    if ((StrCnt == 0) || (*(pstr - 1) != '.'))
    {
        return RK_ERROR;
    }


    ret = ShellCheckCmd(ShellSysResumeName, pItem, StrCnt);
    if(ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;                      //remove '.',the point is the useful item

    ShellHelpDesDisplay(dev, ShellSysResumeName[i].CmdDes, pItem);
    if(ShellSysResumeName[i].ShellCmdParaseFun != NULL)
    {
        ret = ShellSysResumeName[i].ShellCmdParaseFun(dev, pItem);
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
#ifdef SHELL_BSP
/*******************************************************************************
** Name: ShellSysResumeBspSysResume
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.11
** Time: 11:49:52
*******************************************************************************/
_SYSTEM_SYSSEVER_SYSRESUME_SHELL_
SHELL FUN rk_err_t ShellSysResumeBspSysResume(HDC dev, uint8 * pstr)
{
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: ShellSysResumeBspElect
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.11
** Time: 11:49:26
*******************************************************************************/
_SYSTEM_SYSSEVER_SYSRESUME_SHELL_
SHELL FUN rk_err_t ShellSysResumeBspElect(HDC dev, uint8 * pstr)
{
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: ShellSysResumeBspMaskRom
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.11
** Time: 11:48:59
*******************************************************************************/
_SYSTEM_SYSSEVER_SYSRESUME_SHELL_
SHELL FUN rk_err_t ShellSysResumeBspMaskRom(HDC dev, uint8 * pstr)
{
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: ShellSysResumeBspReset
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.11
** Time: 11:48:34
*******************************************************************************/
_SYSTEM_SYSSEVER_SYSRESUME_SHELL_
SHELL FUN rk_err_t ShellSysResumeBspReset(HDC dev, uint8 * pstr)
{
    return RK_SUCCESS;
}


/*******************************************************************************
** Name: SysResumeShellBsp
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.10
** Time: 17:51:29
*******************************************************************************/
_SYSTEM_SYSSEVER_SYSRESUME_SHELL_
SHELL FUN rk_err_t SysResumeShellBsp(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret = RK_SUCCESS;

    uint8 Space;

    if(ShellHelpSampleDesDisplay(dev, ShellSysResumeBspName, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }


    StrCnt = ShellItemExtract(pstr, &pItem, &Space);

    if ((StrCnt == 0) || (*(pstr - 1) != '.'))
    {
        return RK_ERROR;
    }


    ret = ShellCheckCmd(ShellSysResumeBspName, pItem, StrCnt);
    if(ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;                      //remove '.',the point is the useful item

    ShellHelpDesDisplay(dev, ShellSysResumeBspName[i].CmdDes, pItem);
    if(ShellSysResumeBspName[i].ShellCmdParaseFun != NULL)
    {
        ret = ShellSysResumeBspName[i].ShellCmdParaseFun(dev, pItem);
    }

    return ret;
}
#endif


/*******************************************************************************
** Name: SysResumeShellTest
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.10
** Time: 17:50:51
*******************************************************************************/
_SYSTEM_SYSSEVER_SYSRESUME_SHELL_
SHELL FUN rk_err_t SysResumeShellTest(HDC dev, uint8 * pstr)
{
    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }
    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: SysResumeShellDel
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.10
** Time: 17:50:03
*******************************************************************************/
_SYSTEM_SYSSEVER_SYSRESUME_SHELL_
SHELL FUN rk_err_t SysResumeShellDel(HDC dev, uint8 * pstr)
{
    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }
    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: SysResumeShellMc
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.10
** Time: 17:49:25
*******************************************************************************/
_SYSTEM_SYSSEVER_SYSRESUME_SHELL_
SHELL FUN rk_err_t SysResumeShellMc(HDC dev, uint8 * pstr)
{
    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }
    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: SysResumeShellPcb
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.10
** Time: 17:48:50
*******************************************************************************/
_SYSTEM_SYSSEVER_SYSRESUME_SHELL_
SHELL FUN rk_err_t SysResumeShellPcb(HDC dev, uint8 * pstr)
{
    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }
    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }
    return RK_SUCCESS;
}
#endif


#endif
