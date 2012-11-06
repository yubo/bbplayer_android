#define LOG_TAG "NativePlayer:video"
#include "player.h"
/*
 * Set the requested video mode, allocating a shadow buffer if necessary.
 */
SDL_Surface * SDL_SetVideoMode (int width, int height, int bpp, Uint32 flags)
{
	SDL_VideoDevice *video, *this;
	SDL_Surface *prev_mode, *mode;
	int video_w;
	int video_h;
	int video_bpp;
	int is_opengl;
	SDL_GrabMode saved_grab;

	#ifdef WIN32
		sysevents_mouse_pressed = 0;
	#endif

	/* Start up the video driver, if necessary..
	   WARNING: This is the only function protected this way!
	 */
	if ( ! current_video ) {
		if ( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_NOPARACHUTE) < 0 ) {
			return(NULL);
		}
	}
	this = video = current_video;

	/* Default to the current width and height */
	if ( width == 0 ) {
		width = video->info.current_w;
	}
	if ( height == 0 ) {
		height = video->info.current_h;
	}
	/* Default to the current video bpp */
	if ( bpp == 0 ) {
		flags |= SDL_ANYFORMAT;
		bpp = SDL_VideoSurface->format->BitsPerPixel;
	}

	/* Get a good video mode, the closest one possible */
	video_w = width;
	video_h = height;
	video_bpp = bpp;
	if ( ! SDL_GetVideoMode(&video_w, &video_h, &video_bpp, flags) ) {
		return(NULL);
	}

	/* Check the requested flags */
	/* There's no palette in > 8 bits-per-pixel mode */
	if ( video_bpp > 8 ) {
		flags &= ~SDL_HWPALETTE;
	}
#if 0
	if ( (flags&SDL_FULLSCREEN) != SDL_FULLSCREEN ) {
		/* There's no windowed double-buffering */
		flags &= ~SDL_DOUBLEBUF;
	}
#endif
	if ( (flags&SDL_DOUBLEBUF) == SDL_DOUBLEBUF ) {
		/* Use hardware surfaces when double-buffering */
		flags |= SDL_HWSURFACE;
	}

	is_opengl = ( ( flags & SDL_OPENGL ) == SDL_OPENGL );
	if ( is_opengl ) {
		/* These flags are for 2D video modes only */
		flags &= ~(SDL_HWSURFACE|SDL_DOUBLEBUF);
	}

	/* Reset the keyboard here so event callbacks can run */
	SDL_ResetKeyboard();
	SDL_ResetMouse();
	SDL_SetMouseRange(width, height);
	SDL_cursorstate &= ~CURSOR_USINGSW;

	/* Clean up any previous video mode */
	if ( SDL_PublicSurface != NULL ) {
		SDL_PublicSurface = NULL;
	}
	if ( SDL_ShadowSurface != NULL ) {
		SDL_Surface *ready_to_go;
		ready_to_go = SDL_ShadowSurface;
		SDL_ShadowSurface = NULL;
		SDL_FreeSurface(ready_to_go);
	}
	if ( video->physpal ) {
		SDL_free(video->physpal->colors);
		SDL_free(video->physpal);
		video->physpal = NULL;
	}
	if( video->gammacols) {
		SDL_free(video->gammacols);
		video->gammacols = NULL;
	}

	/* Save the previous grab state and turn off grab for mode switch */
	saved_grab = SDL_WM_GrabInputOff();

	/* Try to set the video mode, along with offset and clipping */
	prev_mode = SDL_VideoSurface;
	SDL_LockCursor();
	SDL_VideoSurface = NULL;	/* In case it's freed by driver */
	mode = video->SetVideoMode(this, prev_mode,video_w,video_h,video_bpp,flags);
	if ( mode ) { /* Prevent resize events from mode change */
          /* But not on OS/2 */
#ifndef __OS2__
	    SDL_PrivateResize(mode->w, mode->h);
#endif

	    /* Sam - If we asked for OpenGL mode, and didn't get it, fail */
	    if ( is_opengl && !(mode->flags & SDL_OPENGL) ) {
		mode = NULL;
		SDL_SetError("OpenGL not available");
	    }
	}
	/*
	 * rcg11292000
	 * If you try to set an SDL_OPENGL surface, and fail to find a
	 * matching  visual, then the next call to SDL_SetVideoMode()
	 * will segfault, since  we no longer point to a dummy surface,
	 * but rather NULL.
	 * Sam 11/29/00
	 * WARNING, we need to make sure that the previous mode hasn't
	 * already been freed by the video driver.  What do we do in
	 * that case?  Should we call SDL_VideoInit() again?
	 */
	SDL_VideoSurface = (mode != NULL) ? mode : prev_mode;

	if ( (mode != NULL) && (!is_opengl) ) {
		/* Sanity check */
		if ( (mode->w < width) || (mode->h < height) ) {
			SDL_SetError("Video mode smaller than requested");
			return(NULL);
		}

		/* If we have a palettized surface, create a default palette */
		if ( mode->format->palette ) {
			SDL_PixelFormat *vf = mode->format;
			SDL_DitherColors(vf->palette->colors, vf->BitsPerPixel);
			video->SetColors(this, 0, vf->palette->ncolors,
			                           vf->palette->colors);
		}

		/* Clear the surface to black */
		video->offset_x = 0;
		video->offset_y = 0;
		mode->offset = 0;
		SDL_SetClipRect(mode, NULL);
		SDL_ClearSurface(mode);

		/* Now adjust the offsets to match the desired mode */
		video->offset_x = (mode->w-width)/2;
		video->offset_y = (mode->h-height)/2;
		mode->offset = video->offset_y*mode->pitch +
				video->offset_x*mode->format->BytesPerPixel;
#ifdef DEBUG_VIDEO
  fprintf(stderr,
	"Requested mode: %dx%dx%d, obtained mode %dx%dx%d (offset %d)\n",
		width, height, bpp,
		mode->w, mode->h, mode->format->BitsPerPixel, mode->offset);
#endif
		mode->w = width;
		mode->h = height;
		SDL_SetClipRect(mode, NULL);
	}
	SDL_ResetCursor();
	SDL_UnlockCursor();

	/* If we failed setting a video mode, return NULL... (Uh Oh!) */
	if ( mode == NULL ) {
		return(NULL);
	}

	/* If there is no window manager, set the SDL_NOFRAME flag */
	if ( ! video->info.wm_available ) {
		mode->flags |= SDL_NOFRAME;
	}

	/* Reset the mouse cursor and grab for new video mode */
	SDL_SetCursor(NULL);
	if ( video->UpdateMouse ) {
		video->UpdateMouse(this);
	}
	SDL_WM_GrabInput(saved_grab);
	SDL_GetRelativeMouseState(NULL, NULL); /* Clear first large delta */

#if SDL_VIDEO_OPENGL
	/* Load GL symbols (before MakeCurrent, where we need glGetString). */
	if ( flags & (SDL_OPENGL | SDL_OPENGLBLIT) ) {

#if defined(__QNXNTO__) && (_NTO_VERSION < 630)
#define __SDL_NOGETPROCADDR__
#elif defined(__MINT__)
#define __SDL_NOGETPROCADDR__
#endif
#ifdef __SDL_NOGETPROCADDR__
    #define SDL_PROC(ret,func,params) video->func=func;
#else
    #define SDL_PROC(ret,func,params) \
    do { \
        video->func = SDL_GL_GetProcAddress(#func); \
        if ( ! video->func ) { \
            SDL_SetError("Couldn't load GL function %s: %s\n", #func, SDL_GetError()); \
        return(NULL); \
        } \
    } while ( 0 );

#endif /* __SDL_NOGETPROCADDR__ */

#include "SDL_glfuncs.h"
#undef SDL_PROC	
	}
#endif /* SDL_VIDEO_OPENGL */

	/* If we're running OpenGL, make the context current */
	if ( (video->screen->flags & SDL_OPENGL) &&
	      video->GL_MakeCurrent ) {
		if ( video->GL_MakeCurrent(this) < 0 ) {
			return(NULL);
		}
	}

	/* Set up a fake SDL surface for OpenGL "blitting" */
	if ( (flags & SDL_OPENGLBLIT) == SDL_OPENGLBLIT ) {
		/* Load GL functions for performing the texture updates */
#if SDL_VIDEO_OPENGL

		/* Create a software surface for blitting */
#ifdef GL_VERSION_1_2
		/* If the implementation either supports the packed pixels
		   extension, or implements the core OpenGL 1.2 API, it will
		   support the GL_UNSIGNED_SHORT_5_6_5 texture format.
		 */
		if ( (bpp == 16) &&
		     (SDL_strstr((const char *)video->glGetString(GL_EXTENSIONS), "GL_EXT_packed_pixels") ||
		     (SDL_atof((const char *)video->glGetString(GL_VERSION)) >= 1.2f))
		   ) {
			video->is_32bit = 0;
			SDL_VideoSurface = SDL_CreateRGBSurface(
				flags, 
				width, 
				height,  
				16,
				31 << 11,
				63 << 5,
				31,
				0
				);
		}
		else
#endif /* OpenGL 1.2 */
		{
			video->is_32bit = 1;
			SDL_VideoSurface = SDL_CreateRGBSurface(
				flags, 
				width, 
				height, 
				32, 
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
				0x000000FF,
				0x0000FF00,
				0x00FF0000,
				0xFF000000
#else
				0xFF000000,
				0x00FF0000,
				0x0000FF00,
				0x000000FF
#endif
				);
		}
		if ( ! SDL_VideoSurface ) {
			return(NULL);
		}
		SDL_VideoSurface->flags = mode->flags | SDL_OPENGLBLIT;

		/* Free the original video mode surface (is this safe?) */
		SDL_FreeSurface(mode);

		/* Set the surface completely opaque & white by default */
		SDL_memset( SDL_VideoSurface->pixels, 255, SDL_VideoSurface->h * SDL_VideoSurface->pitch );
		video->glGenTextures( 1, &video->texture );
		video->glBindTexture( GL_TEXTURE_2D, video->texture );
		video->glTexImage2D(
			GL_TEXTURE_2D,
			0,
			video->is_32bit ? GL_RGBA : GL_RGB,
			256,
			256,
			0,
			video->is_32bit ? GL_RGBA : GL_RGB,
#ifdef GL_VERSION_1_2
			video->is_32bit ? GL_UNSIGNED_BYTE : GL_UNSIGNED_SHORT_5_6_5,
#else
			GL_UNSIGNED_BYTE,
#endif
			NULL);

		video->UpdateRects = SDL_GL_UpdateRectsLock;
#else
		SDL_SetError("Somebody forgot to #define SDL_VIDEO_OPENGL");
		return(NULL);
#endif
	}

	/* Create a shadow surface if necessary */
	/* There are three conditions under which we create a shadow surface:
		1.  We need a particular bits-per-pixel that we didn't get.
		2.  We need a hardware palette and didn't get one.
		3.  We need a software surface and got a hardware surface.
	*/
	if ( !(SDL_VideoSurface->flags & SDL_OPENGL) &&
	     (
	     (  !(flags&SDL_ANYFORMAT) &&
			(SDL_VideoSurface->format->BitsPerPixel != bpp)) ||
	     (   (flags&SDL_HWPALETTE) && 
				!(SDL_VideoSurface->flags&SDL_HWPALETTE)) ||
		/* If the surface is in hardware, video writes are visible
		   as soon as they are performed, so we need to buffer them
		 */
	     (   ((flags&SDL_HWSURFACE) == SDL_SWSURFACE) &&
				(SDL_VideoSurface->flags&SDL_HWSURFACE)) ||
	     (   (flags&SDL_DOUBLEBUF) &&
				(SDL_VideoSurface->flags&SDL_HWSURFACE) &&
				!(SDL_VideoSurface->flags&SDL_DOUBLEBUF))
	     ) ) {
		SDL_CreateShadowSurface(bpp);
		if ( SDL_ShadowSurface == NULL ) {
			SDL_SetError("Couldn't create shadow surface");
			return(NULL);
		}
		SDL_PublicSurface = SDL_ShadowSurface;
	} else {
		SDL_PublicSurface = SDL_VideoSurface;
	}
	video->info.vfmt = SDL_VideoSurface->format;
	video->info.current_w = SDL_VideoSurface->w;
	video->info.current_h = SDL_VideoSurface->h;

	/* We're done! */
	return(SDL_PublicSurface);
}

/* 
 * Convert a surface into the video pixel format.
 */
SDL_Surface * SDL_DisplayFormat (SDL_Surface *surface)
{
	Uint32 flags;

	if ( ! SDL_PublicSurface ) {
		SDL_SetError("No video mode has been set");
		return(NULL);
	}
	/* Set the flags appropriate for copying to display surface */
	if (((SDL_PublicSurface->flags&SDL_HWSURFACE) == SDL_HWSURFACE) && current_video->info.blit_hw)
		flags = SDL_HWSURFACE;
	else 
		flags = SDL_SWSURFACE;
#ifdef AUTORLE_DISPLAYFORMAT
	flags |= (surface->flags & (SDL_SRCCOLORKEY|SDL_SRCALPHA));
	flags |= SDL_RLEACCELOK;
#else
	flags |= surface->flags & (SDL_SRCCOLORKEY|SDL_SRCALPHA|SDL_RLEACCELOK);
#endif
	return(SDL_ConvertSurface(surface, SDL_PublicSurface->format, flags));
}

/*
 * Convert a surface into a format that's suitable for blitting to
 * the screen, but including an alpha channel.
 */
SDL_Surface *SDL_DisplayFormatAlpha(SDL_Surface *surface)
{
	SDL_PixelFormat *vf;
	SDL_PixelFormat *format;
	SDL_Surface *converted;
	Uint32 flags;
	/* default to ARGB8888 */
	Uint32 amask = 0xff000000;
	Uint32 rmask = 0x00ff0000;
	Uint32 gmask = 0x0000ff00;
	Uint32 bmask = 0x000000ff;

	if ( ! SDL_PublicSurface ) {
		SDL_SetError("No video mode has been set");
		return(NULL);
	}
	vf = SDL_PublicSurface->format;

	switch(vf->BytesPerPixel) {
	    case 2:
		/* For XGY5[56]5, use, AXGY8888, where {X, Y} = {R, B}.
		   For anything else (like ARGB4444) it doesn't matter
		   since we have no special code for it anyway */
		if ( (vf->Rmask == 0x1f) &&
		     (vf->Bmask == 0xf800 || vf->Bmask == 0x7c00)) {
			rmask = 0xff;
			bmask = 0xff0000;
		}
		break;

	    case 3:
	    case 4:
		/* Keep the video format, as long as the high 8 bits are
		   unused or alpha */
		if ( (vf->Rmask == 0xff) && (vf->Bmask == 0xff0000) ) {
			rmask = 0xff;
			bmask = 0xff0000;
		} else if ( vf->Rmask == 0xFF00 && (vf->Bmask == 0xFF000000) ) {
			amask = 0x000000FF;
			rmask = 0x0000FF00;
			gmask = 0x00FF0000;
			bmask = 0xFF000000;
		}
		break;

	    default:
		/* We have no other optimised formats right now. When/if a new
		   optimised alpha format is written, add the converter here */
		break;
	}
	format = SDL_AllocFormat(32, rmask, gmask, bmask, amask);
	flags = SDL_PublicSurface->flags & SDL_HWSURFACE;
	flags |= surface->flags & (SDL_SRCALPHA | SDL_RLEACCELOK);
	converted = SDL_ConvertSurface(surface, format, flags);
	SDL_FreeFormat(format);
	return(converted);
}

/* Load the GL driver library */
int SDL_GL_LoadLibrary(const char *path)
{
	SDL_VideoDevice *video = current_video;
	SDL_VideoDevice *this = current_video;
	int retval;

	retval = -1;
	if ( video == NULL ) {
		SDL_SetError("Video subsystem has not been initialized");
	} else {
		if ( video->GL_LoadLibrary ) {
			retval = video->GL_LoadLibrary(this, path);
		} else {
			SDL_SetError("No dynamic GL support in video driver");
		}
	}
	return(retval);
}

void *SDL_GL_GetProcAddress(const char* proc)
{
	SDL_VideoDevice *video = current_video;
	SDL_VideoDevice *this = current_video;
	void *func;

	func = NULL;
	if ( video->GL_GetProcAddress ) {
		if ( video->gl_config.driver_loaded ) {
			func = video->GL_GetProcAddress(this, proc);
		} else {
			SDL_SetError("No GL driver has been loaded");
		}
	} else {
		SDL_SetError("No dynamic GL support in video driver");
	}
	return func;
}

/* Set the specified GL attribute for setting up a GL video mode */
int SDL_GL_SetAttribute( SDL_GLattr attr, int value )
{
	int retval;
	SDL_VideoDevice *video = current_video;

	retval = 0;
	switch (attr) {
		case SDL_GL_RED_SIZE:
			video->gl_config.red_size = value;
			break;
		case SDL_GL_GREEN_SIZE:
			video->gl_config.green_size = value;
			break;
		case SDL_GL_BLUE_SIZE:
			video->gl_config.blue_size = value;
			break;
		case SDL_GL_ALPHA_SIZE:
			video->gl_config.alpha_size = value;
			break;
		case SDL_GL_DOUBLEBUFFER:
			video->gl_config.double_buffer = value;
			break;
		case SDL_GL_BUFFER_SIZE:
			video->gl_config.buffer_size = value;
			break;
		case SDL_GL_DEPTH_SIZE:
			video->gl_config.depth_size = value;
			break;
		case SDL_GL_STENCIL_SIZE:
			video->gl_config.stencil_size = value;
			break;
		case SDL_GL_ACCUM_RED_SIZE:
			video->gl_config.accum_red_size = value;
			break;
		case SDL_GL_ACCUM_GREEN_SIZE:
			video->gl_config.accum_green_size = value;
			break;
		case SDL_GL_ACCUM_BLUE_SIZE:
			video->gl_config.accum_blue_size = value;
			break;
		case SDL_GL_ACCUM_ALPHA_SIZE:
			video->gl_config.accum_alpha_size = value;
			break;
		case SDL_GL_STEREO:
			video->gl_config.stereo = value;
			break;
		case SDL_GL_MULTISAMPLEBUFFERS:
			video->gl_config.multisamplebuffers = value;
			break;
		case SDL_GL_MULTISAMPLESAMPLES:
			video->gl_config.multisamplesamples = value;
			break;
		case SDL_GL_ACCELERATED_VISUAL:
			video->gl_config.accelerated = value;
			break;
		case SDL_GL_SWAP_CONTROL:
			video->gl_config.swap_control = value;
			break;
		default:
			SDL_SetError("Unknown OpenGL attribute");
			retval = -1;
			break;
	}
	return(retval);
}

/* Retrieve an attribute value from the windowing system. */
int SDL_GL_GetAttribute(SDL_GLattr attr, int* value)
{
	int retval = -1;
	SDL_VideoDevice* video = current_video;
	SDL_VideoDevice* this = current_video;

	if ( video->GL_GetAttribute ) {
		retval = this->GL_GetAttribute(this, attr, value);
	} else {
		*value = 0;
		SDL_SetError("GL_GetAttribute not supported");
	}
	return retval;
}

/* Perform a GL buffer swap on the current GL context */
void SDL_GL_SwapBuffers(void)
{
	SDL_VideoDevice *video = current_video;
	SDL_VideoDevice *this = current_video;

	if ( video->screen->flags & SDL_OPENGL ) {
		video->GL_SwapBuffers(this);
	} else {
		SDL_SetError("OpenGL video mode has not been set");
	}
}

/* Update rects with locking */
void SDL_GL_UpdateRectsLock(SDL_VideoDevice* this, int numrects, SDL_Rect *rects)
{
	SDL_GL_Lock();
 	SDL_GL_UpdateRects(numrects, rects);
	SDL_GL_Unlock();
}

/* Update rects without state setting and changing (the caller is responsible for it) */
void SDL_GL_UpdateRects(int numrects, SDL_Rect *rects)
{
#if SDL_VIDEO_OPENGL
	SDL_VideoDevice *this = current_video;
	SDL_Rect update, tmp;
	int x, y, i;

	for ( i = 0; i < numrects; i++ )
	{
		tmp.y = rects[i].y;
		tmp.h = rects[i].h;
		for ( y = 0; y <= rects[i].h / 256; y++ )
		{
			tmp.x = rects[i].x;
			tmp.w = rects[i].w;
			for ( x = 0; x <= rects[i].w / 256; x++ )
			{
				update.x = tmp.x;
				update.y = tmp.y;
				update.w = tmp.w;
				update.h = tmp.h;

				if ( update.w > 256 )
					update.w = 256;

				if ( update.h > 256 )
					update.h = 256;
			
				this->glFlush();
				this->glTexSubImage2D( 
					GL_TEXTURE_2D, 
					0, 
					0, 
					0, 
					update.w, 
					update.h, 
					this->is_32bit? GL_RGBA : GL_RGB,
#ifdef GL_VERSION_1_2
					this->is_32bit ? GL_UNSIGNED_BYTE : GL_UNSIGNED_SHORT_5_6_5,
#else
					GL_UNSIGNED_BYTE,
#endif
					(Uint8 *)this->screen->pixels + 
						this->screen->format->BytesPerPixel * update.x + 
						update.y * this->screen->pitch );
	
				this->glFlush();
				/*
				* Note the parens around the function name:
				* This is because some OpenGL implementations define glTexCoord etc 
				* as macros, and we don't want them expanded here.
				*/
				this->glBegin(GL_TRIANGLE_STRIP);
					(this->glTexCoord2f)( 0.0, 0.0 );	
					(this->glVertex2i)( update.x, update.y );
					(this->glTexCoord2f)( (float)(update.w / 256.0), 0.0 );	
					(this->glVertex2i)( update.x + update.w, update.y );
					(this->glTexCoord2f)( 0.0, (float)(update.h / 256.0) );
					(this->glVertex2i)( update.x, update.y + update.h );
					(this->glTexCoord2f)( (float)(update.w / 256.0), (float)(update.h / 256.0) );	
					(this->glVertex2i)( update.x + update.w	, update.y + update.h );
				this->glEnd();	
			
				tmp.x += 256;
				tmp.w -= 256;
			}
			tmp.y += 256;
			tmp.h -= 256;
		}
	}
#endif
}

/* Lock == save current state */
void SDL_GL_Lock()
{
#if SDL_VIDEO_OPENGL
	lock_count--;
	if (lock_count==-1)
	{
		SDL_VideoDevice *this = current_video;

		this->glPushAttrib( GL_ALL_ATTRIB_BITS );	/* TODO: narrow range of what is saved */
#ifdef GL_CLIENT_PIXEL_STORE_BIT
		this->glPushClientAttrib( GL_CLIENT_PIXEL_STORE_BIT );
#endif

		this->glEnable(GL_TEXTURE_2D);
		this->glEnable(GL_BLEND);
		this->glDisable(GL_FOG);
		this->glDisable(GL_ALPHA_TEST);
		this->glDisable(GL_DEPTH_TEST);
		this->glDisable(GL_SCISSOR_TEST);	
		this->glDisable(GL_STENCIL_TEST);
		this->glDisable(GL_CULL_FACE);

		this->glBindTexture( GL_TEXTURE_2D, this->texture );
		this->glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
		this->glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		this->glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		this->glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		this->glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

		this->glPixelStorei( GL_UNPACK_ROW_LENGTH, this->screen->pitch / this->screen->format->BytesPerPixel );
		this->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		(this->glColor4f)(1.0, 1.0, 1.0, 1.0);		/* Solaris workaround */

		this->glViewport(0, 0, this->screen->w, this->screen->h);
		this->glMatrixMode(GL_PROJECTION);
		this->glPushMatrix();
		this->glLoadIdentity();

		this->glOrtho(0.0, (GLdouble) this->screen->w, (GLdouble) this->screen->h, 0.0, 0.0, 1.0);

		this->glMatrixMode(GL_MODELVIEW);
		this->glPushMatrix();
		this->glLoadIdentity();
	}
#endif
}

/* Unlock == restore saved state */
void SDL_GL_Unlock()
{
#if SDL_VIDEO_OPENGL
	lock_count++;
	if (lock_count==0)
	{
		SDL_VideoDevice *this = current_video;

		this->glPopMatrix();
		this->glMatrixMode(GL_PROJECTION);
		this->glPopMatrix();

		this->glPopClientAttrib();
		this->glPopAttrib();
	}
#endif
}