/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: System\Shell\ShellTaskCmd.c
* Owner: aaron.sun
* Date: 2015.7.24
* Time: 11:13:29
* Version: 1.0
* Desc: task shell cmd
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2015.7.24     11:13:29   1.0
********************************************************************************************
*/


#include "BspConfig.h"
#ifdef __SYSTEM_SHELL_SHELLTASKCMD_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "RKOS.h"
#include "BSP.h"
#include "ShellTaskCmd.h"


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
void memory_leak_check_watch(uint32 oder);
rk_err_t ShellMemoryLeakWatch(HDC dev, char * pstr);
rk_err_t ShellTaskLook(HDC dev, uint8 * pstr);
rk_err_t ShellTaskParsing(HDC dev, uint8 * pstr);
rk_err_t ShellTaskList(HDC dev, uint8 * pstr);



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
_SYSTEM_SHELL_SHELLTASKCMD_COMMON_
static SHELL_CMD ShellTaskName[] =
{
    "list",ShellTaskList,"list current all thread","task.list",
#ifdef _MEMORY_LEAK_CHECH_
    "lw",ShellMemoryLeakWatch,"list syste heap user","task.lw [/s | /a]",
#endif
    "\b",NULL,"NULL","NULL",
};

static uint8* ShellTaskState[] =
{
    "Runing ",
    "Ready  ",
    "Blocked",
    "Suspend",
    "Deleted",
    "Wroking",
    "Idle1  ",
    "Ilde2  ",
    "Error  "
    "\b"
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
** Name: GetRKTaskName
** Input:void * pcb,  uint8 * taskname
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.11.6
** Time: 14:40:15
*******************************************************************************/
_SYSTEM_SHELL_SHELLTASKCMD_COMMON_
COMMON API rk_err_t GetRKTaskName(void * pcb, uint8 * name)
{
    if((uint32)pcb == NULL)
    {
        return snprintf(name, 18, "%s\r\n", "Deleted");
    }
    else
    {
        return snprintf(name,18, "%s\r\n", pcTaskGetTaskName(pcb));
    }
}

/*******************************************************************************
** Name: ShellTaskParsing
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.7.24
** Time: 11:17:38
*******************************************************************************/
_SYSTEM_SHELL_SHELLTASKCMD_COMMON_
COMMON API rk_err_t ShellTaskParsing(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret =  RK_SUCCESS;

    uint8 Space;

    if(ShellHelpSampleDesDisplay(dev, ShellTaskName, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }


    StrCnt = ShellItemExtract(pstr,&pItem, &Space);

    if ((StrCnt == 0) || (*(pstr - 1) != '.'))
    {
        return RK_ERROR;
    }

    ret = ShellCheckCmd(ShellTaskName, pItem, StrCnt);
    if (ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;

    ShellHelpDesDisplay(dev, ShellTaskName[i].CmdDes, pItem);
    if(ShellTaskName[i].ShellCmdParaseFun != NULL)
    {
        ret = ShellTaskName[i].ShellCmdParaseFun(dev, pItem);
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
typedef  struct _MEMORY_BLOCK_LIST
{
    struct _MEMORY_BLOCK_LIST * pNext;
    uint32 hTask;
    uint32 addr;
    uint32 size;
    uint32 flag;
    uint32 flag1;

}MEMORY_BLOCK_LIST;

extern const size_t xTotalHeapSize[2];
extern const size_t xHeapFirstAddr[2];

/*******************************************************************************
** Name: memory_leak_check_watch
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.11.8
** Time: 16:11:46
*******************************************************************************/
_SYSTEM_SHELL_SHELLTASKCMD_COMMON_
COMMON FUN void memory_leak_check_watch(uint32 oder)
{
    MEMORY_BLOCK_LIST * pPrev, * pCur, *pFirst;
    uint16 BuffSize = 0;
    uint8 buf[512];
    uint32 totalsize, totalsize1, remainsize, remainsize1, totalCnt;
    MEMORY_BLOCK * p;
    uint32 i, j, err;
    uint8 * pbuf;

    totalsize = 0;
    remainsize = 0;
    i = 0;

    //search
    rkos_enter_critical();

    totalCnt = UsedMemoryCnt + 1;

    pbuf = rkos_memory_malloc(totalCnt * sizeof(MEMORY_BLOCK_LIST));

    remainsize1 = RKTaskHeapFree();
    totalsize1 = RKTaskHeapTotal() - RKTaskHeapFree();

    if(pbuf == NULL)
    {
        rk_printf("memory report maloc fail");
        rk_printf_no_time("    total block cnt = %d, check block cnt = %d, totalsize = %d, check totalsize = %d,  remain = %d, check remain = %d", totalCnt, i, totalsize1,totalsize, remainsize1, remainsize);
        return;
    }

    pCur = (MEMORY_BLOCK_LIST *)pbuf;
    pFirst = pCur;

    pCur->pNext = NULL;

    for(p = pFirstMemoryBlock; p != NULL; p = p->pNext)
    {
        pCur->pNext = pCur + 1;
        pCur->hTask = p->hTask;
        pCur->addr = (uint32)p;
        pCur->flag = p->flag;
        pCur->flag1 = *(uint32*)((uint8 *)p + p->size + sizeof(MEMORY_BLOCK));
        pCur->size = p->size;
        pCur++;
    }

    pCur--;
    pCur->pNext = NULL;

    rkos_exit_critical();

    //sort
    if(oder != 0)
    {
        for(i = (totalCnt - 1); i > 0;  i--)
        {
            pCur = pFirst;
            pPrev = NULL;
            j = 0;

            while((pCur != NULL) && (pCur->pNext != NULL))
            {
                if(oder == 1)
                {
                    if(pCur->size > pCur->pNext->size)
                    {
                        if(pPrev == NULL)
                        {
                            pFirst = pCur->pNext;
                            pCur->pNext = pCur->pNext->pNext;
                            pFirst->pNext = pCur;
                            pPrev = pFirst;
                        }
                        else
                        {
                            pPrev->pNext = pCur->pNext;
                            pCur->pNext = pCur->pNext->pNext;
                            pPrev->pNext->pNext = pCur;
                            pPrev = pPrev->pNext;
                        }

                    }
                    else
                    {
                          pPrev = pCur;
                          pCur = pCur->pNext;
                    }
                }
                else if(oder == 2)
                {
                    if(pCur > pCur->pNext)
                    {
                        if(pPrev == NULL)
                        {
                            pFirst = pCur->pNext;
                            pCur->pNext = pCur->pNext->pNext;
                            pFirst->pNext = pCur;
                            pPrev = pFirst;
                        }
                        else
                        {
                            pPrev->pNext = pCur->pNext;
                            pCur->pNext = pCur->pNext->pNext;
                            pPrev->pNext->pNext = pCur;
                            pPrev = pPrev->pNext;
                        }
                    }
                    else
                    {
                        pPrev = pCur;
                        pCur = pCur->pNext;
                    }
                }


                j++;
                if(j >= i)
                {
                    break;
                }
            }
        }
    }

    //display
    rk_print_string("\r\n");

    pCur = pFirst;
    i = 0;
    err = 0;

    while(pCur != NULL)
    {
        totalsize += pCur->size;

        if(pCur->flag1 != 0x55aa55aa)
        {
            err = 1;
            break;
        }

        if(pCur->flag != 0xaa55aa55)
        {
            err = 1;
            break;
        }

        BuffSize = sprintf(buf,"%d\t%d\t\t",i, pCur->size);
        UartDev_Write(UartHDC,buf,BuffSize,SYNC_MODE,NULL);

        BuffSize = sprintf(buf,"0x%08x\t\t",(uint8 *)pCur + sizeof(MEMORY_BLOCK));
        UartDev_Write(UartHDC,buf,BuffSize,SYNC_MODE,NULL);

        BuffSize = GetRKTaskName((void *)pCur->hTask, buf);
        UartDev_Write(UartHDC,buf,BuffSize,SYNC_MODE,NULL);

        if(oder == 2)
        {
            if(pCur->pNext == NULL)
            {
                if(remainsize > remainsize1)
                {
                    rk_printf("check free memory err, %d %d", remainsize1, remainsize);
                }
                else if(remainsize < remainsize1)
                {
                   BuffSize = sprintf(buf,"%d\t%d\t\t",i,(xHeapFirstAddr[1] + xTotalHeapSize[1]) - (uint32)((uint8 *)pCur->addr + pCur->size + sizeof(MEMORY_BLOCK) + 12));
                   UartDev_Write(UartHDC,buf,BuffSize,SYNC_MODE,NULL);
                   BuffSize = sprintf(buf,"0x%08x\t\t",(uint8 *)pCur->addr + pCur->size + sizeof(MEMORY_BLOCK) + 12);
                   UartDev_Write(UartHDC,buf,BuffSize,SYNC_MODE,NULL);
                   UartDev_Write(UartHDC,"+\r\n",3,SYNC_MODE,NULL);
                   remainsize += ((xHeapFirstAddr[1] + xTotalHeapSize[1]) - (uint32)((uint8 *)pCur->addr + pCur->size + sizeof(MEMORY_BLOCK) + 12));
                }
            }
            else if((uint8 *)pCur->pNext->addr > ((uint8 *)pCur->addr + pCur->size + sizeof(MEMORY_BLOCK) + 12))
            {
                BuffSize = sprintf(buf,"%d\t%d\t\t",i,(uint8 *)pCur->pNext->addr - ((uint8 *)pCur->addr + pCur->size + sizeof(MEMORY_BLOCK) + 12));
                UartDev_Write(UartHDC,buf,BuffSize,SYNC_MODE,NULL);

                BuffSize = sprintf(buf,"0x%08x\t\t",(uint8 *)pCur->addr + pCur->size + sizeof(MEMORY_BLOCK) + 12);
                UartDev_Write(UartHDC,buf,BuffSize,SYNC_MODE,NULL);

                UartDev_Write(UartHDC,"+\r\n",3,SYNC_MODE,NULL);
                remainsize += ((uint8 *)pCur->pNext->addr - ((uint8 *)pCur->addr + pCur->size+ sizeof(MEMORY_BLOCK) + 12));
            }
            else if((uint8 *)pCur->pNext->addr < ((uint8 *)pCur->addr + pCur->size + sizeof(MEMORY_BLOCK) + 12))
            {
                rk_printf("memory sort error");
            }
        }

        i++;
        pCur = pCur->pNext;


    }

    rk_printf_no_time("    total block cnt = %d, check block cnt = %d, totalsize = %d, check totalsize = %d,  remain = %d, check remain = %d", totalCnt, i, totalsize1,totalsize, remainsize1, remainsize);

    if(err)
    {
        rk_printf("heap destoried in 0x%08x by %s used", (uint8 *)pCur->addr + sizeof(MEMORY_BLOCK), "unknown");
    }

    rkos_memory_free(pbuf);
}
#ifdef _MEMORY_LEAK_CHECH_
/*******************************************************************************
** Name: ShellMemoryLeakWatch
** Input:HDC dev, char * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.10.20
** Time: 10:23:36
*******************************************************************************/
_SYSTEM_SHELL_SHELLTASKCMD_COMMON_
COMMON FUN rk_err_t ShellMemoryLeakWatch(HDC dev, char * pstr)
{
    uint32 order;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }

    order = 0;

    if(StrCmpA(pstr, "/s", 2) == 0)
    {
        order = 1;
    }
    else if(StrCmpA(pstr, "/a", 2) == 0)
    {
        order = 2;
    }

    rk_printf("order = %d", order);

    memory_leak_check_watch(order);

    return RK_SUCCESS;
}
#endif

/*******************************************************************************
** Name: ShellTaskList
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.7.24
** Time: 11:15:26
*******************************************************************************/
_SYSTEM_SHELL_SHELLTASKCMD_COMMON_
COMMON FUN rk_err_t ShellTaskList(HDC dev, uint8 * pstr)
{
    RK_TASK_CLASS * TempTaskHandler;
    uint8  TxBuffer[128];
    uint8  TaskState;
    uint32 TaskRemainStack;
    uint16 BuffSize = 0;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }

    TempTaskHandler = RKTaskGetFirstHandle(0xffffffff);

    rk_print_string("\n    this cmd will list all task info!!\r\n\r\n");

    if (TempTaskHandler == NULL)
    {
        rk_print_string("not find task!\r\n\r\n");
        return RK_SUCCESS;
    }

    BuffSize = sprintf(TxBuffer,"State  TCB  IdleTick  Idle1Time  Idle2Time  Event  time  StackSize  RemainStack  MemorySize  Priority  ClassID  ObjectID  Name\r\n");
    UartDev_Write(dev,TxBuffer,BuffSize,SYNC_MODE,NULL);
    BuffSize = sprintf(TxBuffer,"-------------------------------------------------------------------------------------------------------------------------\r\n");
    UartDev_Write(dev,TxBuffer,BuffSize,SYNC_MODE,NULL);

    while (TempTaskHandler != NULL)
    {
        TaskState = RKTaskState(TempTaskHandler);
        TaskRemainStack = RKTaskStackRemain(TempTaskHandler);

        BuffSize = sprintf(TxBuffer,"%s[%s] ",ShellTaskState[TaskState], ShellTaskState[TempTaskHandler->State]);
        UartDev_Write(dev,TxBuffer,BuffSize,SYNC_MODE,NULL);

        BuffSize = sprintf(TxBuffer,"%08x ", TempTaskHandler->hTask);
        UartDev_Write(dev,TxBuffer,BuffSize,SYNC_MODE,NULL);

        BuffSize = sprintf(TxBuffer,"%06d ", TempTaskHandler->IdleTick);
        UartDev_Write(dev,TxBuffer,BuffSize,SYNC_MODE,NULL);

        BuffSize = sprintf(TxBuffer,"%06d ", TempTaskHandler->Idle1EventTime);
        UartDev_Write(dev,TxBuffer,BuffSize,SYNC_MODE,NULL);

        BuffSize = sprintf(TxBuffer,"%06d ", TempTaskHandler->Idle2EventTime);
        UartDev_Write(dev,TxBuffer,BuffSize,SYNC_MODE,NULL);

        #ifdef _MEMORY_LEAK_CHECH_
        if(TaskState == 3)
        {
            BuffSize = sprintf(TxBuffer,"%08x ",rkos_get_task_event(TempTaskHandler->hTask));
            UartDev_Write(dev,TxBuffer,BuffSize,SYNC_MODE,NULL);
        }
        else
        {
            BuffSize = sprintf(TxBuffer,"%s ","unknow  ");
            UartDev_Write(dev,TxBuffer,BuffSize,SYNC_MODE,NULL);
        }
        #else
        BuffSize = sprintf(TxBuffer,"%s ","unknow  ");
        UartDev_Write(dev,TxBuffer,BuffSize,SYNC_MODE,NULL);
        #endif

        BuffSize = sprintf(TxBuffer,"%02d ",RKTaskTime((HTC)TempTaskHandler));
        UartDev_Write(dev,TxBuffer,BuffSize,SYNC_MODE,NULL);

        BuffSize = sprintf(TxBuffer,"%08x ",RKTaskStackSize((HTC)TempTaskHandler));
        UartDev_Write(dev,TxBuffer,BuffSize,SYNC_MODE,NULL);

        BuffSize = sprintf(TxBuffer,"%08x ",TaskRemainStack);
        UartDev_Write(dev,TxBuffer,BuffSize,SYNC_MODE,NULL);

        BuffSize = sprintf(TxBuffer,"%06d ",TempTaskHandler->TotalMemory);
        UartDev_Write(dev,TxBuffer,BuffSize,SYNC_MODE,NULL);

        BuffSize = sprintf(TxBuffer,"%06d ",TempTaskHandler->TaskPriority);
        UartDev_Write(dev,TxBuffer,BuffSize,SYNC_MODE,NULL);

        BuffSize = sprintf(TxBuffer,"%02d ",TempTaskHandler->TaskClassID);
        UartDev_Write(dev,TxBuffer,BuffSize,SYNC_MODE,NULL);

        BuffSize = sprintf(TxBuffer,"%02d ",TempTaskHandler->TaskObjectID);
        UartDev_Write(dev,TxBuffer,BuffSize,SYNC_MODE,NULL);

        BuffSize = sprintf(TxBuffer,"%s\r\n",RKTaskGetTaskName((HTC)TempTaskHandler));

        UartDev_Write(dev,TxBuffer,BuffSize,SYNC_MODE,NULL);

        TaskState = 0;
        TaskRemainStack = 0;

        TempTaskHandler = RKTaskGetNextHandle(TempTaskHandler, 0xffffffff);

    }

    rk_printf("total task cnt = %d, total suspend cnt = %d", TaskTotalCnt, TaskTotalSuspendCnt);

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
