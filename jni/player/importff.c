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
 * $Id: importgl.c,v 1.4 2005/02/08 18:42:55 tonic Exp $
 * $Revision: 1.4 $
 */

/*
 * import FFmpeg libraries
 * 2012-4
 * yubo@yubo.org
 */


#ifndef DISABLE_IMPORTFF


#include <stdlib.h>
#include <dlfcn.h>
static void *sFFSO = NULL;

#endif /* DISABLE_IMPORTFF */

#define IMPORTFF_NO_FNPTR_DEFS
#define IMPORTFF_API
#define IMPORTFF_FNPTRINIT = NULL
#include "importff.h"



int importFFInit(char * path)
{
    int result = 0;

#ifndef DISABLE_IMPORTFF

#undef IMPORT_FUNC
sFFSO = dlopen(path, RTLD_NOW);
if (sFFSO == NULL)
    return 1;   // Cannot find ffmpeg  SO.

#define IMPORT_FUNC(funcName) do { \
        void *procAddress = (void *)dlsym(sFFSO, #funcName); \
        if (procAddress == NULL) result = 1; \
        *((void **)&FNPTR(funcName)) = procAddress; } while (0)


    IMPORT_FUNC(av_bitstream_filter_next);
    IMPORT_FUNC(av_get_picture_type_char);
    IMPORT_FUNC(av_opt_set);
    IMPORT_FUNC(av_strtod);
    IMPORT_FUNC(avcodec_alloc_frame);
    IMPORT_FUNC(avcodec_close);
    IMPORT_FUNC(avcodec_configuration);
    IMPORT_FUNC(avcodec_decode_audio4);
    IMPORT_FUNC(avcodec_decode_video2);
    IMPORT_FUNC(avcodec_find_decoder);
    IMPORT_FUNC(avcodec_find_decoder_by_name);
    IMPORT_FUNC(avcodec_flush_buffers);
    IMPORT_FUNC(avcodec_get_class);
    IMPORT_FUNC(avcodec_get_frame_class);
    IMPORT_FUNC(avcodec_get_frame_defaults);
    IMPORT_FUNC(av_codec_next);
    IMPORT_FUNC(avcodec_open2);
    IMPORT_FUNC(avcodec_register_all);
    IMPORT_FUNC(avcodec_version);
    IMPORT_FUNC(av_dict_free);
    IMPORT_FUNC(av_dict_get);
    IMPORT_FUNC(av_dict_set);
    IMPORT_FUNC(av_dump_format);
    IMPORT_FUNC(av_dup_packet);
    IMPORT_FUNC(av_find_best_stream);
    IMPORT_FUNC(av_find_input_format);
    IMPORT_FUNC(avformat_alloc_context);
    IMPORT_FUNC(avformat_close_input);
    IMPORT_FUNC(avformat_configuration);
    IMPORT_FUNC(avformat_find_stream_info);
    IMPORT_FUNC(avformat_get_class);
    IMPORT_FUNC(avformat_network_deinit);
    IMPORT_FUNC(avformat_network_init);
    IMPORT_FUNC(avformat_open_input);
    IMPORT_FUNC(avformat_seek_file);
    IMPORT_FUNC(avformat_version);
    IMPORT_FUNC(av_free);
    IMPORT_FUNC(av_freep);
    IMPORT_FUNC(av_free_packet);
    IMPORT_FUNC(av_get_bits_per_pixel);
    IMPORT_FUNC(av_get_bytes_per_sample);
    IMPORT_FUNC(av_get_channel_layout_nb_channels);
    IMPORT_FUNC(av_get_default_channel_layout);
    IMPORT_FUNC(av_get_int);
    IMPORT_FUNC(av_get_sample_fmt_name);
    IMPORT_FUNC(av_get_sample_fmt_string);
    IMPORT_FUNC(av_gettime);
    IMPORT_FUNC(av_iformat_next);
    IMPORT_FUNC(av_init_packet);
    IMPORT_FUNC(avio_seek);
    IMPORT_FUNC(avio_size);
    IMPORT_FUNC(av_lockmgr_register);
    IMPORT_FUNC(av_log);
    IMPORT_FUNC(av_log_default_callback);
    IMPORT_FUNC(av_log_format_line);
    IMPORT_FUNC(av_log_get_level);
    IMPORT_FUNC(av_log_set_callback);
    IMPORT_FUNC(av_log_set_level);
    IMPORT_FUNC(av_malloc);
    IMPORT_FUNC(av_mallocz);
    IMPORT_FUNC(av_max_alloc);
    IMPORT_FUNC(av_oformat_next);
    IMPORT_FUNC(av_opt_child_class_next);
    IMPORT_FUNC(av_opt_find);
    IMPORT_FUNC(av_opt_ptr);
    IMPORT_FUNC(av_opt_show2);
    IMPORT_FUNC(av_parse_time);
    IMPORT_FUNC(avpicture_fill);
    IMPORT_FUNC(avpicture_get_size);
    IMPORT_FUNC(av_protocol_next);
    IMPORT_FUNC(av_rdft_end);
    IMPORT_FUNC(av_read_frame);
    IMPORT_FUNC(av_read_pause);
    IMPORT_FUNC(av_read_play);
    IMPORT_FUNC(av_realloc);
    IMPORT_FUNC(av_register_all);
    IMPORT_FUNC(av_samples_get_buffer_size);
    IMPORT_FUNC(av_strdup);
    IMPORT_FUNC(av_strerror);
    IMPORT_FUNC(av_strlcat);
    IMPORT_FUNC(av_strlcpy);
    IMPORT_FUNC(avutil_configuration);
    IMPORT_FUNC(avutil_version);
    IMPORT_FUNC(swr_alloc_set_opts);
    IMPORT_FUNC(swr_convert);
    IMPORT_FUNC(swresample_configuration);
    IMPORT_FUNC(swresample_version);
    IMPORT_FUNC(swr_free);
    IMPORT_FUNC(swr_init);
    IMPORT_FUNC(swr_set_compensation);
    IMPORT_FUNC(swscale_configuration);
    IMPORT_FUNC(swscale_version);
    IMPORT_FUNC(sws_freeContext);
    IMPORT_FUNC(sws_getCachedContext);
    IMPORT_FUNC(sws_get_class);
    IMPORT_FUNC(sws_getContext);
    IMPORT_FUNC(sws_isSupportedInput);
    IMPORT_FUNC(sws_isSupportedOutput);
    IMPORT_FUNC(sws_scale);
    IMPORT_FUNC(url_feof);

#endif /* DISABLE_IMPORTFF */
    return result;
}


void importFFDeinit()
{
#ifndef DISABLE_IMPORTFF
dlclose(sFFSO);
#endif /* DISABLE_IMPORTFF */
}
