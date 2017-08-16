#include "Xstation86.h"
#include "libgpu.h"

char *font4x8;
char *font8x8;
SPRITE_DATA *splite;
int xslide = 0, yslide = 0;
int select_flsec = 0;
int cll = 0;
extern unsigned char *flash_v30;
extern bool compatible;

uint32_t rgb332_table[256];

int xfb_tbl[4];

uint8_t *xfb_spl;
uint8_t *xfb_gpu;
uint8_t *xfb_bmp;
uint8_t *xfb_ibg;

void init_hwfont(void)
{
	FILE *fp0 = fopen("932-6x8a.bin","rb");
	FILE *fp1 = fopen("932-8x8j.bin","rb");

	font4x8 = (char *)malloc(2048);
	font8x8 = (char *)malloc(65536);

	fread(font4x8,sizeof(char),2048,fp0);
	fread(font8x8,sizeof(char),65536,fp1);
	fclose(fp0);
	fclose(fp1);
}

void init_graphic(unsigned char *mem_v30, SDL_Surface *sdl_screen)
{
	cll = 0;

	int i;

	xfb_tbl[0] = 0;
	xfb_tbl[1] = 1;
	xfb_tbl[2] = 2;
	xfb_tbl[3] = 3;

	memset(mem_v30+VRAM_ADDR_BMP,0,0x10000);
	memset(mem_v30+VRAM_ADDR_IBG,0,0x10000);

	xfb_spl = (uint8_t *)malloc(320*200);
	xfb_gpu = (uint8_t *)malloc(320*200);
	xfb_bmp = (uint8_t *)malloc(320*200);
	xfb_ibg = (uint8_t *)malloc(320*200);

	memset(xfb_spl,0,320*200);
	memset(xfb_gpu,0,320*200);
	memset(xfb_bmp,0,320*200);
	memset(xfb_ibg,0,320*200);

	splite = (SPRITE_DATA *)malloc(sizeof(SPRITE_DATA)*SPRITE_NUM);

	for(i = 0; i < 256; i++) {
		rgb332_table[i] = (i << 16) | (i << 8) | i;
	}

	for(i = 0;i < SPRITE_NUM;i++) {
		splite[i].sx = splite[i].sy = splite[i].vx = splite[i].vy = splite[i].use = 0;
	}
}

void rgb332_compatible(void)
{
	int r,g,b;

	for(r = 0; r < 8; r++) {
		for(g = 0; g < 8; g++) {
			for(b = 0; b < 4; b++) {
				int r8, g8, b8;

				r8 = (r << 5) | (r << 2) | (r >> 1);
				g8 = (g << 5) | (g << 2) | (g >> 1);
				b8 = (b << 6) | (b << 4) | (b << 2) | b;
			
				if(r8 > 255) r8 = 255;
				if(g8 > 255) g8 = 255;
				if(b8 > 255) b8 = 255;

				rgb332_table[(r << 5) | (g << 2) | b] = (r8 << 16) | (g8 << 8) | b8;
			}
		}
	}
}

void putblock_bg(unsigned char *vram, int vxsize, int pxsize, int pysize, int px0, int py0, unsigned char *buf, int bxsize)
{
	int x, y;
	int vx, vy;
	for (y = 0; y < pysize; y++) {
		for (x = 0; x < pxsize; x++) {
			vy = y + py0;
			vx = x + px0;

			vx %= SCRN_X * 2;
			vy %= SCRN_Y * 2;

			if(vy < SCRN_Y && vx < SCRN_X && vy >= 0 && vx >= 0) vram[vy * vxsize + vx] = buf[y * bxsize + x];
		}
	}
	return;
}

void putblock_splite(unsigned char *vram, int vxsize, SPRITE_DATA splite)
{
	int x, y;
	int vx, vy;
	for (y = 0; y < splite.sy * splite.scaley; y++) {
		for (x = 0; x < splite.sx * splite.scalex; x++) {
			vy = y + splite.vy;
			vx = x + splite.vx;

			int pix = splite.graph[(int)(y / splite.scaley) * splite.sx + (int)(x / splite.scalex)];

			if(vy < SCRN_Y && vx < SCRN_X && vy >= 0 && vx >= 0 && pix != 0xe3) vram[vy * vxsize + vx] = pix;
		}
	}

	return;
}

void refresh_graphic(SDL_Surface *sdl_screen, unsigned char *mem_v30)
{
	int i;
	int x,y;

	int v_ibg = VRAM_ADDR_IBG;

	bool nf[] = {true,true};

	if(cll != 0x00 && compatible) nf[0] = false;
	if(cll != 0x01 && compatible) nf[1] = false;

	if(compatible) v_ibg = VRAM_ADDR_BMP;

	if(nf[0]) {
		for(y = 0; y < SCRN_Y; y++) {
			for(x = 0; x < SCRN_X; x++) {
				xfb_bmp[y * SCRN_X + x] = mem_v30[VRAM_ADDR_BMP + (y * SCRN_X + x)];
			}
		}
	}
	
	if(nf[1]) {
		for(y = 0; y < SCRN_Y * 2; y += 8) {
			for(x = 0; x < SCRN_X * 2; x += 8) {
				unsigned short *bgfb = (unsigned short *) &mem_v30[v_ibg + (((y >> 3) * ((SCRN_X * 2) >> 3) + (x >> 3)) << 1)];
				int bg = *bgfb;
				int tx = (bg & 15) * 8;
				int ty = (bg >> 4) * 8;

				putblock_bg(xfb_bmp, SCRN_X, 8, 8, x + xslide, y + yslide, &(flash_v30[(select_flsec * 512) + (ty * 128 + tx)]), 128);
			}
		}
	}

	for(i = 0;i < SPRITE_NUM; i++) {
		if(splite[i].use != 0) {
			putblock_splite(xfb_spl, SCRN_X, splite[i]);
		}
	}

	uint32_t xfbs[] = { (uint32_t)xfb_bmp,(uint32_t)xfb_ibg,(uint32_t)xfb_spl,(uint32_t)xfb_gpu };
	uint32_t xfba[] = { 0x00,0x00,0xe3,0x00 };

	memset(sdl_screen->pixels,cll,320*200*4);

	for(i = 0; i < 4; i++) {
		for(y = 0; y < SCRN_Y; y++) {
			for(x = 0; x < SCRN_X; x++) {
				int pix = ((uint8_t *)xfbs[xfb_tbl[i]])[y * SCRN_X + x];
				if(pix != xfba[xfb_tbl[i]]) ((int *)sdl_screen->pixels)[y * SCRN_X + x] = rgb332_table[pix];
			}
		}
	}

	memset(xfb_spl,0xe3,320*200);
}

void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1)
{
	int x, y;
	for (y = y0; y <= y1; y++) {
		for (x = x0; x <= x1; x++) {
			if(x >= 0 && x < 320 && y >= 0 && y < 200)vram[y * xsize + x] = c;
		}
	}
	return;
}

void putfont8x8(unsigned char *vram, int xsize, int x, int y, unsigned char c, char *font)
{
	int i;
	unsigned char *p, d;
	
	for (i = 0; i < 8; i++) {
		p = vram + (y + i) * xsize + x;
		d = font[i];
		if ((d & 0x80) != 0) { p[0] = c; }
		if ((d & 0x40) != 0) { p[1] = c; }
		if ((d & 0x20) != 0) { p[2] = c; }
		if ((d & 0x10) != 0) { p[3] = c; }
		if ((d & 0x08) != 0) { p[4] = c; }
		if ((d & 0x04) != 0) { p[5] = c; }
		if ((d & 0x02) != 0) { p[6] = c; }
		if ((d & 0x01) != 0) { p[7] = c; }
	}
	
	return;
}

void putfonts8x8(unsigned char *vram, int xsize, int x, int y, char c, char *s1)
{
	char *font;
	unsigned char *s = (unsigned char *)s1;
	int l = 1;
	int k,t;
	int langbyte1 = 0;
	
	for (; *s != 0x00; s++) {
		if (langbyte1 == 0) {
			if ((0x81 <= *s && *s <= 0x9f) || (0xe0 <= *s && *s <= 0xfc)) {
				langbyte1 = *s;
			} else {
				putfont8x8(vram, xsize, x, y, c, font4x8 + *s * 8);
				x += 6;
			}
		} else {
			if (0x81 <= langbyte1 && langbyte1 <= 0x9f) {
				k = (langbyte1 - 0x81) * 2;
			} else {
				k = (langbyte1 - 0xe0) * 2 + 62;
			}
			if (0x40 <= *s && *s <= 0x7e) {
				t = *s - 0x40;
			} else if (0x80 <= *s && *s <= 0x9e) {
				t = *s - 0x80 + 63;
			} else {
				t = *s - 0x9f;
				k++;
			}
			langbyte1 = 0;
			putfont8x8(vram, xsize, x, y, c, font8x8 + (k * 94 + t) * 8);
			x += 8;
		}
	}
	return;
}