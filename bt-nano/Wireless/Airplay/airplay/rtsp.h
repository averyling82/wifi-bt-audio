#ifndef _RTSP_H
#define _RTSP_H

void rtsp_listen_loop(void);
void rtsp_shutdown_stream(void);
void close_rtsp_listen_loop(void);
void cleanup_threads(void);

#endif // _RTSP_H
