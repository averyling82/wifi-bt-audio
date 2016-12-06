/*
 * Slave-clocked ALAC stream player. This file is part of Shairport.
 * Copyright (c) James Laird 2011, 2013
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "BspConfig.h"
#ifdef __WIFI_AIRPLAY_C__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define NOT_INCLUDE_OTHER
#include "typedef.h"
#include "RKOS.h"
#include "commons.h"
#include "airplay_player.h"
#include "rtp.h"
#include "airplay_aes.h"
#include "airplay_heap.h"
#include "airplay_mplayer_ctrl.h"

// parameters from the source
static unsigned char *aesiv;
static AES_KEY aes;
static int sampling_rate, frame_size;
int write_fifo = 0;

#define FRAME_BYTES(frame_size) (4*frame_size)
// maximal resampling shift - conservative
#define OUTFRAME_BYTES(frame_size) (4*(frame_size+3))

//static alac_file *decoder_info;

// default buffer size
// needs to be a power of 2 because of the way BUFIDX(seqno) works
#define BUFFER_FRAMES  256//512
#define MAX_FRAMES     40
#define MAX_PACKET	4096

static len_t audio_break;
#define MAX_RESEND     (10) //125
#define MOD_RESEND     (5)

typedef struct audio_buffer_entry {   // decoded audio packets
    int ready;
    signed short *data;
    len_t   len;
    seq_t   seqno;
    seq_t   rsd_end;
    len_t   rsd_count;  //重传次数
    len_t   memlen;

} abuf_t;
static abuf_t audio_buffer[BUFFER_FRAMES];
#define BUFIDX(seqno) ((seq_t)(seqno) % BUFFER_FRAMES)

// mutex-protected variables
static seq_t ab_read, ab_write;
pSemaphore osPopBufferSem;
HTC hPopBuffer;
int reamin;
unsigned int losepackecnt;
unsigned int delaypackecnt;
unsigned int queuepackecnt;
static BOOL g_packet_too_many = FALSE;

#define BUFFER_SIZE (512)

#ifndef M_PI
#define M_PI 3.14159265358979323846 /* pi */
#endif

#if 0
#define fabs(x) ((x) < 0 ? -(x) : (x))
static double pow(double X,int Y){
  double Val = 1;
  while (Y--)
    Val *= X;
  return Val;
}
#endif

static inline int seq_next(seq_t a, seq_t b)
{
    if (a + 1 == b)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

// the sequence numbers will wrap pretty often.
// this returns true if the second arg is after the first
static inline int seq_order(seq_t a, seq_t b)
{
    if(b > a)
    {
        return 1;
    }
    else if((b < 1000) && (a > 60000))
    {
       return 1;
    }
    else
    {
        return 0;
    }
}

static void packet_decrypt(char *packet, uint8_t *buf, int len){
    unsigned char iv[16];
    int aeslen = len & ~0xf;

    //rk_printf("\n-----len: %d\n", len);
    //printf("\n -----aeslen: %d\n", aeslen);

    if(len > MAX_PACKET)
    {
        AIRPLAY_DEBUGF("The packet is too large, len: %d\n", len);
        return;
    }

    memcpy(iv, aesiv, sizeof(iv));
    AES_cbc_encrypt(buf, packet, aeslen, &aes, iv, AES_DECRYPT);
    memcpy(packet+aeslen, buf+aeslen, len-aeslen);

    return;
}

static int init_decoder(int32_t fmtp[12]) {
    uint32_t airplay_fileflag = 0;
    char alac_flag[7] = "ftypM4A";

    printf("\n init_decoder start\n");
    frame_size = fmtp[1]; // stereo samples
    sampling_rate = fmtp[11];

    int sample_size = fmtp[3];
    if (sample_size != 16){
        AIRPLAY_DEBUGF("only 16-bit samples supported!\n");
        shairport_shutdown();
    }

    Airplay_WriteData((unsigned char*)&airplay_fileflag, 4);
    Airplay_WriteData(alac_flag, 7);
    Airplay_WriteData((unsigned char*)fmtp, 48);
    printf("\n init_decoder end\n");

    return 0;
}


static void free_decoder(void) {
    //alac_free(decoder_info);
}

static void pop_buffer(void);

static void init_buffer(void) {
    ab_write = ab_read = 0;
    reamin = 0;
    losepackecnt = 0;
    delaypackecnt = 0;
    queuepackecnt = 0;
    memset(audio_buffer, 0, sizeof(abuf_t) * BUFFER_FRAMES);

    osPopBufferSem = rkos_semaphore_create(60, 0);
    hPopBuffer = (HTC)RKTaskCreate2(pop_buffer, NULL, NULL, "pop buffer", 128  * 2 * 4, 18, NULL);
}

static void free_buffer(void) {
    int i;
    for (i=0; i<BUFFER_FRAMES; i++)
        if(audio_buffer[i].data)
            airplay_free(audio_buffer[i].data);

    RKTaskDelete2(hPopBuffer);

}

static void clear_buffer(seq_t seqno) {
    abuf_t * buf = &audio_buffer[BUFIDX(seqno)];

    #if 1
    if (buf->data) {
        airplay_free(buf->data);
        buf->data = NULL;
    }

    memset(buf, 0, sizeof(abuf_t));
    #else
    buf->ready = 0;
    buf->len = 0;
    buf->rsd_count = 0;
    buf->rsd_end = 0;
    buf->seqno = 0;
    #endif
}

static inline int first_packet(void) {
    return (ab_write == 0 && ab_read == 0);
}

unsigned char packet[MAX_PACKET];
#define BUF_SIZE_MAX 2048//1024
static void send_buffer(char* data, int len, seq_t no) {//jjjhhh 

#if 0  
    int i;
    packet_decrypt(&packet[reamin], data, len);

    //把数据写入FIFO
    write_fifo = 1;//rk_printf("len=%d\n",len);
    Airplay_WriteData(packet, ((len + reamin) / 512) * 512);
    write_fifo = 0;

    i = (len + reamin) % 512;

    if((((len + reamin) / 512) * 512 + i) > MAX_PACKET)
    {
        rk_printf("len too large len = %d, remain = %d", len, reamin);
    }

    memcpy(packet, &packet[((len + reamin) / 512) * 512], i);

    reamin = i;
#else
	int writen_size = 0;
    int writing_size = 0;
	
    packet_decrypt(&packet[reamin], data, len);
	//rk_printf("len=%d\n",len);
	writing_size = len + reamin;
	while(writing_size >= BUF_SIZE_MAX)
	{
		//把数据写入FIFO
		write_fifo = 1;
		Airplay_WriteData(&packet[writen_size], BUF_SIZE_MAX);
		write_fifo = 0;
		writen_size += BUF_SIZE_MAX;
		writing_size -= BUF_SIZE_MAX;
	}
	memcpy(packet, &packet[writen_size], writing_size);
	reamin = writing_size;
#endif

    AIRPLAY_DEBUGF("Send buffer, no = %d, len = %d\n", no, len);

    ab_read = no;
    clear_buffer(no);
}

//将序列号为seqno的数据包存入audio_buffer, 并请求丢包重传
static void push_buffer(char* data, int len, seq_t seqno) {
    abuf_t * buf = &audio_buffer[BUFIDX(seqno)];

    AIRPLAY_DEBUGF("\n push_buffer\n");
    AIRPLAY_DEBUGF("\n seqno: %d\n", seqno);
    AIRPLAY_DEBUGF("\n BUFIDX(seqno): %d\n", BUFIDX(seqno));

    if (seq_order(ab_write, seqno) && !seq_next(ab_write, seqno))
    {
        if(seqno > ab_write)
        {
            seq_t start = ab_write + 1, end = seqno - 1, i;
            // loss packets, need to be resend.
            for (i = start; i <= end; i++) {
                abuf_t * rsd_buf = &audio_buffer[BUFIDX(i)];

                rsd_buf->seqno = i;
                rsd_buf->rsd_count = 1;
                rsd_buf->ready = 0;
                rsd_buf->rsd_end = end;
            }
            //rk_printf("l = %d", end-start + 1);
            //rtp_request_resend(start, end);
        }
    }

    if(queuepackecnt >= MAX_FRAMES)
    {
        rk_printf("queue %d packet too many", queuepackecnt);
		/*if(g_packet_too_many)
		{
			g_packet_too_many = FALSE;
			queuepackecnt = 0;
		}
		else
			g_packet_too_many = TRUE;*/
    }

    while(queuepackecnt >= MAX_FRAMES)
    {
        {
            abuf_t * buf;
            buf = &audio_buffer[BUFIDX(ab_read + 1)];
            buf->rsd_count = (MAX_RESEND + 1);
        }
        rkos_semaphore_give(osPopBufferSem);
        rkos_sleep(1);
    }

    queuepackecnt++;


    if(buf->memlen < len)
    {
        if (buf->data)
        {
           airplay_free(buf->data);
        }

        buf->data = airplay_malloc(len);
        buf->memlen = len;

    }

    memcpy(buf->data, data, len);

    buf->len = len;
    buf->seqno = seqno;
    buf->rsd_count = 0;
    buf->ready = 1;
    buf->rsd_end = seqno + 1;

}

static void pop_buffer(void)
{
    abuf_t * buf;
    uint32 j;

    while(1)
    {

        AIRPLAY_DEBUGF("\n -----pop_buffer\n");
        AIRPLAY_DEBUGF("-----ab_read: %d\n", ab_read);
        AIRPLAY_DEBUGF("-----ab_write: %d\n", ab_write);

        AIRPLAY_DEBUGF("-----BUFIDX(ab_read + 1): %d\n", BUFIDX(ab_read + 1));
        AIRPLAY_DEBUGF("-----buf->rsd_count: %d\n", buf->rsd_count);
        AIRPLAY_DEBUGF("-----buf->ready: %d\n", buf->ready);

        rkos_semaphore_take(osPopBufferSem, MAX_DELAY);

        while(1)
        {
            if(ab_read == ab_write)
            {
                break;
            }

            buf = &audio_buffer[BUFIDX(ab_read + 1)];

            if (buf->ready)
            {
                queuepackecnt--;
                send_buffer((char*)buf->data, buf->len, buf->seqno);
            }
            else
            {  // No ready for send.
                if (buf->rsd_count > MAX_RESEND)
                {
                    // Sorry, we cannot wait for this packet, it's time to drop it.
                    seq_t start = buf->seqno, end = buf->rsd_end;
                    int drop_count = end - start + 1, i;

                    for (i = start; i <= end; i++)
                        clear_buffer(i);

                    AIRPLAY_DEBUGF("\n!!!!!!!!!! May audio break [%d] !!!!!!!!!!!!\nReason: Resend fail, loss %d packets, %u - %u.\n", ++audio_break, drop_count, start, end);

                    //rk_printf("lose = %d, seqno = %d, end = %d", drop_count, buf->seqno, end);
                    // Pop next to send buffer.
                    losepackecnt += drop_count;
                    ab_read = end;
                }
                else
                {
                /*
				Only lost buffers NOT more than 1/3 seconds,
						 * send RTP resend request.
				*/
		        if (seq_diff(ab_write + 1, buf->seqno) <= 125 / 3)
            rtp_request_resend(ab_write+1, buf->seqno-1);
                    /*buf->rsd_count++;
                    if (buf->rsd_count % MOD_RESEND == 0)
                    {
                        rtp_request_resend(buf->seqno, buf->rsd_end);
                        //rk_printf("rcnt = %d, seqno = %d, rsd = %d, dcnt = %d, lcnt = %d", buf->rsd_count, buf->seqno, (buf->rsd_end - buf->seqno + 1), delaypackecnt, losepackecnt);
                    }*/
                    break;
                }

            }
        }

        //printf("\nr = %d", ab_read);
    }
}

static int test = 0;
void player_put_packet(seq_t seqno, uint8_t *data, int len) {
    AIRPLAY_DEBUGF("\n player_put_packet, seqno: %d\n", seqno);
    //AIRPLAY_DEBUGF("Put a packet seqno %04X\n", seqno);

#if 0  //不考虑丢包重传, 数据直接送FIFO
    send_buffer(data, len, seqno);
    if(seqno < test)
    {
        printf("exit Out of order");
    }
    else if((test + 1) != seqno){
        printf("\nlossed %d packets", seqno - test - 1);
    }
    test = seqno;
#else
    if (first_packet()) {
        AIRPLAY_DEBUGF("First packet, %d - %d.\n", ab_read, ab_write);
        send_buffer(data, len, seqno);
        ab_write = seqno;
        goto rtn;
    }

    AIRPLAY_DEBUGF("Buffer put packet %u.\n", seqno);

    if (seq_order(ab_read, seqno)) {            // The expected packet. ab_read+1 = seqno
        if(seqno < ab_read)
        {
            if((seqno % BUFFER_FRAMES) >= (ab_read % BUFFER_FRAMES))
            {
                rk_printf("buf used1");
                //goto rtn;
            }

        }
        else if(seqno > ab_read)
        {
            if(seqno >= (ab_read + BUFFER_FRAMES))
            {
                rk_printf("buf used2 ab_read = %d, ab_write = %d, segno = %d", ab_read, ab_write, seqno);
                //goto rtn;
            }
        }


    } else if ((seq_order(ab_read, seqno) == 0) || seqno == ab_read) {    // Too late. ab_read >= seqno
        AIRPLAY_DEBUGF("It's too late, packet seqno: %u, read at: %u\n", seqno, ab_read);
        //clear_buffer(seqno);
        delaypackecnt++;
        //rk_printf("m ab = %u, seqno =%u", ab_read, seqno);

        goto rtn;
    }/*else if (seq_order(ab_write, seqno)) {    // newer than expected
         Only lost buffers NOT more than 1/3 seconds,
         * send RTP resend request.
         
        if (seq_diff(ab_write + 1, seqno) <= 125 / 3)
            rtp_request_resend(ab_write+1, seqno-1);
    } */
	
    // Must push to buffer to wait resend.
    push_buffer(data, len, seqno);
    // Try to pop from buffer and send a frame.
rtn:
    rkos_semaphore_give(osPopBufferSem);


    if(seq_order(ab_write, seqno))  //seqno > ab_write
        ab_write = seqno;

    //printf("\nw = %d", ab_write);
#endif
}

/*
 * takes the volume as specified by the airplay protocol
 * airplay volume: 0, -1, -3, -5, -7, -9, -11, -13, -15,
 *                 -16, -18, -20, -22, -24, -26, -28, -144
 */
extern int Airplay_MplayerCtrl_Set_volume(int volume);
void player_volume(int volume) {
    int linear_volume = volume + 32;

    if(linear_volume > 32)
        linear_volume = 32;

    if(linear_volume < 0)
        linear_volume = 0;

    Airplay_MplayerCtrl_Set_volume(linear_volume);
}

int airplay_player_play(stream_cfg *stream) {
    //Airplay_MplayerCtrl_Init();
    //AudioControlTask_SendCmd(AUDIO_CMD_DECSTART, NULL, ASYNC_MODE);

    Airplay_SetFifoSize(-1);

    AES_set_decrypt_key(stream->aeskey, 128, &aes);
    aesiv = stream->aesiv;
    init_decoder(stream->fmtp); //stream->fmtp 在handle_announce中赋值

    // must be after decoder init
    init_buffer();

    AIRPLAY_DEBUGF("Now start play.\n");

    return 0;
}

void airplay_player_stop(void) {
    free_buffer();
    free_decoder();
    AIRPLAY_DEBUGF("\n AUDIO_CMD_STOP\n");
    Airplay_SendCmd(AIRPLAY_CMD_STOP);
    Airplay_SetFifoSize(0);
    AIRPLAY_DEBUGF("\n Stoped.\n");
}
#endif

