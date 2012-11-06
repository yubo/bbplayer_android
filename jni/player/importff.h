/* San Angeles Observation OpenGL ES version example
 * Copyright 2004-2005 Jetro Lauha
 * All rights reserved.
 * Web: http://iki.fi/jetro/
 *
 * This source is free software; you can redistribute it and/or
 * modify it under the terms of EITHER:
 *   (1) The GNU Lesser General Public License as published by the Free
 *       Software Foundation; either version 2.1 of the License, or (at
 *       your option) any later version. The text of the GNU Lesser
 *       General Public License is included with this source in the
 *       file LICENSE-LGPL.txt.
 *   (2) The BSD-style license that is included with this source in
 *       the file LICENSE-BSD.txt.
 *
 * This source is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the files
 * LICENSE-LGPL.txt and LICENSE-BSD.txt for more details.
 *
 * $Id: importgl.h,v 1.4 2005/02/24 20:29:33 tonic Exp $
 * $Revision: 1.4 $
 */

/*
 * import FFmpeg libraries
 * 2012-4
 * yubo@yubo.org
 */


#ifndef IMPORTFF_H_INCLUDED
#define IMPORTFF_H_INCLUDED


#ifdef __cplusplus
extern "C" {
#endif


#include "libavutil/avstring.h"
#include "libavutil/colorspace.h"
#include "libavutil/mathematics.h"
#include "libavutil/pixdesc.h"
#include "libavutil/imgutils.h"
#include "libavutil/dict.h"
#include "libavutil/parseutils.h"
#include "libavutil/samplefmt.h"
#include "libavutil/avassert.h"
#include "libavformat/avformat.h"
#include "libavdevice/avdevice.h"
#include "libswscale/swscale.h"
#include "libavcodec/audioconvert.h"
#include "libavutil/opt.h"
#include "libavcodec/avfft.h"
#include "libswresample/swresample.h"


/* Dynamically fetches pointers to the ffmpeg functions.
 * Should be called once on application initialization.
 * Returns non-zero on success and 0 on failure.
 */
extern int importFFInit(char *path);

/* Frees the handle to ffmpeg functions library.
 */
extern void importFFDeinit();


#ifndef DISABLE_IMPORTFF


#ifndef IMPORTFF_API
#define IMPORTFF_API extern
#endif
#ifndef IMPORTFF_FNPTRINIT
#define IMPORTFF_FNPTRINIT
#endif

#define FNDEF(retType, funcName, args) IMPORTFF_API retType (*funcPtr_##funcName) args IMPORTFF_FNPTRINIT


FNDEF(AVBitStreamFilter *, av_bitstream_filter_next, (AVBitStreamFilter *f));
FNDEF(char, av_get_picture_type_char, (enum AVPictureType pict_type));
FNDEF(int, av_opt_set, (void *obj, const char *name, const char *val, int search_flags));
FNDEF(double , av_strtod, (const char *numstr, char **tail));
FNDEF(AVFrame *, avcodec_alloc_frame, (void));
FNDEF(int, avcodec_close, (AVCodecContext *avctx));
FNDEF(const char *, avcodec_configuration, (void));
FNDEF(int, avcodec_decode_audio4, (AVCodecContext *avctx, AVFrame *frame, int *got_frame_ptr, AVPacket *avpkt));
FNDEF(int, avcodec_decode_video2, (AVCodecContext *avctx, AVFrame *picture, int *got_picture_ptr, const AVPacket *avpkt));
FNDEF(AVCodec *, avcodec_find_decoder, (enum CodecID id));
FNDEF(AVCodec *, avcodec_find_decoder_by_name, (const char *name));
FNDEF(void, avcodec_flush_buffers, (AVCodecContext *avctx));
FNDEF(const AVClass *, avcodec_get_class, (void));
FNDEF(const AVClass *, avcodec_get_frame_class, (void));
FNDEF(void, avcodec_get_frame_defaults, (AVFrame *pic));
FNDEF(AVCodec *, av_codec_next, (AVCodec *c));
FNDEF(int, avcodec_open2, (AVCodecContext *avctx, AVCodec *codec, AVDictionary **options));
FNDEF(void, avcodec_register_all, (void));
FNDEF(unsigned, avcodec_version, (void));
FNDEF(void, av_dict_free, (AVDictionary **m));
FNDEF(AVDictionaryEntry *, av_dict_get, (AVDictionary *m, const char *key, const AVDictionaryEntry *prev, int flags));
FNDEF(int, av_dict_set, (AVDictionary **pm, const char *key, const char *value, int flags));
FNDEF(void, av_dump_format, (AVFormatContext *ic, int index, const char *url, int is_output));
FNDEF(int, av_dup_packet, (AVPacket *pkt));
FNDEF(int, av_find_best_stream, (AVFormatContext *ic, enum AVMediaType type, int wanted_stream_nb, int related_stream, AVCodec **decoder_ret, int flags));
FNDEF(AVInputFormat *, av_find_input_format, (const char *short_name));
FNDEF(AVFormatContext *, avformat_alloc_context, (void));
FNDEF(void, avformat_close_input, (AVFormatContext **s));
FNDEF(const char *, avformat_configuration, (void));
FNDEF(int, avformat_find_stream_info, (AVFormatContext *ic, AVDictionary **options));
FNDEF(const AVClass *, avformat_get_class, (void));
FNDEF(int, avformat_network_deinit, (void));
FNDEF(int, avformat_network_init, (void));
FNDEF(int, avformat_open_input, (AVFormatContext **ps, const char *filename, AVInputFormat *fmt, AVDictionary **options));
FNDEF(int, avformat_seek_file, (AVFormatContext *s, int stream_index, int64_t min_ts, int64_t ts, int64_t max_ts, int flags));
FNDEF(unsigned, avformat_version, (void));
FNDEF(void, av_free, (void *ptr));
FNDEF(void, av_freep, (void *ptr));
FNDEF(void, av_free_packet, (AVPacket *pkt));
FNDEF(int, av_get_bits_per_pixel, (const AVPixFmtDescriptor *pixdesc));
FNDEF(int, av_get_bytes_per_sample, (enum AVSampleFormat sample_fmt));
FNDEF(int, av_get_channel_layout_nb_channels, (uint64_t channel_layout));
FNDEF(int64_t, av_get_default_channel_layout, (int nb_channels));
FNDEF(int64_t, av_get_int, (void *obj, const char *name, const AVOption **o_out));
FNDEF(const char *, av_get_sample_fmt_name, (enum AVSampleFormat sample_fmt));
FNDEF(char *, av_get_sample_fmt_string, (char *buf, int buf_size, enum AVSampleFormat sample_fmt));
FNDEF(int64_t, av_gettime, (void));
FNDEF(AVInputFormat *, av_iformat_next, (AVInputFormat *f));
FNDEF(void, av_init_packet, (AVPacket *pkt));
FNDEF(int64_t, avio_seek, (AVIOContext *s, int64_t offset, int whence));
FNDEF(int64_t, avio_size, (AVIOContext *s));
FNDEF(int, av_lockmgr_register, (int (*cb)(void **mutex, enum AVLockOp op)));
FNDEF(void, av_log, (void *avcl, int level, const char *fmt, ...) );
FNDEF(void, av_log_default_callback, (void* ptr, int level, const char* fmt, va_list vl));
FNDEF(void, av_log_format_line, (void *ptr, int level, const char *fmt, va_list vl, char *line, int line_size, int *print_prefix));
FNDEF(int, av_log_get_level, (void));
FNDEF(void, av_log_set_callback, (void (*)(void*, int, const char*, va_list)));
FNDEF(void, av_log_set_level, (int));
FNDEF(void *, av_malloc, (size_t size) );
FNDEF(void *, av_mallocz, (size_t size) );
FNDEF(void, av_max_alloc, (size_t max));
FNDEF(AVOutputFormat *, av_oformat_next, (AVOutputFormat *f));
FNDEF(const AVClass *, av_opt_child_class_next, (const AVClass *parent, const AVClass *prev));
FNDEF(const AVOption *, av_opt_find, (void *obj, const char *name, const char *unit, int opt_flags, int search_flags));
FNDEF(void *, av_opt_ptr, (const AVClass *avclass, void *obj, const char *name));
FNDEF(int, av_opt_show2, (void *obj, void *av_log_obj, int req_flags, int rej_flags));
FNDEF(int, av_parse_time, (int64_t *timeval, const char *timestr, int duration));
FNDEF(int, avpicture_fill, (AVPicture *picture, uint8_t *ptr, enum PixelFormat pix_fmt, int width, int height));
FNDEF(int, avpicture_get_size, (enum PixelFormat pix_fmt, int width, int height));
FNDEF(URLProtocol *, av_protocol_next, (URLProtocol *p));
FNDEF(void, av_rdft_end, (RDFTContext *s));
FNDEF(int, av_read_frame, (AVFormatContext *s, AVPacket *pkt));
FNDEF(int, av_read_pause, (AVFormatContext *s));
FNDEF(int, av_read_play, (AVFormatContext *s));
FNDEF(void *, av_realloc, (void *ptr, size_t size) );
FNDEF(void, av_register_all, (void));
FNDEF(int, av_samples_get_buffer_size, (int *linesize, int nb_channels, int nb_samples, enum AVSampleFormat sample_fmt, int align));
FNDEF(char *, av_strdup, (const char *s));
FNDEF(int, av_strerror, (int errnum, char *errbuf, size_t errbuf_size));
FNDEF(size_t, av_strlcat, (char *dst, const char *src, size_t size));
FNDEF(size_t, av_strlcpy, (char *dst, const char *src, size_t size));
FNDEF(const char *, avutil_configuration, (void));
FNDEF(unsigned, avutil_version, (void));
FNDEF(struct SwrContext *, swr_alloc_set_opts, (struct SwrContext *s, int64_t out_ch_layout, enum AVSampleFormat out_sample_fmt, int out_sample_rate, int64_t in_ch_layout, enum AVSampleFormat in_sample_fmt, int in_sample_rate, int log_offset, void *log_ctx));
FNDEF(int, swr_convert, (struct SwrContext *s, uint8_t *out[16], int out_count, const uint8_t *in [16], int in_count));
FNDEF(const char *, swresample_configuration, (void));
FNDEF(unsigned, swresample_version, (void));
FNDEF(void, swr_free, (struct SwrContext **s));
FNDEF(int, swr_init, (struct SwrContext *s));
FNDEF(int, swr_set_compensation, (struct SwrContext *s, int sample_delta, int compensation_distance));
FNDEF(const char *, swscale_configuration, (void));
FNDEF(unsigned, swscale_version, (void));
FNDEF(void, sws_freeContext, (struct SwsContext *swsContext));
FNDEF(struct SwsContext *, sws_getCachedContext, (struct SwsContext *context, int srcW, int srcH, enum PixelFormat srcFormat, int dstW, int dstH, enum PixelFormat dstFormat, int flags, SwsFilter *srcFilter, SwsFilter *dstFilter, const double *param));
FNDEF(const AVClass *, sws_get_class, (void));
FNDEF(struct SwsContext *, sws_getContext, (int srcW, int srcH, enum PixelFormat srcFormat, int dstW, int dstH, enum PixelFormat dstFormat, int flags, SwsFilter *srcFilter, SwsFilter *dstFilter, const double *param));
FNDEF(int, sws_isSupportedInput, (enum PixelFormat pix_fmt));
FNDEF(int, sws_isSupportedOutput, (enum PixelFormat pix_fmt));
FNDEF(int, sws_scale, (struct SwsContext *c, const uint8_t *const srcSlice[], const int srcStride[], int srcSliceY, int srcSliceH, uint8_t *const dst[], const int dstStride[]));
FNDEF(int, url_feof, (AVIOContext *s));


#undef FN
#define FNPTR(name) funcPtr_##name

#ifndef IMPORTFF_NO_FNPTR_DEFS

// Redirect egl* and gl* function calls to funcPtr_egl* and funcPtr_gl*.


#define av_bitstream_filter_next FNPTR(av_bitstream_filter_next)
#define av_get_picture_type_char FNPTR(av_get_picture_type_char)
#define av_opt_set FNPTR(av_opt_set)
#define av_strtod FNPTR(av_strtod)
#define avcodec_alloc_frame FNPTR(avcodec_alloc_frame)
#define avcodec_close FNPTR(avcodec_close)
#define avcodec_configuration FNPTR(avcodec_configuration)
#define avcodec_decode_audio4 FNPTR(avcodec_decode_audio4)
#define avcodec_decode_video2 FNPTR(avcodec_decode_video2)
#define avcodec_find_decoder FNPTR(avcodec_find_decoder)
#define avcodec_find_decoder_by_name FNPTR(avcodec_find_decoder_by_name)
#define avcodec_flush_buffers FNPTR(avcodec_flush_buffers)
#define avcodec_get_class FNPTR(avcodec_get_class)
#define avcodec_get_frame_class FNPTR(avcodec_get_frame_class)
#define avcodec_get_frame_defaults FNPTR(avcodec_get_frame_defaults)
#define av_codec_next FNPTR(av_codec_next)
#define avcodec_open2 FNPTR(avcodec_open2)
#define avcodec_register_all FNPTR(avcodec_register_all)
#define avcodec_version FNPTR(avcodec_version)
#define av_dict_free FNPTR(av_dict_free)
#define av_dict_get FNPTR(av_dict_get)
#define av_dict_set FNPTR(av_dict_set)
#define av_dump_format FNPTR(av_dump_format)
#define av_dup_packet FNPTR(av_dup_packet)
#define av_find_best_stream FNPTR(av_find_best_stream)
#define av_find_input_format FNPTR(av_find_input_format)
#define avformat_alloc_context FNPTR(avformat_alloc_context)
#define avformat_close_input FNPTR(avformat_close_input)
#define avformat_configuration FNPTR(avformat_configuration)
#define avformat_find_stream_info FNPTR(avformat_find_stream_info)
#define avformat_get_class FNPTR(avformat_get_class)
#define avformat_network_deinit FNPTR(avformat_network_deinit)
#define avformat_network_init FNPTR(avformat_network_init)
#define avformat_open_input FNPTR(avformat_open_input)
#define avformat_seek_file FNPTR(avformat_seek_file)
#define avformat_version FNPTR(avformat_version)
#define av_free FNPTR(av_free)
#define av_freep FNPTR(av_freep)
#define av_free_packet FNPTR(av_free_packet)
#define av_get_bits_per_pixel FNPTR(av_get_bits_per_pixel)
#define av_get_bytes_per_sample FNPTR(av_get_bytes_per_sample)
#define av_get_channel_layout_nb_channels FNPTR(av_get_channel_layout_nb_channels)
#define av_get_default_channel_layout FNPTR(av_get_default_channel_layout)
#define av_get_int FNPTR(av_get_int)
#define av_get_sample_fmt_name FNPTR(av_get_sample_fmt_name)
#define av_get_sample_fmt_string FNPTR(av_get_sample_fmt_string)
#define av_gettime FNPTR(av_gettime)
#define av_iformat_next FNPTR(av_iformat_next)
#define av_init_packet FNPTR(av_init_packet)
#define avio_seek FNPTR(avio_seek)
#define avio_size FNPTR(avio_size)
#define av_lockmgr_register FNPTR(av_lockmgr_register)
#define av_log FNPTR(av_log)
#define av_log_default_callback FNPTR(av_log_default_callback)
#define av_log_format_line FNPTR(av_log_format_line)
#define av_log_get_level FNPTR(av_log_get_level)
#define av_log_set_callback FNPTR(av_log_set_callback)
#define av_log_set_level FNPTR(av_log_set_level)
#define av_malloc FNPTR(av_malloc)
#define av_mallocz FNPTR(av_mallocz)
#define av_max_alloc FNPTR(av_max_alloc)
#define av_oformat_next FNPTR(av_oformat_next)
#define av_opt_child_class_next FNPTR(av_opt_child_class_next)
#define av_opt_find FNPTR(av_opt_find)
#define av_opt_ptr FNPTR(av_opt_ptr)
#define av_opt_show2 FNPTR(av_opt_show2)
#define av_parse_time FNPTR(av_parse_time)
#define avpicture_fill FNPTR(avpicture_fill)
#define avpicture_get_size FNPTR(avpicture_get_size)
#define av_protocol_next FNPTR(av_protocol_next)
#define av_rdft_end FNPTR(av_rdft_end)
#define av_read_frame FNPTR(av_read_frame)
#define av_read_pause FNPTR(av_read_pause)
#define av_read_play FNPTR(av_read_play)
#define av_realloc FNPTR(av_realloc)
#define av_register_all FNPTR(av_register_all)
#define av_samples_get_buffer_size FNPTR(av_samples_get_buffer_size)
#define av_strdup FNPTR(av_strdup)
#define av_strerror FNPTR(av_strerror)
#define av_strlcat FNPTR(av_strlcat)
#define av_strlcpy FNPTR(av_strlcpy)
#define avutil_configuration FNPTR(avutil_configuration)
#define avutil_version FNPTR(avutil_version)
#define swr_alloc_set_opts FNPTR(swr_alloc_set_opts)
#define swr_convert FNPTR(swr_convert)
#define swresample_configuration FNPTR(swresample_configuration)
#define swresample_version FNPTR(swresample_version)
#define swr_free FNPTR(swr_free)
#define swr_init FNPTR(swr_init)
#define swr_set_compensation FNPTR(swr_set_compensation)
#define swscale_configuration FNPTR(swscale_configuration)
#define swscale_version FNPTR(swscale_version)
#define sws_freeContext FNPTR(sws_freeContext)
#define sws_getCachedContext FNPTR(sws_getCachedContext)
#define sws_get_class FNPTR(sws_get_class)
#define sws_getContext FNPTR(sws_getContext)
#define sws_isSupportedInput FNPTR(sws_isSupportedInput)
#define sws_isSupportedOutput FNPTR(sws_isSupportedOutput)
#define sws_scale FNPTR(sws_scale)
#define url_feof FNPTR(url_feof)


#endif // !IMPORTGL_NO_FNPTR_DEFS


#endif // !DISABLE_IMPORTGL


#ifdef __cplusplus
}
#endif


#endif // !IMPORTGL_H_INCLUDED
