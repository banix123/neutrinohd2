/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: messagebox.cpp 2013/10/12 mohousch Exp $

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

#include <gui/widget/messagebox.h>

#include <gui/widget/icons.h>
#include <driver/screen_max.h>
#include <global.h>
#include <neutrino.h>


CMessageBox::CMessageBox(const neutrino_locale_t Caption, const char * const Text, const int Width, const char * const Icon, const CMessageBox::result_ Default, const uint32_t ShowButtons) : CHintBoxExt(Caption, Text, Width, Icon)
{
	returnDefaultOnTimeout = false;

	m_height += (m_fheight << 1);

	result = Default;

	showbuttons = ShowButtons;

	int MaxButtonTextWidth = g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getRenderWidth(g_Locale->getText(LOCALE_MESSAGEBOX_CANCEL), true); // UTF-8
	int ButtonWidth = 20 + 33 + MaxButtonTextWidth + 5;
	int num = 0;
	
	if (showbuttons & mbYes)
		num++;
	
	if (showbuttons & mbNo)
		num++;
	
	if (showbuttons & (mbCancel | mbBack | mbOk))
		num++;
	
	int new_width = 15 + num*ButtonWidth;
	if(new_width > m_width)
		m_width = new_width;
}

CMessageBox::CMessageBox(const neutrino_locale_t Caption, ContentLines& Lines, const int Width, const char * const Icon, const CMessageBox::result_ Default, const uint32_t ShowButtons) : CHintBoxExt(Caption, Lines, Width, Icon)
{
	returnDefaultOnTimeout = false;

	m_height += (m_fheight << 1);

	result = Default;

	showbuttons = ShowButtons;
	int MaxButtonTextWidth = g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getRenderWidth(g_Locale->getText(LOCALE_MESSAGEBOX_CANCEL), true); // UTF-8
	int ButtonWidth = 20 + 33 + MaxButtonTextWidth + 5;
	int num = 0;
	
	if (showbuttons & mbYes)
		num++;
	
	if (showbuttons & mbNo)
		num++;
	
	if (showbuttons & (mbCancel | mbBack | mbOk))
		num++;
	
	int new_width = 15 + num*ButtonWidth;
	if(new_width > m_width)
		m_width = new_width;
}

CMessageBox::CMessageBox(const char* const Caption, const char * const Text, const int Width, const char * const Icon, const CMessageBox::result_ Default, const uint32_t ShowButtons) : CHintBoxExt(Caption, Text, Width, Icon)
{
	returnDefaultOnTimeout = false;

	m_height += (m_fheight << 1);

	result = Default;

	showbuttons = ShowButtons;

	int MaxButtonTextWidth = g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getRenderWidth(g_Locale->getText(LOCALE_MESSAGEBOX_CANCEL), true); // UTF-8
	int ButtonWidth = 20 + 33 + MaxButtonTextWidth + 5;
	int num = 0;
	
	if (showbuttons & mbYes)
		num++;
	
	if (showbuttons & mbNo)
		num++;
	
	if (showbuttons & (mbCancel | mbBack | mbOk))
		num++;
	
	int new_width = 15 + num*ButtonWidth;
	if(new_width > m_width)
		m_width = new_width;
}

CMessageBox::CMessageBox(const char* const Caption, ContentLines& Lines, const int Width, const char * const Icon, const CMessageBox::result_ Default, const uint32_t ShowButtons) : CHintBoxExt(Caption, Lines, Width, Icon)
{
	returnDefaultOnTimeout = false;

	m_height += (m_fheight << 1);

	result = Default;

	showbuttons = ShowButtons;
	int MaxButtonTextWidth = g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getRenderWidth(g_Locale->getText(LOCALE_MESSAGEBOX_CANCEL), true); // UTF-8
	int ButtonWidth = 20 + 33 + MaxButtonTextWidth + 5;
	int num = 0;
	
	if (showbuttons & mbYes)
		num++;
	
	if (showbuttons & mbNo)
		num++;
	
	if (showbuttons & (mbCancel | mbBack | mbOk))
		num++;
	
	int new_width = 15 + num*ButtonWidth;
	if(new_width > m_width)
		m_width = new_width;
}

void CMessageBox::returnDefaultValueOnTimeout(bool returnDefault)
{
	returnDefaultOnTimeout = returnDefault;
}

void CMessageBox::paintButtons()
{
	uint8_t    color;
	fb_pixel_t bgcolor;

	m_window->paintBoxRel(0, m_height - (m_fheight << 1), m_width, (m_fheight << 1), (CFBWindow::color_t)COL_MENUCONTENT_PLUS_0, RADIUS_MID, CORNER_BOTTOM);

	//irgendwann alle vergleichen - aber cancel ist sicher der l�ngste
	int MaxButtonTextWidth = g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getRenderWidth(g_Locale->getText(LOCALE_MESSAGEBOX_CANCEL), true); // UTF-8

	int ButtonWidth = 20 + 33 + MaxButtonTextWidth;

	int ButtonSpacing = (m_width - 20 - (ButtonWidth * 3)) / 2;
	if(ButtonSpacing <= 5) 
		ButtonSpacing = 5;

	int xpos = BORDER_LEFT;
	int iw, ih;
	int fh = g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight();
	const int noname = 20;
	
	// yes
	if (showbuttons & mbYes)
	{
		if (result == mbrYes)
		{
			color   = COL_MENUCONTENTSELECTED;
			bgcolor = COL_MENUCONTENTSELECTED_PLUS_0;
		}
		else
		{
			color   = COL_INFOBAR_SHADOW;
			bgcolor = COL_INFOBAR_SHADOW_PLUS_0;
		}
		

		m_window->paintBoxRel(xpos, m_height - m_fheight - noname, ButtonWidth, m_fheight, (CFBWindow::color_t)bgcolor);
		CFrameBuffer::getInstance()->getIconSize(NEUTRINO_ICON_BUTTON_RED, &iw, &ih);
		m_window->paintIcon(NEUTRINO_ICON_BUTTON_RED, xpos + 15, m_height - m_fheight - noname, m_fheight);
		m_window->RenderString(g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL], xpos + 43, (m_height - noname)-(m_fheight-fh)/2, ButtonWidth- 53, g_Locale->getText(LOCALE_MESSAGEBOX_YES), (CFBWindow::color_t)color, 0, true); // UTF-8
		
		xpos += ButtonWidth + ButtonSpacing;
	}

	// no
	if (showbuttons & mbNo)
	{
		if (result == mbrNo)
		{
			color   = COL_MENUCONTENTSELECTED;
			bgcolor = COL_MENUCONTENTSELECTED_PLUS_0;
		}
		else
		{
			color   = COL_INFOBAR_SHADOW;
			bgcolor = COL_INFOBAR_SHADOW_PLUS_0;
		}

		m_window->paintBoxRel(xpos, m_height - m_fheight-noname, ButtonWidth, m_fheight, (CFBWindow::color_t)bgcolor);
		m_window->paintIcon(NEUTRINO_ICON_BUTTON_GREEN, xpos + 14, m_height - m_fheight - noname, m_fheight);
		m_window->RenderString(g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL], xpos + 43, (m_height - noname)-(m_fheight-fh)/2, ButtonWidth- 53, g_Locale->getText(LOCALE_MESSAGEBOX_NO), (CFBWindow::color_t)color, 0, true); // UTF-8		
	
		xpos += ButtonWidth + ButtonSpacing;
	}


	// cancel|back|ok
	if (showbuttons & (mbCancel | mbBack | mbOk))
	{
		if (result >= mbrCancel)
		{
			color   = COL_MENUCONTENTSELECTED;
			bgcolor = COL_MENUCONTENTSELECTED_PLUS_0;
		}
		else
		{
			color   = COL_INFOBAR_SHADOW;
			bgcolor = COL_INFOBAR_SHADOW_PLUS_0;
		}

		m_window->paintBoxRel(xpos, m_height-m_fheight-noname, ButtonWidth, m_fheight, (CFBWindow::color_t)bgcolor);
		m_window->paintIcon(NEUTRINO_ICON_BUTTON_HOME, xpos + 14, m_height-m_fheight - noname, m_fheight);
		m_window->RenderString(g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL], 
					xpos + 43, 
					(m_height - noname) - (m_fheight - fh)/2, 
					ButtonWidth - 53, 
					g_Locale->getText((showbuttons & mbCancel) ? LOCALE_MESSAGEBOX_CANCEL : (showbuttons & mbOk) ? LOCALE_MESSAGEBOX_OK : LOCALE_MESSAGEBOX_BACK), 
					(CFBWindow::color_t)color, 0, true); // UTF-8	
	}	
}

int CMessageBox::exec(int timeout)
{
	neutrino_msg_t      msg;
	neutrino_msg_data_t data;

	int res = menu_return::RETURN_REPAINT;

	CHintBoxExt::paint();

	if (m_window == NULL)
	{
		return res; /* out of memory */
	}

	paintButtons();

	if ( timeout == -1 )
		timeout = g_settings.timing[SNeutrinoSettings::TIMING_EPG];

	unsigned long long timeoutEnd = CRCInput::calcTimeoutEnd( timeout );

	CFrameBuffer::getInstance()->blit();

	bool loop = true;
	while (loop)
	{
		g_RCInput->getMsgAbsoluteTimeout( &msg, &data, &timeoutEnd );
		if (msg == CRCInput::RC_timeout && returnDefaultOnTimeout)
		{
			// return default
			loop = false;
		}
		else if (((msg == CRCInput::RC_timeout) || (msg == CRCInput::RC_home)) && (showbuttons & (mbCancel | mbBack | mbOk)))
		{
			result = (showbuttons & mbCancel) ? mbrCancel : (showbuttons & mbOk) ? mbrOk: mbrBack;
			loop   = false;
		}
		else if ((msg == CRCInput::RC_green) && (showbuttons & mbNo))
		{
			result = mbrNo;
			loop   = false;
		}
		else if ((msg == CRCInput::RC_red) && (showbuttons & mbYes))
		{
			result = mbrYes;
			loop   = false;
		}
		else if(msg == CRCInput::RC_right)
		{
			bool ok = false;
			while (!ok)
			{
				result = (CMessageBox::result_)((result + 1) & 3);
				ok = showbuttons & (1 << result);
			}

			paintButtons();
		}
		else if (has_scrollbar() && ((msg == CRCInput::RC_up) || (msg == CRCInput::RC_down) || (msg == CRCInput::RC_page_up) || (msg == CRCInput::RC_page_down)))
		{
			if ( (msg == CRCInput::RC_up) || (msg == CRCInput::RC_page_up))
				scroll_up();
			else
				scroll_down();
			
			paintButtons();
		}
		else if(msg == CRCInput::RC_left)
		{
			bool ok = false;
			while (!ok)
			{
				result = (CMessageBox::result_)((result - 1) & 3);
				ok = showbuttons & (1 << result);
			}

			paintButtons();

		}
		else if(msg == CRCInput::RC_ok)
		{
			loop = false;
		}
		else if (CNeutrinoApp::getInstance()->handleMsg(msg, data) & messages_return::cancel_all)
		{
			res  = menu_return::RETURN_EXIT_ALL;
			loop = false;
		}

		CFrameBuffer::getInstance()->blit();
	}

	hide();

	return res;
}

//
int MessageBox(const neutrino_locale_t Caption, const char * const Text, const CMessageBox::result_ Default, const uint32_t ShowButtons, const char * const Icon, const int Width, const int timeout, bool returnDefaultOnTimeout)
{
   	CMessageBox * messageBox = new CMessageBox(Caption, Text, Width, Icon, Default, ShowButtons);
	messageBox->returnDefaultValueOnTimeout(returnDefaultOnTimeout);
	messageBox->exec(timeout);
	int res = messageBox->result;
	delete messageBox;

	return res;
}

int MessageBox(const neutrino_locale_t Caption, const neutrino_locale_t Text, const CMessageBox::result_ Default, const uint32_t ShowButtons, const char * const Icon, const int Width, const int timeout, bool returnDefaultOnTimeout)
{
	return MessageBox(Caption, g_Locale->getText(Text), Default, ShowButtons, Icon, Width, timeout,returnDefaultOnTimeout);
}

int MessageBox(const neutrino_locale_t Caption, const std::string & Text, const CMessageBox::result_ Default, const uint32_t ShowButtons, const char * const Icon, const int Width, const int timeout, bool returnDefaultOnTimeout)
{
	return MessageBox(Caption, Text.c_str(), Default, ShowButtons, Icon, Width, timeout,returnDefaultOnTimeout);
}

int MessageBox(const char * const Caption, const char * const Text, const CMessageBox::result_ Default, const uint32_t ShowButtons, const char * const Icon, const int Width, const int timeout, bool returnDefaultOnTimeout)
{
   	CMessageBox * messageBox = new CMessageBox(Caption, Text, Width, Icon, Default, ShowButtons);
	messageBox->returnDefaultValueOnTimeout(returnDefaultOnTimeout);
	messageBox->exec(timeout);
	int res = messageBox->result;
	delete messageBox;

	return res;
}

int MessageBox(const char * const Caption, const neutrino_locale_t Text, const CMessageBox::result_ Default, const uint32_t ShowButtons, const char * const Icon, const int Width, const int timeout, bool returnDefaultOnTimeout)
{
	return MessageBox(Caption, g_Locale->getText(Text), Default, ShowButtons, Icon, Width, timeout,returnDefaultOnTimeout);
}

int MessageBox(const char * const Caption, const std::string & Text, const CMessageBox::result_ Default, const uint32_t ShowButtons, const char * const Icon, const int Width, const int timeout, bool returnDefaultOnTimeout)
{
	return MessageBox(Caption, Text.c_str(), Default, ShowButtons, Icon, Width, timeout,returnDefaultOnTimeout);
}
