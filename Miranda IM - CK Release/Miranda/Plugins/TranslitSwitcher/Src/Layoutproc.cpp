/*
TranslitSwitcher plugin for
Miranda IM: the free IM client for Microsoft* Windows*

Author
			Copyright (C) 2007 Dmitry Titkov 
			Copyright (C) 2010 tico-tico
			Copyright (C) 2011-2012 Mataes

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

#include "TranslitSwitcher.h"

SMADD_BATCHPARSE2 smgp;
SMADD_BATCHPARSERES *smileyPrs = NULL;

BOOL isItSmiley(unsigned int position)
{
	unsigned int j;

	if (smileyPrs == NULL)
		return FALSE;

	for (j = 0; j < smgp.numSmileys; j++)
	{
		if (position >= smileyPrs[j].startChar && position < (smileyPrs[j].startChar + smileyPrs[j].size))
			return TRUE;
	}
	
	return FALSE;
}

static DWORD CALLBACK StreamOutCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG * pcb)
{
	static int sendBufferSize;
	char ** ppText = (char **) dwCookie;

	if (*ppText == NULL)
		sendBufferSize = 0;

	*ppText = (char *)mir_realloc(*ppText, sendBufferSize + cb + 2);
	memcpy (*ppText + sendBufferSize, pbBuff, cb);
	sendBufferSize += cb;
	*((TCHAR *)(*ppText + sendBufferSize)) = '\0';
	*pcb = cb;
    return 0;
}

TCHAR* Message_GetFromStream(HWND hwndRtf, DWORD dwPassedFlags)
{
	EDITSTREAM stream = {0};
	TCHAR *pszText = NULL;

	if (hwndRtf == 0)
		return NULL;

	stream.pfnCallback = StreamOutCallback;
	stream.dwCookie = (DWORD_PTR) & pszText;
	SendMessage(hwndRtf, EM_STREAMOUT, (WPARAM)dwPassedFlags, (LPARAM) & stream);

	return pszText;
}

VOID Transliterate(TCHAR*& str)
{
	TCHAR* newStr = (TCHAR*)mir_alloc(sizeof(TCHAR)*lstrlen(str)*3+1);
	newStr[0] = 0;
	for (; *str != 0; str++)
	{
		switch (str[0])
		{
			case _T('�'): _tcscat(newStr, _T("a")); break;
			case _T('�'): _tcscat(newStr, _T("b")); break;
			case _T('�'): _tcscat(newStr, _T("v")); break;
			case _T('�'): _tcscat(newStr, _T("g")); break;
			case _T('�'): _tcscat(newStr, _T("d")); break;
			case _T('�'): _tcscat(newStr, _T("e")); break;
			case _T('�'): _tcscat(newStr, _T("ye")); break;
			case _T('�'): _tcscat(newStr, _T("zh")); break;
			case _T('�'): _tcscat(newStr, _T("z")); break;
			case _T('�'): _tcscat(newStr, _T("i")); break;
			case _T('�'): _tcscat(newStr, _T("y")); break;
			case _T('�'): _tcscat(newStr, _T("k")); break;
			case _T('�'): _tcscat(newStr, _T("l")); break;
			case _T('�'): _tcscat(newStr, _T("m")); break;
			case _T('�'): _tcscat(newStr, _T("n")); break;
			case _T('�'): _tcscat(newStr, _T("o")); break;
			case _T('�'): _tcscat(newStr, _T("p")); break;
			case _T('�'): _tcscat(newStr, _T("r")); break;
			case _T('�'): _tcscat(newStr, _T("s")); break;
			case _T('�'): _tcscat(newStr, _T("t")); break;
			case _T('�'): _tcscat(newStr, _T("u")); break;
			case _T('�'): _tcscat(newStr, _T("f")); break;
			case _T('�'): _tcscat(newStr, _T("kh")); break;
			case _T('�'): _tcscat(newStr, _T("ts")); break;
			case _T('�'): _tcscat(newStr, _T("ch")); break;
			case _T('�'): _tcscat(newStr, _T("sh")); break;
			case _T('�'): _tcscat(newStr, _T("sch")); break;
			case _T('�'): _tcscat(newStr, _T("'")); break;
			case _T('�'): _tcscat(newStr, _T("yi")); break;
			case _T('�'): _tcscat(newStr, _T("")); break;
			case _T('�'): _tcscat(newStr, _T("e")); break;
			case _T('�'): _tcscat(newStr, _T("yu")); break;
			case _T('�'): _tcscat(newStr, _T("ya")); break;
			case _T('�'): _tcscat(newStr, _T("A")); break;
			case _T('�'): _tcscat(newStr, _T("B")); break;
			case _T('�'): _tcscat(newStr, _T("V")); break;
			case _T('�'): _tcscat(newStr, _T("G")); break;
			case _T('�'): _tcscat(newStr, _T("D")); break;
			case _T('�'): _tcscat(newStr, _T("E")); break;
			case _T('�'): _tcscat(newStr, _T("Ye")); break;
			case _T('�'): _tcscat(newStr, _T("Zh")); break;
			case _T('�'): _tcscat(newStr, _T("Z")); break;
			case _T('�'): _tcscat(newStr, _T("I")); break;
			case _T('�'): _tcscat(newStr, _T("Y")); break;
			case _T('�'): _tcscat(newStr, _T("K")); break;
			case _T('�'): _tcscat(newStr, _T("L")); break;
			case _T('�'): _tcscat(newStr, _T("M")); break;
			case _T('�'): _tcscat(newStr, _T("N")); break;
			case _T('�'): _tcscat(newStr, _T("O")); break;
			case _T('�'): _tcscat(newStr, _T("P")); break;
			case _T('�'): _tcscat(newStr, _T("R")); break;
			case _T('�'): _tcscat(newStr, _T("S")); break;
			case _T('�'): _tcscat(newStr, _T("T")); break;
			case _T('�'): _tcscat(newStr, _T("U")); break;
			case _T('�'): _tcscat(newStr, _T("F")); break;
			case _T('�'): _tcscat(newStr, _T("Kh")); break;
			case _T('�'): _tcscat(newStr, _T("Ts")); break;
			case _T('�'): _tcscat(newStr, _T("Ch")); break;
			case _T('�'): _tcscat(newStr, _T("Sh")); break;
			case _T('�'): _tcscat(newStr, _T("Sch")); break;
			case _T('�'): _tcscat(newStr, _T("'")); break;
			case _T('�'): _tcscat(newStr, _T("Yi")); break;
			case _T('�'): _tcscat(newStr, _T("")); break;
			case _T('�'): _tcscat(newStr, _T("E")); break;
			case _T('�'): _tcscat(newStr, _T("Yu")); break;
			case _T('�'): _tcscat(newStr, _T("Ya")); break;

			case _T('a'): _tcscat(newStr, _T("�")); break;
			case _T('b'): _tcscat(newStr, _T("�")); break;
			case _T('v'): _tcscat(newStr, _T("�")); break;
			case _T('g'): _tcscat(newStr, _T("�")); break;
			case _T('d'): _tcscat(newStr, _T("�")); break;
			case _T('e'): _tcscat(newStr, _T("�")); break;
			case _T('z'):
				{
					if (str[1] == _T('h'))
					{
						_tcscat(newStr, _T("�"));
						str++;
						break;
					}
					else
					{
						_tcscat(newStr, _T("�"));
						break;
					}
				}
			case _T('i'): _tcscat(newStr, _T("�")); break;
			case _T('y'):
				{
					if (str[1] == _T('a'))
					{
						_tcscat(newStr, _T("�")); 
						str++;
						break;
					}
					else if (str[1] == _T('e'))
					{
						_tcscat(newStr, _T("�"));
						str++;
						break;
					}
					else if (str[1] == _T('u'))
					{
						_tcscat(newStr, _T("�"));
						str++;
						break;
					}
					else if (str[1] == _T('i'))
					{
						_tcscat(newStr, _T("�"));
						str++;
						break;
					}
					else
					{
						_tcscat(newStr, _T("�")); 
						break;
					}
				}
			case _T('k'):
				{
					if (str[1] == _T('h'))
					{
						_tcscat(newStr, _T("�"));
						str++;
						break;
					}
					else
					{
						_tcscat(newStr, _T("�"));
						break;
					}
				}
			case _T('l'): _tcscat(newStr, _T("�")); break;
			case _T('m'): _tcscat(newStr, _T("�")); break;
			case _T('n'): _tcscat(newStr, _T("�")); break;
			case _T('o'): _tcscat(newStr, _T("�")); break;
			case _T('p'): _tcscat(newStr, _T("�")); break;
			case _T('r'): _tcscat(newStr, _T("�")); break;
			case _T('s'):
				{
					if (str[1] == _T('h'))
					{
						_tcscat(newStr, _T("�")); 
						str++;
						break;
					}
					else if (str[1] == _T('c') && str[2] == _T('h'))
					{
						_tcscat(newStr, _T("�"));
						str+=2;
						break;
					}
					else
					{
						_tcscat(newStr, _T("�"));
						break;
					}
				}
			case _T('t'):
				{
					if (str[1] == _T('s'))
					{
						_tcscat(newStr, _T("�"));
						str++;
						break;
					}
					else
					{
						_tcscat(newStr, _T("�"));
						break;
					}
				}
			case _T('u'): _tcscat(newStr, _T("�")); break;
			case _T('f'): _tcscat(newStr, _T("�")); break;
			case _T('c'):
				{
					if (str[1] == _T('h'))
					{
						_tcscat(newStr, _T("�"));
						str++;
						break;
					}
				}
			case _T('A'): _tcscat(newStr, _T("�")); break;
			case _T('B'): _tcscat(newStr, _T("�")); break;
			case _T('V'): _tcscat(newStr, _T("�")); break;
			case _T('G'): _tcscat(newStr, _T("�")); break;
			case _T('D'): _tcscat(newStr, _T("�")); break;
			case _T('E'): _tcscat(newStr, _T("�")); break;
			case _T('Y'):
				{
					if (str[1] == _T('a'))
					{
						_tcscat(newStr, _T("�")); 
						str++;
						break;
					}
					else if (str[1] == _T('e'))
					{
						_tcscat(newStr, _T("�"));
						str++;
						break;
					}
					else if (str[1] == _T('u'))
					{
						_tcscat(newStr, _T("�"));
						str++;
						break;
					}
					else if (str[1] == _T('i'))
					{
						_tcscat(newStr, _T("�"));
						str++;
						break;
					}
					else
					{
						_tcscat(newStr, _T("�")); 
						break;
					}
				}
			case _T('Z'):
				{
					if (str[1] == _T('h'))
					{
						_tcscat(newStr, _T("�"));
						str++;
						break;
					}
					else
					{
						_tcscat(newStr, _T("�"));
						break;
					}
				}
			case _T('I'): _tcscat(newStr, _T("�")); break;
			case _T('K'):
				{
					if (str[1] == _T('h'))
					{
						_tcscat(newStr, _T("�"));
						str++;
						break;
					}
					else
					{
						_tcscat(newStr, _T("�"));
						break;
					}
				}
			case _T('L'): _tcscat(newStr, _T("�")); break;
			case _T('M'): _tcscat(newStr, _T("�")); break;
			case _T('N'): _tcscat(newStr, _T("�")); break;
			case _T('O'): _tcscat(newStr, _T("�")); break;
			case _T('P'): _tcscat(newStr, _T("�")); break;
			case _T('R'): _tcscat(newStr, _T("�")); break;
			case _T('S'):
				{
					if (str[1] == _T('h'))
					{
						_tcscat(newStr, _T("�")); 
						str++;
						break;
					}
					else if (str[1] == _T('c') && str[2] == _T('h'))
					{
						_tcscat(newStr, _T("�"));
						str+=2;
						break;
					}
					else
					{
						_tcscat(newStr, _T("�"));
						break;
					}
				}
			case _T('T'):
				{
					if (str[1] == _T('s'))
					{
						_tcscat(newStr, _T("�"));
						str++;
						break;
					}
					else
					{
						_tcscat(newStr, _T("�"));
						break;
					}
				}
			case _T('U'): _tcscat(newStr, _T("�")); break;
			case _T('F'): _tcscat(newStr, _T("�")); break;
			case _T('C'):
				{
					if (str[1] == _T('h'))
					{
						_tcscat(newStr, _T("�"));
						str++;
						break;
					}
				}
			case _T('\''): _tcscat(newStr, _T("�")); break;

			default: { TCHAR Temp[2] = { str[0], 0} ; _tcscat(newStr, &Temp[0]); }
		}
	}
	int len = lstrlen(newStr);
	str = (TCHAR*)mir_alloc((len+1)*sizeof(TCHAR));
	str[0] = 0;
	_tcscpy(str, newStr);
	mir_free(newStr);
}

VOID Invert(TCHAR *str) 
{
	while(*str)
	{
		if(IsCharUpper(*str))
		{
			*str = (TCHAR)CharLower((LPTSTR)*str);
		}
		else if(IsCharLower(*str))
			*str = (TCHAR)CharUpper((LPTSTR)*str);
		str++;
	}
	keybd_event(VK_CAPITAL,0x45,KEYEVENTF_EXTENDEDKEY|0,0);          
	keybd_event(VK_CAPITAL,0x45,KEYEVENTF_EXTENDEDKEY|KEYEVENTF_KEYUP,0);
}

VOID SwitchLayout(BOOL lastword)
{
	HWND hwnd = GetForegroundWindow();

	if(hwnd != NULL)
	{
		DWORD dwProcessID;
		DWORD dwThreadID = GetWindowThreadProcessId(hwnd, &dwProcessID);
		HWND hwnd2 = GetFocus();

		if(hwnd2 != NULL)
		{
			TCHAR szClassName[MAX_PATH];

			GetClassName(hwnd2, szClassName, SIZEOF(szClassName));
			if((lstrcmp(szClassName, _T("THppRichEdit.UnicodeClass")) == 0 || lstrcmp(szClassName, _T("THistoryGrid.UnicodeClass")) == 0 || lstrcmp(szClassName, _T("TExtHistoryGrid.UnicodeClass")) == 0 || lstrcmp(szClassName, _T("Internet Explorer_Server")) == 0) && ServiceExists(MS_POPUP_SHOWMESSAGE))	// make popup here
			{
				TCHAR buf[2048];
				
				if (lstrcmp(szClassName, _T("Internet Explorer_Server")) == 0)
				{
					TCHAR *selected = 0;
					IEVIEWEVENT event;
					HWND hwnd3 = GetParent(GetParent(hwnd2));
					ZeroMemory((void *)&event, sizeof(event));
					event.cbSize = sizeof(IEVIEWEVENT);
					event.hContact = 0;
					event.dwFlags = 0;
					event.iType = IEE_GET_SELECTION;
					event.hwnd = hwnd3;
					selected = (TCHAR *)CallService(MS_IEVIEW_EVENT, 0, (LPARAM) & event);
					lstrcpy(buf, selected);
				}
				else
					SendMessage(hwnd2, WM_GETTEXT, SIZEOF(buf), (LPARAM)buf);		// gimme, gimme, gimme...
				
				int slen = lstrlen(buf);
				if (slen != 0)
				{	
					HKL hkl;
					ActivateKeyboardLayout((HKL)HKL_NEXT, KLF_ACTIVATE); // go to next layout before....
					hkl = GetKeyboardLayout(dwThreadID);
					ActivateKeyboardLayout((HKL)HKL_PREV, KLF_ACTIVATE); // return to prev layout

					if (ServiceExists(MS_SMILEYADD_BATCHPARSE))
					{
						ZeroMemory(&smgp, sizeof(smgp));
						smgp.cbSize = sizeof(smgp);
						smgp.str = buf;
						smgp.flag = SAFL_TCHAR;
						smileyPrs = (SMADD_BATCHPARSERES *)CallService(MS_SMILEYADD_BATCHPARSE, 0, (LPARAM)&smgp);
					}

					for(int i = 0; i < slen; i++)
					{
						SHORT vks;
						BYTE keys[256] = {0};

						vks = VkKeyScanEx(buf[i], hkl);

						keys[VK_SHIFT]   = (HIBYTE(vks) & 1) ? 0xFF : 0x00; // shift
						keys[VK_CONTROL] = (HIBYTE(vks) & 2) ? 0xFF : 0x00; // ctrl
						keys[VK_MENU]    = (HIBYTE(vks) & 4) ? 0xFF : 0x00;	// alt
						
						if (!isItSmiley(i))
						{
							TCHAR tchr;
							if(ToUnicodeEx(LOBYTE(vks), 0, keys, &tchr, 1, 0, GetKeyboardLayout(dwThreadID)) == 1)
								buf[i] = tchr;
						}
					}

					if (smileyPrs != NULL)
						CallService(MS_SMILEYADD_BATCHFREE, 0, (LPARAM)smileyPrs);
					
					POPUPDATAT_V2 pd;
					ZeroMemory(&pd, sizeof(pd));
					pd.cbSize = sizeof(POPUPDATAT_V2);
					pd.lchContact = NULL; //(HANDLE)wParam;
					pd.lchIcon = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM) "SwitchLayout and Send");
					lstrcpyn(pd.lptzText, buf, SIZEOF(pd.lptzText));
					lstrcpyn(pd.lptzContactName, TranslateT("TranslitSwitcher"), SIZEOF(pd.lptzContactName));
					pd.colorBack = pd.colorText = 0;
					pd.iSeconds = 0;
					CallService(MS_POPUP_ADDPOPUPT, (WPARAM) &pd, 0);

					//PUShowMessageT(buf, SM_NOTIFY);
				}
			}
			else
			if(lstrcmp(szClassName, _T("RichEdit20W")) == 0)
			{
				DWORD dwStart, dwEnd;
				int i, slen, start = 0, end = 0;
				TCHAR *sel, tchr;
				BOOL somethingIsSelected = TRUE;
				SHORT vks; 
				BYTE keys[256] = {0};
				HKL hkl = GetKeyboardLayout(dwThreadID); 
					
				SendMessage(hwnd2, EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);
				
				if (dwStart == dwEnd)
					somethingIsSelected = FALSE;
				
				if (somethingIsSelected)
					sel = Message_GetFromStream(hwnd2, SF_TEXT|SF_UNICODE|SFF_SELECTION);
				else
					sel = Message_GetFromStream(hwnd2, SF_TEXT|SF_UNICODE);
				
				slen = lstrlen(sel);
				
				if (slen != 0)
				{
					if (ServiceExists(MS_SMILEYADD_BATCHPARSE))
					{
						ZeroMemory(&smgp, sizeof(smgp));
						smgp.cbSize = sizeof(smgp);
						smgp.str = sel;
						smgp.flag = SAFL_TCHAR;
						smileyPrs = (SMADD_BATCHPARSERES *)CallService(MS_SMILEYADD_BATCHPARSE, 0, (LPARAM)&smgp);
					}

					end = slen;
					if (lastword && !somethingIsSelected)
					{
						end = dwStart;
						while (end < slen)
						{
							if (_istspace(sel[end]) || isItSmiley(end))
								break;
							end++;
						}
						start = dwStart-1;
						while (start > 0 && start < (int)dwStart)
						{
							if ((_istspace(sel[start]) && !_istspace(sel[start+1])) || isItSmiley(start))
								break;
							start--;
						}
					}

					ActivateKeyboardLayout((HKL)HKL_PREV, KLF_ACTIVATE);

					for(i = start; i < end; i++)
					{
						vks = VkKeyScanEx(sel[i], hkl);

						keys[VK_SHIFT]   = (HIBYTE(vks) & 1) ? 0xFF : 0x00; // shift
						keys[VK_CONTROL] = (HIBYTE(vks) & 2) ? 0xFF : 0x00; // ctrl
						keys[VK_MENU]    = (HIBYTE(vks) & 4) ? 0xFF : 0x00;	// alt
						
						if (!isItSmiley(i))
						{
							if(ToUnicodeEx(LOBYTE(vks), 0, keys, &tchr, 1, 0, GetKeyboardLayout(dwThreadID)) == 1)
								sel[i] = tchr;
						}
					}

					if (smileyPrs != NULL)
						CallService(MS_SMILEYADD_BATCHFREE, 0, (LPARAM)smileyPrs);

					if (somethingIsSelected)
						SendMessage(hwnd2, EM_REPLACESEL, FALSE, (LPARAM)sel);
					else
						SendMessage(hwnd2, WM_SETTEXT, 0, (LPARAM)sel);

					SendMessage(hwnd2, EM_SETSEL, dwStart, dwEnd);
				}
				mir_free(sel);
			}
		}
	}
}

void TranslitLayout(BOOL lastword)
{
	HWND hwnd = GetForegroundWindow();

	if(hwnd != NULL)
	{
		DWORD dwProcessID;
		DWORD dwThreadID = GetWindowThreadProcessId(hwnd, &dwProcessID);
		HWND hwnd2 = GetFocus();

		if(hwnd2 != NULL)
		{
			TCHAR szClassName[16];

			GetClassName(hwnd2, szClassName, SIZEOF(szClassName));

			if(lstrcmp(szClassName, _T("RichEdit20W")) == 0)
			{
				DWORD dwStart, dwEnd;
				int slen, start = 0, end = 0;
				TCHAR *sel, *boo;
				BOOL somethingIsSelected = TRUE;
					
				SendMessage(hwnd2, EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);
				
				if (dwStart == dwEnd)
					somethingIsSelected = FALSE;
				
				if (somethingIsSelected)
					sel = Message_GetFromStream(hwnd2, SF_TEXT|SF_UNICODE|SFF_SELECTION);
				else
					sel = Message_GetFromStream(hwnd2, SF_TEXT|SF_UNICODE);
				
				slen = lstrlen(sel);
				
				if (slen != 0)
				{
					end = slen;
					if (lastword && !somethingIsSelected)
					{
						end = dwStart;
						while (end < slen)
						{
							if (_istspace(sel[end]) || isItSmiley(end))
								break;
							end++;
						}
						start = dwStart-1;
						while (start > 0 && start < (int)dwStart)
						{
							if ((_istspace(sel[start]) && (end-start>2)) || isItSmiley(start))
								break;
							start--;
						}
						boo = (TCHAR*)mir_alloc((end-start+1 ) * sizeof(TCHAR));
						_tcsncpy(boo, sel+start, end-start);
						boo[end-start] = 0;
					}
					else
					{
						boo = (TCHAR*)mir_alloc((slen+1 ) * sizeof(TCHAR));
						_tcscpy(boo, sel);
					}

					Transliterate(boo);

					if (somethingIsSelected)
						SendMessage(hwnd2, EM_REPLACESEL, FALSE, (LPARAM)boo);
					else
					{
						TCHAR* NewText = (TCHAR*)mir_alloc((start+_tcslen(boo)+(slen-start)+1)* sizeof(TCHAR));
						NewText[0] = 0;
						_tcsncat(NewText, sel, start);
						_tcscat(NewText, boo);
						_tcsncat(NewText, sel+end, slen-end);
						SendMessage(hwnd2, WM_SETTEXT, 0, (LPARAM)NewText);
						mir_free(NewText);
					}

					SendMessage(hwnd2, EM_SETSEL, dwStart, dwEnd);
				}
				mir_free(sel);
				mir_free(boo);
			}
		}
	}
}

void InvertCase(BOOL lastword)
{
	HWND hwnd = GetForegroundWindow();

	if(hwnd != NULL)
	{
		DWORD dwProcessID;
		DWORD dwThreadID = GetWindowThreadProcessId(hwnd, &dwProcessID);
		HWND hwnd2 = GetFocus();

		if(hwnd2 != NULL)
		{
			TCHAR szClassName[16];

			GetClassName(hwnd2, szClassName, SIZEOF(szClassName));

			if(lstrcmp(szClassName, _T("RichEdit20W")) == 0)
			{
				DWORD dwStart, dwEnd;
				int slen, start = 0, end = 0;
				TCHAR *sel, *boo;
				BOOL somethingIsSelected = TRUE;
					
				SendMessage(hwnd2, EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);
				
				if (dwStart == dwEnd)
					somethingIsSelected = FALSE;
				
				if (somethingIsSelected)
					sel = Message_GetFromStream(hwnd2, SF_TEXT|SF_UNICODE|SFF_SELECTION);
				else
					sel = Message_GetFromStream(hwnd2, SF_TEXT|SF_UNICODE);
				
				slen = lstrlen(sel);
				
				if (slen != 0)
				{
					end = slen;
					if (lastword && !somethingIsSelected)
					{
						end = dwStart;
						while (end < slen)
						{
							if (_istspace(sel[end]) || isItSmiley(end))
								break;
							end++;
						}
						start = dwStart-1;
						while (start > 0 && start < (int)dwStart)
						{
							if ((_istspace(sel[start]) && (end-start>2)) || isItSmiley(start))
								break;
							start--;
						}
						boo = (TCHAR*)mir_alloc((end-start+1 ) * sizeof(TCHAR));
						_tcsncpy(boo, sel+start, end-start);
						boo[end-start] = 0;
					}
					else
					{
						boo = (TCHAR*)mir_alloc((slen+1 ) * sizeof(TCHAR));
						_tcscpy(boo, sel);
					}

					Invert(boo);

					if (somethingIsSelected)
						SendMessage(hwnd2, EM_REPLACESEL, FALSE, (LPARAM)boo);
					else
					{
						TCHAR* NewText = (TCHAR*)mir_alloc((start+_tcslen(boo)+(slen-start)+1)* sizeof(TCHAR));
						NewText[0] = 0;
						_tcsncat(NewText, sel, start);
						_tcscat(NewText, boo);
						_tcsncat(NewText, sel+end, slen-end);
						SendMessage(hwnd2, WM_SETTEXT, 0, (LPARAM)NewText);
						mir_free(NewText);
					}

					SendMessage(hwnd2, EM_SETSEL, dwStart, dwEnd);
				}
				mir_free(sel);
				mir_free(boo);
			}
		}
	}
}

int OnButtonPressed(WPARAM wParam, LPARAM lParam)
{
	CustomButtonClickData *cbcd = (CustomButtonClickData *)lParam;
	if (lstrcmpA(cbcd->pszModule, "SwitchLayout and Send") == 0)
	{
		HWND hEdit = GetDlgItem(cbcd->hwndFrom, IDC_MESSAGE);
		if (!hEdit) hEdit = GetDlgItem(cbcd->hwndFrom, IDC_CHATMESSAGE);

		BYTE byKeybState[256];
		GetKeyboardState(byKeybState);
		byKeybState[VK_CONTROL] = 128;
		SetKeyboardState(byKeybState);
		SendMessage(hEdit, WM_KEYDOWN, VK_UP, 0);
		byKeybState[VK_CONTROL] = 0;
		SetKeyboardState(byKeybState);

		DWORD dwProcessID;
		DWORD dwThreadID = GetWindowThreadProcessId(cbcd->hwndFrom, &dwProcessID);
		HKL hkl = GetKeyboardLayout(dwThreadID); 
					
		TCHAR *sel = Message_GetFromStream(hEdit, SF_TEXT|SF_UNICODE);
		int slen = lstrlen(sel);
				
		if (slen != 0)
		{
			ZeroMemory(&smgp, sizeof(smgp));
			smgp.cbSize = sizeof(smgp);
			smgp.str = sel;
			smgp.flag = SAFL_TCHAR;

			if (ServiceExists(MS_SMILEYADD_BATCHPARSE)) 
				smileyPrs = (SMADD_BATCHPARSERES *)CallService(MS_SMILEYADD_BATCHPARSE, 0, (LPARAM)&smgp);

			ActivateKeyboardLayout((HKL)HKL_PREV, KLF_ACTIVATE);

			for(int i = 0; i < slen; i++)
			{
				TCHAR tchr;
				BYTE keys[256] = {0};
				SHORT vks = VkKeyScanEx(sel[i], hkl);

				keys[VK_SHIFT]   = (HIBYTE(vks) & 1) ? 0xFF : 0x00; // shift
				keys[VK_CONTROL] = (HIBYTE(vks) & 2) ? 0xFF : 0x00; // ctrl
				keys[VK_MENU]    = (HIBYTE(vks) & 4) ? 0xFF : 0x00;	// alt
						
				if (!isItSmiley(i))
				{
					if(ToUnicodeEx(LOBYTE(vks), 0, keys, &tchr, 1, 0, GetKeyboardLayout(dwThreadID)) == 1)
						sel[i] = tchr;
				}
			}
			if (smileyPrs != NULL)
				CallService(MS_SMILEYADD_BATCHFREE, 0, (LPARAM)smileyPrs);

			DBVARIANT dbv = {0};
			DBGetContactSettingTString(NULL, "TranslitSwitcher", "ResendSymbol", &dbv);
			if (lstrcmp(dbv.ptszVal, NULL) == 0)
			{
				DBFreeVariant(&dbv);
				SendMessage(hEdit, WM_SETTEXT, 0, (LPARAM)sel);
				SendMessage(hEdit, EM_SETSEL, 0, slen);
				SendMessage(cbcd->hwndFrom, WM_COMMAND, IDOK, 0);
			}
			else
			{
				if (_tcsncmp(sel, dbv.ptszVal, _tcslen(dbv.ptszVal)) == 0)
				{
					SendMessage(hEdit, WM_SETTEXT, 0, (LPARAM)sel);
					SendMessage(hEdit, EM_SETSEL, 0, slen);
					SendMessage(cbcd->hwndFrom, WM_COMMAND, IDOK, 0);
				}
				else
				{
					int FinalLen = slen + SIZEOF(dbv.ptszVal) + 1;
					TCHAR* FinalString = (TCHAR*)mir_alloc((FinalLen+1)*sizeof(TCHAR));
					mir_sntprintf(FinalString, FinalLen, _T("%s %s"), dbv.ptszVal, sel);
					SendMessage(hEdit, WM_SETTEXT, 0, (LPARAM)FinalString);
					SendMessage(hEdit, EM_SETSEL, 0, FinalLen);
					SendMessage(cbcd->hwndFrom, WM_COMMAND, IDOK, 0);
					mir_free(FinalString);
				}
			}
		}
		mir_free(sel);
		return 1;
	}
	if (lstrcmpA(cbcd->pszModule, "Translit and Send") == 0)
	{
		HWND hEdit = GetDlgItem(cbcd->hwndFrom, IDC_MESSAGE);
		if (!hEdit) hEdit = GetDlgItem(cbcd->hwndFrom, IDC_CHATMESSAGE);

		BYTE byKeybState[256];
		GetKeyboardState(byKeybState);
		byKeybState[VK_CONTROL] = 128;
		SetKeyboardState(byKeybState);
		SendMessage(hEdit, WM_KEYDOWN, VK_UP, 0);
		byKeybState[VK_CONTROL] = 0;
		SetKeyboardState(byKeybState);

		TCHAR *sel = Message_GetFromStream(hEdit, SF_TEXT|SF_UNICODE);
		int slen = lstrlen(sel);
		if (slen != 0)
			Transliterate(sel);
		DBVARIANT dbv = {0};
		DBGetContactSettingTString(NULL, "TranslitSwitcher", "ResendSymbol", &dbv);
		if (lstrcmp(dbv.ptszVal, NULL) == 0)
		{
			DBFreeVariant(&dbv);
			SendMessage(hEdit, WM_SETTEXT, 0, (LPARAM)sel);
			SendMessage(hEdit, EM_SETSEL, 0, slen);
			SendMessage(cbcd->hwndFrom, WM_COMMAND, IDOK, 0);
		}
		else
		{
			if (_tcsncmp(sel, dbv.ptszVal, _tcslen(dbv.ptszVal)) == 0)
			{
				SendMessage(hEdit, WM_SETTEXT, 0, (LPARAM)sel);
				SendMessage(hEdit, EM_SETSEL, 0, slen);
				SendMessage(cbcd->hwndFrom, WM_COMMAND, IDOK, 0);
			}
			else
			{
				int FinalLen = lstrlen(sel) + SIZEOF(dbv.ptszVal) + 1;
				TCHAR* FinalString = (TCHAR*)mir_alloc((FinalLen+1)*sizeof(TCHAR));
				mir_sntprintf(FinalString, FinalLen, _T("%s %s"), dbv.ptszVal, sel);
				SendMessage(hEdit, WM_SETTEXT, 0, (LPARAM)FinalString);
				SendMessage(hEdit, EM_SETSEL, 0, FinalLen);
				SendMessage(cbcd->hwndFrom, WM_COMMAND, IDOK, 0);
				mir_free(FinalString);
			}
		}
		mir_free(sel);
		return 1;
	}
	if (lstrcmpA(cbcd->pszModule, "Invert Case and Send") == 0)
	{
		HWND hEdit = GetDlgItem(cbcd->hwndFrom, IDC_MESSAGE);
		if (!hEdit) hEdit = GetDlgItem(cbcd->hwndFrom, IDC_CHATMESSAGE);

		BYTE byKeybState[256];
		GetKeyboardState(byKeybState);
		byKeybState[VK_CONTROL] = 128;
		SetKeyboardState(byKeybState);
		SendMessage(hEdit, WM_KEYDOWN, VK_UP, 0);
		byKeybState[VK_CONTROL] = 0;
		SetKeyboardState(byKeybState);

		TCHAR *sel = Message_GetFromStream(hEdit, SF_TEXT|SF_UNICODE);
		int slen = lstrlen(sel);
		if (slen != 0)
			Invert(sel);
		DBVARIANT dbv = {0};
		DBGetContactSettingTString(NULL, "TranslitSwitcher", "ResendSymbol", &dbv);
		if (lstrcmp(dbv.ptszVal, NULL) == 0)
		{
			DBFreeVariant(&dbv);
			SendMessage(hEdit, WM_SETTEXT, 0, (LPARAM)sel);
			SendMessage(hEdit, EM_SETSEL, 0, slen);
			SendMessage(cbcd->hwndFrom, WM_COMMAND, IDOK, 0);
		}
		else
		{
			if (_tcsncmp(sel, dbv.ptszVal, _tcslen(dbv.ptszVal)) == 0)
			{
				SendMessage(hEdit, WM_SETTEXT, 0, (LPARAM)sel);
				SendMessage(hEdit, EM_SETSEL, 0, slen);
				SendMessage(cbcd->hwndFrom, WM_COMMAND, IDOK, 0);
			}
			else
			{
				int FinalLen = slen + SIZEOF(dbv.ptszVal) + 1;
				TCHAR* FinalString = (TCHAR*)mir_alloc((FinalLen+1)*sizeof(TCHAR));
				mir_sntprintf(FinalString, FinalLen, _T("%s %s"), dbv.ptszVal, sel);
				SendMessage(hEdit, WM_SETTEXT, 0, (LPARAM)FinalString);
				SendMessage(hEdit, EM_SETSEL, 0, FinalLen);
				SendMessage(cbcd->hwndFrom, WM_COMMAND, IDOK, 0);
				mir_free(FinalString);
			}
		}
		mir_free(sel);
		return 1;
	}
	return 0;
}