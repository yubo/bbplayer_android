LOCAL_PATH := $(call my-dir)


#include $(CLEAR_VARS)
#LOCAL_MODULE := libavcodec
#LOCAL_SRC_FILES := lib/libavcodec.a
#include $(PREBUILT_STATIC_LIBRARY)	

#include $(CLEAR_VARS)
#LOCAL_MODULE := libavformat
#LOCAL_SRC_FILES := lib/libavformat.a
#include $(PREBUILT_STATIC_LIBRARY)

#include $(CLEAR_VARS)
#LOCAL_MODULE := libavutil
#LOCAL_SRC_FILES := lib/libavutil.a
#include $(PREBUILT_STATIC_LIBRARY)

#include $(CLEAR_VARS)
#LOCAL_MODULE := libswscale
#LOCAL_SRC_FILES := lib/libswscale.a
#include $(PREBUILT_STATIC_LIBRARY)


#include $(CLEAR_VARS)
#LOCAL_MODULE := ffmpeg
#LOCAL_WHOLE_STATIC_LIBRARIES := libavutil libavcodec libavformat  libswscale libpostproc
#LOCAL_LDLIBS := -lz
#include $(BUILD_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := ffmpeg
LOCAL_SRC_FILES := lib/libffmpeg.so
include $(PREBUILD_SHARED_LIBRARY)



