/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: stringinput.cpp 2013/10/12 mohousch Exp $

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

#include <gui/widget/stringinput.h>

#include <driver/fontrenderer.h>
#include <driver/rcinput.h>
#include <driver/screen_max.h>

#include <gui/color.h>

#include <gui/widget/buttons.h>
#include <gui/widget/icons.h>
#include <gui/widget/messagebox.h>

#include <global.h>
#include <neutrino.h>


#define borderwidth 4


CStringInput::CStringInput(const neutrino_locale_t Name, char *Value, int Size, const neutrino_locale_t Hint_1, const neutrino_locale_t Hint_2, const char * const Valid_Chars, CChangeObserver *Observ, const char * const Icon)
{
	frameBuffer = CFrameBuffer::getInstance();
	name =  Name;
	head = NULL;
	value = Value;
	valueString = NULL;
	size =  Size;

	hint_1 = Hint_1;
	hint_2 = Hint_2;
	validchars = Valid_Chars;
	iconfile = Icon ? Icon : "";

	observ = Observ;
	init();
}

CStringInput::CStringInput(const neutrino_locale_t Name, std::string* Value, int Size, const neutrino_locale_t Hint_1, const neutrino_locale_t Hint_2, const char * const Valid_Chars, CChangeObserver* Observ, const char * const Icon)
{
        frameBuffer = CFrameBuffer::getInstance();
        name =  Name;
	head = NULL;
        value = new char[Size+1];
        value[Size] = '\0';
        strncpy(value, Value->c_str(), Size);
        valueString = Value;
        size = Size;

        hint_1 = Hint_1;
        hint_2 = Hint_2;
        validchars = Valid_Chars;
        iconfile = Icon ? Icon : "";

        observ = Observ;
        init();
}

CStringInput::CStringInput(char * Head, char* Value, int Size, const neutrino_locale_t Hint_1, const neutrino_locale_t Hint_2, const char * const Valid_Chars, CChangeObserver* Observ, const char * const Icon)
{
        frameBuffer = CFrameBuffer::getInstance();
        head = strdup(Head);
        value = Value;
        valueString = NULL;
        size =  Size;

        hint_1 = Hint_1;
        hint_2 = Hint_2;
        validchars = Valid_Chars;
        iconfile = Icon ? Icon : "";

        observ = Observ;
        init();
}

CStringInput::~CStringInput() 
{
	if (valueString != NULL) 
	{
		delete[] value;
	}
	
	if(head) 
	{
		free(head);
	}
}

void CStringInput::init() 
{
	width = (size*20) + 40;

	if (width < 420)
		width = 420;

	int neededWidth;
	if(head) 
	{
		neededWidth = g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getRenderWidth(head); // UTF-8
	}
	else
		neededWidth = g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getRenderWidth(g_Locale->getText(name)); // UTF-8

	if (!(iconfile.empty()))
		neededWidth += 28;
	if (neededWidth + 20 > width)
		width = neededWidth + 20;

	hheight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight();
	
	if ( !(iconfile.empty()))
	{
		int icol_w = 28;
		int icol_h = 16;
		frameBuffer->getIconSize(iconfile.c_str(), &icol_w, &icol_h);
		hheight = std::max(hheight, icol_h + 4);
	}
	
	mheight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight();
	iheight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU_INFO]->getHeight();

	height = hheight + mheight + 50 + 2*iheight;

	// coordinate
	x = frameBuffer->getScreenX() + ((frameBuffer->getScreenWidth() - width) >> 1 );
	y = frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - height) >> 1 );
	
	smstimer = 0;
	
	selected = 0;
}

void CStringInput::NormalKeyPressed(const neutrino_msg_t key)
{
	if (CRCInput::isNumeric(key))
	{
		value[selected] = validchars[CRCInput::getNumericValue(key)];

		if (selected < (size - 1))
		{
			selected++;
			paintChar(selected - 1);
		}
		
		paintChar(selected);
	}
}

void CStringInput::keyRedPressed()
{
	if (index(validchars, ' ') != NULL)
	{
		value[selected] = ' ';

		if (selected < (size - 1))
		{
			selected++;
			paintChar(selected - 1);
		}
  
		paintChar(selected);
	}
}

void CStringInput::keyYellowPressed()
{
	selected = 0;
	for(int i = 0 ; i < size ; i++)
	{
		value[i] = ' ';
		paintChar(i);
	}
}

void CStringInput::keyBluePressed()
{
	if (((value[selected] | 32) >= 'a') && ((value[selected] | 32) <= 'z'))
	{
		char newValue = value[selected] ^ 32;
		if (index(validchars, newValue) != NULL) 
		{
			value[selected] = newValue;
			paintChar(selected);
		}
	}
}

void CStringInput::keyUpPressed()
{
	int npos = 0;
	for(int count = 0; count < (int)strlen(validchars);count++)
		if(value[selected]==validchars[count])
			npos = count;
	npos++;
	if(npos>=(int)strlen(validchars))
		npos = 0;
	value[selected]=validchars[npos];
	paintChar(selected);
}

void CStringInput::keyDownPressed()
{
	int npos = 0;
	for(int count=0;count<(int)strlen(validchars);count++)
		if(value[selected]==validchars[count])
			npos = count;
	npos--;
	if(npos<0)
		npos = strlen(validchars)-1;
	value[selected]=validchars[npos];
	paintChar(selected);
}

void CStringInput::keyLeftPressed()
{
	int old = selected;
	if(selected>0) {
		selected--;
	} else {
		selected = size - 1;
	}
	paintChar(old);
	paintChar(selected);
}

void CStringInput::keyRightPressed()
{
	int old = selected;
	if (selected < (size - 1)) 
	{
		selected++;
	} else
		selected = 0;
	paintChar(old);
	paintChar(selected);
}

void CStringInput::keyMinusPressed()
{
	int item = selected;
	while (item < (size -1))
	{
		value[item] = value[item+1];
		paintChar(item);
		item++;
	}
	value[item] = ' ';
	paintChar(item);
}

void CStringInput::keyPlusPressed()
{
	int item = size -1;
	while (item > selected)
	{
		value[item] = value[item-1];
		paintChar(item);
		item--;
	}
	value[item] = ' ';
	paintChar(item);
}

int CStringInput::exec( CMenuTarget* parent, const std::string & )
{
	neutrino_msg_t      msg;
	neutrino_msg_data_t data;
	int res = menu_return::RETURN_REPAINT;

        char oldval[size+1], dispval[size+1];
        oldval[size] = 0;
        dispval[size] = 0;

	if (parent)
		parent->hide();

	for(int count = strlen(value)-1;count<size-1;count++)
		strcat(value, " ");
	
	strncpy(oldval, value, size);

	paint();
	
	frameBuffer->blit();

	unsigned long long timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_MENU] == 0 ? 0xFFFF : g_settings.timing[SNeutrinoSettings::TIMING_MENU]);

	bool loop = true;
	while (loop)
	{
		if ( strncmp(value, dispval, size) != 0)
		{
			CVFD::getInstance()->showMenuText(1, value, selected+1);

			strncpy(dispval, value, size);
		}

		g_RCInput->getMsgAbsoluteTimeout(&msg, &data, &timeoutEnd, true );

		if ( msg <= CRCInput::RC_MaxRC )
			timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_MENU] == 0 ? 0xFFFF : g_settings.timing[SNeutrinoSettings::TIMING_MENU]);
		
		//
		if (!(msg & CRCInput::RC_Release))
			g_RCInput->killTimer(smstimer);

		if (msg == CRCInput::RC_left)
		{
			keyLeftPressed();
		}
		else if ((msg == CRCInput::RC_right) || (msg == NeutrinoMessages::EVT_TIMER && data == smstimer))
		{
			keyRightPressed();
		}
		else if (CRCInput::getUnicodeValue(msg) != -1)
		{
			NormalKeyPressed(msg);
		}
		else if (msg == CRCInput::RC_red)
		{
			keyRedPressed();
		}
		else if (msg == CRCInput::RC_yellow)
		{
			keyYellowPressed();
		}
		else if ( (msg == CRCInput::RC_green) && (index(validchars, '.') != NULL))
		{
			value[selected] = '.';

			if (selected < (size - 1))
			{
				selected++;
				paintChar(selected - 1);
			}
  
			paintChar(selected);
		}
		else if (msg == CRCInput::RC_blue)
		{
			keyBluePressed();
		}
		else if (msg == CRCInput::RC_up)
		{
			keyUpPressed();
		}
		else if (msg == CRCInput::RC_down)
		{
			keyDownPressed();
		} else if (msg == CRCInput::RC_plus)
		{
			keyPlusPressed();
		} else if (msg == CRCInput::RC_minus)
		{
			keyMinusPressed();
		}
		else if (msg == CRCInput::RC_ok)
		{
			loop = false;
		}
		else if ( (msg == CRCInput::RC_home) || (msg == CRCInput::RC_timeout) )
		{
			if ( ( strcmp(value, oldval) != 0) && (MessageBox(name, LOCALE_MESSAGEBOX_DISCARD, CMessageBox::mbrYes, CMessageBox::mbYes | CMessageBox::mbCancel) == CMessageBox::mbrCancel))
				continue;

			strncpy(value, oldval, size);
			loop = false;
		}
		else
		{
			int r = handleOthers( msg, data );
			if (r & (messages_return::cancel_all | messages_return::cancel_info))
			{
				res = (r & messages_return::cancel_all) ? menu_return::RETURN_EXIT_ALL : menu_return::RETURN_EXIT;
				loop = false;
			}
			else if ( r & messages_return::unhandled )
			{
				if ( CNeutrinoApp::getInstance()->handleMsg( msg, data ) & messages_return::cancel_all )
				{
					loop = false;
					res = menu_return::RETURN_EXIT_ALL;
				}
			}
		}

		frameBuffer->blit();	
	}

	hide();

	for(int count = size - 1; count >= 0; count--)
	{
		if((value[count] == ' ') || (value[count] == 0))
		{
			value[count] = 0;
		}
		else
			break;
	}
	value[size] = 0;

	if (valueString != NULL)
        {
                *valueString = value;
        }

        if ( (observ) && (msg == CRCInput::RC_ok) )
        {
                observ->changeNotify(name, value);
        }

	return res;
}

int CStringInput::handleOthers(const neutrino_msg_t /*msg*/, const neutrino_msg_data_t /*data*/)
{
	return messages_return::unhandled;
}

void CStringInput::hide()
{
	frameBuffer->paintBackgroundBoxRel(x, y, width + SHADOW_OFFSET, height + SHADOW_OFFSET);
	frameBuffer->blit();
}

const char * CStringInput::getHint1(void)
{
	return g_Locale->getText(hint_1);
}

void CStringInput::paint()
{
	int iconoffset;
	int icol_w = 28, icol_h = 16;

	//head
	frameBuffer->paintBoxRel(x + SHADOW_OFFSET, y + SHADOW_OFFSET, width, hheight, COL_INFOBAR_SHADOW_PLUS_0, RADIUS_MID, CORNER_TOP); //round
	frameBuffer->paintBoxRel(x, y, width, hheight, COL_MENUHEAD_PLUS_0, RADIUS_MID, CORNER_TOP); //round
	
	// foot
	frameBuffer->paintBoxRel(x + SHADOW_OFFSET, y + hheight + SHADOW_OFFSET, width, height - hheight, COL_INFOBAR_SHADOW_PLUS_0, RADIUS_MID, CORNER_BOTTOM);//round
	frameBuffer->paintBoxRel(x, y + hheight, width, height - hheight, COL_MENUCONTENT_PLUS_0, RADIUS_MID, CORNER_BOTTOM);//round

	if (!(iconfile.empty()))
	{
		frameBuffer->getIconSize(iconfile.c_str(), &icol_w, &icol_h);
		
		frameBuffer->paintIcon(iconfile, x + 8, y + (hheight - icol_h)/2); //not correct fixme

		iconoffset = icol_w + BORDER_LEFT;
	}
	else
		iconoffset = 0;

	g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(x + iconoffset + BORDER_LEFT, y + hheight, width - 10 - iconoffset, head ? head : g_Locale->getText(name), COL_MENUHEAD, 0, true); // UTF-8

	if (hint_1 != NONEXISTANT_LOCALE)
	{
		g_Font[SNeutrinoSettings::FONT_TYPE_MENU_INFO]->RenderString(x + 20, y + hheight + mheight + iheight + 40, width- 20, getHint1(), COL_MENUCONTENT, 0, true); // UTF-8
		if (hint_2 != NONEXISTANT_LOCALE)
			g_Font[SNeutrinoSettings::FONT_TYPE_MENU_INFO]->RenderString(x + 20, y + hheight + mheight + iheight*2 + 40, width - 20, g_Locale->getText(hint_2), COL_MENUCONTENT, 0, true); // UTF-8
	}

	for (int count = 0; count < size; count++)
		paintChar(count);
}

void CStringInput::paintChar(int pos, const char c)
{
	const int xs = 20;
	int ys = mheight;
	int xpos = x + 20+ pos* xs;
	int ypos = y + hheight+ 25;

	char ch[2] = {c, 0};

	uint8_t    color;
	fb_pixel_t bgcolor;
	
	if (pos == selected)
	{
		color   = COL_MENUCONTENTSELECTED;
		bgcolor = COL_MENUCONTENTSELECTED_PLUS_0;
	}
	else
	{
		color   = COL_MENUCONTENT;
		bgcolor = COL_MENUCONTENT_PLUS_0;
	}

	frameBuffer->paintBoxRel(xpos, ypos, xs, ys, COL_MENUCONTENT_PLUS_4);
	frameBuffer->paintBoxRel(xpos+ 1, ypos+ 1, xs- 2, ys- 2, bgcolor);

	int xfpos = xpos + ((xs- g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(ch))>>1);

	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(xfpos,ypos+ys, width, ch, color);
}

void CStringInput::paintChar(int pos)
{
	if(pos < (int)strlen(value))
		paintChar(pos, value[pos]);
}

// CStringInputSMS
CStringInputSMS::CStringInputSMS(const neutrino_locale_t Name, std::string* Value, int Size, const neutrino_locale_t Hint_1, const neutrino_locale_t Hint_2, const char * const Valid_Chars, CChangeObserver* Observ, const char * const Icon)
		: CStringInput(Name, Value, Size, Hint_1, Hint_2, Valid_Chars, Observ, Icon)
{
	initSMS(Valid_Chars);
}

CStringInputSMS::CStringInputSMS(const neutrino_locale_t Name, char* Value, int Size, const neutrino_locale_t Hint_1, const neutrino_locale_t Hint_2, const char * const Valid_Chars, CChangeObserver* Observ, const char * const Icon)
   		: CStringInput(Name, Value, Size, Hint_1, Hint_2, Valid_Chars, Observ, Icon)
{
	initSMS(Valid_Chars);
}

CStringInputSMS::CStringInputSMS(char * Head, char* Value, int Size, const neutrino_locale_t Hint_1, const neutrino_locale_t Hint_2, const char * const Valid_Chars, CChangeObserver* Observ, const char * const Icon)
   		: CStringInput(Head, Value, Size, Hint_1, Hint_2, Valid_Chars, Observ, Icon)
{
	initSMS(Valid_Chars);
}

void CStringInputSMS::initSMS(const char * const Valid_Chars)
{
	last_digit = -1;				// no key pressed yet
	const char CharList[10][11] = { "0 -_/()<>=",	// 9 characters
					"1+.,:!?\\",
					"abc2ä",
					"def3",
					"ghi4",
					"jkl5",
					"mno6ö",
					"pqrs7ß",
					"tuv8ü",
					"wxyz9" };

	for (int i = 0; i < 10; i++)
	{
		int j = 0;
		for (int k = 0; k < (int) strlen(CharList[i]); k++)
			if (strchr(Valid_Chars, CharList[i][k]) != NULL)
				Chars[i][j++] = CharList[i][k];
		if (j == 0)
			Chars[i][j++] = ' ';	// prevent empty char lists 
		arraySizes[i] = j;
	}

	height += 260;
	//y = ((500 - height)>>1);
	y = frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - height) >> 1 );
}

void CStringInputSMS::NormalKeyPressed(const neutrino_msg_t key)
{
	if (CRCInput::isNumeric(key))
	{
		int numericvalue = CRCInput::getNumericValue(key);
		if (last_digit != numericvalue)
		{
			if ((last_digit != -1) &&	// there is a last key
			    (selected < (size- 1)))	// we can shift the cursor one field to the right
			{
				selected++;
				paintChar(selected - 1);
			}
			keyCounter = 0;
		}
		else
			keyCounter = (keyCounter + 1) % arraySizes[numericvalue];
		
		value[selected] = Chars[numericvalue][keyCounter];
		last_digit = numericvalue;
		paintChar(selected);
		//
		smstimer = g_RCInput->addTimer(2*1000*1000);
	}
	else
	{
		value[selected] = (char)CRCInput::getUnicodeValue(key);
		keyRedPressed();   /* to lower, paintChar */
		keyRightPressed(); /* last_digit = -1, move to next position */
	}
}

void CStringInputSMS::keyRedPressed()		// switch between lower & uppercase
{
	if (((value[selected] | 32) >= 'a') && ((value[selected] | 32) <= 'z'))
	{
		value[selected] ^= 32;
		//
		paintChar(selected);
	}

	//paintChar(selected);
	smstimer = g_RCInput->addTimer(2*1000*1000);
}

void CStringInputSMS::keyYellowPressed()		// clear all
{
	last_digit = -1;
	CStringInput::keyYellowPressed();
}

void CStringInputSMS::keyUpPressed()
{
	last_digit = -1;

	if (selected > 0)
	{
		int lastselected = selected;
		selected = 0;
		paintChar(lastselected);
		paintChar(selected);
	}
}

void CStringInputSMS::keyDownPressed()
{
	last_digit = -1;

	int lastselected = selected;
	
	selected = size - 1;

	while (value[selected] == ' ')
	{
		selected--;
		if (selected < 0)
			break;
	}

	if (selected < (size - 1))
		selected++;
	
	paintChar(lastselected);
	paintChar(selected);
}

void CStringInputSMS::keyLeftPressed()
{
	last_digit = -1;
	CStringInput::keyLeftPressed();
}

void CStringInputSMS::keyRightPressed()
{
	last_digit = -1;
	CStringInput::keyRightPressed();
}

const struct button_label CStringInputSMSButtons[2] =
{
	{ NEUTRINO_ICON_BUTTON_RED   , LOCALE_STRINGINPUT_CAPS  },
//	{ NEUTRINO_ICON_BUTTON_GREEN , LOCALE_XXX               },
	{ NEUTRINO_ICON_BUTTON_YELLOW, LOCALE_STRINGINPUT_CLEAR }
//	{ NEUTRINO_ICON_BUTTON_BLUE  , LOCALE_XXX               }
};

void CStringInputSMS::paint()
{
	CStringInput::paint();

	//numeric pad
	int icol_w, icol_h;
	frameBuffer->getIconSize(NEUTRINO_ICON_NUMERIC_PAD, &icol_w, &icol_h);
	frameBuffer->paintIcon(NEUTRINO_ICON_NUMERIC_PAD, x + (width - icol_w)/2, y + hheight + mheight + iheight*3 + 20, COL_MENUCONTENT);

	// foot
	int icon_w, icon_h;
	frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_RED, &icon_w, &icon_h);
	int ButtonHeight = std::max(g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight(), icon_h) + 6;
	frameBuffer->paintBoxRel(x, y + height - ButtonHeight, width, ButtonHeight, COL_MENUHEAD_PLUS_0, RADIUS_MID, CORNER_BOTTOM);

	// foot buttons
	::paintButtons(frameBuffer, g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL], g_Locale, x + BORDER_LEFT, y + height - ButtonHeight, ((width - 20)/4)*2, 2, CStringInputSMSButtons, ButtonHeight);
}

void CPINInput::paintChar(int pos)
{
	CStringInput::paintChar(pos, (value[pos] == ' ') ? ' ' : '*');
}

//CPINInput
int CPINInput::exec( CMenuTarget* parent, const std::string & )
{
	neutrino_msg_t      msg;
	neutrino_msg_data_t data;

	int res = menu_return::RETURN_REPAINT;

	if (parent)
		parent->hide();

	for(int count=strlen(value)-1;count<size-1;count++)
		strcat(value, " ");

	paint();
	
	frameBuffer->blit();

	bool loop = true;

	while(loop)
	{
		g_RCInput->getMsg( &msg, &data, 300 );

		if (msg==CRCInput::RC_left)
		{
			keyLeftPressed();
		}
		else if (msg == CRCInput::RC_right)
		{
			keyRightPressed();
		}
		else if (CRCInput::isNumeric(msg))
		{
			int old_selected = selected;
			NormalKeyPressed(msg);
			if ( old_selected == ( size- 1 ) )
				loop = false;
		}
		else if ( (msg == CRCInput::RC_up) || (msg == CRCInput::RC_down) )
		{
			g_RCInput->postMsg( msg, data );
			res = menu_return::RETURN_EXIT;
			loop = false;
		}
		else if ( (msg == CRCInput::RC_home) || (msg == CRCInput::RC_timeout) || (msg == CRCInput::RC_ok) )
		{
			loop = false;
		}
		else
		{
			int r = handleOthers(msg, data);
			if (r & (messages_return::cancel_all | messages_return::cancel_info))
			{
				res = (r & messages_return::cancel_all) ? menu_return::RETURN_EXIT_ALL : menu_return::RETURN_EXIT;
				loop = false;
			}
			else if ( r & messages_return::unhandled )
			{
				if ( CNeutrinoApp::getInstance()->handleMsg( msg, data ) & ( messages_return::cancel_all | messages_return::cancel_info ) )
				{
					loop = false;
					res = menu_return::RETURN_EXIT_ALL;
				}
			}
		}

		frameBuffer->blit();
	}

	hide();

	for(int count=size-1;count>=0;count--)
	{
		if((value[count]==' ') || (value[count]==0))
		{
			value[count]=0;
		}
		else
			break;
	}
	value[size]=0;

	if ( (observ) && (msg==CRCInput::RC_ok) )
	{
		observ->changeNotify(name, value);
	}

	return res;
}

int CPLPINInput::handleOthers(neutrino_msg_t msg, neutrino_msg_data_t data)
{
	int res = messages_return::unhandled;

	if ( msg == NeutrinoMessages::EVT_PROGRAMLOCKSTATUS )
	{
		// trotzdem handlen
		CNeutrinoApp::getInstance()->handleMsg(msg, data);

		if (data != (neutrino_msg_data_t) fsk)
			res = messages_return::cancel_info;
	}

	return res;
}

const char * CPLPINInput::getHint1(void)
{
	if (fsk == 0x100)
	{
		hint_1 = LOCALE_PARENTALLOCK_LOCKEDCHANNEL;
		return CStringInput::getHint1();
	}
	else
	{
		sprintf(hint, g_Locale->getText(LOCALE_PARENTALLOCK_LOCKEDPROGRAM), fsk);
		return hint;
	}
}

int CPLPINInput::exec( CMenuTarget* parent, const std::string & )
{
	fb_pixel_t * pixbuf = new fb_pixel_t[(width + 2 * borderwidth) * (height + 2 * borderwidth)];

	if (pixbuf != NULL)
	{
		frameBuffer->SaveScreen(x- borderwidth, y- borderwidth, width+ 2* borderwidth, height+ 2* borderwidth, pixbuf);
		
		frameBuffer->blit();
	}

	// clear border
	frameBuffer->paintBackgroundBoxRel(x- borderwidth, y- borderwidth, width+ 2* borderwidth, borderwidth);
	frameBuffer->paintBackgroundBoxRel(x- borderwidth, y+ height, width+ 2* borderwidth, borderwidth);
	frameBuffer->paintBackgroundBoxRel(x- borderwidth, y, borderwidth, height);
	frameBuffer->paintBackgroundBoxRel(x+ width, y, borderwidth, height);
	
	frameBuffer->blit();

	int res = CPINInput::exec ( parent, "" );

	if (pixbuf != NULL)
	{
		frameBuffer->RestoreScreen(x- borderwidth, y- borderwidth, width+ 2* borderwidth, height+ 2* borderwidth, pixbuf);

		frameBuffer->blit();
		
		delete[] pixbuf;//Mismatching allocation and deallocation: pixbuf
	}

	return ( res );
}
