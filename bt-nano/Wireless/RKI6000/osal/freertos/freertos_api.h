#ifndef __FREERTOS_API_H_
#define __FREERTOS_API_H_

#include "wlan_include.h"
#include "freertos_types.h"

#define WIFI_TASK_STK_POOL_SIZE         (256*8)
#define WIFI_TASK_MAX   4


uint8* freertos_zmalloc(uint32 sz);

uint8* freertos_malloc(uint32 sz);

void freertos_mfree(uint8 *pbuf, uint32 sz);

void freertos_msleep(int32 ms);

void freertos_spinlock_init(_lock *plock);

void freertos_spinlock_free(_lock *plock);

void freertos_spin_lock(_lock   *plock);

void freertos_spin_unlock(_lock *plock);

void freertos_init_timer(_timer *ptimer,void *padapter, TIMER_FUN pfunc,void* cntx, const uint8 *name);

void freertos_set_timer(_timer *ptimer,uint32 delay_time);

boolean freertos_timer_pending(_timer *ptimer);

void freertos_cancel_timer(_timer *ptimer,uint8 *bcancelled);

void freertos_del_timer_safe(_timer *ptimer);

int32 freertos_create_task(_os_task *task, const uint8 *name, uint32 stack_size, uint32 queue_size,
                           uint32 priority, task_func_t func, void *ctx);

void freertos_delete_task(_os_task *task_tcb);

#endif

