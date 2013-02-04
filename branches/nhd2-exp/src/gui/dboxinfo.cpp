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
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <gui/dboxinfo.h>

#include <global.h>

#include <driver/fontrenderer.h>
#include <driver/screen_max.h>
#include <driver/rcinput.h>

#include <sys/sysinfo.h>
#include <sys/vfs.h>

/*zapit includes*/
#include <frontend_c.h>


static const int FSHIFT = 16;              /* nr of bits of precision */
#define FIXED_1         (1<<FSHIFT)     /* 1.0 as fixed-point */
#define LOAD_INT(x) ((x) >> FSHIFT)
#define LOAD_FRAC(x) LOAD_INT(((x) & (FIXED_1-1)) * 100)

extern int FrontendCount;
CFrontend * getFE(int index);

// hdd
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <dlfcn.h>
#include <sys/mount.h>


static int my_filter(const struct dirent * dent)
{
	if(dent->d_name[0] == 's' && dent->d_name[1] == 'd')
		return 1;
	
	return 0;
}

CDBoxInfoWidget::CDBoxInfoWidget()
{
	frameBuffer = CFrameBuffer::getInstance();
	
	hheight     = g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight();
	mheight     = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight();
	
	width  = w_max (MENU_WIDTH + 100, 0);
	height = h_max (hheight + (10 + FrontendCount)*mheight + 20, 0);
	
    	x = (((g_settings.screen_EndX - g_settings.screen_StartX) - width) / 2) + g_settings.screen_StartX;
	y = (((g_settings.screen_EndY - g_settings.screen_StartY) - height) / 2) + g_settings.screen_StartY;
}

int CDBoxInfoWidget::exec(CMenuTarget * parent, const std::string &)
{
	if (parent)
	{
		parent->hide();
	}
	
	paint();
		
#if !defined USE_OPENGL
	frameBuffer->blit();
#endif	

	int res = g_RCInput->messageLoop();

	hide();	

	return res;
}

void CDBoxInfoWidget::hide()
{
	frameBuffer->paintBackgroundBoxRel(x, y, width, height);
	
#if !defined USE_OPENGL
	frameBuffer->blit();
#endif
}

void CDBoxInfoWidget::paint()
{
	int ypos=y;
	int i = 0;

	// head
	frameBuffer->paintBoxRel(x, ypos, width, hheight, COL_MENUHEAD_PLUS_0, RADIUS_MID, CORNER_TOP);
	
	// icon
	frameBuffer->paintIcon(NEUTRINO_ICON_INFO, x + 8, ypos + 8);
	
	// title
	int neededWidth = g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getRenderWidth("Box info", true); // UTF-8
	int stringstartposX = x +(width >> 1) - (neededWidth >> 1);
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(stringstartposX, ypos + hheight + 1, width - (stringstartposX - x) , "Box info", COL_MENUHEAD, 0, true); // UTF-8

	// foot
	frameBuffer->paintBoxRel(x, ypos + hheight, width, height - hheight, COL_MENUCONTENT_PLUS_0, RADIUS_MID, CORNER_BOTTOM);

	ypos += hheight + (mheight >>1);


	//cpu
	FILE* fd = fopen("/proc/cpuinfo", "rt");

	if (fd==NULL) 
	{
		printf("error while opening proc-cpuinfo\n" );
	} 
	else 
	{
		char *buffer=NULL;
		size_t len = 0;
		ssize_t read;
		while ((read = getline(&buffer, &len, fd)) != -1) 
		{
			if (!(strncmp(const_cast<char *>("Hardware"),buffer, 8))) 
			{
				char *t=rindex(buffer,'\n');
				if (t)
					*t='\0';

				std::string hw;
				char *p=rindex(buffer,':');
				if (p)
					hw=++p;
				hw+=" Info";
				g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(x+10, y + hheight+1, width - 10, hw.c_str(), COL_MENUHEAD, 0, true); // UTF-8
				break;
			}
			i++;
			if (i > 4)
				continue;

			g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(x+ 10, ypos+ mheight, width - 10, buffer, COL_MENUCONTENT, true);
			ypos+= mheight;
		}
		fclose(fd);

		if (buffer)
			free(buffer);
	}

	// up
	int updays, uphours, upminutes;
	struct sysinfo info;
	struct tm *current_time;
	time_t current_secs;
	char ubuf[256], sbuf[256];

	memset(sbuf, 0, 256);
	time(&current_secs);
	current_time = localtime(&current_secs);

	sysinfo(&info);

	sprintf( ubuf, "%2d:%02d%s  up ", 
			current_time->tm_hour%12 ? current_time->tm_hour%12 : 12, 
			current_time->tm_min, current_time->tm_hour > 11 ? "pm" : "am");

	strcat(sbuf, ubuf);
	updays = (int) info.uptime / (60*60*24);
	if (updays) 
	{
		sprintf(ubuf, "%d day%s, ", updays, (updays != 1) ? "s" : "");
		strcat(sbuf, ubuf);
	}
	upminutes = (int) info.uptime / 60;
	uphours = (upminutes / 60) % 24;
	upminutes %= 60;
	if(uphours)
		sprintf(ubuf,"%2d:%02d, ", uphours, upminutes);
	else
		sprintf(ubuf,"%d min, ", upminutes);
	strcat(sbuf, ubuf);

	sprintf(ubuf, "load: %ld.%02ld, %ld.%02ld, %ld.%02ld\n", 
			LOAD_INT(info.loads[0]), LOAD_FRAC(info.loads[0]), 
			LOAD_INT(info.loads[1]), LOAD_FRAC(info.loads[1]), 
			LOAD_INT(info.loads[2]), LOAD_FRAC(info.loads[2]));

	strcat(sbuf, ubuf);
	ypos += mheight/2;
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(x+ 10, ypos+ mheight, width, sbuf, COL_MENUCONTENT, true);
	
	// mem
	ypos += mheight;

	sprintf(ubuf, "memory total %dKb, free %dKb", (int) info.totalram/1024, (int) info.freeram/1024);
	ypos += mheight/2;

	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(x+ 10, ypos+ mheight, width, ubuf, COL_MENUCONTENT, true);
    	
    	//hdd devices
	FILE * f;
	int fd_hdd;
	struct dirent **namelist;
	
	int n = scandir("/sys/block", &namelist, my_filter, alphasort);
	
	for(int i = 0; i < n; i++) 
	{
		ypos += mheight;
		
		char str[256];
		char vendor[128];
		char model[128];
		int64_t bytes;
		int64_t megabytes;
		int removable = 0;
		
		sprintf(str, "/dev/%s", namelist[i]->d_name);
		fd_hdd = open(str, O_RDONLY);

		if(fd_hdd < 0) 
		{
			//printf("[neutrino] HDD: Cant open %s\n", str);
			continue;
		}
		
		if (ioctl(fd_hdd, BLKGETSIZE64, &bytes))
			perror("BLKGETSIZE64");
                
                close(fd_hdd);

		megabytes = bytes/1000000;

		// vendor
		sprintf(str, "/sys/block/%s/device/vendor", namelist[i]->d_name);
		f = fopen(str, "r");

		if(!f) 
		{
			continue;
		}
		fscanf(f, "%s", vendor);
		fclose(f);

		// model
		sprintf(str, "/sys/block/%s/device/model", namelist[i]->d_name);
		f = fopen(str, "r");
		
		if(!f) 
		{
			continue;
		}
		fscanf(f, "%s", model);
		fclose(f);

		// removable
		sprintf(str, "/sys/block/%s/removable", namelist[i]->d_name);
		f = fopen(str, "r");
		
		if(!f) 
		{
			continue;
		}
		fscanf(f, "%d", &removable);
		fclose(f);
		
		// free space on hdd
		struct statfs s;
		
		if (::statfs(g_settings.network_nfs_recordingdir, &s) == 0) 
		{
			//strcat(str, ", free %ldKb", (long)(s.f_bfree/1024)*s.f_bsize);
			sprintf(str, "HDD: %s (%s-%s %lld %s), free %ldMB", namelist[i]->d_name, vendor, model, megabytes < 10000 ? megabytes : megabytes/1000, megabytes < 10000 ? "MB" : "GB", (long)( ((s.f_bfree/1024)/1024))*s.f_bsize);
		}
		else
			sprintf(str, "HDD: %s (%s-%s %lld %s)", namelist[i]->d_name, vendor, model, megabytes < 10000 ? megabytes : megabytes/1000, megabytes < 10000 ? "MB" : "GB");
		
		free(namelist[i]);
		
		g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(x+ 10, ypos + mheight, width, str, COL_MENUCONTENT, true);
	}

	// free space
	ypos += mheight/2;
	
	//frontend
	for(int i = 0; i < FrontendCount; i++)
	{
		CFrontend * fe = getFE(i);
		ypos += mheight;
		char tbuf[255];
		
		sprintf(tbuf, "Tuner-%d: %s", i + 1, fe->getInfo()->name);
		g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(x+ 10, ypos+ mheight, width, tbuf, COL_MENUCONTENT, true);
	}	
}
