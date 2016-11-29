#ifndef _DLNA_DESCRIPTIONS_CHANGE_H_
#define _DLNA_DESCRIPTIONS_CHANGE_H_

#include <cybergarage/upnp/std/av/cmediarenderer.h>

BOOL local_descriptions_load(CgUpnpDevice *dev);
int file_getlength(char *filename);
BOOL file_load(char * descriptions, char *filename, int filelen);


#endif