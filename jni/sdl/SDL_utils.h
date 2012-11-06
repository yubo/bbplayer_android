#include "SDL_error.h"
#include <android/log.h>
#include <jni.h>
#include <sys/time.h>

#define LOG_TAG "SDL_LOG"
#define eLog(...) ((void)__android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__))

void SDL_Error(SDL_errorcode code);
void SDL_SetError(char *c);
void SDL_ClearError(void);

