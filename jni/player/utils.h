#ifndef NATIVEPLAYER_UTILS_H
#define NATIVEPLAYER_UTILS_H

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <android/log.h>
#include <pthread.h>



#define UNUSED  __attribute__((unused))
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))
#define eLog(...) ((void)__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))
#define iLog(...) ((void)__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__))
#define wLog(...) ((void)__android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__))

/* This is the system-independent thread info structure */
typedef struct  {
	Uint32 threadid;
	SYS_ThreadHandle handle;
	int status;
	SDL_error errbuf;
	void *data;
} SDL_Thread;


int EGL_DisplayRGBOverlay(VideoState *is, SDL_Overlay *overlay, SDL_Rect *dstrect);
int saveYUV(AVFrame * pFrame);

#endif /* NATIVEPLAYER_UTILS_H */
