/*
 * simple android media player based on the FFmpeg libraries
 * 2012-3
 * yubo@yubo.org
 */

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

void av_log_callback(void* ptr, int level, const char* fmt, va_list vl);
int saveYUV(AVFrame * pFrame);
int IsSpace(int c);
void LTrim(char * s);
void RTrim(char *s);
void trim(char *s);
int htoi(char c);
void decode_filename(char* out,const char* in);

#endif /* NATIVEPLAYER_UTILS_H */
