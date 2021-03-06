/*
 *					GPAC Multimedia Framework
 *
 *			Authors: DINH Anh Min - Jean le Feuvre
 *				Copyright (c) 2005-200X ENST
 *					All rights reserved
 *
 *  This file is part of GPAC /  X11 video output module
 *
 *  GPAC is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  GPAC is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *   
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *
 */

#include "x11_out.h"
#include <gpac/constants.h>

void X11_SetupWindow (GF_VideoOutput * vout);

//=====================================
/*
 * Flush video: draw main image(surface/pixmap) -> os_handle
 */
//=====================================
GF_Err X11_Flush(struct _video_out *vout, GF_Window * dest)
{
	Window cur_wnd;
	/*
	 * write backbuffer to screen
	 */
	X11VID ();

	XSync(xWindow->display, False);

	if (!xWindow->is_init) return GF_OK;
	cur_wnd = xWindow->fullscreen ? xWindow->full_wnd : xWindow->wnd;

	if (xWindow->is_3D_out) {
#ifdef GPAC_HAS_OPENGL
		glXSwapBuffers(xWindow->display, cur_wnd);
#endif
		return GF_OK;
	}

	switch (xWindow->videoaccesstype) {
#ifdef GPAC_HAS_X11_SHM
	case VIDEO_XI_SHMSTD:
	  XShmPutImage (xWindow->display, cur_wnd, xWindow->the_gc, xWindow->surface,
			0, 0, dest->x, dest->y, dest->w, dest->h, True);
		break;
	case VIDEO_XI_SHMPIXMAP:
		XClearWindow(xWindow->display, cur_wnd);
		break;
#endif
	case VIDEO_XI_STANDARD:
		XRaiseWindow(xWindow->display, xWindow->wnd);
		XPutImage (xWindow->display, cur_wnd, xWindow->the_gc, xWindow->surface,
			0, 0, dest->x, dest->y, dest->w, dest->h);
		break;
	}
	return GF_OK;
}

//=====================================
/*
 * Translate X_Key to GF_Key
 */
//=====================================
static void x11_translate_key(u32 X11Key, GF_EventKey *evt)
{
	evt->flags = 0;
	evt->hw_code = X11Key & 0xFF;
	switch (X11Key) {

	case XK_BackSpace: evt->key_code = GF_KEY_BACKSPACE; break;
	case XK_Tab: evt->key_code = GF_KEY_TAB; break;
	//case XK_Linefeed: evt->key_code = GF_KEY_LINEFEED; break;
	case XK_Clear: evt->key_code = GF_KEY_CLEAR; break;

	case XK_KP_Enter:
		evt->flags = GF_KEY_EXT_NUMPAD; 
	case XK_Return: 
		evt->key_code = GF_KEY_ENTER; 
		break;
	case XK_Pause: evt->key_code = GF_KEY_PAUSE; break;
	case XK_Caps_Lock: evt->key_code = GF_KEY_CAPSLOCK; break;
	case XK_Scroll_Lock: evt->key_code = GF_KEY_SCROLL; break;
	case XK_Escape: evt->key_code = GF_KEY_ESCAPE; break;
	case XK_Delete: 
		evt->key_code = GF_KEY_DEL; 
		break;

	case XK_Kanji: evt->key_code = GF_KEY_KANJIMODE; break;
	case XK_Katakana: evt->key_code = GF_KEY_JAPANESEKATAKANA; break;
	case XK_Romaji: evt->key_code = GF_KEY_JAPANESEROMAJI; break;
	case XK_Hiragana: evt->key_code = GF_KEY_JAPANESEHIRAGANA; break;
	case XK_Kana_Lock: evt->key_code = GF_KEY_KANAMODE; break;

	case XK_Home: evt->key_code = GF_KEY_HOME; break;
	case XK_Left: evt->key_code = GF_KEY_LEFT; break;
	case XK_Up: evt->key_code = GF_KEY_UP; break;
	case XK_Right: evt->key_code = GF_KEY_RIGHT; break;
	case XK_Down: evt->key_code = GF_KEY_DOWN; break;
	case XK_Page_Up: evt->key_code = GF_KEY_PAGEUP; break;
	case XK_Page_Down: evt->key_code = GF_KEY_PAGEDOWN; break;
	case XK_End: evt->key_code = GF_KEY_END; break;
	//case XK_Begin: evt->key_code = GF_KEY_BEGIN; break;


	case XK_Select: evt->key_code = GF_KEY_SELECT; break;
	case XK_Print: evt->key_code = GF_KEY_PRINTSCREEN; break;
	case XK_Execute: evt->key_code = GF_KEY_EXECUTE; break;
	case XK_Insert: evt->key_code = GF_KEY_INSERT; break;
	case XK_Undo: evt->key_code = GF_KEY_UNDO; break;
	//case XK_Redo: evt->key_code = GF_KEY_BEGIN; break;
	//case XK_Menu: evt->key_code = GF_KEY_BEGIN; break;
	case XK_Find: evt->key_code = GF_KEY_FIND; break;
	case XK_Cancel: evt->key_code = GF_KEY_CANCEL; break;
	case XK_Help: evt->key_code = GF_KEY_HELP; break;
	//case XK_Break: evt->key_code = GF_KEY_BREAK; break;
	//case XK_Mode_switch: evt->key_code = GF_KEY_BEGIN; break;
	case XK_Num_Lock: evt->key_code = GF_KEY_NUMLOCK; break;
	
	case XK_F1: evt->key_code = GF_KEY_F1; break;
	case XK_F2: evt->key_code = GF_KEY_F2; break;
	case XK_F3: evt->key_code = GF_KEY_F3; break;
	case XK_F4: evt->key_code = GF_KEY_F4; break;
	case XK_F5: evt->key_code = GF_KEY_F5; break;
	case XK_F6: evt->key_code = GF_KEY_F6; break;
	case XK_F7: evt->key_code = GF_KEY_F7; break;
	case XK_F8: evt->key_code = GF_KEY_F8; break;
	case XK_F9: evt->key_code = GF_KEY_F9; break;
	case XK_F10: evt->key_code = GF_KEY_F10; break;
	case XK_F11: evt->key_code = GF_KEY_F11; break;
	case XK_F12: evt->key_code = GF_KEY_F12; break;
	case XK_F13: evt->key_code = GF_KEY_F13; break;
	case XK_F14: evt->key_code = GF_KEY_F14; break;
	case XK_F15: evt->key_code = GF_KEY_F15; break;
	case XK_F16: evt->key_code = GF_KEY_F16; break;
	case XK_F17: evt->key_code = GF_KEY_F17; break;
	case XK_F18: evt->key_code = GF_KEY_F18; break;
	case XK_F19: evt->key_code = GF_KEY_F19; break;
	case XK_F20: evt->key_code = GF_KEY_F20; break;
	case XK_F21: evt->key_code = GF_KEY_F21; break;
	case XK_F22: evt->key_code = GF_KEY_F22; break;
	case XK_F23: evt->key_code = GF_KEY_F23; break;
	case XK_F24: evt->key_code = GF_KEY_F24; break;

	case XK_KP_Delete: 
	case XK_KP_Decimal: 
		evt->flags = GF_KEY_EXT_NUMPAD; 
		evt->key_code = GF_KEY_COMMA; 
		break;

	case XK_KP_Insert:
	case XK_KP_0: 
		evt->flags = GF_KEY_EXT_NUMPAD; evt->key_code = GF_KEY_0; break;
	case XK_KP_End:
	case XK_KP_1: 
		evt->flags = GF_KEY_EXT_NUMPAD; evt->key_code = GF_KEY_1; break;
	case XK_KP_Down: 
	case XK_KP_2: 
		evt->flags = GF_KEY_EXT_NUMPAD; evt->key_code = GF_KEY_2; break;
	case XK_KP_Page_Down: 
	case XK_KP_3:
		 evt->flags = GF_KEY_EXT_NUMPAD; evt->key_code = GF_KEY_3; break;
	case XK_KP_Left: 
	case XK_KP_4: 
		evt->flags = GF_KEY_EXT_NUMPAD; evt->key_code = GF_KEY_4; break;
	case XK_KP_Begin: 
	case XK_KP_5:
		evt->flags = GF_KEY_EXT_NUMPAD; evt->key_code = GF_KEY_5; break;
	case XK_KP_Right: 
	case XK_KP_6:
		evt->flags = GF_KEY_EXT_NUMPAD; evt->key_code = GF_KEY_6; break;
	case XK_KP_Home: 
	case XK_KP_7:
		evt->flags = GF_KEY_EXT_NUMPAD; evt->key_code = GF_KEY_7; break;
	case XK_KP_Up: 
	case XK_KP_8:
		evt->flags = GF_KEY_EXT_NUMPAD; evt->key_code = GF_KEY_8; break;
	case XK_KP_Page_Up: 
	case XK_KP_9:
		evt->flags = GF_KEY_EXT_NUMPAD; evt->key_code = GF_KEY_9; break;
	case XK_KP_Add:
		evt->flags = GF_KEY_EXT_NUMPAD; evt->key_code = GF_KEY_PLUS; break;
	case XK_KP_Subtract:
		evt->flags = GF_KEY_EXT_NUMPAD; evt->key_code = GF_KEY_HYPHEN; break;
	case XK_KP_Multiply:
		evt->flags = GF_KEY_EXT_NUMPAD; evt->key_code = GF_KEY_STAR; break;
	case XK_KP_Divide:
		evt->flags = GF_KEY_EXT_NUMPAD; evt->key_code = GF_KEY_SLASH; break;


	case XK_Shift_R:
		evt->flags = GF_KEY_EXT_RIGHT;
	case XK_Shift_L:
		evt->key_code = GF_KEY_SHIFT; 
		break;
	case XK_Control_R:
		evt->flags = GF_KEY_EXT_RIGHT;
	case XK_Control_L:
		evt->key_code = GF_KEY_CONTROL; 
		break;
	case XK_Alt_R:
		evt->flags = GF_KEY_EXT_RIGHT;
	case XK_Alt_L:
		evt->key_code = GF_KEY_ALT; 
		break;
	case XK_Super_R:
		evt->flags = GF_KEY_EXT_RIGHT;
	case XK_Super_L:
		evt->key_code = GF_KEY_WIN; 
		break;

	case XK_Menu: evt->key_code = GF_KEY_META; break;
#ifdef XK_XKB_KEYS
	case XK_ISO_Level3_Shift: evt->key_code = GF_KEY_ALTGRAPH; break;

#endif
	case '!': evt->key_code = GF_KEY_EXCLAMATION; break;
	case '"': evt->key_code = GF_KEY_QUOTATION; break;
	case '#': evt->key_code = GF_KEY_NUMBER; break;
	case '$': evt->key_code = GF_KEY_DOLLAR; break;
	case '&': evt->key_code = GF_KEY_AMPERSAND; break;
	case '\'': evt->key_code = GF_KEY_APOSTROPHE; break;
	case '(': evt->key_code = GF_KEY_LEFTPARENTHESIS; break;
	case ')': evt->key_code = GF_KEY_RIGHTPARENTHESIS; break;
	case ',': evt->key_code = GF_KEY_COMMA; break;
	case ':': evt->key_code = GF_KEY_COLON; break;
	case ';': evt->key_code = GF_KEY_SEMICOLON; break;
	case '<': evt->key_code = GF_KEY_LESSTHAN; break;
	case '>': evt->key_code = GF_KEY_GREATERTHAN; break;
	case '?': evt->key_code = GF_KEY_QUESTION; break;
	case '@': evt->key_code = GF_KEY_AT; break;
	case '[': evt->key_code = GF_KEY_LEFTSQUAREBRACKET; break;
	case ']': evt->key_code = GF_KEY_RIGHTSQUAREBRACKET; break;
	case '\\': evt->key_code = GF_KEY_BACKSLASH; break;
	case '_': evt->key_code = GF_KEY_UNDERSCORE; break;
	case '`': evt->key_code = GF_KEY_GRAVEACCENT; break;
	case ' ': evt->key_code = GF_KEY_SPACE; break;
	case '/': evt->key_code = GF_KEY_SLASH; break;
	case '*': evt->key_code = GF_KEY_STAR; break;
	case '-': evt->key_code = GF_KEY_HYPHEN; break;
	case '+': evt->key_code = GF_KEY_PLUS; break;
	case '=': evt->key_code = GF_KEY_EQUALS; break;
	case '^': evt->key_code = GF_KEY_CIRCUM; break;
	case '{': evt->key_code = GF_KEY_LEFTCURLYBRACKET; break;
	case '}': evt->key_code = GF_KEY_RIGHTCURLYBRACKET; break;
	case '|': evt->key_code = GF_KEY_PIPE; break;
	default:
		if ((X11Key>='0') && (X11Key<='9'))  evt->key_code = GF_KEY_0 + X11Key - '0';
		else if ((X11Key>='A') && (X11Key<='Z'))  evt->key_code = GF_KEY_A + X11Key - 'A';
		/*DOM3: translate to A -> Z, not a -> z*/
		else if ((X11Key>='a') && (X11Key<='z'))  {
			evt->key_code = GF_KEY_A + X11Key - 'a';
			evt->hw_code = X11Key - 'a' + 'A';
		}
		else {
			evt->key_code = 0; 
			GF_LOG(GF_LOG_WARNING, GF_LOG_MMIO, ("[X11] Unrecognized key %X\n", X11Key));
		}
		break;
	}
}



/* taken from SDL
 Ack!  XPending() actually performs a blocking read if no events available 
*/
static int X11_Pending(Display *display)
{
	/* Flush the display connection and look to see if events are queued */
	XFlush(display);
	if ( XEventsQueued(display, QueuedAlready) ) return(1);

	/* More drastic measures are required -- see if X is ready to talk */
	{
		static struct timeval zero_time;	/* static == 0 */
		int x11_fd;
		fd_set fdset;
		x11_fd = ConnectionNumber(display);
		FD_ZERO(&fdset);
		FD_SET(x11_fd, &fdset);
		if ( select(x11_fd+1, &fdset, NULL, NULL, &zero_time) == 1 ) {
			return(XPending(display));
		}
	}
	/* Oh well, nothing is ready .. */
	return(0);
}

/*
 * handle X11 events
 * here we handle key, mouse, repaint and window sizing events
 */
static void X11_HandleEvents(GF_VideoOutput *vout)
{
	GF_Event evt;
	Window the_window;
	XComposeStatus state;
	X11VID();
	unsigned char keybuf[32];
	XEvent xevent;

	the_window = xWindow->fullscreen ? xWindow->full_wnd : xWindow->wnd;
	XSync(xWindow->display, False);

	while (X11_Pending(xWindow->display)) {
		  XNextEvent(xWindow->display, &xevent);
		  if (xevent.xany.window!=the_window) continue;
		  switch (xevent.type) {
		    /*
		     * X11 window resized event
		     * must inform GPAC to resize os_handle wnd
		     */
		  case ConfigureNotify:
		    if ((unsigned int) xevent.xconfigure.width != xWindow->w_width
			|| (unsigned int) xevent.xconfigure.height != xWindow->w_height)
		      {
			evt.type = GF_EVENT_SIZE;
			xWindow->w_width = evt.size.width = xevent.xconfigure.width;
			xWindow->w_height = evt.size.height = xevent.xconfigure.height;
			vout->on_event(vout->evt_cbk_hdl, &evt);
		      }
		    break;
		    /*
		     * Windows need repaint
		     */
		  case Expose:
		    if (xevent.xexpose.count > 0)	break;
		    evt.type = GF_EVENT_REFRESH;
		    vout->on_event (vout->evt_cbk_hdl, &evt);
		    break;
		    
		    /* Have we been requested to quit (or another client message?) */
		  case ClientMessage:
		    if ( (xevent.xclient.format == 32) && (xevent.xclient.data.l[0] == xWindow->WM_DELETE_WINDOW) ) {
		      evt.type = GF_EVENT_QUIT;
		      vout->on_event(vout->evt_cbk_hdl, &evt);
		    }
		    break;

		  case KeyPress:
		  case KeyRelease:
		    x11_translate_key(XKeycodeToKeysym (xWindow->display, xevent.xkey.keycode, 0), &evt.key);
			evt.type = (xevent.type ==KeyPress) ? GF_EVENT_KEYDOWN : GF_EVENT_KEYUP;
			vout->on_event (vout->evt_cbk_hdl, &evt);
		    
			if (xevent.type ==KeyPress) {
				XLookupString (&xevent.xkey, (char *) keybuf, sizeof(keybuf), NULL, &state);
				if (keybuf[0]) {
					evt.character.unicode_char = keybuf[0];
					evt.type = GF_EVENT_TEXTINPUT;
					vout->on_event (vout->evt_cbk_hdl, &evt);
				}
			}
			break;
		    
		  case ButtonPress:
			  if (!xWindow->fullscreen && !xWindow->has_focus) {
				  xWindow->has_focus = 1;
				  XSetInputFocus(xWindow->display, xWindow->wnd, RevertToParent, CurrentTime);
			  }
		  case ButtonRelease:
		    //				last_mouse_move = xevent.xbutton.time;
		    evt.mouse.x = xevent.xbutton.x;
		    evt.mouse.y = xevent.xbutton.y;
			evt.type = (xevent.type == ButtonRelease) ? GF_EVENT_MOUSEUP : GF_EVENT_MOUSEDOWN;
		    
		    switch (xevent.xbutton.button) {
		    case Button1:
		      evt.mouse.button = GF_MOUSE_LEFT;
		      vout->on_event (vout->evt_cbk_hdl, &evt);
		      break;
		    case Button2:
		      evt.mouse.button = GF_MOUSE_MIDDLE;
		      vout->on_event (vout->evt_cbk_hdl, &evt);
		      break;
		    case Button3:
		      evt.mouse.button = GF_MOUSE_RIGHT;
		      vout->on_event (vout->evt_cbk_hdl, &evt);
		      break;
		    case Button4:
		      evt.type = GF_EVENT_MOUSEWHEEL;
		      evt.mouse.wheel_pos = FIX_ONE;
		      vout->on_event(vout->evt_cbk_hdl, &evt);
		      break;
		    case Button5:
		      evt.type = GF_EVENT_MOUSEWHEEL;
		      evt.mouse.wheel_pos = -FIX_ONE;
		      vout->on_event(vout->evt_cbk_hdl, &evt);
		      break;
		    }
		    if (!xWindow->fullscreen && (xevent.type==ButtonPress) ) 
		      XSetInputFocus(xWindow->display, xWindow->wnd, RevertToNone, CurrentTime);
		    break;
		    
		  case MotionNotify:
		    evt.type = GF_EVENT_MOUSEMOVE;
		    evt.mouse.x = xevent.xmotion.x;
		    evt.mouse.y = xevent.xmotion.y;
		    vout->on_event (vout->evt_cbk_hdl, &evt);
		    break;
		    
		  case PropertyNotify:
		    break;
		  case MapNotify:
		    break;
		  case CirculateNotify:
		    break;
		  case UnmapNotify:
		    break;
		  case ReparentNotify:
		    break;
		  case FocusOut:
			if (!xWindow->fullscreen) xWindow->has_focus = 0;
		    break;
		  case FocusIn:
			if (!xWindow->fullscreen) xWindow->has_focus = 1;
		    break;
		    
		  case DestroyNotify:
		      evt.type = GF_EVENT_QUIT;
		      vout->on_event(vout->evt_cbk_hdl, &evt);
		    break;
		    
		  default:
		    break;
		  }
	 }
}



#ifdef GPAC_HAS_OPENGL
static GF_Err X11_SetupGL(GF_VideoOutput *vout)
{
  GF_Event evt;
  XWindow *xWin = (XWindow *)vout->opaque;
  XSync(xWin->display, False);
  xWin->glx_context = glXCreateContext(xWin->display,xWin->glx_visualinfo, NULL, True);
  XSync(xWin->display, False);
  if (!xWin->glx_context) return GF_IO_ERR;
  if ( ! glXMakeCurrent(xWin->display, xWin->fullscreen ? xWin->full_wnd : xWin->wnd, xWin->glx_context) ) return GF_IO_ERR;
  XSync(xWin->display, False);
  evt.type = GF_EVENT_VIDEO_SETUP;
  vout->on_event (vout->evt_cbk_hdl,&evt);
  xWin->is_init = 1;
  return GF_OK;
}

static void X11_ReleaseGL(XWindow *xWin)
{
  XSync(xWin->display, False);
  if (xWin->glx_context) {
    glXMakeCurrent(xWin->display, None, NULL);
    glXDestroyContext(xWin->display, xWin->glx_context);
    xWin->glx_context = NULL;
  }
  xWin->is_init = 0;
  XSync(xWin->display, False);
}
#endif


static void X11_ReleaseBackBuffer (GF_VideoOutput * vout)
{
	X11VID ();
	if (xWindow->is_init == 1) {
		switch (xWindow->videoaccesstype) {
#ifdef GPAC_HAS_X11_SHM
		case VIDEO_XI_SHMSTD:
			XShmDetach (xWindow->display, xWindow->shmseginfo);
			if (xWindow->surface) XDestroyImage(xWindow->surface);
			xWindow->surface = NULL;
			if (xWindow->shmseginfo->shmaddr) shmdt(xWindow->shmseginfo->shmaddr);
			if (xWindow->shmseginfo->shmid >= 0)
				shmctl (xWindow->shmseginfo->shmid, IPC_RMID, NULL);
			free (xWindow->shmseginfo);
			xWindow->shmseginfo = NULL;
			break;
		case VIDEO_XI_SHMPIXMAP:
			XShmDetach(xWindow->display, xWindow->shmseginfo);
			XFreePixmap(xWindow->display, xWindow->pixmap);
			if (xWindow->shmseginfo->shmaddr) shmdt (xWindow->shmseginfo->shmaddr);
			if (xWindow->shmseginfo->shmid >= 0) shmctl (xWindow->shmseginfo->shmid, IPC_RMID, NULL);
			free (xWindow->shmseginfo);
			xWindow->shmseginfo = NULL;
			break;
#endif
		case VIDEO_XI_STANDARD:
			if (xWindow->back_buffer->buffer) free (xWindow->back_buffer->buffer);
			xWindow->back_buffer->buffer = NULL;
			if (xWindow->surface) XFree(xWindow->surface);
			xWindow->surface = NULL;
			break;
		}
		xWindow->is_init = 0;
	}
}

/*
 * init backbuffer
 */
GF_Err X11_InitBackBuffer (GF_VideoOutput * vout, u32 VideoWidth, u32 VideoHeight)
{
	Window cur_wnd;
	u32 size;
	VideoWidth = VideoWidth > 32 ? VideoWidth : 32;
	VideoWidth = VideoWidth < 4096 ? VideoWidth : 4096;
	VideoHeight = VideoHeight > 32 ? VideoHeight : 32;
	VideoHeight = VideoHeight > 4096 ? 4096 : VideoHeight;

	X11VID ();
	if (!xWindow || !VideoWidth || !VideoHeight)
		return GF_BAD_PARAM;

	X11_ReleaseBackBuffer(vout);
	/*WATCHOUT seems we need even width when using shared memory extensions*/
	if ((xWindow->videoaccesstype!=VIDEO_XI_STANDARD) && (VideoWidth%2)) 
	  VideoWidth++;

	xWindow->back_buffer->BPP = xWindow->bpp;
	xWindow->back_buffer->width = VideoWidth;
	xWindow->back_buffer->height = VideoHeight;
	xWindow->back_buffer->pitch = VideoWidth * xWindow->bpp;
	size = VideoWidth * VideoHeight * xWindow->bpp;
	cur_wnd = xWindow->fullscreen ? xWindow->full_wnd : xWindow->wnd;

	switch (xWindow->videoaccesstype) {
#ifdef GPAC_HAS_X11_SHM
	case VIDEO_XI_SHMPIXMAP:
		GF_SAFEALLOC(xWindow->shmseginfo, XShmSegmentInfo);
		xWindow->shmseginfo->shmid = shmget(IPC_PRIVATE, size, IPC_CREAT | 0777);
		xWindow->shmseginfo->shmaddr = shmat(xWindow->shmseginfo->shmid, 0, 0);
		xWindow->back_buffer->buffer = (unsigned char *) xWindow->shmseginfo->shmaddr;
		xWindow->shmseginfo->readOnly = False;
		XShmAttach (xWindow->display, xWindow->shmseginfo);
		xWindow->pixmap = XShmCreatePixmap (xWindow->display, cur_wnd,
								xWindow->back_buffer->buffer, xWindow->shmseginfo,
								VideoWidth, VideoHeight, xWindow->depth);
		XSetWindowBackgroundPixmap (xWindow->display, cur_wnd, xWindow->pixmap);
		break;

	case VIDEO_XI_SHMSTD:
		GF_SAFEALLOC(xWindow->shmseginfo, XShmSegmentInfo);
		xWindow->surface = XShmCreateImage (xWindow->display, xWindow->visual,
										 xWindow->depth, ZPixmap, NULL, xWindow->shmseginfo, 
										 VideoWidth, VideoHeight);
		xWindow->shmseginfo->shmid = shmget(IPC_PRIVATE,
									 xWindow->surface->bytes_per_line * xWindow->surface->height,
									 IPC_CREAT | 0777);

		xWindow->surface->data = xWindow->shmseginfo->shmaddr = shmat(xWindow->shmseginfo->shmid, NULL, 0);
		xWindow->back_buffer->buffer = (unsigned char *) xWindow->surface->data;
		xWindow->shmseginfo->readOnly = False;
		XShmAttach (xWindow->display, xWindow->shmseginfo);
		break;
#endif

	case VIDEO_XI_STANDARD:
		xWindow->back_buffer->buffer = (char *) malloc(sizeof(char)*size);
		xWindow->surface = XCreateImage (xWindow->display, xWindow->visual,
				      xWindow->depth, ZPixmap,
				      0,
				      xWindow->back_buffer->buffer,
				      VideoWidth, VideoHeight,
				      xWindow->bpp*8, xWindow->back_buffer->pitch);
	}
	xWindow->is_init = 1;
	return GF_OK;
}

/*
 * resize buffer: note we don't resize window here
 */
GF_Err X11_ResizeBackBuffer (struct _video_out *vout, u32 newWidth, u32 newHeight)
{
	X11VID ();
	if ((newWidth != xWindow->back_buffer->width) || (newHeight != xWindow->back_buffer->height)) {
		if ((newWidth >= 32) && (newHeight >= 32))
			return X11_InitBackBuffer (vout, newWidth, newHeight);
	}
	return GF_OK;
}

GF_Err X11_ProcessEvent (struct _video_out * vout, GF_Event * evt)
{
	X11VID ();

	if (!xWindow->setup_done) X11_SetupWindow(vout);

	if (evt) {

	switch (evt->type) {
	case GF_EVENT_SET_CURSOR:
		break;
	case GF_EVENT_SET_CAPTION:
		if (!xWindow->par_wnd) XStoreName (xWindow->display, xWindow->wnd, evt->caption.caption);
		break;
	case GF_EVENT_SHOWHIDE:
		break;
	case GF_EVENT_SIZE:
		/*if owning the window and not in fullscreen, resize it (initial scene size)*/
	xWindow->w_width = evt->size.width;
	xWindow->w_height = evt->size.height;
	  if (!xWindow->fullscreen) {
	    if (xWindow->par_wnd) {
	      XWindowAttributes pwa;
	      XGetWindowAttributes(xWindow->display, xWindow->par_wnd, &pwa);
	      XMoveResizeWindow(xWindow->display, xWindow->wnd, pwa.x, pwa.y, evt->size.width, evt->size.height);
	      if (!xWindow->no_select_input) XSetInputFocus(xWindow->display, xWindow->wnd, RevertToNone, CurrentTime);
	    } else {
		XResizeWindow (xWindow->display, xWindow->wnd, evt->size.width, evt->size.height);
	    }
	  }
	  break;
	case GF_EVENT_VIDEO_SETUP:
		/*and resetup HW*/
#ifdef GPAC_HAS_OPENGL
		if (xWindow->is_3D_out) return X11_SetupGL(vout);
#endif
		return X11_ResizeBackBuffer(vout, evt->size.width, evt->size.height);
	}
	} else {
	  X11_HandleEvents(vout);
	}
	return GF_OK;

}

/* switch from/to full screen mode */
GF_Err X11_SetFullScreen (struct _video_out * vout, u32 bFullScreenOn, u32 * screen_width, u32 * screen_height)
{
	X11VID ();
	xWindow->fullscreen = bFullScreenOn;
#ifdef GPAC_HAS_OPENGL
	if (xWindow->is_3D_out) X11_ReleaseGL(xWindow);
#endif

	if (bFullScreenOn) {
		xWindow->store_width = *screen_width;
		xWindow->store_height = *screen_height;

		xWindow->w_width = vout->max_screen_width;
		xWindow->w_height = vout->max_screen_height;

		XFreeGC (xWindow->display, xWindow->the_gc);
		xWindow->the_gc = XCreateGC (xWindow->display, xWindow->full_wnd, 0, NULL);

		XMoveResizeWindow (xWindow->display,
				   (Window) xWindow->full_wnd, 0, 0,
				   vout->max_screen_width,
				   vout->max_screen_height);
		*screen_width = xWindow->w_width;
		*screen_height = xWindow->w_height;
		XUnmapWindow (xWindow->display, xWindow->wnd);
		XMapWindow (xWindow->display, xWindow->full_wnd);
		XSetInputFocus(xWindow->display, xWindow->full_wnd, RevertToNone, CurrentTime);
		XRaiseWindow(xWindow->display, xWindow->full_wnd);
		XGrabKeyboard(xWindow->display, xWindow->full_wnd, True, GrabModeAsync, GrabModeAsync, CurrentTime);
	} else {
		*screen_width = xWindow->store_width;
		*screen_height = xWindow->store_height;
		XFreeGC (xWindow->display, xWindow->the_gc);
		xWindow->the_gc = XCreateGC (xWindow->display, xWindow->wnd, 0, NULL);
		XUnmapWindow (xWindow->display, xWindow->full_wnd);
		XMapWindow (xWindow->display, xWindow->wnd);
		XUngrabKeyboard(xWindow->display, CurrentTime);
		if (xWindow->par_wnd) XSetInputFocus(xWindow->display, xWindow->wnd, RevertToNone, CurrentTime);
		/*backbuffer resize will be done right after this is called */
	}
#ifdef GPAC_HAS_OPENGL
	if (xWindow->is_3D_out) X11_SetupGL(vout);
#endif
	return GF_OK;
}

/*
 * lock video mem
 */
GF_Err X11_LockBackBuffer(struct _video_out * vout, GF_VideoSurface * vi, u32 do_lock)
{
	X11VID ();

	if (do_lock) {
		if (!vi) return GF_BAD_PARAM;
		vi->width = xWindow->back_buffer->width;
		vi->height = xWindow->back_buffer->height;
		vi->pitch = xWindow->back_buffer->pitch;
		vi->pixel_format = xWindow->pixel_format;
		vi->video_buffer = xWindow->back_buffer->buffer;
		vi->is_hardware_memory = (xWindow->videoaccesstype==VIDEO_XI_STANDARD) ? 0 : 1;
		return GF_OK;
	} else {
		return GF_OK;
	}
}


static XErrorHandler old_handler = NULL;
static int selectinput_err = 0;
static int X11_BadAccess_ByPass(Display * display,
                                        XErrorEvent * event)
{
	char msg[60];
	if (event->error_code == BadAccess)
	{
		selectinput_err = 1;
		return 0;
	}
	if (old_handler != NULL)
		old_handler(display, event);
	else
	{
		XGetErrorText(display, event->error_code, (char *) &msg, 60);
		GF_LOG(GF_LOG_ERROR, GF_LOG_MMIO, ("[X11] Error %s\n",msg));
	}
	return 0;
}

/*
 * Setup X11 wnd System
 */
void
X11_SetupWindow (GF_VideoOutput * vout)
{
	X11VID ();
	const char *sOpt;

	xWindow->display = XOpenDisplay (NULL);
	xWindow->screennum = DefaultScreen (xWindow->display);
	xWindow->screenptr = DefaultScreenOfDisplay (xWindow->display);
	xWindow->visual = DefaultVisualOfScreen (xWindow->screenptr);
	xWindow->depth = DefaultDepth (xWindow->display, xWindow->screennum);

	switch (xWindow->depth) {
	case 8:
		xWindow->pixel_format = GF_PIXEL_GREYSCALE;
		break;
	case 16:
		xWindow->pixel_format = GF_PIXEL_RGB_565;
		break;
	case 24:
		xWindow->pixel_format = GF_PIXEL_RGB_32;
		break;
	default:
		xWindow->pixel_format = GF_PIXEL_GREYSCALE;
		break;
	}
	xWindow->bpp = xWindow->depth / 8;
	xWindow->bpp = xWindow->bpp == 3 ? 4 : xWindow->bpp;

	vout->max_screen_width = DisplayWidth(xWindow->display, xWindow->screennum);
	vout->max_screen_height = DisplayHeight(xWindow->display, xWindow->screennum);

	/*
	 * Full screen wnd
	 */
	xWindow->full_wnd = XCreateWindow (xWindow->display,
								   RootWindowOfScreen (xWindow->screenptr),
								   0, 0,
								   vout->max_screen_width,
								   vout->max_screen_height, 0,
								   xWindow->depth, InputOutput,
								   xWindow->visual, 0, NULL);

	XSelectInput(xWindow->display, xWindow->full_wnd,
					FocusChangeMask | ExposureMask | PointerMotionMask | ButtonReleaseMask | ButtonPressMask | KeyPressMask | KeyReleaseMask);

	if (!xWindow->par_wnd) {
		xWindow->w_width = 320;
		xWindow->w_height = 20;
		xWindow->wnd = XCreateWindow (xWindow->display,
					   RootWindowOfScreen(xWindow->screenptr), 0, 0,
					   xWindow->w_width, xWindow->w_height, 0,
					   xWindow->depth, InputOutput,
					   xWindow->visual, 0, NULL);
		XMapWindow (xWindow->display, (Window) xWindow->wnd);
	} else {
		XWindowAttributes pwa;
		XGetWindowAttributes(xWindow->display, xWindow->par_wnd, &pwa);
		xWindow->w_width = pwa.width;
		xWindow->w_height = pwa.height;
		xWindow->wnd = XCreateWindow (xWindow->display, xWindow->par_wnd, pwa.x, pwa.y,
					xWindow->w_width, xWindow->w_height, 0,
					   xWindow->depth, InputOutput,
					   xWindow->visual, 0, NULL);
		XMapWindow (xWindow->display, (Window) xWindow->wnd);
	}

	XSync(xWindow->display, False);
	XUnmapWindow (xWindow->display, (Window) xWindow->wnd);
	XSync(xWindow->display, False);
	old_handler = XSetErrorHandler(X11_BadAccess_ByPass);
	selectinput_err = 0;
	XSelectInput(xWindow->display, xWindow->wnd,
		FocusChangeMask | StructureNotifyMask | PropertyChangeMask | ExposureMask |
		PointerMotionMask | ButtonReleaseMask | ButtonPressMask |
		KeyPressMask | KeyReleaseMask);
	XSync(xWindow->display, False);
	XSetErrorHandler(old_handler);
	if (selectinput_err) {
       	XSelectInput(xWindow->display, xWindow->wnd,
			StructureNotifyMask | PropertyChangeMask | ExposureMask |
			KeyPressMask | KeyReleaseMask);

		GF_LOG(GF_LOG_ERROR, GF_LOG_MMIO, ("[X11] Cannot select input focus\n"));
	}
	XSync(xWindow->display, False);
	XMapWindow (xWindow->display, (Window) xWindow->wnd);

	XSizeHints *Hints = XAllocSizeHints ();
	Hints->flags = PSize | PMinSize;
	Hints->min_width = 32;
	Hints->min_height = 32;
	Hints->max_height = 4096;
	Hints->max_width = 4096;
	if (!xWindow->par_wnd) {
		XSetWMNormalHints (xWindow->display, xWindow->wnd, Hints);
		XStoreName (xWindow->display, xWindow->wnd, "GPAC X11 Output");
	}
	Hints->x = 0;
	Hints->y = 0;
	Hints->flags |= USPosition;
	XSetWMNormalHints (xWindow->display, xWindow->full_wnd, Hints);
	XFree (Hints);


	xWindow->the_gc = XCreateGC (xWindow->display, xWindow->wnd, 0, NULL);
	xWindow->videoaccesstype = VIDEO_XI_STANDARD;

#ifdef GPAC_HAS_X11_SHM
	sOpt = gf_modules_get_option((GF_BaseInterface *)vout, "Video", "UseHardwareMemory");
        if (sOpt && !strcmp(sOpt, "yes")) {
	  int XShmMajor, XShmMinor;
	  Bool XShmPixmaps;
	  if (XShmQueryVersion(xWindow->display, &XShmMajor, &XShmMinor, &XShmPixmaps)) {
	    /*this is disabled due to flip pb (we cannot reposition backbuffer)*/
	    if (0 && XShmPixmaps && (XShmPixmapFormat(xWindow->display) == ZPixmap)) {
		xWindow->videoaccesstype = VIDEO_XI_SHMPIXMAP;
	    } else {
	      xWindow->videoaccesstype = VIDEO_XI_SHMSTD;
	      GF_LOG(GF_LOG_INFO, GF_LOG_MMIO, ("[X11] Using X11 Hardware Blit\n"));
	    }
	  }
	}
#endif

	GF_SAFEALLOC(xWindow->back_buffer, X11WrapSurface);
	xWindow->back_buffer->id = -1;


	XSetWindowAttributes xsw;
	xsw.border_pixel = WhitePixel (xWindow->display, xWindow->screennum);
	xsw.background_pixel = BlackPixel (xWindow->display, xWindow->screennum);
	xsw.win_gravity = NorthWestGravity;
	XChangeWindowAttributes (xWindow->display, xWindow->wnd, CWBackPixel | CWWinGravity, &xsw);

	xsw.override_redirect = True;
	XChangeWindowAttributes(xWindow->display, xWindow->full_wnd,
				CWOverrideRedirect | CWBackPixel | CWBorderPixel | CWWinGravity, &xsw);

	if (!xWindow->par_wnd) {
		xWindow->WM_DELETE_WINDOW = XInternAtom (xWindow->display, "WM_DELETE_WINDOW", False);
		XSetWMProtocols(xWindow->display, xWindow->wnd, &xWindow->WM_DELETE_WINDOW, 1);
	}

	{
		XEvent ev;
		long mask;

		memset (&ev, 0, sizeof (ev));
		ev.xclient.type = ClientMessage;
		ev.xclient.window = RootWindowOfScreen (xWindow->screenptr);
		ev.xclient.message_type = XInternAtom (xWindow->display, "KWM_KEEP_ON_TOP", False);
		ev.xclient.format = 32;
		ev.xclient.data.l[0] = xWindow->full_wnd;
		ev.xclient.data.l[1] = CurrentTime;
		mask = SubstructureRedirectMask;
		XSendEvent (xWindow->display,RootWindowOfScreen (xWindow->screenptr), False,
			    mask, &ev);
	}
#ifdef GPAC_HAS_OPENGL
	if (xWindow->is_3D_out) {
	  int attribs[64];
	  int i;

	  i=0;
	  attribs[i++] = GLX_RGBA;
	  attribs[i++] = GLX_RED_SIZE;
	  attribs[i++] = 5;
	  attribs[i++] = GLX_GREEN_SIZE;
	  attribs[i++] = 5;
	  attribs[i++] = GLX_BLUE_SIZE;
	  attribs[i++] = 5;
	  attribs[i++] = GLX_DEPTH_SIZE;
	  attribs[i++] = 16;
	  if (xWindow->gl_cfg.double_buffered) attribs[i++] = GLX_DOUBLEBUFFER;
	  attribs[i++] = None;
	  xWindow->glx_visualinfo = glXChooseVisual(xWindow->display, xWindow->screennum, attribs);
	  if (!xWindow->glx_visualinfo) {
		  GF_LOG(GF_LOG_ERROR, GF_LOG_MMIO, ("[X11] Error selecting GL display\n"));
	  }
	}
#endif
	xWindow->setup_done = 1;
}

GF_Err X11_Setup(struct _video_out *vout, void *os_handle, void *os_display, u32 no_proc_override, GF_GLConfig * cfg)
{
	X11VID ();
	/*assign window if any, NEVER display*/
	xWindow->par_wnd = (Window) os_handle;
	/*OSMOZILLA HACK*/
	if (os_display) xWindow->no_select_input = 1;
	if (cfg) {
#ifdef GPAC_HAS_OPENGL
		xWindow->is_3D_out = 1;
	  xWindow->gl_cfg = *cfg;
#else
	  return GF_NOT_SUPPORTED;
#endif
	} else {
	  xWindow->is_3D_out = 0;
	}
	/*the rest is done THROUGH THE MAIN RENDERER TRHEAD!!*/
	return GF_OK;
}

/* Shutdown X11 */
void X11_Shutdown (struct _video_out *vout)
{
	X11VID ();

	if (xWindow->is_3D_out) {
#ifdef GPAC_HAS_OPENGL
		X11_ReleaseGL(xWindow);
#endif
	} else {
		X11_ReleaseBackBuffer (vout);
	}
	free(xWindow->back_buffer);
	XFreeGC (xWindow->display, xWindow->the_gc);
	XUnmapWindow (xWindow->display, (Window) xWindow->wnd);
	XDestroyWindow (xWindow->display, (Window) xWindow->wnd);
	XDestroyWindow (xWindow->display, (Window) xWindow->full_wnd);
	XCloseDisplay (xWindow->display);
	free (xWindow);
}


void *NewX11VideoOutput ()
{
	GF_VideoOutput *driv;
	XWindow *xWindow;
	GF_SAFEALLOC(driv, GF_VideoOutput);
	if (!driv) return NULL;
	GF_SAFEALLOC(xWindow, XWindow);
	if (!xWindow) {
		free(driv);
		return NULL;
	}
	GF_REGISTER_MODULE_INTERFACE(driv, GF_VIDEO_OUTPUT_INTERFACE, "X11 Video Output", "gpac distribution")

	driv->opaque = xWindow;

	driv->Flush = X11_Flush;
	driv->SetFullScreen = X11_SetFullScreen;
	driv->Setup = X11_Setup;
	driv->Shutdown = X11_Shutdown;
	driv->LockBackBuffer = X11_LockBackBuffer;
	driv->ProcessEvent = X11_ProcessEvent;
	driv->hw_caps = GF_VIDEO_HW_HAS_OPENGL;
	return (void *) driv;

}


void
DeleteX11VideoOutput (GF_VideoOutput * vout)
{
	free (vout);
}

/*
 * interface query
 */
Bool
QueryInterface (u32 InterfaceType)
{
	if (InterfaceType == GF_VIDEO_OUTPUT_INTERFACE)
		return 1;
	return 0;
}


/*
 * interface create
 */
GF_BaseInterface *
LoadInterface (u32 InterfaceType)
{
	if (InterfaceType == GF_VIDEO_OUTPUT_INTERFACE)
		return (GF_BaseInterface *) NewX11VideoOutput ();
	return NULL;
}


/*
 * interface destroy
 */
void
ShutdownInterface (GF_BaseInterface *ifce)
{
	switch (ifce->InterfaceType)
	{
	case GF_VIDEO_OUTPUT_INTERFACE:
		DeleteX11VideoOutput ((GF_VideoOutput *)ifce);
		break;
	}
}
