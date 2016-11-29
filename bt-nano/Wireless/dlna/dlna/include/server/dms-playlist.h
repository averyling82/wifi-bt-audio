#ifndef _CLIENT_CONTROLLER_H_
#define  _CLIENT_CONTROLLER_H_

//#include  <playlist/playlist_load.h>
//#include <player/player.h>
#include <utils/typedef.h>

typedef struct _MUSICLIST{
    struct _MUSICLIST *prev;
    struct _MUSICLIST *next;
    int id;
    int index;
    char *name;
}MUSICLIST;

typedef struct _PLAYLIST{
    //BOOL headFlag;
    struct _PLAYLIST *prev;
    struct _PLAYLIST *next;
    int id;
    int index;
    char *name;
    int music_count;
    int music_current_disctance;//the current_music  disctance musiclist is common is 1;
    MUSICLIST *musiclist;

}PLAYLIST;


BOOL playlist_create_condition();
BOOL playlist_create(PLAYLIST **playlist, const char *type);
BOOL musiclist_create(PLAYLIST *playlist, const char *type);
int  musiclist_load_msg(char ***name, int **id, MUSICLIST *musiclist);
char *playlist_get_enconding(int id);
int  playlist_load_name(char ***name, int **id, PLAYLIST *playlist);
PLAYLIST * playlist_load_playlist_forname(const char *name, PLAYLIST *playlist);
const char *playlist_load_string(int id, const char *type);

#endif

