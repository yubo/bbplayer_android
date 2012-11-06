#define LOG_TAG "NativePlayer:utils"
#include "player.h"

static int gYUVi=0;
EGL_Surface  *gVideo;

int EGL_DisplayRGBOverlay(VideoState *is, EGL_Overlay *overlay, SDL_Rect *dstrect)
{
        glClear(GL_COLOR_BUFFER_BIT);
        glTexImage2D(GL_TEXTURE_2D,		/* target */
        	0,			/* level */
        	GL_RGB,			/* internal format */
        	overlay->w,		/* width */
        	overlay->h,		/* height */
        	0,			/* border */
        	GL_RGB,			/* format */
        	GL_UNSIGNED_SHORT_5_6_5,/* type */
        	overlay->pict.data[0]);		/* pixels */

        glDrawTexiOES(dstrect->x, dstrect->y, 0, dstrect->w, dstrect->h);
        check_gl_error("video_image_display");
        eglSwapBuffers(is->engine->display, is->engine->surface);
}

EGL_Overlay *EGL_CreateGRBOverlay(int w, int h, uint32_t format)
{

    EGL_Overlay *overlay;
    uint8_t *pixels;
    

    /* Verify that we support the format */
    switch (format) {
        case GL_UNSIGNED_SHORT_5_6_5:
                break;
        case GL_UNSIGNED_SHORT_4_4_4_4:
        case GL_UNSIGNED_SHORT_5_5_5_1:
        default:
    	eLog("Unsupported GRB format");
    	return(NULL);
    }

    /* Create the overlay structure */
    overlay = (EGL_Overlay *)malloc(sizeof *overlay);
    if ( overlay == NULL ) {
    	SDL_OutOfMemory();
    	return(NULL);
    }
    memset(overlay, 0, (sizeof *overlay));

    /* Fill in the basic members */
    overlay->format = format;
    overlay->w = w;
    overlay->h = h;
    overlay->planes = 1;


    

    /* Find the pitch and offset values for the overlay */
    switch (format) {
        case GL_UNSIGNED_SHORT_5_6_5:
            pixels=(uint8_t *)malloc(avpicture_get_size(PIX_FMT_RGB565, w,h));
            if ( pixels == NULL ) {
            	SDL_OutOfMemory();
            	return(NULL);
            }
	     avpicture_fill(&overlay->pict, pixels, PIX_FMT_RGB565, w, h);
    	break;
        default:
    	/* We should never get here (caught above) */
    	break;
    }

    /* We're all done.. */
    return(overlay);
    
}

EGL_Surface * EGL_SetVideoMode (int width, int height, int bpp, Uint32 flags, VideoState *is){
    EGL_Surface *screen;
    if(gVideo == NULL){
        gVideo = (EGL_Surface *)malloc(sizeof(EGL_Surface));
        if(!gVideo){
            return NULL;
        }
        memset(gVideo,0,sizeof(EGL_Surface));
    }
    screen = gVideo;
    
    if (is->video_st && is->video_st->codec->width) {
        int rect[4] = {0, is->video_st->codec->height, is->video_st->codec->width, -is->video_st->codec->height};
        glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, rect);
    }

    screen->w = width;
    screen->h = height;
    screen->flags = flags;    
    return screen;    
}

void EGL_FreeGRBOverlay(EGL_Overlay * overlay)
{
    if ( overlay == NULL ) {
        return;
    }
    if ( overlay->pict.data[0] ) {
        free(overlay->pict.data[0]);
    }
    free(overlay);
}



int saveYUV(AVFrame * pFrame)
{

	FILE *pFile;
	char *szFilename="/tmp/debug.YUV";
	int y;

	if(gYUVi++ < 10){
		if(gYUVi==1){
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

void SaveFrame(AVFrame *pFrame, int width, int height, int iFrame) {
  FILE *pFile;
  char szFilename[32];
  int  y;
  
  // Open file
  sprintf(szFilename, "frame%d.ppm", iFrame);
  pFile=fopen(szFilename, "wb");
  if(pFile==NULL)
    return;
  
  // Write header
  fprintf(pFile, "P6\n%d %d\n255\n", width, height);
  
  // Write pixel data
  for(y=0; y<height; y++)
    fwrite(pFrame->data[0]+y*pFrame->linesize[0], 1, width*3, pFile);
  
  // Close file
  fclose(pFile);
}


void check_gl_error(const char* op)
{
	GLint error;
	for (error = glGetError(); error; error = glGetError())
		LOGI("after %s() glError (0x%x)\n", op, error);
}
