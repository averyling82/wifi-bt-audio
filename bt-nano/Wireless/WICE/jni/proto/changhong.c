#include "BspConfig.h"
#ifdef _DRIVER_WIFI__
#include <string.h>

#include "changhong.h"

//#pragma arm section code = "ap6181wifiCode", rodata = "ap6181wifiCode", rwdata = "ap6181wifidata", zidata = "ap6181wifidata"

static changhong_result_t g_changhong_result = {0};

void changhong_set_result(const void* p) {
    memcpy(&g_changhong_result, p, sizeof(g_changhong_result));
    LOGE("changhong_set_result: state: %d\n", g_changhong_result.es_result.state);
}

int changhong_get_sec_mode(uint8* sec) {
    if (g_changhong_result.es_result.state != EASY_SETUP_STATE_DONE) {
        LOGE("easy setup data unavailable\n");
        return -1;
    }

    *sec = g_changhong_result.sec_mode;
    return 0;
}

//#pragma arm section code

#endif

