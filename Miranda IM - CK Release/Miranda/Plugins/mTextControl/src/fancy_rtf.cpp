/*
Miranda Text Control for
Miranda IM: the free IM client for Microsoft* Windows*

Authors 
				Copyright (C) 2005 Victor Pavlychko (nullbie@gmail.com),
							  2010 Merlin_de

Copyright 2000-2012 Miranda IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

===============================================================================

File name      : $HeadURL: 
Revision       : $Revision: 
Last change on : $Date: 
Last change by : $Author:
$Id$		   : $Id$:

===============================================================================

* Implementation of the IDataObject COM interface @By J Brown 2004 
* www.catch22.net

* Implementation of the IEnumFORMATETC interface
* For Win2K and above look at the SHCreateStdEnumFmtEtc API call!!

* Apparently the order of formats in an IEnumFORMATETC object must be the same as
* those that were stored in the clipboard.
*/

#include "headers.h"

struct BBCodeInfo
{
	TCHAR *start;
	TCHAR *end;
	bool (* func)(IFormattedTextDraw *ftd, CHARRANGE range, TCHAR *txt, DWORD cookie);
	DWORD cookie;
};

enum { BBS_BOLD_S, BBS_BOLD_E, BBS_ITALIC_S, BBS_ITALIC_E, BBS_UNDERLINE_S, BBS_UNDERLINE_E,
	BBS_STRIKEOUT_S, BBS_STRIKEOUT_E, BBS_COLOR_S, BBS_COLOR_E, BBS_URL1, BBS_URL2,
	BBS_IMG1, BBS_IMG2 };

static bool bbCodeSimpleFunc(IFormattedTextDraw *ftd, CHARRANGE range, TCHAR *txt, DWORD cookie)
{
	CHARFORMAT cf = {0};
	cf.cbSize = sizeof(cf);
	switch (cookie)
	{
		case BBS_BOLD_S:
			cf.dwMask = CFM_BOLD;
			cf.dwEffects = CFE_BOLD;
			break;
		case BBS_BOLD_E:
			cf.dwMask = CFM_BOLD;
			break;
		case BBS_ITALIC_S:
			cf.dwMask = CFM_ITALIC;
			cf.dwEffects = CFE_ITALIC;
			break;
		case BBS_ITALIC_E:
			cf.dwMask = CFM_ITALIC;
			break;
		case BBS_UNDERLINE_S:
			cf.dwMask = CFM_UNDERLINE;
			cf.dwEffects = CFE_UNDERLINE;
			break;
		case BBS_UNDERLINE_E:
			cf.dwMask = CFM_UNDERLINE;
			break;
		case BBS_STRIKEOUT_S:
			cf.dwMask = CFM_STRIKEOUT;
			cf.dwEffects = CFE_STRIKEOUT;
			break;
		case BBS_STRIKEOUT_E:
			cf.dwMask = CFM_STRIKEOUT;
			break;
	}

	ITextServices *ts = ftd->getTextService();

	LRESULT lResult;
	ts->TxSendMessage(EM_SETSEL, range.cpMin, -1, &lResult);
	ts->TxSendMessage(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf, &lResult);
	ts->TxSendMessage(EM_SETSEL, range.cpMin, range.cpMax, &lResult);
	ts->TxSendMessage(EM_REPLACESEL, FALSE, (LPARAM)_T(""), &lResult);

	return true;
}

static bool bbCodeImageFunc(IFormattedTextDraw *ftd, CHARRANGE range, TCHAR *txt, DWORD cookie)
{
	ITextServices *ts = ftd->getTextService();
	ITextDocument *td = ftd->getTextDocument();

	long cnt;
	LRESULT lResult;
/*
	TEXTRANGE trg;
	trg.chrg = range;
	trg.lpstrText = new TCHAR[trg.chrg.cpMax - trg.chrg.cpMin + 1];
	ts->TxSendMessage(EM_GETTEXTRANGE, 0, (LPARAM)&trg, &lResult);
	MessageBox(0, txt, trg.lpstrText, MB_OK);
*/
	ts->TxSendMessage(EM_SETSEL, range.cpMin, range.cpMax, &lResult);
	IRichEditOle* RichEditOle;
	ts->TxSendMessage(EM_GETOLEINTERFACE, 0, (LPARAM)&RichEditOle, &lResult);
	td->Freeze(&cnt);
//	HDC emfdc = CreateEnhMetaFile(NULL, NULL, NULL, _T("icon"));
//	DrawIconEx(emfdc, 0, 0, (HICON)_ttol(txt), 16, 16, 0, NULL, DI_NORMAL);
//	InsertBitmap(RichEditOle, CloseEnhMetaFile(emfdc));
#ifdef _WIN64
	bool res = InsertBitmap(RichEditOle, CacheIconToEmf((HICON)_tstoi64(txt)));
#else
	bool res = InsertBitmap(RichEditOle, CacheIconToEmf((HICON)_ttoi(txt)));
#endif

	td->Unfreeze(&cnt);
	RichEditOle->Release();
	return res;
}

static BBCodeInfo bbCodes[] =
{
	{ _T("[b]"),      0,           bbCodeSimpleFunc, BBS_BOLD_S },
	{ _T("[/b]"),     0,           bbCodeSimpleFunc, BBS_BOLD_E },
	{ _T("[i]"),      0,           bbCodeSimpleFunc, BBS_ITALIC_S },
	{ _T("[/i]"),     0,           bbCodeSimpleFunc, BBS_ITALIC_E },
	{ _T("[u]"),      0,           bbCodeSimpleFunc, BBS_UNDERLINE_S },
	{ _T("[/u]"),     0,           bbCodeSimpleFunc, BBS_UNDERLINE_E },
	{ _T("[s]"),      0,           bbCodeSimpleFunc, BBS_STRIKEOUT_S },
	{ _T("[/s]"),     0,           bbCodeSimpleFunc, BBS_STRIKEOUT_E },

//	{ _T("[color="),  _T("]"),     bbCodeSimpleFunc, BBS_COLOR_S },
//	{ _T("[/color]"), 0,           bbCodeSimpleFunc, BBS_COLOR_E }

	{ _T("[$hicon="),  _T("$]"),   bbCodeImageFunc,  0 }

//	{ _T("[url]"),   _T("[/url]"), bbCodeSimpleFunc, BBS_URL1 },
//	{ _T("[url="),    _T("]"),     bbCodeSimpleFunc, BBS_URL2 },
//	{ _T("[url]"),   _T("[/url]"), bbCodeSimpleFunc, BBS_IMG1 },
//	{ _T("[url="),    _T("]"),     bbCodeSimpleFunc, BBS_IMG2 },
};
static int bbCodeCount = sizeof(bbCodes) / sizeof(*bbCodes);

void bbCodeParse(IFormattedTextDraw *ftd)
{
	ITextServices *ts = ftd->getTextService();
	LRESULT lResult;

	int pos = 0;
	for (bool found = true; found; )
	{
		found = false;
		CHARRANGE fRange; fRange.cpMin = -1;
		TCHAR *fText = 0;
		BBCodeInfo *fBBCode;

		for (int i = 0; i < bbCodeCount; i++)
		{
			CHARRANGE range;

			FINDTEXTEX fte;
			fte.chrg.cpMin = pos;
			fte.chrg.cpMax = -1;

			fte.lpstrText = bbCodes[i].start;
			ts->TxSendMessage(EM_FINDTEXTEX, (WPARAM)FR_DOWN, (LPARAM)&fte, &lResult);
			if (lResult == -1)
				continue;
			range = fte.chrgText;

			if (bbCodes[i].end)
			{
				fte.chrg.cpMin = fte.chrgText.cpMax;
				fte.lpstrText = bbCodes[i].end;
				ts->TxSendMessage(EM_FINDTEXTEX, (WPARAM)FR_DOWN, (LPARAM)&fte, &lResult);
				if (lResult == -1)
					continue;
				range.cpMax = fte.chrgText.cpMax;
			}

			if ((fRange.cpMin == -1) || (fRange.cpMin > range.cpMin))
			{
				fRange = range;
				fBBCode = bbCodes+i;
				found = true;

				if (fText) delete [] fText;
				if (bbCodes[i].end)
				{
					TEXTRANGE trg;
					trg.chrg.cpMin = fte.chrg.cpMin;
					trg.chrg.cpMax = fte.chrgText.cpMin;
					trg.lpstrText = new TCHAR[trg.chrg.cpMax - trg.chrg.cpMin + 1];
					ts->TxSendMessage(EM_GETTEXTRANGE, 0, (LPARAM)&trg, &lResult);
					fText = trg.lpstrText;
				}
			}
		}

		if (found)
		{
			found = fBBCode->func(ftd, fRange, fText, fBBCode->cookie);
			if (fText) delete [] fText;
		}
	}
}
