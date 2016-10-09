/*
 * Copyright 2015, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "wwd_rtos.h"
#include "wwd_debug.h"
#include "besl_host.h"

/******************************************************
 *                      Macros
 ******************************************************/
#define BESL_INFO(x)     WPRINT_SECURITY_INFO(x)
#define BESL_ERROR(x)    WPRINT_SECURITY_ERROR(x)
#define BESL_DEBUG(x)    WPRINT_SECURITY_DEBUG(x)


/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/


typedef struct
{
    host_thread_type_t         thread;
    void*                      thread_stack;
    host_queue_type_t          event_queue;
    besl_event_message_t       event_buffer[10];
    uint32_t                   timer_reference;
    uint32_t                   timer_timeout;
    wwd_interface_t            interface;
} besl_host_workspace_t;

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

#ifdef __cplusplus
} /*extern "C" */
#endif
