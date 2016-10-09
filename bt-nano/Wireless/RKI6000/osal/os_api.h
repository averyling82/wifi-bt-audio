#ifndef __OS_API_H_
#define __OS_API_H_

#include <stddef.h> /* for size_t */

#include "debug_cfg.h"

#if defined(PLATFORM_OS_UCOS)
//#include "ucos_types.h"
#else

#endif
#include "wlan_intf_def.h"

#ifdef __func__
#define __WLAN_FUNC__   __func__
#elif defined __FUNC__
#define __WLAN_FUNC__   __FUNC__
#elif defined __function__
#define __WLAN_FUNC__   __function__
#elif defined __FUNCTION__
#define __WLAN_FUNC__   __FUNCTION__
#else
#define __STRING_FMT(x) #x
#define STRING_FMT(x) __STRING_FMT(x)
#define __FUNCTION__ "[Function@" __FILE__ " (" STRING_FMT(__LINE__) ")]"
#define __WLAN_FUNC__   __FUNCTION__
#endif


extern size_t strlen(const char *);


#define LIST_CONTAINOR(ptr, type, member) \
    ((type *)((uint8 *)(ptr)-(SIZE_T)((uint8 *)&((type *)ptr)->member - (uint8 *)ptr)))

#define time_after(a,b) ((long)(b) - (long)(a) < 0)


#define _INIT_WORK(_work, _func)                    \
    do {                                            \
        (_work)->func = (_func);                    \
        (_work)->data = NULL;                   \
    } while (0)

#define OS_INIT_DELAYED_WORK(_dwork, _func)     \
    do {                                            \
        _INIT_WORK(&(_dwork)->work, (_func));       \
    } while (0)


struct os_api_ops
{

    uint8 *(*_malloc)(uint32 sz);
    uint8 *(*_zmalloc)(uint32 sz);
    void (*_mfree)(uint8 *pbuf, uint32 sz);
    void (*_memcpy)(void* dst, void* src, uint32 sz);
    int32 (*_memcmp)(void *dst, void *src, uint32 sz);
    void (*_memset)(void *pbuf, int32 c, uint32 sz);

    void (*_init_sema)(_sema *sema, int32 init_val);
    void (*_free_sema)(_sema *sema);
    void (*_up_sema)(_sema *sema);
    void (*_down_sema)(_sema *sema);
    void (*_mutex_init)(_mutex *pmutex);
    void (*_mutex_free)(_mutex *pmutex);
    void (*_mutex_get)(_mutex *pmutex);
    void (*_mutex_put)(_mutex *pmutex);
    void (*_spinlock_init)(_lock *plock);
    void (*_spinlock_free)(_lock *plock);
    void (*_spin_lock)(_lock *plock);
    void (*_spin_unlock)(_lock *plock);
    void (*_spin_lock_irqsave)(_lock *plock, _irqL *irqL);
    void (*_spin_unlock_irqsave)(_lock *plock, _irqL *irqL);
    void (*_enter_critical)(_lock *plock, _irqL *pirqL);
    void (*_exit_critical)(_lock *plock, _irqL *pirqL);

    uint32  (*_get_current_time)(void);
    void (*_msleep)(int32 ms);
    void (*_usleep)(int32 us);
    void (*_mdelay)(int32 ms);
    void (*_udelay)(int32 us);

    void (*_yield)(void);

    void (*_init_timer)(_timer *ptimer, void *adapter, TIMER_FUN pfunc,void* cntx, const uint8 *name);
    void (*_set_timer)(_timer *ptimer,uint32 delay_time);
    boolean (*_timer_pending)(_timer *ptimer);
    void (*_cancel_timer)(_timer *ptimer,uint8 *bcancelled);
    void (*_del_timer)(_timer *ptimer);

    int32 (*_get_random_bytes)(void* dst, uint32 size);

    int32 (*_create_task)(_os_task *task, const uint8 *name, uint32 stack_size, uint32 queue_size,
                          uint32 priority, task_func_t func, void *ctx);
    void (*_delete_task)(_os_task *task);

    struct workqueue_struct *(*_create_workqueue)(const uint8 *name,
            uint32 stack_size, uint32 queue_size, uint32 priority);
    void (*_destroy_workqueue)(struct workqueue_struct *wq);
    void (* _init_delayed_work)(struct delayed_work *dwork, work_func_t func, void * work_data,
                                uint32 delay_time, const uint8 *timer_name);
    int32 (*_queue_delayed_work)(struct workqueue_struct *wq,
                                 struct delayed_work *dwork, uint32 delay, void* context);
};

uint8* os_malloc(uint32 sz);

uint8* os_zmalloc(uint32 sz);

void os_mfree(uint8 *pbuf, uint32 sz);

void os_memcpy(void* dst, void* src, uint32 sz);

int32 os_memcmp(void *dst, void *src, uint32 sz);

void os_memset(void *pbuf, int32 c, uint32 sz);

void os_init_sema(_sema *sema, int32 init_val);

void os_free_sema(_sema *sema);

void os_up_sema(_sema *sema);

void os_down_sema(_sema *sema);

void os_mutex_init(_mutex *pmutex);

void os_mutex_free(_mutex *pmutex);

void os_mutex_put(_mutex *pmutex);

void os_mutex_get(_mutex *pmutex);

void os_enter_critical(_lock *plock, _irqL *pirqL);

void os_exit_critical(_lock *plock, _irqL *pirqL);

void os_spinlock_init(_lock *plock);

void os_spinlock_free(_lock *plock);

void os_spin_lock(_lock *plock);

void os_spin_unlock(_lock *plock);

void os_spin_lock_irqsave(_lock *plock, _irqL *irqL);

void os_spin_unlock_irqsave(_lock *plock, _irqL *irqL);

uint32 os_get_current_time(void);

void os_msleep(int32 ms);

void os_usleep(int32 us);

void os_mdelay(int32 ms);

void os_udelay(int32 us);

void os_yield(void);

void os_init_timer(_timer *ptimer, void *adapter, TIMER_FUN pfunc,void* cntx, const uint8 *name);

void os_set_timer(_timer *ptimer,uint32 delay_time);

boolean os_timer_pending(_timer *ptimer);

void os_cancel_timer(_timer *ptimer,uint8 *bcancelled);

void os_del_timer(_timer *ptimer);

int32 os_get_random_bytes(void* dst, uint32 size);

int32 os_create_task(_os_task *task, const uint8 *name, uint32 stack_size, uint32 queue_size,
                     uint32 priority, task_func_t func, void *ctx);

void os_delete_task(_os_task *task);

struct workqueue_struct *os_create_workqueue(const uint8 *name,
        uint32 stack_size, uint32 queue_size, uint32 priority);

void os_destroy_workqueue(struct workqueue_struct *wq);

void os_init_delayed_work(struct delayed_work *dwork, work_func_t func, void * work_data,
                          uint32 delay_time, const uint8 *timer_name);

int32 os_queue_delayed_work(struct workqueue_struct *wq,
                            struct delayed_work *dwork, uint32 delay, void* context);


/*
 *  the following is os independent
 */
void os_init_listhead(_list *list);

uint32 os_list_empty(_list *phead);

void os_list_insert_head(_list *plist, _list *phead);

void os_list_insert_tail(_list *plist, _list *phead);

__inline static _list *os_list_get_next(_list   *list)
{
    return list->next;
}

void os_list_delete(_list *plist);

void    os_init_queue(_queue    *pqueue);

void    os_deinit_queue(_queue  *pqueue);

uint32 os_queue_empty(_queue    *pqueue);

uint32 os_end_of_queue(_list *head, _list *plist);

__inline static _list   *os_get_queue_head(_queue   *queue)
{
    return (&(queue->queue));
}

uint8 os_cancel_timer_ex(_timer *ptimer);

#endif

