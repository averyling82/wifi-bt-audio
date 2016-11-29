#ifndef _DMS_ACTION_H_
#define  _DMS_ACTION_H_

#include <cybergarage/upnp/std/av/cmediarenderer.h>
//#include <server/dms_media_type.h>
#include <server/dms_filesys.h>

#define ROOT_PUBDIRECT "Rockchip"

BOOL dms_actionreceived(CgUpnpAction *action);
void dms_filesys_updaterootcontentlist(CgUpnpMediaServer *dms);

#endif