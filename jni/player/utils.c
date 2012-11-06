/*
 * simple android media player based on the FFmpeg libraries
 * 2012-3
 * yubo@yubo.org
 */


#define LOG_TAG "NativePlayer:utils"
#include "player.h"

static int gYUVi=0;
static int glog_level=ANDROID_LOG_UNKNOWN;
static int flags;


static void log_format_line(void *ptr, int level, const char *fmt, va_list vl,
                        char *line, int line_size, int *print_prefix)
{
    AVClass* avc = ptr ? *(AVClass **) ptr : NULL;
    line[0] = 0;
    if (*print_prefix && avc) {
        if (avc->parent_log_context_offset) {
            AVClass** parent = *(AVClass ***) (((uint8_t *) ptr) +
                                   avc->parent_log_context_offset);
            if (parent && *parent) {
                snprintf(line, line_size, "[%s @ %p] ",
                         (*parent)->item_name(parent), parent);
            }
        }
        snprintf(line + strlen(line), line_size - strlen(line), "[%s @ %p] ",
                 avc->item_name(ptr), ptr);
    }

    vsnprintf(line + strlen(line), line_size - strlen(line), fmt, vl);

    *print_prefix = strlen(line) && line[strlen(line) - 1] == '\n';
}


int set_log_level(int level){
    int i = glog_level;
    glog_level = level;
    return i;
}

static void sanitize(uint8_t *line){
    while(*line){
        if(*line < 0x08 || (*line > 0x0D && *line < 0x20))
            *line='?';
        line++;
    }
}

void av_log_callback(void* ptr, int level, const char* fmt, va_list vl)
{

    static int print_prefix = 1;
    static int count;
    static char prev[1024];
    char line[1024];

    switch(level){
        case AV_LOG_QUIET:        level = ANDROID_LOG_UNKNOWN; break;
        case AV_LOG_PANIC:        level = ANDROID_LOG_UNKNOWN; break;
        case AV_LOG_FATAL:        level = ANDROID_LOG_FATAL; break;
        case AV_LOG_ERROR:       level = ANDROID_LOG_ERROR; break;
        case AV_LOG_WARNING:   level = ANDROID_LOG_WARN; break;
        case AV_LOG_INFO:          level = ANDROID_LOG_INFO; break;
        case AV_LOG_VERBOSE:   level = ANDROID_LOG_VERBOSE; break;
        case AV_LOG_DEBUG:      level = ANDROID_LOG_DEBUG; break;     
        default: level = ANDROID_LOG_UNKNOWN; break;
    }


    if (level < glog_level)
        return;
    log_format_line(ptr, level, fmt, vl, line, sizeof(line), &print_prefix);

    if (print_prefix && (flags & AV_LOG_SKIP_REPEATED) && !strcmp(line, prev)){
        count++;
        return;
    }
    if (count > 0) {
        (void)__android_log_print(ANDROID_LOG_ERROR, "ffmpeg", "    Last message repeated %d times\n", count);
        count = 0;
    }
    strcpy(prev, line);
    sanitize(line);
    (void)__android_log_print(level, "ffmpeg", line);
}


EGL_Overlay *EGL_CreateGRBOverlay(int w, int h, uint32_t format)
{

    EGL_Overlay *overlay;
    uint8_t *pixels;
    

    /* Verify that we support the format */
    switch (format) {
        case PIX_FMT_RGB565:
        case PIX_FMT_RGB32:
        case PIX_FMT_RGB0:
        case PIX_FMT_RGB24:
                break;
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
        case PIX_FMT_RGB565:
            pixels=(uint8_t *)malloc(avpicture_get_size(PIX_FMT_RGB565, w,h));
            if ( pixels == NULL ) {
            	SDL_OutOfMemory();
            	return(NULL);
            }
	     avpicture_fill(&overlay->pict, pixels, PIX_FMT_RGB565, w, h);
    	break;
        case PIX_FMT_RGB32:
            pixels=(uint8_t *)malloc(avpicture_get_size(PIX_FMT_RGB32, w,h));
            if ( pixels == NULL ) {
            	SDL_OutOfMemory();
            	return(NULL);
            }
	     avpicture_fill(&overlay->pict, pixels, PIX_FMT_RGB32, w, h);
    	break;
        case PIX_FMT_RGB24:
            pixels=(uint8_t *)malloc(avpicture_get_size(PIX_FMT_RGB24, w,h));
            if ( pixels == NULL ) {
            	SDL_OutOfMemory();
            	return(NULL);
            }
	     avpicture_fill(&overlay->pict, pixels, PIX_FMT_RGB24, w, h);
    	break;
        default:
    	/* We should never get here (caught above) */
    	break;
    }

    /* We're all done.. */
    return(overlay);
    
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

void saveRGB24(const uint8_t * data,int linesize,  int width, int height) {
  FILE *pFile;
  char szFilename[32];
  int  y;

  
  // Open file
  if(gYUVi < 10){    
      sprintf(szFilename, "/tmp/frame%03d.ppm", gYUVi);
      pFile=fopen(szFilename, "wb");
      if(pFile==NULL)
        return;
      
      // Write header
      fprintf(pFile, "P6\n%d %d\n255\n", width, height);
      
      // Write pixel data
      for(y=0; y<height; y++)
        fwrite(data+y*linesize, 1, width*3, pFile);
      
      // Close file
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



int IsSpace(int c)
{
       return c==' ' || c=='\n' || c=='\t';
}

void LTrim(char *s)
{
       char *p;

        for (p=s;IsSpace(*p);p++);
        while (*p)
           *(s++) = *(p++);
        *s=0;
}

void RTrim(char *s)
{
        char *p;

        for (p=strchr(s,0);p>s && IsSpace(*(p-1));p--);
        *p=0;
}


void trim(char *s) 
{
    LTrim(s);
    RTrim(s);
}

int htoi(char c)
{ 
        if(c<='F' && c>='A')
                return c + 10 - 'A';
        if(c<='f' && c>='a')
                return c + 10 - 'a';
        if(c<='9' && c>='0')
                return c - '0';
        return 0;

}

void decode_filename(char* out,const char* in)
{
    int i;

    for (i=0; i<strlen(in); i++){
            if(in[i] == '%'){
                *out++ = htoi(in[i+1])*16+htoi(in[i+2]);
                i += 2;
            }else{
                *out++ = in[i];
            }   
    }
    *out = 0;
}
