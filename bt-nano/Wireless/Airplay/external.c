#include "BspConfig.h"
#ifdef __WIFI_AIRPLAY_C__

#include <string.h>
#include <stdlib.h>
#include "external.h"
#include "commons.h"
#include "airplay_heap.h"

#ifdef strtok_r
#undef strtok_r
#endif

/* Parse S into tokens separated by characters in DELIM.
   If S is NULL, the saved pointer in SAVE_PTR is used as
   the next starting point.  For example:
        char s[] = "-abc-=-def";
        char *sp;
        x = strtok_r(s, "-", &sp);      // x = "abc", sp = "=-def"
        x = strtok_r(NULL, "-=", &sp);  // x = "def", sp = NULL
        x = strtok_r(NULL, "=", &sp);   // x = NULL
                // s = "abc/0-def/0"
*/
char *strtok_r(char *s, const char *delim, char **save_ptr) {
    char *token;

    if (s == NULL) s = *save_ptr;

    /* Scan leading delimiters.  */
    s += strspn(s, delim);
    if (*s == '\0')
        return NULL;

    /* Find the end of the token.  */
    token = s;
    s = strpbrk(token, delim);
    if (s == NULL)
        /* This token finishes the string.  */
        *save_ptr = strchr(token, '\0');
    else {
        /* Terminate the token and make *SAVE_PTR point past it.  */
        *s = '\0';
        *save_ptr = s + 1;
    }

    return token;
}

char* strdup (const char *string)
{
    char *new_string = (char *)airplay_malloc(strlen(string)+1);
    if(new_string == NULL)
        return NULL;

    strcpy(new_string,string);
    return new_string;
}

/* const char *
 * inet_ntop4(src, dst, size)
 *      format an IPv4 address
 * return:
 *      'dst' (as a const)
 * notes:
 *      (1) uses no statics
 *      (2) takes a u_char* not an in_addr as input
 * author:
 *      Paul Vixie, 1996.
 */
static const char *
inet_ntop4 (const unsigned char *src, char *dst, int size)
{
  char tmp[sizeof "255.255.255.255"];
  int len;

  len = sprintf (tmp, "%u.%u.%u.%u", src[0], src[1], src[2], src[3]);
  if (len < 0)
    return NULL;

  if (len > size)
    {
      AIRPLAY_DEBUGF("inet_ntop4 error\n");
      return NULL;
    }

  return strcpy (dst, tmp);
}

static const char *
inet_ntop6 (const unsigned char *src, char *dst, int size){
    return NULL;
}

/* char *
 * isc_net_ntop(af, src, dst, size)
 *      convert a network format address to presentation format.
 * return:
 *      pointer to presentation format address (`dst'), or NULL (see errno).
 * author:
 *      Paul Vixie, 1996.
 */
const char *
inet_ntop(int af, const void *src, char *dst, int size) {
    switch ( af ) {
        case AF_INET:
            return(inet_ntop4(src, dst, size));
#ifdef HAVE_IPV6
        case AF_INET6:
            return(inet_ntop6(src, dst, size));
#endif
        default:
            return 0;
    }
    /* NOTREACHED */
}

/**
 * strsep - Split a string into tokens
 * @s: The string to be searched
 * @ct: The characters to search for
 *
 * strsep() updates @s to point after the token, ready for the next call.
 *
 * It returns empty tokens, too, behaving exactly like the libc function
 * of that name. In fact, it was stolen from glibc2 and de-fancy-fied.
 * Same semantics, slimmer shape. ;)
 */
char *strsep_t(char **s, const char *ct)
{
    char *sbegin = *s;
    char *end;

    if (sbegin == NULL)
        return NULL;

    end = strpbrk(sbegin, ct);
    if (end){
        *end++ = '\0';
        *s = end;
    }
    else
        *s = NULL;
    return sbegin;
}

int gethostname(char *hostname, int len){
    if (len >= strlen("rk-nanod-iot"))
        strcpy(hostname, "rk-nanod-iot");

    return 1;
}

int nanosleep( const struct timeval * rqtp, struct timeval * rmtp )
{
    if (rqtp->tv_usec > 999999) {
        /* The time interval specified 1,000,000 or more microseconds. */
        AIRPLAY_DEBUGF("The time interval specified 1,000,000 or more microseconds\n");
        return -1;
    }
    return rkos_sleep(rqtp->tv_sec * 1000 + rqtp->tv_usec / 1000);
}

long currentTime()
{
    portTickType tickCount;

    /* tick count == ms, if configTICK_RATE_HZ is set to 1000 */
    tickCount = xTaskGetTickCount();

    //return (double)tickCount / 1000;    //return s
    return tickCount;    //return ms
}

#endif