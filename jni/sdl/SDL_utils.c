#include "SDL_utils.h"

void SDL_Error(SDL_errorcode code){
	eLog("SDL_Error %d",code);
}
void SDL_SetError(char *c){
	eLog("SDL_SetError %s",c);
}
void SDL_ClearError(void)
{
}
/*
void SDL_Delay (Uint32 ms)
{
	int was_error;

	struct timespec elapsed, tv;

	elapsed.tv_sec = ms/1000;
	elapsed.tv_nsec = (ms%1000)*1000000;
	do {
		tv.tv_sec = elapsed.tv_sec;
		tv.tv_nsec = elapsed.tv_nsec;
		was_error = nanosleep(&tv, &elapsed);
	} while ( was_error );
}
*/


