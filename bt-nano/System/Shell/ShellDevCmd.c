/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: System\Shell\ShellDevCmd.c
* Owner: aaron.sun
* Date: 2015.10.20
* Time: 11:36:04
* Version: 1.0
* Desc: shell device cmd
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2015.10.20     11:36:04   1.0
********************************************************************************************
*/


#include "BspConfig.h"
#ifdef __SYSTEM_SHELL_SHELLDEVCMD_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "rkos.h"
#include "Bsp.h"
#include "ShellDevCmd.h"

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
*                                                   local function declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
rk_err_t ShellDeviceList(HDC dev, uint8 * pstr);


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static SHELL_CMD ShellDeviceName[] =
{
    "list",ShellDeviceList,"list all aready create ok device","NULL",
    "add",NULL,"NULL","NULL",
    "rem",NULL,"NULL","NULL",
    "\b",NULL,"NULL","NULL",
};

static uint8* ShellDevState[] =
{
    "working   ",
    "idle1     ",
    "idle2     ",
    "err       ",
    "\b",
};

static uint8* ShellSuspednMode[] =
{
    "DISABLE  ",
    "ENABLE   ",
    "FORCE    ",
    "\b",
};

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
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: ShellDeviceParsing
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.10.20
** Time: 11:49:44
*******************************************************************************/
_SYSTEM_SHELL_SHELLDEVCMD_COMMON_
COMMON API rk_err_t ShellDeviceParsing(HDC dev, uint8 * pstr)
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

    ret = ShellCheckCmd(ShellDeviceName, pItem, StrCnt);

    if (ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;

    ShellHelpDesDisplay(dev, ShellDeviceName[i].CmdDes, pItem);
    if(ShellDeviceName[i].ShellCmdParaseFun != NULL)
    {
        ret = ShellDeviceName[i].ShellCmdParaseFun(dev, pItem);
    }

    return ret;

}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: ShellDeviceList
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.4.12
** Time: 14:41:20
*******************************************************************************/
_SYSTEM_SHELL_SHELLDEVCMD_COMMON_
COMMON FUN rk_err_t ShellDeviceList(HDC dev, uint8 * pstr)
{
    DEVICE_CLASS * TempDevHandler;
    uint8  TxBuffer[128];
    uint8  TaskState;
    uint32 TaskRemainStack;
    uint16 BuffSize = 0;
    uint32 i, j;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    TempDevHandler = RKDev_GetFirstHandler(0xffffffff);

    rk_print_string("\n this cmd will list all device info!!\r\n\r\n");

    if (TempDevHandler == NULL)
    {
        rk_print_string("not find device!\r\n\r\n");
        return RK_SUCCESS;
    }

    BuffSize = sprintf(TxBuffer,"State\tIdleTick\tSuspendMode\tIdle2Time\tIdle3Time\tUserCnt\tSuspenderCnt\tClassID\tObjectID\tName\r\n");
    UartDev_Write(dev,TxBuffer,BuffSize,SYNC_MODE,NULL);
    BuffSize = sprintf(TxBuffer,"---------------------------------------------------------------------------------------------------\r\n");
    UartDev_Write(dev,TxBuffer,BuffSize,SYNC_MODE,NULL);

    while (TempDevHandler != NULL)
    {

        BuffSize = sprintf(TxBuffer,"%s ",ShellDevState[TempDevHandler->State]);
        UartDev_Write(dev,TxBuffer,BuffSize,SYNC_MODE,NULL);

        BuffSize = sprintf(TxBuffer,"%d\t",TempDevHandler->IdleTick);
        UartDev_Write(dev,TxBuffer,BuffSize,SYNC_MODE,NULL);

        BuffSize = sprintf(TxBuffer,"%s ",ShellSuspednMode[TempDevHandler->SuspendMode]);
        UartDev_Write(dev,TxBuffer,BuffSize,SYNC_MODE,NULL);

        BuffSize = sprintf(TxBuffer,"%d\t",TempDevHandler->Idle1EventTime);
        UartDev_Write(dev,TxBuffer,BuffSize,SYNC_MODE,NULL);

        BuffSize = sprintf(TxBuffer,"%d\t",TempDevHandler->Idle2EventTime);
        UartDev_Write(dev,TxBuffer,BuffSize,SYNC_MODE,NULL);

        BuffSize = sprintf(TxBuffer,"%d\t",TempDevHandler->UseCnt);
        UartDev_Write(dev,TxBuffer,BuffSize,SYNC_MODE,NULL);

        BuffSize = sprintf(TxBuffer,"%d\t",TempDevHandler->SuspendCnt);
        UartDev_Write(dev,TxBuffer,BuffSize,SYNC_MODE,NULL);

        BuffSize = sprintf(TxBuffer,"%d\t",TempDevHandler->DevClassID);
        UartDev_Write(dev,TxBuffer,BuffSize,SYNC_MODE,NULL);

        BuffSize = sprintf(TxBuffer,"%d\t",TempDevHandler->DevID);
        UartDev_Write(dev,TxBuffer,BuffSize,SYNC_MODE,NULL);

        BuffSize = sprintf(TxBuffer,"%s\r\n",DevInfo_Table[TempDevHandler->DevClassID].DeviceName);

        UartDev_Write(dev,TxBuffer,BuffSize,SYNC_MODE,NULL);

        TaskState = 0;
        TaskRemainStack = 0;

        TempDevHandler = RKDev_GetNexttHandler(TempDevHandler, 0xffffffff);

    }

    rk_printf("total dev cnt = %d, total suspend dev cnt = %d", DevTotalCnt, DevTotalSuspendCnt);

    {
        uint32 * pList;
        rk_print_string("\n this cmd will list all device list info!!\r\n\r\n");
        //rk_printf("DeviceList = %x", DeviceList);

        pList = DeviceList;

        uint32 top;

        for(i = 0; i < (DEVICE_LIST_NUM + MAX_D_DEV_LIST); i++)
        {
            BuffSize = 0;
            top = 0;

            for(j = 0; j < (MAX_DEV_PER_LIST + 1); j++)
            {
                //rk_printf("DeviceList[%d][%d] = %x, %x",  i, j, DeviceList, DeviceList[j]);

                if(pList[j] == 0)
                {
                    break;
                }

                if( (((pList[j] >> 24) & 0x000000ff) == ((pList[j - 1] >> 24) & 0x000000ff))
                    && (((pList[j] >> 16) & 0x000000ff) == 0)
                    && (((pList[j] >> 8) & 0x000000ff) == ((pList[j - 1] >> 8) & 0x000000ff)))
                {
                        BuffSize += sprintf(TxBuffer + BuffSize,",%d", pList[j] & 0x000000ff);
                }
                else
                {
                     BuffSize += sprintf(TxBuffer + BuffSize,"%s[%d",
                     DevInfo_Table[(pList[j] >> 8) & 0x000000ff].DeviceName,
                     pList[j] & 0x000000ff);
                }

                if(pList[j + 1] == 0)
                {
                    BuffSize += sprintf(TxBuffer + BuffSize,"]");
                }
                else if(((pList[j] >> 24) & 0x000000ff) == (j+ 1))
                {
                    BuffSize += sprintf(TxBuffer + BuffSize,"]->");
                    top = 1;
                }
                else if((((pList[j] >> 24) & 0x000000ff) != ((pList[j + 1] >> 24) & 0x000000ff))
                         && (((pList[j] >> 16) & 0x000000ff) != (j + 1)))
                {
                    if(top == 1)
                    {
                        BuffSize += sprintf(TxBuffer + BuffSize,"],");
                    }
                    else
                    {
                        BuffSize += sprintf(TxBuffer + BuffSize,"] ");
                    }

                    top = 0;
                }
                else if(((pList[j] >> 24) & 0x000000ff) == ((pList[j + 1] >> 24) & 0x000000ff))
                {
                    if(((pList[j] >> 16) & 0x000000ff) == (j + 1))
                    {
                        BuffSize += sprintf(TxBuffer + BuffSize,"],");
                    }
                    else if((((pList[j] >> 16) & 0x000000ff) == 0) && (((pList[j] >> 8) & 0x000000ff) == ((pList[j + 1] >> 8) & 0x000000ff)))
                    {

                    }
                    else
                    {
                        BuffSize += sprintf(TxBuffer + BuffSize,"] ");
                    }

                }
                else
                {
                    BuffSize += sprintf(TxBuffer + BuffSize,"] ");
                }
            }

            BuffSize += sprintf(TxBuffer + BuffSize,"\r\n");

            if(j)
            {
                UartDev_Write(dev,TxBuffer,BuffSize,SYNC_MODE,NULL);
            }

            pList += (MAX_DEV_PER_LIST + 1);
        }
    }

    return RK_SUCCESS;

}
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
