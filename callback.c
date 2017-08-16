#include "Xstation86.h"

extern unsigned char *mem_v30;
extern int fps_flag;
extern int cll;
extern SPRITE_DATA *splite;
extern unsigned char *flash_v30;
extern int xfb_tbl[];
extern int select_flsec;
extern int xslide, yslide;
extern uint8_t *xfb_gpu;
extern int def_ss;

unsigned int select_splite;
int pmd_sel = 0;
int pmd_time = 0;
int pmd_ch = 0;
int pmd_ret = 0;
int pmd_stat = 0;

void v30_read_memory(void* _ctx,unsigned int address,unsigned char *buf,int size)
{
	memcpy(buf,mem_v30+(address&0xfffff),size);
}

/* called when writing memory */
void v30_write_memory(void* _ctx,unsigned int address,unsigned char *buf,int size)
{
	memcpy(mem_v30+(address&0xfffff),buf,size);
}

#define RIO_WBUF(dat) \
	if(size == 1) *((unsigned char *)buf) = (dat); \
	if(size == 2) *((unsigned short *)buf) = (dat); \

#define WIO_RBUF(dat) \
	if(size == 1) (dat) = *((unsigned char *)buf); \
	if(size == 2) (dat) = *((unsigned short *)buf); \

#define WIO_RBUF2 (size == 1 ? *((unsigned char *)buf) : *((unsigned short *)buf))

/* called when reading an I/O port */
void v30_read_io(void* _ctx,unsigned int address,unsigned char *buf,int size)
{
	memset(buf,0,size);

	Uint8 *keystate = SDL_GetKeyState(NULL);
	int keybit = 0;

	if(keystate[SDLK_z]) {
		keybit |= 0x01;	// A
	}
	if(keystate[SDLK_x]) {
		keybit |= 0x02;	// B
	}
	if(keystate[SDLK_b]) {
		keybit |= 0x04;	// select
	}
	if(keystate[SDLK_n]) {
		keybit |= 0x08;	// start
	}

	if(keystate[SDLK_UP]) {
		keybit |= 0x10;	// up
	}
	if(keystate[SDLK_DOWN]) {
		keybit |= 0x20;	// down
	}
	if(keystate[SDLK_LEFT]) {
		keybit |= 0x40;	// left
	}
	if(keystate[SDLK_RIGHT]) {
		keybit |= 0x80;	// right
	}

	switch(address) {
	case 4:
		RIO_WBUF(pmd_ret);
		break;
	case 21:
		RIO_WBUF(keybit);
		break;
	case 22:
		RIO_WBUF(fps_flag);
		fps_flag = 0;
		break;
	case 26:
		RIO_WBUF(rand());
		break;
	case 27:
		RIO_WBUF(pmd_stat);
		break;
	default:
		printf("Unknown port read : 0x%04x\n",address);
		break;
	}
}

/* called when writing an I/O port */
void v30_write_io(void* _ctx,unsigned int address,unsigned char *buf,int size)
{
	/*memcpy(&(port_v30[address]),buf,size);
	port_v30_flag[address] = true;**/

	int _cll;
	int tmp,i,j;
	int tmpl[256];
	struct HW_BOXDRAW *hwbox;
	struct HW_STRDRAW *hwstr;
	struct HW_BGDRAW *hwbg;
	unsigned short *data;
	unsigned short *bgfb;
	char *str;

	unsigned short tmp1;
	short *tmp2;

	switch(address) {
	case 0:
		cll = WIO_RBUF2 & 0xff;
		break;
	case 1:
		pmd_sel = WIO_RBUF2;
		break;
	case 2:
		pmd_time = WIO_RBUF2 * 11025;
		break;
	case 3:
		stop_pcm(WIO_RBUF2);
		break;
	case 4:
		pmd_ret = play_pcm((void *)&flash_v30[512*pmd_sel],pmd_time);
		break;
	case 5:
		i = 0;
		j = 0;
		tmp = WIO_RBUF2;
		for(i = 0; i < 4; i++) {
			if(i != tmp) tmpl[j++] = xfb_tbl[i];
		}
		xfb_tbl[0] = tmp;
		xfb_tbl[1] = tmpl[0];
		xfb_tbl[2] = tmpl[1];
		xfb_tbl[3] = tmpl[2];
		break;
	case 6:
		i = 0;
		j = 0;
		tmp = WIO_RBUF2;
		for(i = 0; i < 4; i++) {
			if(i != tmp) tmpl[j++] = xfb_tbl[i];
		}
		xfb_tbl[3] = tmp;
		xfb_tbl[2] = tmpl[2];
		xfb_tbl[1] = tmpl[1];
		xfb_tbl[0] = tmpl[0];
		break;
	case 7:
		WIO_RBUF(select_splite);
		break;
	case 8:
		WIO_RBUF(splite[select_splite].use);
		break;
	case 9:
		WIO_RBUF(splite[select_splite].sx);
		break;
	case 10:
		WIO_RBUF(splite[select_splite].sy);
		break;
	case 11:
		tmp1 = WIO_RBUF2;
		splite[select_splite].vx = *((short *)&tmp1);
		break;
	case 12:
		tmp1 = WIO_RBUF2;
		splite[select_splite].vy = *((short *)&tmp1);
		break;
	case 13:
		splite[select_splite].graph = flash_v30 + (512 * WIO_RBUF2);
		splite[select_splite].scalex = 1.0f;
		splite[select_splite].scaley = 1.0f;
		break;
	case 14:
		splite[select_splite].scalex = (float)*((short *)buf) / 100.0f;
		splite[select_splite].scaley = (float)*((short *)buf) / 100.0f;
		break;
	case 15:
		select_flsec = WIO_RBUF2;
		break;
	case 17:
		tmp1 = WIO_RBUF2;
		xslide = *((short *)&tmp1);
		break;
	case 18:
		tmp1 = WIO_RBUF2;
		yslide = *((short *)&tmp1);
		break;
	case 19:
		hwbox = (struct HW_BOXDRAW *)(mem_v30 + WIO_RBUF2 + 0x10000 + (def_ss << 4));
		boxfill8(xfb_gpu,SCRN_X, hwbox->c, hwbox->x0, hwbox->y0, hwbox->x1, hwbox->y1);
		break;
	case 20:
		xfb_tbl[0] = 0;
		xfb_tbl[1] = 3;
		xfb_tbl[2] = 2;
		xfb_tbl[3] = 1;
		memcpy(mem_v30+VRAM_ADDR_BMP,xfb_gpu,320*200);
		break;
	case 23:
		splite[select_splite].scalex = (float)WIO_RBUF2 / 100.0f;
		splite[select_splite].scaley = (float)WIO_RBUF2 / 100.0f;
		break;
	case 24:
		splite[select_splite].scalex = (float)WIO_RBUF2 / 100.0f;
		break;
	case 25:
		splite[select_splite].scaley = (float)WIO_RBUF2 / 100.0f;
		break;
	case 27:
		pmd_stat = status_pcm(WIO_RBUF2);
		break;
	case 28:
		hwstr = (struct HW_STRDRAW *)(mem_v30 + WIO_RBUF2 + 0x10000 + (def_ss << 4));
		str = (char *)(mem_v30 + (int)hwstr->str + 0x10000 + (def_ss << 4));
		boxfill8(xfb_gpu,SCRN_X, 0, hwstr->x, hwstr->y, hwstr->x + (strlen(str)*8),hwstr->y+8);
		putfonts8x8(xfb_gpu,SCRN_X,hwstr->x,hwstr->y,hwstr->c,/*"ï€ã∂í}âŸâŸã∞íoùbèc"*/str);
		break;
	case 29:
		hwbg = (struct HW_BGDRAW *)(mem_v30 + WIO_RBUF2 + 0x10000 + (def_ss << 4));
		int x,y;
		data = (unsigned short *)(mem_v30 + (unsigned short)hwbg->data + 0x10000 + (def_ss << 4));
		bgfb = (unsigned short *) &mem_v30[VRAM_ADDR_IBG];
		for(y = 0; y < hwbg->sy; y++) {
			for(x = 0; x < hwbg->sx; x++) {
				bgfb[(hwbg->y+y) * 80 + (hwbg->x+x)] = data[y * hwbg->sx + x];
			}
		}
		break;
	default:
		printf("Unknown port write : 0x%04x data 0x%04x\n",address,WIO_RBUF2);
		break;
	}
}

unsigned char v30_read_memory_byte(void *_ctx, unsigned long address)
{
	unsigned char buf;
	v30_read_memory(_ctx, address, (unsigned char *)&buf, 1);
	return buf;
}

void v30_write_memory_byte(void *_ctx, unsigned long address, unsigned char data)
{
	v30_write_memory(_ctx, address, (unsigned char *)&data, 1);
}

unsigned short v30_read_memory_word(void *_ctx, unsigned long address)
{
	unsigned short buf;
	v30_read_memory(_ctx, address, (unsigned char *)&buf, 2);
	return buf;
}

void v30_write_memory_word(void *_ctx, unsigned long address, unsigned short data)
{
	v30_write_memory(_ctx, address, (unsigned char *)&data, 2);
}

unsigned char v30_read_io_byte(void *_ctx, unsigned long address)
{
	unsigned char buf;
	v30_read_io(_ctx, address, (unsigned char *)&buf, 1);
	return buf;
}

void v30_write_io_byte(void *_ctx, unsigned long address, unsigned char data)
{
	v30_write_io(_ctx, address, (unsigned char *)&data, 1);
}

unsigned short v30_read_io_word(void *_ctx, unsigned long address)
{
	unsigned short buf;
	v30_read_io(_ctx, address, (unsigned char *)&buf, 2);
	return buf;
}

void v30_write_io_word(void *_ctx, unsigned long address, unsigned short data)
{
	v30_write_io(_ctx, address, (unsigned char *)&data, 2);
}

/* function to set up the callbacks for given cpu */
void v30_setup_callbacks(e8086_t *v30)
{
	v30->mem = v30;
	v30->prt = v30;

	v30->mem_get_uint8 = v30_read_memory_byte;
	v30->mem_get_uint16 = v30_read_memory_word;

	v30->mem_set_uint8 = v30_write_memory_byte;
	v30->mem_set_uint16 = v30_write_memory_word;

	v30->prt_get_uint8 = v30_read_io_byte;
	v30->prt_get_uint16 = v30_read_io_word;

	v30->prt_set_uint8 = v30_write_io_byte;
	v30->prt_set_uint16 = v30_write_io_word;
}