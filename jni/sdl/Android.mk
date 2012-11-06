LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE    := sdl

LOCAL_C_INCLUDES := $(LOCAL_PATH) $(LOCAL_PATH)/include $(LOCAL_PATH)/pthread

LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog 
#LOCAL_STATIC_LIBRARIES := avutil avcodec avformat swscale 
LOCAL_SRC_FILES := thread/pthread/SDL_syscond.c \
		thread/pthread/SDL_sysmutex.c \
		thread/pthread/SDL_syssem.c \
		thread/pthread/SDL_systhread.c \
		thread/SDL_thread.c \
		SDL_utils.c \


include $(BUILD_STATIC_LIBRARY)
