
#ifndef RK_BT_API_H
#define RK_BT_API_H

#if 0
#include "bd_addr.h"
#include "btHwControl.h"
#include "hci.h"
#include "hci.h"
#include "l2cap.h"
#include "sdp.h"
#include "rfcomm.h"
#include "bt_pbuf.h"
#include "avdtp.h"
#include "avctp.h"
#include "avrcp.h"
#include "hsp_hfp.h"
#include "spp.h"
#include "bt_ip_dt.h"
#include "phybusif.h"
#include "opp.h"
#endif
#if 1
#include "btHwControl.h"

#ifdef LWIP_ERR_T
typedef LWIP_ERR_T err_t;
#else /* LWIP_ERR_T */
typedef int err_t;
#endif /* LWIP_ERR_T*/

/* Definitions for error constants. */

#define ERR_OK          0    /* No error, everything OK. */
#define ERR_MEM        -1    /* Out of memory error.     */
#define ERR_BUF        -2    /* Buffer error.            */
#define ERR_TIMEOUT    -3    /* Timeout.                 */
#define ERR_RTE        -4    /* Routing problem.         */

#define ERR_IS_FATAL(e) ((e) < ERR_RTE)

#define ERR_ABRT       -5    /* Connection aborted.      */
#define ERR_RST        -6    /* Connection reset.        */
#define ERR_CLSD       -7    /* Connection closed.       */
#define ERR_CONN       -8    /* Not connected.           */

#define ERR_VAL        -9    /* Illegal value.           */

#define ERR_ARG        -10   /* Illegal argument.        */

#define ERR_USE        -11   /* Address in use.          */

#define ERR_IF         -12   /* Low-level netif error    */
#define ERR_ISCONN     -13   /* Already connected.       */

#define ERR_INPROGRESS -14   /* Operation in progress    */

#define ERR_SDP_NO_FIND -15   /* sdp no found    */


///////////////////////HCI//////////////////////////////////////////////

#if 1
__packed
struct hci_inq_res
{
    struct hci_inq_res *next; /* For the linked list */

    struct bd_addr bdaddr; /* Bluetooth address of a device found in an inquiry */
    uint8 cod[3]; /* Class of the remote device */
    uint8 psrm; /* Page scan repetition mode */
    uint8 psm; /* Page scan mode */
    uint16 co; /* Clock offset */
    uint8 name[32]; /*name*/
};

int hci_sleep_conf(uint8 big_sleep_mode,uint8 deep_sleep_mode);
int hci_write_scan_enable(uint8 scan_enable);

void hci_sco_connect_notify(void (* sco_connected)(void));
void hci_sco_disconnect_notify ( void(* sco_disconnect)(void));
void hci_sco_recv( void(* sco_recv)(unsigned char *data, unsigned int len));
void hci_acl_disconnect_notify ( void(* acl_disconnect)(void));
void hci_acl_connected_notify ( void(* acl_connected)(void));
int  lp_sco_write(uint8 *data, uint16 len, void (*func)(void));
int hci_enable_device_under_test_mode_command(void);

typedef enum
{

    HCI_APP_EVENT_ACL_CONNECTED = 0x01,
    HCI_APP_EVENT_ACL_DISCONNECTED,
    HCI_APP_EVENT_SCO_CONNECTED,
    HCI_APP_EVENT_SCO_DISCONNECTED,
    HCI_APP_EVENT_BLE_CONNECTED,
    HCI_APP_EVENT_BLE_DISCONNECTED,

} HCI_EVENT_CODE;
#endif



/*-----------------------------------------------------------------------------------*/
/* hci_delete_link_key():
 *
 * delete_all_flag 0: Delete only the Link Key for specified BD_ADDR
                   1: Delete all stored Link Keys
 */
/*-----------------------------------------------------------------------------------*/
int hci_delete_link_key(struct bd_addr *bdaddr, uint8 delete_all_flag) ;
///////////////////////HFP//////////////////////////////////////////////

#define HFP_SERVICE_NO_AVAILABLE    0
#define HFP_SERVICE_AVAILABLE        1

#define HFP_SERVICE_NO_In_Band    0
#define HFP_SERVICE_In_Band        1

#define HFP_NO_CALL              0
#define HFP_CALL_PLAYING          1

#define HFP_CALL_NO_SET_UP             0
#define HFP_CALL_INCOMING_SET_UP      1
#define HFP_CALL_OUTGOING_SET_UP      2
#define HFP_CALL_OUTGOING_ALERT       3

#define HFP_CALL_NO_HELD               0
#define HFP_CALL_IS_HELD               1





extern uint8 hfp_get_call_status();
extern uint8 hfp_get_callsetup_status();
extern uint8 hfp_get_call_setup_status();
extern uint8 hfp_get_signal_status();
extern uint8 hfp_get_callheld_status();
extern uint8 hfp_get_service_status();
extern uint8 hfp_get_battchg_status();
extern uint8  hfp_get_In_Band_Ringing_status();
void hfp_Answer_incoming_call ();
void hfp_Reject_Terminate_calling ();
void hfp_Redial_Last_Number();

void hsp_hfp_deinit(void);


void hfp_call_incoming_notify(void(*call_incoming_notify)(void));
void hfp_call_setup_notify(void(*call_setup_notify)(void));
void hfp_call_hangup_notify(void(*call_hangup_notify)(void));
void hfp_call_ring_notify(void(*call_ring_notify)(void));
void hfp_connected_notify(void(*hfp_connected_notify)(void));
void hfp_register_event_handler(void (* hfp_event_handler)(uint8 event_type, uint8 *parameter, uint16 param_size));

typedef enum
{
    HFP_EVENT_CALL_STATUS_CHANGE = 0x01,
    HFP_EVENT_SETUP_STATUS_CHANGE,
    HFP_EVENT_CALL_RING,
    HFP_EVENT_CONNECTED,
    HFP_EVENT_DISCONNECTED,
    HFP_EVENT_GET_PHONE_NUMBER,
} HFP_EVENT_CODE;

///////////////////////AVDTP//////////////////////////////////////////////

void avdtp_streaming_notify(void(*streaming_notify)(void));
void avdtp_suspend_notify(void(*suspend_notify)(void));
void avdtp_close_notify(void(*close_notify)(void));
void avdtp_abort_notify(void(*abort_notify)(void));
void avdtp_connected_notify(void(*connected_notify)(void));
void avdtp_media_recv(void(*media_data_recv)(char * buf, int len));
int avdtp_connect(struct bd_addr *bdaddr, void(*connect_result)(int result));
uint16 get_avdtp_state(void);
uint8 avdtp_get_codec_type(void);
void avdtp_tmr(void);
void avdtp_register_event_handler(void (* avdtp_event_handler)(uint8 event_type, uint8 *parameter, uint16 param_size));
/* SBC CODEC definitions */
#define AVDTP_AUDIO_CODEC_SBC               0x00
#define AVDTP_AUDIO_CODEC_MP3               0x01
#define AVDTP_AUDIO_CODEC_AAC               0x02

typedef enum
{

    AVDTP_EVENT_CONNECTED = 0x01,
    AVDTP_EVENT_DISCONNECTED,
    AVDTP_EVENT_STREAMING,
    AVDTP_EVENT_SUSPEND,
    AVDTP_EVENT_CLOSE,
    AVDTP_EVENT_ABORT,
} AVDTP_EVENT_CODE;

///////////////////////AVCTP/////////////////////////////////////////////////

void avctp_connected_notify(void(*connected_notify)(void));
void avctp_close_notify(void(*close_notify)(void));
void avctp_audio_play_notify(void(*audio_play_notify)(void));
void avctp_audio_pause_notify(void(*audio_pause_notify)(void));

void avctp_audio_next_notify(void(*audio_next_notify)(void));
void avctp_audio_previous_notify(void(*audio_previous_notify)(void));
void avctp_audio_stop_notify(void(*audio_stop_notify)(void));

void avctp_register_event_handler(void (* avctp_event_handler)(uint8 event_type, uint8 *parameter, uint16 param_size));
typedef enum
{

    AVCTP_EVENT_CONNECTED = 0x01,
    AVCTP_EVENT_DISCONNECTED,

} AVCTP_EVENT_CODE;
///////////////////////phybusif//////////////////////////////////////////////

/* Application program's interface: */
err_t phybusif_init(bt_hw_control_t * hw_control); /* Must be called first to initialize the physical bus interface */
int phybusif_reset(struct phybusif_cb *cb);
int phybusif_input(unsigned char c);

void hci_serve(void); //wp add
void cc2564_reset(void);
void cc2564_init(uint32 script_module_id);

/* Upper layer interface: */
void _phybusif_output(struct pbuf *p, uint16 len, void (*func)(void)) ;
#define  phybusif_output(p, len)   _phybusif_output(p, len , NULL)
uint32 HciServeIsrDisable(void);
void HciServeIsrEnable(void);
void send_go_to_sleep_ack();

void send_wake_up_ind(void);
void send_wake_up_ack(void);
void hcill_go_to_sleep_ind_notify(void(*hcill_go_to_sleep_ind_notify)(void));
void hcill_wake_up_ind_notify(void(*hcill_wake_up_ind_notify)(void));
void hcill_wake_up_ack_notify(void(*hcill_wake_up_ack_notify)(void));

void phybusif_output_insleep_proc(void(*output_insleep_proc)(void));
void phybusif_get_cc2564_sleep_state(uint8(*get_cc2564_sleep_state)(void));




void hci_acl_connnect_req(void (* acl_connect_req)(struct bd_addr *bdaddr));

void bt_init(uint32 flag);

void bt_start(uint32 flag);

void bt_restart(void);

void bt_time_serve(void);
void bt_deinit(void);
void bt_set_local_name(char * name, int namelen); //需要在bt_start 之前调用
void bt_set_pin_code(char * pincode, int len);
int  bt_disconnect(struct bd_addr *bdaddr);


void bt_discoverable_enable(void);
void bt_discoverable_disable(void);
void bt_sleep_enable(void);
void bt_sleep_disable(void);
void bt_a2dp_connect(struct bd_addr *bdaddr, void(*connect_result_hook)(uint result));
void bt_scan(void (* scan_result_hook)(struct hci_inq_res *ires, uint16 devCnt, int isComplete));
void bt_a2dp_disconnect(struct bd_addr *bdaddr, void(*disconnect_result_hook)(int result));
void bt_clean_scan_result(void);
void bt_set_init_complete_hook(void(*bt_init_complete_hook)(void));

void bt_scan_cancel(void);
void bt_read_rssi(struct bd_addr *bdaddr, void(*reesult_hook)(struct bd_addr *bdaddr, int8 rssi));
int bt_wait_hci_cmd_init_complete(void);
/* ---------- bt_init flag ---------- */

#define    ENABLE_A2DP  (1<< 0)
#define    ENABLE_HFP   (1<< 1)
#define    ENABLE_SPP   (1<< 2)
#define    ENABLE_AUTO_CONNECT (1<< 3)
#define    ENABLE_CMD_COMPLETE (1<< 4)

/* ---------- bt_start flag ---------- */

#define    BT_START_FLAG_DEFAULT (0)
#define    SEND_HCI_RESET   (1<< 0)




///////////////////////phybusif/////////////////////////////////////////////


///////////////////////SPP/////////////////////////////////////////////////
#define SPP_SEND_MAX        600

int spp_send(uint8 *data, int len, void (*func)(void)); //send data to spp ,when spp not connected it  return  ERR; the data len must be < SPP_SEND_MAX
uint8 spp_connected_state(void); //0: not connened   1: connected
void spp_data_recv(void(*spp_recv_func)(char * buf, int len)); //this hook func will be call when spp receive data, APP can use it to receive data from spp


void spp_register_event_handler(void (* spp_event_handler)(uint8 event_type, uint8 *parameter, uint16 param_size,err_t err));

typedef enum
{

    SPP_EVENT_CONNECTED = 0x01,
    SPP_EVENT_DISCONNECTED,

} SPP_EVENT_CODE;

///////////////////////SPP/////////////////////////////////////////////////



///////////////////////BLE SPP/////////////////////////////////////////////////
void blespp_register_packet_recv(void(*blespp_data_recv)(char * buf, int len));
int blespp_send(char * buf, int len);
int blespp_connected_state(void);

///////////////////////BLE SPP/////////////////////////////////////////////////

#endif


#endif
