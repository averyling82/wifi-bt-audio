#ifndef _AIRPLAY_PLAYER_H
#define _AIRPLAY_PLAYER_H

#include "type.h"

typedef struct {
    uint8_t aesiv[16], aeskey[16];
    int32_t fmtp[12];
} stream_cfg;

typedef uint16_t seq_t;
typedef uint32_t len_t;

// wrapped number between two seq_t.
static inline uint16_t seq_diff(seq_t a, seq_t b) {
    int16_t diff = b - a;
    return diff;
}

int airplay_player_play(stream_cfg *cfg);
void airplay_player_stop(void);
void player_volume(int v);
void player_put_packet(seq_t seqno, uint8_t *data, int len);

#endif //_PLAYER_H
