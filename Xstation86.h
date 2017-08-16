#include <SDL/SDL.h>
#include <SDL/SDL_timer.h>
#include "e8086/e8086.h"
#include <stdio.h>

typedef int bool;
#define true 1
#define false 0

/* sdl.cpp */
int poll_event(SDL_Event *sdl_event);

/* callback.cpp */
void v30_setup_callbacks(e8086_t *v30);

/* 86engine.cpp */
int v30_loadrom(e8086_t *v30);

/* audio.cpp */
void init_pcm(void);
int play_pcm(void *pcm_dat, int pcm_time);
void stop_pcm(int pcm_ch);
int status_pcm(int pcm_ch);
void speed_pcm(int pcm_ch, float speed);

/* graphic.cpp */
#define VRAM_ADDR_BMP	0x30000
#define VRAM_ADDR_IBG	0x40000
#define SPRITE_NUM	8192
#define SCRN_X	320
#define	SCRN_Y	200
typedef struct {
	int vx,vy;
	int sx,sy;
	int use;
	unsigned char *graph;
	float scalex;
	float scaley;
} SPRITE_DATA;
void init_graphic(unsigned char *mem_v30, SDL_Surface *sdl_screen);
void refresh_graphic(SDL_Surface *sdl_screen, unsigned char *mem_v30);
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);
void putfonts8x8(unsigned char *vram, int xsize, int x, int y, char c, char *s1);
void init_hwfont(void);
void rgb332_compatible(void);

#define gl_tocolor(r,g,b) (((int)r << 16) | ((int)g << 8) | (int)b)
#define gl_fromcolor(c,r,g,b) (*r = (c >> 16) & 0xff, *g = (c >> 8) & 0xff, *b = c & 0xff)

void libini_iniopen(char *name);
int libini_iniread_n(char *name);
char *libini_iniread_s(char *name);

struct HW_BOXDRAW {
	short x0, y0, x1, y1;
	unsigned char c;
};

struct HW_STRDRAW {
	short x,y;
	unsigned char c;
	unsigned short str;
};

struct HW_BGDRAW {
	short x, y, sx, sy;
	unsigned short *data;
};
