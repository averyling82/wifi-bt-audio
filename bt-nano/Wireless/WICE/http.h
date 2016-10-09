/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: WICE\http.h
* Owner: lyb
* Date: 2015.6.26
* Time: 15:22:25
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    lyb     2015.6.26     15:22:25   1.0
********************************************************************************************
*/


#ifndef __WICE_HTTP_H__
#define __WICE_HTTP_H__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/

#include <typedef.h>

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/



#define _WICE_HTTP_COMMON_  __attribute__((section("wice_http_common")))
#define _WICE_HTTP_INIT_  __attribute__((section("wice_http_init")))
//#define _WICE_HTTP_SHELL_  __attribute__((section("wice_http_shell")))

typedef struct _RK_TASK_HTTP_ARG
{
    uint32 http_arg;
}RK_TASK_HTTP_ARG;



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable declare
*
*---------------------------------------------------------------------------------------------------------------------
*/

typedef void (*HTTP_URL_CALLFUNCTION)(int status, void *httppcb);
typedef rk_err_t(*HTTP_WR_FUNCTION)(uint8 *buf, uint16 write_len, uint32 leave_len);
typedef int (*HTTP_UP_READFUNCTION)(uint8 *buf, uint16 read_len);
typedef void (*HTTP_UP_CALLFUNCTION)(int status, void *httpuppcb);

#define ERR_OK   0
#define FIFO_OPEN_ERR   -100
#define MALLOC_ERR -101
#define FIFO_WRITE_ERR -102
#define TCP_RECIVER_ERR -103
#define READ_DATA_ERR -104
#define DELEE_FLAG -105

#define FIFOWRITE 0
#define FILEWRITE 1


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API Declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
extern void HttpUp_Close(void *httpup_pcb);
extern void httpup_test(void);
extern void HTTPTaskUp_Enter(HDC httpup_pcb);
extern rk_err_t HttpPost_up(void *mhttpup_pcb, uint8 *host, uint32 port, uint8 *serverpath, uint8 *name, uint32 datalength);
extern void Http_Test(void);
extern HDC HttpPcb_New(HTTP_URL_CALLFUNCTION functioncallback, HTTP_WR_FUNCTION write_function, int flag);
extern void HTTPTask_Enter(HDC httppcb);
extern rk_err_t HTTPTask_DeInit(void *pvParameters);
extern rk_err_t HTTPTask_Init(void *pvParameters, void *arg);
extern void Http_Init(void);
extern rk_err_t HttpGet_Url(void *mHttpPcb,uint8 *pburl, uint32 StartLength);
extern void Http_Close(void *httppcb);

#endif
