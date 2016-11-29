#ifndef AEC_INTERFACE_H
#define AEC_INTERFACE_H
/*aec initial*/
int Aec_init(int aec_pre_len);

/*do aec*/
int Aec_do(short *mic_buf,short *echo_buf,short *out_buf);

#endif

