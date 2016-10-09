#ifndef __FREERTOS_WQ_H_
#define __FREERTOS_WQ_H_

/* implement queue work like linux style */

struct workqueue_struct * freertos_create_workqueue(const uint8 *name,
        uint32 stack_size, uint32 queue_size, uint32 priority);

void freertos_destroy_workqueue(struct workqueue_struct *wq);

void freertos_init_delayed_work(struct delayed_work *dwork, work_func_t func, void * work_data, uint32 delay_time, const uint8 *timer_name);

int32 freertos_queue_delayed_work(struct workqueue_struct *wq,
                                  struct delayed_work *dwork, uint32 delay, void* context);

boolean freertos_cancel_delayed_work(struct delayed_work *work);

#endif

