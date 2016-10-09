/*
 * Copyright (c) 2003 EISLAB, Lulea University of Technology.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwBT Bluetooth stack.
 *
 * Author: Conny Ohult <conny@sm.luth.se>
 *
 */

#ifndef __HCI_H__
#define __HCI_H__
//#include "BlueToothControl.h"
//#include "lwip/def.h"
#include "bt_pbuf.h"
#include "lwbterr.h"

#include "bd_addr.h"
#include "lwbtopts.h"
//bt stack
typedef enum
{
    HCI_STATE_OFF = 0,
    HCI_STATE_INITIALIZING,
    HCI_STATE_WORKING,
    HCI_STATE_HALTING,
    HCI_STATE_SLEEPING,
    HCI_STATE_FALLING_ASLEEP
} HCI_STATE;

// calculate combined ogf/ocf value
#define OPCODE(ogf, ocf) (ocf | ogf << 10)

#define BTSTACK_EVENT_STATE                                0x60



#define HCI_SCO_MAX_NUMBER 3

#define HCI_CONNECT_TYPE_ACL  0x01
#define HCI_CONNECT_TYPE_SCO  0x00
#define HCI_CONNECT_TYPE_ESCO 0x02

#define HCI_CONNECT_TYPE_BLEACL 0x04
// Security Manager protocol data

// Attribute protocol data
#define ATT_DATA_PACKET         0x08
#define SM_DATA_PACKET          0x09

#define ATT_HANDLE_VALUE_INDICATION_IN_PORGRESS            0x90
#define ATT_HANDLE_VALUE_INDICATION_TIMEOUT                0x91

#define GATT_CLIENT_BUSY                                   0x94


/**
 * @format H1
 * @param handle
 * @param status
 */
#define GATT_QUERY_COMPLETE                                0xA0

/**
 * @format HX
 * @param handle
 * @param service
 */
#define GATT_SERVICE_QUERY_RESULT                          0xA1

/**
 * @format HY
 * @param handle
 * @param characteristic
 */
#define GATT_CHARACTERISTIC_QUERY_RESULT                   0xA2

/**
 * @format HX
 * @param handle
 * @param service
 */
#define GATT_INCLUDED_SERVICE_QUERY_RESULT                 0xA3

/**
 * @format HY
 * @param handle
 * @param characteristic_descriptor
 */
#define GATT_ALL_CHARACTERISTIC_DESCRIPTORS_QUERY_RESULT   0xA4

/**
 * @format H2LV
 * @param handle
 * @param value_handle
 * @param value_length
 * @param value
 */
#define GATT_CHARACTERISTIC_VALUE_QUERY_RESULT             0xA5

/**
 * @format H2LV
 * @param handle
 * @param value_handle
 * @param value_length
 * @param value
 */
#define GATT_LONG_CHARACTERISTIC_VALUE_QUERY_RESULT        0xA6

#define GATT_NOTIFICATION                                  0xA7
#define GATT_INDICATION                                    0xA8

#define GATT_CHARACTERISTIC_DESCRIPTOR_QUERY_RESULT        0xA9
#define GATT_LONG_CHARACTERISTIC_DESCRIPTOR_QUERY_RESULT   0xAA

// data: event(8), len(8), status (8), hci_handle (16), attribute_handle (16)
#define ATT_HANDLE_VALUE_INDICATION_COMPLETE               0xB6

// data: event(8), address_type(8), address (48), [number(32)]
#define SM_JUST_WORKS_REQUEST                              0xD0
#define SM_JUST_WORKS_CANCEL                               0xD1
#define SM_PASSKEY_DISPLAY_NUMBER                          0xD2
#define SM_PASSKEY_DISPLAY_CANCEL                          0xD3
#define SM_PASSKEY_INPUT_NUMBER                            0xD4
#define SM_PASSKEY_INPUT_CANCEL                            0xD5
#define SM_IDENTITY_RESOLVING_STARTED                      0xD6
#define SM_IDENTITY_RESOLVING_FAILED                       0xD7
#define SM_IDENTITY_RESOLVING_SUCCEEDED                    0xD8
#define SM_AUTHORIZATION_REQUEST                           0xD9
#define SM_AUTHORIZATION_RESULT                            0xDA


// last error code in 2.1 is 0x38 - we start with 0x50 for BTstack errors
#define BTSTACK_CONNECTION_TO_BTDAEMON_FAILED              0x50
#define BTSTACK_ACTIVATION_FAILED_SYSTEM_BLUETOOTH         0x51
#define BTSTACK_ACTIVATION_POWERON_FAILED                  0x52
#define BTSTACK_ACTIVATION_FAILED_UNKNOWN                  0x53
#define BTSTACK_NOT_ACTIVATED                              0x54
#define BTSTACK_BUSY                                       0x55
#define BTSTACK_MEMORY_ALLOC_FAILED                        0x56
#define BTSTACK_ACL_BUFFERS_FULL                           0x57

// l2cap errors - enumeration by the command that created them
#define L2CAP_COMMAND_REJECT_REASON_COMMAND_NOT_UNDERSTOOD 0x60
#define L2CAP_COMMAND_REJECT_REASON_SIGNALING_MTU_EXCEEDED 0x61
#define L2CAP_COMMAND_REJECT_REASON_INVALID_CID_IN_REQUEST 0x62

#define L2CAP_CONNECTION_RESPONSE_RESULT_SUCCESSFUL        0x63
#define L2CAP_CONNECTION_RESPONSE_RESULT_PENDING           0x64
#define L2CAP_CONNECTION_RESPONSE_RESULT_REFUSED_PSM       0x65
#define L2CAP_CONNECTION_RESPONSE_RESULT_REFUSED_SECURITY  0x66
#define L2CAP_CONNECTION_RESPONSE_RESULT_REFUSED_RESOURCES 0x65
#define L2CAP_CONNECTION_RESPONSE_RESULT_RTX_TIMEOUT       0x66

#define L2CAP_CONFIG_RESPONSE_RESULT_SUCCESSFUL            0x67
#define L2CAP_CONFIG_RESPONSE_RESULT_UNACCEPTABLE_PARAMS   0x68
#define L2CAP_CONFIG_RESPONSE_RESULT_REJECTED              0x69
#define L2CAP_CONFIG_RESPONSE_RESULT_UNKNOWN_OPTIONS       0x6a
#define L2CAP_SERVICE_ALREADY_REGISTERED                   0x6b

#define RFCOMM_MULTIPLEXER_STOPPED                         0x70
#define RFCOMM_CHANNEL_ALREADY_REGISTERED                  0x71
#define RFCOMM_NO_OUTGOING_CREDITS                         0x72
#define RFCOMM_AGGREGATE_FLOW_OFF                          0x73

#define SDP_HANDLE_ALREADY_REGISTERED                      0x80
#define SDP_QUERY_INCOMPLETE                               0x81

#define ATT_HANDLE_VALUE_INDICATION_IN_PORGRESS            0x90
#define ATT_HANDLE_VALUE_INDICATION_TIMEOUT                0x91

#define GATT_CLIENT_BUSY                                   0x94



struct hci_pcb;
struct hci_link;
struct hci_inq_res;

/*  Functions for interfacing with HCI */
err_t hci_init(void); /* Must be called first to initialize HCI */
void hci_deinit(void);

/* Application program's interface: */
err_t hci_close(struct hci_link *link);
void hci_reset_all(void);
void hci_arg(void *arg);
void hci_cmd_complete(err_t (* cmd_complete)(void *arg, struct hci_pcb *pcb,
                      uint8 ogf, uint8 ocf, uint8 result));
void hci_pin_req(err_t (* pin_req)(void *arg, struct bd_addr *bdaddr));
void hci_link_key_not(err_t (* link_key_not)(void *arg, struct bd_addr *bdaddr, uint8 *key));
void  hci_wlp_complete(err_t (* wlp_complete)(void *arg, struct bd_addr *bdaddr));
void hci_connection_complete(err_t (* conn_complete)(void *arg, struct bd_addr *bdaddr));
#define hci_num_cmd(pcb) ((pcb)->numcmd)
#define hci_num_acl(pcb) ((pcb)->hc_num_acl)
#define hci_maxsize(pcb) ((pcb)->maxsize)
err_t hci_inquiry(uint32 lap, uint8 inq_len, uint8 num_resp,
                  err_t (* inq_complete)(void *arg, struct hci_pcb *pcb, struct hci_inq_res *ires,
                          uint16 result));

//err_t hci_disconnect(struct bd_addr *bdaddr, uint8 reason);
int hci_disconnect(struct bd_addr *bdaddr, uint8 reason, uint8 link_type);

err_t hci_pin_code_request_reply(struct bd_addr *bdaddr, uint8 pinlen, uint8 *pincode);
err_t hci_pin_code_request_neg_reply(struct bd_addr *bdaddr);
err_t hci_write_stored_link_key(struct bd_addr *bdaddr, uint8 *key);
err_t hci_change_local_name(uint8 *name, uint8 len);
err_t hci_sniff_mode(struct bd_addr *bdaddr, uint16 max_interval, uint16 min_interval,  uint16 attempt,
                     uint16 timeout);
err_t hci_write_link_policy_settings(struct bd_addr *bdaddr, uint16 link_policy);
err_t hci_reset(void);
err_t hci_set_event_filter(uint8 filter_type, uint8 filter_cond_type, uint8* cond);
err_t hci_write_page_timeout(uint16 page_timeout);
err_t hci_write_scan_enable(uint8 scan_enable);
err_t hci_write_cod(uint8 *cod);
err_t hci_set_hc_to_h_fc(void);
err_t hci_host_buffer_size(void);
err_t hci_host_num_comp_packets(uint16 conhdl, uint16 num_complete);
err_t hci_read_buffer_size(void);
err_t hci_read_bd_addr(err_t (* rbd_complete)(void *arg, struct bd_addr *bdaddr));

/* Lower layers interface */
void hci_acl_input(struct pbuf *p);
void hci_event_input(struct pbuf *p);

/* L2CAP interface */
err_t lp_write_flush_timeout(struct bd_addr *bdaddr, uint16 flushto);
//err_t lp_acl_write(struct bd_addr *bdaddr, struct pbuf *p, uint16 len, uint8 pb);
int lp_acl_write(struct bd_addr *bdaddr, struct pbuf *p, uint16 len, uint8 pb, void (*func)(void));

err_t lp_connect_req(struct bd_addr *bdaddr, uint8 allow_role_switch);
uint8 lp_is_connected(struct bd_addr *bdaddr);
uint16 lp_pdu_maxsize(void);

// rk add
int hci_write_link_supervision_timeout(uint16 link_supervision_timeout);
int hci_exit_sniff_mode(uint16 conhdl);
int hci_write_extended_inquiry_response_commad(uint8 FEC_Required, char * response_data );
int hci_read_rssi_command(struct bd_addr * bdaddr);
int hci_write_inquiry_scan_activity(uint16 scan_interva, uint16 scan_window);
int hci_accept_connection_Request(struct bd_addr *bdaddr, uint8 role);
int hci_write_page_scan_activity(uint16 scan_interva, uint16 scan_window);
void hci_link_key_request(int (* link_key_request)(void *arg, struct bd_addr *bdaddr));
int hci_Io_Capability_request_reply(struct bd_addr *bdaddr, uint8 Io_Capability, uint8 OOB_Data, uint8 Auth_Req);
int hci_Io_Capability_request_negative_reply(struct bd_addr *bdaddr, uint8 Reason);
int hci_Read_Remote_Extended_Features(uint16 conhdl, uint8 Page_Number);
int hci_write_simple_pairing_mode(uint8 simple_pairing_enable);
int hci_user_confirmati_request_reply(struct bd_addr *bdaddr);
int hci_user_confirmati_request_negative_reply(struct bd_addr *bdaddr);
int hci_Authentication_Requested(uint16 conhdl);
struct hci_link *hci_get_link(struct bd_addr *bdaddr, uint8 link_type);
void hci_req_remote_name_complete(int (* req_remote_name_complete)(void *arg, struct hci_pcb *pcb,uint8 scan_ret, struct bd_addr *bdaddr, uint8 *name, struct hci_inq_res *ires_header));
int hci_read_remote_name(struct bd_addr *bdaddr, uint8 psrm , uint16 clk_offset);
int hci_inquiry_cancel(void);
void hci_clean_scan_result(void);
int hci_read_remote_features(uint16 conhdl);
int hci_can_send_packet_now(void);
void hci_register_event_handler(void (* hci_event_handler)(uint8 packet_type, uint16 channel, uint8 *packet, uint16 size));
#define l2cap_can_send_connectionless_packet_now(x)   hci_can_send_packet_now(x)

// reserves outgoing packet buffer. @returns 1 if successful
int hci_reserve_packet_buffer(void);
void hci_release_packet_buffer(void);
void hci_register_app_event_handler(void (* hci_app_event_handler)(uint8 event_type, uint8 *parameter, uint16 param_size,err_t err));
void hci_buf_subtract(void);
// reserves outgoing packet buffer. @returns 1 if successful

// ble func
void hci_le_advertisement_address(uint8 * addr_type, struct bd_addr *addr);
void hci_le_advertisement_address(uint8 * addr_type, struct bd_addr *addr);
int hci_write_le_host_supported_command(uint8 LE_Supported_Host,uint8 Simultaneous_LE_Host);
int hci_le_set_event_mask_command(long long mask_flag);
int hci_le_read_buffer_size_command(void);
int hci_le_read_local_supported_features_command(void);
int hci_le_set_random_address_command(struct bd_addr *bdaddr);
int hci_le_advertising_parameters_command(uint16 Advertising_Interval_Min, \
        uint16 Advertising_Interval_Max,\
        uint8 Advertising_Type,\
        uint8 Own_Address_Type,\
        uint8 Direct_Address_Type,\
        struct bd_addr *Direct_Address,\
        uint8 Advertising_Channel_Map,\
        uint8 Advertising_Filter_Policy\
                                         );
int hci_le_read_advertising_channel_tx_power_command(struct bd_addr *bdaddr);
int hci_le_set_advertising_data_command(uint8 Advertising_Data_Length, uint8* Advertising_Data);
int hci_le_set_response_data_command(uint8 Scan_Response_Data_Length, uint8* Scan_Response_Data);
int hci_le_set_advertise_enable_command(uint8 Advertising_Enable);
int hci_le_scan_parameters_command(uint8 LE_Scan_Type, uint16 LE_Scan_Interval, uint16 LE_Scan_Window, uint8 Own_Address_Type, uint8 Scanning_Filter_Policy );
int hci_le_set_scan_enable_command(uint8 LE_Scan_Enable, uint8 Filter_Duplicates);
int hci_le_create_connection_command(struct bd_addr *bdaddr, uint8 address_type);
int hci_le_create_connection_cancel_command(void);
int hci_le_read_white_list_size_command(void);
int hci_le_clear_white_list_command(void);

int hci_le_connection_update_command(uint16 Connection_Handle, \
                                     uint16 Conn_Interval_Min, \
                                     uint16 Conn_Interval_Max, \
                                     uint16 Conn_Latency, \
                                     uint16 Supervision_Timeout, \
                                     uint16 Minimum_CE_Length, \
                                     uint16 Maximum_CE_Length);
int hci_le_set_host_chanel_classification_command(long long Channel_Map);
int hci_le_read_channel_map_commad(long long Connection_Handle);
int hci_le_read_remote_used_features_command(uint16 conhdl);
int hci_le_encrypt_command(uint8 *Key, uint8 *Plaintext_Data);
int hci_le_rand_command (void);
int hci_le_start_encryption_command(uint16 conhdl, long long Random_Number, uint16  Encrypted_Diversifier, uint8 *Long_Term_Key);
int hci_le_term_key_request_reply_command(uint16 conhdl, uint8 * long_term_key);
int hci_le_long_term_key_request_negative_reply_command(uint16 conhdl);
int hci_le_read_supported_states_command(void);
int hci_le_receiver_test_command(uint8 RX_Frequenc);
int hci_le_transmitter_test_command(uint8 TX_Frequenc, uint8 Length_Of_Test_Data, uint8 Packet_Payload);
int hci_le_test_end_command(void);







/*-----------------------------------------------------------------------------------*/
/* hci_delete_link_key():
 *
 * delete_all_flag 0: Delete only the Link Key for specified BD_ADDR
                   1: Delete all stored Link Keys
 */
/*-----------------------------------------------------------------------------------*/

int hci_delete_link_key(struct bd_addr *bdaddr, uint8 delete_all_flag) ;
int hci_enable_device_under_test_mode_command(void);


void hci_auth_complete(void (* acl_link_auth_complete)(uint8 result));


void hci_sco_connect_notify(void (* sco_connected)(void));
void hci_sco_disconnect_notify ( void(* sco_disconnect)(void));
void hci_sco_recv( void(* sco_recv)(unsigned char *data, unsigned int len));
void hci_acl_disconnect_notify ( void(* acl_disconnect)(struct bd_addr *bdaddr));

void hci_acl_connected_notify ( void(* acl_connected)(uint8 result, struct bd_addr *bdaddr));

void hci_acl_connnect_req(void (* acl_connect_req)(struct bd_addr *bdaddr));

struct hci_inq_res  * hci_updata_devName(struct bd_addr *bdaddr, uint8 * name);

void hci_tmr(void);
void hci_read_rssi_hook(int (* read_rssi_hook)(struct bd_addr *bdaddr, int8 rssi));
int l2cap_send_connectionless(uint16 handle, uint16 cid, uint8 *data, uint16 len);


/* HCI packet indicators */
#define HCI_COMMAND_DATA_PACKET 0x01
#define HCI_ACL_DATA_PACKET     0x02
#define HCI_SCO_DATA_PACKET     0x03
#define HCI_EVENT_PACKET        0x04

#define HCI_EVENT_HDR_LEN 2
#define HCI_ACL_HDR_LEN 4
#define HCI_SCO_HDR_LEN 3
#define HCI_CMD_HDR_LEN 3

/* Opcode Group Field (OGF) values */
#define HCI_LINK_CONTROL 0x01   /* Link Control Commands */
#define HCI_LINK_POLICY 0x02    /* Link Policy Commands */
#define HCI_HOST_C_N_BB 0x03    /* Host Controller & Baseband Commands */
#define HCI_INFO_PARAM 0x04     /* Informational Parameters */
#define HCI_STATUS_PARAM 0x05   /* Status Parameters */
#define HCI_TESTING 0x06        /* Testing Commands */
#define HCI_BLE_OGF 0x08        /* ble ofg */
/* Opcode Command Field (OCF) values */

/* Link control commands */
#define HCI_INQUIRY 0x01
#define HCI_CREATE_CONNECTION 0x05
#define HCI_REJECT_CONNECTION_REQUEST 0x0A
#define HCI_DISCONNECT 0x06
#define HCI_PIN_CODE_REQ_REP 0x0D
#define HCI_PIN_CODE_REQ_NEG_REP 0x0E
#define HCI_SET_CONN_ENCRYPT 0x13
#define HCI_LINK_KEY_REQUEST_REPLY 0x0B
#define HCI_LINK_KEY_REQUEST_NEGATIVE 0x0C
#define HCI_IO_CAPABILITY_REQUEST_REPLY 0x2B
#define HCI_IO_CAPABILITY_REQUEST_NEGATIVE 0x34
#define HCI_USER_CONFIRMATI_REQUEST_REPLY  0x2C
#define HCI_USER_CONFIRMATI_REQUEST_NEGATIVE_REPLY 0x2D
#define HCI_ACCEPT_SYNCHRONOUS_CONNECTION_REQUEST 0x29
#define HCI_REJECT_SYNCHRONOUS_CONNECTION_REQUEST 0x2A


/* Link Policy commands */
#define HCI_HOLD_MODE 0x01
#define HCI_SNIFF_MODE 0x03
#define HCI_EXIT_SNIFF_MODE 0x04
#define HCI_PARK_MODE 0x05
#define HCI_EXIT_PARK_MODE 0x06
#define HCI_W_LINK_POLICY 0x0D

/* Host-Controller and Baseband Commands */
#define HCI_SET_EVENT_MASK 0x01
#define HCI_RESET 0x03
#define HCI_SET_EVENT_FILTER 0x05
#define HCI_WRITE_STORED_LINK_KEY 0x11
#define HCI_ROLE_CHANGE 0x12
#define HCI_CHANGE_LOCAL_NAME 0x13

#define HCI_WRITE_PAGE_TIMEOUT 0x18
#define HCI_WRITE_SCAN_ENABLE 0x1A
#define HCI_WRITE_COD 0x24
#define HCI_W_FLUSHTO 0x28
#define HCI_SET_HC_TO_H_FC 0x31

#define HCI_WRITE_SIMPLE_PAIRING_MODE 0x56


/* Status Parameters Commands */
#define HCI_GET_RSSI  0x05
#define HCI_READ_LINK_QUALITYS 0x03
/* ble Commands */
#define HCI_LE_READ_BUFFER_SIZE  0x02
#define HCI_LE_ADVERTISING_PARAMETER 0x06
#define HCI_LE_SET_ADVERTISING_DATA  0x08
#define HCI_LE_SET_SCAN_RESPONSE_DATA  0x09
#define HCI_LE_SET_ADVERTISE_ENABLE    0x0A
#define HCI_LE_SET_SCAN_ENABLE   0x0C
#define HCI_LE_CLEAR_WHITE_LIST  0x10

/* VS Commands */



// rk add
#define HCI_W_INQUIRY_SCAN_ACTIVITY 0x1E
#define HCI_W_PAGE_SCAN_ACTIVITY 0x1C
#define HCI_W_CONNECTION_ACCEPT_TIMEOUT 0x16
#define HCI_ACCEPT_CONNECTION_REQUEST 0x09
#define HCI_WRITE_LE_HOST_SUPPOURTE   0x6d


/* Informational Parameters */
#define HCI_READ_BUFFER_SIZE 0x05
#define HCI_READ_BD_ADDR 0x09

/* Status Parameters */
#define HCI_READ_FAILED_CONTACT_COUNTER 0x01
#define HCI_RESET_FAILED_CONTACT_COUNTER 0x02
#define HCI_GET_LINK_QUALITY 0x03
#define HCI_READ_RSSI 0x05

/* Testing commands */

/* Possible event codes */
#define HCI_INQUIRY_COMPLETE 0x01
#define HCI_INQUIRY_RESULT 0x02
#define HCI_CONNECTION_COMPLETE 0x03
#define HCI_CONNECTION_REQUEST 0x04
#define HCI_DISCONNECTION_COMPLETE 0x05
#define HCI_AUTHENTICATION_COMPLETE 0x06
#define HCI_REMOTE_NAME_REQUEST_COMPLETE_EVENT 0x07
#define HCI_ENCRYPTION_CHANGE 0x08
#define HCI_QOS_SETUP_COMPLETE 0x0D
#define HCI_COMMAND_COMPLETE 0x0E
#define HCI_COMMAND_STATUS 0x0F
#define HCI_HARDWARE_ERROR 0x10
#define HCI_ROLE_CHANGE 0x12
#define HCI_NBR_OF_COMPLETED_PACKETS 0x13
#define HCI_MODE_CHANGE 0x14
#define HCI_PIN_CODE_REQUEST 0x16
#define HCI_LINK_KEY_NOTIFICATION 0x18
#define HCI_DATA_BUFFER_OVERFLOW 0x1A
#define HCI_MAX_SLOTS_CHANGE 0x1B
#define HCI_LINK_KEY_RESQUEST 0x17
#define HCI_IO_CAPABILITY_REQUEST 0x31
#define HCI_IO_CAPABILITY_RESPONSE 0x32
#define HCI_USER_CONFIRMATION_REQUEST 0x33
#define HCI_SIMPLE_PAIRING_COMPLETE 0x36
#define HCI_SYNCHRONOUS_CONNECTION_COMPLETE 0x2C
#define HCI_READ_REMOTE_EXTENDED_FEATURES_COMPLETE 0x23
#define HCI_READ_REMOTE_FEATURES_COMPLETE 0x0B
#define HCI_EXTENDED_INQUIRY_RESULT_EVENT 0x2F


#define HCI_FLUSH_OCCURRED 0x11
#define HCI_BLE_META_EVENT 0x3E
#define HCI_BLE_CONNECTION_COMPLETE_EVENT 0x01
#define HCI_BLE_ADVERTISING_REPORT_EVENT  0x02
#define HCI_BLE_CONNETION_UPDATE_COMPLETE_EVENT 0x03
#define HCI_BLE_REMOTE_USED_FEATURES_COMPLETE_EVENT 0x04
#define HCI_LONG_TERM_KEY_REQUEST_EVENT             0x05

/* Success code */
#define HCI_SUCCESS 0x00
/* Possible error codes */
#define HCI_UNKNOWN_HCI_COMMAND 0x01
#define HCI_NO_CONNECTION 0x02
#define HCI_HW_FAILURE 0x03
#define HCI_PAGE_TIMEOUT 0x04
#define HCI_AUTHENTICATION_FAILURE 0x05
#define HCI_KEY_MISSING 0x06
#define HCI_MEMORY_FULL 0x07
#define HCI_CONN_TIMEOUT 0x08
#define HCI_MAX_NUMBER_OF_CONNECTIONS 0x09
#define HCI_MAX_NUMBER_OF_SCO_CONNECTIONS_TO_DEVICE 0x0A
#define HCI_ACL_CONNECTION_EXISTS 0x0B
#define HCI_COMMAND_DISSALLOWED 0x0C
#define HCI_HOST_REJECTED_DUE_TO_LIMITED_RESOURCES 0x0D
#define HCI_HOST_REJECTED_DUE_TO_SECURITY_REASONS 0x0E
#define HCI_HOST_REJECTED_DUE_TO_REMOTE_DEVICE_ONLY_PERSONAL_SERVICE 0x0F
#define HCI_HOST_TIMEOUT 0x10
#define HCI_UNSUPPORTED_FEATURE_OR_PARAMETER_VALUE 0x11
#define HCI_INVALID_HCI_COMMAND_PARAMETERS 0x12
#define HCI_OTHER_END_TERMINATED_CONN_USER_ENDED 0x13
#define HCI_OTHER_END_TERMINATED_CONN_LOW_RESOURCES 0x14
#define HCI_OTHER_END_TERMINATED_CONN_ABOUT_TO_POWER_OFF 0x15
#define HCI_CONN_TERMINATED_BY_LOCAL_HOST 0x16
#define HCI_REPETED_ATTEMPTS 0x17
#define HCI_PAIRING_NOT_ALLOWED 0x18
#define HCI_UNKNOWN_LMP_PDU 0x19
#define HCI_UNSUPPORTED_REMOTE_FEATURE 0x1A
#define HCI_SCO_OFFSET_REJECTED 0x1B
#define HCI_SCO_INTERVAL_REJECTED 0x1C
#define HCI_SCO_AIR_MODE_REJECTED 0x1D
#define HCI_INVALID_LMP_PARAMETERS 0x1E
#define HCI_UNSPECIFIED_ERROR 0x1F
#define HCI_UNSUPPORTED_LMP_PARAMETER_VALUE 0x20
#define HCI_ROLE_CHANGE_NOT_ALLOWED 0x21
#define HCI_LMP_RESPONSE_TIMEOUT 0x22
#define HCI_LMP_ERROR_TRANSACTION_COLLISION 0x23
#define HCI_LMP_PDU_NOT_ALLOWED 0x24
#define HCI_ENCRYPTION_MODE_NOT_ACCEPTABLE 0x25
#define HCI_UNIT_KEY_USED 0x26
#define HCI_QOS_NOT_SUPPORTED 0x27
#define HCI_INSTANT_PASSED 0x28
#define HCI_PAIRING_UNIT_KEY_NOT_SUPPORTED 0x29

/* Specification specific parameters */
#define HCI_BD_ADDR_LEN 6
#define HCI_LMP_FEATURES_LEN 8
#define HCI_LINK_KEY_LEN 16
#define HCI_LMP_FEAT_LEN 8

/* Command OGF */
#define HCI_LINK_CTRL_OGF 0x01 /* Link ctrl cmds */
#define HCI_LINK_POLICY_OGF 0x02 /* link policy */

#define HCI_HC_BB_OGF 0x03 /* Host controller and baseband commands */
#define HCI_INFO_PARAM_OGF 0x04 /* Informal parameters */
#define HCI_VS_OGF  0x3F
#define HCI_STATUS_PARAM_OGF 0x05   /* Status Parameters */

/* Command OCF */
#define HCI_INQUIRY_OCF 0x01
#define HCI_CREATE_CONN_OCF 0x05
#define HCI_DISCONN_OCF 0x06
#define HCI_REJECT_CONN_REQ_OCF 0x0A
#define HCI_SET_EV_MASK_OCF 0x01
#define HCI_RESET_OCF 0x03
#define HCI_SET_EV_FILTER_OCF 0x05
#define HCI_W_PAGE_TIMEOUT_OCF 0x18
#define HCI_W_SCAN_EN_OCF 0x1A
#define HCI_R_COD_OCF 0x23
#define HCI_W_COD_OCF 0x24
#define HCI_SET_HC_TO_H_FC_OCF 0x31
#define HCI_H_BUF_SIZE_OCF 0x33
#define HCI_H_NUM_COMPL_OCF 0x35
#define HCI_R_BUF_SIZE_OCF 0x05
#define HCI_R_BD_ADDR_OCF 0x09
//////// rk add
#define HCI_W_INQUIRY_SCAN_ACTIVITY_OCF 0x1E //wp
#define HCI_W_PAGE_SCAN_ACTIVITY_OCF 0x1C //wp
#define HCI_W_CONNECTION_ACCEPT_TIMEOUT_OCF 0x16
#define HCI_ACCEPT_CONNECTION_REQUEST_OCF 0x09
#define HCI_W_POLICY_SETTING_OCF 0x0F
#define HCI_W_LINK_SUPERVISION_TIMEOUT_OCF 0x37 //yh


/* Command packet length (including ACL header)*/
#define HCI_INQUIRY_PLEN 9
#define HCI_CREATE_CONN_PLEN 17
#define HCI_DISCONN_PLEN 7
#define HCI_REJECT_CONN_REQ_PLEN 11
#define HCI_PIN_CODE_REQ_REP_PLEN 27
#define HCI_PIN_CODE_REQ_NEG_REP_PLEN 10
#define HCI_SET_CONN_ENCRYPT_PLEN 7
#define HCI_WRITE_STORED_LINK_KEY_PLEN 27
#define HCI_CHANGE_LOCAL_NAME_PLEN 4
#define HCI_SET_EV_MASK_PLEN 12
#define HCI_SNIFF_PLEN 14
#define HCI_W_LINK_POLICY_PLEN 8
#define HCI_RESET_PLEN 4
#define HCI_SET_EV_FILTER_PLEN 6
#define HCI_W_PAGE_TIMEOUT_PLEN 6
#define HCI_W_SCAN_EN_PLEN 5
#define HCI_R_COD_PLEN 4
#define HCI_W_COD_PLEN 7
#define HCI_W_FLUSHTO_PLEN 7
#define HCI_SET_HC_TO_H_FC_PLEN 5
#define HCI_H_BUF_SIZE_PLEN 7
#define HCI_H_NUM_COMPL_PLEN 7
#define HCI_R_BUF_SIZE_PLEN 4
#define HCI_R_BD_ADDR_PLEN 4
//////// rk add
#define HCI_W_INQUIRY_SCAN_ACTIVITY_PLEN 8
#define HCI_W_PAGE_SCAN_ACTIVITY_PLEN 8
#define HCI_W_CONNECTION_ACCEPT_TIMEOUT_PLEN 6
#define HCI_ACCEPT_CONNECTION_REQUEST_PLEN 11
#define HCI_W_LINK_SUPERVISION_TIMEOUT_PLEN 6

#define POLICY_SETTING_ALL_DEFAULT (0)
#define POLICY_SETTING_ENABLE_ROLE_SWITCH (1<< 0)
#define POLICY_SETTING_ENABLE_HOLD_MODE   (1<< 1)
#define POLICY_SETTING_ENABLE_SNIFF_MODE  (1<< 2)
#define POLICY_SETTING_ENABLE_PART_STATE  (1<< 3)

#define POLICY_SETTING_ENABLE_ALL (POLICY_SETTING_ENABLE_ROLE_SWITCH | POLICY_SETTING_ENABLE_HOLD_MODE| POLICY_SETTING_ENABLE_SNIFF_MODE| POLICY_SETTING_ENABLE_PART_STATE)


__packed
struct hci_event_hdr
{
    uint8 code; /* Event code */
    uint8 len;  /* Parameter total length */
};

__packed
struct hci_acl_hdr
{
    uint16 conhdl_pb_bc; /* Connection handle, packet boundary and broadcast flag
             flag */
    uint16 len; /* length of data */
};

__packed
struct hci_sco_hdr
{
    uint16 conhdl_ps; /* Connection handle, packet status and
             flag */
    uint8 len; /* length of data */
};


__packed
struct hci_inq_res
{
    struct hci_inq_res *next; /* For the linked list */

    struct bd_addr bdaddr; /* Bluetooth address of a device found in an inquiry */
    uint8 cod[3]; /* Class of the remote device */
    uint8 psrm; /* Page scan repetition mode */
    uint8 psm; /* Page scan mode */
    uint8 rssi;/*rssi*/
    uint16 co; /* Clock offset */
    uint8 name[32]; /*name*/
};

__packed
struct hci_link
{
    struct hci_link *next; /* For the linked list */

    struct bd_addr bdaddr; /* The remote peers Bluetooth address for this connection */
    uint16 conhdl; /* Connection handle */
    uint8  role;   /* For ble */
    uint16 Conn_Interval;/* For ble */
    uint16 Conn_Latency; /* For ble */
    uint16 Supervision_Timeout; /* For ble */
    uint8  Master_Clock_Accuracy;/* For ble */

    uint8  link_type;
    uint8  encrypt_mode;
    uint8  link_flag;
    uint8  feature[8];
    uint8  name[32];
#if HCI_FLOW_QUEUEING
    struct pbuf *p;
    uint16 len;
    uint8 pb;
    void (*func)(void);
#endif
};



typedef enum
{
    HCI_FLAG_IR,
    HCI_AUTH_COMPLETE,
    HCI_BR_EDR_LINK,
    HCI_BLE_LINK,
    HCI_REMOTE_SUPPORT_SPP,
    HCI_AUTO_READ_REMOTE_NAME,

} HCI_FLAG;


#define CHECK_HCI_FLAG(x, State)  ((x >> State) & 0x01)
#define SET_HCI_FLAG(x, State)    ( x |=(1<<State) )
#define CLEAR_HCI_FLAG(x, State)  ( x &= ~(1<<State))

__packed
struct hci_pcb
{
    void *callback_arg;
    uint8 hci_scolink_number;
    uint8 hci_flag;//  mask HCI_IR
    /* Host to host controller flow control */
    uint8 numcmd; /* Number of command packets that the host controller (Bluetooth module)
          can buffer */
    uint16 maxsize; /* Maximum length of the data portion of each HCI ACL data packet that the
           Host Controller is able to accept */
    uint16 hc_num_acl; /* Number of ACL packets that the Bluetooth module can buffer */
    uint16 hc_le_max_data_size; /* Number of le packets that the Bluetooth module can buffer */
    uint16 hc_le_num_acl; /* Number of le packets that the Bluetooth module can buffer */
    uint16 hc_le_num_total; /* rk add */
    uint16 hc_num_acl_total; /* rk add */
    uint16 flush_handle; /* rk add */
    uint8  flush_timer_enable; /* rk add */
    uint   flush_timer_timeout; /* rk add */
    struct bd_addr bdaddr;      /* rk add */
    uint8   adv_addr_type;    /* rk add */
    struct bd_addr adv_address; /* rk add */
    uint8  hci_packet_buffer_reserved; /* rk add */
    /* Host controller to host flow control */
    uint8 flow; /* Indicates if host controller to host flow control is on */
    uint16 host_num_acl; /* Number of ACL packets that we (the host) can buffer */

    struct hci_inq_res *ires; /* Results of an inquiry */
    struct hci_inq_res *iresAll; /* Results of ALL inquiry */
    struct hci_inq_res *iresWithName; /* Results of ALL inquiry */

    err_t (* pin_req)(void *arg, struct bd_addr *bdaddr);
    err_t (* inq_complete)(void *arg, struct hci_pcb *pcb, struct hci_inq_res *ires,
                           uint16 result,uint16 complete);
    err_t (* rbd_complete)(void *arg, struct bd_addr *bdaddr);
    err_t (* link_key_not)(void *arg, struct bd_addr *bdaddr, uint8 *key);
    err_t (* wlp_complete)(void *arg, struct bd_addr *bdaddr);
    err_t (* conn_complete)(void *arg, struct bd_addr *bdaddr);
    err_t (* cmd_complete)(void *arg, struct hci_pcb *pcb, uint8 ogf, uint8 ocf, uint8 result);
    err_t (* link_key_request)(void *arg, struct bd_addr *bdaddr);

    err_t (*req_remote_name_complete)(void *arg, struct hci_pcb *pcb,uint8 scan_status, struct bd_addr *bdaddr, uint8 *name, struct hci_inq_res *ires_header);
    void (* sco_connected)(void);
    void (* sco_disconnect)(void);
    void (* acl_connected)(uint8 result, struct bd_addr *bdaddr);
    void (* acl_disconnect)(struct bd_addr *bdaddr);
    void (* sco_recv)(unsigned char *data, unsigned int len);
    void (* acl_link_auth_complete)(uint8 result);
    void (* acl_connect_req)(struct bd_addr *bdaddr);
    void (*read_rssi_hook)(struct bd_addr *bdaddr, int8 rssi);
    void (* hci_event_handler)(uint8 packet_type, uint16 channel, uint8 *packet, uint16 size);
    void (* hci_app_event_handler)(uint8 event_type, uint8 *parameter, uint16 param_size, err_t err);
};

#define HCI_EVENT_PIN_REQ(pcb,bdaddr,ret) \
                         if((pcb)->pin_req != NULL) { \
                           (ret = (pcb)->pin_req((pcb)->callback_arg,(bdaddr))); \
                         } else { \
                           ret = hci_pin_code_request_neg_reply(bdaddr); \
             }
#define HCI_EVENT_LINK_KEY_NOT(pcb,bdaddr,key,ret) \
                              if((pcb)->link_key_not != NULL) { \
                                (ret = (pcb)->link_key_not((pcb)->callback_arg,(bdaddr),(key))); \
                              }
#define HCI_EVENT_INQ_COMPLETE(pcb,result,complete, ret) \
                              if((pcb)->inq_complete != NULL) \
                              (ret = (pcb)->inq_complete((pcb)->callback_arg,(pcb),(pcb)->ires,(result), (complete)))
#define HCI_EVENT_RBD_COMPLETE(pcb,bdaddr,ret) \
                              if((pcb)->rbd_complete != NULL) \
                              (ret = (pcb)->rbd_complete((pcb)->callback_arg,(bdaddr)));
#define HCI_EVENT_WLP_COMPLETE(pcb,bdaddr,ret) \
                               if((pcb)->wlp_complete != NULL) \
                               (ret = (pcb)->wlp_complete((pcb)->callback_arg,(bdaddr)));
#define HCI_EVENT_CONN_COMPLETE(pcb,bdaddr,ret) \
                               if((pcb)->conn_complete != NULL) \
                               (ret = (pcb)->conn_complete((pcb)->callback_arg,(bdaddr)));
#define HCI_EVENT_CMD_COMPLETE(pcb,ogf,ocf,result,ret) \
                              if((pcb)->cmd_complete != NULL) \
                              (ret = (pcb)->cmd_complete((pcb)->callback_arg,(pcb),(ogf),(ocf),(result)))


#define HCI_EVENT_LINK_KEY_REQ(pcb,bdaddr,ret) \
                           if((pcb)->link_key_request != NULL) { \
                             (ret = (pcb)->link_key_request((pcb)->callback_arg,(bdaddr))); \
                           } else { \
                             ret = hci_link_key_request_negative_reply(bdaddr); \
               }


#define HCI_EVENT_SCO_CONNECT(pcb) \
                              if((pcb)->sco_connected != NULL) \
                              ((pcb)->sco_connected())


#define HCI_EVENT_SCO_DISCONNECT(pcb) \
                              if((pcb)->sco_disconnect != NULL) \
                              ((pcb)->sco_disconnect())




#define HCI_EVENT_ACL_CONNECTED(pcb, result, bdaddr) \
                              if((pcb)->acl_connected != NULL) \
                              ((pcb)->acl_connected(result,bdaddr))


#define HCI_EVENT_ACL_DISCONNECT(pcb, bdaddr) \
                              if((pcb)->acl_disconnect != NULL) \
                              ((pcb)->acl_disconnect(bdaddr))

#define HCI_EVENT_READ_REMOTE_NAME_COMPLETE(pcb,scan_status,bdaddr,name,ret) \
                              if((pcb)->req_remote_name_complete != NULL) { \
                                (ret = (pcb)->req_remote_name_complete((pcb)->callback_arg,(pcb),(scan_status),(bdaddr),(name), (pcb)->iresAll)); \
                              }

#define HCI_EVENT_ACL_AUTH_COMPLETE(pcb, result) \
                                if((pcb)->acl_link_auth_complete != NULL) \
                                                            ((pcb)->acl_link_auth_complete(result))
/* The HCI LINK lists. */
extern struct hci_link *hci_active_links; /* List of all active HCI LINKs */
extern struct hci_link *hci_tmp_link; /* Only used for temporary storage. */

extern struct hci_pcb *pcb;


#define HCI_REG(links, nlink) do { \
                            nlink->next = *links; \
                            *links = nlink; \
                            } while(0)
#define HCI_RMV(links, nlink) do { \
                            if(*links == nlink) { \
                               *links = (*links)->next; \
                            } else for(hci_tmp_link = *links; hci_tmp_link != NULL; hci_tmp_link = hci_tmp_link->next) { \
                               if(hci_tmp_link->next != NULL && hci_tmp_link->next == nlink) { \
                                  hci_tmp_link->next = nlink->next; \
                                  break; \
                               } \
                            } \
                            nlink->next = NULL; \
                            } while(0)


#define HCI_RMV_COMMON(links, nlink, linktmp) do { \
                            if(*links == nlink) { \
                               *links = (*links)->next; \
                            } else for(linktmp = *links; linktmp != NULL; linktmp = linktmp->next) { \
                               if(linktmp->next != NULL && linktmp->next == nlink) { \
                                  linktmp->next = nlink->next; \
                                  break; \
                               } \
                            } \
                            nlink->next = NULL; \
                            } while(0)





#define HCI_SEND_EVENT(pcb,event_type,parameter, param_size, err)         \
if(pcb != NULL)                                                      \
{                                                                    \
    if((pcb)->hci_app_event_handler != NULL)                             \
    {                                                                \
        (pcb)->hci_app_event_handler(event_type, parameter, param_size, err); \
    }                                                                \
}

typedef enum
{

    HCI_APP_EVENT_ACL_CONNECTED = 0x01,
    HCI_APP_EVENT_ACL_DISCONNECTED,
    HCI_APP_EVENT_SCO_CONNECTED,
    HCI_APP_EVENT_SCO_DISCONNECTED,
    HCI_APP_EVENT_BLE_CONNECTED,
    HCI_APP_EVENT_BLE_DISCONNECTED,

} HCI_EVENT_CODE;

#endif /* __HCI_H__ */
