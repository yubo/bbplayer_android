/*
 * simple android media player based on the FFmpeg libraries
 * 2012-3
 * yubo@yubo.org
 */

#define LOG_TAG "NativePlayer:player"
 
#include <jni.h>
#include <sys/time.h>
#include <time.h>
#include <android/log.h>
#include <stdint.h>
#include "player.h"

static VideoState *is;


/* Call to initialize the graphics state */
void
Java_org_yubo_player_NPlayerRenderer_nativeInit( JNIEnv*  env , jobject thiz, jint argc, jstring argv)
{
    const char *ARGV;
    ARGV = (*env)->GetStringUTFChars(env, argv, NULL);
    iLog("Java_org_yubo_player_NPlayerRenderer_nativeInit argc[%d] argv[%s]",argc, ARGV);
    is = nativeInit(argc, ARGV);
    (*env)->ReleaseStringUTFChars(env, argv, ARGV);
}

void
Java_org_yubo_player_NPlayerRenderer_nativeResize( JNIEnv*  env, jobject  thiz, jint w, jint h )
{
    nativeResize(w,h,is);
}

/* Call to finalize the graphics state */
void
Java_org_yubo_player_NPlayerRenderer_nativeDone( JNIEnv*  env )
{
	nativeDone(is);
}

/* Call to render the next GL frame */
void
Java_org_yubo_player_NPlayerRenderer_nativeRender( JNIEnv*  env )
{
//	    iLog("Java_org_yubo_player_NPlayerRenderer_nativeRender begin");
    if(is->video_st)
        nativeRender(is);
}

jint
Java_org_yubo_player_NPlayerRenderer_nativeStart( JNIEnv*  env, jobject  thiz, jint w, jint h  )
{
    iLog("Java_org_yubo_player_NPlayerRenderer_nativeStart begin");
    return nativeStart(w,h,is);
}


void
Java_org_yubo_player_NativePlayer_nativeSeek( JNIEnv*  env, jobject  thiz, jint seek  )
{
    iLog("Java_org_yubo_player_NPlayerRenderer_nativeSeek begin");
    nativeSeek(seek,is);
}


/* This is called to indicate to the render loop that it should
 * stop as soon as possible.
 */
void
Java_org_yubo_player_NativePlayer_nativeNotify( JNIEnv*  env, jobject thiz, jint msgid )
{
    iLog("Java_org_yubo_player_NPlayerRenderer_nativePause begin");
    nativeNotify(is,msgid);
}

jint
Java_org_yubo_player_NPlayerRenderer_nativeGetClock( JNIEnv*  env, jobject thiz)
{
    iLog("Java_org_yubo_player_NativePlayer_nativeGetClock begin");
    return nativeGetClock(is);
}


void
Java_org_yubo_player_NPlayerRenderer_nativeSetObj( JNIEnv*  env, jobject thiz, jobject obj)
{
    iLog("Java_org_yubo_player_NativePlayer_nativeSetObj begin");
    is->np_env = env;
//    is->np_cls = (*env)->FindClass(env,"org.yubo.player.NativePlayer");
    is->np_obj = (*env)->NewGlobalRef(env, obj);
    is->np_cls = (*env)->GetObjectClass(env, is->np_obj); 
    is->np_NativePlayerMsg_mid = (*env)->GetMethodID(env, is->np_cls, "NativePlayerMsg","(I)V");
    iLog("env[0x%x]thiz[0x%x]obj[0x%x]mid1[0x%x]",env , thiz, obj, is->np_NativePlayerMsg_mid );
}

jint
Java_org_yubo_player_NPlayerRenderer_nativeGetVh( JNIEnv*  env, jobject thiz)
{
    return is->video_st->codec->height;
}

jint
Java_org_yubo_player_NPlayerRenderer_nativeGetVw( JNIEnv*  env, jobject thiz)
{
    return is->video_st->codec->width;
}



