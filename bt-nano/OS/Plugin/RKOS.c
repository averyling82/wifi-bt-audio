/*
********************************************************************************************
*
*               Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: OS\Plugin\RKOS.c
* Owner: aaron.sun
* Date: 2014.10.15
* Time: 10:00:08
* Desc: RKOS.c
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2014.10.15     10:00:08   1.0
********************************************************************************************
*/
#define __OS_PLUGIN_RKOS_C__
#include "BspConfig.h"
#ifdef __OS_PLUGIN_RKOS_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "FreeRTOS.h"
#include "RKOS.h"

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
MEMORY_BLOCK * pFirstMemoryBlock = NULL;
uint32 UsedMemoryCnt = NULL;
uint32 MemoryMallocDisplay = 0;
SEMAPHORE_QUEUE_BLOCK * pFristSemaPhoreQueueBlock = NULL;


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



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: rkos_get_task_event
** Input:HTC hTask
** Return: void *
** Owner:aaron.sun
** Date: 2016.5.24
** Time: 19:06:44
*******************************************************************************/
_OS_PLUGIN_RKOS_COMMON_
COMMON API void * rkos_get_task_event(HTC hTask)
{
    #ifdef 0
    SEMAPHORE_QUEUE_BLOCK * pCur;
    pCur = pFristSemaPhoreQueueBlock;
    while (pCur != NULL)
    {
        if(pCur->p == (uint32 *)hTask + 3 - 5)
        {
            break;
        }
        pCur = pCur->pNext;
    }
    return pCur;
    #else
    xListItem * p;
    xList * p1;
    p = (xListItem *)((uint8 *)hTask + 4 + sizeof(xListItem));
    p1 = p->pvContainer;
    return (uint8 *)p1 - 16 - sizeof(xList);
    #endif
}

/*******************************************************************************
** Name: rkos_GetFreeHeapSize
** Input:void
** Return: uint32
** Owner:aaron.sun
** Date: 2016.3.31
** Time: 10:44:23
*******************************************************************************/
_OS_PLUGIN_RKOS_COMMON_
COMMON API uint32 rkos_GetFreeHeapSize(void)
{
    return xPortGetFreeHeapSize(1);
}


extern const size_t xTotalHeapSize[2];
extern const size_t xHeapFirstAddr[2];

_OS_PLUGIN_RKOS_COMMON_
void rkos_memory_list_check(void)
{
    MEMORY_BLOCK * pMblock,* pCur, * pPrev;
#ifdef _MEMORY_LEAK_CHECH_
    rkos_enter_critical();
    pCur = pFirstMemoryBlock;
    while (pCur != NULL)
    {
        if(((size_t)(pCur+sizeof(MEMORY_BLOCK)) < xHeapFirstAddr[1]) || ((size_t)(pCur+sizeof(MEMORY_BLOCK)) > (xHeapFirstAddr[1] + xTotalHeapSize[1])))
        {
            printf("\n pCur list error = %x", pCur+sizeof(MEMORY_BLOCK));
            IntPendingSet(2);
            while(1);
        }

        if(pCur->flag != 0xaa55aa55)
        {
            printf("\nlist destory1 = %x", pCur+sizeof(MEMORY_BLOCK));
            IntPendingSet(2);
            rkos_exit_critical();
            while(1);
        }
        if(*(uint32 *)( (uint8*)pCur + pCur->size+sizeof(MEMORY_BLOCK)) != 0x55aa55aa)
        {
            printf("\nlist destory2 = %x", (uint8 *)pCur+sizeof(MEMORY_BLOCK));
            IntPendingSet(2);
            rkos_exit_critical();
            while(1);
        }
        pCur = pCur->pNext;
    }

    rkos_exit_critical();
#endif
}


/*******************************************************************************
** Name: rkos_memory_check
** Input:void * p
** Return: void
** Owner:aaron.sun
** Date: 2016.3.15
** Time: 15:06:54
*******************************************************************************/
_OS_PLUGIN_RKOS_COMMON_
COMMON API void rkos_memory_check(void * buf)
{
    uint8 * p;
    MEMORY_BLOCK * pMblock,* pCur, * pPrev;

    if(buf == NULL)
    {
       printf("\ncheck buf = %x", buf);
       return;
    }

#ifdef _MEMORY_LEAK_CHECH_
    p = (uint8 *)buf;

    pMblock = (MEMORY_BLOCK *)(p - sizeof(MEMORY_BLOCK));

    if(pMblock->flag != 0xaa55aa55)
    {
        printf("\ncheck destory1 = %x", p);
        IntPendingSet(2);
        while(1);
    }

    if(*(uint32 *)(p + pMblock->size) != 0x55aa55aa)
    {
        printf("\ncheck destory2 = %x", p);
        IntPendingSet(2);
        while(1);
    }

    printf("\nbuf = %x check ok", buf);

#else

#endif
}
/*******************************************************************************
** Name: rkos_memory_malloc_display
** Input:uint32 display
** Return: void
** Owner:aaron.sun
** Date: 2016.3.15
** Time: 8:40:11
*******************************************************************************/
_OS_PLUGIN_RKOS_COMMON_
COMMON API void rkos_memory_malloc_display(uint32 display)
{
    MemoryMallocDisplay = display;
}
/*******************************************************************************
** Name: rkos_task_delete
** Input:HTC hTask
** Return: void
** Owner:aaron.sun
** Date: 2014.12.18
** Time: 15:44:52
*******************************************************************************/
_OS_PLUGIN_RKOS_COMMON_
COMMON API void rkos_task_delete(HTC hTask)
{
#ifdef _MEMORY_LEAK_CHECH_
    MEMORY_BLOCK * pCur;
    rkos_enter_critical();
    pCur = pFirstMemoryBlock;
    while (pCur != NULL)
    {
        if (pCur->hTask == (uint32)hTask)
        {
            pCur->hTask = 0;
            #ifdef _SUPPORT_MEMORY_COUNT_
            pCur->hRkTask = 0;
            #endif
        }
        pCur = pCur->pNext;
    }
    rkos_exit_critical();
#endif
    vTaskDelete(hTask);
}

/*******************************************************************************
** Name: rkos_task_create
** Input:pRkosTaskCode TaskCode, uint32 StackDeep, uint32 p, void * para
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.12.18
** Time: 14:44:46
*******************************************************************************/
_OS_PLUGIN_RKOS_COMMON_
COMMON API HTC rkos_task_create(pRkosTaskCode TaskCode,char *name, uint32 StatckBase, uint32 StackDeep, uint32 Priority, void * para)
{
    HTC hTask;
    if(xTaskCreate(TaskCode, name, StackDeep, para, Priority, &hTask, (uint32 *)StatckBase) != pdPASS)
    {
        return NULL;
    }
    else
    {
        return hTask;
    }
}

/*******************************************************************************
** Name: rkos_queue_recive_fromisr
** Input:pQueue pQue, void * buf
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.12.18
** Time: 14:26:40
*******************************************************************************/
_OS_PLUGIN_RKOS_COMMON_
COMMON API rk_err_t rkos_queue_recive_fromisr(pQueue pQue, void * buf)
{

}

/*******************************************************************************
** Name: rkos_queue_send_fromisr
** Input:pQueue pQue, void * buf
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.12.18
** Time: 14:25:02
*******************************************************************************/
_OS_PLUGIN_RKOS_COMMON_
COMMON API rk_err_t rkos_queue_send_fromisr(pQueue pQue, void * buf)
{
    portBASE_TYPE xHigherPriorityTaskWoken;

    if (pdPASS == xQueueSendFromISR(pQue, buf, &xHigherPriorityTaskWoken))
    {
        return RK_SUCCESS;
    }
    else
    {
        return RK_ERROR;
    }
}

/*******************************************************************************
** Name: rkos_start_timer
** Input:pTimer timer
** Return: void
** Owner:aaron.sun
** Date: 2014.11.10
** Time: 13:53:20
*******************************************************************************/
_OS_PLUGIN_RKOS_COMMON_
COMMON API rk_err_t rkos_start_timer(pTimer timer)
{
    if (xTimerStart(timer,0) == pdFAIL)
    {
        return RK_ERROR;
    }
    else
    {
        return RK_SUCCESS;
    }
}

/*******************************************************************************
** Name: rkos_stop_timer
** Input:pTimer timer
** Return: void
** Owner:wangping
** Date: 2015.9.20
** Time: 15:47:40
*******************************************************************************/
_OS_PLUGIN_RKOS_COMMON_
COMMON API rk_err_t rkos_stop_timer(pTimer timer)
{
    if (xTimerStop(timer, 0) == pdFAIL)
    {
        return RK_ERROR;
    }
    else
    {
        return RK_SUCCESS;
    }
}

/*******************************************************************************
** Name: rkos_get_timer_param
** Input:pTimer timer
** Return: void*  timer's param
** Owner:wangping
** Date: 2015.9.20
** Time: 15:47:40
*******************************************************************************/
_OS_PLUGIN_RKOS_COMMON_
COMMON API void* rkos_get_timer_param(pTimer timer)
{
    return pvTimerGetTimerID(timer);
}

/*******************************************************************************
** Name: rkos_mod_timer
** Input:pTimer timer ,int NewPeriod, int
** Return: rk_err_t
** Owner:wangping
** Date: 2015.10.22
** Time: 17:00:40
*******************************************************************************/
_OS_PLUGIN_RKOS_COMMON_
COMMON API rk_err_t rkos_mod_timer(pTimer timer, int NewPeriod, int BlockTime)
{
    if (xTimerChangePeriod(timer, NewPeriod, BlockTime) == pdFAIL)
    {
        return RK_ERROR;
    }
    else
    {
        return RK_SUCCESS;
    }
}

/*******************************************************************************
** Name: rkos_delete_timer
** Input:pTimer timer
** Return: void
** Owner:aaron.sun
** Date: 2014.11.10
** Time: 13:45:01
*******************************************************************************/
_OS_PLUGIN_RKOS_COMMON_
COMMON API void rkos_delete_timer(pTimer timer)
{
    xTimerDelete(timer, 0);
}

/*******************************************************************************
** Name: rkos_create_timer
** Input:uint32 period, uint32 reload, pRkosTimerCallBack pfCall
** Return: pTimer
** Owner:aaron.sun
** Date: 2014.11.10
** Time: 11:57:47
*******************************************************************************/
_OS_PLUGIN_RKOS_COMMON_
COMMON API pTimer rkos_create_timer(uint32 period, uint32 reload,void *param, pRkosTimerCallBack pfCall)
{
    return xTimerCreate(NULL, period ,reload, param, pfCall);
}

/*******************************************************************************
** Name: rkos_resume_all_task
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2014.11.10
** Time: 11:41:49
*******************************************************************************/
_OS_PLUGIN_RKOS_COMMON_
COMMON API void rkos_resume_all_task(void)
{
    xTaskResumeAll();
}

/*******************************************************************************
** Name: rkos_suspend_all_task
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2014.11.10
** Time: 11:41:09
*******************************************************************************/
_OS_PLUGIN_RKOS_COMMON_
COMMON API void rkos_suspend_all_task(void)
{
    vTaskSuspendAll();
}

/*******************************************************************************
** Name: rkos_queue_create
** Input:uint32 blockcnt, uint32 blocksize
** Return: pQueue
** Owner:aaron.sun
** Date: 2014.10.20
** Time: 15:06:20
*******************************************************************************/
_OS_PLUGIN_RKOS_COMMON_
COMMON API pQueue rkos_queue_create(uint32 blockcnt, uint32 blocksize)
{
    return xQueueCreate(blockcnt, blocksize);
}

/*******************************************************************************
** Name: rkos_queue_delete
** Input:pQueue pQue
** Return: void
** Owner:aaron.sun
** Date: 2014.10.20
** Time: 15:02:52
*******************************************************************************/
_OS_PLUGIN_RKOS_COMMON_
COMMON API void rkos_queue_delete(pQueue pQue)
{
    vQueueDelete(pQue);
}

/*******************************************************************************
** Name: rkos_queue_send
** Input:pQueue pQue, void * buf, uint32 time
** Return: void
** Owner:aaron.sun
** Date: 2014.10.20
** Time: 15:01:04
*******************************************************************************/
_OS_PLUGIN_RKOS_COMMON_
COMMON API rk_err_t rkos_queue_send(pQueue pQue, void * buf, uint32 time)
{
    if (xQueueSend(pQue, buf, time) == pdTRUE)
    {
        return RK_SUCCESS;
    }
    else
    {
        return RK_ERROR;
    }
}

/*******************************************************************************
** Name: rkos_queue_receive
** Input:pQueue pQue, void * buf, uint32 time
** Return: void
** Owner:aaron.sun
** Date: 2014.10.20
** Time: 14:57:50
*******************************************************************************/
_OS_PLUGIN_RKOS_COMMON_
COMMON API rk_err_t rkos_queue_receive(pQueue pQue, void * buf, uint32 time)
{
    if (xQueueReceive(pQue, buf, time) == pdPASS)
    {
        return RK_SUCCESS;
    }
    else
    {
        return RK_ERROR;
    }
}

/*******************************************************************************
** Name: rkos_semaphore_give_fromisr
** Input:pSemaphore pSem
** Return: void
** Owner:aaron.sun
** Date: 2014.10.15
** Time: 11:54:06
*******************************************************************************/
_OS_PLUGIN_RKOS_COMMON_
COMMON API void rkos_semaphore_give_fromisr(pSemaphore pSem)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    #ifdef 0
    SEMAPHORE_QUEUE_BLOCK * p1;
    p1 = (SEMAPHORE_QUEUE_BLOCK *)pSem;
    xSemaphoreGiveFromISR(p1->p, &xHigherPriorityTaskWoken);
    #else
    xSemaphoreGiveFromISR(pSem, &xHigherPriorityTaskWoken);
    #endif

}

/*******************************************************************************
** Name: rkos_start
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2014.10.15
** Time: 11:00:08
*******************************************************************************/
_OS_PLUGIN_RKOS_COMMON_
COMMON API void rkos_start(void)
{
    vTaskStartScheduler();
    while (1);
}

/*******************************************************************************
** Name: rkos_init
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2014.10.15
** Time: 10:59:35
*******************************************************************************/
_OS_PLUGIN_RKOS_COMMON_
COMMON API void rkos_init(void)
{
    RKTaskInit();
    SchedulerTaskCreate();
}

/*******************************************************************************
** Name: rkos_memory_free
** Input:void * buf
** Return: void
** Owner:aaron.sun
** Date: 2014.10.15
** Time: 10:51:43
*******************************************************************************/
_OS_PLUGIN_RKOS_COMMON_
COMMON API void rkos_memory_free(void * buf)
{
    uint8 * p;
    MEMORY_BLOCK * pMblock,* pCur, * pPrev;

    if(buf == NULL)
    {
        return;
    }

    if(((size_t)buf < xHeapFirstAddr[1]) || ((size_t)buf >(xHeapFirstAddr[1] + xTotalHeapSize[1])))
    {
        printf("\n buf error = %x", buf);
        IntPendingSet(2);
        while(1);
    }

#ifdef _MEMORY_LEAK_CHECH_

    rkos_enter_critical();

    p = (uint8 *)buf;

    if(MemoryMallocDisplay)
    {
        printf("\nthread:%s free addr = %x", pcTaskGetTaskName(pxCurrentTCB), p);
    }

    pMblock = (MEMORY_BLOCK *)(p - sizeof(MEMORY_BLOCK));

    if(pMblock->flag != 0xaa55aa55)
    {
        printf("\nmemory destory1 = %x", p);
        IntPendingSet(2);
        rkos_exit_critical();
        while(1);
    }

    if(*(uint32 *)(p + pMblock->size) != 0x55aa55aa)
    {
        printf("\nmemory destory2 = %x", p);
        IntPendingSet(2);
        rkos_exit_critical();
        while(1);
    }

    pCur = pFirstMemoryBlock;
    pPrev = NULL;
    while (pCur != NULL)
    {
        if (pCur == pMblock)
        {
            if (pPrev == NULL)
            {
                pFirstMemoryBlock = pCur->pNext;
            }
            else
            {
                pPrev->pNext = pCur->pNext;
            }

            #ifdef _SUPPORT_MEMORY_COUNT_
            {
                RK_TASK_CLASS * TempTaskHandler;
                TempTaskHandler = (RK_TASK_CLASS *)pCur->hRkTask;
                if(TempTaskHandler != NULL)
                {
                    TempTaskHandler->TotalMemory -= pCur->size;
                }
            }
            #endif

            break;
        }

        pPrev = pCur;
        pCur = pCur->pNext;
    }
    UsedMemoryCnt--;

    if(MemoryMallocDisplay)
    {
        printf("\nthread:%s free ok = %x", pcTaskGetTaskName(pxCurrentTCB),p);
    }

    rkos_exit_critical();

    vPortFree(pMblock, 1);
#else
    vPortFree(buf, 1);
#endif

}

/*******************************************************************************
** Name: rkos_memory_malloc
** Input:uint32 size
** Return: void *
** Owner:aaron.sun
** Date: 2014.10.15
** Time: 10:38:53
*******************************************************************************/
_OS_PLUGIN_RKOS_COMMON_
COMMON API void * rkos_memory_malloc(uint32 size)
{
    uint8 * p;
    MEMORY_BLOCK * pMblock;

    if(size == 0)
    {
        return NULL;
    }

#ifdef _MEMORY_LEAK_CHECH_
    if(size % 4)
    {
        size += (size % 4);
    }

    p = pvPortMalloc(size +sizeof(MEMORY_BLOCK) + 4, 1);
#else
    p = pvPortMalloc(size, 1);
#endif

    if (p != NULL)
    {
#ifdef _MEMORY_LEAK_CHECH_
        rkos_enter_critical();
        pMblock = (MEMORY_BLOCK *)p;
        pMblock->size = size;
        pMblock->hTask = (uint32)pxCurrentTCB;
        pMblock->flag = 0xaa55aa55;

#ifdef _SUPPORT_MEMORY_COUNT_
        {
            RK_TASK_CLASS * TempTaskHandler;

            TempTaskHandler = RKTaskGetRunHandle();

            if (TempTaskHandler == NULL)
            {
                pMblock->hRkTask = 0;
            }
            else
            {
                pMblock->hRkTask = (uint32)TempTaskHandler;
                TempTaskHandler->TotalMemory += size;
            }
        }
#endif

#ifdef _SUPPORT_SORT_BY_ADDR_
        {
            MEMORY_BLOCK * pCur, * pPrev;
            pCur = pFirstMemoryBlock;
            pPrev = NULL;

            if (pFirstMemoryBlock == NULL)
            {
                pFirstMemoryBlock = pMblock;
                pMblock->pNext = NULL;
            }
            else
            {
                while (pCur != NULL)
                {
                    if (pMblock < pCur)
                    {
                        if (pPrev == NULL)
                        {
                            pMblock->pNext = pFirstMemoryBlock;
                            pFirstMemoryBlock = pMblock;
                        }
                        else
                        {
                            pMblock->pNext = pPrev->pNext;
                            pPrev->pNext = pMblock;
                        }

                        pMblock = NULL;

                        break;
                    }

                    pPrev = pCur;
                    pCur = pCur->pNext;

                }
            }

            if (pMblock != NULL)
            {
                pPrev->pNext = pMblock;
                pMblock->pNext = NULL;
            }
        }
#else
        pMblock->pNext = pFirstMemoryBlock;
        pFirstMemoryBlock = pMblock;
#endif

        p = p + sizeof(MEMORY_BLOCK);
        *(uint32 *)(p + size) = 0x55aa55aa;
        UsedMemoryCnt++;

        if(MemoryMallocDisplay)
        {
            printf("\nthread:%s malloc addr = %x, size = %d", pcTaskGetTaskName(pxCurrentTCB), p, size);
        }
        rkos_exit_critical();
        memset(p, 0, size);
        return p;
#else
        memset(p, 0, size);
        return p;
#endif
    }
    else
    {
        rk_printf("thread:%s", pcTaskGetTaskName(pxCurrentTCB));

        return NULL;
    }
}


/*******************************************************************************
** Name: rkos_memory_realloc
** Input:void * pv, uint32 size
** Return: void *
** Owner:aaron.sun
** Date: 2014.10.15
** Time: 10:38:53
*******************************************************************************/
_OS_PLUGIN_RKOS_COMMON_
COMMON API void * rkos_memory_realloc(void * pv, uint32 size)
{
    void *new_point = NULL;

    #ifdef _MEMORY_LEAK_CHECH_
    MEMORY_BLOCK *block_point = NULL;
    #else
    xBlockLink *block_point = NULL;
    #endif

    size_t copy_size = 0;

    if (!pv)
        return rkos_memory_malloc(size);

    new_point = rkos_memory_malloc(size);
    if (new_point == NULL)
    {
        rk_printf("rkos realloc fail size = %d", size);
        return NULL;
    }

    #ifdef _MEMORY_LEAK_CHECH_
    block_point = (MEMORY_BLOCK *)(( unsigned char * )pv - sizeof(MEMORY_BLOCK));
    copy_size = block_point->size < size ? block_point->size  : size;
    #else
    block_point = (xBlockLink *)(( unsigned char * )pv - 8);
    copy_size = block_point->xBlockSize < size ? block_point->xBlockSize  : size;
    #endif


    memcpy(new_point, pv, copy_size);
    rkos_memory_free(pv);

    return new_point;

}




/******************************************************************************
 * rkos_memcpy -
 * DESCRIPTION: -
 *
 * Input:
 * Output:
 * Returns:
 *
 * modification history
 * --------------------
 * Owner: wangping
 * Date : 2015.4.30  written
 * Time : 15:18:06
 * --------------------
 ******************************************************************************/
_OS_PLUGIN_RKOS_COMMON_
API void rkos_memcpy(void* dst, void* src, uint32 sz)
{
    memcpy(dst, src, sz);
}

/******************************************************************************
 * rkos_memcmp -
 * DESCRIPTION: -
 *
 * Input:
 * Output:
 * Returns:
 *
 * modification history
 * --------------------
 * Owner: wangping
 * Date : 2015.4.30  written
 * Time : 15:18:31
 * --------------------
 ******************************************************************************/
_OS_PLUGIN_RKOS_COMMON_
API int32 rkos_memcmp(void *dst, void *src, uint32 sz)
{
    //under spread sdk, the return value of
    //memcmp for two same mem. chunk is 0
    if (!memcmp(dst, src, sz))
        return TRUE;
    else
        return FALSE;
}

/******************************************************************************
 * rkos_memset -
 * DESCRIPTION: -
 *
 * Input:
 * Output:
 * Returns:
 *
 * modification history
 * --------------------
 * Owner: wangping
 * Date : 2015.4.30  written
 * Time : 15:18:43
 * --------------------
 ******************************************************************************/
_OS_PLUGIN_RKOS_COMMON_
API void rkos_memset(void *pbuf, int32 c, uint32 sz)
{
    memset(pbuf, c, sz);
}


/*******************************************************************************
** Name: rkos_semaphore_delete
** Input:pSemaphore pSem
** Return: void
** Owner:aaron.sun
** Date: 2014.10.15
** Time: 10:36:22
*******************************************************************************/
_OS_PLUGIN_RKOS_COMMON_
COMMON API void rkos_semaphore_delete(pSemaphore pSem)
{
    #ifdef 0
    SEMAPHORE_QUEUE_BLOCK * p1, * pCur, * pPrev;
    p1 = (SEMAPHORE_QUEUE_BLOCK *)pSem;
    pCur = pFristSemaPhoreQueueBlock;
    pPrev = NULL;
    while (pCur != NULL)
    {
        if (pCur == p1)
        {
            if (pPrev == NULL)
            {
                pFristSemaPhoreQueueBlock = pCur->pNext;
            }
            else
            {
                pPrev->pNext = pCur->pNext;
            }
            break;
        }

        pPrev = pCur;
        pCur = pCur->pNext;
    }
    vSemaphoreDelete(p1->p);
    vPortFree(p1, 0);
    #else
    vSemaphoreDelete(pSem);
    #endif
}

/*******************************************************************************
** Name: rkos_semaphore_create
** Input:uint32 MaxCnt, uint32 InitCnt
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.10.15
** Time: 10:28:20
*******************************************************************************/
_OS_PLUGIN_RKOS_COMMON_
COMMON API pSemaphore rkos_semaphore_create(uint32 MaxCnt, uint32 InitCnt)
{
    pSemaphore * p;
    SEMAPHORE_QUEUE_BLOCK * p1;

    p = xSemaphoreCreateCounting(MaxCnt,InitCnt);

    #ifdef 0
    if(p)
    {
        p1 = pvPortMalloc(sizeof(SEMAPHORE_QUEUE_BLOCK), 0);
        if(p1 == NULL);
        {
            vSemaphoreDelete(p);
            return NULL;
        }
        memset(p1,0, sizeof(SEMAPHORE_QUEUE_BLOCK));

        rkos_enter_critical();
        p1->pNext = pFristSemaPhoreQueueBlock;
        p1->p = p;
        pFristSemaPhoreQueueBlock = p1;
        rkos_exit_critical();

    }
    return p1;
    #else
    return p;
    #endif
}

/*******************************************************************************
** Name: rkos_mutex_create
** Return: pSemaphore
** Owner:yangh.yang
** Date: 2015.11.23
** Time: 10:28:20
*******************************************************************************/
_OS_PLUGIN_RKOS_COMMON_
COMMON API pSemaphore rkos_mutex_create()
{
    return xSemaphoreCreateMutex();
}

/*******************************************************************************
** Name: rkos_semaphore_give
** Input:pSemaphore pSem
** Return: void
** Owner:aaron.sun
** Date: 2014.10.15
** Time: 10:23:01
*******************************************************************************/
_OS_PLUGIN_RKOS_COMMON_
COMMON API void rkos_semaphore_give(pSemaphore pSem)
{
    #ifdef 0
    SEMAPHORE_QUEUE_BLOCK * p1;
    p1 = (SEMAPHORE_QUEUE_BLOCK *)pSem;
    xSemaphoreGive(p1->p);
    #else
    xSemaphoreGive(pSem);
    #endif
}

/*******************************************************************************
** Name: rkos_semaphore_take
** Input:pSemaphore pSem, uint32 time
** Return: void
** Owner:aaron.sun
** Date: 2014.10.15
** Time: 10:18:46
*******************************************************************************/
_OS_PLUGIN_RKOS_COMMON_
COMMON API rk_err_t rkos_semaphore_take(pSemaphore pSem, uint32 time)
{
    #ifdef 0
    SEMAPHORE_QUEUE_BLOCK * p1;
    p1 = (SEMAPHORE_QUEUE_BLOCK *)pSem;
    if (xSemaphoreTake(p1->p, time)== pdPASS)
    {
        return RK_SUCCESS;
    }
    else
    {
        return RK_ERROR;
    }
    #else
    if (xSemaphoreTake(pSem, time)== pdPASS)
    {
        return RK_SUCCESS;
    }
    else
    {
        return RK_ERROR;
    }
    #endif


}

/*******************************************************************************
** Name: rkos_exit_critical
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2014.10.15
** Time: 10:12:07
*******************************************************************************/
_OS_PLUGIN_RKOS_COMMON_
COMMON API void rkos_exit_critical(void)
{
    portEXIT_CRITICAL();
}

/*******************************************************************************
** Name: rkos_enter_critical
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2014.10.15
** Time: 10:11:34
*******************************************************************************/
_OS_PLUGIN_RKOS_COMMON_
COMMON API void rkos_enter_critical(void)
{
    portENTER_CRITICAL();
}

_OS_PLUGIN_RKOS_COMMON_
API void rkos_sleep(uint32 ms)
{
    uint delay = ms/portTICK_RATE_MS;
    if (delay == 0)
    {
        delay = 1; /*the min time is 1 tick*/
    }
    vTaskDelay(delay);
}

_OS_PLUGIN_RKOS_COMMON_
API void rkos_delay(uint32 ms)
{
    uint delay = ms/portTICK_RATE_MS;
    if (delay == 0)
    {
        delay = 1; /*the min time is 1 tick*/
    }
    vTaskDelay(delay);
}


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(common) define
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

