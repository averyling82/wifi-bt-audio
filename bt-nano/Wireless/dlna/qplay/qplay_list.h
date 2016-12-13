#ifndef _QPLAY_LIST_H
#define _QPLAY_LIST_H
#include <cybergarage/upnp/caction.h>
#include <cybergarage/http/chttp.h>

#define CG_HTTP_LEN_MAX   5000//5K
#define HTTP_CONTENT   L"httpcontent.txt"
#define TRACK_DATA     L"TracksMetaData.txt"



typedef enum {
	QPLAY_LIST_VALUE_UNKNOW = 0,
	QPLAY_LIST_VALUE_TRACKSMETADATA,
	QPLAY_LIST_VALUE_QUEUEID,
	QPLAY_LIST_VALUE_TRACKSNUM,
	QPLAY_LIST_VALUE_TRACKSCUR,
	QPLAY_LIST_VALUE_TRACKSDURATION,
	QPLAY_LIST_VALUE_PTRACKSROOT,
	QPLAY_LIST_VALUE_PTRACKSARRAY,
	QPLAY_LIST_VALUE_COUNT
} G_QPLAY_LIST_VALUE ;

typedef enum {
	QPLAY_TRANSPORT_STATE_DISABLE = 0,
	QPLAY_TRANSPORT_STATE_NOTSEEKING,
	QPLAY_TRANSPORT_STATE_SEEKING,
	QPLAY_TRANSPORT_STATE__COUNT
} G_QPLAY_TRANSPORT_STATE;


G_QPLAY_TRANSPORT_STATE get_qplay_seek_state(void);
void set_qplay_seek_state(G_QPLAY_TRANSPORT_STATE state);

BOOL qplay_actionreceived(CgUpnpAction * action);
void player_init_qplay_list(void);
BOOL player_set_qplay_queueID(char *newid);
void player_reset_qplay_list(BOOL resetPlayer);
void *player_get_qplaylist_value(G_QPLAY_LIST_VALUE type);
char *player_get_curQPLAYtrack_metadata(void);
BOOL player_seek_qplay_index(int index);
BOOL player_seek_next_qplay_index(void);



int qplay_httptrackmedata_packet_write_file(CgHttpPacket *httpPkt, CgSocket *sock,unsigned long conLen);
void qplay_httptrackmedata_response_from_file(CgHttpPacket *httpPkt, CgSocket *sock);

#endif
