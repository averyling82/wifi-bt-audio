/* Copyright (C) 2012 ROCK-CHIP FUZHOU. All Rights Reserved. */
/*
File: cgi.c
Desc:
Author: csx
Date: 2014-01-01
Notes:
*/

#include "BspConfig.h"
#include "source_macro.h"
#include <FreeRTOS.h>
#include <FreeRTOSConfig.h>
#include "httpd.h"
#include "cgi.h"
#include <opt.h>
#include <typedef.h>
#ifdef __WICE_HTTP_SERVER__
#ifdef USE_LWIP
#if LWIP_HTTPD
static char *
LoginCGIHandler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
static char *
PrevCGIHandler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
static char *
NextCGIHandler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);

char *tags[] = {
	{"ssid0"},	// SSI_INDEX_SSID0
	{"ssid1"},
	{"ssid2"},
	{"ssid3"},
	{"ssid4"},
	{"ssid5"},
	{"ssid6"},
	{"ssid7"},
	{"ssid8"},
	{"ssid9"},
	{"menu"},	// SSI_INDEX_MENU
};

tCGI cgis[] = {
	{"/login.cgi", LoginCGIHandler},	// CGI_INDEX_LOGIN
	{"/prev.cgi", PrevCGIHandler},		// CGI_INDEX_PREV
	{"/next.cgi", NextCGIHandler},		// CGI_INDEX_NEXT
};

static int iMenuCur = 1;
static int iMenuMax = 1;

static u16_t
SSIHandler(int iIndex, char *pcInsert, int iInsertLen)
{
	uint8 num_bsscfg;
	int num_menu = 0;
	int num_index;
	int ret = 0;

//	printf("SSIHandler: \n");

	//num_bsscfg = get_max_bsscfg();
	if (iIndex == SSI_INDEX_MENU) {
		taskENTER_CRITICAL();
		iMenuMax = (num_bsscfg + 10) / 10;
		if (iMenuMax == 0) {
			iMenuCur = 0;
		}
		taskEXIT_CRITICAL();
		sprintf(pcInsert, "%d/%d", iMenuCur, iMenuMax);
		ret = 3;
	} else {
		num_index = iIndex + ((iMenuCur - 1) * 10);
		if (num_index >= num_bsscfg || num_bsscfg == 0) {
			return 0;
		}
		//ret = get_bsscfg_print(num_index, pcInsert);
	}

	return ret;
}

static char *
LoginCGIHandler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
	char uri[] = "/runtime.shtml";

	rk_printf("LoginCGIHandler: \n");
	return uri;
}

static char *
PrevCGIHandler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
	char uri[] = "/index.shtml";

//	printf("PrevCGIHandler: \n");
	taskENTER_CRITICAL();
	if (iMenuCur != 0) {
		if (iMenuCur == 1) {
			iMenuCur = iMenuMax;
		} else {
			iMenuCur--;
		}
	}
	taskEXIT_CRITICAL();
	return uri;
}

static char *
NextCGIHandler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
	char uri[] = "/index.shtml";
	int i;

//	printf("NextCGIHandler: \n");
	taskENTER_CRITICAL();
	if (iMenuCur != 0) {
		if (iMenuCur == iMenuMax) {
			iMenuCur = 1;
		} else {
			iMenuCur++;
		}
	}
	taskEXIT_CRITICAL();
	return uri;
}

void
CGIInit(void)
{
	http_set_ssi_handler(SSIHandler, tags, SSI_INDEX_NUM);
	http_set_cgi_handlers(cgis, CGI_INDEX_NUM);
}
#endif

#endif
#endif