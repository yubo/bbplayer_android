#define LOG_TAG "NativePlayer:player"
#include "player.h"


static pthread_cond_t s_vsync_cond;
static pthread_mutex_t s_vsync_mutex;

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




int playerInit(struct engine* engine){
        int i;
        
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






