#define LOG_TAG "NativePlayer:player"
#include "player.h"


static pthread_cond_t s_vsync_cond;
static pthread_mutex_t s_vsync_mutex;


AVFormatContext     *pFormatCtx;
AVCodec                 *pCodec;
AVFrame                 *pFrame; 
AVFrame                 *pFrameRGB;
AVPacket                  packet;
int                            frameFinished;
int                            numBytes;
uint8_t                    *buffer;
AVCodecContext     *pCodecCtx;
AVCodecContext     *aCodecCtx;
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
//createAssetAudioPlayer() 
//setPlayingAssetAudioPlayer(isPlayingAsset);
//selectClip(CLIP_HELLO, 5);


#define AUDIO_BUFFER_SIZE 1024

typedef struct PacketQueue {
  AVPacketList *first_pkt, *last_pkt;
  int nb_packets;
  int size;
  pthread_mutex_t *mutex;
  pthread_cond_t *cond;
} PacketQueue;

//		pthread_mutex_lock(&s_vsync_mutex);
//		pthread_cond_wait(&s_vsync_cond, &s_vsync_mutex);
//		pthread_mutex_unlock(&s_vsync_mutex);
//	pthread_cond_signal(&s_vsync_cond);

static int videoInit(){
}

static int videoDone(){
}

static init audioInit(){

}

static init audioDone(){
}



void packet_queue_init(PacketQueue *q) {
    memset(q, 0, sizeof(PacketQueue));
    pthread_cond_init(q->cond , NULL);
    pthread_mutex_init(q->mutex, NULL);
}



int packet_queue_put(PacketQueue *q, AVPacket *pkt) {

    AVPacketList *pkt1;
    if(av_dup_packet(pkt) < 0) {
        return -1;
    }
    pkt1 = av_malloc(sizeof(AVPacketList));
    if (!pkt1)
        return -1;
    pkt1->pkt = *pkt;
    pkt1->next = NULL;

    pthread_mutex_lock(q->mutex);

    if (!q->last_pkt)
        q->first_pkt = pkt1;
    else
        q->last_pkt->next = pkt1;
    q->last_pkt = pkt1;
    q->nb_packets++;
    q->size += pkt1->pkt.size;
    pthread_cond_signal(q->cond);

    pthread_mutex_unlock(q->mutex);
    return 0;
}
static int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block)
{
    AVPacketList *pkt1;
    int ret;

    pthread_mutex_lock(q->mutex);

    for(;;) {

        if(quit) {
            ret = -1;
            break;
        }

        pkt1 = q->first_pkt;
        if (pkt1) {
            q->first_pkt = pkt1->next;
            if (!q->first_pkt)
            q->last_pkt = NULL;
            q->nb_packets--;
            q->size -= pkt1->pkt.size;
            *pkt = pkt1->pkt;
            av_free(pkt1);
            ret = 1;
            break;
        } else if (!block) {
            ret = 0;
            break;
        } else {
            pthread_cond_wait(q->cond, q->mutex);
        }
    }
    pthread_mutex_unlock(q->mutex);
    return ret;
}

int audio_decode_frame(AVCodecContext *aCodecCtx, uint8_t *audio_buf, int buf_size) {

  static AVPacket pkt;
  static uint8_t *audio_pkt_data = NULL;
  static int audio_pkt_size = 0;

  int len1, data_size;

  for(;;) {
    while(audio_pkt_size > 0) {
      data_size = buf_size;
      len1 = avcodec_decode_audio2(aCodecCtx, (int16_t *)audio_buf, &data_size, 
				  audio_pkt_data, audio_pkt_size);
      if(len1 < 0) {
	/* if error, skip frame */
	audio_pkt_size = 0;
	break;
      }
      audio_pkt_data += len1;
      audio_pkt_size -= len1;
      if(data_size <= 0) {
	/* No data yet, get more frames */
	continue;
      }
      /* We have data, return it and come back for more later */
      return data_size;
    }
    if(pkt.data)
      av_free_packet(&pkt);

    if(quit) {
      return -1;
    }

    if(packet_queue_get(&audioq, &pkt, 1) < 0) {
      return -1;
    }
    audio_pkt_data = pkt.data;
    audio_pkt_size = pkt.size;
  }
}

// this callback handler is called every time a buffer finishes playing
void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
{
    assert(bq == bqPlayerBufferQueue);
    assert(NULL == context);
    // for streaming playback, replace this test by logic to find and fill the next buffer
    if (--nextCount > 0 && NULL != nextBuffer && 0 != nextSize) {
        SLresult result;
        // enqueue another buffer
        result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, nextBuffer, nextSize);
        // the most likely other result is SL_RESULT_BUFFER_INSUFFICIENT,
        // which for this code example would indicate a programming error
        assert(SL_RESULT_SUCCESS == result);
    }
}



int playerInit(struct engine* engine){
    int i,videoStream,audioStream;
    SLresult result;
    
    av_register_all();

    // Open video file
    if(av_open_input_file(&pFormatCtx, "/sdcard-ext/video/VID_20120314_194434.3gp", NULL, 0, NULL)!=0){
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
    audioStream=-1;
    for(i=0; i<pFormatCtx->nb_streams; i++){
        if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO && videoStream<0) {
            videoStream=i;
            LOGI("videoStream %d", i);
            break;
        }
        if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO && audioStream<0) {
            audioStream=i;
            LOGI("audioStream %d", i);
            break;
        }
    }
    if(videoStream==-1)
        return -1; // Didn't find a video stream
    if(audioStream==-1)
        return -1;

    
    //#######################
    //audio init
    //#######################
    aCodecCtx=pFormatCtx->streams[audioStream]->codec;

// engine interfaces
static SLObjectItf engineObject = NULL;
static SLEngineItf engineEngine;
// output mix interfaces
static SLObjectItf outputMixObject = NULL;
static SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;
// aux effect on the output mix, used by the buffer queue player
static const SLEnvironmentalReverbSettings reverbSettings =
    SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;

    // create engine
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    // realize the engine
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    // get the engine interface, which is needed in order to create other objects
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    // create output mix, with environmental reverb specified as a non-required interface
    const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean req[1] = {SL_BOOLEAN_FALSE};
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, ids, req);
    // realize the output mix
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    // get the environmental reverb interface
    // this could fail if the environmental reverb effect is not available,
    // either because the feature is not present, excessive CPU load, or
    // the required MODIFY_AUDIO_SETTINGS permission was not requested and granted
    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
            &outputMixEnvironmentalReverb);
    if (SL_RESULT_SUCCESS == result) {
        result = (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                outputMixEnvironmentalReverb, &reverbSettings);
    }

    // configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 1, SL_SAMPLINGRATE_8,
        SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
        SL_SPEAKER_FRONT_CENTER, SL_BYTEORDER_LITTLEENDIAN};
    SLDataSource audioSrc = {&loc_bufq, &format_pcm};

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};

    // create audio player
    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND,
            /*SL_IID_MUTESOLO,*/ SL_IID_VOLUME};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE,
            /*SL_BOOLEAN_TRUE,*/ SL_BOOLEAN_TRUE};
    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &bqPlayerObject, &audioSrc, &audioSnk,
            3, ids, req);

    // realize the player
    result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
    // get the play interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerPlay);
    // get the buffer queue interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE,
            &bqPlayerBufferQueue);
    // register callback on the buffer queue
    result = (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCallback, NULL);
    // get the effect send interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_EFFECTSEND,
            &bqPlayerEffectSend);
    // get the volume interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_VOLUME, &bqPlayerVolume);
    // set the player's state to playing
    result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
    result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, nextBuffer, nextSize);
    if (SL_RESULT_SUCCESS != result) {
        return JNI_FALSE;
    }

        nextBuffer = (short *) hello;
        nextSize = sizeof(hello);
    
    
    //video init
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
    buffer=(uint16_t *)av_malloc(numBytes*sizeof(uint16_t));
    avpicture_fill((AVPicture *)pFrameRGB, buffer, PIX_FMT_RGB565,
    		 TEXTURE_WIDTH, TEXTURE_HEIGHT);


    sws_opts = sws_getContext(16, 16, 0, 16, 16, 0, SWS_BICUBIC,
                                 NULL, NULL, NULL);
  
	  
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
		LOGI("av_read_frame !packet.stream_index[%d], videoStream[%d]", packet.stream_index, videoStream);
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






