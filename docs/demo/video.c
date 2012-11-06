/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

//BEGIN_INCLUDE(all)
#include <android_native_app_glue.h>
#define LOG_TAG "NativePlayer:main"
#include <jni.h>
#include <errno.h>

#include <EGL/egl.h>
#include <GLES/gl.h>
#include <GLES/glext.h>


// for native audio
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#include <android/sensor.h>
#include <android/log.h>



#include <pthread.h>
#include <string.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>


#define UNUSED  __attribute__((unused))
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__))


#define TEXTURE_WIDTH pCodecCtx->width
#define TEXTURE_HEIGHT  pCodecCtx->height

/**
 * Our saved state data.
 */
struct saved_state {
    float angle;
    int32_t x;
    int32_t y;
};

/**
 * Shared state for our app.
 */
struct engine {
    struct android_app* app;

    ASensorManager* sensorManager;
    const ASensor* accelerometerSensor;
    ASensorEventQueue* sensorEventQueue;

    int animating;
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    int32_t width;
    int32_t height;
    struct saved_state state;
};





int playerDraw(struct engine* engine);
int playerDone();
int playerInit(struct engine* engine);

void check_gl_error(const char* op);


static int s_x = 10;
static int s_y = 50;
int s_w = 0;
int s_h = 0;

AVFormatContext     *pFormatCtx;
int                            videoStream;
AVCodec                 *pCodec;
AVFrame                 *pFrame; 
AVFrame                 *pFrameRGB;
AVPacket                  packet;
int                            frameFinished;
int                            numBytes;
uint8_t                    *buffer;
AVCodecContext     *pCodecCtx;
static int                   YUVi=0;
struct SwsContext  *sws_opts;
struct SwsContext  *img_convert_ctx;
static int                   sws_flags = SWS_BICUBIC;



static SLObjectItf bqPlayerObject = NULL;
static SLPlayItf bqPlayerPlay;
static SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;
static SLEffectSendItf bqPlayerEffectSend;
static SLMuteSoloItf bqPlayerMuteSolo;
static SLVolumeItf bqPlayerVolume;

static GLuint s_disable_caps[] = {
    GL_FOG,
    GL_LIGHTING,
    GL_CULL_FACE,
    GL_ALPHA_TEST,
    GL_BLEND,
    GL_COLOR_LOGIC_OP,
    GL_DITHER,
    GL_STENCIL_TEST,
    GL_DEPTH_TEST,
    GL_COLOR_MATERIAL,
    0   
};
/*
const EGLint attribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_BLUE_SIZE, 5,
        EGL_GREEN_SIZE, 6,
        EGL_RED_SIZE, 5,
        EGL_NONE
};
*/
int playerInit(struct engine* engine){
        int i;
        
	av_register_all();

	// Open video file
	if(av_open_input_file(&pFormatCtx, "/sdcard-ext/video/VID_20120314_194434.3gp", NULL, 0, NULL)!=0){
//	if(av_open_input_file(&pFormatCtx, "/sdcard/2.rmvb", NULL, 0, NULL)!=0){
		LOGI("playerInit av_open_input_file error");
		return -1; // Couldn't open file
	}

	// Retrieve stream information
	if(av_find_stream_info(pFormatCtx)<0){
		LOGI("av_find_stream_info error");
		return -1; // Couldn't find stream information
	}


	// Find the first video stream
	videoStream=-1;
	for(i=0; i<pFormatCtx->nb_streams; i++)
		if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) {
		videoStream=i;
		LOGI("videoStream %d", i);
		break;
	}

	// Get a pointer to the codec context for the video stream
	pCodecCtx=pFormatCtx->streams[videoStream]->codec;

	// Find the decoder for the video stream
	pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
	if(pCodec==NULL) {
		LOGI("Unsupported codec!");
		return -1; // Codec not found
	}

	// Open codec
	if(avcodec_open(pCodecCtx, pCodec)<0){
		LOGI("avcodec_open error!");
		return -1; // Could not open codec
	}

	// Allocate video frame
	pFrame=avcodec_alloc_frame();

	LOGI("TEXTURE_WIDTH %d TEXTURE_HEIGHT %d", TEXTURE_WIDTH, TEXTURE_HEIGHT);

	pFrameRGB = avcodec_alloc_frame();
	numBytes=avpicture_get_size(PIX_FMT_RGB565, TEXTURE_WIDTH,
			  TEXTURE_HEIGHT);
	buffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
	avpicture_fill((AVPicture *)pFrameRGB, buffer, PIX_FMT_RGB565,
			 TEXTURE_WIDTH, TEXTURE_HEIGHT);
    

	sws_opts = sws_getContext(16, 16, 0, 16, 16, 0, SWS_BICUBIC,
                                 NULL, NULL, NULL);    
//  rect[4] = { x, 
//        int rect[4] = {0, pCodecCtx->height, pCodecCtx->width, -pCodecCtx->height};
        int rect[4] = {0, pCodecCtx->height, pCodecCtx->width, -pCodecCtx->height};
        glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, rect);
	  
}

int playerDone(){
	// Free the YUV frame
	av_free(pFrame);
	av_free(pFrameRGB);
	av_free(buffer);
	// Close the codec
	avcodec_close(pCodecCtx);

	// Close the video file
	av_close_input_file(pFormatCtx);    
}



int saveYUV(AVFrame * pFrame){

	FILE *pFile;
	char *szFilename="/tmp/debug.YUV";
	int y;
//	LOGI(" pFrame data[0]0x%x data[1]0x%x data[2]0x%x linesize[0] %d linesize[1] %d linesize[2]%d " , 
//		pFrame->data[0],pFrame->data[1],pFrame->data[2],pFrame->linesize[0],pFrame->linesize[1],pFrame->linesize[2]);
	//return 0;

	if(YUVi++ < 10){
		if(YUVi==1){
			pFile=fopen(szFilename, "wb");
		}else{
			pFile=fopen(szFilename, "ab");
		}

		for(y=0; y < pFrame->height; y++)
			fwrite(pFrame->data[0]+y*pFrame->linesize[0], 1, pFrame->width, pFile);

		for(y=0; y < pFrame->height/2; y++)
			fwrite(pFrame->data[1]+y*pFrame->linesize[1], 1, pFrame->width/2, pFile);

		for(y=0; y < pFrame->height/2; y++)
			fwrite(pFrame->data[2]+y*pFrame->linesize[2], 1, pFrame->width/2, pFile);

		fclose(pFile);		
	}
}

void check_gl_error(const char* op)
{
	GLint error;
	for (error = glGetError(); error; error = glGetError())
		LOGI("check_gl_error after %s() glError (0x%x)\n", op, error);
}


static void fix565(AVFrame *pFrame, AVFrame *pFrameRGB)
{

	 sws_flags = av_get_int(sws_opts, "sws_flags", NULL);
        img_convert_ctx = sws_getCachedContext(img_convert_ctx,
            TEXTURE_WIDTH, TEXTURE_HEIGHT, pCodecCtx->pix_fmt, TEXTURE_WIDTH, TEXTURE_HEIGHT,
            PIX_FMT_RGB565, sws_flags, NULL, NULL, NULL);
        if (img_convert_ctx == NULL) {
            LOGI( "Cannot initialize the conversion context");
	     exit(1);
        }    
        sws_scale(img_convert_ctx, pFrame->data, pFrame->linesize,
                  0, TEXTURE_HEIGHT, pFrameRGB->data, pFrameRGB->linesize);

}




int playerDraw(struct engine* engine)
{
	while(av_read_frame(pFormatCtx, &packet)>=0) {
//LOGI("av_read_frame !packet.stream_index[%d], videoStream[%d]", packet.stream_index, videoStream);
		// Is this a packet from the video stream?
		if(packet.stream_index==videoStream) {
			// Decode video frame
			//avcodec_decode_video(pCodecCtx, pFrame, &frameFinished, 
			//	   packet.data, packet.size);
			avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);
			//saveYUV(pFrame);

			// Did we get a video frame?
			if(frameFinished) {
				// Convert the image into YUV format that SDL uses
				//LOGI("img_convert begin!");
				fix565(pFrame,pFrameRGB);
				//LOGI("img_convert done!");
				//rgb24to565(pFrameRGB, pCodecCtx->width, pCodecCtx->height, pixels);
				//LOGI("rgb24to565 done!");
				av_free_packet(&packet);				
                break;
			}else{
				LOGI("frameFinished ");
			}
		}
		// Free the packet that was allocated by av_read_frame
		av_free_packet(&packet);
	}

	glClear(GL_COLOR_BUFFER_BIT);
	glTexImage2D(GL_TEXTURE_2D,		/* target */
			0,			/* level */
			GL_RGB,			/* internal format */
			TEXTURE_WIDTH,		/* width */
			TEXTURE_HEIGHT,		/* height */
			0,			/* border */
			GL_RGB,			/* format */
			GL_UNSIGNED_SHORT_5_6_5,/* type */
			pFrameRGB->data[0]);		/* pixels */
        LOGI("playerDraw engine->width[%d], engine->height[%d] ",engine->width, engine->height);      
	check_gl_error("glTexImage2D");
       glDrawTexiOES(0, 0, 0, engine->width, engine->height);
       check_gl_error("glDrawTexiOES");
       eglSwapBuffers(engine->display, engine->surface);
       return 0;
	/* tell the other thread to carry on */
}


static int  config_changed(struct engine* engine) {
    static GLuint s_texture = 0;
    EGLint w, h, dummy, format;
    EGLSurface surface;
    EGLContext context;
    EGLint numConfigs;
    EGLConfig config;    
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(engine->display, engine->context);
    eglDestroySurface(engine->display, engine->surface);
    eglTerminate(engine->display);

    
    eglInitialize(display, 0, 0);

    /* Here, the application chooses the configuration it desires. In this
     * sample, we have a very simplified selection process, where we pick
     * the first EGLConfig that matches our criteria */
    //eglChooseConfig(display, attribs, &config, 1, &numConfigs);

    /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
     * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
     * As soon as we picked a EGLConfig, we can safely reconfigure the
     * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
    //eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

    //ANativeWindow_setBuffersGeometry(engine->app->window, 0, 0, format);

    surface = eglCreateWindowSurface(display, config, engine->app->window, NULL);
    context = eglCreateContext(display, config, NULL, NULL);

    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
        LOGW("Unable to eglMakeCurrent");
        return -1;
    }

    eglQuerySurface(display, surface, EGL_WIDTH, &w);
    eglQuerySurface(display, surface, EGL_HEIGHT, &h);

    engine->display = display;
    engine->context = context;
    engine->surface = surface;
    engine->width = w;
    engine->height = h;
    engine->state.angle = 0;
    
    glDeleteTextures(1, &s_texture);
    GLuint *start = s_disable_caps;
    while (*start)
        glDisable(*start++);
    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &s_texture);
    glBindTexture(GL_TEXTURE_2D, s_texture);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glShadeModel(GL_FLAT);
    glColor4x(0x10000, 0x10000, 0x10000, 0x10000);
    int rect[4] = {0, pCodecCtx->height, pCodecCtx->width, -pCodecCtx->height};
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, rect);
    
    LOGI("windowResized:width %d height %d:<<<<<<<<<<<<<<<< ",w,h);
//    int rect[4] = {0, h, w, -h};
//    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, rect);



    return 0;   
}


/**
 * Initialize an EGL context for the current display.
 */
static int engine_init_display(struct engine* engine) {
    // initialize OpenGL ES and EGL

    /*
     * Here specify the attributes of the desired configuration.
     * Below, we select an EGLConfig with at least 8 bits per color
     * component compatible with on-screen windows
     */




    
    EGLint w, h, dummy, format;
    EGLint numConfigs;
    EGLConfig config;
    EGLSurface surface;
    EGLContext context;
    static GLuint s_texture = 0;
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    eglInitialize(display, 0, 0);

    /* Here, the application chooses the configuration it desires. In this
     * sample, we have a very simplified selection process, where we pick
     * the first EGLConfig that matches our criteria */
    //eglChooseConfig(display, attribs, &config, 1, &numConfigs);

    /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
     * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
     * As soon as we picked a EGLConfig, we can safely reconfigure the
     * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
    //eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

    //ANativeWindow_setBuffersGeometry(engine->app->window, 0, 0, format);

    surface = eglCreateWindowSurface(display, config, engine->app->window, NULL);
    context = eglCreateContext(display, config, NULL, NULL);

    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
        LOGW("Unable to eglMakeCurrent");
        return -1;
    }

    eglQuerySurface(display, surface, EGL_WIDTH, &w);
    eglQuerySurface(display, surface, EGL_HEIGHT, &h);

    engine->display = display;
    engine->context = context;
    engine->surface = surface;
    engine->width = w;
    engine->height = h;
    engine->state.angle = 0;

    // Initialize GL state.
    //glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    //glEnable(GL_CULL_FACE);
    //glShadeModel(GL_SMOOTH);
    //glDisable(GL_DEPTH_TEST);


    glDeleteTextures(1, &s_texture);
    GLuint *start = s_disable_caps;
    while (*start)
        glDisable(*start++);
    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &s_texture);
    glBindTexture(GL_TEXTURE_2D, s_texture);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glShadeModel(GL_FLAT);
    glColor4x(0x10000, 0x10000, 0x10000, 0x10000);

    playerInit(engine);



    return 0;
}

/**
 * Just the current frame in the display.
 */
static void engine_draw_frame(struct engine* engine) {
    if (engine->display == NULL) {
        // No display.
        return;
    }

    // Just fill the screen with a color.
    /*
    glClearColor(((float)engine->state.x)/engine->width, engine->state.angle,
            ((float)engine->state.y)/engine->height, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    eglSwapBuffers(engine->display, engine->surface);
    */
    playerDraw(engine);
}

/**
 * Tear down the EGL context currently associated with the display.
 */
static void engine_term_display(struct engine* engine) {
    if (engine->display != EGL_NO_DISPLAY) {
        eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (engine->context != EGL_NO_CONTEXT) {
            eglDestroyContext(engine->display, engine->context);
        }
        if (engine->surface != EGL_NO_SURFACE) {
            eglDestroySurface(engine->display, engine->surface);
        }
        eglTerminate(engine->display);
    }
    engine->animating = 0;
    engine->display = EGL_NO_DISPLAY;
    engine->context = EGL_NO_CONTEXT;
    engine->surface = EGL_NO_SURFACE;
    playerDone();
}

/**
 * Process the next input event.
 */
static int32_t engine_handle_input(struct android_app* app, AInputEvent* event) {
    struct engine* engine = (struct engine*)app->userData;
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        engine->animating = 1;
        engine->state.x = AMotionEvent_getX(event, 0);
        engine->state.y = AMotionEvent_getY(event, 0);
        return 1;
    }
    return 0;
}

/**
 * Process the next main command.
 */
static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
LOGI("engine_handle_cmd:cmd[%d]",cmd);
    struct engine* engine = (struct engine*)app->userData;
    switch (cmd) {
        case APP_CMD_SAVE_STATE:
            // The system has asked us to save our current state.  Do so.
            engine->app->savedState = malloc(sizeof(struct saved_state));
            *((struct saved_state*)engine->app->savedState) = engine->state;
            engine->app->savedStateSize = sizeof(struct saved_state);
            break;
        case APP_CMD_INIT_WINDOW:
            
            // The window is being shown, get it ready.
            if (engine->app->window != NULL) {
                engine_init_display(engine);
                engine_draw_frame(engine);
                
            }
            break;
        case APP_CMD_TERM_WINDOW:
            // The window is being hidden or closed, clean it up.
            engine_term_display(engine);
            break;
        case APP_CMD_GAINED_FOCUS:
            // When our app gains focus, we start monitoring the accelerometer.
            if (engine->accelerometerSensor != NULL) {
                ASensorEventQueue_enableSensor(engine->sensorEventQueue,
                        engine->accelerometerSensor);
                // We'd like to get 60 events per second (in us).
                ASensorEventQueue_setEventRate(engine->sensorEventQueue,
                        engine->accelerometerSensor, (1000L/60)*1000);
            }
            break;
        case APP_CMD_CONFIG_CHANGED:
            //config_changed(engine);
            //engine_draw_frame(engine);
            
            break;
        case APP_CMD_LOST_FOCUS:
            // When our app loses focus, we stop monitoring the accelerometer.
            // This is to avoid consuming battery while not being used.
            if (engine->accelerometerSensor != NULL) {
                ASensorEventQueue_disableSensor(engine->sensorEventQueue,
                        engine->accelerometerSensor);
            }
            // Also stop animating.
            engine->animating = 0;
            engine_draw_frame(engine);
            break;
    }
}

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main(struct android_app* state) {
    struct engine engine;

    // Make sure glue isn't stripped.
    app_dummy();

    memset(&engine, 0, sizeof(engine));
    state->userData = &engine;
    state->onAppCmd = engine_handle_cmd;
    state->onInputEvent = engine_handle_input;
    engine.app = state;

    // Prepare to monitor accelerometer
    engine.sensorManager = ASensorManager_getInstance();
    engine.accelerometerSensor = ASensorManager_getDefaultSensor(engine.sensorManager,
            ASENSOR_TYPE_ACCELEROMETER);
    engine.sensorEventQueue = ASensorManager_createEventQueue(engine.sensorManager,
            state->looper, LOOPER_ID_USER, NULL, NULL);

    if (state->savedState != NULL) {
        // We are starting with a previous saved state; restore from it.
        engine.state = *(struct saved_state*)state->savedState;
    }

    // loop waiting for stuff to do.

    while (1) {
        // Read all pending events.
        int ident;
        int events;
        struct android_poll_source* source;

        // If not animating, we will block forever waiting for events.
        // If animating, we loop until all events are read, then continue
        // to draw the next frame of animation.
        while ((ident=ALooper_pollAll(engine.animating ? 0 : -1, NULL, &events,
                (void**)&source)) >= 0) {


            // Process this event.
            if (source != NULL) {
                source->process(state, source);
                 LOGI("ALooper_pollAll:ident[%d] id[%d]:<<<<<<<<<<<<<<<< ",ident, source->id);
            }

            // If a sensor has data, process it now.
            if (ident == LOOPER_ID_USER) {
                if (engine.accelerometerSensor != NULL) {
                    ASensorEvent event;
                    while (ASensorEventQueue_getEvents(engine.sensorEventQueue,
                            &event, 1) > 0) {
//	                        LOGI("accelerometer: x=%f y=%f z=%f",
//	                                event.acceleration.x, event.acceleration.y,
//	                                event.acceleration.z);
                    }
                }
            }

            // Check if we are exiting.
            if (state->destroyRequested != 0) {
                engine_term_display(&engine);
                return;
            }
        }

        if (engine.animating) {
            // Done with events; draw next animation frame.
            engine.state.angle += .01f;
            if (engine.state.angle > 1) {
                engine.state.angle = 0;
            }

            // Drawing is throttled to the screen update rate, so there
            // is no need to do timing here.
            engine_draw_frame(&engine);
        }
    }
}
//END_INCLUDE(all)
