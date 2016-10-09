#include "wiced_resource.h"
#include "BspConfig.h"

#ifdef _DRIVER_WIFI__
#include "fw_AP6181.h"

const char wifi_firmware_image_data[1] = {
    0
};

//const resource_hnd_t wifi_firmware_image = { RESOURCE_IN_MEMORY, 206163, {.mem = { (const char *) wifi_fw }}};
//const resource_hnd_t wifi_firmware_image = { RESOURCE_IN_MEMORY, 210412, {.mem = { (const char *) wifi_fw }}};
#ifdef _WIFI_5G_AP6234
const resource_hnd_t wifi_firmware_image = { RESOURCE_IN_MEMORY, 367844, {.mem = { (const char *) wifi_fw }}};
#endif

#ifdef _WIFI_AP6181
const resource_hnd_t wifi_firmware_image = { RESOURCE_IN_MEMORY, 224548, {.mem = { (const char *) wifi_fw }}};
#endif

#ifdef _WIFI_5G_AP6255
const resource_hnd_t wifi_firmware_image = { RESOURCE_IN_MEMORY, 491205, {.mem = { (const char *) wifi_fw }}};
//const resource_hnd_t wifi_firmware_image = { RESOURCE_IN_MEMORY, 472267, {.mem = { (const char *) wifi_fw }}};
//const resource_hnd_t wifi_firmware_image = { RESOURCE_IN_MEMORY, 483181, {.mem = { (const char *) wifi_fw }}};
#endif

#ifdef _WIFI_AP6212
const resource_hnd_t wifi_firmware_image = { RESOURCE_IN_MEMORY, 362725, {.mem = { (const char *) wifi_fw }}};
#endif
#endif
