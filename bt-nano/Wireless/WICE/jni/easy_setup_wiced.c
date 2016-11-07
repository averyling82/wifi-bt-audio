/*
 * $ Copyright Broadcom Corporation $
 */

/** @file
 *
 */
#include "BspConfig.h"
#ifdef _DRIVER_WIFI__
#define NOT_INCLUDE_OTHER

#include "typedef.h"
#include "RKOS.h"
#include "global.h"
#include "SysInfoSave.h"
#include "device.h"
#include "DriverInclude.h"
#include "DeviceInclude.h"

#include "wiced.h"
#include "stdint.h"
#include "stddef.h"
//#include "wwd_network_interface.h"
#include "wwd_buffer_interface.h"
//#include "tlv.h"
//#include "wiced_security.h"

#include "../internal/wwd_sdpcm.h"
#include "wwd_wlioctl.h"
#include "wwd_events.h"

#include "easy_setup.h"

//extern easy_setup_result_t wifi_setup_result;
//extern wiced_security_t wifi_setup_security;
extern pTimer  TimerHandler;
extern WIFI_AP JoinAp;
extern WIFI_AP  easy_join_ap;
//#pragma arm section code = "ap6181wice_wifi_init", rodata = "ap6181wice_wifi_init", rwdata = "ap6181wice_wifi_init", zidata = "ap6181wice_wifi_init"

unsigned char easy_setup_flag = WICED_FALSE;
unsigned char easy_setup_stop_flag = 0;
//unsigned char easy_setup_deinit = 0;

#define PARAM_MAX_LEN (256)
#define RESULT_MAX_LEN (256)
typedef struct
{
    easy_setup_result_t result;
    easy_setup_param_t param;
    wiced_semaphore_t es_complete;
    wiced_semaphore_t scan_complete;
    wiced_security_t security;
    uint8_t channel;
    int abort_scan;
    uint8_t protocol;
} easy_setup_workspace_t;

// Trigger custom scan to fetch the security info of the AP
#define SCAN_TO_GET_SECURITY

#ifdef SCAN_TO_GET_SECURITY
static  wiced_scan_result_t  scan_result[1]= {0};
static  wiced_scan_result_t* result_ptr = (wiced_scan_result_t *) &scan_result;
#endif


/* error code in WICED/WWD/include/wwd_constants.h */
#define easy_setup_assert(s, v) \
do {\
    if (v != WWD_SUCCESS) {\
        WPRINT_WICED_INFO(("%s, %s: %d\n", __FUNCTION__, s, v));\
        WICED_ASSERTION_FAIL_ACTION();\
    }\
} while (0)

#define CHECK_IOCTL_BUFFER( buff )  \
    if ( buff == NULL ) {  \
        easy_setup_assert("Allocation failed", WWD_BUFFER_UNAVAILABLE_TEMPORARY);\
        return WWD_BUFFER_ALLOC_FAIL; \
    }

static const wwd_event_num_t easy_setup_events[] = { WLC_E_WAKE_EVENT, WLC_E_NONE };

uint16_t g_protocol_mask = 0x0;
static uint8_t g_protocol = 0;

static void* easy_setup_result_handler( const wwd_event_header_t* event_header,
    const uint8_t* event_data,
    void* handler_user_data )
{
    easy_setup_workspace_t *ws = (easy_setup_workspace_t *)handler_user_data;

    if (event_header->status == WLC_E_STATUS_SUCCESS)
    {
        WPRINT_WICED_INFO( ("\r\nEvent Found => Success\r\n") );
        memcpy( &ws->result, event_data, sizeof(easy_setup_result_t) );
        wiced_rtos_set_semaphore( &ws->es_complete );
        //easy_setup_stop( );
    }
    else if (event_header->status == WLC_E_STATUS_TIMEOUT)
    {
        WPRINT_WICED_INFO( ("\r\nEvent Found => Timeout\r\n") );
    }
    else
    {
        WPRINT_WICED_INFO( ("\r\nEvent Found => Unknown\r\n") );
    }
    return handler_user_data;
}

wwd_result_t easy_setup_restart( void )
{
    wiced_buffer_t buffer;
    wwd_result_t result;
    easy_setup_param_t *param;

    param = wwd_sdpcm_get_iovar_buffer( &buffer, sizeof(easy_setup_param_t), "easy_setup");
    CHECK_IOCTL_BUFFER( param );

    param->enable = EASY_SETUP_RESTART;

    result = wwd_sdpcm_send_iovar( SDPCM_SET, buffer, 0, WWD_STA_INTERFACE );
    //easy_setup_assert("Failed to set IOCTL", result);
    if(result != WWD_SUCCESS)
    {
         rk_printf("restart easy setup fail = %d", result);
    }
    return result;
}

int easy_setup_stop( void )
{
    wiced_buffer_t buffer;
    wwd_result_t result;
    easy_setup_param_t *p;
    p = wwd_sdpcm_get_iovar_buffer( &buffer, sizeof(easy_setup_param_t), "easy_setup");
    CHECK_IOCTL_BUFFER(p);
    p->enable = EASY_SETUP_STOP;

    result = wwd_sdpcm_send_iovar( SDPCM_SET, buffer, 0, WWD_STA_INTERFACE );
    //easy_setup_assert("Failed to set IOCTL", result);
    if(result != WWD_SUCCESS)
    {
         rk_printf("stop easy setup fail = %d", result);
    }
    return result;
}

wwd_result_t easy_setup_enable( uint16_t proto_mask, easy_setup_workspace_t *ws )
{
    wiced_buffer_t buffer;
    wwd_result_t result;
    easy_setup_param_t *p;

    result = wwd_management_set_event_handler( easy_setup_events, easy_setup_result_handler, ws, WWD_STA_INTERFACE );
   // easy_setup_assert("Failed to set Event Handler", result);
   if(result != WWD_SUCCESS)
   {
     rk_printf("Failed to set Event Handler = %d", result);
   }

    p = wwd_sdpcm_get_iovar_buffer( &buffer, PARAM_MAX_LEN, "easy_setup");
    CHECK_IOCTL_BUFFER(p);

    p->enable = EASY_SETUP_START;
    p->protocol_mask = proto_mask;

    int param_len = 0;
    param_len += ((uint8*)&p->param-(uint8*)p);

    tlv_t* t = &p->param;
    easy_setup_get_param(proto_mask, &t);

    result = wwd_sdpcm_send_iovar( SDPCM_SET, buffer, 0, WWD_STA_INTERFACE);
    //easy_setup_assert("Failed to set IOCTL", result);

    if(result != WWD_SUCCESS)
    {
        rk_printf("enable fail = %d", result);
    }

    return result;
}

int easy_setup_query( easy_setup_workspace_t *ws )
{
    wiced_buffer_t buffer;
    wiced_buffer_t response;
    wwd_result_t ret;
    easy_setup_result_t* r = &ws->result;

    UNUSED_PARAMETER(ws);

    (void)wwd_sdpcm_get_iovar_buffer(&buffer, RESULT_MAX_LEN, "easy_setup");
    ret = wwd_sdpcm_send_iovar(SDPCM_GET, buffer, &response, WWD_STA_INTERFACE );
    if ( ret == WWD_SUCCESS )
    {
        tlv_t* t = (tlv_t*) host_buffer_get_current_piece_data_pointer(response);
        memcpy(r, t->value, sizeof(easy_setup_result_t));
        WPRINT_WICED_INFO(("state [%d]\r\n", r->state));
        if (r->state == EASY_SETUP_STATE_DONE)
        {
            WPRINT_WICED_INFO(("Finished! protocol: %d\r\n", t->type));

            g_protocol = t->type;
            easy_setup_set_result(g_protocol, t->value);

            wiced_rtos_set_semaphore( &ws->es_complete );
        }

        host_buffer_release(response, WWD_NETWORK_RX);
    }

    return WICED_SUCCESS;
}

#ifdef SCAN_TO_GET_SECURITY
static void easy_scan_result_callback( wiced_scan_result_t** result_ptr, void* user_data, wiced_scan_status_t status  )
{
    easy_setup_workspace_t *ws = (easy_setup_workspace_t *)user_data;

    //printf("aa status = %d, ws->abort_scan = %d\n", status, ws->abort_scan);
    //UNUSED_PARAMETER(status);
#if 1
    if(status == WICED_SCAN_ABORTED ||status == WICED_SCAN_COMPLETED_SUCCESSFULLY)
    {
         printf("easy_scan_result_callback free ws->scan_complete\n");
         //wiced_rtos_set_semaphore(&ws->scan_complete);

         return;
    }
#endif
    if (ws->abort_scan == WICED_TRUE) {
        return;
    }

    if (NULL == result_ptr)
    {
        wiced_rtos_set_semaphore(&ws->scan_complete);
    }
    else
    {
    /*
        uint8_t* o = (*result_ptr)->BSSID.octet;
    WPRINT_WICED_INFO( ("scan cb, %02x:%02x:%02x:%02x:%02x:%02x\r\n", o[0], o[1], o[2], o[3], o[4], o[5]) );
    */
        /* Only look for a match on our locked BSSID */
        if (memcmp(ws->result.ap_bssid.octet, (*result_ptr)->BSSID.octet, sizeof(wiced_mac_t)) == 0)
        {
            ws->abort_scan = WICED_TRUE;
            ws->channel = (*result_ptr)->channel;
            ws->security = (*result_ptr)->security;
            //wiced_rtos_set_semaphore(&ws->scan_complete);
        }
    }
}
#endif

static int hex_str_to_int( const char* hex_str )
{
    int n = 0;
    uint32_t value = 0;
    int shift = 7;
    while ( hex_str[n] != '\0' && n < 8 )
    {
        if ( hex_str[n] > 0x21 && hex_str[n] < 0x40 )
        {
            value |= (uint32_t)(( hex_str[n] & 0x0f ) << ( shift << 2 ));
        }
        else if ( ( hex_str[n] >= 'a' && hex_str[n] <= 'f' ) || ( hex_str[n] >= 'A' && hex_str[n] <= 'F' ) )
        {
            value |= (uint32_t)(( ( hex_str[n] & 0x0f ) + 9 ) << ( shift << 2 ));
        }
        else
        {
            break;
        }
        n++;
        shift--;
    }

    return (int)( value >> ( ( shift + 1 ) << 2 ) );
}


int easy_setup_start()
{
    easy_setup_workspace_t* ws = NULL;
    easy_setup_result_t *result = NULL;
    wiced_bool_t led_flag;
    wiced_config_ap_entry_t* dct_ap_entry;
    wwd_result_t ret;
#ifdef SCAN_TO_GET_SECURITY
    wiced_mac_t bogus_scan_mac;
    wiced_ssid_t ssid;
    wiced_scan_extended_params_t extparam = { 5, 110, 110, 50 };
    int retry_times;
#endif
#ifdef AUTO_TEST
    int timeout_count = 0;
#endif
    //easy_setup_deinit = 1;

    if (!g_protocol_mask) {
        WPRINT_WICED_INFO(("No protocol enabled.\r\n"));
        return WICED_ERROR;
    }

    ws = malloc_named( "es", sizeof(easy_setup_workspace_t) );
    if ( ws == NULL )
    {
        return WICED_OUT_OF_HEAP_SPACE;
    }
    memset(ws, 0, sizeof(easy_setup_workspace_t));

    ws->abort_scan = WICED_FALSE;
    wiced_rtos_init_semaphore( &ws->es_complete);


    led_flag = WICED_FALSE;


    ret = easy_setup_enable(g_protocol_mask, ws);
    #if 1
    if(ret != WWD_SUCCESS)
    {
        rkos_sleep(2000);
        //rk_printf("easy_setup_enable fail");
        ret = easy_setup_restart();
        if(ret != WWD_SUCCESS)
        {
            wiced_rtos_deinit_semaphore( &ws->es_complete );
            rkos_memory_free( ws );
           // rkos_sleep(1000);
            easy_setup_stop( );
            return WICED_ERROR;
        }
    }
    #endif

    WPRINT_WICED_INFO( ("Enabled, waiting ...\r\n") );


    while (wiced_rtos_get_semaphore( &ws->es_complete, 500 * MILLISECONDS ) == WICED_TIMEOUT)
    {

        easy_setup_query( ws );
        //rk_printf("setstop = %d", easy_setup_stop_flag);
        if(easy_setup_stop_flag == 1)
        {

            wiced_rtos_set_semaphore( &ws->es_complete );
            easy_setup_stop( );
            wiced_rtos_deinit_semaphore( &ws->es_complete );
            rkos_memory_free( ws );
            //rk_printf("setstop11 = %d", easy_setup_stop_flag);
           // easy_setup_stop_flag = 0;
           // easy_setup_deinit = 0;
            return WWD_TIMEOUT;
        }

#ifdef AUTO_TEST
        timeout_count++;
        /* if each takes 500ms, then 60 means wait time is 500ms x 60 = 30s */
        if (timeout_count == 60)
        {
            wiced_rtos_set_semaphore( &ws->es_complete );
            easy_setup_stop();
            wiced_rtos_deinit_semaphore( &ws->es_complete );
            return WWD_TIMEOUT;
        }
#endif
    }


    wwd_management_set_event_handler( easy_setup_events, NULL, NULL, WWD_STA_INTERFACE );
    wiced_rtos_deinit_semaphore( &ws->es_complete );

    easy_setup_stop( );
    /* LED show status, could be removed */
    wiced_gpio_output_high( WICED_LED1 );



    rkos_memset(&easy_join_ap, 0x00, sizeof(WIFI_AP));
    result = &ws->result;
    easy_join_ap.ap_ssid_len = result->ap_ssid.len;
    rkos_memcpy(easy_join_ap.ap_ssid_value, result->ap_ssid.val, easy_join_ap.ap_ssid_len);
    easy_join_ap.security_key_length = result->security_key_length;
    rkos_memcpy(easy_join_ap.security_key, result->security_key, easy_join_ap.security_key_length);
    rkos_memcpy(easy_join_ap.ap_bssid, result->ap_bssid.octet, 6);
	/***********JJJHHH just for test*************/
	/*AIRMUSIC-HOME test airmusic   WICED_SECURITY_WPA2_MIXED_PSK  18 8	*/
	/*Baidu_music_0117 Good8888yeah  	WICED_SECURITY_WPA_TKIP_PSK  16 12*/
//	#define STA_TEST__
#ifdef STA_TEST__
	#define TEST_SSID "AIRMUSIC-HOME test"
	#define TEST_SSID_LEN 18
	#define TEST_PASSWORD "airmusic"
	#define TEST_PASSWORD_LEN 8 
	#define TEST_SECURITY_TYPE WICED_SECURITY_WPA2_MIXED_PSK
	
	easy_join_ap.ap_ssid_len =TEST_SSID_LEN; 
	easy_join_ap.security_key_length=TEST_PASSWORD_LEN;
	rkos_memcpy(easy_join_ap.ap_ssid_value, TEST_SSID, easy_join_ap.ap_ssid_len);
	rkos_memcpy(easy_join_ap.security_key, TEST_PASSWORD, easy_join_ap.security_key_length);

	result->ap_ssid.len =16; 
	rkos_memcpy(result->ap_ssid.val, TEST_SSID, TEST_SSID_LEN);
	rkos_memcpy(result->security_key, TEST_PASSWORD, TEST_PASSWORD_LEN);
#endif
	/************************/


    WPRINT_WICED_INFO( ("SSID        : %s\r\n", result->ap_ssid.val) );
    WPRINT_WICED_INFO( ("PASSWORD    : %.64s\r\n", result->security_key) );
    WPRINT_WICED_INFO( ("BSSID       : %02x:%02x:%02x:%02x:%02x:%02x\r\n",
                    (uint)result->ap_bssid.octet[0], (uint)result->ap_bssid.octet[1],
                    (uint)result->ap_bssid.octet[2], (uint)result->ap_bssid.octet[3],
                    (uint)result->ap_bssid.octet[4], (uint)result->ap_bssid.octet[5]));

    //wiced_rtos_init_semaphore(&ws->scan_complete);
    //wiced_rtos_get_semaphore( &ws->scan_complete, 200 * MILLISECONDS );
    //host_rtos_delay_milliseconds(2000);
#ifdef SCAN_TO_GET_SECURITY
    memcpy( &bogus_scan_mac, &result->ap_bssid, sizeof(wiced_mac_t) );
    memcpy( &ssid, &result->ap_ssid, sizeof(wiced_ssid_t) );

    wiced_rtos_init_semaphore( &ws->scan_complete );

    result_ptr = &scan_result[0];
    printf("scan_result[0] = 0x%4x   ssid=%s", result_ptr,result_ptr->SSID);
    /* we need to scan and get security & channel, will retry several times */
    retry_times = 3;
    do {
        wwd_wifi_scan( WICED_SCAN_TYPE_PASSIVE, WICED_BSS_TYPE_INFRASTRUCTURE, &ssid, &bogus_scan_mac, NULL, &extparam, easy_scan_result_callback, (wiced_scan_result_t **) &result_ptr, ws, WWD_STA_INTERFACE  );
        wiced_rtos_get_semaphore( &ws->scan_complete, 2000 * MILLISECONDS );
        retry_times--;
    } while ((ws->channel == 0) && (retry_times > 0));
    WPRINT_WICED_INFO( ("AP Channel  : %d\r\n", ws->channel) );
    WPRINT_WICED_INFO( ("AP Security : %s ",
                    ws->security == WICED_SECURITY_OPEN ? "Open" :
                    ws->security == WICED_SECURITY_WEP_PSK ? "WEP-OPEN" :
                    ws->security == WICED_SECURITY_WEP_SHARED ? "WEP-SHARED" :
                    ws->security == WICED_SECURITY_WPA_TKIP_PSK ? "WPA-PSK TKIP" :
                    ws->security == WICED_SECURITY_WPA_AES_PSK ? "WPA-PSK AES" :
                    ws->security == WICED_SECURITY_WPA2_AES_PSK ? "WPA2-PSK AES" :
                    ws->security == WICED_SECURITY_WPA2_TKIP_PSK ? "WPA2-PSK TKIP" :
                    ws->security == WICED_SECURITY_WPA2_MIXED_PSK ? "WPA2-PSK Mixed" :
                     "Unknown" ) );
     WPRINT_WICED_INFO( ("\r\n") );
#endif /* SCAN_TO_GET_SECURITY */
//   wifi_setup_security = ws->security;
#ifdef STA_TEST__//JJJHHH
    easy_join_ap.ap_security_type = TEST_SECURITY_TYPE;
#else
    easy_join_ap.ap_security_type = ws->security;
#endif

    wiced_rtos_deinit_semaphore(&ws->scan_complete);
    rkos_memory_free( ws );


    return WWD_SUCCESS;
}

int easy_setup_get_protocol(uint8_t* protocol) {
    *protocol = g_protocol;
    return 0;
}

//#pragma arm section code

#endif

