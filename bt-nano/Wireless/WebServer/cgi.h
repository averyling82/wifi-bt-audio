/* Copyright (C) 2012 ROCK-CHIP FUZHOU. All Rights Reserved. */
/*
File: cgi.h
Desc:
Author: csx
Date: 2014-01-01
Notes:
*/
#ifndef __CGI_H__
#define __CGI_H__

enum {
	SSI_INDEX_SSID0,
	SSI_INDEX_SSID1,
	SSI_INDEX_SSID2,
	SSI_INDEX_SSID3,
	SSI_INDEX_SSID4,
	SSI_INDEX_SSID5,
	SSI_INDEX_SSID6,
	SSI_INDEX_SSID7,
	SSI_INDEX_SSID8,
	SSI_INDEX_SSID9,
	SSI_INDEX_MENU,

	/* don't remove */
	SSI_INDEX_MAX,
};

#define SSI_INDEX_NUM	SSI_INDEX_MAX

enum {
	CGI_INDEX_LOGIN,
	CGI_INDEX_PREV,
	CGI_INDEX_NEXT,

	/* don't remove */
	CGI_INDEX_MAX,
};

#define CGI_INDEX_NUM	CGI_INDEX_MAX

void CGIInit(void);

#endif /* __CGI_H__ */