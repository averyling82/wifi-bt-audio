
#include "BspConfig.h"
#include <stdio.h>
#include "RKOS.h"


#ifdef _LOG_DEBUG_

#define DEBUG_TIME_LEN  8

uint8 BBDebugBuf[512];
uint8 BcoreDebuging;

void BBDebugReq(void);

#include <stdarg.h>
extern uint32 SysTickCounter2;

void rk_printf2(const char * fmt,...)
{
    uint8 buf[256];
    uint8 size;
    uint32 tmpcnt,i,j;
    uint8 dgbBuffer[DEBUG_TIME_LEN];
    va_list arg_ptr;

    tmpcnt = SysTickCounter2;
    for (i = 0; i < DEBUG_TIME_LEN; i++)
    {
        dgbBuffer[i] = tmpcnt % 10;
        tmpcnt = tmpcnt / 10;
    }

    buf[0] = '\r';
    buf[1] = '\n';
    buf[2] = '[';

    buf[3] = 'B';

    buf[4] = ']';

    buf[5] = '[';

    for (i = 0 ,j = 0; i < DEBUG_TIME_LEN; i++)
    {
        buf[j + 6] = dgbBuffer[DEBUG_TIME_LEN - 1 - i] + 0x30;
        if (DEBUG_TIME_LEN - 1 - i == 2)
        {
            j++;
            buf[j + 6] = '.';
        }
        j++;
    }

    buf[j + 6] = ']';

    va_start(arg_ptr, fmt);


    size =  vsprintf((char*)(buf + j + 7),(const char *)fmt, arg_ptr);

    memcpy(BBDebugBuf,buf, size + 16 + 2);

    BBDebugReq();
}

void printf2(const char * fmt,...)
{
    uint8 buf[256];
    uint8 size;
    uint32 tmpcnt,i,j;
    uint8 dgbBuffer[DEBUG_TIME_LEN];
    va_list arg_ptr;

    /*
    tmpcnt = SysTickCounter2;
    for (i = 0; i < DEBUG_TIME_LEN; i++)
    {
        dgbBuffer[i] = tmpcnt % 10;
        tmpcnt = tmpcnt / 10;
    }

    buf[0] = '\r';
    buf[1] = '\n';
    buf[2] = '[';

    buf[3] = 'B';

    buf[4] = ']';

    buf[5] = '[';

    for (i = 0 ,j = 0; i < DEBUG_TIME_LEN; i++)
    {
        buf[j + 6] = dgbBuffer[DEBUG_TIME_LEN - 1 - i] + 0x30;
        if (DEBUG_TIME_LEN - 1 - i == 2)
        {
            j++;
            buf[j + 6] = '.';
        }
        j++;
    }

    buf[j + 6] = ']';
    */

    va_start(arg_ptr, fmt);



    size =  vsprintf((char*)(buf),(const char *)fmt, arg_ptr);
    memset(BBDebugBuf,0,sizeof(BBDebugBuf));
    memcpy(BBDebugBuf,buf, size);

    BBDebugReq();
}

void rk_printf_no_time2(const char * fmt,...)
{
    uint8 buf[256];
    uint8 size;

    va_list arg_ptr;
    va_start(arg_ptr, fmt);

    memcpy(BBDebugBuf, "\r\n[B]", 5);

    size =  vsprintf((char*)buf,(const char *)fmt, arg_ptr);
    memcpy(BBDebugBuf,buf + 5, size + 2);


    BBDebugReq();

}


void rk_print_string2(const uint8 * pstr)
{

    if (pstr[0] == '\r')
    {
        memcpy(BBDebugBuf, "\r\n[B]", 5);

        memcpy(BBDebugBuf + 5, pstr + 2, strlen(pstr) - 2);

    }
    else
    {

        memcpy(BBDebugBuf + 5, pstr, strlen(pstr));

    }

    BBDebugReq();
}



void BBDebugReq(void)
{
    BcoreDebuging = 1;
    MailBoxWriteB2ACmd(MSGBOX_CMD_SYSTEM_PRINT_LOG, MAILBOX_ID_0, MAILBOX_CHANNEL_0);
    MailBoxWriteB2AData((uint32)BBDebugBuf, MAILBOX_ID_0, MAILBOX_CHANNEL_0);

    while (BcoreDebuging == 1)
    {
        __WFI2();
    }
}


void BBDebugOk(void)
{
    BcoreDebuging = 0;
}

void DebugInit2()
{
    BcoreDebuging = 0;
}

#endif

