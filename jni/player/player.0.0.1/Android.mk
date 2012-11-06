LOCAL_PATH := /cygdrive/e/yubo/native-player/jni/player

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE    := player

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../libffmpeg 

LOCAL_LDLIBS := -llog -lGLESv1_CM -landroid -lEGL
LOCAL_STATIC_LIBRARIES := avutil avcodec avformat swscale 
LOCAL_SRC_FILES := player.c

include $(BUILD_STATIC_LIBRARY)

