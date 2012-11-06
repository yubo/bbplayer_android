LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE    := native-player
#BUILD_TYPE := BIG

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../libffmpeg \
			$(LOCAL_PATH)/../sdl \
			$(LOCAL_PATH)/../sdl/include 


LOCAL_LDLIBS := -lz \
		-llog \
		-lGLESv1_CM \
		-landroid \
		-lEGL \
		-lOpenSLES \
		-ldl

		
LOCAL_STATIC_LIBRARIES := sdl 

LOCAL_SRC_FILES := player.c \
		utils.c \
		cmdutils.c \
		app-android.c \
		importff.c


ifeq ($(BUILD_TYPE),BIG)
	LOCAL_LDLIBS := -lz \
			-llog \
			-lGLESv1_CM \
			-landroid \
			-lEGL \
			-lOpenSLES 
	LOCAL_WHOLE_STATIC_LIBRARIES := sdl avutil avcodec avformat swscale swresample
endif

LOCAL_CFLAGS := -DLOG



include $(BUILD_SHARED_LIBRARY)
