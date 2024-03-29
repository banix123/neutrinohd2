/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: msgbox.cpp 2013/10/12 mohousch Exp $

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

	***********************************************************

	Module Name: msgbox.cpp: .

	Description: Implementation of the CMsgBox class
				 This class provides a  message box using CTextBox.

  	Date:	Nov 2005

	Author: Günther@tuxbox.berlios.org
		based on code of Steffen Hehn 'McClean'

	Revision History:
	Date			Author		Change Description
	   Nov 2005		Gnther	initial implementation
	   
	   renamed to infobox.cpp
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <infobox.h>

#include <gui/widget/icons.h>
#include <neutrino.h>

#include <system/debug.h>

#define WINDOW_FRAME_BORDER_WIDTH	 	4
#define ADD_FOOT_HEIGHT	 			20
#define	TEXT_BORDER_WIDTH			8
#define	TITLE_ICON_WIDTH			(40 - TEXT_BORDER_WIDTH)

#define MAX_WINDOW_WIDTH  			(g_settings.screen_EndX - g_settings.screen_StartX )
#define MAX_WINDOW_HEIGHT 			(g_settings.screen_EndY - g_settings.screen_StartY - 40)	

#define MIN_WINDOW_WIDTH  			(MAX_WINDOW_WIDTH>>1)
#define MIN_WINDOW_HEIGHT 			40	

#define DEFAULT_TITLE_FONT			g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]
#define DEFAULT_FOOT_FONT			g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Function Name:	CInfoBox	
// Description:		
// Parameters:		
// Data IN/OUT:		
// Return:		
// Notes:		
//////////////////////////////////////////////////////////////////////
CInfoBox::CInfoBox(  const char * text, 
				   CFont *fontText,
				   const int _mode, 
				   const CBox* position, 
				   const char * title,
				   CFont *fontTitle,
				   const char * icon,
				   int return_button , 
				   const result_ default_result)
{
	initVar();

	if(title != NULL)		
		m_cTitle = title;
	
	if(fontTitle != NULL)	
		m_pcFontTitle = fontTitle;
	
	if(icon != NULL)		
		m_cIcon = icon;
	
	if(position != NULL)	
		m_cBoxFrame	= *position;
	
	m_nMode	= _mode;

	if(m_nMode & BORDER)
		m_nWindowFrameBorderWidth = WINDOW_FRAME_BORDER_WIDTH;
	else
		m_nWindowFrameBorderWidth = 0;


	/* Initialise the window frames first */
	initFramesRel();

	m_pcTextBox = new CTextBox(text, fontText, _mode, &m_cBoxFrameText);

	if(_mode & AUTO_WIDTH || _mode & AUTO_HIGH)
	{
		/* window might changed in size ...*/
		m_cBoxFrameText = m_pcTextBox->getWindowsPos();

		m_cBoxFrame.iWidth = m_cBoxFrameText.iWidth + m_nWindowFrameBorderWidth;
		m_cBoxFrame.iHeight = m_cBoxFrameText.iHeight + m_cBoxFrameFootRel.iHeight +  m_cBoxFrameTitleRel.iHeight + m_nWindowFrameBorderWidth;

		initFramesRel();
	}

	if(_mode & CENTER)
	{
		m_cBoxFrame.iX = g_settings.screen_StartX + ((g_settings.screen_EndX - g_settings.screen_StartX - m_cBoxFrame.iWidth) >>1);
		m_cBoxFrame.iY = g_settings.screen_StartY + ((g_settings.screen_EndY - g_settings.screen_StartY - m_cBoxFrame.iHeight) >>2);
	}

	m_nResult = default_result;
	m_nFootButtons = return_button;
}

//////////////////////////////////////////////////////////////////////
// Function Name:	CInfoBox	
// Description:		
// Parameters:		
// Data IN/OUT:		
// Return:		
// Notes:		
//////////////////////////////////////////////////////////////////////
CInfoBox::CInfoBox(const char * text)
{
	initVar();

	m_pcTextBox = new CTextBox(text);
	
	/* Initialise the window frames first */
	initFramesRel();
}

//////////////////////////////////////////////////////////////////////
// Function Name:	CInfoBox	
// Description:		
// Parameters:		
// Data IN/OUT:		
// Return:		
// Notes:		
//////////////////////////////////////////////////////////////////////
CInfoBox::CInfoBox()
{
	m_pcTextBox = NULL;

	initVar();
	initFramesRel();
}

//////////////////////////////////////////////////////////////////////
// Function Name:	~CInfoBox	
// Description:		
// Parameters:		
// Data IN/OUT:		
// Return:		
// Notes:		
//////////////////////////////////////////////////////////////////////
CInfoBox::~CInfoBox()
{
	if (m_pcWindow != NULL)
	{
		delete m_pcWindow;
		m_pcWindow = NULL;
	}

	if (m_pcTextBox != NULL)
	{
		delete m_pcTextBox;
		m_pcTextBox = NULL;
	}
}

//////////////////////////////////////////////////////////////////////
// Function Name:	InitVar	
// Description:		
// Parameters:		
// Data IN/OUT:		
// Return:		
// Notes:		
//////////////////////////////////////////////////////////////////////
void CInfoBox::initVar(void)
{
	m_cTitle = "";
	m_cIcon = "";
	m_nMode = SCROLL | TITLE | BORDER ;

	// set the title varianles
	m_pcFontTitle  =  DEFAULT_TITLE_FONT;
	m_nFontTitleHeight = m_pcFontTitle->getHeight();

	// set the foot variables
	m_pcFontFoot  =  DEFAULT_FOOT_FONT;
	m_nFontFootHeight = m_pcFontFoot->getHeight();
	m_nFootButtons = 0;

	// set the main frame border width
	if(m_nMode & BORDER)
		m_nWindowFrameBorderWidth = WINDOW_FRAME_BORDER_WIDTH;
	else
		m_nWindowFrameBorderWidth = 0;

	// set the main frame to default
	m_cBoxFrame.iX		= g_settings.screen_StartX + ((g_settings.screen_EndX - g_settings.screen_StartX - MIN_WINDOW_WIDTH) >>1);
	m_cBoxFrame.iWidth	= MIN_WINDOW_WIDTH;
	m_cBoxFrame.iY		= g_settings.screen_StartY + ((g_settings.screen_EndY - g_settings.screen_StartY - MIN_WINDOW_HEIGHT) >>2);
	m_cBoxFrame.iHeight	= MIN_WINDOW_HEIGHT;

	m_pcWindow = NULL;
}

//////////////////////////////////////////////////////////////////////
// Function Name:	InitFramesRel	
// Description:		
// Parameters:		
// Data IN/OUT:		
// Return:		
// Notes:		
//////////////////////////////////////////////////////////////////////
void CInfoBox::initFramesRel(void)
{
	// init the title frame
	if(m_nMode & TITLE)
	{
		m_cBoxFrameTitleRel.iX		= 0;
		m_cBoxFrameTitleRel.iY		= 0;
		m_cBoxFrameTitleRel.iWidth	= m_cBoxFrame.iWidth - m_nWindowFrameBorderWidth;
		m_cBoxFrameTitleRel.iHeight	= m_nFontTitleHeight + 2;
	}
	else
	{
		m_cBoxFrameTitleRel.iX		= 0;
		m_cBoxFrameTitleRel.iY		= 0;
		m_cBoxFrameTitleRel.iHeight	= 0;
		m_cBoxFrameTitleRel.iWidth	= 0;
	}

	// init the foot frame
	if(m_nMode & FOOT)
	{
		m_cBoxFrameFootRel.iX		= 0;
		m_cBoxFrameFootRel.iY		= m_cBoxFrame.iHeight - m_nFontFootHeight - m_nWindowFrameBorderWidth - ADD_FOOT_HEIGHT;
		m_cBoxFrameFootRel.iWidth	= m_cBoxFrame.iWidth - m_nWindowFrameBorderWidth;
		m_cBoxFrameFootRel.iHeight	= m_nFontFootHeight + ADD_FOOT_HEIGHT;
	}
	else
	{
		m_cBoxFrameFootRel.iX		= 0;
		m_cBoxFrameFootRel.iY		= 0;
		m_cBoxFrameFootRel.iHeight   = 0;
		m_cBoxFrameFootRel.iWidth    = 0;
	}

	// init the text frame
	m_cBoxFrameText.iY		= m_cBoxFrame.iY + m_cBoxFrameTitleRel.iY + m_cBoxFrameTitleRel.iHeight;
	m_cBoxFrameText.iX		= m_cBoxFrame.iX + m_cBoxFrameTitleRel.iX;
	m_cBoxFrameText.iHeight	= m_cBoxFrame.iHeight - m_cBoxFrameTitleRel.iHeight - m_cBoxFrameFootRel.iHeight - m_nWindowFrameBorderWidth;
	m_cBoxFrameText.iWidth	= m_cBoxFrame.iWidth  - m_nWindowFrameBorderWidth;		
}

//////////////////////////////////////////////////////////////////////
// Function Name:	RefreshFoot	
// Description:		
// Parameters:		
// Data IN/OUT:		
// Return:		
// Notes:		
//////////////////////////////////////////////////////////////////////
void CInfoBox::refreshFoot(void)
{
	uint8_t color;
	fb_pixel_t bgcolor;
	
	if(!(m_nMode & FOOT)) 
		return;

	m_pcWindow->paintBoxRel(m_cBoxFrameFootRel.iX + m_cBoxFrame.iX, 
								m_cBoxFrameFootRel.iY + m_cBoxFrame.iY, 
								m_cBoxFrameFootRel.iWidth, 
								m_cBoxFrameFootRel.iHeight,  
								COL_MENUHEAD_PLUS_0,
								RADIUS_MID, CORNER_BOTTOM);

	int iw, ih;
	m_pcWindow->getIconSize(NEUTRINO_ICON_BUTTON_HOME, &iw, &ih);
	int MaxButtonTextWidth = m_pcFontFoot->getRenderWidth(g_Locale->getText(LOCALE_MESSAGEBOX_CANCEL), true); // UTF-8
	int ButtonWidth = BORDER_LEFT + BORDER_RIGHT + iw + MaxButtonTextWidth;
	int ButtonSpacing = (m_cBoxFrameFootRel.iWidth - BORDER_LEFT - BORDER_RIGHT - (ButtonWidth*3) ) / 2;
	int xpos = m_cBoxFrameFootRel.iX;

	// draw Button mbYes
	if (m_nFootButtons & mbYes)
	{
		if (m_nResult == mbrYes)
		{
			color   = COL_MENUCONTENTSELECTED;
			bgcolor = COL_MENUCONTENTSELECTED_PLUS_0;
		}
		else
		{
			color   = COL_INFOBAR_SHADOW;
			bgcolor = COL_INFOBAR_SHADOW_PLUS_0;
		}
		
		m_pcWindow->paintBoxRel(xpos + m_cBoxFrame.iX, 
						m_cBoxFrame.iY + m_cBoxFrameFootRel.iY + (m_cBoxFrameFootRel.iHeight - (m_nFontFootHeight + 4))/2, 
						ButtonWidth, 
						m_nFontFootHeight + 4, 
						bgcolor);
		
		m_pcWindow->getIconSize(NEUTRINO_ICON_BUTTON_RED, &iw, &ih);
		m_pcWindow->paintIcon(NEUTRINO_ICON_BUTTON_RED, 
						xpos + BORDER_LEFT + 4 + m_cBoxFrame.iX, 
						m_cBoxFrame.iY + m_cBoxFrameFootRel.iY + (m_cBoxFrameFootRel.iHeight - ih)/2);
						
		m_pcFontFoot->RenderString(xpos + BORDER_LEFT + iw + m_cBoxFrame.iX, 
						m_cBoxFrame.iY + m_cBoxFrameFootRel.iY + m_nFontFootHeight + (m_cBoxFrameFootRel.iHeight - m_nFontFootHeight)/2, 
						ButtonWidth - ( BORDER_LEFT + BORDER_RIGHT + iw), 
						g_Locale->getText(LOCALE_MESSAGEBOX_YES), 
						color, 
						0, true); // UTF-8
	}

	xpos += ButtonWidth + ButtonSpacing;

	// draw Button mbNo
	if (m_nFootButtons & mbNo)
	{
		if (m_nResult == mbrNo)
		{
			color   = COL_MENUCONTENTSELECTED;
			bgcolor = COL_MENUCONTENTSELECTED_PLUS_0;
		}
		else
		{
			color   = COL_INFOBAR_SHADOW;
			bgcolor = COL_INFOBAR_SHADOW_PLUS_0;
		}

		m_pcWindow->paintBoxRel(xpos + m_cBoxFrame.iX, 
						m_cBoxFrame.iY + m_cBoxFrameFootRel.iY + (m_cBoxFrameFootRel.iHeight - (m_nFontFootHeight + 4))/2, 
						ButtonWidth, 
						m_nFontFootHeight + 4, 
						bgcolor);
		
		m_pcWindow->getIconSize(NEUTRINO_ICON_BUTTON_GREEN, &iw, &ih);
		m_pcWindow->paintIcon(NEUTRINO_ICON_BUTTON_GREEN, 
						xpos + BORDER_LEFT + 4 + m_cBoxFrame.iX, 
						m_cBoxFrame.iY + m_cBoxFrameFootRel.iY + (m_cBoxFrameFootRel.iHeight - ih)/2);
						
		m_pcFontFoot->RenderString(xpos + ( BORDER_LEFT + iw) + m_cBoxFrame.iX, 
						m_cBoxFrame.iY + m_cBoxFrameFootRel.iY + m_nFontFootHeight + (m_cBoxFrameFootRel.iHeight - m_nFontFootHeight)/2, 
						ButtonWidth - ( BORDER_LEFT + BORDER_RIGHT + iw), 
						g_Locale->getText(LOCALE_MESSAGEBOX_NO), 
						color, 
						0, true); // UTF-8
	}

	xpos += ButtonWidth + ButtonSpacing;

	// draw Button mbCancel
	if (m_nFootButtons & (mbCancel | mbBack))
	{
		if (m_nResult >= mbrCancel)
		{
			color   = COL_MENUCONTENTSELECTED;
			bgcolor = COL_MENUCONTENTSELECTED_PLUS_0;
		}
		else
		{
			color   = COL_INFOBAR_SHADOW;
			bgcolor = COL_INFOBAR_SHADOW_PLUS_0;
		}

		m_pcWindow->paintBoxRel(xpos + m_cBoxFrame.iX, 
						m_cBoxFrame.iY + m_cBoxFrameFootRel.iY + (m_cBoxFrameFootRel.iHeight - (m_nFontFootHeight + 4))/2, 
						ButtonWidth, 
						m_nFontFootHeight + 4, 
						bgcolor);
		
		m_pcWindow->getIconSize(NEUTRINO_ICON_BUTTON_HOME, &iw, &ih);
		m_pcWindow->paintIcon(NEUTRINO_ICON_BUTTON_HOME, 
						xpos + BORDER_LEFT + m_cBoxFrame.iX, 
						m_cBoxFrame.iY + m_cBoxFrameFootRel.iY + (m_cBoxFrameFootRel.iHeight - ih)/2);
						
		m_pcFontFoot->RenderString(xpos + ( BORDER_LEFT + iw) + m_cBoxFrame.iX, 
						m_cBoxFrame.iY + m_cBoxFrameFootRel.iY + m_nFontFootHeight + (m_cBoxFrameFootRel.iHeight - m_nFontFootHeight)/2, 
						ButtonWidth - ( BORDER_LEFT + BORDER_RIGHT + iw), 
						g_Locale->getText((m_nFootButtons & mbCancel) ? LOCALE_MESSAGEBOX_CANCEL : LOCALE_MESSAGEBOX_BACK), 
						color, 
						0, true); // UTF-8
	}
}

//////////////////////////////////////////////////////////////////////
// Function Name:	RefreshTitle	
// Description:		
// Parameters:		
// Data IN/OUT:		
// Return:		
// Notes:		
//////////////////////////////////////////////////////////////////////
void CInfoBox::refreshTitle(void)
{
	// first check if title is configured
	if(!(m_nMode & TITLE)) 
		return;

	m_pcWindow->paintBoxRel(m_cBoxFrame.iX + m_cBoxFrameTitleRel.iX, 
					m_cBoxFrame.iY + m_cBoxFrameTitleRel.iY, 
					m_cBoxFrameTitleRel.iWidth, 
					m_cBoxFrameTitleRel.iHeight, 
					COL_MENUHEAD_PLUS_0, 
					RADIUS_MID, CORNER_TOP);

	int iw = 0;
	int ih = 0;
	if (!m_cIcon.empty())
	{
		m_pcWindow->getIconSize(m_cIcon.c_str(), &iw, &ih);
	
		m_pcWindow->paintIcon(m_cIcon.c_str(), m_cBoxFrame.iX + m_cBoxFrameTitleRel.iX + BORDER_LEFT, m_cBoxFrame.iY + m_cBoxFrameTitleRel.iY + (m_cBoxFrameTitleRel.iHeight - ih)/2);
	}
	m_pcFontTitle->RenderString(m_cBoxFrame.iX + m_cBoxFrameTitleRel.iX + BORDER_LEFT + iw + TEXT_BORDER_WIDTH, m_cBoxFrame.iY + m_cBoxFrameTitleRel.iHeight + (m_cBoxFrameTitleRel.iHeight - m_pcFontTitle->getHeight())/2, m_cBoxFrameTitleRel.iWidth - (BORDER_LEFT + BORDER_RIGHT + iw + TEXT_BORDER_WIDTH), m_cTitle.c_str(), COL_MENUHEAD, 0, true); // UTF-8
}

//////////////////////////////////////////////////////////////////////
// Function Name:	RefreshBorder	
// Description:		
// Parameters:		
// Data IN/OUT:		
// Return:		
// Notes:		
//////////////////////////////////////////////////////////////////////
void CInfoBox::refreshBorder(void)
{
	if(!(m_nMode & BORDER && m_nWindowFrameBorderWidth > 0))
		return;

	/*
	//draw bottom shadow
	m_pcWindow->paintBoxRel(m_nWindowFrameBorderWidth + m_cBoxFrame.iX, 
					m_cBoxFrame.iHeight - m_nWindowFrameBorderWidth + m_cBoxFrame.iY, 
					m_cBoxFrame.iWidth - m_nWindowFrameBorderWidth, 
					m_nWindowFrameBorderWidth, 
					COL_INFOBAR_SHADOW_PLUS_0, 
					RADIUS_MID, CORNER_BOTTOM);

	//draw top shadow
	m_pcWindow->paintBoxRel(m_cBoxFrame.iWidth - m_nWindowFrameBorderWidth+m_cBoxFrame.iX, 
					m_nWindowFrameBorderWidth + m_cBoxFrame.iY, 
					m_nWindowFrameBorderWidth, 
					m_cBoxFrame.iHeight - m_nWindowFrameBorderWidth, 
					COL_INFOBAR_SHADOW_PLUS_0,
					RADIUS_MID, CORNER_TOP);
	*/
}

//////////////////////////////////////////////////////////////////////
// global Functions
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// Function Name:	Hide	
// Description:		
// Parameters:		
// Data IN/OUT:		
// Return:		
// Notes:		
//////////////////////////////////////////////////////////////////////
bool CInfoBox::hide(void)
{
	if (m_pcWindow == NULL)
	{
		return (false);
	}
	
	if(m_pcTextBox != NULL)
	{
		m_pcTextBox->hide();
	}

	m_pcWindow->paintBackgroundBoxRel(m_cBoxFrame.iX, m_cBoxFrame.iY, m_cBoxFrame.iWidth, m_cBoxFrame.iHeight);
	
	m_pcWindow->blit();

	m_pcWindow = NULL;
	
	return (true);
}

//////////////////////////////////////////////////////////////////////
// Function Name:	ScrollPageDown	
// Description:		
// Parameters:		
// Data IN/OUT:		
// Return:		
// Notes:		
//////////////////////////////////////////////////////////////////////
void CInfoBox::scrollPageDown(const int pages)
{
	// send scroll up event to text box if there is one
	if(m_pcTextBox != NULL)
	{
		m_pcTextBox->scrollPageDown(pages);
	}

};

//////////////////////////////////////////////////////////////////////
// Function Name:	ScrollPageUp	
// Description:		
// Parameters:		
// Data IN/OUT:		
// Return:		
// Notes:		
//////////////////////////////////////////////////////////////////////
void CInfoBox::scrollPageUp(const int pages)
{
	// send scroll up event to text box if there is one
	if(m_pcTextBox != NULL)
	{
		m_pcTextBox->scrollPageUp(pages);
	}
};

//////////////////////////////////////////////////////////////////////
// Function Name:	Paint	
// Description:		
// Parameters:		
// Data IN/OUT:		
// Return:		
// Notes:		
//////////////////////////////////////////////////////////////////////
bool CInfoBox::paint(void)
{
	if (m_pcWindow != NULL)
	{
		return (false);
	}

	// create new window
	m_pcWindow = CFrameBuffer::getInstance();
	
	if(m_pcTextBox != NULL)
	{
		m_pcTextBox->paint();
	}
	
	refresh();
	
	return (true);
}

//////////////////////////////////////////////////////////////////////
// Function Name:	Refresh	
// Description:		
// Parameters:		
// Data IN/OUT:		
// Return:		
// Notes:		
//////////////////////////////////////////////////////////////////////
void CInfoBox::refresh(void)
{
	if (m_pcWindow == NULL)
	{
		return;
	}
	
	//re-draw message box window
	refreshTitle();
	refreshFoot();
	refreshBorder();

	// rep-draw textbox if there is one
	if(m_pcTextBox != NULL)
	{
		//m_pcTextBox->refresh();
	}
}

//////////////////////////////////////////////////////////////////////
// Function Name:	Exec	
// Description:		
// Parameters:		
// Data IN/OUT:		
// Return:		
// Notes:		
//////////////////////////////////////////////////////////////////////
int CInfoBox::exec( int timeout, int returnDefaultOnTimeout)
{
	neutrino_msg_t      msg;
	neutrino_msg_data_t data;
	int return_button = m_nFootButtons;
	int res = menu_return::RETURN_REPAINT;

	// show infobox
	paint();
	m_pcWindow->blit();
	
	if (m_pcWindow == NULL)
		return res; /* out of memory */

	if ( timeout == -1 )
		timeout = g_settings.timing[SNeutrinoSettings::TIMING_EPG];

	unsigned long long timeoutEnd = CRCInput::calcTimeoutEnd( timeout );

	bool loop = true;
	while (loop)
	{
		g_RCInput->getMsgAbsoluteTimeout( &msg, &data, &timeoutEnd );

		if (msg == CRCInput::RC_timeout && returnDefaultOnTimeout)
		{
			// return default 
			loop = false;
		}
		else if (((msg == CRCInput::RC_timeout) || (msg == CRCInput::RC_home)) && (return_button & (mbCancel | mbBack)))
		{
			m_nResult = (return_button & mbCancel) ? mbrCancel : mbrBack;
			loop = false;
		}
		else if ((msg == CRCInput::RC_green) && (return_button & mbNo))
		{
			m_nResult = mbrNo;
			loop = false;
		}
		else if ((msg == CRCInput::RC_red) && (return_button & mbYes))
		{
			m_nResult = mbrYes;
			loop = false;
		}
		else if(msg == CRCInput::RC_right)
		{
			bool ok = false;
			while (!ok)
			{
				m_nResult = (CInfoBox::result_)((m_nResult + 1) & 3);
				ok = m_nFootButtons & (1 << m_nResult);
			}

			refreshFoot();
		}
		else if (msg == CRCInput::RC_up || msg == CRCInput::RC_page_up)
		{
			scrollPageUp(1);
		}
		else if (msg == CRCInput::RC_down || msg == CRCInput::RC_page_down)
		{
			scrollPageDown(1);
		}
		else if(msg == CRCInput::RC_left)
		{
			bool ok = false;
			while (!ok)
			{
				m_nResult = (CInfoBox::result_)((m_nResult - 1) & 3);
				ok = return_button & (1 << m_nResult);
			}

			refreshFoot();
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

		m_pcWindow->blit();
	}

	hide();
	
	return res;
}

//////////////////////////////////////////////////////////////////////
// Function Name:	SetText	
// Description:		
// Parameters:		
// Data IN/OUT:		
// Return:		
// Notes:		
//////////////////////////////////////////////////////////////////////
bool CInfoBox::setText(const std::string* newText, std::string _thumbnail, int _lx, int _ly, int _tw, int _th)
{
	bool _result = false;
	
	// update text in textbox if there is one
	if(m_pcTextBox != NULL && newText != NULL)
	{
		_result = m_pcTextBox->setText(newText, _thumbnail, _lx, _ly, _tw, _th );
		
		if(m_nMode & AUTO_WIDTH || m_nMode & AUTO_HIGH)
		{
			/* window might changed in size ...*/
			m_cBoxFrameText = m_pcTextBox->getWindowsPos();

			m_cBoxFrame.iWidth = m_cBoxFrameText.iWidth + m_nWindowFrameBorderWidth;
			m_cBoxFrame.iHeight = m_cBoxFrameText.iHeight + m_cBoxFrameFootRel.iHeight +  m_cBoxFrameTitleRel.iHeight + m_nWindowFrameBorderWidth;

			initFramesRel();

			// since the frames size has changed, we have to recenter the window again */
			if(m_nMode & CENTER)
			{
				m_cBoxFrame.iX = g_settings.screen_StartX + ((g_settings.screen_EndX - g_settings.screen_StartX - m_cBoxFrame.iWidth) >>1);
				m_cBoxFrame.iY = g_settings.screen_StartY + ((g_settings.screen_EndY - g_settings.screen_StartY - m_cBoxFrame.iHeight) >>1);
			}
		}
	}
	
	return(_result);
};

//////////////////////////////////////////////////////////////////////
// Function Name:	SetText	
// Description:		
// Parameters:		
// Data IN/OUT:		
// Return:		
// Notes:		
//////////////////////////////////////////////////////////////////////
int CInfoBox::result(void)
{
	return m_nResult;
}

//////////////////////////////////////////////////////////////////////
// Function Name:	ShowInfoBox	
// Description:		
// Parameters:		
// Data IN/OUT:		
// Return:		
// Notes:		
//////////////////////////////////////////////////////////////////////
int InfoBox(const neutrino_locale_t Caption, const char * const Text, const CInfoBox::result_ Default, const uint32_t ShowButtons, const char * const Icon, const int Width, const int timeout, bool returnDefaultOnTimeout)
{
	int result = InfoBox(g_Locale->getText(Caption),Text, Default, ShowButtons, Icon, Width, timeout, returnDefaultOnTimeout);

	return (result);

}

//////////////////////////////////////////////////////////////////////
// Function Name:	ShowInfoBox	
// Description:		
// Parameters:		
// Data IN/OUT:		
// Return:		
// Notes:		
//////////////////////////////////////////////////////////////////////
int InfoBox(const char * const Title,const char * const Text, const CInfoBox::result_ Default, const uint32_t ShowButtons, const char * const Icon, const int /*Width*/, const int timeout, bool returnDefaultOnTimeout)
{
	int mode =  CInfoBox::SCROLL | CInfoBox::TITLE | CInfoBox::FOOT | CInfoBox::BORDER;// | //CInfoBox::NO_AUTO_LINEBREAK | //CInfoBox::CENTER | //CInfoBox::AUTO_WIDTH | //CInfoBox::AUTO_HIGH;
	
	CBox position(g_settings.screen_StartX + 30, g_settings.screen_StartY + 30, g_settings.screen_EndX - g_settings.screen_StartX - 60, g_settings.screen_EndY - g_settings.screen_StartY - 60); 
	
   	CInfoBox * infoBox = new CInfoBox(Text, g_Font[SNeutrinoSettings::FONT_TYPE_MENU], mode, &position, Title, g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE], Icon, ShowButtons, Default);

	infoBox->exec( timeout, returnDefaultOnTimeout);

	int res = infoBox->result();

	delete infoBox;
	
	return res;
}
