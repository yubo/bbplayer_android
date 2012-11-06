LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE    := thread

LOCAL_C_INCLUDES := $(LOCAL_PATH) $(LOCAL_PATH)/include $(LOCAL_PATH)/pthread

LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog 
#LOCAL_STATIC_LIBRARIES := avutil avcodec avformat swscale 
LOCAL_SRC_FILES := pthread/SDL_syscond.c \
		pthread/SDL_sysmutex.c \
		pthread/SDL_syssem.c \
		pthread/SDL_systhread.c \
		SDL_thread.c \
		util.c \


include $(BUILD_SHARED_LIBRARY)
