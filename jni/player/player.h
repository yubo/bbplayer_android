/*
 * simple android media player based on the FFmpeg libraries
 * 2012-3
 * yubo@yubo.org
 */

#ifndef _player_h_
#define _player_h_

// for system
#include <jni.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include <string.h>
#include <android/log.h>
#include <android/sensor.h>

// for ffmpeg
#include <config.h>
#include <inttypes.h>
#include <math.h>
#include <limits.h>
#include <signal.h>
//	#include "libavutil/avstring.h"
//	#include "libavutil/colorspace.h"
//	#include "libavutil/mathematics.h"
//	#include "libavutil/pixdesc.h"
//	#include "libavutil/imgutils.h"
//	#include "libavutil/dict.h"
//	#include "libavutil/parseutils.h"
//	#include "libavutil/samplefmt.h"
//	#include "libavutil/avassert.h"
//	#include "libavformat/avformat.h"
//	#include "libavdevice/avdevice.h"
//	#include "libswscale/swscale.h"
//	#include "libavcodec/audioconvert.h"
//	#include "libavutil/opt.h"
//	#include "libavcodec/avfft.h"
//	#include "libswresample/swresample.h"
#include  "importff.h"


// for SDL, will fixed 
#include <SDL.h>
#include <SDL_thread.h>

// for video 
#include <EGL/egl.h>
#include <GLES/gl.h>
#include <GLES/glext.h>

// for native audio
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

// for native asset manager
#include <sys/types.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

// for player
#include "cmdutils.h"
#include "utils.h"







#define MAX_QUEUE_SIZE (24 * 1024 * 1024)
#define MIN_AUDIOQ_SIZE (20 * 16 * 1024)
#define MIN_FRAMES 5

/* SDL audio buffer size, in samples. Should be small to have precise
   A/V sync as SDL does not have hardware buffer fullness info. */
#define SDL_AUDIO_BUFFER_SIZE 1024

/* no AV sync correction is done if below the AV sync threshold */
#define AV_SYNC_THRESHOLD 0.01
/* no AV correction is done if too big error */
#define AV_NOSYNC_THRESHOLD 10.0

/* maximum audio speed change to get correct sync */
#define SAMPLE_CORRECTION_PERCENT_MAX 10

/* we use about AUDIO_DIFF_AVG_NB A-V differences to make the average */
#define AUDIO_DIFF_AVG_NB   20

/* NOTE: the size must be big enough to compensate the hardware audio buffersize size */
#define SAMPLE_ARRAY_SIZE (2 * 65536)

static int sws_flags = SWS_BICUBIC;



#define VIDEO_PICTURE_QUEUE_SIZE 4
#define SUBPICTURE_QUEUE_SIZE 4

#define FF_ALLOC_EVENT   (SDL_USEREVENT)
#define FF_REFRESH_EVENT (SDL_USEREVENT + 1)
#define FF_QUIT_EVENT    (SDL_USEREVENT + 2)


enum {
    AV_SYNC_AUDIO_MASTER, /* default choice */
    AV_SYNC_VIDEO_MASTER,
    AV_SYNC_EXTERNAL_CLOCK, /* synchronize to an external clock */
};


enum{
    NOTIFY_TOUCH_UP=1,
    NOTIFY_TOUCH_DOWN,
    NOTIFY_TOUCH_LEFT,
    NOTIFY_TOUCH_RIGHT,
};

enum {
    SYNC_PLAYER_CLOCK=1,
};

/**
 * Our saved state data.
 */
struct saved_state {
    float angle;
    int32_t x;
    int32_t y;
};



/** This structure should be treated as read-only, except for 'pixels',
 *  which, if not NULL, contains the raw pixel data for the surface.
 */
typedef struct EGL_Surface {
	Uint32 flags;				/**< Read-only */
	SDL_PixelFormat *format;		/**< Read-only */
	int w, h;				/**< Read-only */
	Uint16 pitch;				/**< Read-only */
	void *pixels;				/**< Read-write */
	int offset;				/**< Private */

	/** Hardware-specific surface info */
	struct private_hwdata *hwdata;

	/** clipping information */
	SDL_Rect clip_rect;			/**< Read-only */
	Uint32 unused1;				/**< for binary compatibility */

	/** Allow recursive locks */
	Uint32 locked;				/**< Private */

	/** info for fast blit mapping to other surfaces */
	struct SDL_BlitMap *map;		/**< Private */

	/** format version, bumped at every change to invalidate blit maps */
	unsigned int format_version;		/**< Private */

	/** Reference count -- used when freeing surface */
	int refcount;				/**< Read-mostly */
} EGL_Surface;


typedef struct PacketQueue {
    AVPacketList *first_pkt, *last_pkt;
    int nb_packets;
    int size;
    int abort_request;
    SDL_mutex *mutex;
    SDL_cond *cond;
} PacketQueue;


/** The EGL  video overlay */
typedef struct EGL_Overlay {
	AVPicture pict;				/**< Read-write */
	uint32_t format;				/**< Read-only */
	int w, h;				/**< Read-only */
	int planes;				/**< Read-only */
} EGL_Overlay;


typedef struct VideoPicture {
    double pts;                                  ///< presentation time stamp for this picture
    double duration;                             ///< expected duration of the frame
    int64_t pos;                                 ///< byte position in file
    int skip;
    EGL_Overlay *bmp;
    int width, height; /* source height & width */
    int allocated;
    int reallocate;
    enum PixelFormat pix_fmt;
} VideoPicture;

typedef struct SubPicture {
    double pts; /* presentation time stamp for this picture */
    AVSubtitle sub;
} SubPicture;


typedef struct VideoState {
    SDL_Thread *read_tid;
    SDL_Thread *loop_tid;
    SDL_Thread *video_tid;
    SDL_Thread *audio_tid;
//    SDL_Thread *refresh_tid;
    AVInputFormat *iformat;
    int no_background;
    int abort_request;
    int paused;
    int last_paused;
    int seek_req;
    int seek_flags;
    int64_t seek_pos;
    int64_t seek_rel;
    int read_pause_return;
    AVFormatContext *ic;

    int audio_stream;

    int av_sync_type;
    double external_clock; /* external clock base */
    int64_t external_clock_time;

    double audio_clock;
    double audio_diff_cum; /* used for AV difference average computation */
    double audio_diff_avg_coef;
    double audio_diff_threshold;
    int audio_diff_avg_count;
    AVStream *audio_st;
    PacketQueue audioq;
    int audio_hw_buf_size;

    uint8_t silence_buf[SDL_AUDIO_BUFFER_SIZE];
    uint8_t *audio_buf;
    uint8_t *audio_buf1;
    unsigned int audio_buf_size; /* in bytes */
    int audio_buf_index; /* in bytes */
    int audio_write_buf_size;
    AVPacket audio_pkt_temp;
    AVPacket audio_pkt;
    enum AVSampleFormat audio_src_fmt;
    enum AVSampleFormat audio_tgt_fmt;
    int audio_src_channels;
    int audio_tgt_channels;
    int64_t audio_src_channel_layout;
    int64_t audio_tgt_channel_layout;
    int audio_src_freq;
    int audio_tgt_freq;
    int audio_callback_len;
    struct SwrContext *swr_ctx;
    double audio_current_pts;
    double audio_current_pts_drift;
    int frame_drops_early;
    int frame_drops_late;
    AVFrame *frame;

    enum ShowMode {
        SHOW_MODE_NONE = -1, SHOW_MODE_VIDEO = 0, SHOW_MODE_WAVES, SHOW_MODE_RDFT, SHOW_MODE_NB
    } show_mode;
    int16_t sample_array[SAMPLE_ARRAY_SIZE];
    int sample_array_index;
    int last_i_start;
    RDFTContext *rdft;
    int rdft_bits;
    FFTSample *rdft_data;
    int xpos;

    SDL_Thread *subtitle_tid;
    int subtitle_stream;
    int subtitle_stream_changed;
    AVStream *subtitle_st;
    PacketQueue subtitleq;
    SubPicture subpq[SUBPICTURE_QUEUE_SIZE];
    int subpq_size, subpq_rindex, subpq_windex;
    SDL_mutex *subpq_mutex;
    SDL_cond *subpq_cond;

    double frame_timer;
    double frame_last_pts;
    double frame_last_duration;
    double frame_last_dropped_pts;
    double frame_last_returned_time;
    double frame_last_filter_delay;
    int64_t frame_last_dropped_pos;
    double video_clock;                          ///< pts of last decoded frame / predicted pts of next decoded frame
    int video_stream;
    AVStream *video_st;
    PacketQueue videoq;
    double video_current_pts;                    ///< current displayed pts (different from video_clock if frame fifos are used)
    double video_current_pts_drift;              ///< video_current_pts - time (av_gettime) at which we updated video_current_pts - used to have running video pts
    int64_t video_current_pos;                   ///< current displayed file pos
    VideoPicture pictq[VIDEO_PICTURE_QUEUE_SIZE];
    int pictq_size, pictq_rindex, pictq_windex;
    SDL_mutex *pictq_mutex;
    SDL_cond *pictq_cond;
//	    SDL_mutex *surface_mutex;
//	    SDL_cond *surface_cond;
//	    SDL_cond *surface2_cond;
    struct SwsContext *img_convert_ctx;
    char filename[1024];
    int width, height, xleft, ytop;
    int step;
    int refresh;
    int skip_pkt;
    int sensor;
    JNIEnv  *np_env;
    jobject np_obj;
    jclass np_cls;
    jmethodID np_NativePlayerMsg_mid;
    
//    SDL_Rect rect;
//    EGL_Overlay *gl_bmp;
    DECLARE_ALIGNED(16,uint8_t,audio_buf2)[AVCODEC_MAX_AUDIO_FRAME_SIZE * 4];
} VideoState;





static void engine_draw_frame();
static void engine_term_display();
static int  lockmgr(void **mtx, enum AVLockOp op);

EGL_Overlay *EGL_CreateGRBOverlay(int w, int h, Uint32 format);
EGL_Surface *EGL_SetVideoMode (int width, int height, int bpp, Uint32 flags, VideoState *is);
VideoState  *nativeInit();

void check_gl_error(const char* op);
void EGL_FreeGRBOverlay(EGL_Overlay *overlay);
void check_gl_error(const char* op);
void egl_audio_callback(SLAndroidSimpleBufferQueueItf bq, void *context);
void audio_term();

int EGL_DisplayRGBOverlay(VideoState *is,  EGL_Overlay *overlay, SDL_Rect *dstrect);
int native_main();
int nativeResize(int w, int h, VideoState *is);
int nativeStart( int w, int h,VideoState *is);
int nativeRender(VideoState *is);
int nativePause(VideoState *is);
int nativeDone(VideoState *is);
int nativeSeek(int seek,VideoState *is);
int nativeGetClock(VideoState *is);
int nativeSetObj(jobject thiz, VideoState *is);
int RemotePlayerMsg(int msgid, VideoState *is);

#endif

