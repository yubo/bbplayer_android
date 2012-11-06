LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE    := native-player

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../libffmpeg \
			$(LOCAL_PATH)/../sdl \
			$(LOCAL_PATH)/../sdl/include 

LOCAL_LDLIBS :=  -L$(call host-path,$(LOCAL_PATH)/../libffmpeg/lib) \
		-lz \
		-llog \
		-lGLESv1_CM \
		-landroid \
		-lEGL \
		-lffmpeg \
		-lOpenSLES
#LOCAL_SHARED_LIBRARIES := avutil avcodec avformat swscale 
LOCAL_STATIC_LIBRARIES := sdl android_native_app_glue
LOCAL_SRC_FILES := player.c utils.c cmdutils.c

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)


