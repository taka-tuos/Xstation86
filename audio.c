#include "Xstation86.h"
#include <math.h>

#define PCM_CHANNEL 512

float pcm_count[PCM_CHANNEL];
float pcm_speed[PCM_CHANNEL];
int pcm_ch[PCM_CHANNEL];
int pcm_limitflame[PCM_CHANNEL];
bool pcm_flag[PCM_CHANNEL];
char *pcm_dats[PCM_CHANNEL];

extern int sblen;

void callback_pcm(void *unused, Uint8 *stream, int len);

SDL_AudioSpec Desired;
SDL_AudioSpec Obtained;

int play_pcm(void *pcm_dat, int pcm_time)
{
	int ch;

	for(ch = 0; ch < PCM_CHANNEL; ch++) {
		if(pcm_flag[pcm_ch[ch]] == false) break;
	}

	pcm_flag[pcm_ch[ch]] = true;
	pcm_limitflame[pcm_ch[ch]] = pcm_time;
	pcm_dats[pcm_ch[ch]] = (char *)pcm_dat;
	pcm_count[pcm_ch[ch]] = 0;
	pcm_speed[pcm_ch[ch]] = 1.0;

	return ch;
}

void speed_pcm(int pcm_ch, float speed)
{
	pcm_speed[pcm_ch] = speed;
}

void stop_pcm(int pcm_ch)
{
	pcm_flag[pcm_ch] = false;
}

int status_pcm(int pcm_ch)
{
	return (pcm_flag[pcm_ch] == true)? (1) : (0);
}

void init_pcm(void)
{
	int i;

	for(i = 0; i < PCM_CHANNEL; i++) {
		pcm_count[i] = 0;
		pcm_ch[i] = i;
		pcm_flag[i] = false;
	}

	Desired.freq= 44100; /* Sampling rate: 22050Hz */
	Desired.format= AUDIO_S16; /* 16-bit signed audio */
	Desired.channels= 1; /* Mono */
	Desired.samples= sblen; /* Buffer size: 2205000B = 10 sec. */
	Desired.callback = callback_pcm;
	Desired.userdata = NULL;

	SDL_OpenAudio(&Desired, &Obtained);
	SDL_PauseAudio(0);
}

void callback_pcm(void *unused, Uint8 *stream, int len)
{
	int i;
	int chs;
	static unsigned int step= 0;
	Sint16 *frames = (Sint16 *) stream;
	int framesize = len / 2;
	for (i = 0; i < framesize; i++, step++) {
		int dat = 0;
		for(chs = 0; chs < PCM_CHANNEL; chs++) {
			if(pcm_flag[chs] == true) {
				dat += pcm_dats[chs][(int)pcm_count[chs]] * 128;
				pcm_count[chs] += pcm_speed[chs] / 4.0;
				if(pcm_limitflame[chs] == pcm_count[chs]) pcm_flag[chs] = false;
			}
		}
		if(dat < -32768) dat = -32768;
		if(dat > 32767) dat = 32767;
		frames[i] = dat;
	}
}
