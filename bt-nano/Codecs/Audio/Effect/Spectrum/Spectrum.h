#ifndef _SPECTRUM_H_
#define _SPECTRUM_H_

//------------------------------------------------------------------------------
//Music Section define

#define     _ATTR_SPECTRUM_TEXT_          __attribute__((section("AudioCode")))//__attribute__((section("SpectrumCode")))
#define     _ATTR_SPECTRUM_DATA_          __attribute__((section("AudioData")))//__attribute__((section("SpectrumData")))
#define     _ATTR_SPECTRUM_BSS_           __attribute__((section("AudioBss"),zero_init))//__attribute__((section("SpectrumBss"),zero_init))

#define FFT_BUFFER_SIZE      64     //perform fft pcm size(16bits/sample)

typedef short          Spectrum_int16;
typedef unsigned short Spectrum_uint16;
typedef long           Spectrum_int32;
typedef unsigned long  Spectrum_uint32;
typedef char           Spectrum_char;
typedef unsigned char  Spectrum_uchar;
typedef unsigned char  Spectrum_bool;

typedef struct{
    Spectrum_int16 r;
    Spectrum_int16 i;
}Complex;

void Spectrum_fft_main(Spectrum_int16* pcmreadbuf,Spectrum_int16 *fft_output);
void Spectrum_fft_perform(Spectrum_int16* pcminput, Spectrum_int16* fftin, Spectrum_int16* fftout);
void Spectrum_fft_prepare(Spectrum_int16* pcminput ,Spectrum_int16* fftin);
void Spectrum_fft_calculate(Complex* fftin , Complex* fftout );
void Spectrum_fft_output(Complex* fftout , Spectrum_int32* intensityout);

#endif  /* _SPECTRUM_H */
