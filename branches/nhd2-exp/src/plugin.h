/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: plugin.h 2015/07/18 mohousch Exp $

	Copyright (C) 2001 Steffen Hehn 'McClean'
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
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#ifndef TPLUGIN_H
#define TPLUGIN_H

#include <config.h>

#include <stdio.h>
#include <dirent.h>

#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>

#include <semaphore.h>

#include <system/set_threadname.h>

// 
#include <client/zapitclient.h>
#include <sectionsdclient/sectionsdclient.h>
#include <timerdclient/timerdclient.h>

#include <driver/fontrenderer.h>
#include <driver/rcinput.h>
#include <driver/radiotext.h>
#include <driver/encoding.h>
#include <driver/framebuffer.h>
#include <driver/fontrenderer.h>
#include <driver/rcinput.h>
#include <driver/stream2file.h>
#include <driver/vcrcontrol.h>
#include <driver/shutdown_count.h>
#include <driver/screen_max.h>
#include <driver/audioplay.h>
#if ENABLE_LCD
#include <driver/lcdd.h>
#define CVFD CLCD
#else
#if defined (PLATFORM_COOLSTREAM)
#include <cs_vfd.h>
#endif
#include "driver/vfd.h"
#endif
#include <driver/file.h>

#include <daemonc/remotecontrol.h>

#include <gui/epgview.h>
#include <gui/infoviewer.h>
#include <gui/eventlist.h>
#include <gui/epgplus.h>
#include <gui/streaminfo2.h>
#include <gui/widget/colorchooser.h>
#include <gui/widget/menue.h>
#include <gui/widget/messagebox.h>
#include <gui/widget/hintbox.h>
#include <gui/widget/icons.h>
#include <gui/widget/keychooser.h>
#include <gui/widget/stringinput.h>
#include <gui/widget/stringinput_ext.h>
#include <gui/widget/mountchooser.h>
#include <gui/widget/buttons.h>
#include <gui/widget/helpbox.h>
#include <gui/widget/infobox.h>
#include <gui/widget/listbox.h>
#include <gui/widget/textbox.h>
#include <gui/widget/vfdcontroler.h>
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
#include <gui/widget/progressbar.h>
#include <gui/cam_menu.h>
#include <gui/hdd_menu.h>
#include <gui/dboxinfo.h>
#include <gui/audio_select.h>
#include <gui/scan_setup.h>
#include <gui/dvbsub_select.h>
#include <gui/webtv.h>
#include <gui/timeosd.h>
#include <gui/upnpbrowser.h>
#include <gui/channel_select.h>
#include <gui/audio_video_select.h>
#include <gui/bedit/bouqueteditor_chanselect.h>
#include <gui/bedit/bouqueteditor_channels.h>
#include <gui/bedit/bouqueteditor_bouquets.h>
#include <gui/psisetup.h>
#include <gui/rc_lock.h>

#include <system/localize.h>
#include <system/settings.h>
#include <system/setting_helpers.h>
#include <system/settings.h>
#include <system/debug.h>
#include <system/flashtool.h>
#include <system/fsmounter.h>
#include <system/helpers.h>

#include <timerdclient/timerdmsg.h>

/*zapit includes*/
#include <frontend_c.h>
#include <getservices.h>
#include <satconfig.h>
#include <channel.h>
#include <bouquets.h>

// libdvbapi
#include <playback_cs.h>
#include <video_cs.h>
#include <audio_cs.h>

// neutrino
#include <neutrino.h>


// globals
extern  SNeutrinoSettings g_settings;

extern  CZapitClient *g_Zapit;
extern  CSectionsdClient *g_Sectionsd;
extern  CTimerdClient *g_Timerd;

extern  FBFontRenderClass *g_fontRenderer;

extern  CFont * g_Font[FONT_TYPE_COUNT];
extern  CFont * g_SignalFont;

extern  CRCInput *g_RCInput;

extern  CEpgData *g_EpgData;
extern  CInfoViewer *g_InfoViewer;
extern  EventList *g_EventList;

extern CLocaleManager *g_Locale;

extern CRadioText *g_Radiotext;

extern tallchans allchans;				// defined in zapit.cpp
extern CBouquetManager * g_bouquetManager;		// defined in zapit.cpp

extern bool has_hdd;					// defined in gui/hdd_menu.cpp

// tuxtxt
//extern int  tuxtxt_init();
//extern void tuxtxt_start(int tpid, int source );
extern int  tuxtxt_stop();
extern void tuxtxt_close();
extern void tuxtx_pause_subtitle(bool pause, int source);
extern void tuxtx_stop_subtitle();
extern void tuxtx_set_pid(int pid, int page, const char * cc);
extern int tuxtx_subtitle_running(int *pid, int *page, int *running);
extern int tuxtx_main(int pid, int page, int source );

// dvbsub
//extern int dvbsub_initialise();
extern int dvbsub_init( /*int source*/);
extern int dvbsub_stop();
extern int dvbsub_close();
extern int dvbsub_start(int pid);
extern int dvbsub_pause();
extern int dvbsub_getpid();
extern void dvbsub_setpid(int pid);
extern int dvbsub_terminate();

// streamts thread
extern int streamts_stop;				// defined in streamts.cpp
// zapit thread
extern int zapit_ready;					//defined in zapit.cpp
extern t_channel_id live_channel_id; 			//defined in zapit.cpp
extern CZapitChannel * live_channel;			// defined in zapit.cpp
extern CFrontend * live_fe;
extern CScanSettings * scanSettings;

// sectionsd thread
extern int sectionsd_stop;				// defined in sectionsd.cpp
extern bool timeset;

//Audio/Video Decoder
extern cVideo 		* videoDecoder;		//libcoolstream (video_cs.cpp)
extern cAudio 		* audioDecoder;		//libcoolstream (audio_cs.cpp)

/* bouquets lists */
extern CBouquetList   		* bouquetList; 				//current bqt list

extern CBouquetList   		* TVbouquetList;
extern CBouquetList   		* TVsatList;
extern CBouquetList   		* TVfavList;
extern CBouquetList   		* TVallList;

extern CBouquetList   		* RADIObouquetList;
extern CBouquetList   		* RADIOsatList;
extern CBouquetList   		* RADIOfavList;
extern CBouquetList   		* RADIOallList;

extern CPlugins       		* g_PluginList;
extern CRemoteControl 		* g_RemoteControl;
extern SMSKeyInput 		* c_SMSKeyInput;	//defined in filebrowser and used in ChanneList
extern CMoviePlayerGui		* moviePlayerGui;
extern CPictureViewer 		* g_PicViewer;
//extern CCAMMenuHandler 	* g_CamHandler;

// webtv
extern CWebTV * webtv;
extern CVCRControl::CDevice * recordingdevice;

//
extern cPlayback *playback;

// record and timeshift
extern bool autoshift;
extern uint32_t shift_timer;
extern uint32_t scrambled_timer;
extern char recDir[255];
extern char timeshiftDir[255];
extern std::string tmode;

void sectionsd_getChannelEvents(CChannelEventList &eList, const bool tv_mode, t_channel_id *chidlist, int clen);
void sectionsd_getEventsServiceKey(t_channel_id serviceUniqueKey, CChannelEventList &eList, char search = 0, std::string search_text = "");
void sectionsd_getCurrentNextServiceKey(t_channel_id uniqueServiceKey, CSectionsdClient::responseGetCurrentNextInfoChannelID& current_next );
bool sectionsd_getActualEPGServiceKey(const t_channel_id uniqueServiceKey, CEPGData * epgdata);
bool sectionsd_getEPGidShort(event_id_t epgID, CShortEPGData * epgdata);
bool sectionsd_getEPGid(const event_id_t epgID, const time_t startzeit, CEPGData * epgdata);
bool sectionsd_getComponentTagsUniqueKey(const event_id_t uniqueKey, CSectionsdClient::ComponentTagList& tags);
bool sectionsd_getLinkageDescriptorsUniqueKey(const event_id_t uniqueKey, CSectionsdClient::LinkageDescriptorList& descriptors);
bool sectionsd_getNVODTimesServiceKey(const t_channel_id uniqueServiceKey, CSectionsdClient::NVODTimesList& nvod_list);
void sectionsd_setPrivatePid(unsigned short pid);

#endif
