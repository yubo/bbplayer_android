LOCAL_PATH := $(call my-dir)

ifeq ($(TARGET_ARCH_ABI),armeabi)
	include $(CLEAR_VARS)
	LOCAL_MODULE := libavcodec
	LOCAL_SRC_FILES := lib/armeabi/libavcodec.a
	include $(PREBUILT_STATIC_LIBRARY)	

	include $(CLEAR_VARS)
	LOCAL_MODULE := libavformat
	LOCAL_SRC_FILES := lib/armeabi/libavformat.a
	include $(PREBUILT_STATIC_LIBRARY)

	include $(CLEAR_VARS)
	LOCAL_MODULE := libavutil
	LOCAL_SRC_FILES := lib/armeabi/libavutil.a
	include $(PREBUILT_STATIC_LIBRARY)

	include $(CLEAR_VARS)
	LOCAL_MODULE := libswscale
	LOCAL_SRC_FILES := lib/armeabi/libswscale.a
	include $(PREBUILT_STATIC_LIBRARY)

	include $(CLEAR_VARS)
	LOCAL_MODULE := libswresample
	LOCAL_SRC_FILES := lib/armeabi/libswresample.a
	include $(PREBUILT_STATIC_LIBRARY)
endif


ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
	include $(CLEAR_VARS)
	LOCAL_MODULE := libavcodec
	LOCAL_SRC_FILES := lib/armeabi-v7a/libavcodec.a
	include $(PREBUILT_STATIC_LIBRARY)	

	include $(CLEAR_VARS)
	LOCAL_MODULE := libavformat
	LOCAL_SRC_FILES := lib/armeabi-v7a/libavformat.a
	include $(PREBUILT_STATIC_LIBRARY)
	
	include $(CLEAR_VARS)
	LOCAL_MODULE := libavutil
	LOCAL_SRC_FILES := lib/armeabi-v7a/libavutil.a
	include $(PREBUILT_STATIC_LIBRARY)

	include $(CLEAR_VARS)
	LOCAL_MODULE := libswscale
	LOCAL_SRC_FILES := lib/armeabi-v7a/libswscale.a
	include $(PREBUILT_STATIC_LIBRARY)

	include $(CLEAR_VARS)
	LOCAL_MODULE := libswresample
	LOCAL_SRC_FILES := lib/armeabi-v7a/libswresample.a
	include $(PREBUILT_STATIC_LIBRARY)

endif

include $(CLEAR_VARS)
LOCAL_MODULE := ffmpeg
LOCAL_WHOLE_STATIC_LIBRARIES := libavutil libavcodec libavformat  libswscale libswresample
LOCAL_LDLIBS := -lz
#include $(BUILD_SHARED_LIBRARY)