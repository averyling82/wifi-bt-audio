/* Copyright (C) 2012 ROCK-CHIP FUZHOU. All Rights Reserved. */
/*
File: sys_arch.c
Desc: Arch dependent types for the following objects:
    sys_sem_t, sys_mbox_t, sys_thread_t,
  And, optionally:
    sys_prot_t

  Defines to set vars of sys_mbox_t and sys_sem_t to NULL.
    SYS_MBOX_NULL NULL
    SYS_SEM_NULL NULL
Author: dgl
Date: 2013-11-20
Notes:
*/

/* lwIP includes. */

#include <stdio.h>
#include "lwip/debug.h"
#include "lwip/def.h"
#include "lwip/sys.h"
#include "lwip/mem.h"
#include "arch/sys_arch.h"
#include "opt.h"
#include "RKOS.h"

#ifdef USE_LWIP
//#pragma arm section code = "lwipcode", rodata = "lwipcode", rwdata = "lwipdata", zidata = "lwipdata"
/* Message queue constants. */
#define archMESG_QUEUE_LENGTH    (100)//( 6 )
#define archPOST_BLOCK_TIME_MS    ( ( unsigned portLONG ) 10000 )


struct timeoutlist
{
    //struct sys_timeouts timeouts;
    xTaskHandle pid;
};

/* This is the number of threads that can be started with sys_thread_new() */
#define SYS_THREAD_MAX 4

//static struct timeoutlist timeoutlist[SYS_THREAD_MAX];
//static u16_t nextthread = 0;
int intlevel = 0;

/*-----------------------------------------------------------------------------------*/
// Initialize sys arch
void
sys_init(void)
{

}
void sys_deinit( void )
{
}

/*-----------------------------------------------------------------------------------*/
//  Creates and returns a new semaphore. The "count" argument specifies
//  the initial state of the semaphore. TBD finish and test
err_t sys_sem_new(sys_sem_t *sem, u8_t count)
{
    err_t xReturn = ERR_MEM;


    //*sem = xSemaphoreCreateCounting(1,count);
    *sem = rkos_semaphore_create(1,count); // rk change 2015/11/23
    if (*sem != NULL)
        return ERR_OK;
    else
        return ERR_MEM;
#if 0
    vSemaphoreCreateBinary( *sem );

    if ( (*sem) != NULL )
    {
        if (count == 0)   // Means it can't be taken,»√÷µ±‰Œ™0
        {
            xSemaphoreTake(*sem,1);
        }
        xReturn = ERR_OK;
    }
    else
    {
        ;  // TBD need assert
    }

    return xReturn;
#endif
}


/*-----------------------------------------------------------------------------------*/
// Deallocates a semaphore
void sys_sem_free(sys_sem_t *sem)
{
//vQueueDelete( sem );
    //vSemaphoreDelete(*sem);
    rkos_semaphore_delete(*sem);// rk change 2015/11/23
}


/*-----------------------------------------------------------------------------------*/
// Signals a semaphore
void sys_sem_signal(sys_sem_t *sem)
{
    //xSemaphoreGive( *sem );
    rkos_semaphore_give(*sem);// rk change 2015/11/23
}


/*-----------------------------------------------------------------------------------*/
/*
  Blocks the thread while waiting for the semaphore to be
  signaled. If the "timeout" argument is non-zero, the thread should
  only be blocked for the specified time (measured in
  milliseconds).

  If the timeout argument is non-zero, the return value is the number of
  milliseconds spent waiting for the semaphore to be signaled. If the
  semaphore wasn't signaled within the specified time, the return value is
  SYS_ARCH_TIMEOUT. If the thread didn't have to wait for the semaphore
  (i.e., it was already signaled), the function may return zero.

  Notice that lwIP implements a function with a similar name,
  sys_sem_wait(), that uses the sys_arch_sem_wait() function.
*/
u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{
    portTickType StartTime, EndTime, Elapsed;
    unsigned long ulReturn;

    StartTime = xTaskGetTickCount();

    if (    timeout != 0)
    {
        //if( xSemaphoreTake( *sem, timeout/ portTICK_RATE_MS ) == pdTRUE )
        if ( rkos_semaphore_take( *sem, timeout/ portTICK_RATE_MS ) == RK_SUCCESS)// rk change 2015/11/23
        {
            EndTime = xTaskGetTickCount();
            Elapsed = (EndTime - StartTime)* portTICK_RATE_MS;
            if ( Elapsed == 0 )
            {
                Elapsed = 1;
            }
            ulReturn = Elapsed;
        }
        else
        {
            ulReturn = SYS_ARCH_TIMEOUT;
        }
    }
    else // must block without a timeout
    {
        //xSemaphoreTake(*sem, portMAX_DELAY);
        rkos_semaphore_take(*sem, portMAX_DELAY);// rk change 2015/11/23
        EndTime = xTaskGetTickCount();

        Elapsed = ( EndTime - StartTime ) * portTICK_RATE_MS;
        if ( Elapsed == 0 )
        {
            Elapsed = 1;
        }
        ulReturn = Elapsed;
    }

    return ulReturn ; // return time blocked
}

/*-----------------------------------------------------------------------------------*/
//  Creates an empty mailbox.
err_t sys_mbox_new(sys_mbox_t *mbox, int size)
{
    err_t xReturn = ERR_MEM;

    //*mbox = xQueueCreate( size, sizeof( void * ));
    *mbox = rkos_queue_create( size, sizeof( void * ));// rk change 2015/11/23

    if ( *mbox!=NULL )
    {
        xReturn = ERR_OK;
    }

    return xReturn;
}

/*-----------------------------------------------------------------------------------*/
/*
  Deallocates a mailbox. If there are messages still present in the
  mailbox when the mailbox is deallocated, it is an indication of a
  programming error in lwIP and the developer should be notified.
*/
void sys_mbox_free(sys_mbox_t *mbox)
{
    if ( uxQueueMessagesWaiting( *mbox ) )
    {
        /* Line for breakpoint.  Should never break here! */
//        __asm volatile ( "NOP" );
    }

    //vQueueDelete( *mbox );
    rkos_queue_delete( *mbox );// rk change 2015/11/23
}


/*-----------------------------------------------------------------------------------*/
//   Posts the "msg" to the mailbox.
void sys_mbox_post(sys_mbox_t *mbox, void *msg)
{
    //while ( xQueueSendToBack( *mbox, &msg, portMAX_DELAY ) != pdTRUE );
    while ( rkos_queue_send( *mbox, &msg, portMAX_DELAY ) != RK_SUCCESS );// rk change 2015/11/23
}

err_t sys_mbox_trypost(sys_mbox_t *mbox, void *msg)
{
    err_t xReturn;

    //if ( xQueueSend( *mbox, &msg, ( portTickType )0) == pdPASS)
    if ( rkos_queue_send( *mbox, &msg, ( portTickType )0) == RK_SUCCESS)// rk change 2015/11/23
    {
        xReturn = ERR_OK;
    }
    else
    {
        xReturn = ERR_MEM;
    }
    return xReturn;
}



/*-----------------------------------------------------------------------------------*/
/*
  Blocks the thread until a message arrives in the mailbox, but does
  not block the thread longer than "timeout" milliseconds (similar to
  the sys_arch_sem_wait() function). The "msg" argument is a result
  parameter that is set by the function (i.e., by doing "*msg =
  ptr"). The "msg" parameter maybe NULL to indicate that the message
  should be dropped.

  The return values are the same as for the sys_arch_sem_wait() function:
  Number of milliseconds spent waiting or SYS_ARCH_TIMEOUT if there was a
  timeout.

  Note that a function with a similar name, sys_mbox_fetch(), is
  implemented by lwIP.
*/
u32_t sys_arch_mbox_fetch(sys_mbox_t *mbox, void **msg, u32_t timeout)
{
    void *dummyptr;
    portTickType StartTime, EndTime, Elapsed;
    unsigned long ulReturn;

    StartTime = xTaskGetTickCount();

    if ( msg == NULL )
    {
        msg = &dummyptr;
    }

    if (    timeout != 0 )
    {
        //if (pdTRUE == xQueueReceive( *mbox, &(*msg), timeout / portTICK_RATE_MS )  )
        if (RK_SUCCESS == rkos_queue_receive( *mbox, &(*msg), timeout / portTICK_RATE_MS )  )// rk change 2015/11/23
        {
            EndTime = xTaskGetTickCount();
            Elapsed = (EndTime - StartTime)* portTICK_RATE_MS;
            if ( Elapsed == 0 )
            {
                Elapsed = 1;
            }

            ulReturn = Elapsed;
        }
        else // timed out blocking for message
        {
            *msg = NULL;
            ulReturn = SYS_ARCH_TIMEOUT;
        }
    }
    else // block forever for a message.
    {
        //while ( pdTRUE != xQueueReceive( *mbox, &(*msg), portMAX_DELAY ) );
        while ( RK_SUCCESS != rkos_queue_receive( *mbox, &(*msg), portMAX_DELAY ) );// rk change 2015/11/23

        EndTime = xTaskGetTickCount();
        Elapsed = (EndTime - StartTime)* portTICK_RATE_MS;
        if ( Elapsed == 0 )
        {
            Elapsed = 1;
        }
        ulReturn = Elapsed;
    }

    return ulReturn ; // return time blocked TBD test
}


u32_t sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg)
{
    void *pvDummy;
    unsigned long ulReturn;

    if ( msg== NULL )
    {
        msg = &pvDummy;
    }

    //if ( pdTRUE == xQueueReceive( *mbox, &(*msg), 0 ) )
    if ( RK_SUCCESS == rkos_queue_receive( *mbox, &(*msg), 0 ) )// rk change 2015/11/23
    {
        ulReturn = ERR_OK;
    }
    else
    {
        ulReturn = SYS_MBOX_EMPTY;
    }

    return ulReturn;
}



/** Create a new mutex
 * @param mutex pointer to the mutex to create
 * @return a new mutex */
err_t sys_mutex_new( sys_mutex_t *pxMutex )
{
    err_t xReturn = ERR_MEM;

    //*pxMutex = xSemaphoreCreateMutex();
    *pxMutex = rkos_mutex_create();// rk change 2015/11/23

    if ( *pxMutex != NULL )
    {
        xReturn = ERR_OK;

    }
    else
    {
        ;
    }

    return xReturn;
}

/** Lock a mutex
 * @param mutex the mutex to lock */
void sys_mutex_lock( sys_mutex_t *pxMutex )
{
    //while ( xSemaphoreTake( *pxMutex, portMAX_DELAY ) != pdPASS );
    while ( rkos_semaphore_take( *pxMutex, portMAX_DELAY ) != RK_SUCCESS );// rk change 2015/11/23
}

/** Unlock a mutex
 * @param mutex the mutex to unlock */
void sys_mutex_unlock(sys_mutex_t *pxMutex )
{
    //xSemaphoreGive( *pxMutex );
    rkos_semaphore_give( *pxMutex );// rk change 2015/11/23
}


/** Delete a semaphore
 * @param mutex the mutex to delete */
void sys_mutex_free( sys_mutex_t *pxMutex )
{
    //vQueueDelete( *pxMutex );
    rkos_queue_delete( *pxMutex );// rk change 2015/11/23
}

u32_t sys_now(void)
{
    return xTaskGetTickCount()*portTICK_RATE_MS;
}

/*-----------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------*/
// TBD
/*-----------------------------------------------------------------------------------*/
/*
  Starts a new thread with priority "prio" that will begin its execution in the
  function "thread()". The "arg" argument will be passed as an argument to the
  thread() function. The id of the new thread is returned. Both the id and
  the priority are system dependent.
*/
sys_thread_t sys_thread_new(const char *name, lwip_thread_fn thread, void *arg, int stacksize, int prio)
{
    xTaskHandle CreatedTask;
    portBASE_TYPE result;

    result = xTaskCreate(thread, ( signed char * )name, stacksize, arg, prio, &CreatedTask, NULL);

    if (result == pdPASS)
    {
        return CreatedTask;
    }
    else
    {
        return NULL;
    }
}
void sys_thread_free( sys_thread_t task )
{
//#if ( configFREE_TASKS_IN_IDLE == 0 )
    vTaskFreeTerminated( task );
//#endif /* if ( configFREE_TASKS_IN_IDLE == 0 ) */
}
/*
  This optional function does a "fast" critical region protection and returns
  the previous protection level. This function is only called during very short
  critical regions. An embedded system which supports ISR-based drivers might
  want to implement this function by disabling interrupts. Task-based systems
  might want to implement this by using a mutex or disabling tasking. This
  function should support recursive calls from the same task or interrupt. In
  other words, sys_arch_protect() could be called while already protected. In
  that case the return value indicates that it is already protected.

  sys_arch_protect() is only required if your port is supporting an operating
  system.
*/
sys_prot_t sys_arch_protect(void)
{
    vPortEnterCritical();
    return ( sys_prot_t ) 1;
}

/*
  This optional function does a "fast" set of critical region protection to the
  value specified by pval. See the documentation for sys_arch_protect() for
  more information. This function is only required if your port is supporting
  an operating system.
*/
void sys_arch_unprotect(sys_prot_t pval)
{
    ( void ) pval;
    vPortExitCritical();
}

/*
 * Prints an assertion messages and aborts execution.
 */
void sys_assert( const char *pcMessage )
{
    (void) pcMessage;

    for (;;)
    {
    }
}
//#pragma arm section code
#endif /*UES_LWIP*/