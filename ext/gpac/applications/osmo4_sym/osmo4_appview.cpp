/*
 *			GPAC - Multimedia Framework C SDK
 *
 *			Copyright (c) ENST 2006-200X
 *				Authors: Jean Le Feuvre 
 *					All rights reserved
 *
 *  This file is part of GPAC / Symbian GUI player
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


// INCLUDE FILES
#include <coemain.h>
#include <eikenv.h>

#include "osmo4_AppView.h"
#include "osmo4_appui.h"

#include <gpac/options.h>
/*for initial setup*/
#include <gpac/modules/service.h>

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// Cosmo4AppView::NewL()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
COsmo4AppView* COsmo4AppView::NewL( const TRect& aRect )
{
    COsmo4AppView* self = COsmo4AppView::NewLC( aRect );
    CleanupStack::Pop( self );
    return self;
}

// -----------------------------------------------------------------------------
// COsmo4AppView::NewLC()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
COsmo4AppView* COsmo4AppView::NewLC( const TRect& aRect )
{
    COsmo4AppView* self = new ( ELeave ) COsmo4AppView;
    CleanupStack::PushL( self );
    self->ConstructL( aRect );
    return self;
}



// -----------------------------------------------------------------------------
// COsmo4AppView::COsmo4AppView()
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
COsmo4AppView::COsmo4AppView()
{
    // No implementation required
	m_pTimer = NULL;
	memset(&m_user, 0, sizeof(GF_User));
	m_term = NULL;
	m_mx = NULL;
	last_title_update = 0;
	show_rti = 0;
	memset(&m_rti, 0, sizeof(GF_SystemRTInfo));
	selector = NULL;
	target = NULL;
}



// -----------------------------------------------------------------------------
// COsmo4AppView::~COsmo4AppView()
// Destructor.
// -----------------------------------------------------------------------------
//
COsmo4AppView::~COsmo4AppView()
{
	Shutdown();
	if (m_mx) gf_mx_del(m_mx);
	if (selector) delete selector;
	//if (target) delete target;
}

void COsmo4AppView::Shutdown()
{
//	MessageBox("Osmo4 shutdown request", "");
	if (m_pTimer) {
		m_pTimer->Cancel();
		delete m_pTimer;
		m_pTimer = NULL;
	}
	if (m_term) {
		GF_Terminal *t = m_term;
		m_term = NULL;
		gf_term_del(t);
	}
    if (m_user.config) {
		gf_cfg_del(m_user.config);
		m_user.config = NULL;
	}
	if (m_user.modules) {
		gf_modules_del(m_user.modules);
		m_user.modules = NULL;
	}
//	MessageBox("Osmo4 shutdown OK", "");
}

void COsmo4AppView::MessageBox(const char *text, const char *title)
{
	HBufC *msg1, *msg2;
	TInt length = User::StringLength( ( TUint8* ) text) + 1;
	msg1 = HBufC::NewL( length );
	msg1->Des().Copy( TPtrC8(( TText8* ) text) );
	
	length = User::StringLength( ( TUint8* ) title) + 1;
	msg2 = HBufC::NewL( length );
	msg2->Des().Copy( TPtrC8(( TText8* ) title) );
	
	CEikonEnv::Static()->InfoWinL(*msg2, *msg1);
	delete msg1;
	delete msg2;
}

TInt myTick(TAny* aObject)
{
	return ((COsmo4AppView*)aObject)->OnTick();
}

TInt COsmo4AppView::OnTick()
{
	if (m_term) gf_term_process_step(m_term);

	/*check RTI display*/
	if (show_rti && gf_sys_get_rti(500, &m_rti, 0)) DisplayRTI();
	/*never stop...*/
	return 1;
}

void COsmo4AppView::DisplayRTI()
{
	COsmo4AppUi *app = (COsmo4AppUi *) CEikonEnv::Static()->AppUi();
	char szInfo[20];
	sprintf(szInfo, "CPU %02d FPS %02.2f", m_rti.process_cpu_usage, gf_term_get_framerate(m_term, 0));
	app->SetInfo(szInfo);
}


//GPAC log function
static void on_gpac_log(void *cbk, u32 ll, u32 lm, const char *fmt, va_list list)
{
	char szMsg[2048];
	COsmo4AppView *app = (COsmo4AppView *)cbk;

	gf_mx_p(app->m_mx);
	if (app->do_log) {
		FILE *logs = fopen("\\data\\gpac_logs.txt", "a+t");
		if (logs) {
			vfprintf(logs, fmt, list);
			fclose(logs);
		}
	} else {
		vsprintf(szMsg, fmt, list);
		app->MessageBox(szMsg, "Error:");
	}
	gf_mx_v(app->m_mx);
}

static Bool GPAC_EventProc(void *ptr, GF_Event *evt)
{
	COsmo4AppView *app = (COsmo4AppView *)ptr;
	return app->EventProc(evt);
}

Bool COsmo4AppView::EventProc(GF_Event *evt)
{
	TRect r;

	switch (evt->type) {
	case GF_EVENT_MESSAGE:
		if (!evt->message.message) return 0;
		if (evt->message.error) {
			char err[1024];
			sprintf(err, "Error: %s", gf_error_to_string(evt->message.error));
			MessageBox(evt->message.message, err);
		} else {
			MessageBox(evt->message.message, "Info");
		}
		break;
	case GF_EVENT_SCENE_SIZE:
		r = Rect();
		gf_term_set_size(m_term, r.Width(), r.Height());
		break;
	}
	return 0;
}

void COsmo4AppView::SetupLogs()
{
	const char *opt;

	gf_mx_p(m_mx);
	if (do_log) {
		gf_log_set_level(0);
		do_log = 0;
	}
	/*setup GPAC logs: log all errors*/
	opt = gf_cfg_get_key(m_user.config, "General", "LogLevel");
	if ((opt && !stricmp(opt, "debug")) /*|| 1*/) { 
		FILE *logs = fopen("\\data\\gpac_logs.txt", "wt");
		if (!logs) {
			MessageBox("Cannot open log file - disabling logs", "Warning !");
		} else {
			MessageBox("Debug log enabled in \\data\\gpac_logs.txt", "Info");
			fclose(logs);
			do_log = 1;
			gf_log_set_level(GF_LOG_DEBUG);
			gf_log_set_tools(0xFFFFFFFF);
		}
	}
	if (!do_log) {
		gf_log_set_level(GF_LOG_ERROR);
		gf_log_set_tools(0xFFFFFFFF);
	}

	gf_log_set_callback(this, on_gpac_log);
	gf_mx_v(m_mx);

	GF_LOG(GF_LOG_DEBUG, GF_LOG_CORE, ("Osmo4 logs initialized\n"));
}


static void Osmo4_progress_cbk(void *usr, char *title, u32 done, u32 total)
{
	COsmo4AppView *view = (COsmo4AppView *) usr;
	COsmo4AppUi *app = (COsmo4AppUi *) CEikonEnv::Static()->AppUi();

	if (done==total) {
		app->SetInfo(NULL);
	} else if (view->last_title_update + 500 < gf_sys_clock()) {
		char szName[1024];
		view->last_title_update = gf_sys_clock();
		sprintf(szName, "%s %02d %%", title, (done*100 / total) );
		app->SetInfo(szName);
	}

}

// -----------------------------------------------------------------------------
// COsmo4AppView::ConstructL()
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void COsmo4AppView::ConstructL( const TRect& aRect )
{
	const char *opt;
	Bool first_launch = 0;

	selector = CRemConInterfaceSelector::NewL();
	target = CRemConCoreApiTarget::NewL(*selector, *this);
	selector->OpenTargetL();

    // Create a window for this application view
    CreateWindowL();
    // Set the windows size
    SetRect( aRect );
	//draw
    ActivateL();
	m_mx = gf_mx_new();

	m_window = Window();
	m_session = CEikonEnv::Static()->WsSession();

	//load config file
	m_user.config = gf_cfg_new("\\private\\F01F9075\\", "GPAC.cfg");
	if (!m_user.config) {
		first_launch = 1;
		FILE *ft = fopen("\\private\\F01F9075\\GPAC.cfg", "wt");
		if (!ft) {
			MessageBox("Cannot create GPAC Config file", "Fatal Error");
			User::Leave(KErrGeneral);
		} else {
			fclose(ft);
		}
		m_user.config = gf_cfg_new("\\private\\F01F9075\\", "GPAC.cfg");
		if (!m_user.config) {
			MessageBox("GPAC Configuration file not found", "Fatal Error");
			User::Leave(KErrGeneral);
		}
	}

	SetupLogs();
	gf_set_progress_callback(this, Osmo4_progress_cbk);

	opt = gf_cfg_get_key(m_user.config, "General", "ModulesDirectory");
	if (!opt) first_launch = 2;
	
	if (first_launch) {
		/*hardcode module directory*/
		gf_cfg_set_key(m_user.config, "General", "ModulesDirectory", "\\sys\\bin");
		/*hardcode cache directory*/
		gf_cfg_set_key(m_user.config, "General", "CacheDirectory", "\\private\\F01F9075\\cache");
		gf_cfg_set_key(m_user.config, "Downloader", "CleanCache", "yes");
		/*startup file*/
		//gf_cfg_set_key(m_user.config, "General", "StartupFile", "\\private\\F01F9075\\gpac.mp4");
		/*setup UDP traffic autodetect*/
		gf_cfg_set_key(m_user.config, "Network", "AutoReconfigUDP", "yes");
		gf_cfg_set_key(m_user.config, "Network", "UDPNotAvailable", "no");
		gf_cfg_set_key(m_user.config, "Network", "UDPTimeout", "10000");
		gf_cfg_set_key(m_user.config, "Network", "BufferLength", "3000");
		
		gf_cfg_set_key(m_user.config, "Rendering", "RendererName", "GPAC 2D Renderer");

		gf_cfg_set_key(m_user.config, "FontEngine", "TextureTextMode", "3D");

		
		/*save cfg and reload*/
		gf_cfg_del(m_user.config);
		m_user.config = gf_cfg_new("\\private\\F01F9075\\", "GPAC.cfg");
		if (!m_user.config) {
			MessageBox("Cannot save initial GPAC Config file", "Fatal Error");
			User::Leave(KErrGeneral);
		}


		MessageBox("Osmo4", "Thank you for Installing");
	}

	/*load modules*/
	opt = gf_cfg_get_key(m_user.config, "General", "ModulesDirectory");
	m_user.modules = gf_modules_new(opt, m_user.config);
	if (!m_user.modules || !gf_modules_get_count(m_user.modules)) {
		MessageBox(m_user.modules ? "No modules available" : "Cannot create module manager", "Fatal Error");
		if (m_user.modules) gf_modules_del(m_user.modules);
		gf_cfg_del(m_user.config);
		User::Leave(KErrGeneral);
	}

	if (first_launch) {
		/*first launch, register all files ext*/
		for (u32 i=0; i<gf_modules_get_count(m_user.modules); i++) {
			GF_InputService *ifce = (GF_InputService *) gf_modules_load_interface(m_user.modules, i, GF_NET_CLIENT_INTERFACE);
			if (!ifce) continue;
			if (ifce) {
				ifce->CanHandleURL(ifce, "test.test");
				gf_modules_close_interface((GF_BaseInterface *)ifce);
			}
		}
	}

	/*we don't thread the terminal, ie appart from the audio renderer, media decoding and visual rendering is 
	handled by the app process*/
	m_user.init_flags = GF_TERM_NO_VISUAL_THREAD | GF_TERM_NO_REGULATION;
	m_user.EventProc = GPAC_EventProc;
	m_user.opaque = this;
	m_user.os_window_handler = (void *) &m_window;
	m_user.os_display = (void *) &m_session;

	m_term = gf_term_new(&m_user);
	if (!m_term) {
		MessageBox("Cannot load GPAC terminal", "Fatal Error");
		gf_modules_del(m_user.modules);
		gf_cfg_del(m_user.config);
		User::Leave(KErrGeneral);
	}
	//MessageBox("GPAC terminal loaded", "Success !");

	/*ok set output size*/
	TSize s = m_window.Size();
	gf_term_set_size(m_term, s.iWidth, s.iHeight);


	/*start our callback (every ms)*/
	const TInt KTickInterval = 33000;
	m_pTimer = CPeriodic::NewL(CActive::EPriorityStandard);
	m_pTimer->Start(KTickInterval, KTickInterval, TCallBack(myTick, this));

	opt = gf_cfg_get_key(m_user.config, "General", "StartupFile");
	if (opt) gf_term_connect(m_term, opt);
}


// -----------------------------------------------------------------------------
// COsmo4AppView::Draw()
// Draws the display.
// -----------------------------------------------------------------------------
//
void COsmo4AppView::Draw( const TRect& /*aRect*/ ) const
{
	if (!m_term) {
		CWindowGc& gc = SystemGc();
		TRgb black(0,0,0);
		gc.SetBrushColor(black);
		gc.Clear();
	} else {
		/*FIXME - this is just to force a screen flush, needs rework*/
		gf_term_set_option(m_term, GF_OPT_FREEZE_DISPLAY, 0);
	}
}

void COsmo4AppView::ShowHide(Bool show)
{
	if (show) {
		MakeVisible(ETrue);
		if (m_term) {
			gf_term_set_option(m_term, GF_OPT_VISIBLE, 1);
			DrawDeferred();
		}
	} else {
		MakeVisible(EFalse);
		if (m_term) gf_term_set_option(m_term, GF_OPT_VISIBLE, 0);
	}
}

// -----------------------------------------------------------------------------
// COsmo4AppView::SizeChanged()
// Called by framework when the view size is changed.
// -----------------------------------------------------------------------------
//
void COsmo4AppView::SizeChanged()
{  
	if (m_term) {
		TSize s = m_window.Size();
		gf_term_set_size(m_term, s.iWidth, s.iHeight);
	}
	DrawNow();
}

void COsmo4AppView::Connect(const char *url)
{
	char the_url[1024];
	/*copy before removing from recent files*/
	strcpy(the_url, url);
	gf_cfg_set_key(m_user.config, "RecentFiles", the_url, NULL);
	gf_cfg_insert_key(m_user.config, "RecentFiles", the_url, "", 0);
	u32 count = gf_cfg_get_key_count(m_user.config, "RecentFiles");
	if (count > 10) gf_cfg_set_key(m_user.config, "RecentFiles", gf_cfg_get_key_name(m_user.config, "RecentFiles", count-1), NULL);

	if (m_term) gf_term_connect(m_term, the_url);
}


TKeyResponse COsmo4AppView::OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType)
{
	GF_Event evt;
	u32 ret;

	evt.key.hw_code = aKeyEvent.iScanCode;
	evt.key.flags = 0;
	switch (aType) {
	case EEventKeyUp:
		evt.type = GF_EVENT_KEYUP;
		break;
	case EEventKeyDown:
	case EEventKey:
		evt.type = GF_EVENT_KEYDOWN;
		break;
	default:
		return EKeyWasNotConsumed;
	}

	switch (aKeyEvent.iCode) {
	case EKeyLeftArrow: evt.key.key_code = GF_KEY_LEFT; break;
	case EKeyRightArrow: evt.key.key_code = GF_KEY_RIGHT; break;
	case EKeyUpArrow: evt.key.key_code = GF_KEY_UP; break;
	case EKeyDownArrow: evt.key.key_code = GF_KEY_DOWN; break;
	case EKeyIncVolume: evt.key.key_code = GF_KEY_VOLUMEUP; break;
	case EKeyDecVolume: evt.key.key_code = GF_KEY_VOLUMEDOWN; break;
	default:
		switch (aKeyEvent.iScanCode) {
		case EStdKeyIncVolume: evt.key.key_code = GF_KEY_VOLUMEUP; break;
		case EStdKeyDecVolume: evt.key.key_code = GF_KEY_VOLUMEDOWN; break;
		default:
			return EKeyWasNotConsumed;
		}
	}
	ret = gf_term_user_event(m_term, &evt);
	/*generate a key up*/
	if (aType==EEventKey) {
		evt.type = GF_EVENT_KEYUP;
		ret += gf_term_user_event(m_term, &evt);
	}
	return ret ? EKeyWasConsumed : EKeyWasNotConsumed;
}

void COsmo4AppView::MrccatoCommand(TRemConCoreApiOperationId aOperationId, TRemConCoreApiButtonAction aButtonAct)
{
	GF_Event e;
	switch (aOperationId) {
/*
    TRequestStatus status;
	case ERemConCoreApiPausePlayFunction:
	case ERemConCoreApiStop:
	case ERemConCoreApiRewind:
	case ERemConCoreApiForward:
	case ERemConCoreApiFastForward:
	case ERemConCoreApiBackward:
		switch (aButtonAct) {
		case ERemConCoreApiButtonPress:
			break;
		case ERemConCoreApiButtonRelease:
			break;
		case ERemConCoreApiButtonClick:
			break;
		default:
			break;
		}
*/
	case ERemConCoreApiVolumeUp:
	case ERemConCoreApiVolumeDown:
		e.key.hw_code = 0;
		e.key.flags = 0;
		e.key.key_code = (aOperationId==ERemConCoreApiVolumeUp) ? GF_KEY_VOLUMEUP : GF_KEY_VOLUMEDOWN;
		switch (aButtonAct) {
		case ERemConCoreApiButtonPress: 
			e.type = GF_EVENT_KEYDOWN; 
			gf_term_user_event(m_term, &e);
			break;
		case ERemConCoreApiButtonRelease: 
			e.type = GF_EVENT_KEYUP; 
			gf_term_user_event(m_term, &e);
			break;
		default: 
			e.type = GF_EVENT_KEYDOWN; 
			gf_term_user_event(m_term, &e);
			e.type = GF_EVENT_KEYUP; 
			gf_term_user_event(m_term, &e);
			break;
		}
		break;
	default:
		break;
	}
}

void COsmo4AppView::ReloadTerminal()
{
	COsmo4AppUi *app = (COsmo4AppUi *) CEikonEnv::Static()->AppUi();
	GF_Terminal *term = m_term;

	app->SetInfo("Reloading terminal...");
	m_term = NULL;
	gf_term_del(term);
	m_term = gf_term_new(&m_user);
	app->SetInfo(NULL);

	if (!m_term) {
		MessageBox("Couldn't reload terminal", "Fatal Error !");
		Panic(EOsmo4Ui);
	}
}
