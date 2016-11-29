#ifndef _PLAYER_H_
#define _PLAYER_H_

//#include <player/play-state.h>
#include <controller.h>

typedef struct player_s
{
    ctrl_t * controller;
    char * url;
} player_t;

extern int   player_create(player_t **player);
extern int   player_destroy(player_t *player);
extern int   player_set_url(player_t *player, char *url);
extern char *player_get_url(player_t *player);
extern void  player_clear_url(player_t *player);
extern int   player_play(player_t *player);
extern int   player_stop(player_t *player);
extern int   player_pause(player_t *player);
extern int   player_resume(player_t *player);
extern int   player_seek(player_t *player, int ms);
extern int   player_current_time(player_t *player, char *buf, int len);
extern int   player_get_duration(player_t *player, char *buf, int len);
extern int   player_get_volume(player_t *player);
extern int   player_set_volume(player_t *player, int volume);
extern int   player_state_listener(player_t *player, state_listener_t l);
extern pstate_t player_get_state(player_t *player);

#endif
