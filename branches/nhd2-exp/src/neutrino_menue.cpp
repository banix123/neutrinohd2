/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: neutrino_menu.cpp 2013/10/12 11:23:30 mohousch Exp $

	Copyright (C) 2001 Steffen Hehn 'McClean' and some other guys
	Homepage: http://dbox.cyberphoria.org/

	Kommentar:

	Diese GUI wurde von Grund auf neu programmiert und sollte nun vom
	Aufbau und auch den Ausbaumoeglichkeiten gut aussehen. Neutrino basiert
	auf der Client-Server Idee, diese GUI ist also von der direkten DBox-
	Steuerung getrennt. Diese wird dann von Daemons uebernommen.


	License: GPL

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <dlfcn.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/statvfs.h>
#include <sys/vfs.h>

#include <sys/socket.h>

#include <iostream>
#include <fstream>
#include <string>

#include <global.h>
#include <neutrino.h>

#include <daemonc/remotecontrol.h>

#include <driver/encoding.h>
#include <driver/framebuffer.h>
#include <driver/fontrenderer.h>
#include <driver/rcinput.h>
#include <driver/stream2file.h>
#include <driver/vcrcontrol.h>
#include <driver/shutdown_count.h>
#include <driver/screen_max.h>

#include <gui/epgplus.h>
#include <gui/streaminfo2.h>

#include <gui/widget/colorchooser.h>
#include <gui/widget/menue.h>
#include <gui/widget/messagebox.h>
#include <gui/widget/hintbox.h>
#include <gui/widget/icons.h>
#include <gui/widget/vfdcontroler.h>
#include <gui/widget/keychooser.h>
#include <gui/widget/stringinput.h>
#include <gui/widget/stringinput_ext.h>
#include <gui/widget/mountchooser.h>

#include <gui/color.h>

#include <gui/bedit/bouqueteditor_bouquets.h>
#include <gui/bouquetlist.h>
#include <gui/eventlist.h>
#include <gui/channellist.h>
#include <gui/screensetup.h>
#include <gui/pluginlist.h>
#include <gui/plugins.h>
#include <gui/infoviewer.h>
#include <gui/epgview.h>
#include <gui/epg_menu.h>
#include <gui/update.h>
#include <gui/scan.h>
#include <gui/sleeptimer.h>
#include <gui/rc_lock.h>
#include <gui/timerlist.h>
#include <gui/alphasetup.h>
#include <gui/audioplayer.h>
#include <gui/imageinfo.h>
#include <gui/movieplayer.h>
#include <gui/nfs.h>
#include <gui/pictureviewer.h>
#include <gui/motorcontrol.h>
#include <gui/filebrowser.h>
#include <gui/psisetup.h>

#include <system/setting_helpers.h>
#include <system/settings.h>
#include <system/debug.h>
#include <system/flashtool.h>
#include <system/fsmounter.h>

#include <timerdclient/timerdmsg.h>

#include <video_cs.h>
#include <audio_cs.h>

#include <xmlinterface.h>

#include <string.h>

#include <gui/dboxinfo.h>
#include <gui/hdd_menu.h>
#include <gui/audio_select.h>

#if !defined (PLATFORM_COOLSTREAM)
#include <gui/cam_menu.h>
#endif

#include <gui/scan_setup.h>
#include <gui/zapit_setup.h>

/*zapit includes*/
#include <getservices.h>
#include <satconfig.h>
#include <client/zapitclient.h>
#include <frontend_c.h>

#include <gui/proxyserver_setup.h>
#include <gui/opkg_manager.h>
#include <gui/themes.h>
#include <gui/webtv.h>
#include <gui/upnpbrowser.h>


extern CMoviePlayerGui * moviePlayerGui;	// defined in neutrino.cpp
extern CPlugins * g_PluginList;		// defined in neutrino.cpp
extern bool parentallocked;			// defined neutrino.cpp
extern CRemoteControl * g_RemoteControl;	// defined neutrino.cpp
#if !defined (PLATFORM_COOLSTREAM)
extern CCAMMenuHandler * g_CamHandler;		// defined neutrino.cpp
#endif

static CTimingSettingsNotifier timingsettingsnotifier;

extern int FrontendCount;			// defined in zapit.cpp
CFrontend * getFE(int index);

extern Zapit_config zapitCfg;			//defined in neutrino.cpp
//void setZapitConfig(Zapit_config * Cfg);
void getZapitConfig(Zapit_config *Cfg);

extern char recDir[255];			// defined in neutrino.cpp

// webtv
extern CWebTV * webtv;

// option off0_on1
#define OPTIONS_OFF0_ON1_OPTION_COUNT 2
const CMenuOptionChooser::keyval OPTIONS_OFF0_ON1_OPTIONS[OPTIONS_OFF0_ON1_OPTION_COUNT] =
{
        { 0, LOCALE_OPTIONS_OFF, NULL },
        { 1, LOCALE_OPTIONS_ON, NULL }
};

// option off1 on0
#define OPTIONS_OFF1_ON0_OPTION_COUNT 2
const CMenuOptionChooser::keyval OPTIONS_OFF1_ON0_OPTIONS[OPTIONS_OFF1_ON0_OPTION_COUNT] =
{
        { 1, LOCALE_OPTIONS_OFF, NULL },
        { 0, LOCALE_OPTIONS_ON, NULL }
};

// msg yes_no needed from audioplayer/filebrowser
#define MESSAGEBOX_NO_YES_OPTION_COUNT 2
const CMenuOptionChooser::keyval MESSAGEBOX_NO_YES_OPTIONS[MESSAGEBOX_NO_YES_OPTION_COUNT] =
{
	{ 0, LOCALE_MESSAGEBOX_NO, NULL },
	{ 1, LOCALE_MESSAGEBOX_YES, NULL }
};

// Init Main Menu
void CNeutrinoApp::InitMainMenu(CMenuWidget &mainMenu, CMenuWidget &mainSettings, CMenuWidget &videoSettings, CMenuWidget &audioSettings, CMenuWidget &parentallockSettings, CMenuWidget &networkSettings, CMenuWidget &recordingSettings, CMenuWidget &colorSettings, CMenuWidget &lcdSettings, CMenuWidget &keySettings, CMenuWidget &miscSettings, CMenuWidget &service, CMenuWidget &audioplayerSettings, CMenuWidget &PicViewerSettings, CMenuWidget &moviePlayerSettings, CMenuWidget &MediaPlayer)
{
	int shortcut = 1;

	dprintf(DEBUG_NORMAL, "CNeutrinoApp::InitMainMenu\n");

#if !defined (USE_OPENGL)  // needed for testing/debugging
	if(FrontendCount)
#endif	  
	{	  
		// tv modus
		mainMenu.addItem(new CMenuForwarderExtended(LOCALE_MAINMENU_TVMODE, true, this, "tv", CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED, NEUTRINO_ICON_MENUITEM_TV, LOCALE_HELPTEXT_TVMODE ), true);

		// radio modus
		mainMenu.addItem(new CMenuForwarderExtended(LOCALE_MAINMENU_RADIOMODE, true, this, "radio", CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN, NEUTRINO_ICON_MENUITEM_RADIO, LOCALE_HELPTEXT_RADIOMODE ));	
	}
	
	// webtv
	mainMenu.addItem(new CMenuForwarderExtended(LOCALE_MAINMENU_WEBTVMODE, true, this, "webtv", CRCInput::RC_yellow, NEUTRINO_ICON_BUTTON_YELLOW, NEUTRINO_ICON_MENUITEM_WEBTV, LOCALE_HELPTEXT_SCART) );
	
#if defined (ENABLE_SCART)
	// webtv
	mainMenu.addItem(new CMenuForwarderExtended(LOCALE_MAINMENU_SCARTMODE, true, this, "scart", CRCInput::RC_blue, NEUTRINO_ICON_BUTTON_BLUE, NEUTRINO_ICON_MENUITEM_SCART, LOCALE_HELPTEXT_SCART) );
#endif

	// media center
	// Media player main menu
	mainMenu.addItem(new CMenuForwarderExtended(LOCALE_MAINMENU_MEDIAPLAYER, true, &MediaPlayer, NULL, CRCInput::convertDigitToKey(shortcut++), NULL, NEUTRINO_ICON_MENUITEM_MOVIEPLAYER, LOCALE_HELPTEXT_MEDIAPLAYER ));
	
	int shortcutMediaPlayer = 1;
	
	//
#if defined (ENABLE_LIBEPLAYER3) || defined (ENABLE_GSTREAMER)	
	//Internet Radio
	MediaPlayer.addItem(new CMenuForwarderExtended(LOCALE_MAINMENU_INETRADIO, true, new CAudioPlayerGui(true), NULL, CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED, NEUTRINO_ICON_MENUITEM_INTERNETRADIO, LOCALE_HELPTEXT_INTERNETRADIO ));

	//AudioPlayer
	MediaPlayer.addItem(new CMenuForwarderExtended(LOCALE_MAINMENU_AUDIOPLAYER, true, new CAudioPlayerGui(), NULL, CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN, NEUTRINO_ICON_MENUITEM_AUDIOPLAYER, LOCALE_HELPTEXT_AUDIOPLAYER ));
#endif	
	
	// movieplayer
	moviePlayerGui = new CMoviePlayerGui();
	
#if defined (ENABLE_LIBEPLAYER3) || defined (ENABLE_GSTREAMER)
	// ts/movie/file
	// ts player
	MediaPlayer.addItem(new CMenuForwarderExtended(LOCALE_MOVIEPLAYER_RECORDS, true, moviePlayerGui, "tsmoviebrowser", CRCInput::convertDigitToKey(shortcutMediaPlayer++), NULL, NEUTRINO_ICON_MENUITEM_MOVIEPLAYER, LOCALE_HELPTEXT_TSMOVIEBROWSER ));
	
	// movie player
	MediaPlayer.addItem(new CMenuForwarderExtended(LOCALE_MOVIEPLAYER_MOVIES, true, moviePlayerGui, "moviebrowser", CRCInput::convertDigitToKey(shortcutMediaPlayer++), NULL, NEUTRINO_ICON_MENUITEM_MOVIEPLAYER, LOCALE_HELPTEXT_TSMOVIEBROWSER ));
	
	// fileplayback
	MediaPlayer.addItem(new CMenuForwarderExtended(LOCALE_MOVIEPLAYER_FILEPLAYBACK, true, moviePlayerGui, "fileplayback", CRCInput::convertDigitToKey(shortcutMediaPlayer++), NULL, NEUTRINO_ICON_MENUITEM_MOVIEPLAYER, LOCALE_HELPTEXT_FILEPLAYBACK ));	
#endif	

	//PictureViewer
	MediaPlayer.addItem(new CMenuForwarderExtended(LOCALE_MAINMENU_PICTUREVIEWER, true, new CPictureViewerGui(), NULL, CRCInput::RC_yellow, NEUTRINO_ICON_BUTTON_YELLOW, NEUTRINO_ICON_MENUITEM_PICTUREVIEWER, LOCALE_HELPTEXT_PICTUREVIEWER ));
	
	//UPNP Browser
 	MediaPlayer.addItem(new CMenuForwarderExtended(LOCALE_UPNPBROWSER_HEAD, true, new CUpnpBrowserGui(), NULL, CRCInput::RC_blue, NEUTRINO_ICON_BUTTON_BLUE, NEUTRINO_ICON_MENUITEM_UPNPBROWSER, LOCALE_HELPTEXT_UPNPBROWSER ));
	
	//Main Setting Menu
	mainMenu.addItem(new CMenuForwarderExtended(LOCALE_MAINMENU_SETTINGS, true, &mainSettings, NULL, CRCInput::convertDigitToKey(shortcut++), NULL, NEUTRINO_ICON_MENUITEM_MAINSETTINGS, LOCALE_HELPTEXT_MAINSETTINGS ));

	//Service
	mainMenu.addItem(new CMenuForwarderExtended(LOCALE_MAINMENU_SERVICE, true, &service, NULL, CRCInput::convertDigitToKey(shortcut++), NULL, NEUTRINO_ICON_MENUITEM_SERVICE, LOCALE_HELPTEXT_SERVICE ));
	
	// timerlist
	mainMenu.addItem(new CMenuForwarderExtended(LOCALE_TIMERLIST_NAME, true, Timerlist, NULL, CRCInput::convertDigitToKey(shortcut++), NULL, NEUTRINO_ICON_MENUITEM_SLEEPTIMER, LOCALE_HELPTEXT_TIMERLIST ));
	
	// features
	mainMenu.addItem(new CMenuForwarderExtended(LOCALE_INFOVIEWER_FEATURES, true, this, "features", CRCInput::convertDigitToKey(shortcut++), NULL, NEUTRINO_ICON_MENUITEM_PLUGINS, LOCALE_HELPTEXT_FEATURES ));

	//sleep timer
	mainMenu.addItem(new CMenuForwarderExtended(LOCALE_MAINMENU_SLEEPTIMER, true, new CSleepTimerWidget, NULL, CRCInput::convertDigitToKey(shortcut++), NULL, NEUTRINO_ICON_MENUITEM_SLEEPTIMER, LOCALE_HELPTEXT_SLEEPTIMER ));

	//Reboot
	mainMenu.addItem(new CMenuForwarderExtended(LOCALE_MAINMENU_REBOOT, true, this, "reboot", CRCInput::convertDigitToKey(shortcut++), NULL, NEUTRINO_ICON_MENUITEM_REBOOT, LOCALE_HELPTEXT_REBOOT ));

	//Shutdown
	mainMenu.addItem(new CMenuForwarderExtended(LOCALE_MAINMENU_SHUTDOWN, true, this, "shutdown", CRCInput::RC_standby, NEUTRINO_ICON_BUTTON_POWER, NEUTRINO_ICON_MENUITEM_SHUTDOWN, LOCALE_HELPTEXT_SHUTDOWN ));//FIXME

	//box info
	mainMenu.addItem( new CMenuForwarderExtended(LOCALE_DBOXINFO, true, new CDBoxInfoWidget, NULL, CRCInput::RC_info, NEUTRINO_ICON_BUTTON_HELP_SMALL, NEUTRINO_ICON_MENUITEM_BOXINFO, LOCALE_HELPTEXT_BOXINFO ));
	// end main menu

	// main settings
	int shortcutMainSettings = 1;

	// video settings
	mainSettings.addItem(new CMenuForwarderExtended(LOCALE_MAINSETTINGS_VIDEO, true, &videoSettings, NULL, CRCInput::convertDigitToKey(shortcutMainSettings++), NULL, NEUTRINO_ICON_MENUITEM_TV, LOCALE_HELPTEXT_VIDEOSETTINGS ));

	//Audio Settings
	mainSettings.addItem(new CMenuForwarderExtended(LOCALE_MAINSETTINGS_AUDIO, true, &audioSettings, NULL, CRCInput::convertDigitToKey(shortcutMainSettings++), NULL, NEUTRINO_ICON_MENUITEM_AUDIOSETTINGS, LOCALE_HELPTEXT_AUDIOSETTINGS ));

	//Parentallock
	if(g_settings.parentallock_prompt)
		mainSettings.addItem(new CLockedMenuForwarderExtended(LOCALE_PARENTALLOCK_PARENTALLOCK, g_settings.parentallock_pincode, true, true, &parentallockSettings, NULL, CRCInput::convertDigitToKey(shortcutMainSettings++), NULL, "parentallock", LOCALE_HELPTEXT_PARENTALLOCK ));
	else
		mainSettings.addItem(new CMenuForwarderExtended(LOCALE_PARENTALLOCK_PARENTALLOCK, true, &parentallockSettings, NULL, CRCInput::convertDigitToKey(shortcutMainSettings++), NULL, NEUTRINO_ICON_MENUITEM_PARENTALLOCK, LOCALE_HELPTEXT_PARENTALLOCK ));

	//Network Settings
	mainSettings.addItem(new CMenuForwarderExtended(LOCALE_MAINSETTINGS_NETWORK, true, &networkSettings  , NULL, CRCInput::convertDigitToKey(shortcutMainSettings++), NULL, NEUTRINO_ICON_MENUITEM_UPNPBROWSER, LOCALE_HELPTEXT_NETWORKSETTINGS ));

	//Recording Settings
	mainSettings.addItem(new CMenuForwarderExtended(LOCALE_MAINSETTINGS_RECORDING, true, &recordingSettings, NULL, CRCInput::convertDigitToKey(shortcutMainSettings++), NULL, NEUTRINO_ICON_MENUITEM_RECORDINGSETTINGS, LOCALE_HELPTEXT_RECORDINGSETTINGS ));

	//MoviePlayer und Streamings Settings
	mainSettings.addItem(new CMenuForwarderExtended(LOCALE_MAINSETTINGS_STREAMING, true, &moviePlayerSettings, NULL, CRCInput::convertDigitToKey(shortcutMainSettings++), NULL, NEUTRINO_ICON_MENUITEM_MOVIEPLAYER, LOCALE_HELPTEXT_MOVIEPLAYERSETTINGS ));

	//OSD settings
	mainSettings.addItem(new CMenuForwarderExtended(LOCALE_MAINSETTINGS_OSD, true, &colorSettings, NULL, CRCInput::convertDigitToKey(shortcutMainSettings++), NULL, NEUTRINO_ICON_MENUITEM_OSDSETTINGS, LOCALE_HELPTEXT_OSDSETTINGS ));

	// vfd/lcd settings
	//if(CVFD::getInstance()->has_lcd)
		mainSettings.addItem(new CMenuForwarderExtended(LOCALE_MAINSETTINGS_LCD, true, &lcdSettings, NULL, CRCInput::convertDigitToKey(shortcutMainSettings++), NULL, NEUTRINO_ICON_MENUITEM_MAINSETTINGS, LOCALE_HELPTEXT_VFDSETTINGS ));	

	//Keys settings
	mainSettings.addItem(new CMenuForwarderExtended(LOCALE_MAINSETTINGS_KEYBINDING, true, &keySettings, NULL, CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED, NEUTRINO_ICON_MENUITEM_KEYSSETTINGS, LOCALE_HELPTEXT_KEYSSETTINGS ));

	//Audio-Player settings
	mainSettings.addItem(new CMenuForwarderExtended(LOCALE_AUDIOPLAYERSETTINGS_GENERAL, true, &audioplayerSettings, NULL, CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN, NEUTRINO_ICON_MENUITEM_AUDIOPLAYERSETTINGS, LOCALE_HELPTEXT_AUDIOPLAYERSETTINGS ));
	
	//Picture-Player settings
	mainSettings.addItem(new CMenuForwarderExtended(LOCALE_PICTUREVIEWERSETTINGS_GENERAL, true, &PicViewerSettings, NULL, CRCInput::RC_yellow, NEUTRINO_ICON_BUTTON_YELLOW, NEUTRINO_ICON_MENUITEM_PICTUREVIEWER, LOCALE_HELPTEXT_PICTUREVIEWERSETTINGS ));

	//Misc Settings
	mainSettings.addItem(new CMenuForwarderExtended(LOCALE_MAINSETTINGS_MISC, true, &miscSettings, NULL, CRCInput::RC_blue, NEUTRINO_ICON_BUTTON_BLUE, NEUTRINO_ICON_MENUITEM_MAINSETTINGS, LOCALE_HELPTEXT_MISCSETTINGS ));

	//HDD settings
	mainSettings.addItem(new CMenuForwarderExtended(LOCALE_HDD_SETTINGS, true, new CHDDMenuHandler(), NULL, CRCInput::convertDigitToKey(shortcutMainSettings++), NULL, NEUTRINO_ICON_MENUITEM_HDDSETTINGS, LOCALE_HELPTEXT_HDDSETTINGS ));
}

//Video Settings
//hdmi color space
#if !defined (PLATFORM_COOLSTREAM)
#ifdef __sh__
#define VIDEOMENU_HDMI_COLOR_SPACE_OPTION_COUNT 3
const CMenuOptionChooser::keyval VIDEOMENU_HDMI_COLOR_SPACE_OPTIONS[VIDEOMENU_HDMI_COLOR_SPACE_OPTION_COUNT] =
{
	 { HDMI_RGB, NONEXISTANT_LOCALE, "HDMI-RGB" },
	 { HDMI_YUV, NONEXISTANT_LOCALE, "HDMI-YUV" } ,
	 { HDMI_422, NONEXISTANT_LOCALE, "HDMI-422" }
};
#else
// giga
/*
Edid(Auto) 
Hdmi_Rgb 
Itu_R_BT_709 
Unknown
*/
#define VIDEOMENU_HDMI_COLOR_SPACE_OPTION_COUNT 4
const CMenuOptionChooser::keyval VIDEOMENU_HDMI_COLOR_SPACE_OPTIONS[VIDEOMENU_HDMI_COLOR_SPACE_OPTION_COUNT] =
{
	 { HDMI_AUTO, NONEXISTANT_LOCALE, "Edid(Auto)" },
	 { HDMI_RGB, NONEXISTANT_LOCALE, "Hdmi_Rgb" } ,
	 { HDMI_ITU_R_BT_709, NONEXISTANT_LOCALE, "Itu_R_BT_709" },
	 { HDMI_UNKNOW, NONEXISTANT_LOCALE, "Unknow" }
};
#endif
#endif /* !coolstream*/

//Analog Output 
#ifdef __sh__
//rgb/cvbs/svideo/yuv
#define VIDEOMENU_ANALOGUE_MODE_OPTION_COUNT 4
const CMenuOptionChooser::keyval VIDEOMENU_ANALOGUE_MODE_OPTIONS[VIDEOMENU_ANALOGUE_MODE_OPTION_COUNT] =
{
	 { ANALOG_RGB, NONEXISTANT_LOCALE, "RGB" },
	 { ANALOG_CVBS, NONEXISTANT_LOCALE, "CVBS" },
	 { ANALOG_SVIDEO, NONEXISTANT_LOCALE, "SVIDEO" }, //not used
	 { ANALOG_YUV, NONEXISTANT_LOCALE, "YUV" }
};
#elif defined (PLATFORM_COOLSTREAM)
#define VIDEOMENU_ANALOGUE_MODE_OPTION_COUNT 4
const CMenuOptionChooser::keyval VIDEOMENU_ANALOGUE_MODE_OPTIONS[VIDEOMENU_ANALOGUE_MODE_OPTION_COUNT] =
{
	{ ANALOG_SD_RGB_SCART, NONEXISTANT_LOCALE, "RGB"   }, /* composite + RGB */
	{ ANALOG_SD_YPRPB_SCART, NONEXISTANT_LOCALE, "YPbPr" }, /* YPbPr SCART */
	{ ANALOG_HD_RGB_SCART, NONEXISTANT_LOCALE, "RGB Scart"   },
	{ ANALOG_HD_YPRPB_SCART, NONEXISTANT_LOCALE, "YPbPr Scart" },
};
#else
//rgb/cvbs/yuv
#define VIDEOMENU_ANALOGUE_MODE_OPTION_COUNT 3
const CMenuOptionChooser::keyval VIDEOMENU_ANALOGUE_MODE_OPTIONS[VIDEOMENU_ANALOGUE_MODE_OPTION_COUNT] =
{
	 { ANALOG_RGB, NONEXISTANT_LOCALE, "RGB" },
	 { ANALOG_CVBS, NONEXISTANT_LOCALE, "CVBS" },
	 { ANALOG_YUV, NONEXISTANT_LOCALE, "YUV" }
};
#endif

// aspect ratio
#ifdef __sh__
#define VIDEOMENU_VIDEORATIO_OPTION_COUNT 2
const CMenuOptionChooser::keyval VIDEOMENU_VIDEORATIO_OPTIONS[VIDEOMENU_VIDEORATIO_OPTION_COUNT] =
{
	{ ASPECTRATIO_43, LOCALE_VIDEOMENU_VIDEORATIO_43, NULL },
	{ ASPECTRATIO_169, LOCALE_VIDEOMENU_VIDEORATIO_169, NULL }
};
#elif defined (PLATFORM_COOLSTREAM)
#define VIDEOMENU_VIDEORATIO_OPTION_COUNT 2
const CMenuOptionChooser::keyval VIDEOMENU_VIDEORATIO_OPTIONS[VIDEOMENU_VIDEORATIO_OPTION_COUNT] =
{
	{ DISPLAY_AR_4_3, LOCALE_VIDEOMENU_VIDEORATIO_43, NULL },
	{ DISPLAY_AR_16_9, LOCALE_VIDEOMENU_VIDEORATIO_169, NULL },
};
#else
#define VIDEOMENU_VIDEORATIO_OPTION_COUNT 3
const CMenuOptionChooser::keyval VIDEOMENU_VIDEORATIO_OPTIONS[VIDEOMENU_VIDEORATIO_OPTION_COUNT] =
{
	{ ASPECTRATIO_43, LOCALE_VIDEOMENU_VIDEORATIO_43, NULL },
	{ ASPECTRATIO_169, LOCALE_VIDEOMENU_VIDEORATIO_169, NULL },
	{ ASPECTRATIO_AUTO, NONEXISTANT_LOCALE, "Auto" }
};
#endif

// policy
#ifdef __sh__
/*
letterbox 
panscan 
non 
bestfit
*/
#define VIDEOMENU_VIDEOFORMAT_OPTION_COUNT 4
const CMenuOptionChooser::keyval VIDEOMENU_VIDEOFORMAT_OPTIONS[VIDEOMENU_VIDEOFORMAT_OPTION_COUNT] = 
{
	{ VIDEOFORMAT_LETTERBOX, LOCALE_VIDEOMENU_LETTERBOX, NULL },
	{ VIDEOFORMAT_PANSCAN, LOCALE_VIDEOMENU_PANSCAN, NULL },
	{ VIDEOFORMAT_FULLSCREEN, LOCALE_VIDEOMENU_FULLSCREEN, NULL },
	{ VIDEOFORMAT_PANSCAN2, LOCALE_VIDEOMENU_PANSCAN2, NULL }
};
#elif defined (PLATFORM_COOLSTREAM)
#define VIDEOMENU_VIDEOFORMAT_OPTION_COUNT 4
const CMenuOptionChooser::keyval VIDEOMENU_VIDEOFORMAT_OPTIONS[VIDEOMENU_VIDEOFORMAT_OPTION_COUNT] = 
{
	{ DISPLAY_AR_MODE_PANSCAN, LOCALE_VIDEOMENU_PANSCAN, NULL },
	{ DISPLAY_AR_MODE_PANSCAN2, LOCALE_VIDEOMENU_PANSCAN2, NULL },
	{ DISPLAY_AR_MODE_LETTERBOX, LOCALE_VIDEOMENU_LETTERBOX, NULL },
	{ DISPLAY_AR_MODE_NONE, LOCALE_VIDEOMENU_FULLSCREEN, NULL }
};
#else
// giga
/*
letterbox 
panscan 
bestfit 
nonlinear
*/
#define VIDEOMENU_VIDEOFORMAT_OPTION_COUNT 4
const CMenuOptionChooser::keyval VIDEOMENU_VIDEOFORMAT_OPTIONS[VIDEOMENU_VIDEOFORMAT_OPTION_COUNT] = 
{
	{ VIDEOFORMAT_LETTERBOX, LOCALE_VIDEOMENU_LETTERBOX, NULL },
	{ VIDEOFORMAT_PANSCAN, LOCALE_VIDEOMENU_PANSCAN, NULL },
	{ VIDEOFORMAT_PANSCAN2, LOCALE_VIDEOMENU_PANSCAN2, NULL },
	{ VIDEOFORMAT_FULLSCREEN, LOCALE_VIDEOMENU_FULLSCREEN, NULL }
};
#endif

//video mode
#ifdef __sh__
// cuberevo
/*
pal 
1080i50 
720p50 
576p50 
576i50 
1080i60 
720p60 
1080p24 
1080p25 
1080p30 
1080p50
PC
*/
#define VIDEOMENU_VIDEOMODE_OPTION_COUNT 12
const CMenuOptionChooser::keyval VIDEOMENU_VIDEOMODE_OPTIONS[VIDEOMENU_VIDEOMODE_OPTION_COUNT] =
{
	{ VIDEO_STD_PAL, NONEXISTANT_LOCALE, "PAL"		},
	{ VIDEO_STD_1080I50, NONEXISTANT_LOCALE, "1080i 50Hz"	},
	{ VIDEO_STD_720P50, NONEXISTANT_LOCALE, "720p 50Hz"	},
	{ VIDEO_STD_576P50, NONEXISTANT_LOCALE, "576p 50Hz"	},
	{ VIDEO_STD_576I50, NONEXISTANT_LOCALE, "576i 50Hz"	},
	{ VIDEO_STD_1080I60, NONEXISTANT_LOCALE, "1080i 60Hz"	},
	{ VIDEO_STD_720P60, NONEXISTANT_LOCALE, "720p 60Hz"	},
	{ VIDEO_STD_1080P24, NONEXISTANT_LOCALE, "1080p 24Hz"	},
	{ VIDEO_STD_1080P25, NONEXISTANT_LOCALE, "1080p 25Hz"	},
	{ VIDEO_STD_1080P30, NONEXISTANT_LOCALE, "1080p 30Hz"	},
	{ VIDEO_STD_1080P50, NONEXISTANT_LOCALE, "1080p 50Hz" 	},
	{ VIDEO_STD_PC, NONEXISTANT_LOCALE, "PC"		}
};
#elif defined (PLATFORM_COOLSTREAM)
#define VIDEOMENU_VIDEOMODE_OPTION_COUNT 12
const CMenuOptionChooser::keyval VIDEOMENU_VIDEOMODE_OPTIONS[VIDEOMENU_VIDEOMODE_OPTION_COUNT] =
{
	{ VIDEO_STD_SECAM,   NONEXISTANT_LOCALE, "SECAM"	},
	{ VIDEO_STD_PAL,     NONEXISTANT_LOCALE, "PAL"		},
	{ VIDEO_STD_576P,    NONEXISTANT_LOCALE, "576p"		},
	{ VIDEO_STD_720P50,  NONEXISTANT_LOCALE, "720p 50Hz"	},
	{ VIDEO_STD_1080I50, NONEXISTANT_LOCALE, "1080i 50Hz"	},
	{ VIDEO_STD_1080P24, NONEXISTANT_LOCALE, "1080p 24Hz"	},
	{ VIDEO_STD_1080P25, NONEXISTANT_LOCALE, "1080p 25Hz"	},
	{ VIDEO_STD_NTSC,    NONEXISTANT_LOCALE, "NTSC"		},
	{ VIDEO_STD_480P,    NONEXISTANT_LOCALE, "480p"		},
	{ VIDEO_STD_720P60,  NONEXISTANT_LOCALE, "720p 60Hz"	},
	{ VIDEO_STD_1080I60, NONEXISTANT_LOCALE, "1080i 60Hz"	},
	{ VIDEO_STD_AUTO,    NONEXISTANT_LOCALE, "Auto"         }
};
#else
// giga
/*
pal 
ntsc 
480i 
576i 
480p 
576p 
720p50 
720p 
1080i50 
1080i
*/
#define VIDEOMENU_VIDEOMODE_OPTION_COUNT 10
const CMenuOptionChooser::keyval VIDEOMENU_VIDEOMODE_OPTIONS[VIDEOMENU_VIDEOMODE_OPTION_COUNT] =
{
	{ VIDEO_STD_PAL, NONEXISTANT_LOCALE, "PAL"		},
	{ VIDEO_STD_NTSC, NONEXISTANT_LOCALE, "NTSC"		},
	{ VIDEO_STD_480I60, NONEXISTANT_LOCALE, "480i 60Hz"	},
	{ VIDEO_STD_576I50, NONEXISTANT_LOCALE, "576i 50Hz"	},
	{ VIDEO_STD_480P60, NONEXISTANT_LOCALE, "480p 60Hz"	},
	{ VIDEO_STD_576P50, NONEXISTANT_LOCALE, "576p 50Hz"	},
	{ VIDEO_STD_720P50, NONEXISTANT_LOCALE, "720p 50Hz"	},
	{ VIDEO_STD_720P60, NONEXISTANT_LOCALE, "720p 60Hz"	},
	{ VIDEO_STD_1080I50, NONEXISTANT_LOCALE, "1080i 50Hz"	},
	{ VIDEO_STD_1080I60, NONEXISTANT_LOCALE, "1080i 60Hz"	}
};
#endif

// wss
/*
off 
auto 
auto(4:3_off) 
4:3_full_format 
16:9_full_format 
14:9_letterbox_center 
14:9_letterbox_top 
16:9_letterbox_center 
16:9_letterbox_top 
>16:9_letterbox_center 
14:9_full_format
*/
#if !defined (PLATFORM_COOLSTREAM)
#ifdef __sh__
#define VIDEOMENU_WSS_OPTION_COUNT 3
const CMenuOptionChooser::keyval VIDEOMENU_WSS_OPTIONS[VIDEOMENU_WSS_OPTION_COUNT] =
{
	{ WSS_OFF, NONEXISTANT_LOCALE, "Off" },
	{ WSS_AUTO, NONEXISTANT_LOCALE, "Auto" },
	{ WSS_43_OFF, NONEXISTANT_LOCALE, "Auto(4:3_off)" },
};
#else
// giga
/*
off 
auto 
auto(4:3_off) 
4:3_full_format 
16:9_full_format 
14:9_letterbox_center 
14:9_letterbox_top 
16:9_letterbox_center 
16:9_letterbox_top 
>16:9_letterbox_center 
14:9_full_format
*/
#define VIDEOMENU_WSS_OPTION_COUNT 11
const CMenuOptionChooser::keyval VIDEOMENU_WSS_OPTIONS[VIDEOMENU_WSS_OPTION_COUNT] =
{
	{ WSS_OFF, NONEXISTANT_LOCALE, "Off" },
	{ WSS_AUTO, NONEXISTANT_LOCALE, "Auto" },
	{ WSS_43_OFF, NONEXISTANT_LOCALE, "Auto(4:3_off)" },
	{ WSS_43_FULL, NONEXISTANT_LOCALE, "4:3(full_format)" },
	{ WSS_169_FULL, NONEXISTANT_LOCALE, "16:9(full_format)" },
	{ WSS_149_LETTERBOX_CENTER, NONEXISTANT_LOCALE, "14:9(letterbox-center)" },
	{ WSS_149_LETTERBOX_TOP, NONEXISTANT_LOCALE, "14:9(letterbox-top)" },
	{ WSS_169_LETTERBOX_CENTER, NONEXISTANT_LOCALE, "16:9(letterbox-center)" },
	{ WSS_169_LETTERBOX_TOP, NONEXISTANT_LOCALE, "16:9(letterbox-top)" },
	{ WSS_169_LETTERBOX_CENTER_RIGHT, NONEXISTANT_LOCALE, ">16:9(letterbox-center)" },
	{ WSS_149_FULL, NONEXISTANT_LOCALE, "14:9(full)"}
	
};
#endif
#endif /* !coolstream*/

void CNeutrinoApp::InitVideoSettings(CMenuWidget &videoSettings, CVideoSetupNotifier * videoSetupNotifier)
{
	dprintf(DEBUG_NORMAL, "CNeutrinoApp::InitVideoSettings\n");
	
	int shortcutVideo = 1;
	
	// intros
	videoSettings.addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	videoSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	
	// save settings
	videoSettings.addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_SAVESETTINGSNOW, true, NULL, this, "savesettings", CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));
	videoSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );

	// video aspect ratio 4:3/16:9
	videoSettings.addItem(new CMenuOptionChooser(LOCALE_VIDEOMENU_VIDEORATIO, &g_settings.video_Ratio, VIDEOMENU_VIDEORATIO_OPTIONS, VIDEOMENU_VIDEORATIO_OPTION_COUNT, true, videoSetupNotifier, CRCInput::convertDigitToKey(shortcutVideo++), "", true ));
	
	// video format bestfit/letterbox/panscan/non
	videoSettings.addItem(new CMenuOptionChooser(LOCALE_VIDEOMENU_VIDEOFORMAT, &g_settings.video_Format, VIDEOMENU_VIDEOFORMAT_OPTIONS, VIDEOMENU_VIDEOFORMAT_OPTION_COUNT, true, videoSetupNotifier, CRCInput::convertDigitToKey(shortcutVideo++), "", true ));
	
	// video analogue mode
	videoSettings.addItem(new CMenuOptionChooser(LOCALE_VIDEOMENU_ANALOG_MODE, &g_settings.analog_mode, VIDEOMENU_ANALOGUE_MODE_OPTIONS, VIDEOMENU_ANALOGUE_MODE_OPTION_COUNT, true, videoSetupNotifier, CRCInput::convertDigitToKey(shortcutVideo++), "", true ));

#if !defined (PLATFORM_COOLSTREAM)
	// video hdmi space colour	
	videoSettings.addItem(new CMenuOptionChooser(LOCALE_VIDEOMENU_HDMI_COLOR_SPACE, &g_settings.hdmi_color_space, VIDEOMENU_HDMI_COLOR_SPACE_OPTIONS, VIDEOMENU_HDMI_COLOR_SPACE_OPTION_COUNT, true, videoSetupNotifier, CRCInput::convertDigitToKey(shortcutVideo++), "", true ));	
	
	// wss
	videoSettings.addItem(new CMenuOptionChooser(LOCALE_VIDEOMENU_WSS, &g_settings.wss_mode, VIDEOMENU_WSS_OPTIONS, VIDEOMENU_WSS_OPTION_COUNT, true, videoSetupNotifier, CRCInput::convertDigitToKey(shortcutVideo++), "", true ));
#endif	

	// video mode
	videoSettings.addItem(new CMenuOptionChooser(LOCALE_VIDEOMENU_VIDEOMODE, &g_settings.video_Mode, VIDEOMENU_VIDEOMODE_OPTIONS, VIDEOMENU_VIDEOMODE_OPTION_COUNT, true, videoSetupNotifier, CRCInput::convertDigitToKey(shortcutVideo++), "", true));
}

// Init Audio Settings
#define AUDIOMENU_ANALOGOUT_OPTION_COUNT 3
const CMenuOptionChooser::keyval AUDIOMENU_ANALOGOUT_OPTIONS[AUDIOMENU_ANALOGOUT_OPTION_COUNT] =
{
	{ 0, LOCALE_AUDIOMENU_STEREO, NULL },
	{ 1, LOCALE_AUDIOMENU_MONOLEFT, NULL },
	{ 2, LOCALE_AUDIOMENU_MONORIGHT, NULL }
};

#if defined (PLATFORM_COOLSTREAM)
#define AUDIOMENU_AVSYNC_OPTION_COUNT 3
const CMenuOptionChooser::keyval AUDIOMENU_AVSYNC_OPTIONS[AUDIOMENU_AVSYNC_OPTION_COUNT] =
{
        { 0, LOCALE_OPTIONS_OFF, NULL },
        { 1, LOCALE_OPTIONS_ON, NULL },
        { 2, LOCALE_AUDIOMENU_AVSYNC_AM, NULL }
};
#else
#define AUDIOMENU_AVSYNC_OPTION_COUNT 3
const CMenuOptionChooser::keyval AUDIOMENU_AVSYNC_OPTIONS[AUDIOMENU_AVSYNC_OPTION_COUNT] =
{
        { AVSYNC_OFF, LOCALE_OPTIONS_OFF, NULL },
        { AVSYNC_ON, LOCALE_OPTIONS_ON, NULL },
        { AVSYNC_AM, LOCALE_AUDIOMENU_AVSYNC_AM, NULL }
};
#endif

// ac3
#if !defined (PLATFORM_COOLSTREAM)
#define AC3_OPTION_COUNT 2
const CMenuOptionChooser::keyval AC3_OPTIONS[AC3_OPTION_COUNT] =
{
	{ AC3_DOWNMIX, NONEXISTANT_LOCALE, "downmix" },
	{ AC3_PASSTHROUGH, NONEXISTANT_LOCALE, "passthrough" }
};

#define AUDIODELAY_OPTION_COUNT 9
const CMenuOptionChooser::keyval AUDIODELAY_OPTIONS[AUDIODELAY_OPTION_COUNT] =
{
	{ -1000, NONEXISTANT_LOCALE, "-1000" },
	{ -750, NONEXISTANT_LOCALE, "-750" },
	{ -500, NONEXISTANT_LOCALE, "-500" },
	{ -250, NONEXISTANT_LOCALE, "-250" },
	{ 0, NONEXISTANT_LOCALE, "0" },
	{ 250, NONEXISTANT_LOCALE, "250" },
	{ 500, NONEXISTANT_LOCALE, "500" },
	{ 750, NONEXISTANT_LOCALE, "750" },
	{ 1000, NONEXISTANT_LOCALE, "1000" }
};
#endif

void CNeutrinoApp::InitAudioSettings(CMenuWidget &audioSettings, CAudioSetupNotifier * audioSetupNotifier)
{
	dprintf(DEBUG_NORMAL, "CNeutrinoApp::InitAudioSettings\n");
	
	int shortcutAudio = 1;
	
	// intros
	audioSettings.addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	audioSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	
	// save settings
	audioSettings.addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_SAVESETTINGSNOW, true, NULL, this, "savesettings", CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));
	audioSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );

	// analog output
	audioSettings.addItem(new CMenuOptionChooser(LOCALE_AUDIOMENU_ANALOGOUT, &g_settings.audio_AnalogMode, AUDIOMENU_ANALOGOUT_OPTIONS, AUDIOMENU_ANALOGOUT_OPTION_COUNT, true, audioSetupNotifier, CRCInput::convertDigitToKey(shortcutAudio++), "", true ));

#if !defined (PLATFORM_COOLSTREAM)	
	// hdmi-dd
	audioSettings.addItem(new CMenuOptionChooser(LOCALE_AUDIOMENU_HDMI_DD, &g_settings.hdmi_dd, AC3_OPTIONS, AC3_OPTION_COUNT, true, audioSetupNotifier, CRCInput::convertDigitToKey(shortcutAudio++) ));	
#endif	

	// A/V sync
	audioSettings.addItem(new CMenuOptionChooser(LOCALE_AUDIOMENU_AVSYNC, &g_settings.avsync, AUDIOMENU_AVSYNC_OPTIONS, AUDIOMENU_AVSYNC_OPTION_COUNT, true, audioSetupNotifier, CRCInput::convertDigitToKey(shortcutAudio++), "", true ));
	
#if !defined (PLATFORM_COOLSTREAM)	
	// ac3 delay
	audioSettings.addItem(new CMenuOptionChooser(LOCALE_AUDIOMENU_AC3_DELAY, &g_settings.ac3_delay, AUDIODELAY_OPTIONS, AUDIODELAY_OPTION_COUNT, true, audioSetupNotifier, CRCInput::convertDigitToKey(shortcutAudio++) ));
	
	// pcm delay
	audioSettings.addItem(new CMenuOptionChooser(LOCALE_AUDIOMENU_PCM_DELAY, &g_settings.pcm_delay, AUDIODELAY_OPTIONS, AUDIODELAY_OPTION_COUNT, true, audioSetupNotifier, CRCInput::convertDigitToKey(shortcutAudio++) ));
#endif	
	
	// pref sub/lang
	audioSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_AUDIOMENU_PREF_LANG_HEAD));
	
	// auto ac3 
	CMenuOptionChooser * a1 = new CMenuOptionChooser(LOCALE_AUDIOMENU_DOLBYDIGITAL, &g_settings.audio_DolbyDigital, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, g_settings.auto_lang, audioSetupNotifier );
	
	// audiolang
	CMenuOptionStringChooser * audiolangSelect[3];
	
	for(int i = 0; i < 3; i++) 
	{
		audiolangSelect[i] = new CMenuOptionStringChooser(LOCALE_AUDIOMENU_PREF_LANG, g_settings.pref_lang[i], g_settings.auto_lang, NULL, CRCInput::RC_nokey, "", true);
		
		audiolangSelect[i]->addOption("");
		std::map<std::string, std::string>::const_iterator it;
		for(it = iso639rev.begin(); it != iso639rev.end(); it++) 
			audiolangSelect[i]->addOption(it->first.c_str());
	}
	
	CAutoAudioNotifier * autoAudioNotifier = new CAutoAudioNotifier(a1, audiolangSelect[0], audiolangSelect[1], audiolangSelect[2]);
	
	// auto lang
	audioSettings.addItem(new CMenuOptionChooser(LOCALE_AUDIOMENU_AUTO_LANG, &g_settings.auto_lang, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true, autoAudioNotifier));
	
	// ac3
	audioSettings.addItem(a1);
	
	// lang
	for(int i = 0; i < 3; i++) 
		audioSettings.addItem(audiolangSelect[i]);
	
	// sublang
	audioSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_AUDIOMENU_PREF_SUBS_HEAD));
	
	CMenuOptionStringChooser * sublangSelect[3];
	for(int i = 0; i < 3; i++) 
	{
		sublangSelect[i] = new CMenuOptionStringChooser(LOCALE_AUDIOMENU_PREF_SUBS, g_settings.pref_subs[i], g_settings.auto_subs, NULL, CRCInput::RC_nokey, "", true);
		std::map<std::string, std::string>::const_iterator it;
		
		sublangSelect[i]->addOption("");
		for(it = iso639rev.begin(); it != iso639rev.end(); it++) 
			sublangSelect[i]->addOption(it->first.c_str());
	}
	
	CSubLangSelectNotifier * subLangSelectNotifier = new CSubLangSelectNotifier(sublangSelect[0], sublangSelect[1], sublangSelect[2]);
	
	// auto sublang
	audioSettings.addItem(new CMenuOptionChooser(LOCALE_AUDIOMENU_AUTO_SUBS, &g_settings.auto_subs, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true, subLangSelectNotifier));
	
	// sublang
	for(int i = 0; i < 3; i++) 
		audioSettings.addItem(sublangSelect[i]);
}

// Init Parentallock Settings
#define PARENTALLOCK_PROMPT_OPTION_COUNT 3
const CMenuOptionChooser::keyval PARENTALLOCK_PROMPT_OPTIONS[PARENTALLOCK_PROMPT_OPTION_COUNT] =
{
	{ PARENTALLOCK_PROMPT_NEVER         , LOCALE_PARENTALLOCK_NEVER, NULL         },
	{ PARENTALLOCK_PROMPT_CHANGETOLOCKED, LOCALE_PARENTALLOCK_CHANGETOLOCKED, NULL },
	{ PARENTALLOCK_PROMPT_ONSIGNAL      , LOCALE_PARENTALLOCK_ONSIGNAL, NULL       }
};

#define PARENTALLOCK_LOCKAGE_OPTION_COUNT 3
const CMenuOptionChooser::keyval PARENTALLOCK_LOCKAGE_OPTIONS[PARENTALLOCK_LOCKAGE_OPTION_COUNT] =
{
	{ 12, LOCALE_PARENTALLOCK_LOCKAGE12, NULL },
	{ 16, LOCALE_PARENTALLOCK_LOCKAGE16, NULL },
	{ 18, LOCALE_PARENTALLOCK_LOCKAGE18, NULL }
};

void CNeutrinoApp::InitParentalLockSettings(CMenuWidget &parentallockSettings)
{
	dprintf(DEBUG_NORMAL, "CNeutrinoApp::InitParentalLockSettings\n");
	
	int shortcutLock = 1;
	
	// intrso
	parentallockSettings.addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	parentallockSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	
	// save settings
	parentallockSettings.addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_SAVESETTINGSNOW, true, NULL, this, "savesettings", CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));
	parentallockSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );

	// prompt
	parentallockSettings.addItem(new CMenuOptionChooser(LOCALE_PARENTALLOCK_PROMPT, &g_settings.parentallock_prompt, PARENTALLOCK_PROMPT_OPTIONS, PARENTALLOCK_PROMPT_OPTION_COUNT, !parentallocked, NULL, CRCInput::convertDigitToKey(shortcutLock++), "", true ));

	// lockage
	parentallockSettings.addItem(new CMenuOptionChooser(LOCALE_PARENTALLOCK_LOCKAGE, &g_settings.parentallock_lockage, PARENTALLOCK_LOCKAGE_OPTIONS, PARENTALLOCK_LOCKAGE_OPTION_COUNT, !parentallocked, NULL, CRCInput::convertDigitToKey(shortcutLock++), "", true ));

	// Pin
	CPINChangeWidget * pinChangeWidget = new CPINChangeWidget(LOCALE_PARENTALLOCK_CHANGEPIN, g_settings.parentallock_pincode, 4, LOCALE_PARENTALLOCK_CHANGEPIN_HINT1);
	parentallockSettings.addItem( new CMenuForwarder(LOCALE_PARENTALLOCK_CHANGEPIN, true, g_settings.parentallock_pincode, pinChangeWidget, NULL, CRCInput::convertDigitToKey(shortcutLock++) ));
}

// Init Service Settings
void CNeutrinoApp::InitServiceSettings(CMenuWidget &service, CMenuWidget & TunerSetup)
{
	dprintf(DEBUG_NORMAL, "CNeutrinoApp::InitServiceSettings\n");
	
	int shortcutService = 1;
	
#if !defined (USE_OPENGL)  // needed for testing/debugging	
	if(FrontendCount)
#endif	  
	{
		// scan setup
		if(FrontendCount > 1)
		{
			// intros
			TunerSetup.addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
			TunerSetup.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
			
			for(int i = 0; i < FrontendCount; i++)
			{
				CFrontend * fe = getFE(i);
				char tbuf[255];
			
				sprintf(tbuf, "Tuner-%d: %s", i + 1, fe->getInfo()->name);
				TunerSetup.addItem(new CMenuForwarder(tbuf, true, NULL, new CScanSetup(i) ));
			}	
			
			// scan settings
			service.addItem(new CMenuForwarderExtended(LOCALE_SERVICEMENU_SCANTS, true, &TunerSetup, NULL, CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED, NEUTRINO_ICON_MENUITEM_SERVICE, LOCALE_HELPTEXT_SCANSETUP ));
		}
		else
		{
			// scan settings
			service.addItem(new CMenuForwarderExtended(LOCALE_SERVICEMENU_SCANTS, true, new CScanSetup(), "", CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED, NEUTRINO_ICON_MENUITEM_SERVICE, LOCALE_HELPTEXT_SCANSETUP ));
		}

		// reload Channels
		service.addItem(new CMenuForwarderExtended(LOCALE_SERVICEMENU_RELOAD, true, this, "reloadchannels", CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN, NEUTRINO_ICON_MENUITEM_SERVICE, LOCALE_HELPTEXT_RELOADCHANNELS ));

		// Bouquets Editor
		service.addItem(new CMenuForwarderExtended(LOCALE_BOUQUETEDITOR_NAME, true, new CBEBouquetWidget(), NULL, CRCInput::RC_yellow, NEUTRINO_ICON_BUTTON_YELLOW, NEUTRINO_ICON_MENUITEM_SERVICE, LOCALE_HELPTEXT_BOUQUETSEDITOR ));
	
	// CI Cam 	
#if defined (ENABLE_CI)
		service.addItem(new CMenuForwarderExtended(LOCALE_CAM_SETTINGS, true, g_CamHandler, NULL, CRCInput::convertDigitToKey(shortcutService++), NULL, NEUTRINO_ICON_MENUITEM_CAM, LOCALE_HELPTEXT_CAM ));
#endif
	}
	
	service.addItem(new CMenuForwarderExtended(LOCALE_SERVICEMENU_IMAGEINFO,  true, new CImageInfo(), NULL, CRCInput::RC_info, NEUTRINO_ICON_BUTTON_HELP_SMALL, NEUTRINO_ICON_MENUITEM_BOXINFO, LOCALE_HELPTEXT_IMAGEINFO), false);

	// restart neutrino
	service.addItem(new CMenuForwarderExtended(LOCALE_SERVICEMENU_RESTART, true, this, "restart", CRCInput::RC_standby, NEUTRINO_ICON_BUTTON_POWER, NEUTRINO_ICON_MENUITEM_SHUTDOWN, LOCALE_HELPTEXT_SOFTRESTART ));

	// softupdate	
	dprintf(DEBUG_NORMAL, "CNeutrinoApp::InitServiceSettings. init soft-update-stuff\n");
		
	CMenuWidget * updateSettings = new CMenuWidget(LOCALE_SERVICEMENU_UPDATE, NEUTRINO_ICON_UPDATE);
		
	// intros
	updateSettings->addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	updateSettings->addItem(new CMenuSeparator(CMenuSeparator::LINE));
	
	// save settings
	updateSettings->addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_SAVESETTINGSNOW, true, NULL, this, "savesettings", CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));
	updateSettings->addItem( new CMenuSeparator(CMenuSeparator::LINE) );

	// expert-function for mtd read/write
	CMenuWidget * mtdexpert = new CMenuWidget(LOCALE_FLASHUPDATE_EXPERTFUNCTIONS, NEUTRINO_ICON_UPDATE);
		
	// intros
	mtdexpert->addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	mtdexpert->addItem(new CMenuSeparator(CMenuSeparator::LINE));
		
	CFlashExpert * fe = new CFlashExpert();

	// read mtd 
	mtdexpert->addItem(new CMenuForwarder(LOCALE_FLASHUPDATE_READFLASHMTD , true, NULL, fe, "readflashmtd" ));

	// write mtd
	mtdexpert->addItem(new CMenuForwarder(LOCALE_FLASHUPDATE_WRITEFLASHMTD, true, NULL, fe, "writeflashmtd"));

	// experten function
	//FIXME: allow update only when the rootfs is jffs2/squashfs
	updateSettings->addItem(new CMenuForwarder(LOCALE_FLASHUPDATE_EXPERTFUNCTIONS, true, NULL, mtdexpert, "", CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN));
	updateSettings->addItem(new CMenuSeparator(CMenuSeparator::LINE));
		
	// update dir
	updateSettings->addItem( new CMenuForwarder(LOCALE_EXTRA_UPDATE_DIR, true, g_settings.update_dir , this, "update_dir") );
	
	// url
	CStringInputSMS * updateSettings_url_file = new CStringInputSMS(LOCALE_FLASHUPDATE_URL_FILE, g_settings.softupdate_url_file);
	updateSettings->addItem(new CMenuForwarder(LOCALE_FLASHUPDATE_URL_FILE, true, g_settings.softupdate_url_file, updateSettings_url_file));

	// show current version
	updateSettings->addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_FLASHUPDATE_CURRENTVERSION_SEP));

	// get current version SBBB YYYY MM TT HH MM -- formatsting
	CConfigFile lconfigfile('\t');

	const char * versionString = (lconfigfile.loadConfig("/etc/.version")) ? (lconfigfile.getString( "version", "1200201205091849").c_str()) : "1200201205091849";

	dprintf(DEBUG_INFO, "CNeutrinoApp::InitServiceSettings: current flash-version: %s\n", versionString);

	static CFlashVersionInfo versionInfo(versionString);

	// release cycle
	updateSettings->addItem(new CMenuForwarder(LOCALE_FLASHUPDATE_CURRENTRELEASECYCLE, false, versionInfo.getReleaseCycle() ));
		
	// date
	updateSettings->addItem(new CMenuForwarder(LOCALE_FLASHUPDATE_CURRENTVERSIONDATE, false, versionInfo.getDate() ));
		
	// time
	updateSettings->addItem(new CMenuForwarder(LOCALE_FLASHUPDATE_CURRENTVERSIONTIME, false, versionInfo.getTime()));
		
	// type
	/* versionInfo.getType() returns const char * which is never deallocated */
	updateSettings->addItem(new CMenuForwarder(LOCALE_FLASHUPDATE_CURRENTVERSIONSNAPSHOT, false, versionInfo.getType()));

	// check update
	//FIXME: allow update only when the rootfs is jffs2/squashfs
	updateSettings->addItem(new CMenuSeparator(CMenuSeparator::LINE));
	
	// offline
	updateSettings->addItem(new CMenuForwarder(LOCALE_FLASHUPDATE_UPDATEMODE_MANUAL, true, NULL, new CFlashUpdate(CFlashUpdate::UPDATEMODE_MANUAL), "", CRCInput::RC_yellow, NEUTRINO_ICON_BUTTON_YELLOW));

	// online
	updateSettings->addItem(new CMenuForwarder(LOCALE_FLASHUPDATE_UPDATEMODE_INTERNET, true, NULL, new CFlashUpdate(CFlashUpdate::UPDATEMODE_INTERNET), "", CRCInput::RC_blue, NEUTRINO_ICON_BUTTON_BLUE));
	
	// software update
	// updatesettings
	service.addItem(new CMenuForwarderExtended(LOCALE_SERVICEMENU_UPDATE, true, updateSettings, NULL, CRCInput::RC_blue, NEUTRINO_ICON_BUTTON_BLUE, NEUTRINO_ICON_MENUITEM_SERVICE, LOCALE_HELPTEXT_SOFTWAREUPDATE ));
}

// Init AudioPlayer Settings
#define AUDIOPLAYER_DISPLAY_ORDER_OPTION_COUNT 2
const CMenuOptionChooser::keyval AUDIOPLAYER_DISPLAY_ORDER_OPTIONS[AUDIOPLAYER_DISPLAY_ORDER_OPTION_COUNT] =
{
	{ CAudioPlayerGui::ARTIST_TITLE, LOCALE_AUDIOPLAYER_ARTIST_TITLE, NULL },
	{ CAudioPlayerGui::TITLE_ARTIST, LOCALE_AUDIOPLAYER_TITLE_ARTIST, NULL }
};

void CNeutrinoApp::InitAudioplayerSettings(CMenuWidget &audioplayerSettings)
{
	dprintf(DEBUG_NORMAL, "CNeutrinoApp::InitAudioplayerSettings\n");
	
	int shortcutAudioPlayer = 1;
	
	// intros
	audioplayerSettings.addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	audioplayerSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	
	// save settings
	audioplayerSettings.addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_SAVESETTINGSNOW, true, NULL, this, "savesettings", CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));
	audioplayerSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );

	// Audio Player
	audioplayerSettings.addItem(new CMenuOptionChooser(LOCALE_AUDIOPLAYER_DISPLAY_ORDER, &g_settings.audioplayer_display, AUDIOPLAYER_DISPLAY_ORDER_OPTIONS, AUDIOPLAYER_DISPLAY_ORDER_OPTION_COUNT, true, NULL, CRCInput::convertDigitToKey(shortcutAudioPlayer++), "", true ));

	// select ton pid
	audioplayerSettings.addItem(new CMenuOptionChooser(LOCALE_AUDIOPLAYER_FOLLOW, &g_settings.audioplayer_follow, MESSAGEBOX_NO_YES_OPTIONS, MESSAGEBOX_NO_YES_OPTION_COUNT, true, NULL, CRCInput::convertDigitToKey(shortcutAudioPlayer++) ));

	// select by title
	audioplayerSettings.addItem(new CMenuOptionChooser(LOCALE_AUDIOPLAYER_SELECT_TITLE_BY_NAME, &g_settings.audioplayer_select_title_by_name, MESSAGEBOX_NO_YES_OPTIONS, MESSAGEBOX_NO_YES_OPTION_COUNT, true, NULL, CRCInput::convertDigitToKey(shortcutAudioPlayer++) ));

	// repeat
	audioplayerSettings.addItem(new CMenuOptionChooser(LOCALE_AUDIOPLAYER_REPEAT_ON, &g_settings.audioplayer_repeat_on, MESSAGEBOX_NO_YES_OPTIONS, MESSAGEBOX_NO_YES_OPTION_COUNT, true, NULL, CRCInput::convertDigitToKey(shortcutAudioPlayer++) ));

	// hide playlist
	audioplayerSettings.addItem(new CMenuOptionChooser(LOCALE_AUDIOPLAYER_HIDE_PLAYLIST, &g_settings.audioplayer_hide_playlist, MESSAGEBOX_NO_YES_OPTIONS, MESSAGEBOX_NO_YES_OPTION_COUNT, true, NULL, CRCInput::convertDigitToKey(shortcutAudioPlayer++) ));

	// high prio
	audioplayerSettings.addItem(new CMenuOptionChooser(LOCALE_AUDIOPLAYER_HIGHPRIO, &g_settings.audioplayer_highprio, MESSAGEBOX_NO_YES_OPTIONS, MESSAGEBOX_NO_YES_OPTION_COUNT, true, NULL, CRCInput::convertDigitToKey(shortcutAudioPlayer++) ));

	// start dir
	audioplayerSettings.addItem(new CMenuForwarder(LOCALE_AUDIOPLAYER_DEFDIR, true, g_settings.network_nfs_audioplayerdir, this, "audioplayerdir", CRCInput::convertDigitToKey(shortcutAudioPlayer++)));
}

// InitPicViewerSettings
#define PICTUREVIEWER_SCALING_OPTION_COUNT 3
const CMenuOptionChooser::keyval PICTUREVIEWER_SCALING_OPTIONS[PICTUREVIEWER_SCALING_OPTION_COUNT] =
{
	{ CFrameBuffer::SIMPLE, LOCALE_PICTUREVIEWER_RESIZE_SIMPLE, NULL        },
	{ CFrameBuffer::COLOR , LOCALE_PICTUREVIEWER_RESIZE_COLOR_AVERAGE, NULL },
	{ CFrameBuffer::NONE  , LOCALE_PICTUREVIEWER_RESIZE_NONE, NULL         }
};

void CNeutrinoApp::InitPicViewerSettings(CMenuWidget &PicViewerSettings)
{
	dprintf(DEBUG_NORMAL, "CNeutrinoApp::InitPicViewerSettings\n");
	
	int shortcutPicViewer = 1;
	
	// intros
	PicViewerSettings.addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	PicViewerSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	
	// save settings
	PicViewerSettings.addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_SAVESETTINGSNOW, true, NULL, this, "savesettings", CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));
	PicViewerSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );

	// Pic Viewer Scaling
	PicViewerSettings.addItem(new CMenuOptionChooser(LOCALE_PICTUREVIEWER_SCALING, &g_settings.picviewer_scaling, PICTUREVIEWER_SCALING_OPTIONS, PICTUREVIEWER_SCALING_OPTION_COUNT, true, NULL, CRCInput::convertDigitToKey(shortcutPicViewer++),"", true ));

	// slide Timeout
	CStringInput * pic_timeout= new CStringInput(LOCALE_PICTUREVIEWER_SLIDE_TIME, g_settings.picviewer_slide_time, 2, NONEXISTANT_LOCALE, NONEXISTANT_LOCALE, "0123456789 ");
	PicViewerSettings.addItem(new CMenuForwarder(LOCALE_PICTUREVIEWER_SLIDE_TIME, true, g_settings.picviewer_slide_time, pic_timeout, NULL, CRCInput::convertDigitToKey(shortcutPicViewer++)));

	// Pic Viewer Default Dir
	PicViewerSettings.addItem(new CMenuForwarder(LOCALE_PICTUREVIEWER_DEFDIR, true, g_settings.network_nfs_picturedir, this, "picturedir", CRCInput::convertDigitToKey(shortcutPicViewer++)));
}

// Init Misc Settigns
#define MISCSETTINGS_FILESYSTEM_IS_UTF8_OPTION_COUNT 2
const CMenuOptionChooser::keyval MISCSETTINGS_FILESYSTEM_IS_UTF8_OPTIONS[MISCSETTINGS_FILESYSTEM_IS_UTF8_OPTION_COUNT] =
{
	{ 0, LOCALE_FILESYSTEM_IS_UTF8_OPTION_ISO8859_1, NULL },
	{ 1, LOCALE_FILESYSTEM_IS_UTF8_OPTION_UTF8, NULL      }
};

#define INFOBAR_SUBCHAN_DISP_POS_OPTIONS_COUNT 4
const CMenuOptionChooser::keyval  INFOBAR_SUBCHAN_DISP_POS_OPTIONS[INFOBAR_SUBCHAN_DISP_POS_OPTIONS_COUNT]=
{
	{ 0 , LOCALE_SETTINGS_POS_TOP_RIGHT, NULL },
	{ 1 , LOCALE_SETTINGS_POS_TOP_LEFT, NULL },
	{ 2 , LOCALE_SETTINGS_POS_BOTTOM_LEFT, NULL },
	{ 3 , LOCALE_SETTINGS_POS_BOTTOM_RIGHT, NULL }
};

#define SECTIONSD_SCAN_OPTIONS_COUNT 2
const CMenuOptionChooser::keyval SECTIONSD_SCAN_OPTIONS[SECTIONSD_SCAN_OPTIONS_COUNT] =
{
	{ 0, LOCALE_OPTIONS_OFF, NULL },
	{ 1, LOCALE_OPTIONS_ON, NULL }
};

// volumebar position
#define VOLUMEBAR_DISP_POS_OPTIONS_COUNT 6
const CMenuOptionChooser::keyval  VOLUMEBAR_DISP_POS_OPTIONS[VOLUMEBAR_DISP_POS_OPTIONS_COUNT]=
{
	{ 0 , LOCALE_SETTINGS_POS_TOP_RIGHT, NULL },
	{ 1 , LOCALE_SETTINGS_POS_TOP_LEFT, NULL },
	{ 2 , LOCALE_SETTINGS_POS_BOTTOM_LEFT, NULL },
	{ 3 , LOCALE_SETTINGS_POS_BOTTOM_RIGHT, NULL },
	{ 4 , LOCALE_SETTINGS_POS_DEFAULT_CENTER, NULL },
	{ 5 , LOCALE_SETTINGS_POS_HIGHER_CENTER, NULL }
};

#define MENU_CORNERSETTINGS_TYPE_OPTION_COUNT 2
const CMenuOptionChooser::keyval MENU_CORNERSETTINGS_TYPE_OPTIONS[MENU_CORNERSETTINGS_TYPE_OPTION_COUNT] =
{
	{ 0, LOCALE_EXTRA_ROUNDED_CORNERS_OFF, NULL },
	{ 1, LOCALE_EXTRA_ROUNDED_CORNERS_ON, NULL }	
};

CMenuOptionStringChooser * tzSelect;

void CNeutrinoApp::InitMiscSettings(CMenuWidget &miscSettings, CMenuWidget &miscSettingsGeneral, CMenuWidget &miscSettingsChannelList, CMenuWidget &miscSettingsEPG, CMenuWidget &miscSettingsFileBrowser )
{
	dprintf(DEBUG_NORMAL, "CNeutrinoApp::InitMiscSettings\n");
	
	int shortcutMiscSettings = 1;
	
	// intros
	miscSettingsGeneral.addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	miscSettingsGeneral.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	
	// save settings
	miscSettingsGeneral.addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_SAVESETTINGSNOW, true, NULL, this, "savesettings", CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));
	miscSettingsGeneral.addItem( new CMenuSeparator(CMenuSeparator::LINE) );

	// rc delay
	CMenuOptionChooser * m1 = new CMenuOptionChooser(LOCALE_MISCSETTINGS_SHUTDOWN_REAL_RCDELAY, &g_settings.shutdown_real_rcdelay, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, !g_settings.shutdown_real);

	CMiscNotifier * miscNotifier = new CMiscNotifier( m1 );

	// shutdown real
	miscSettingsGeneral.addItem(new CMenuOptionChooser(LOCALE_MISCSETTINGS_SHUTDOWN_REAL, &g_settings.shutdown_real, OPTIONS_OFF1_ON0_OPTIONS, OPTIONS_OFF1_ON0_OPTION_COUNT, true, miscNotifier ));

	// delayed shutdown
	miscSettingsGeneral.addItem(m1);

	// delay counter
	CStringInput * miscSettings_shutdown_count = new CStringInput(LOCALE_MISCSETTINGS_SHUTDOWN_COUNT, g_settings.shutdown_count, 3, LOCALE_MISCSETTINGS_SHUTDOWN_COUNT_HINT1, LOCALE_MISCSETTINGS_SHUTDOWN_COUNT_HINT2, "0123456789 ");
	miscSettingsGeneral.addItem(new CMenuForwarder(LOCALE_MISCSETTINGS_SHUTDOWN_COUNT, true, g_settings.shutdown_count, miscSettings_shutdown_count));

	// start to standby
	miscSettingsGeneral.addItem(new CMenuOptionChooser(LOCALE_EXTRA_STARTSTANDBY, &g_settings.power_standby, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true));

	// sig/snr
	miscSettingsGeneral.addItem(new CMenuOptionChooser(LOCALE_MISCSETTINGS_INFOBAR_SAT_DISPLAY, &g_settings.infobar_sat_display, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true));
	
	// radio text	
	miscSettingsGeneral.addItem(new CMenuOptionChooser(LOCALE_MISCSETTINGS_INFOBAR_RADIOTEXT, &g_settings.radiotext_enable, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true, this ));
	
	// logos dir
	miscSettingsGeneral.addItem( new CMenuForwarder(LOCALE_MISCSETTINGS_LOGOSDIR, true, g_settings.logos_dir, this, "logos_dir" ) );
	
	// epgplus logos
	miscSettingsGeneral.addItem(new CMenuOptionChooser(LOCALE_MISCSETTINGS_EPGPLUS_SHOW_LOGOS, &g_settings.epgplus_show_logo, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true ));
	
	// infobar show channelname
	miscSettingsGeneral.addItem(new CMenuOptionChooser(LOCALE_MISCSETTINGS_INFOBAR_SHOW_CHANNELNAME, &g_settings.show_channelname, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true ));
	
	// recording screenshot
	miscSettingsGeneral.addItem(new CMenuOptionChooser(LOCALE_MISCSETTINGS_RECORDING_SCREENSHOT, &g_settings.recording_screenshot, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true ));

	// subchan pos
	miscSettingsGeneral.addItem(new CMenuOptionChooser(LOCALE_INFOVIEWER_SUBCHAN_DISP_POS, &g_settings.infobar_subchan_disp_pos, INFOBAR_SUBCHAN_DISP_POS_OPTIONS, INFOBAR_SUBCHAN_DISP_POS_OPTIONS_COUNT, true, NULL, CRCInput::RC_nokey, "", true));
	
	// volumebar position
	miscSettingsGeneral.addItem(new CMenuOptionChooser(LOCALE_EXTRA_VOLUME_POS, &g_settings.volume_pos, VOLUMEBAR_DISP_POS_OPTIONS, VOLUMEBAR_DISP_POS_OPTIONS_COUNT, true, NULL, CRCInput::RC_nokey, "", true ));
	
	// corners
	miscSettingsGeneral.addItem(new CMenuOptionChooser(LOCALE_EXTRA_ROUNDED_CORNERS, &g_settings.rounded_corners, MENU_CORNERSETTINGS_TYPE_OPTIONS, MENU_CORNERSETTINGS_TYPE_OPTION_COUNT, true));
	
	// volume bar steps
	CStringInput * audio_step = new CStringInput(LOCALE_AUDIOMENU_VOLUMEBAR_AUDIOSTEPS,g_settings.audio_step, 2, NONEXISTANT_LOCALE, NONEXISTANT_LOCALE, "0123456789 " );
	CMenuForwarder *as = new CMenuForwarder(LOCALE_AUDIOMENU_VOLUMEBAR_AUDIOSTEPS, true, g_settings.audio_step, audio_step );
	miscSettingsGeneral.addItem(as);

	// timezone
	xmlDocPtr parser;

	parser = parseXmlFile("/etc/timezone.xml");
	if (parser != NULL) 
	{	
		tzSelect = new CMenuOptionStringChooser(LOCALE_MAINSETTINGS_TIMEZONE, g_settings.timezone, true, new CTZChangeNotifier(), CRCInput::RC_nokey, "", true);

		xmlNodePtr search = xmlDocGetRootElement(parser)->xmlChildrenNode;
		bool found = false;

		while (search) 
		{
			if (!strcmp(xmlGetName(search), "zone")) 
			{
				std::string name = xmlGetAttribute(search, (char *) "name");
				std::string zone = xmlGetAttribute(search, (char *) "zone");
				
				//printf("Timezone: %s -> %s\n", name.c_str(), zone.c_str());
				
				tzSelect->addOption(name.c_str());
				found = true;
			}
			search = search->xmlNextNode;
		}

		if(found)
			miscSettingsGeneral.addItem(tzSelect);
		else 
		{
			delete tzSelect;
			tzSelect = NULL;
		}	
		xmlFreeDoc(parser);
	}
	
	// reset factory setup
	miscSettingsGeneral.addItem(new CMenuSeparator(CMenuSeparator::LINE));
	
	CDataResetNotifier * resetNotifier = new CDataResetNotifier();
	miscSettingsGeneral.addItem(new CMenuForwarder(LOCALE_MISCSETTINGS_RESET, true, NULL, resetNotifier, "factory", CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN ));
	miscSettingsGeneral.addItem(new CMenuForwarder(LOCALE_SETTINGS_BACKUP,  true, NULL, resetNotifier, "backup", CRCInput::RC_yellow, NEUTRINO_ICON_BUTTON_YELLOW ));
	miscSettingsGeneral.addItem(new CMenuForwarder(LOCALE_SETTINGS_RESTORE, true, NULL, resetNotifier, "restore", CRCInput::RC_blue, NEUTRINO_ICON_BUTTON_BLUE ));

	miscSettings.addItem(new CMenuForwarderExtended(LOCALE_MISCSETTINGS_GENERAL, true, &miscSettingsGeneral, NULL, CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED, NEUTRINO_ICON_MENUITEM_MAINSETTINGS, LOCALE_HELPTEXT_MISCSETTINGSGENERAL ));
	
	//channels settings
	int shortcutMiscChannel = 1;
	
	// intros
	miscSettingsChannelList.addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	miscSettingsChannelList.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	
	// save settings
	miscSettingsChannelList.addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_SAVESETTINGSNOW, true, NULL, this, "savesettings", CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));
	miscSettingsChannelList.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	
	// HD list
	miscSettingsChannelList.addItem(new CMenuOptionChooser(LOCALE_CHANNELLIST_MAKE_HDLIST, &g_settings.make_hd_list, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true, this, CRCInput::convertDigitToKey(shortcutMiscChannel++) ));
	
	// virtual zap
	miscSettingsChannelList.addItem(new CMenuOptionChooser(LOCALE_MISCSETTINGS_VIRTUAL_ZAP_MODE, &g_settings.virtual_zap_mode, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true, NULL, CRCInput::convertDigitToKey(shortcutMiscChannel++) ));
	
	// zap cycle
	miscSettingsChannelList.addItem(new CMenuOptionChooser(LOCALE_EXTRA_ZAP_CYCLE, &g_settings.zap_cycle, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true, NULL, CRCInput::convertDigitToKey(shortcutMiscChannel++) ));
	
	// sms channel
	//miscSettingsChannelList.addItem(new CMenuOptionChooser(LOCALE_EXTRA_SMS_CHANNEL, &g_settings.sms_channel, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, false, NULL, CRCInput::convertDigitToKey(shortcutMiscChannel++) ));
	
	// channellist ca
	miscSettingsChannelList.addItem(new CMenuOptionChooser(LOCALE_CHANNELLIST_SHOWCA, &g_settings.channellist_ca, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true, NULL, CRCInput::convertDigitToKey(shortcutMiscChannel++) ));
	
	// extended channel list
	miscSettingsChannelList.addItem(new CMenuOptionChooser(LOCALE_CHANNELLIST_EXTENDED, &g_settings.channellist_extended, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true, NULL, CRCInput::convertDigitToKey(shortcutMiscChannel++) ));
	
	//
	getZapitConfig(&zapitCfg);
	
	miscSettingsChannelList.addItem(new CMenuOptionChooser(LOCALE_EXTRA_ZAPIT_MAKE_BOUQUET, (int *)&zapitCfg.makeRemainingChannelsBouquet, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true, NULL, CRCInput::convertDigitToKey(shortcutMiscChannel++) ));
	miscSettingsChannelList.addItem( new CMenuOptionChooser(LOCALE_ZAPIT_SCANSDT, (int *)&zapitCfg.scanSDT, SECTIONSD_SCAN_OPTIONS, SECTIONSD_SCAN_OPTIONS_COUNT, true, NULL, CRCInput::convertDigitToKey(shortcutMiscChannel++)) );

	miscSettings.addItem(new CMenuForwarderExtended(LOCALE_MISCSETTINGS_CHANNELLIST, true, &miscSettingsChannelList, NULL, CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN, NEUTRINO_ICON_MENUITEM_MAINSETTINGS, LOCALE_HELPTEXT_MISCSETTINGSCHANNELLIST ));

	// epg settings
	int shortcutMiscEpg = 1;
	
	// intros
	miscSettingsEPG.addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	miscSettingsEPG.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	
	// save settings
	miscSettingsEPG.addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_SAVESETTINGSNOW, true, NULL, this, "savesettings", CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));
	miscSettingsEPG.addItem( new CMenuSeparator(CMenuSeparator::LINE) );

	// save epg
	CSectionsdConfigNotifier* sectionsdConfigNotifier = new CSectionsdConfigNotifier;
	miscSettingsEPG.addItem(new CMenuOptionChooser(LOCALE_MISCSETTINGS_EPG_SAVE, &g_settings.epg_save, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true, NULL, CRCInput::convertDigitToKey(shortcutMiscEpg++) ));

	// epg cache
        CStringInput * miscSettings_epg_cache = new CStringInput(LOCALE_MISCSETTINGS_EPG_CACHE, &g_settings.epg_cache, 2,LOCALE_MISCSETTINGS_EPG_CACHE_HINT1, LOCALE_MISCSETTINGS_EPG_CACHE_HINT2 , "0123456789 ", sectionsdConfigNotifier);
        miscSettingsEPG.addItem(new CMenuForwarder(LOCALE_MISCSETTINGS_EPG_CACHE, true, g_settings.epg_cache, miscSettings_epg_cache, NULL, CRCInput::convertDigitToKey(shortcutMiscEpg++) ));

	// extended epg cache
        CStringInput * miscSettings_epg_cache_e = new CStringInput(LOCALE_MISCSETTINGS_EPG_EXTENDEDCACHE, &g_settings.epg_extendedcache, 3,LOCALE_MISCSETTINGS_EPG_EXTENDEDCACHE_HINT1, LOCALE_MISCSETTINGS_EPG_EXTENDEDCACHE_HINT2 , "0123456789 ", sectionsdConfigNotifier);
        miscSettingsEPG.addItem(new CMenuForwarder(LOCALE_MISCSETTINGS_EPG_EXTENDEDCACHE, true, g_settings.epg_extendedcache, miscSettings_epg_cache_e, NULL, CRCInput::convertDigitToKey(shortcutMiscEpg++)));

	// old events
        CStringInput * miscSettings_epg_old_events = new CStringInput(LOCALE_MISCSETTINGS_EPG_OLD_EVENTS, &g_settings.epg_old_events, 2,LOCALE_MISCSETTINGS_EPG_OLD_EVENTS_HINT1, LOCALE_MISCSETTINGS_EPG_OLD_EVENTS_HINT2 , "0123456789 ", sectionsdConfigNotifier);
        miscSettingsEPG.addItem(new CMenuForwarder(LOCALE_MISCSETTINGS_EPG_OLD_EVENTS, true, g_settings.epg_old_events, miscSettings_epg_old_events, NULL, CRCInput::convertDigitToKey(shortcutMiscEpg++) ));

	// max epg events
        CStringInput * miscSettings_epg_max_events = new CStringInput(LOCALE_MISCSETTINGS_EPG_MAX_EVENTS, &g_settings.epg_max_events, 5,LOCALE_MISCSETTINGS_EPG_MAX_EVENTS_HINT1, LOCALE_MISCSETTINGS_EPG_MAX_EVENTS_HINT2 , "0123456789 ", sectionsdConfigNotifier);
        miscSettingsEPG.addItem(new CMenuForwarder(LOCALE_MISCSETTINGS_EPG_MAX_EVENTS, true, g_settings.epg_max_events, miscSettings_epg_max_events, NULL, CRCInput::convertDigitToKey(shortcutMiscEpg++) ));

	// epg save dir
        miscSettingsEPG.addItem(new CMenuForwarder(LOCALE_MISCSETTINGS_EPG_DIR, true, g_settings.epg_dir, this, "epgdir", CRCInput::convertDigitToKey(shortcutMiscEpg++) ));
	
	// epglang
	miscSettingsEPG.addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_MISCSETTINGS_PREF_EPGS_HEAD));
	
	CMenuOptionStringChooser * epglangSelect[3];
	CEPGlangSelectNotifier * EPGlangNotifier = new CEPGlangSelectNotifier();
	
	for(int i = 0; i < 3; i++) 
	{
		epglangSelect[i] = new CMenuOptionStringChooser(LOCALE_MISCSETTINGS_PREF_EPGS, g_settings.pref_epgs[i], true, EPGlangNotifier, CRCInput::RC_nokey, "", true);
		std::map<std::string, std::string>::const_iterator it;
		
		epglangSelect[i]->addOption("");
		for(it = iso639rev.begin(); it != iso639rev.end(); it++) 
			epglangSelect[i]->addOption(it->first.c_str());
	}
	
	// epglang
	for(int i = 0; i < 3; i++) 
		miscSettingsEPG.addItem(epglangSelect[i]);
	///

	miscSettings.addItem(new CMenuForwarderExtended(LOCALE_MISCSETTINGS_EPG_HEAD, true, &miscSettingsEPG, NULL, CRCInput::RC_yellow, NEUTRINO_ICON_BUTTON_YELLOW, NEUTRINO_ICON_MENUITEM_MAINSETTINGS, LOCALE_HELPTEXT_MISCSETTINGSEPG ));

	// filebrowser settings
	int shortcutMiscFileBrowser = 1;
	
	// intros
	miscSettingsFileBrowser.addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	miscSettingsFileBrowser.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	
	// save settings
	miscSettingsFileBrowser.addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_SAVESETTINGSNOW, true, NULL, this, "savesettings", CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));
	miscSettingsFileBrowser.addItem( new CMenuSeparator(CMenuSeparator::LINE) );

	// UTF 
	miscSettingsFileBrowser.addItem(new CMenuOptionChooser(LOCALE_FILESYSTEM_IS_UTF8, &g_settings.filesystem_is_utf8, MISCSETTINGS_FILESYSTEM_IS_UTF8_OPTIONS, MISCSETTINGS_FILESYSTEM_IS_UTF8_OPTION_COUNT, true, NULL, CRCInput::convertDigitToKey(shortcutMiscFileBrowser++), "", true ));

	// show rights
	miscSettingsFileBrowser.addItem(new CMenuOptionChooser(LOCALE_FILEBROWSER_SHOWRIGHTS, &g_settings.filebrowser_showrights, MESSAGEBOX_NO_YES_OPTIONS, MESSAGEBOX_NO_YES_OPTION_COUNT, true, NULL, CRCInput::convertDigitToKey(shortcutMiscFileBrowser++) ));

	// deny dir
	miscSettingsFileBrowser.addItem(new CMenuOptionChooser(LOCALE_FILEBROWSER_DENYDIRECTORYLEAVE, &g_settings.filebrowser_denydirectoryleave, MESSAGEBOX_NO_YES_OPTIONS, MESSAGEBOX_NO_YES_OPTION_COUNT, true, NULL, CRCInput::convertDigitToKey(shortcutMiscFileBrowser++) ));

	miscSettings.addItem(new CMenuForwarderExtended(LOCALE_MISCSETTINGS_FILEBROWSER, true, &miscSettingsFileBrowser, NULL, CRCInput::RC_blue, NEUTRINO_ICON_BUTTON_BLUE, NEUTRINO_ICON_MENUITEM_MAINSETTINGS, LOCALE_HELPTEXT_MISCSETTINGSFILEBROWSER ));
	
	// zapit setup (start channel)
	miscSettings.addItem(new CMenuForwarderExtended(LOCALE_MISCSETTINGS_ZAPIT, true, new CZapitSetup(), NULL, CRCInput::convertDigitToKey(shortcutMiscSettings++), NULL, NEUTRINO_ICON_MENUITEM_MAINSETTINGS, LOCALE_HELPTEXT_MISCSETTINGSZAPITSETUP ));
	
	// psi setup	
	CPSISetup * chPSISetup = new CPSISetup(LOCALE_VIDEOMENU_PSISETUP, &g_settings.contrast, &g_settings.saturation, &g_settings.brightness, &g_settings.tint);
	miscSettings.addItem( new CMenuForwarderExtended(LOCALE_VIDEOMENU_PSISETUP, true, chPSISetup, NULL, CRCInput::convertDigitToKey(shortcutMiscSettings++), NULL, NEUTRINO_ICON_MENUITEM_MAINSETTINGS, LOCALE_HELPTEXT_MISCSETTINGSPSISETUP ));
}

// Init Language Settings
void CNeutrinoApp::InitLanguageSettings(CMenuWidget &languageSettings)
{
	dprintf(DEBUG_NORMAL, "CNeutrinoApp::InitLanguageSettings\n");
	
	// intros
	languageSettings.addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	languageSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE));

	struct dirent **namelist;
	int n;

	//printf("scanning locale dir now....(perhaps)\n");

	char *path[] = {(char *) DATADIR "/neutrino/locale", (char *) CONFIGDIR "/locale"};

	for(int p = 0; p < 2; p++) 
	{
		n = scandir(path[p], &namelist, 0, alphasort);
		
		if(n > 0)
		{
			for(int count = 0; count < n; count++) 
			{
				char * locale = strdup(namelist[count]->d_name);
				char * pos = strstr(locale, ".locale");

				if(pos != NULL) 
				{
					*pos = '\0';
				
					CMenuOptionLanguageChooser* oj = new CMenuOptionLanguageChooser((char*)locale, this, locale);
					oj->addOption(locale);
					languageSettings.addItem( oj );
				} 
				else
					free(locale);
				free(namelist[count]);
			}
			free(namelist);
		}
	}
}

// Init Network Settings
#define OPTIONS_NTPENABLE_OPTION_COUNT 2
const CMenuOptionChooser::keyval OPTIONS_NTPENABLE_OPTIONS[OPTIONS_NTPENABLE_OPTION_COUNT] =
{
	{ 0, NONEXISTANT_LOCALE, "DVB" },
	{ 1, NONEXISTANT_LOCALE, "NTP" }
};

#define OPTIONS_WLAN_SECURITY_OPTION_COUNT 2
const CMenuOptionChooser::keyval OPTIONS_WLAN_SECURITY_OPTIONS[OPTIONS_WLAN_SECURITY_OPTION_COUNT] =
{
        { 0, NONEXISTANT_LOCALE, "WPA" },
        { 1, NONEXISTANT_LOCALE, "WPA2"  }
};

static int my_filter(const struct dirent * dent)
{
	if(dent->d_name[0] == 'l' && dent->d_name[1] == 'o')
		return 0;
	if(dent->d_name[0] == '.')
		return 0;
	
	return 1;
}

void CNeutrinoApp::InitNetworkSettings(CMenuWidget &networkSettings)
{
	dprintf(DEBUG_NORMAL, "CNeutrinoApp::InitNetworkSettings\n");
	
	struct dirent **namelist;

	//if select
	int ifcount = scandir("/sys/class/net", &namelist, my_filter, alphasort);

	CMenuOptionStringChooser * ifSelect = new CMenuOptionStringChooser(LOCALE_NETWORKMENU_SELECT_IF, g_settings.ifname, ifcount > 1, this, CRCInput::RC_nokey, "", true);

	bool found = false;

	for(int i = 0; i < ifcount; i++) 
	{
		ifSelect->addOption(namelist[i]->d_name);
		
		if(strcmp(g_settings.ifname, namelist[i]->d_name) == 0)
			found = true;
		free(namelist[i]);
	}

	if (ifcount >= 0)
		free(namelist);

	if(!found)
		strcpy(g_settings.ifname, "eth0");
	
	networkConfig.readConfig(g_settings.ifname);

	network_hostname	= networkConfig.hostname;
	mac_addr		= networkConfig.mac_addr;
	network_ssid		= networkConfig.ssid;
	network_key		= networkConfig.key;
	network_encryption	= (networkConfig.encryption == "WPA") ? 0 : 1;
	
	//eth id
	CMenuForwarder * mac = new CMenuForwarder("MAC", false, mac_addr);
	
	CIPInput * networkSettings_NetworkIP  = new CIPInput(LOCALE_NETWORKMENU_IPADDRESS, networkConfig.address, LOCALE_IPSETUP_HINT_1, LOCALE_IPSETUP_HINT_2, MyIPChanger);
	CIPInput * networkSettings_NetMask    = new CIPInput(LOCALE_NETWORKMENU_NETMASK, networkConfig.netmask, LOCALE_IPSETUP_HINT_1, LOCALE_IPSETUP_HINT_2);
	CIPInput * networkSettings_Broadcast  = new CIPInput(LOCALE_NETWORKMENU_BROADCAST, networkConfig.broadcast, LOCALE_IPSETUP_HINT_1, LOCALE_IPSETUP_HINT_2);
	CIPInput * networkSettings_Gateway    = new CIPInput(LOCALE_NETWORKMENU_GATEWAY, networkConfig.gateway, LOCALE_IPSETUP_HINT_1, LOCALE_IPSETUP_HINT_2);
	CIPInput * networkSettings_NameServer = new CIPInput(LOCALE_NETWORKMENU_NAMESERVER, networkConfig.nameserver, LOCALE_IPSETUP_HINT_1, LOCALE_IPSETUP_HINT_2);
	
	//hostname
	CStringInputSMS * networkSettings_Hostname = new CStringInputSMS(LOCALE_NETWORKMENU_HOSTNAME, &network_hostname);

        CSectionsdConfigNotifier * sectionsdConfigNotifier = new CSectionsdConfigNotifier;
	// ntp server
        CStringInputSMS * networkSettings_NtpServer = new CStringInputSMS(LOCALE_NETWORKMENU_NTPSERVER, &g_settings.network_ntpserver, MAX_INPUT_CHARS, LOCALE_NETWORKMENU_NTPSERVER_HINT1, LOCALE_NETWORKMENU_NTPSERVER_HINT2, "abcdefghijklmnopqrstuvwxyz0123456789-. ", sectionsdConfigNotifier);
        CStringInput * networkSettings_NtpRefresh = new CStringInput(LOCALE_NETWORKMENU_NTPREFRESH, &g_settings.network_ntprefresh, 3, LOCALE_NETWORKMENU_NTPREFRESH_HINT1, LOCALE_NETWORKMENU_NTPREFRESH_HINT2 , "0123456789 ", sectionsdConfigNotifier);

	CMenuForwarder * m0 = new CMenuForwarder(LOCALE_NETWORKMENU_SETUPNOW, true, NULL, this, "network", CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN);
	CMenuForwarder * m1 = new CMenuForwarder(LOCALE_NETWORKMENU_IPADDRESS, networkConfig.inet_static, networkConfig.address   , networkSettings_NetworkIP );
	CMenuForwarder * m2 = new CMenuForwarder(LOCALE_NETWORKMENU_NETMASK, networkConfig.inet_static, networkConfig.netmask   , networkSettings_NetMask   );
	CMenuForwarder * m3 = new CMenuForwarder(LOCALE_NETWORKMENU_BROADCAST, networkConfig.inet_static, networkConfig.broadcast , networkSettings_Broadcast );
	CMenuForwarder * m4 = new CMenuForwarder(LOCALE_NETWORKMENU_GATEWAY, networkConfig.inet_static, networkConfig.gateway   , networkSettings_Gateway   );
	CMenuForwarder * m5 = new CMenuForwarder(LOCALE_NETWORKMENU_NAMESERVER, networkConfig.inet_static, networkConfig.nameserver, networkSettings_NameServer);
        CMenuForwarder * m6 = new CMenuForwarder( LOCALE_NETWORKMENU_NTPSERVER, true, g_settings.network_ntpserver, networkSettings_NtpServer );
        CMenuForwarder * m7 = new CMenuForwarder( LOCALE_NETWORKMENU_NTPREFRESH, true, g_settings.network_ntprefresh, networkSettings_NtpRefresh );
	
	CMenuForwarder * m8 = new CMenuForwarder(LOCALE_NETWORKMENU_HOSTNAME, true, network_hostname, networkSettings_Hostname);

	CDHCPNotifier * dhcpNotifier = new CDHCPNotifier(m1, m2, m3, m4, m5);

	network_automatic_start = networkConfig.automatic_start ? 1 : 0;
	CMenuOptionChooser * oj = new CMenuOptionChooser(LOCALE_NETWORKMENU_SETUPONSTARTUP, &network_automatic_start, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true);

	// intros
	networkSettings.addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	networkSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	
	// save settings
	networkSettings.addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_SAVESETTINGSNOW, true, NULL, this, "savesettings", CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));
	networkSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );

	networkSettings.addItem( oj );
	networkSettings.addItem(new CMenuForwarder(LOCALE_NETWORKMENU_TEST, true, NULL, this, "networktest"));
	networkSettings.addItem(new CMenuForwarder(LOCALE_NETWORKMENU_SHOW, true, NULL, this, "networkshow", CRCInput::RC_info, NEUTRINO_ICON_BUTTON_HELP_SMALL));
	
	// setup on start
	networkSettings.addItem( m0 );
	
	// mac id
	networkSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE));
	networkSettings.addItem(mac);	//eth id
	
	// if select
	if(ifcount)
		networkSettings.addItem(ifSelect);	//if select
	else
		delete ifSelect;

	networkSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE));

	network_dhcp = networkConfig.inet_static ? 0 : 1;
	oj = new CMenuOptionChooser(LOCALE_NETWORKMENU_DHCP, &network_dhcp, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true, dhcpNotifier);
	networkSettings.addItem(oj);
	networkSettings.addItem( m8);	//hostname
	networkSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE));

	networkSettings.addItem( m1);
	networkSettings.addItem( m2);
	networkSettings.addItem( m3);

	networkSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE));
	networkSettings.addItem( m4);
	networkSettings.addItem( m5);
	
	//
	if(ifcount > 1) // if there is only one, its probably wired
	{
		//ssid
		CStringInputSMS * networkSettings_ssid = new CStringInputSMS(LOCALE_NETWORKMENU_SSID, &network_ssid);
		CMenuForwarder * m9 = new CMenuForwarder(LOCALE_NETWORKMENU_SSID, networkConfig.wireless, network_ssid , networkSettings_ssid );
		//key
		CStringInputSMS *networkSettings_key = new CStringInputSMS(LOCALE_NETWORKMENU_PASSWORD, &network_key);
		CMenuForwarder *m10 = new CMenuForwarder(LOCALE_NETWORKMENU_PASSWORD, networkConfig.wireless, network_key , networkSettings_key );

		wlanEnable[0] = m9;
		wlanEnable[1] = m10;

		networkSettings.addItem( m9);	//ssid
		networkSettings.addItem( m10);	//key

		//encryption
		CMenuOptionChooser * m11 = new CMenuOptionChooser(LOCALE_NETWORKMENU_WLAN_SECURITY, &network_encryption, OPTIONS_WLAN_SECURITY_OPTIONS, OPTIONS_WLAN_SECURITY_OPTION_COUNT, true);
		wlanEnable[2] = m11;
		networkSettings.addItem( m11); //encryption
		networkSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE));
	}
	
	// ntp
	networkSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_NETWORKMENU_NTPTITLE));
	networkSettings.addItem(new CMenuOptionChooser(LOCALE_NETWORKMENU_NTPENABLE, &g_settings.network_ntpenable, OPTIONS_NTPENABLE_OPTIONS, OPTIONS_NTPENABLE_OPTION_COUNT, true, sectionsdConfigNotifier));
        networkSettings.addItem( m6);
        networkSettings.addItem( m7);
	
	//proxyserver submenu
	networkSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE));
	networkSettings.addItem(new CMenuForwarder(LOCALE_FLASHUPDATE_PROXYSERVER_SEP, true, NULL, new CProxySetup(LOCALE_FLASHUPDATE_PROXYSERVER_SEP), NULL, CRCInput::RC_nokey, NULL));

	networkSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_NETWORKMENU_MOUNT));
	networkSettings.addItem(new CMenuForwarder(LOCALE_NFS_MOUNT , true, NULL, new CNFSMountGui(), NULL, CRCInput::RC_yellow, NEUTRINO_ICON_BUTTON_YELLOW));
	networkSettings.addItem(new CMenuForwarder(LOCALE_NFS_UMOUNT, true, NULL, new CNFSUmountGui(), NULL, CRCInput::RC_blue, NEUTRINO_ICON_BUTTON_BLUE));
}

// Init Recording Settings
void CNeutrinoApp::InitRecordingSettings(CMenuWidget &recordingSettings)
{
	dprintf(DEBUG_NORMAL, "CNeutrinoApp::InitRecordingSettings\n");
	
	int rec_pre = 0;
	int rec_post = 0;
	
	g_Timerd->getRecordingSafety(rec_pre, rec_post);

	sprintf(g_settings.record_safety_time_before, "%02d", rec_pre/60);
	sprintf(g_settings.record_safety_time_after, "%02d", rec_post/60);

	CRecordingSafetyNotifier *RecordingSafetyNotifier = new CRecordingSafetyNotifier;

	//safety time befor
	CStringInput * timerBefore = new CStringInput(LOCALE_TIMERSETTINGS_RECORD_SAFETY_TIME_BEFORE, g_settings.record_safety_time_before, 2, LOCALE_TIMERSETTINGS_RECORD_SAFETY_TIME_BEFORE_HINT_1, LOCALE_TIMERSETTINGS_RECORD_SAFETY_TIME_BEFORE_HINT_2,"0123456789 ", RecordingSafetyNotifier);
	CMenuForwarder *fTimerBefore = new CMenuForwarder(LOCALE_TIMERSETTINGS_RECORD_SAFETY_TIME_BEFORE, true, g_settings.record_safety_time_before, timerBefore);

	//safety time after
	CStringInput * timerAfter = new CStringInput(LOCALE_TIMERSETTINGS_RECORD_SAFETY_TIME_AFTER, g_settings.record_safety_time_after, 2, LOCALE_TIMERSETTINGS_RECORD_SAFETY_TIME_AFTER_HINT_1, LOCALE_TIMERSETTINGS_RECORD_SAFETY_TIME_AFTER_HINT_2,"0123456789 ", RecordingSafetyNotifier);
	CMenuForwarder *fTimerAfter = new CMenuForwarder(LOCALE_TIMERSETTINGS_RECORD_SAFETY_TIME_AFTER, true, g_settings.record_safety_time_after, timerAfter);

	//audiopids
	g_settings.recording_audio_pids_std = ( g_settings.recording_audio_pids_default & TIMERD_APIDS_STD ) ? 1 : 0 ;
	g_settings.recording_audio_pids_alt = ( g_settings.recording_audio_pids_default & TIMERD_APIDS_ALT ) ? 1 : 0 ;
	g_settings.recording_audio_pids_ac3 = ( g_settings.recording_audio_pids_default & TIMERD_APIDS_AC3 ) ? 1 : 0 ;
	
	CRecAPIDSettingsNotifier * an = new CRecAPIDSettingsNotifier;

	//default
	CMenuOptionChooser* aoj1 = new CMenuOptionChooser(LOCALE_RECORDINGMENU_APIDS_STD, &g_settings.recording_audio_pids_std, MESSAGEBOX_NO_YES_OPTIONS, MESSAGEBOX_NO_YES_OPTION_COUNT, true, an);

	//alt
	CMenuOptionChooser* aoj2 = new CMenuOptionChooser(LOCALE_RECORDINGMENU_APIDS_ALT, &g_settings.recording_audio_pids_alt, MESSAGEBOX_NO_YES_OPTIONS, MESSAGEBOX_NO_YES_OPTION_COUNT, true, an);

	//ac3
	CMenuOptionChooser* aoj3 = new CMenuOptionChooser(LOCALE_RECORDINGMENU_APIDS_AC3, &g_settings.recording_audio_pids_ac3, MESSAGEBOX_NO_YES_OPTIONS, MESSAGEBOX_NO_YES_OPTION_COUNT, true, an);

	//epg in name format
	CMenuOptionChooser* oj11 = new CMenuOptionChooser(LOCALE_RECORDINGMENU_EPG_FOR_FILENAME, &g_settings.recording_epg_for_filename, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true);

	// save in channeldir
	CMenuOptionChooser* oj13 = new CMenuOptionChooser(LOCALE_RECORDINGMENU_SAVE_IN_CHANNELDIR, &g_settings.recording_save_in_channeldir, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true);

	//RecDir
	CMenuForwarder* fRecDir = new CMenuForwarder(LOCALE_RECORDINGMENU_DEFDIR, true, g_settings.network_nfs_recordingdir, this, "recordingdir");

	// intros
	recordingSettings.addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	recordingSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	
	// save settings
	recordingSettings.addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_SAVESETTINGSNOW, true, NULL, this, "savesettings", CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));
	recordingSettings.addItem(new CMenuForwarder(LOCALE_RECORDINGMENU_SETUPNOW, true, NULL, this, "recording", CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN));

	recordingSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_TIMERSETTINGS_SEPARATOR));
	recordingSettings.addItem(fTimerBefore);
	recordingSettings.addItem(fTimerAfter);

	//apids
	recordingSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_RECORDINGMENU_APIDS));
	recordingSettings.addItem(aoj1);
	recordingSettings.addItem(aoj2);
	recordingSettings.addItem(aoj3);

	//
	recordingSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE));
	//epg in name format
	recordingSettings.addItem(oj11);
	
	// save in channeldir
	recordingSettings.addItem(oj13);

	recordingSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE));

	//recdir
	recordingSettings.addItem(fRecDir);
	
	// timeshift
	recordingSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_EXTRA_TIMESHIFT));
	
	// record time
	recordingSettings.addItem(new CMenuOptionNumberChooser(LOCALE_EXTRA_RECORD_TIME, &g_settings.record_hours, true, 1, 24, NULL) );

	// timeshift
	if (recDir != NULL)
	{
		// permanent timeshift
		recordingSettings.addItem(new CMenuOptionChooser(LOCALE_EXTRA_AUTO_TIMESHIFT, &g_settings.auto_timeshift, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true, this));
	}
}

/* for streaming settings menu */
#define STREAMINGMENU_STREAMING_TRANSCODE_VIDEO_CODEC_OPTION_COUNT 2
const CMenuOptionChooser::keyval STREAMINGMENU_STREAMING_TRANSCODE_VIDEO_CODEC_OPTIONS[STREAMINGMENU_STREAMING_TRANSCODE_VIDEO_CODEC_OPTION_COUNT] =
{
	{ 0, LOCALE_STREAMINGMENU_MPEG1, NULL },
	{ 1, LOCALE_STREAMINGMENU_MPEG2, NULL }
};

#define STREAMINGMENU_STREAMING_RESOLUTION_OPTION_COUNT 5
const CMenuOptionChooser::keyval STREAMINGMENU_STREAMING_RESOLUTION_OPTIONS[STREAMINGMENU_STREAMING_RESOLUTION_OPTION_COUNT] =
{
	{ 0, LOCALE_STREAMINGMENU_352X288, NULL },
	{ 1, LOCALE_STREAMINGMENU_352X576, NULL },
	{ 2, LOCALE_STREAMINGMENU_480X576, NULL },
	{ 3, LOCALE_STREAMINGMENU_704X576, NULL },
	{ 4, LOCALE_STREAMINGMENU_704X288, NULL }
};

#define STREAMINGMENU_STREAMING_TYPE_OPTION_COUNT 2
const CMenuOptionChooser::keyval STREAMINGMENU_STREAMING_TYPE_OPTIONS[STREAMINGMENU_STREAMING_TYPE_OPTION_COUNT] =
{
	{ 0, LOCALE_STREAMINGMENU_OFF, NULL },
	{ 1, LOCALE_STREAMINGMENU_ON, NULL }
};

#define STREAMINGMENU_VLC_VERSION_OPTION_COUNT 3
const CMenuOptionChooser::keyval STREAMINGMENU_VLC_VERSION_OPTIONS[STREAMINGMENU_VLC_VERSION_OPTION_COUNT] =
{
	{ 0, LOCALE_STREAMINGMENU_STREAMING_VLCVER08, NULL },
	{ 1, LOCALE_STREAMINGMENU_STREAMING_VLCVER10, NULL },
	{ 2, LOCALE_STREAMINGMENU_STREAMING_VLCVER20, NULL },
};

// init streamingssettings
void CNeutrinoApp::InitMoviePlayerSettings(CMenuWidget &moviePlayerSettings)
{
	dprintf(DEBUG_NORMAL, "CNeutrinoApp::InitMoviePlayerSettings\n");
	
	// intros
	moviePlayerSettings.addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	moviePlayerSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	
	// save settings
	moviePlayerSettings.addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_SAVESETTINGSNOW, true, NULL, this, "savesettings", CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));
	moviePlayerSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	
	// multi select
	moviePlayerSettings.addItem(new CMenuOptionChooser(LOCALE_STREAMINGMENU_FILEBROWSER_ALLOW_MULTISELECT, &g_settings.movieplayer_allow_multiselect, MESSAGEBOX_NO_YES_OPTIONS, MESSAGEBOX_NO_YES_OPTION_COUNT, true));

	// multiformat Dir
	moviePlayerSettings.addItem(new CMenuForwarder(LOCALE_MOVIEPLAYER_DEFDIR, true, g_settings.network_nfs_moviedir, this, "moviedir") ); 
}

// Init Color Settings
void CNeutrinoApp::InitColorSettings(CMenuWidget &colorSettings)
{
	dprintf(DEBUG_NORMAL, "CNeutrinoApp::InitColorSettings\n");
	
	int shortcutOSD = 1;
	
	// intros
	colorSettings.addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	colorSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	
	// save settings
	colorSettings.addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_SAVESETTINGSNOW, true, NULL, this, "savesettings", CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));
	colorSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );

	// Themes
	CThemes * colorSettings_Themes = new CThemes();
	
	colorSettings.addItem( new CMenuForwarder(LOCALE_COLORMENU_THEMESELECT, true, NULL, colorSettings_Themes, NULL, CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN) );

	// menu colors
	CMenuWidget * colorSettings_menuColors = new CMenuWidget(LOCALE_COLORMENUSETUP_HEAD, NEUTRINO_ICON_SETTINGS );
	InitColorSettingsMenuColors(*colorSettings_menuColors);

	//
	colorSettings.addItem( new CMenuForwarder(LOCALE_COLORMENU_MENUCOLORS, true, NULL, colorSettings_menuColors, NULL, CRCInput::RC_yellow, NEUTRINO_ICON_BUTTON_YELLOW) );

	// infobar
	CMenuWidget *colorSettings_statusbarColors = new CMenuWidget(LOCALE_COLORMENU_STATUSBAR, NEUTRINO_ICON_SETTINGS);
	InitColorSettingsStatusBarColors(*colorSettings_statusbarColors);

	colorSettings.addItem( new CMenuForwarder(LOCALE_COLORSTATUSBAR_HEAD, true, NULL, colorSettings_statusbarColors, NULL, CRCInput::RC_blue, NEUTRINO_ICON_BUTTON_BLUE) );

	// language
	CMenuWidget *colorSettings_language = new CMenuWidget(LOCALE_MAINSETTINGS_LANGUAGE, NEUTRINO_ICON_LANGUAGE);
	InitLanguageSettings(*colorSettings_language);

	// language
	colorSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	
	colorSettings.addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_LANGUAGE, true, NULL, colorSettings_language, NULL, CRCInput::convertDigitToKey(shortcutOSD++) ));

	// OSD
	colorSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_VIDEOMENU_OSD));
	
	// select font
	colorSettings.addItem( new CMenuForwarder(LOCALE_EPGPLUS_SELECT_FONT_NAME, true, NULL, this, "select_font", CRCInput::convertDigitToKey(shortcutOSD++) ));
	
	//font scaling
	colorSettings.addItem(new CMenuForwarder(LOCALE_FONTMENU_SCALING, true, NULL, this, "font_scaling", CRCInput::convertDigitToKey(shortcutOSD++) ));

	// osd-timing
	CMenuWidget *colorSettings_timing = new CMenuWidget(LOCALE_COLORMENU_TIMING, NEUTRINO_ICON_SETTINGS);
	InitColorSettingsTiming(*colorSettings_timing);

	colorSettings.addItem(new CMenuForwarder(LOCALE_TIMING_HEAD, true, NULL, colorSettings_timing, NULL, CRCInput::convertDigitToKey(shortcutOSD++) ));

	// sceensetup
	colorSettings.addItem(new CMenuForwarder(LOCALE_VIDEOMENU_SCREENSETUP, true, NULL, new CScreenSetup(), NULL, CRCInput::convertDigitToKey(shortcutOSD++) ));
	
	colorSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE));

	// alpha setup
	CAlphaSetup * chAlphaSetup = new CAlphaSetup(LOCALE_COLORMENU_GTX_ALPHA, &g_settings.gtx_alpha);
	colorSettings.addItem( new CMenuForwarder(LOCALE_COLORMENU_GTX_ALPHA, true, NULL, chAlphaSetup, NULL, CRCInput::convertDigitToKey(shortcutOSD++)));
}

void CNeutrinoApp::InitColorSettingsMenuColors(CMenuWidget &colorSettings_menuColors)
{
	// intros
	colorSettings_menuColors.addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));

	// head
	CColorChooser* chHeadcolor = new CColorChooser(LOCALE_COLORMENU_BACKGROUND, &g_settings.menu_Head_red, &g_settings.menu_Head_green, &g_settings.menu_Head_blue, &g_settings.menu_Head_alpha, colorSetupNotifier);
	CColorChooser* chHeadTextcolor = new CColorChooser(LOCALE_COLORMENU_TEXTCOLOR, &g_settings.menu_Head_Text_red, &g_settings.menu_Head_Text_green, &g_settings.menu_Head_Text_blue,NULL, colorSetupNotifier);

	// content
	CColorChooser* chContentcolor = new CColorChooser(LOCALE_COLORMENU_BACKGROUND, &g_settings.menu_Content_red, &g_settings.menu_Content_green, &g_settings.menu_Content_blue,&g_settings.menu_Content_alpha, colorSetupNotifier);
	CColorChooser* chContentTextcolor = new CColorChooser(LOCALE_COLORMENU_TEXTCOLOR, &g_settings.menu_Content_Text_red, &g_settings.menu_Content_Text_green, &g_settings.menu_Content_Text_blue,NULL, colorSetupNotifier);

	// inactive
	CColorChooser* chContentInactivecolor = new CColorChooser(LOCALE_COLORMENU_BACKGROUND, &g_settings.menu_Content_inactive_red, &g_settings.menu_Content_inactive_green, &g_settings.menu_Content_inactive_blue,&g_settings.menu_Content_inactive_alpha, colorSetupNotifier);
	CColorChooser* chContentInactiveTextcolor = new CColorChooser(LOCALE_COLORMENU_TEXTCOLOR, &g_settings.menu_Content_inactive_Text_red, &g_settings.menu_Content_inactive_Text_green, &g_settings.menu_Content_inactive_Text_blue, NULL, colorSetupNotifier);
	
	// selected
	CColorChooser* chContentSelectedcolor = new CColorChooser(LOCALE_COLORMENU_BACKGROUND, &g_settings.menu_Content_Selected_red, &g_settings.menu_Content_Selected_green, &g_settings.menu_Content_Selected_blue,&g_settings.menu_Content_Selected_alpha, colorSetupNotifier);
	CColorChooser* chContentSelectedTextcolor = new CColorChooser(LOCALE_COLORMENU_TEXTCOLOR, &g_settings.menu_Content_Selected_Text_red, &g_settings.menu_Content_Selected_Text_green, &g_settings.menu_Content_Selected_Text_blue,NULL, colorSetupNotifier);
	
	// foot
	CColorChooser* chFootcolor = new CColorChooser(LOCALE_COLORMENU_BACKGROUND, &g_settings.menu_Foot_red, &g_settings.menu_Foot_green, &g_settings.menu_Foot_blue, &g_settings.menu_Foot_alpha, colorSetupNotifier);
	CColorChooser * chFootTextcolor = new CColorChooser(LOCALE_COLORMENU_TEXTCOLOR, &g_settings.menu_Foot_Text_red, &g_settings.menu_Foot_Text_green, &g_settings.menu_Foot_Text_blue, NULL, colorSetupNotifier);

	// head
	colorSettings_menuColors.addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_COLORMENUSETUP_MENUHEAD));
	colorSettings_menuColors.addItem( new CMenuForwarder(LOCALE_COLORMENU_BACKGROUND, true, NULL, chHeadcolor ));
	colorSettings_menuColors.addItem( new CMenuForwarder(LOCALE_COLORMENU_TEXTCOLOR, true, NULL, chHeadTextcolor ));

	// menu content
	colorSettings_menuColors.addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_COLORMENUSETUP_MENUCONTENT));
	colorSettings_menuColors.addItem( new CMenuForwarder(LOCALE_COLORMENU_BACKGROUND, true, NULL, chContentcolor ));
	colorSettings_menuColors.addItem( new CMenuForwarder(LOCALE_COLORMENU_TEXTCOLOR, true, NULL, chContentTextcolor ));

	// inactiv
	colorSettings_menuColors.addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_COLORMENUSETUP_MENUCONTENT_INACTIVE));
	colorSettings_menuColors.addItem( new CMenuForwarder(LOCALE_COLORMENU_BACKGROUND, true, NULL, chContentInactivecolor ));
	colorSettings_menuColors.addItem( new CMenuForwarder(LOCALE_COLORMENU_TEXTCOLOR, true, NULL, chContentInactiveTextcolor));

	// selected
	colorSettings_menuColors.addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_COLORMENUSETUP_MENUCONTENT_SELECTED));
	colorSettings_menuColors.addItem( new CMenuForwarder(LOCALE_COLORMENU_BACKGROUND, true, NULL, chContentSelectedcolor ));
	colorSettings_menuColors.addItem( new CMenuForwarder(LOCALE_COLORMENU_TEXTCOLOR, true, NULL, chContentSelectedTextcolor ));
	
	// foot
	colorSettings_menuColors.addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_COLORMENU_HELPBAR));
	colorSettings_menuColors.addItem( new CMenuForwarder(LOCALE_COLORMENU_BACKGROUND, true, NULL, chFootcolor ));
	colorSettings_menuColors.addItem( new CMenuForwarder(LOCALE_COLORMENU_TEXTCOLOR, true, NULL, chFootTextcolor ));
}

void CNeutrinoApp::InitColorSettingsStatusBarColors(CMenuWidget &colorSettings_statusbarColors)
{
	// intros
	colorSettings_statusbarColors.addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));

	// bg
	CColorChooser * chInfobarcolor = new CColorChooser(LOCALE_COLORMENU_BACKGROUND, &g_settings.infobar_red, &g_settings.infobar_green, &g_settings.infobar_blue,&g_settings.infobar_alpha, colorSetupNotifier);
	
	// text
	CColorChooser * chInfobarTextcolor = new CColorChooser(LOCALE_COLORMENU_TEXTCOLOR, &g_settings.infobar_Text_red, &g_settings.infobar_Text_green, &g_settings.infobar_Text_blue,NULL, colorSetupNotifier);
	
	// clolored events
	CColorChooser * chColored_Events = new CColorChooser(LOCALE_COLORMENU_TEXTCOLOR, &g_settings.infobar_colored_events_red, &g_settings.infobar_colored_events_green, &g_settings.infobar_colored_events_blue, NULL, colorSetupNotifier);

	colorSettings_statusbarColors.addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_COLORSTATUSBAR_TEXT));

	colorSettings_statusbarColors.addItem( new CMenuForwarder(LOCALE_COLORMENU_BACKGROUND, true, NULL, chInfobarcolor ));

	colorSettings_statusbarColors.addItem( new CMenuForwarder(LOCALE_COLORMENU_TEXTCOLOR, true, NULL, chInfobarTextcolor ));
	
	// clored events
	colorSettings_statusbarColors.addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_MISCSETTINGS_INFOBAR_COLORED_EVENTS));
	colorSettings_statusbarColors.addItem( new CMenuForwarder(LOCALE_COLORMENU_TEXTCOLOR, true, NULL, chColored_Events ));
}

void CNeutrinoApp::InitColorSettingsTiming(CMenuWidget &colorSettings_timing)
{
	// intros
	colorSettings_timing.addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	colorSettings_timing.addItem(new CMenuSeparator(CMenuSeparator::LINE));

	for (int i = 0; i < TIMING_SETTING_COUNT; i++)
	{
		CStringInput * colorSettings_timing_item = new CStringInput(timing_setting_name[i], g_settings.timing_string[i], 3, LOCALE_TIMING_HINT_1, LOCALE_TIMING_HINT_2, "0123456789 ", &timingsettingsnotifier);
		colorSettings_timing.addItem(new CMenuForwarder(timing_setting_name[i], true, g_settings.timing_string[i], colorSettings_timing_item));
	}

	colorSettings_timing.addItem(new CMenuForwarder(LOCALE_OPTIONS_DEFAULT, true, NULL, this, "osd.def"));
}

#if defined (ENABLE_LCD)
/* for lcd settings menu*/
#define LCDMENU_STATUSLINE_OPTION_COUNT 4
const CMenuOptionChooser::keyval LCDMENU_STATUSLINE_OPTIONS[LCDMENU_STATUSLINE_OPTION_COUNT] =
{
	{ 0, LOCALE_LCDMENU_STATUSLINE_PLAYTIME, NULL   },
	{ 1, LOCALE_LCDMENU_STATUSLINE_VOLUME, NULL     },
	{ 2, LOCALE_LCDMENU_STATUSLINE_BOTH, NULL       },
	{ 3, LOCALE_LCDMENU_STATUSLINE_BOTH_AUDIO, NULL }
};

/* for lcd EPG menu*/
#define LCDMENU_EPG_OPTION_COUNT 6
const CMenuOptionChooser::keyval LCDMENU_EPG_OPTIONS[LCDMENU_EPG_OPTION_COUNT] =
{
	{ 1, LOCALE_LCDMENU_EPG_NAME, NULL		},
	{ 2, LOCALE_LCDMENU_EPG_TITLE, NULL		},
	{ 3, LOCALE_LCDMENU_EPG_NAME_TITLE, NULL	},
	{ 7, LOCALE_LCDMENU_EPG_NAME_SEPLINE_TITLE, NULL },
	{ 11, LOCALE_LCDMENU_EPG_NAMESHORT_TITLE, NULL },
	{ 15, LOCALE_LCDMENU_EPG_NAMESHORT_SEPLINE_TITLE, NULL }
};

#define LCDMENU_EPGALIGN_OPTION_COUNT 2
const CMenuOptionChooser::keyval LCDMENU_EPGALIGN_OPTIONS[LCDMENU_EPGALIGN_OPTION_COUNT] =
{
	{ 0, LOCALE_LCDMENU_EPGALIGN_LEFT, NULL   },
	{ 1, LOCALE_LCDMENU_EPGALIGN_CENTER, NULL }
};
#endif

/* led color */
#if defined (PLATFORM_GIGABLUE) 
#if !defined (ENABLE_LCD)
#define LCDMENU_LEDCOLOR_OPTION_COUNT 4
const CMenuOptionChooser::keyval LCDMENU_LEDCOLOR_OPTIONS[LCDMENU_LEDCOLOR_OPTION_COUNT] =
{
	{ CVFD::LED_OFF, LOCALE_OPTIONS_OFF, NULL },
	{ CVFD::LED_BLUE, LOCALE_LCDMENU_LEDCOLOR_BLUE, NULL },
	{ CVFD::LED_RED, LOCALE_LCDMENU_LEDCOLOR_RED, NULL },
	{ CVFD::LED_PURPLE, LOCALE_LCDMENU_LEDCOLOR_PURPLE, NULL },
};
#endif
#endif

/* Init LCD Settings */
void CNeutrinoApp::InitLcdSettings(CMenuWidget &lcdSettings)
{
	dprintf(DEBUG_NORMAL, "CNeutrinoApp::InitLcdSettings\n");
	
	int shortcutVFD = 1;
	
	// intros
	lcdSettings.addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	lcdSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	
	// save settings
	lcdSettings.addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_SAVESETTINGSNOW, true, NULL, this, "savesettings", CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));
	lcdSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE));
	
	CLcdNotifier * lcdnotifier = new CLcdNotifier();
	
	CVfdControler * lcdsliders = new CVfdControler(LOCALE_LCDMENU_HEAD, NULL);
	
	// LCD
#if defined (ENABLE_LCD)
	//option invert
	CMenuOptionChooser* oj_inverse = new CMenuOptionChooser(LOCALE_LCDMENU_INVERSE, &g_settings.lcd_setting[SNeutrinoSettings::LCD_INVERSE], OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true, lcdnotifier, CRCInput::convertDigitToKey(shortcutVFD++) );
	lcdSettings.addItem(oj_inverse);

	//status display
	CMenuOptionChooser* oj_status = new CMenuOptionChooser(LOCALE_LCDMENU_STATUSLINE, &g_settings.lcd_setting[SNeutrinoSettings::LCD_SHOW_VOLUME], LCDMENU_STATUSLINE_OPTIONS, LCDMENU_STATUSLINE_OPTION_COUNT, true);
	lcdSettings.addItem(oj_status);
	
	//lcd_epg
	CMenuOptionChooser* oj_epg = new CMenuOptionChooser(LOCALE_LCDMENU_EPG, &g_settings.lcd_setting[SNeutrinoSettings::LCD_EPGMODE], LCDMENU_EPG_OPTIONS, LCDMENU_EPG_OPTION_COUNT, true);
	lcdSettings.addItem(oj_epg);

	//align
	CMenuOptionChooser* oj_align = new CMenuOptionChooser(LOCALE_LCDMENU_EPGALIGN, &g_settings.lcd_setting[SNeutrinoSettings::LCD_EPGALIGN], LCDMENU_EPGALIGN_OPTIONS, LCDMENU_EPGALIGN_OPTION_COUNT, true);
	lcdSettings.addItem(oj_align);

	//dump to png
	CMenuOptionChooser* oj_dumppng = new CMenuOptionChooser(LOCALE_LCDMENU_DUMP_PNG, &g_settings.lcd_setting[SNeutrinoSettings::LCD_DUMP_PNG], OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true);
	lcdSettings.addItem(oj_dumppng);
	
	// lcd controller
	lcdSettings.addItem(new CMenuForwarder(LOCALE_LCDMENU_LCDCONTROLER, true, NULL, lcdsliders, NULL, CRCInput::convertDigitToKey(shortcutVFD++) ));
#else	
#if defined (PLATFORM_GIGABLUE)	
	// led color
	lcdSettings.addItem(new CMenuOptionChooser(LOCALE_LCDMENU_LEDCOLOR, &g_settings.lcd_ledcolor, LCDMENU_LEDCOLOR_OPTIONS, LCDMENU_LEDCOLOR_OPTION_COUNT, true, lcdnotifier, CRCInput::convertDigitToKey(shortcutVFD++) ));	
#elif !defined (PLATFORM_CUBEREVO_250HD) && !defined (PLATFORM_SPARK)
	// vfd power
	CMenuOptionChooser * oj2 = new CMenuOptionChooser(LOCALE_LCDMENU_POWER, &g_settings.lcd_setting[SNeutrinoSettings::LCD_POWER], OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true, lcdnotifier, CRCInput::convertDigitToKey(shortcutVFD++) );
	lcdSettings.addItem(oj2);
	
	// dimm-time
	CStringInput * dim_time = new CStringInput(LOCALE_LCDMENU_DIM_TIME, g_settings.lcd_setting_dim_time, 3, NONEXISTANT_LOCALE, NONEXISTANT_LOCALE,"0123456789 ");
	lcdSettings.addItem(new CMenuForwarder(LOCALE_LCDMENU_DIM_TIME,true, g_settings.lcd_setting_dim_time, dim_time, NULL, CRCInput::convertDigitToKey(shortcutVFD++)));

	// dimm brightness
	//CStringInput * dim_brightness = new CStringInput(LOCALE_LCDMENU_DIM_BRIGHTNESS, g_settings.lcd_setting_dim_brightness, 3,NONEXISTANT_LOCALE, NONEXISTANT_LOCALE,"0123456789 ");
	//lcdSettings.addItem(new CMenuForwarder(LOCALE_LCDMENU_DIM_BRIGHTNESS,true, g_settings.lcd_setting_dim_brightness, dim_brightness, NULL, CRCInput::convertDigitToKey(shortcutVFD++) ));

	// vfd controller
	lcdSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE));
	lcdSettings.addItem(new CMenuForwarder(LOCALE_LCDMENU_LCDCONTROLER, true, NULL, lcdsliders, NULL, CRCInput::convertDigitToKey(shortcutVFD++) ));	
#endif	
#endif	
}

// Init Keys Settings
enum keynames {
	// zap
	KEY_TV_RADIO_MODE,
	KEY_PAGE_UP,
	KEY_PAGE_DOWN,
	KEY_LIST_START,
	KEY_LIST_END,
	KEY_CANCEL_ACTION,
	KEY_SORT,
	KEY_RELOAD,
	VKEY_SEARCH,
	KEY_ADD_RECORD,
	KEY_ADD_REMIND,
	KEY_BOUQUET_UP,
	KEY_BOUQUET_DOWN,
	KEY_CHANNEL_UP,
	KEY_CHANNEL_DOWN,
	KEY_SUBCHANNEL_UP,
	KEY_SUBCHANNEL_DOWN,
	KEY_ZAP_HISTORY,
	KEY_LASTCHANNEL,
	KEY_SAME_TP,
	
	// mp
        MPKEY_REWIND,
        MPKEY_FORWARD,
        MPKEY_PAUSE,
        MPKEY_STOP,
        MPKEY_PLAY,
        MPKEY_AUDIO,
        MPKEY_TIME,
        MPKEY_BOOKMARK,
	KEY_TIMESHIFT,
	
	// media
	KEY_EXTRAS_RECORDSBROWSER,
	KEY_EXTRAS_AUDIOPLAYER,
	KEY_EXTRAS_PICTUREVIEWER,
	KEY_EXTRAS_TIMERLIST,
	KEY_EXTRAS_INETRADIO,
	KEY_EXTRAS_MOVIEBROWSER,
	KEY_EXTRAS_FILEBROWSER,
	KEY_EXTRAS_WEBTV,
	KEY_EXTRAS_SCREENSHOT,
	
	// mb
	KEY_EXTRAS_MB_COPY_JUMP,
	KEY_EXTRAS_MB_CUT_JUMP,
	KEY_EXTRAS_MB_TRUNCATE
};

#define KEYBINDS_COUNT 41
const neutrino_locale_t keydescription_head[KEYBINDS_COUNT] =
{
	// zap
	LOCALE_KEYBINDINGMENU_TVRADIOMODE,
	LOCALE_KEYBINDINGMENU_PAGEUP,
	LOCALE_KEYBINDINGMENU_PAGEDOWN,
	LOCALE_EXTRA_KEY_LIST_START,
	LOCALE_EXTRA_KEY_LIST_END,
	LOCALE_KEYBINDINGMENU_CANCEL,
	LOCALE_KEYBINDINGMENU_SORT,
	LOCALE_KEYBINDINGMENU_RELOAD,
	LOCALE_KEYBINDINGMENU_SEARCH,
	LOCALE_KEYBINDINGMENU_ADDRECORD,
	LOCALE_KEYBINDINGMENU_ADDREMIND,
	LOCALE_KEYBINDINGMENU_BOUQUETUP,
	LOCALE_KEYBINDINGMENU_BOUQUETDOWN,
	LOCALE_KEYBINDINGMENU_CHANNELUP,
	LOCALE_KEYBINDINGMENU_CHANNELDOWN,
	LOCALE_KEYBINDINGMENU_SUBCHANNELUP,
	LOCALE_KEYBINDINGMENU_SUBCHANNELDOWN,
	LOCALE_KEYBINDINGMENU_ZAPHISTORY,
	LOCALE_KEYBINDINGMENU_LASTCHANNEL,
	LOCALE_KEYBINDINGMENU_PIP,
	
	// mp
        LOCALE_MPKEY_REWIND,
        LOCALE_MPKEY_FORWARD,
        LOCALE_MPKEY_PAUSE,
        LOCALE_MPKEY_STOP,
        LOCALE_MPKEY_PLAY,
        LOCALE_MPKEY_AUDIO,
        LOCALE_MPKEY_TIME,
        LOCALE_MPKEY_BOOKMARK,
	LOCALE_EXTRA_KEY_TIMESHIFT,

	// media
	LOCALE_KEYBINDINGMENU_RECORDSBROWSER,
	LOCALE_KEYBINDINGMENU_AUDIOPLAYER,
	LOCALE_KEYBINDINGMENU_PICTUREVIEWER,
	LOCALE_KEYBINDINGMENU_TIMERLIST,
	LOCALE_KEYBINDINGMENU_INETRADIO,
	LOCALE_KEYBINDINGMENU_MOVIEBROWSER,
	LOCALE_KEYBINDINGMENU_FILEBROWSER,
	LOCALE_KEYBINDINGMENU_WEBTV,
	LOCALE_KEYBINDINGMENU_SCREENSHOT,
	
	// mb
	LOCALE_KEYBINDINGMENU_MB_COPY_JUMP,
	LOCALE_KEYBINDINGMENU_MB_CUT_JUMP,
	LOCALE_KEYBINDINGMENU_MB_TRUNCATE
};

const neutrino_locale_t keydescription[KEYBINDS_COUNT] =
{
	// zap
	LOCALE_KEYBINDINGMENU_TVRADIOMODE,
	LOCALE_KEYBINDINGMENU_PAGEUP,
	LOCALE_KEYBINDINGMENU_PAGEDOWN,
	LOCALE_EXTRA_KEY_LIST_START,
	LOCALE_EXTRA_KEY_LIST_END,
	LOCALE_KEYBINDINGMENU_CANCEL,
	LOCALE_KEYBINDINGMENU_SORT,
	LOCALE_KEYBINDINGMENU_RELOAD,
	LOCALE_KEYBINDINGMENU_SEARCH,
	LOCALE_KEYBINDINGMENU_ADDRECORD,
	LOCALE_KEYBINDINGMENU_ADDREMIND,
	LOCALE_KEYBINDINGMENU_BOUQUETUP,
	LOCALE_KEYBINDINGMENU_BOUQUETDOWN,
	LOCALE_KEYBINDINGMENU_CHANNELUP,
	LOCALE_KEYBINDINGMENU_CHANNELDOWN,
	LOCALE_KEYBINDINGMENU_SUBCHANNELUP,
	LOCALE_KEYBINDINGMENU_SUBCHANNELDOWN,
	LOCALE_KEYBINDINGMENU_ZAPHISTORY,
	LOCALE_KEYBINDINGMENU_LASTCHANNEL,
	LOCALE_KEYBINDINGMENU_PIP,
	
	// mp
        LOCALE_MPKEY_REWIND,
        LOCALE_MPKEY_FORWARD,
        LOCALE_MPKEY_PAUSE,
        LOCALE_MPKEY_STOP,
        LOCALE_MPKEY_PLAY,
        LOCALE_MPKEY_AUDIO,
        LOCALE_MPKEY_TIME,
        LOCALE_MPKEY_BOOKMARK,
	LOCALE_EXTRA_KEY_TIMESHIFT,
	
	// media
	LOCALE_KEYBINDINGMENU_RECORDSBROWSER,
	LOCALE_KEYBINDINGMENU_AUDIOPLAYER,
	LOCALE_KEYBINDINGMENU_PICTUREVIEWER,
	LOCALE_KEYBINDINGMENU_TIMERLIST,
	LOCALE_KEYBINDINGMENU_INETRADIO,
	LOCALE_KEYBINDINGMENU_MOVIEBROWSER,
	LOCALE_KEYBINDINGMENU_FILEBROWSER,
	LOCALE_KEYBINDINGMENU_WEBTV,
	LOCALE_KEYBINDINGMENU_SCREENSHOT,
	
	// mb
	LOCALE_KEYBINDINGMENU_MB_COPY_JUMP,
	LOCALE_KEYBINDINGMENU_MB_CUT_JUMP,
	LOCALE_KEYBINDINGMENU_MB_TRUNCATE
};

void CNeutrinoApp::InitKeySettings(CMenuWidget &keySettings, CMenuWidget &bindSettings)
{
	dprintf(DEBUG_NORMAL, "CNeutrinoApp::InitKeySettings\n");
	
	int shortcutkeysettings = 1;
	
	// intros
	keySettings.addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	keySettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	
	// save settings
	keySettings.addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_SAVESETTINGSNOW, true, NULL, this, "savesettings", CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED/*, CRCInput::convertDigitToKey(shortcutkeysettings++)*/ ));

	keySetupNotifier = new CKeySetupNotifier;
	
	// repeat generic blocker
	CStringInput * keySettings_repeat_genericblocker = new CStringInput(LOCALE_KEYBINDINGMENU_REPEATBLOCKGENERIC, g_settings.repeat_genericblocker, 3, LOCALE_REPEATBLOCKER_HINT_1, LOCALE_REPEATBLOCKER_HINT_2, "0123456789 ", keySetupNotifier);
	
	// repeat blocker
	CStringInput * keySettings_repeatBlocker = new CStringInput(LOCALE_KEYBINDINGMENU_REPEATBLOCK, g_settings.repeat_blocker, 3, LOCALE_REPEATBLOCKER_HINT_1, LOCALE_REPEATBLOCKER_HINT_2, "0123456789 ", keySetupNotifier);
	keySetupNotifier->changeNotify(NONEXISTANT_LOCALE, NULL);

	keySettings.addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_KEYBINDINGMENU_RC));
	
	// repeat blocker
	keySettings.addItem(new CMenuForwarder(LOCALE_KEYBINDINGMENU_REPEATBLOCK, true, g_settings.repeat_blocker, keySettings_repeatBlocker, NULL, CRCInput::convertDigitToKey(shortcutkeysettings++)));
	
	// repeat generic blocker
 	keySettings.addItem(new CMenuForwarder(LOCALE_KEYBINDINGMENU_REPEATBLOCKGENERIC, true, g_settings.repeat_genericblocker, keySettings_repeat_genericblocker, NULL, CRCInput::convertDigitToKey(shortcutkeysettings++)));

	// keys binding
	int * keyvalue_p[KEYBINDS_COUNT] =
	{
		// zap
		&g_settings.key_tvradio_mode,
		&g_settings.key_channelList_pageup,
		&g_settings.key_channelList_pagedown,
		&g_settings.key_list_start,
		&g_settings.key_list_end,
		&g_settings.key_channelList_cancel,
		&g_settings.key_channelList_sort,
		&g_settings.key_channelList_reload,
		&g_settings.key_channelList_search,
		&g_settings.key_channelList_addrecord,
		&g_settings.key_channelList_addremind,
		&g_settings.key_bouquet_up,
		&g_settings.key_bouquet_down,
		&g_settings.key_quickzap_up,
		&g_settings.key_quickzap_down,
		&g_settings.key_subchannel_up,
		&g_settings.key_subchannel_down,
		&g_settings.key_zaphistory,
		&g_settings.key_lastchannel,
		&g_settings.key_pip,

		// mp
		&g_settings.mpkey_rewind,
		&g_settings.mpkey_forward,
		&g_settings.mpkey_pause,
		&g_settings.mpkey_stop,
		&g_settings.mpkey_play,
		&g_settings.mpkey_audio,
		&g_settings.mpkey_time,
		&g_settings.mpkey_bookmark,
		&g_settings.key_timeshift,
		
		// media
		&g_settings.key_recordsbrowser,
		&g_settings.key_audioplayer,
		&g_settings.key_pictureviewer,
		&g_settings.key_timerlist,
		&g_settings.key_inetradio,
		&g_settings.key_moviebrowser,
		&g_settings.key_filebrowser,
		&g_settings.key_webtv,
		
		// misc
		&g_settings.key_screenshot,
		
		// mb
		&g_settings.mb_copy_jump,
		&g_settings.mb_cut_jump,
		&g_settings.mb_truncate
	};

	CKeyChooser * keychooser[KEYBINDS_COUNT];

	for (int i = 0; i < KEYBINDS_COUNT; i++)
		keychooser[i] = new CKeyChooser(keyvalue_p[i], keydescription_head[i], NEUTRINO_ICON_SETTINGS);

	keySettings.addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_KEYBINDINGMENU_HEAD));
	
	// keybinding menu
	// intros
	bindSettings.addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));

	bindSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_KEYBINDINGMENU_MODECHANGE));
	
	// tv/radio mode
	bindSettings.addItem(new CMenuForwarder(keydescription[KEY_TV_RADIO_MODE], true, NULL, keychooser[KEY_TV_RADIO_MODE]));

	// channellist
	bindSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_KEYBINDINGMENU_CHANNELLIST));

	for (int i = KEY_PAGE_UP; i <= KEY_BOUQUET_DOWN; i++)
		bindSettings.addItem(new CMenuForwarder(keydescription[i], true, NULL, keychooser[i]));

	// quick zap
	bindSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_KEYBINDINGMENU_QUICKZAP));

	for (int i = KEY_CHANNEL_UP; i <= KEY_SAME_TP; i++)
		bindSettings.addItem(new CMenuForwarder(keydescription[i], true, NULL, keychooser[i]));

	// mp keys
	bindSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_MAINMENU_MOVIEPLAYER));
	for (int i = MPKEY_REWIND; i <= KEY_TIMESHIFT; i++)
		bindSettings.addItem(new CMenuForwarder(keydescription[i], true, NULL, keychooser[i]));
	
	// media
	bindSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_MAINMENU_MEDIAPLAYER));
	for (int i = KEY_EXTRAS_RECORDSBROWSER; i <= KEY_EXTRAS_WEBTV; i++)
		bindSettings.addItem(new CMenuForwarder(keydescription[i], true, NULL, keychooser[i]));
	
	// mb
	bindSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_MOVIEBROWSER_HEAD));
	for (int i = KEY_EXTRAS_MB_COPY_JUMP; i <= KEY_EXTRAS_MB_TRUNCATE; i++)
		bindSettings.addItem(new CMenuForwarder(keydescription[i], true, NULL, keychooser[i]));

	// misc
	bindSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_MAINSETTINGS_MISC));
	
	// screenshot key
	bindSettings.addItem(new CMenuForwarder(keydescription[KEY_EXTRAS_SCREENSHOT], true, NULL, keychooser[KEY_EXTRAS_SCREENSHOT]));
	
	// save keymap
	bindSettings.addItem(new CMenuForwarder(LOCALE_KEYBINDINGMENU_SAVEKEYMAP, true, NULL, this, "savekeymap" ) );

	keySettings.addItem(new CMenuForwarder(LOCALE_KEYBINDINGMENU_HEAD, true, NULL, &bindSettings, NULL, CRCInput::convertDigitToKey(shortcutkeysettings++)));

        // USERMENU
        keySettings.addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_USERMENU_HEAD));
	
        keySettings.addItem(new CMenuForwarder(LOCALE_USERMENU_BUTTON_RED, true, NULL, new CUserMenuMenu(LOCALE_USERMENU_BUTTON_RED, 0), NULL, CRCInput::convertDigitToKey(shortcutkeysettings++) ));
        keySettings.addItem(new CMenuForwarder(LOCALE_USERMENU_BUTTON_GREEN, true, NULL, new CUserMenuMenu(LOCALE_USERMENU_BUTTON_GREEN, 1), NULL, CRCInput::convertDigitToKey(shortcutkeysettings++) ));
        keySettings.addItem(new CMenuForwarder(LOCALE_USERMENU_BUTTON_YELLOW, true, NULL, new CUserMenuMenu(LOCALE_USERMENU_BUTTON_YELLOW, 2), NULL, CRCInput::convertDigitToKey(shortcutkeysettings++) ));
        keySettings.addItem(new CMenuForwarder(LOCALE_USERMENU_BUTTON_BLUE, true, NULL, new CUserMenuMenu(LOCALE_USERMENU_BUTTON_BLUE, 3), NULL, CRCInput::convertDigitToKey(shortcutkeysettings++) ));
#if defined (ENABLE_FUNCTIONKEYS)	
	keySettings.addItem(new CMenuForwarder(LOCALE_USERMENU_BUTTON_F1, true, NULL, new CUserMenuMenu(LOCALE_USERMENU_BUTTON_F1, 4) ));
        keySettings.addItem(new CMenuForwarder(LOCALE_USERMENU_BUTTON_F2, true, NULL, new CUserMenuMenu(LOCALE_USERMENU_BUTTON_F2, 5) ));
        keySettings.addItem(new CMenuForwarder(LOCALE_USERMENU_BUTTON_F3, true, NULL, new CUserMenuMenu(LOCALE_USERMENU_BUTTON_F3, 6) ));
        keySettings.addItem(new CMenuForwarder(LOCALE_USERMENU_BUTTON_F4, true, NULL, new CUserMenuMenu(LOCALE_USERMENU_BUTTON_BLUE, 7) ));	
#endif
}

// User menu
// leave this functions, somebody might want to use it in the future again
void CNeutrinoApp::SelectNVOD()
{
        if (!(g_RemoteControl->subChannels.empty()))
        {
                // NVOD/SubService- Kanal!
                CMenuWidget NVODSelector(g_RemoteControl->are_subchannels ? LOCALE_NVODSELECTOR_SUBSERVICE : LOCALE_NVODSELECTOR_HEAD, NEUTRINO_ICON_VIDEO);
		
                if(getNVODMenu(&NVODSelector))
                        NVODSelector.exec(NULL, "");
        }
}

bool CNeutrinoApp::getNVODMenu(CMenuWidget * menu)
{
        if(menu == NULL)
                return false;
	
        if (g_RemoteControl->subChannels.empty())
                return false;

        int count = 0;
        char nvod_id[5];

        for( CSubServiceListSorted::iterator e = g_RemoteControl->subChannels.begin(); e != g_RemoteControl->subChannels.end(); ++e)
        {
                sprintf(nvod_id, "%d", count);

                if( !g_RemoteControl->are_subchannels ) 
		{
                        char nvod_time_a[50], nvod_time_e[50], nvod_time_x[50];
                        char nvod_s[100];
                        struct  tm *tmZeit;

                        tmZeit= localtime(&e->startzeit);
                        sprintf(nvod_time_a, "%02d:%02d", tmZeit->tm_hour, tmZeit->tm_min);

                        time_t endtime = e->startzeit+ e->dauer;
                        tmZeit= localtime(&endtime);
                        sprintf(nvod_time_e, "%02d:%02d", tmZeit->tm_hour, tmZeit->tm_min);

                        time_t jetzt=time(NULL);
                        if(e->startzeit > jetzt) 
			{
                                int mins=(e->startzeit- jetzt)/ 60;
                                sprintf(nvod_time_x, g_Locale->getText(LOCALE_NVOD_STARTING), mins);
                        }
                        else if( (e->startzeit<= jetzt) && (jetzt < endtime) ) 
			{
                                int proz=(jetzt- e->startzeit)*100/ e->dauer;
                                sprintf(nvod_time_x, g_Locale->getText(LOCALE_NVOD_PERCENTAGE), proz);
                        }
                        else
                                nvod_time_x[0]= 0;

                        sprintf(nvod_s, "%s - %s %s", nvod_time_a, nvod_time_e, nvod_time_x);
                        menu->addItem(new CMenuForwarder(nvod_s, true, NULL, NVODChanger, nvod_id), (count == g_RemoteControl->selected_subchannel));
                } 
		else 
		{
			if (count == 0)
				menu->addItem(new CMenuForwarder( (Latin1_to_UTF8(e->subservice_name)).c_str(), true, NULL, NVODChanger, nvod_id, CRCInput::RC_blue, NEUTRINO_ICON_BUTTON_BLUE));
			else
				menu->addItem(new CMenuForwarder( (Latin1_to_UTF8(e->subservice_name)).c_str(), true, NULL, NVODChanger, nvod_id, CRCInput::convertDigitToKey(count)), (count == g_RemoteControl->selected_subchannel));
                }

                count++;
        }

        if( g_RemoteControl->are_subchannels ) 
	{
                menu->addItem(new CMenuSeparator(CMenuSeparator::LINE));
                CMenuOptionChooser* oj = new CMenuOptionChooser(LOCALE_NVODSELECTOR_DIRECTORMODE, &g_RemoteControl->director_mode, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true, NULL, CRCInput::RC_yellow, NEUTRINO_ICON_BUTTON_YELLOW);
                menu->addItem(oj);
        }

        return true;
}

#define MAINMENU_RECORDING_OPTION_COUNT 2
const CMenuOptionChooser::keyval MAINMENU_RECORDING_OPTIONS[MAINMENU_RECORDING_OPTION_COUNT] =
{
	{ 0, LOCALE_MAINMENU_RECORDING_START, NULL },
	{ 1, LOCALE_MAINMENU_RECORDING_STOP, NULL  }
};

// USERMENU
// This is just a quick helper for the usermenu only. I already made it a class for future use.
#if defined (ENABLE_FUNCTIONKEYS) //FIXME:???
#define BUTTONMAX 8
#else
#define BUTTONMAX 4
#endif

const neutrino_msg_t key_helper_msg_def[BUTTONMAX] = {
	CRCInput::RC_red,
	CRCInput::RC_green,
	CRCInput::RC_yellow,
	CRCInput::RC_blue,
#if defined (ENABLE_FUNCTIONKEYS)
	CRCInput::RC_f1,
	CRCInput::RC_f2,
	CRCInput::RC_f3,
	CRCInput::RC_f4
#endif
};

const char * key_helper_icon_def[BUTTONMAX]={
	NEUTRINO_ICON_BUTTON_RED, 
	NEUTRINO_ICON_BUTTON_GREEN, 
	NEUTRINO_ICON_BUTTON_YELLOW, 
	NEUTRINO_ICON_BUTTON_BLUE,
#if defined (ENABLE_FUNCTIONKEYS)	
	NEUTRINO_ICON_BUTTON_F1, 
	NEUTRINO_ICON_BUTTON_F2, 
	NEUTRINO_ICON_BUTTON_F3, 
	NEUTRINO_ICON_BUTTON_F4, 
#endif
};

class CKeyHelper
{
        private:
                int number_key;
                bool color_key_used[BUTTONMAX];
        public:
                CKeyHelper(){reset();};
                void reset(void)
                {
                        number_key = 1;
                        for(int i= 0; i < BUTTONMAX; i++ )
                                color_key_used[i] = false;
                };

                /* Returns the next available button, to be used in menu as 'direct' keys. Appropriate
                 * definitions are returnd in msp and icon
                 * A color button could be requested as prefered button (other buttons are not supported yet).
                 * If the appropriate button is already in used, the next number_key button is returned instead
                 * (first 1-9 and than 0). */
                bool get(neutrino_msg_t* msg, const char** icon, neutrino_msg_t prefered_key = CRCInput::RC_nokey)
                {
                        bool result = false;
                        int button = -1;
                        if(prefered_key == CRCInput::RC_red)
                                button = 0;
                        if(prefered_key == CRCInput::RC_green)
                                button = 1;
                        if(prefered_key == CRCInput::RC_yellow)
                                button = 2;
                        if(prefered_key == CRCInput::RC_blue)
                                button = 3;
#if defined (ENABLE_FUNCTIONKEYS) //FIXME:???
			if(prefered_key == CRCInput::RC_f1)
                                button = 4;
			if(prefered_key == CRCInput::RC_f2)
                                button = 5;
			if(prefered_key == CRCInput::RC_f3)
                                button = 6;
			if(prefered_key == CRCInput::RC_f4)
                                button = 7;
#endif

                        *msg = CRCInput::RC_nokey;
                        *icon = "";
                        if(button >= 0 && button < BUTTONMAX)
                        {
				// try to get color button
                                if( color_key_used[button] == false)
                                {
                                        color_key_used[button] = true;
                                        *msg = key_helper_msg_def[button];
                                        *icon = key_helper_icon_def[button];
                                        result = true;
                                }
                        }

                        if( result == false && number_key < 10) // no key defined yet, at least try to get a numbered key
                        {
                                // there is still a available number_key
                                *msg = CRCInput::convertDigitToKey(number_key);
                                *icon = "";
                                if(number_key == 9)
                                        number_key = 0;
                                else if(number_key == 0)
                                        number_key = 10;
                                else
                                        number_key++;
                                result = true;
                        }
                        return (result);
                };
};

// USERMENU
bool CNeutrinoApp::showUserMenu(int button)
{
	dprintf(DEBUG_NORMAL, "CNeutrinoApp::showUserMenu\n");
	
        if(button < 0 || button >= SNeutrinoSettings::BUTTON_MAX)
                return false;

        CMenuItem * menu_item = NULL;
        CKeyHelper keyhelper;
        neutrino_msg_t key = CRCInput::RC_nokey;
        const char * icon = NULL;
	//int dummy;

        int menu_items = 0;
        int menu_prev = -1;
	static int selected[SNeutrinoSettings::BUTTON_MAX] = {
		-1, 
		-1, 
		-1, 
		-1,
#if defined (ENABLE_FUNCTIONKEYS) //FIXME:???
		-1,
		-1,
		-1,
		-1,
#endif		
	};

        /* define classes */
        CPauseSectionsdNotifier * tmpPauseSectionsdNotifier     = NULL;
        CAudioSelectMenuHandler * tmpAudioSelectMenuHandler     = NULL;
        CMenuWidget * tmpNVODSelector                           = NULL;
        CStreamInfo2Handler * tmpStreamInfo2Handler          	= NULL;
        CEventListHandler * tmpEventListHandler                 = NULL;
        CEPGplusHandler * tmpEPGplusHandler                     = NULL;
        CEPGDataHandler * tmpEPGDataHandler                     = NULL;
	COPKGManager * tmpOPKGManager				= NULL;

        std::string txt = g_settings.usermenu_text[button];

        if (button == SNeutrinoSettings::BUTTON_RED) 
	{
                if( txt.empty() )
                        txt = g_Locale->getText(LOCALE_INFOVIEWER_EVENTLIST);
        }
        else if( button == SNeutrinoSettings::BUTTON_GREEN) 
	{
                if( txt.empty() )
                        txt = g_Locale->getText(LOCALE_INFOVIEWER_LANGUAGES);
        }
        else if( button == SNeutrinoSettings::BUTTON_YELLOW) 
	{
                if( txt.empty() )
                        txt = g_Locale->getText((g_RemoteControl->are_subchannels) ? LOCALE_INFOVIEWER_SUBSERVICE : LOCALE_INFOVIEWER_SELECTTIME);
        }
        else if( button == SNeutrinoSettings::BUTTON_BLUE) 
	{
                if( txt.empty() )
                        txt = g_Locale->getText(LOCALE_INFOVIEWER_FEATURES);
        }

        CMenuWidget * menu = new CMenuWidget(txt.c_str() , NEUTRINO_ICON_FEATURES);
        if (menu == NULL)
                return 0;
	
	// intros
	menu->addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	menu->addItem( new CMenuSeparator(CMenuSeparator::LINE) );

        /* go through any postition number */
        for(int pos = 0; pos < SNeutrinoSettings::ITEM_MAX ; pos++) 
	{
                /* now compare pos with the position of any item. Add this item if position is the same */
                switch(g_settings.usermenu[button][pos]) 
		{
			case SNeutrinoSettings::ITEM_NONE:
				// do nothing
				break;

			case SNeutrinoSettings::ITEM_BAR:
				if(menu_prev == -1 || menu_prev == SNeutrinoSettings::ITEM_BAR )
					break;

				menu->addItem(new CMenuSeparator(CMenuSeparator::LINE));
				menu_prev = SNeutrinoSettings::ITEM_BAR;
				break;

                        case SNeutrinoSettings::ITEM_MOVIEPLAYER_TSMB:
                                menu_items++;
                                menu_prev = SNeutrinoSettings::ITEM_MOVIEPLAYER_TSMB;
                                keyhelper.get(&key, &icon, CRCInput::RC_green);
				menu_item = new CMenuForwarder(LOCALE_MOVIEPLAYER_RECORDS, true, NULL, moviePlayerGui, "tsmoviebrowser", key, icon);
                                menu->addItem(menu_item, false);
                                break;
				
			case SNeutrinoSettings::ITEM_MOVIEPLAYER_MB:
                                menu_items++;
                                menu_prev = SNeutrinoSettings::ITEM_MOVIEPLAYER_MB;
                                keyhelper.get(&key, &icon, CRCInput::RC_green);
				menu_item = new CMenuForwarder(LOCALE_MOVIEPLAYER_MOVIES, true, NULL, moviePlayerGui, "moviebrowser", key, icon);
                                menu->addItem(menu_item, false);
                                break;

                        case SNeutrinoSettings::ITEM_TIMERLIST:
                                menu_items++;
                                menu_prev = SNeutrinoSettings::ITEM_TIMERLIST;
                                keyhelper.get(&key, &icon, CRCInput::RC_yellow);
				menu_item = new CMenuForwarder(LOCALE_TIMERLIST_NAME, true, NULL, Timerlist, "-1", key, icon);
                                menu->addItem(menu_item, false);
                                break;

                        case SNeutrinoSettings::ITEM_REMOTE:
                                menu_items++;
                                menu_prev = SNeutrinoSettings::ITEM_REMOTE;
                                keyhelper.get(&key, &icon);
				menu_item = new CMenuForwarder(LOCALE_RCLOCK_MENUEADD, true, NULL, this->rcLock, "-1", key, icon);
                                menu->addItem(menu_item, false);
                                break;

                        case SNeutrinoSettings::ITEM_EPG_SUPER:
				if (CNeutrinoApp::getInstance()->getMode() != NeutrinoMessages::mode_iptv)
				{
					menu_items++;
					menu_prev = SNeutrinoSettings::ITEM_EPG_SUPER;
					tmpEPGplusHandler = new CEPGplusHandler();
					keyhelper.get(&key, &icon, CRCInput::RC_green);
					menu_item = new CMenuForwarder(LOCALE_EPGMENU_EPGPLUS, true, NULL, tmpEPGplusHandler  ,  "-1", key, icon);
					menu->addItem(menu_item, false);
				}
                                break;

                        case SNeutrinoSettings::ITEM_EPG_LIST:
				if (CNeutrinoApp::getInstance()->getMode() != NeutrinoMessages::mode_iptv)
				{
					menu_items++;
					menu_prev = SNeutrinoSettings::ITEM_EPG_LIST;
					tmpEventListHandler = new CEventListHandler();
					keyhelper.get(&key, &icon, CRCInput::RC_red);
					menu_item = new CMenuForwarder(LOCALE_EPGMENU_EVENTLIST, true, NULL, tmpEventListHandler,  "-1", key, icon);
					menu->addItem(menu_item, false);
				}
                                break;

                        case SNeutrinoSettings::ITEM_EPG_INFO:
				if (CNeutrinoApp::getInstance()->getMode() != NeutrinoMessages::mode_iptv)
				{
					menu_items++;
					menu_prev = SNeutrinoSettings::ITEM_EPG_INFO;
					tmpEPGDataHandler = new CEPGDataHandler();
					keyhelper.get(&key, &icon, CRCInput::RC_yellow);
					menu_item = new CMenuForwarder(LOCALE_EPGMENU_EVENTINFO, true, NULL, tmpEPGDataHandler ,  "-1", key, icon);
					menu->addItem(menu_item, false);
				}
                                break;

                        case SNeutrinoSettings::ITEM_AUDIO_SELECT:
				if (CNeutrinoApp::getInstance()->getMode() != NeutrinoMessages::mode_iptv)
				{
					menu_items++;
					menu_prev = SNeutrinoSettings::ITEM_AUDIO_SELECT;
					tmpAudioSelectMenuHandler = new CAudioSelectMenuHandler;
					keyhelper.get(&key, &icon);
					menu_item = new CMenuForwarder(LOCALE_AUDIOSELECTMENUE_HEAD, true, NULL, tmpAudioSelectMenuHandler, "-1", key,icon);
					menu->addItem(menu_item, false);
				}
                                break;

                        case SNeutrinoSettings::ITEM_SUBCHANNEL:
				if (CNeutrinoApp::getInstance()->getMode() != NeutrinoMessages::mode_iptv)
				{
					if (!(g_RemoteControl->subChannels.empty())) 
					{
						// NVOD/SubService- Kanal!
						tmpNVODSelector = new CMenuWidget(g_RemoteControl->are_subchannels ? LOCALE_NVODSELECTOR_SUBSERVICE : LOCALE_NVODSELECTOR_HEAD, NEUTRINO_ICON_VIDEO);
						
						if(getNVODMenu(tmpNVODSelector)) 
						{
							menu_items++;
							menu_prev = SNeutrinoSettings::ITEM_SUBCHANNEL;
							keyhelper.get(&key, &icon);
							menu_item = new CMenuForwarder(g_RemoteControl->are_subchannels ? LOCALE_NVODSELECTOR_SUBSERVICE : LOCALE_NVODSELECTOR_HEAD, true, NULL, tmpNVODSelector, "-1", key,icon);
							menu->addItem(menu_item, false);
						}
					}
				}
                                break;

                        case SNeutrinoSettings::ITEM_TECHINFO:
				if (CNeutrinoApp::getInstance()->getMode() != NeutrinoMessages::mode_iptv)
				{
					menu_items++;
					menu_prev = SNeutrinoSettings::ITEM_TECHINFO;
					tmpStreamInfo2Handler = new CStreamInfo2Handler();
					keyhelper.get(&key, &icon, CRCInput::RC_blue);
					menu_item = new CMenuForwarder(LOCALE_EPGMENU_STREAMINFO, true, NULL, tmpStreamInfo2Handler, "-1", key, icon);
					menu->addItem(menu_item, false);
				}
                                break;
				
			case SNeutrinoSettings::ITEM_VTXT:
				if (CNeutrinoApp::getInstance()->getMode() != NeutrinoMessages::mode_iptv)
				{
					menu_items++;
					menu_prev = SNeutrinoSettings::ITEM_VTXT;
					keyhelper.get(&key, &icon);
					menu_item = new CMenuForwarder(LOCALE_USERMENU_ITEM_VTXT, true, NULL, TuxtxtChanger, "-1", key, icon);
					menu->addItem(menu_item, false);
				}
                                break;	

			case SNeutrinoSettings::ITEM_OPKG:
                                menu_items++;
                                menu_prev = SNeutrinoSettings::ITEM_OPKG;
                               
				tmpOPKGManager = new COPKGManager();
				
                                keyhelper.get(&key, &icon);
                                menu_item = new CMenuForwarder(LOCALE_OPKG_MANAGER, true, NULL, tmpOPKGManager, "-1", key, icon);
                                menu->addItem(menu_item, false);
                                break;
			
			// plugins
                        case SNeutrinoSettings::ITEM_PLUGIN:
				{
					menu_item++;
					menu_prev = SNeutrinoSettings::ITEM_PLUGIN;
					keyhelper.get(&key, &icon, CRCInput::RC_blue);
					menu_item = new CMenuForwarder(LOCALE_USERMENU_ITEM_PLUGINS, true, NULL, new CPluginList( LOCALE_USERMENU_ITEM_PLUGINS, CPlugins::P_TYPE_TOOL | CPlugins::P_TYPE_SCRIPT ), "-1", key, icon);
					menu->addItem(menu_item, false);
                                }
                                break;
				
			// games
			case SNeutrinoSettings::ITEM_GAME:
				{
					menu_item++;
					menu_prev = SNeutrinoSettings::ITEM_GAME;
					keyhelper.get(&key, &icon);
					menu_item = new CMenuForwarder(LOCALE_MAINMENU_GAMES, true, NULL, new CPluginList(LOCALE_MAINMENU_GAMES, CPlugins::P_TYPE_GAME), "-1", key, icon);
					menu->addItem(menu_item, false);
                                }
                                break;

                        default:
                                printf("[neutrino] WARNING! menu wrong item!!\n");
                                break;
                }
        }

        // show menu if there are more than 2 items only
	// otherwise, we start the item directly (must be the last one)
        if(menu_items > 1 ) 
	{
		menu->setSelected(selected[button]);
                menu->exec(NULL,"");
		selected[button] = menu->getSelected();
	}
        else if (menu_item != NULL)
                menu_item->exec( NULL );

        // clear the heap
        if(tmpPauseSectionsdNotifier)
		delete tmpPauseSectionsdNotifier;

        if(tmpAudioSelectMenuHandler)   
		delete tmpAudioSelectMenuHandler;

        if(tmpNVODSelector)
		delete tmpNVODSelector;

        if(tmpStreamInfo2Handler)
		delete tmpStreamInfo2Handler;

        if(tmpEventListHandler)
		delete tmpEventListHandler;

        if(tmpEPGplusHandler)
		delete tmpEPGplusHandler;

        if(tmpEPGDataHandler)
		delete tmpEPGDataHandler;
	
	if(tmpOPKGManager)
		delete tmpOPKGManager;

        if(menu)
		delete menu;

	return 0;
}

