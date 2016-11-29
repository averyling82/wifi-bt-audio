/*
 *
 *  Rockchip device media renderer.
 *
 *    2014-01-16 by rockchip IOT team.
 *
 *
 */
#ifndef _DMR_H_
#define _DMR_H_
#include <cybergarage/upnp/std/av/cmediarenderer.h>

extern CgUpnpAvRenderer * dmr_create(char * suffix);
extern void dmr_destroy(CgUpnpAvRenderer * dmr);
extern void dmr_restart(CgUpnpAvRenderer * dmr);
extern int dmr_set_volume(CgUpnpAvRenderer *dmr, int volume);

#endif
