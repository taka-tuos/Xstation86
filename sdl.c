#include "Xstation86.h"

uint32_t button;

int poll_event(SDL_Event *sdl_event)
{
	if(SDL_PollEvent(sdl_event)) {
		switch (sdl_event->type) {
			button = 0;
		case SDL_KEYDOWN:
			button = sdl_event->key.keysym.unicode;
			break;
		case SDL_KEYUP:
			button = 0;
			break;
		case SDL_QUIT:
			return 1;
		}
	}

	return 0;
}