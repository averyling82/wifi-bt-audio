/*
 *
 *  Rockchip device media server.
 *
 *    2014-01-14 by rockchip IOT team.
 *
 *
 */

#ifndef _DMS_H_
#define _DMS_H_
#include <cybergarage/upnp/std/av/cmediarenderer.h>

extern CgUpnpAvServer * dms_create(char * suffix);
extern void dms_destroy(CgUpnpAvServer * dms);
extern void dms_restart(CgUpnpAvServer * dms);

#endif
