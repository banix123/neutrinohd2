/*
	Neutrino-GUI  -   DBoxII-Project

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

#ifndef __MOD_rcinput__
#define __MOD_rcinput__

#include <linux/input.h>
#include <sys/types.h>
#include <string>
#include <vector>
#include <stdint.h>

#include <configfile.h>


#ifndef KEY_OK
#define KEY_OK           0x160
#endif

#ifndef KEY_RED
#define KEY_RED          0x18e
#endif

#ifndef KEY_GREEN
#define KEY_GREEN        0x18f
#endif

#ifndef KEY_YELLOW
#define KEY_YELLOW       0x190
#endif

#ifndef KEY_BLUE
#define KEY_BLUE         0x191
#endif

// SAGEM remote controls have the following additional keys
#ifndef KEY_TOPLEFT
#define KEY_TOPLEFT      0x1a2
#endif

#ifndef KEY_TOPRIGHT
#define KEY_TOPRIGHT     0x1a3
#endif

#ifndef KEY_BOTTOMLEFT
#define KEY_BOTTOMLEFT   0x1a4
#endif

#ifndef KEY_BOTTOMRIGHT
#define KEY_BOTTOMRIGHT  0x1a5
#endif

#ifndef KEY_GAMES
#define KEY_GAMES        0x1a1
#endif

/* this values are token from cuberevo3000hd */
#ifndef KEY_PIP	
#define KEY_PIP		0x041
#endif

#ifndef KEY_PIPPOS	
#define KEY_PIPPOS	0x0BE
#endif

#ifndef KEY_PIPSWAP 
#define KEY_PIPSWAP	0x0BF
#endif

#ifndef __KEY_PIPSUBCH	
#define KEY_PIPSUBCH	0x0C0
#endif

#ifndef KEY_BOOKMARK	
#define	KEY_BOOKMARK	0x03f
#endif

#ifndef KEY_MUSIC	
#define KEY_MUSIC	0x0c1
#endif

#ifndef KEY_PICTURE	
#define KEY_PICTURE	0x03e
#endif

#ifndef KEY_REPEAT	
#define	KEY_REPEAT	0x040
#endif

#ifndef KEY_SLOW	
#define KEY_SLOW	0x199
#endif

#ifndef KEY_MULTFEED	
#define KEY_MULTIFEED	0x0bd
#endif

#ifndef KEY_DVBSUB	
#define KEY_DVBSUB	0x172
#endif

#ifndef KEY_NET
#define KEY_NET		0x096
#endif

/* VFD */
#define VFD_UP		0x042
#define VFD_DOWN	0x043
#define VFD_RIGHT	0x057
#define VFD_LEFT	0x044
#define VFD_POWER	0x0ba
#define VFD_MENU	0x0b8
#define VFD_EXIT	0x0b7
#define VFD_OK		0x058



typedef uint32_t neutrino_msg_t;
typedef uint32_t neutrino_msg_data_t;

#define NEUTRINO_UDS_NAME 		"/tmp/neutrino.sock"
#define NEUTRINO_KEYMAP_FILE		CONFIGDIR "/keymap.conf"


class CRCInput
{
	private:
		struct event
		{
			neutrino_msg_t      msg;
			neutrino_msg_data_t data;
		};

		struct timer
		{
			uint			id;
			unsigned long long	interval;
			unsigned long long	times_out;
			bool			correct_time;
		};

		uint32_t               timerid;
		std::vector<timer> timers;

		int 		fd_pipe_high_priority[2];
		int 		fd_pipe_low_priority[2];

#define NUMBER_OF_EVENT_DEVICES 4

		int         	fd_rc[NUMBER_OF_EVENT_DEVICES];
		int		fd_keyb;
		int		fd_event;
		int		fd_max;
		
		__u16 rc_last_key;

		void open();
		void close();
		int translate(unsigned int code, int num);

		void calculateMaxFd(void);

		int checkTimers();

	public:
		//rc-code definitions
		static const neutrino_msg_t RC_Repeat   = 0x0400;
		static const neutrino_msg_t RC_Release  = 0x0800;
		static const neutrino_msg_t RC_MaxRC    = KEY_MAX | RC_Repeat | RC_Release; /* /include/linux/input.h: #define KEY_MAX 0x1ff */
		static const neutrino_msg_t RC_KeyBoard = 0x4000;
		static const neutrino_msg_t RC_Events   = 0x80000000;
		static const neutrino_msg_t RC_Messages = 0x90000000;
		static const neutrino_msg_t RC_WithData = 0xA0000000;

		enum
		{
			RC_0		= KEY_0,	    /* /include/linux/input.h: #define KEY_0			 0xb   */
			RC_1		= KEY_1,	    /* /include/linux/input.h: #define KEY_1			  0x2   */
			RC_2		= KEY_2,	    /* /include/linux/input.h: #define KEY_2			  0x3   */
			RC_3		= KEY_3,	    /* /include/linux/input.h: #define KEY_3			  0x4   */
			RC_4		= KEY_4,	    /* /include/linux/input.h: #define KEY_4			  0x5   */
			RC_5		= KEY_5,	    /* /include/linux/input.h: #define KEY_5			  0x6   */
			RC_6		= KEY_6,	    /* /include/linux/input.h: #define KEY_6			  0x7   */
			RC_7		= KEY_7,	    /* /include/linux/input.h: #define KEY_7			  0x8   */
			RC_8		= KEY_8,	    /* /include/linux/input.h: #define KEY_8			  0x9   */
			RC_9		= KEY_9,	    /* /include/linux/input.h: #define KEY_9			 0xa   */
			
			RC_up		= KEY_UP,	    /* /include/linux/input.h: #define KEY_UP			0x67   */
			RC_left		= KEY_LEFT,	    /* /include/linux/input.h: #define KEY_LEFT			0x69   */
			RC_right	= KEY_RIGHT,	    /* /include/linux/input.h: #define KEY_RIGHT		0x6a   */
			RC_down		= KEY_DOWN,	    /* /include/linux/input.h: #define KEY_DOWN			0x6c   */
			
			RC_spkr		= KEY_MUTE,	    /* /include/linux/input.h: #define KEY_MUTE			0x71   */
			
			RC_minus        = KEY_VOLUMEDOWN,   /* /include/linux/input.h: #define KEY_VOLUMEDOWN          114   */
			RC_plus         = KEY_VOLUMEUP,     /* /include/linux/input.h: #define KEY_VOLUMEUP            115   */			

			RC_standby	= KEY_POWER,	    /* /include/linux/input.h: #define KEY_POWER		0x74   */			
			
			RC_home         = KEY_HOME,         /* /include/linux/input.h: #define KEY_HOME                	0x66   */
			RC_setup	= KEY_MENU,	    /* /include/linux/input.h: #define KEY_SETUP		0x8d   */
			
			RC_page_up	= KEY_PAGEUP,	    /* /include/linux/input.h: #define KEY_PAGEUP		0x68   */
			RC_page_down	= KEY_PAGEDOWN,	    /* /include/linux/input.h: #define KEY_PAGEDOWN		0x6d   */
			
			RC_ok		= KEY_OK,	    /* /include/linux/input.h: #define KEY_OK			0x160 */ /* in patched input.h */
			
			RC_red		= KEY_RED,	    /* /include/linux/input.h: #define KEY_RED			0x18e */ /* in patched input.h */
			RC_green	= KEY_GREEN,	    /* /include/linux/input.h: #define KEY_GREEN		0x18f */ /* in patched input.h */
			RC_yellow	= KEY_YELLOW,	    /* /include/linux/input.h: #define KEY_YELLOW		0x190 */ /* in patched input.h */
			RC_blue		= KEY_BLUE,	    /* /include/linux/input.h: #define KEY_BLUE			0x191 */ /* in patched input.h */

			RC_audio	= KEY_AUDIO,		/* 0x188 */
			RC_video	= KEY_VIDEO,		/* 0x189 */
			
			RC_text		= KEY_TEXT,		/* 0x184 */
			RC_info		= KEY_INFO,		/* 0x166 */			
			RC_epg		= KEY_EPG,		/* 0x16d */
			RC_recall 	= KEY_BACK,		/* 0x9E */
			RC_favorites	= KEY_FAVORITES,	/* 0x16c */
			RC_sat		= KEY_SAT,		/* 0x17d */
			
			RC_record	= KEY_RECORD,		/* 0xA7 */
			RC_play		= KEY_PLAY,		/* 0xCF */
			RC_pause	= KEY_PAUSE,		/* 0x77 */
			RC_forward	= KEY_FASTFORWARD,	/* 0xD0 */
			RC_rewind	= KEY_REWIND,		/* 0xA8 */
			RC_stop		= KEY_STOP,		/* 0x80 */
			
			RC_timeshift	= KEY_TIME,		/* 0x167 */
						
			RC_mode		= KEY_MODE,		/* 0x175 */

			RC_next		= 0xFFFFFFF0,
			RC_prev		= 0xFFFFFFF1,

			/* added from cuberevo3000hd so fix it please */
			RC_music	= KEY_MUSIC,
			RC_picture	= /*KEY_PICTURE*/KEY_ARCHIVE,			
			
			RC_repeat	= KEY_REPEAT,
			RC_slow		= KEY_SLOW,
			
			RC_dvbsub	= KEY_DVBSUB,

			RC_pip		= KEY_PIP,
			RC_pippos	= KEY_PIPPOS,
			RC_pipswap	= KEY_PIPSWAP,
			RC_pipsubch	= KEY_PIPSUBCH,

			RC_net		= KEY_NET,
			RC_bookmark	= KEY_BOOKMARKS,
			RC_multifeed	= KEY_MULTIFEED,

			/* functions */
			RC_f1		= KEY_F1,
			RC_f2		= KEY_F2,
			RC_f3		= KEY_F3,
			RC_f4		= KEY_F4,
			
			//RC_aspect	= 0x40,

			/* frontpanel */
			RC_vfdup	= VFD_UP,
			RC_vfddown	= VFD_DOWN,
			RC_vfdright	= VFD_RIGHT,
			RC_vfdleft	= VFD_LEFT,
			RC_vfdpower	= VFD_POWER,
			RC_vfdmenu	= VFD_MENU,
			RC_vfdexit	= VFD_EXIT,
			RC_vfdok	= VFD_OK,

			RC_timeout	= 0xFFFFFFFF,
			RC_nokey	= 0xFFFFFFFE
		};
		
		CConfigFile	configfile;

		neutrino_msg_t key_0;
		neutrino_msg_t key_1;
		neutrino_msg_t key_2;
		neutrino_msg_t key_3;
		neutrino_msg_t key_4;
		neutrino_msg_t key_5;
		neutrino_msg_t key_6;
		neutrino_msg_t key_7;
		neutrino_msg_t key_8;
		neutrino_msg_t key_9;
			
		neutrino_msg_t key_up;
		neutrino_msg_t key_left;
		neutrino_msg_t key_right;
		neutrino_msg_t key_down;
			
		neutrino_msg_t key_spkr;
						
		neutrino_msg_t key_minus;
		neutrino_msg_t key_plus;			

		neutrino_msg_t key_standby;
		neutrino_msg_t key_home;
		neutrino_msg_t key_setup;				
			
		neutrino_msg_t key_page_up;
		neutrino_msg_t key_page_down;			
			
		neutrino_msg_t key_ok;
			
		neutrino_msg_t key_red;
		neutrino_msg_t key_green;
		neutrino_msg_t key_yellow;
		neutrino_msg_t key_blue;

		neutrino_msg_t key_audio;
		neutrino_msg_t key_video;
			
		neutrino_msg_t key_text;

		neutrino_msg_t key_info;				
			
		neutrino_msg_t key_epg;
			
		neutrino_msg_t key_recall;		

		neutrino_msg_t key_favorites;

		neutrino_msg_t key_sat;
			
		neutrino_msg_t key_record;
		neutrino_msg_t key_play;
		neutrino_msg_t key_pause;
		neutrino_msg_t key_forward;
		neutrino_msg_t key_rewind;
		neutrino_msg_t key_stop;
			
		neutrino_msg_t key_timeshift;
						
		neutrino_msg_t key_mode;			

		neutrino_msg_t key_next;
		neutrino_msg_t key_prev;			

		/* added from cuberevo3000hd so fix it please */
		neutrino_msg_t key_music;
		neutrino_msg_t key_picture;	
		neutrino_msg_t key_repeat;
		neutrino_msg_t key_slow;
			
		neutrino_msg_t key_dvbsub;

		neutrino_msg_t key_pip;
		neutrino_msg_t key_pippos;
		neutrino_msg_t key_pipswap;
		neutrino_msg_t key_pipsubch;

		neutrino_msg_t key_net;	
		neutrino_msg_t key_bookmark;
		neutrino_msg_t key_multifeed;
					
		neutrino_msg_t key_f1;
		neutrino_msg_t key_f2;
		neutrino_msg_t key_f3;
		neutrino_msg_t key_f4;
		
		//neutrino_msg_t key_aspect;
			
		neutrino_msg_t key_vfdup;
		neutrino_msg_t key_vfddown;
		neutrino_msg_t key_vfdright;
		neutrino_msg_t key_vfdleft;
		neutrino_msg_t key_vfdpower;
		neutrino_msg_t key_vfdmenu;
		neutrino_msg_t key_vfdexit;
		neutrino_msg_t key_vfdok;
		
		bool loadKeyMap(const char * const fileName);
		bool saveKeyMap(const char * const fileName);

		inline int getFileHandle(void) /* used for tuxtxt and plugins */
		{
#if defined (PLATFORM_VIP) || defined (PLATFORM_XTREND)
				return fd_rc[1];
#else
				return fd_rc[0];
#endif				
		}
		
		void stopInput();
		void restartInput();

		unsigned long long repeat_block;
		unsigned long long repeat_block_generic;
		
		void setRepeat(unsigned int delay, unsigned int period);
		
		CRCInput();      //constructor - opens rc-device and starts needed threads
		~CRCInput();     //destructor - closes rc-device

		static bool isNumeric(const neutrino_msg_t key);
		static int getNumericValue(const neutrino_msg_t key);
		static unsigned int convertDigitToKey(const unsigned int digit);
		static int getUnicodeValue(const neutrino_msg_t key);

		static const char * getSpecialKeyName(const unsigned int key);
		static std::string getKeyName(const unsigned int key);

		int addTimer(unsigned long long Interval, bool oneshot = true, bool correct_time = true );
		int addTimer(struct timeval Timeout);
		int addTimer(const time_t *Timeout);

		void killTimer(uint32_t id);

		static long long calcTimeoutEnd_MS(const int timeout_in_milliseconds);
		static long long calcTimeoutEnd(const int timeout_in_seconds);

		void getMsgAbsoluteTimeout(neutrino_msg_t * msg, neutrino_msg_data_t * data, unsigned long long *TimeoutEnd, bool bAllowRepeatLR = false);
		void getMsg(neutrino_msg_t * msg, neutrino_msg_data_t * data, int Timeout, bool bAllowRepeatLR= false);                  //get message, timeout in 1/10 secs :)
		void getMsg_ms(neutrino_msg_t * msg, neutrino_msg_data_t * data, int Timeout, bool bAllowRepeatLR= false);               //get message, timeout in msecs :)
		void getMsg_us(neutrino_msg_t * msg, neutrino_msg_data_t * data, unsigned long long Timeout, bool bAllowRepeatLR= false);//get message, timeout in �secs :)
		void postMsg(const neutrino_msg_t msg, const neutrino_msg_data_t data, const bool Priority = true);  // push message back into buffer
		void clearRCMsg();

		int messageLoop( bool anyKeyCancels = false, int timeout= -1 );
};


#endif
