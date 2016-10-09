//#include "malloc.h"
#include "FreeRTOS.h"
#include "RKOS.h"
#include "delay.h"
#include "freertos_types.h"
#include "wlan_include.h"
#include "freertos_api.h"
#include "freertos_wq.h"
#define DEBUG1(format,...)  //printf("FILE: %s, LINE: %d: "format, __FILE__, __LINE__, ##__VA_ARGS__)
#ifdef __DRIVER_I6000_WIFI_C__
int32 malloc_total = 0;
int32 malloc_max = 0;
//__inline static void freertos_mem_assert(uint value)
#define MEM_ASSERT(x)                                \
{   uint a = x;                                      \
    if(a == 0)                                       \
    {                                                \
        DEBUG1("value ==0\n");                       \
    }                                                \
    else if(a >= 0x03000000 && a <= 0x03050000 )     \
    {                                                \
                                                     \
    }                                                \
    else if(a >= 0x03050000 && a <= 0x03090000)      \
    {                                                \
                                                     \
    }                                                \
    else                                             \
    {                                                \
        DEBUG1("value INVALID =0x%04x\n",a);         \
    }                                                \
                                                     \
}

//#define MEM_ASSERT(a)  freertos_mem_assert(a)

uint16 wifi_task_stk_offset = 0; /*point32 to the next available task stack beginning*/

extern int32 malloc_count;
extern uint16 Get_Adc(uint8 ch);

#ifdef CONFIG_HEAP_USAGE_TRACKING
int32 used_heap_size;
int32 max_used_heap_size;
int32 min_free_heap_size;

void init_heap_usage(void)
{
#if 0
    used_heap_size = 0;
    max_used_heap_size = 0;
    min_free_heap_size = get_free_heap_size();
#endif
}

void add_heap_usage(void *ptr, int32 size, uint8 memx)
{
#if 0
    int32 free_heap_size = get_free_heap_size();
    uint16 block_num = 0;
    uint16 block_size;

    if(ptr == NULL)
    {
        DBG_PRINT(WLAN_DBG_OSAL, _drv_err_, ("Alloc memory fail, size: %d, current heap free size: %d\n", size, free_heap_size));
        return;
    }
    else
    {
        DBG_PRINT(WLAN_DBG_OSAL, _drv_dump_, ("Alloc memory at %p with size of %d\n", ptr, size));
    }

    block_size = get_heap_block_size(memx);

    CPU_IntDis();

    if (size % block_size)
    {
        block_num = (size / block_size) + 1;
        used_heap_size += (block_num * block_size);
    }
    else
    {
        used_heap_size += size;
    }

    if (used_heap_size > max_used_heap_size)
        max_used_heap_size = used_heap_size;

    if (min_free_heap_size > free_heap_size)
        min_free_heap_size = free_heap_size;

    CPU_IntEn();
#endif
}

void del_heap_usage(void *ptr, int32 size, uint8 memx)
{
#if 0
    uint16 block_num = 0;
    uint16 block_size;

    if (ptr == NULL)
        return;

    DBG_PRINT(WLAN_DBG_OSAL, _drv_dump_, ("Free memory at %p with size of %d\n", ptr, size));

    block_size = get_heap_block_size(memx);
    if (size % block_size)
    {
        block_num = (size / block_size) + 1;
        used_heap_size -= (block_num * block_size);
    }
    else
    {
        used_heap_size -= size;
    }
#endif
}

#endif

uint8* freertos_zmalloc(uint32 sz)
{
    void* ptr = 0;
    malloc_count++;
    malloc_total += sz;
    if(malloc_max < malloc_total)
    {
        malloc_max = malloc_total;
    }
    ptr = rkos_memory_malloc(sz);
    if (ptr != 0)
    {
        memset(ptr, 0, sz);
    }
    else
    {
        printf("free rtos malloc err size : %d\n", sz);
    }
    //printf("zmalloc size : %d, addr :0x%04x\n", sz, (uint32)ptr);
    //printf("malloc total =%d, malloc max=%d",malloc_total,malloc_max);
#ifdef CONFIG_HEAP_USAGE_TRACKING
//  add_heap_usage(ptr, sz, SRAMEX);
#endif

    return ptr;
}


uint8* freertos_malloc(uint32 sz)
{
    void* ptr = 0;
    malloc_count++;
    malloc_total += sz;
    if(malloc_max < malloc_total)
    {
        malloc_max = malloc_total;
    }
    ptr = rkos_memory_malloc(sz);
    if (ptr != 0)
    {
        memset(ptr, 0, sz);
    }
    else
    {
        printf("free rtos malloc err size : %d\n", sz);
    }

    //printf("malloc size : %d\n", sz);
    //printf("malloc total =%d, malloc max=%d",malloc_total,malloc_max);
#ifdef CONFIG_HEAP_USAGE_TRACKING
//  add_heap_usage(ptr, sz, SRAMEX);
#endif

    return ptr;
}

void freertos_mfree(uint8 *pbuf, uint32 sz)
{
    if(pbuf == NULL)
        return;
    memset(pbuf, 0, sz);
    rkos_memory_free(pbuf);
#ifdef CONFIG_HEAP_USAGE_TRACKING
//  del_heap_usage(pbuf, sz, SRAMEX);
#endif
    pbuf = NULL;
    malloc_count--;
    malloc_total = malloc_total- sz;
//    printf("malloc size : %d, addr :0x%04x", sz, (uint32)pbuf);
}

int32 get_free_heap_size(void)
{
    return rkos_GetFreeHeapSize();
}

static void freertos_memcpy(void* dst, void* src, uint32 sz)
{
    memcpy(dst, src, sz);
}

static int32 freertos_memcmp(void *dst, void *src, uint32 sz)
{
    //under spread sdk, the return value of
    //memcmp for two same mem. chunk is 0
    if(!memcmp(dst, src, sz))
        return _TRUE;
    else
        return _FALSE;
}

static void freertos_memset(void *pbuf, int32 c, uint32 sz)
{
    memset(pbuf, c, sz);
}

static void freertos_init_sema(_sema *sema, int32 init_val)
{

    *sema = xSemaphoreCreateCounting(10000, init_val);
    if(*sema= NULL )
    {
        DBG_PRINT(WLAN_DBG_OSAL, _drv_err_, ("freertos_init_sema failed!\n"));
        printf("freertos_init_sema\n");
    }
}

static void freertos_free_sema(_sema *sema)
{
    vSemaphoreDelete( *sema );
}

static void freertos_up_sema(_sema  *sema)
{
    xSemaphoreGive(*sema);
}

static void freertos_down_sema(_sema *sema)
{
    xSemaphoreTake( *sema, portMAX_DELAY );
}

static uint32   freertos_get_current_time(void)
{
    //OS_ERR err;

    //return (uint32)((OSTimeGet(&err)*1000)/OS_CFG_TICK_RATE_HZ);

    return xTaskGetTickCount()*portTICK_RATE_MS;
}

void freertos_msleep(int32 ms)
{
    rkos_sleep(ms);
}

static void freertos_usleep(int32 us)
{
    if (us >= 1000)
        os_msleep(us/1000);
    else
        os_msleep(1); //there is no usleep in freertos,  so we use sleep 1ms here.
}
static void freertos_mdelay(int32 ms)
{
    rkos_sleep(ms);
}

static void freertos_udelay(int32 us)
{
    if (us >= 1000)
        os_msleep(us/1000);
    else
        os_msleep(1);
}

static void freertos_mutex_init(_mutex *pmutex)
{
    *pmutex = xSemaphoreCreateCounting(1, 1);
    if(*pmutex == NULL)
    {
        printf("freertos_mutex_init fail\n");
        //DEBUG1("freertos_mutex_init err\n");
    }
}

static void freertos_mutex_free(_mutex *pmutex)
{
    MEM_ASSERT((uint32)*pmutex);
    vSemaphoreDelete( *pmutex );
}

__inline static void _freertos_mutex_get(_mutex *pmutex)
{
    MEM_ASSERT((uint32)*pmutex);
    xSemaphoreTake( *pmutex, portMAX_DELAY );
}

__inline static void _freertos_mutex_put(_mutex *pmutex)
{
    MEM_ASSERT((uint32)*pmutex);
    xSemaphoreGive( *pmutex );
}

static void freertos_mutex_get(_lock    *plock)
{
    _freertos_mutex_get((_mutex *)plock);
}

static void freertos_mutex_put(_lock *plock)
{
    _freertos_mutex_put((_mutex *)plock);
}

void freertos_spinlock_init(_lock *plock)
{
    //printf("freertos_spinlock_init\n");
    freertos_mutex_init((_mutex *)plock);
}

void freertos_spinlock_free(_lock *plock)
{
    freertos_mutex_free((_mutex *)plock);
}

void freertos_spin_lock(_lock   *plock)
{
    _freertos_mutex_get((_mutex *)plock);
}

void freertos_spin_unlock(_lock *plock)
{
    _freertos_mutex_put((_mutex *)plock);
}

static void freertos_spin_lock_irqsave(_lock *plock, _irqL *pirqL)
{
    long pxHigherPriorityTaskWoken;
    MEM_ASSERT((uint32)*plock);
    xSemaphoreTakeFromISR(*plock, &pxHigherPriorityTaskWoken );
    //_freertos_mutex_get(plock);
}

static void freertos_spin_unlock_irqsave(_lock *plock, _irqL *pirqL)
{
    //_freertos_mutex_put(plock);
    long pxHigherPriorityTaskWoken;
    MEM_ASSERT((uint32)*plock);
    xSemaphoreGiveFromISR( *plock, &pxHigherPriorityTaskWoken );
}

static void freertos_enter_critical(_lock *plock, _irqL *pirqL)
{
    portENTER_CRITICAL();
}

static void freertos_exit_critical(_lock *plock, _irqL *pirqL)
{
    portEXIT_CRITICAL();
}

static void freertos_timer_handler(xTimerHandle p_tmr)
{
    _timer *ptimer = (_timer *)pvTimerGetTimerID(p_tmr);
    MEM_ASSERT((uint32)p_tmr);
    DEBUG1("freertos_timer_handler enter");
    DBG_PRINT(WLAN_DBG_OSAL, _drv_debug_, ("freertos_timer_handler!\n"));

    if (ptimer->function)
        ptimer->function((uint32)ptimer);
    //printf("freertos_timer_handler is %s",ptimer->timer_name);
    DEBUG1("freertos_timer_handler exit");
}

static void freertos_yield(void)
{
    DEBUG1("freertos_create_task enter");
    taskYIELD ();
    DEBUG1("freertos_create_task exit");
}

void freertos_init_timer(_timer *ptimer,void *padapter, TIMER_FUN pfunc,void* cntx, const uint8 *name)
{
//  printf("freertos_init_timer enter\n");
    os_memset((void*)ptimer->timer_name, 0, 32);
    os_memcpy((void*)ptimer->timer_name, (void*)name, 32);
    MEM_ASSERT((uint32)ptimer);
    ptimer->function = (TIMER_FUN)pfunc;

    ptimer->adapter = (void*)padapter;
    ptimer->context = (void*)cntx;
    //printf("ptimer->adapter = 0x%04x\n",(uint32)padapter);
    //printf("ptimer->context = 0x%04x\n",(uint32)cntx);

    ptimer->os_timer = xTimerCreate(  (const signed char *)name, (portTickType)( 10/ portTICK_RATE_MS ), pdFALSE, (void*)ptimer, freertos_timer_handler);
    if ( ptimer->os_timer == NULL )
    {
        printf("freertos_init_timer fail\n");
    }

    DEBUG1("ptimer->os_timer = 0x%04x\n",(uint32)ptimer->os_timer);
//  printf("freertos_init_timer exit\n");
}

void freertos_set_timer(_timer *ptimer,uint32 delay_time)
{
    DEBUG1("freertos_set_timer enter 0x%04x, delay_time = %d\n", (uint32)ptimer, delay_time);
    MEM_ASSERT((uint32)ptimer);
    if(!ptimer->function)
        return;
    DEBUG1("ptimer->os_timer 0x%04x\n",ptimer->os_timer);
    if ((delay_time*configTICK_RATE_HZ) >= 1000)
    {
        xTimerChangePeriod( ptimer->os_timer , delay_time*configTICK_RATE_HZ/1000, 0 );
        xTimerStart(ptimer->os_timer,0);
    }
    else
    {


        xTimerChangePeriod( ptimer->os_timer , 1, 0 );
        xTimerStart(ptimer->os_timer,0);
    }
}

boolean freertos_timer_pending(_timer *ptimer)
{
    DEBUG1(" freertos_timer_pending enter\n");
    MEM_ASSERT((uint32)ptimer);
    if(xTimerIsTimerActive(ptimer->os_timer) != pdFALSE)
    {
        return _TRUE;
    }
    else
    {

         return _FALSE;
    }
    DEBUG1("freertos_timer_pending exit\n");

}

void freertos_cancel_timer(_timer *ptimer,uint8 *bcancelled)
{
    int ret;
    MEM_ASSERT((uint32)ptimer);
    DEBUG1("freertos_cancel_timer enter\n");
    if (ptimer->function)
    {
        ret = xTimerStop( ptimer->os_timer, 0);
        if(ret == pdPASS)
        {
            *bcancelled=  1;//TRUE ==1; FALSE==0
        }
        else
        {
            *bcancelled=  0;
        }
    }
    DEBUG1("freertos_cancel_timer exit\n");
}

void freertos_del_timer_safe(_timer *ptimer)
{
    DEBUG1("freertos_del_timer_safe enter\n");
    uint8 bcancelled = 0;
    MEM_ASSERT((uint32)ptimer);
    if (ptimer->function)
    {
        freertos_cancel_timer(ptimer, &bcancelled);
        xTimerDelete(ptimer->os_timer, 0);
    }
    ptimer->function = (TIMER_FUN)NULL;
    DEBUG1(" freertos_del_timer_safe exit\n");
}

static uint32 _adc_srand32(void)
{
    uint32 tmp = nvic->SysTick.Value;
#if 0
    uint32 mask = 0x80000000;

    do
    {
        tmp |= (Get_Adc(ADC_Channel_1) & 0x01)? mask : 0; //set tmp based on adc's lsb
        mask = mask >> 1;  //shift to the next bit
    }
    while (mask);
#endif

    return tmp;
}

static int32 _freertos_arc4random()
{
    uint32 res = freertos_get_current_time();
    uint32 seed = _adc_srand32();

    seed = ((seed & 0x007F00FF) << 7) ^
           ((seed & 0x0F80FF00) >> 8) ^ // be sure to stir those low bits
           (res << 13) ^ (res >> 9);    // using the clock too!

    return (int)seed;
}

static int32 freertos_get_random_bytes(void* dst, uint32 size)
{
    uint32 ranbuf;
    uint32 *lp;
    int32 i, count;
    count = size / sizeof(uint32);
    lp = (uint32 *) dst;

    for(i = 0; i < count; i ++)
    {
        lp[i] = _freertos_arc4random();
        size -= sizeof(uint32);
    }

    if(size > 0)
    {
        ranbuf = _freertos_arc4random();
        freertos_memcpy(&lp[i], &ranbuf, size);
    }

    return 0;
}
#define PRIORITY_TO_NATIVE_PRIORITY(priority) (uint8)(configMAX_PRIORITIES - priority)

int32 freertos_create_task(_os_task *task, const uint8 *name, uint32 stack_size, uint32 queue_size,
                           uint32 priority, task_func_t func, void *ctx)
{
    int ret;
    DEBUG1("freertos_create_task enter\n");
    ret = xTaskCreate( (pdTASK_CODE)func, (const signed char * )name, (stack_size)/4, (void*)ctx, (unsigned portBASE_TYPE) PRIORITY_TO_NATIVE_PRIORITY(priority), *task /*rk add start*/, 0 /*rk add end*/);
    if(ret == pdPASS)
    {
        //printf("freertos_create_task ok stack size = %d\n",stack_size);
    }
    else
    {
        printf("freertos_create_task fail stack size = %d\n", stack_size);

    }
    DEBUG1("freertos_create_task exit\n");
//  wifi_task_stk_offset+=stk_size;

    return 1;

}

void freertos_delete_task(_os_task *task)
{
    MEM_ASSERT((uint32)*task);
    DEBUG1("freertos_create_task enter\n");
    vTaskDelete(*task);
    DEBUG1("freertos_create_task exit\n");
}

const struct os_api_ops os_api =
{

    freertos_malloc,
    freertos_zmalloc,
    freertos_mfree,
    freertos_memcpy,
    freertos_memcmp,
    freertos_memset,

    freertos_init_sema,
    freertos_free_sema,
    freertos_up_sema,
    freertos_down_sema,
    freertos_mutex_init,
    freertos_mutex_free,
    freertos_mutex_get,
    freertos_mutex_put,
    freertos_spinlock_init,
    freertos_spinlock_free,
    freertos_spin_lock,
    freertos_spin_unlock,
    freertos_spin_lock_irqsave,
    freertos_spin_unlock_irqsave,
    freertos_enter_critical,
    freertos_exit_critical,

    freertos_get_current_time,
    freertos_msleep,
    freertos_usleep,
    freertos_mdelay,
    freertos_udelay,

    freertos_yield,

    freertos_init_timer,
    freertos_set_timer,
    freertos_timer_pending,
    freertos_cancel_timer,
    freertos_del_timer_safe,

    freertos_get_random_bytes,

    freertos_create_task,
    freertos_delete_task,

    freertos_create_workqueue,
    freertos_destroy_workqueue,
    freertos_init_delayed_work,
    freertos_queue_delayed_work
};
//#pragma arm section code
#endif /*#ifdef __DRIVER_I6000_WIFI_C__*/