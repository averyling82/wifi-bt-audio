#include "wiced_resource.h"
#include "BspConfig.h"

#ifdef _CHIP_AP6212_WIFI_C__
#include "fw_AP6212.h"

const char wifi_firmware_image_data[1] = {
    0
};

//const resource_hnd_t wifi_firmware_image = { RESOURCE_IN_MEMORY, 206163, {.mem = { (const char *) wifi_fw }}};
//const resource_hnd_t wifi_firmware_image = { RESOURCE_IN_MEMORY, 210412, {.mem = { (const char *) wifi_fw }}};
const resource_hnd_t wifi_firmware_image = { RESOURCE_IN_MEMORY, 224548, {.mem = { (const char *) wifi_fw }}};

#endif
