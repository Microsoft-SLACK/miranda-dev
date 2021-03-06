/*
TipperYM plugin for
Miranda IM: the free IM client for Microsoft* Windows*

Authors
            Copyright (C) 2006-2009 Scott Ellis
            Copyright (C) 2007-2011 Jan Holub

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
*/

#include "common.h"
#include "str_utils.h"

int iCodePage = CP_ACP;

bool a2w(const char *as, wchar_t *buff, int bufflen)
{
	if (as) MultiByteToWideChar(iCodePage, 0, as, -1, buff, bufflen);
	return true;
}

bool w2a(const wchar_t *ws, char *buff, int bufflen) 
{
	if (ws) WideCharToMultiByte(iCodePage, 0, ws, -1, buff, bufflen, 0, 0);
	return true;
}

bool utf2w(const char *us, wchar_t *buff, int bufflen) 
{
	if (us) MultiByteToWideChar(CP_UTF8, 0, us, -1, buff, bufflen);
	return true;
}

bool w2utf(const wchar_t *ws, char *buff, int bufflen) 
{
	if (ws) WideCharToMultiByte(CP_UTF8, 0, ws, -1, buff, bufflen, 0, 0);
	return true;
}

bool a2utf(const char *as, char *buff, int bufflen) 
{
	if (!as) return false;

	wchar_t *ws = a2w(as);
	if (ws) WideCharToMultiByte(CP_UTF8, 0, ws, -1, buff, bufflen, 0, 0);
	mir_free(ws);
	return true;
}

bool utf2a(const char *us, char *buff, int bufflen)
{
	if (!us) return false;

	wchar_t *ws = utf2w(us);
	if (ws) WideCharToMultiByte(iCodePage, 0, ws, -1, buff, bufflen, 0, 0);
	mir_free(ws);
	return true;
}


bool t2w(const TCHAR *ts, wchar_t *buff, int bufflen) 
{
#ifdef _UNICODE
	wcsncpy(buff, ts, bufflen);
	return true;
#else
	return a2w(ts, buff, bufflen);
#endif
}

bool w2t(const wchar_t *ws, TCHAR *buff, int bufflen)
{
#ifdef _UNICODE
	wcsncpy(buff, ws, bufflen);
	return true;
#else
	return w2a(ws, buff, bufflen);
#endif
}

bool t2a(const TCHAR *ts, char *buff, int bufflen) 
{
#ifdef _UNICODE
	return w2a(ts, buff, bufflen);
#else
	strncpy(buff, ts, bufflen);
	return true;
#endif
}

bool a2t(const char *as, TCHAR *buff, int bufflen) 
{
#ifdef _UNICODE
	return a2w(as, buff, bufflen);
#else
	strncpy(buff, as, bufflen);
	return true;
#endif
}

bool t2utf(const TCHAR *ts, char *buff, int bufflen)
{
#ifdef _UNICODE
	return w2utf(ts, buff, bufflen);
#else
	return a2utf(ts, buff, bufflen);
#endif
}

bool utf2t(const char *us, TCHAR *buff, int bufflen) 
{
#ifdef _UNICODE
	return utf2w(us, buff, bufflen);
#else
	return utf2a(us, buff, bufflen);
#endif
}

wchar_t *utf2w(const char *us)
{
	if (us)
	{
		int size = MultiByteToWideChar(CP_UTF8, 0, us, -1, 0, 0);
		wchar_t *buff = (wchar_t *)mir_alloc(size * sizeof(wchar_t));
		MultiByteToWideChar(CP_UTF8, 0, us, -1, buff, size);
		return buff;
	} 
	else
	{
		return 0;
	}
}

char *w2utf(const wchar_t *ws) 
{
	if (ws)
	{
		int size = WideCharToMultiByte(CP_UTF8, 0, ws, -1, 0, 0, 0, 0);
		char *buff = (char *)mir_alloc(size);
		WideCharToMultiByte(CP_UTF8, 0, ws, -1, buff, size, 0, 0);
		return buff;
	} 
	else 
	{
		return 0;
	}
}

wchar_t *a2w(const char *as)
{
	int size = MultiByteToWideChar(iCodePage, 0, as, -1, 0, 0);
	wchar_t *buff = (wchar_t *)mir_alloc(size * sizeof(wchar_t));
	MultiByteToWideChar(iCodePage, 0, as, -1, buff, size);
	return buff;
}

char *w2a(const wchar_t *ws)
{
	int size = WideCharToMultiByte(iCodePage, 0, ws, -1, 0, 0, 0, 0);
	char *buff = (char *)mir_alloc(size);
	WideCharToMultiByte(iCodePage, 0, ws, -1, buff, size, 0, 0);
	return buff;
}

char *utf2a(const char *utfs)
{
	wchar_t *ws = utf2w(utfs);
	char *ret = w2a(ws);
	mir_free(ws);
	return ret;
}

char *a2utf(const char *as) 
{
	wchar_t *ws = a2w(as);
	char *ret = w2utf(ws);
	mir_free(ws);
	return ret;
}

TCHAR *w2t(const wchar_t *ws)
{
#ifdef _UNICODE
	return mir_wstrdup(ws);
#else
	return w2a(ws);
#endif
}

wchar_t *t2w(const TCHAR *ts)
{
#ifdef _UNICODE
	return mir_tstrdup(ts);
#else
	return a2w(ts);
#endif
}


char *t2a(const TCHAR *ts)
{
#ifdef _UNICODE
	return w2a(ts);
#else
	return mir_strdup(ts);
#endif
}

TCHAR *a2t(const char *as) 
{
#ifdef _UNICODE
	return a2w(as);
#else
	return mir_strdup(as);
#endif
}

TCHAR *utf2t(const char *utfs)
{
#ifdef _UNICODE
	return utf2w(utfs);
#else
	wchar_t *ws = utf2w(utfs);
	char *ret = w2a(ws);
	mir_free(ws);
	return ret;
#endif
}

char *t2utf(const TCHAR *ts)
{
#ifdef _UNICODE
	return w2utf(ts);
#else
	wchar_t *ws = a2w(ts);
	char *ret = w2utf(ws);
	mir_free(ws);
	return ret;
#endif
}

TCHAR *myfgets(TCHAR *Buf, int MaxCount, FILE *File)
{
	_fgetts(Buf, MaxCount, File);
	for (size_t i = _tcslen(Buf) - 1; i >= 0; i--)
	{
		if (Buf[i] == '\n' || Buf[i] == ' ')
			Buf[i] = 0;
		else
			break;
	}

	CharLower(Buf);
	return Buf;
}