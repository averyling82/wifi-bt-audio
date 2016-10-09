#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_
//#include <player/play-state.h>

typedef enum __pstate
{
    PLAYER_STOPPED,
    PLAYER_PLAYING,
    PLAYER_PAUSED,
    PLAYER_TRANSITIONING,
    PLAYER_NO_MEDIA,
} pstate_t;

typedef void (* state_listener_t)(pstate_t state);


typedef struct ctrl_s
{
    char name[128];
    int (*init)(void);
    int (*deinit)(void);
    int (*play)(char* url);
    int (*stop)(void);
    int (*resume)(void);
    int (*pause)(void);
    int (*seek)(char *url,int ms);
    int (*current_time)(char* buf, int len);
    int (*get_duration)(char* buf, int len);
    int (*get_state)(void);
    int (*set_state)(int state, char *url);
    int (*state_listener)(state_listener_t listener);
    int (*set_volume)(int volume);
    int (*get_volume)(void);
} ctrl_t;

#endif
