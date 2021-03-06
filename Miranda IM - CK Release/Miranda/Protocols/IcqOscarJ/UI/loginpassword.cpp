/*
ICQ plugin for
Miranda IM: the free IM client for Microsoft* Windows*

Authors
			Copyright � 2001-2004 Richard Hughes
			Copyright � 2002-2004 Martin �berg, Sam Kothari, Robert Rainwater
			Copyright � 2004-2010 Joe Kucera, Bio
			Copyright � 2010-2012 Borkra, Georg Hazan

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

* Implements Manage Server List dialog
*/

#include "../src/icqoscar.h"


INT_PTR CALLBACK LoginPasswdDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CIcqProto* ppro = (CIcqProto*)GetWindowLongPtr( hwndDlg, GWLP_USERDATA );

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		ppro = (CIcqProto*)lParam;
		SetWindowLongPtr( hwndDlg, GWLP_USERDATA, lParam );
		{
			SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)ppro->m_hIconProtocol->GetIcon(true));
			SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)ppro->m_hIconProtocol->GetIcon());

			DWORD dwUin = ppro->getContactUin(NULL);

			char pszUIN[MAX_PATH], str[MAX_PATH];
			null_snprintf(pszUIN, 128, ICQTranslateUtfStatic(LPGEN("Enter a password for UIN %u:"), str, MAX_PATH), dwUin);
			SetDlgItemTextUtf(hwndDlg, IDC_INSTRUCTION, pszUIN);

			SendDlgItemMessage(hwndDlg, IDC_LOGINPW, EM_LIMITTEXT, PASSWORDMAXLEN - 1, 0);

			CheckDlgButton(hwndDlg, IDC_SAVEPASS, ppro->getSettingByte(NULL, "RememberPass", 0));
		}
		break;

	case WM_DESTROY:
		ppro->m_hIconProtocol->ReleaseIcon(true);
		ppro->m_hIconProtocol->ReleaseIcon();
		break;

	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		break;

	case WM_COMMAND:
		{
			switch (LOWORD(wParam)) {
			case IDOK:
				ppro->m_bRememberPwd = (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SAVEPASS);
				ppro->setSettingByte(NULL, "RememberPass", ppro->m_bRememberPwd);

				GetDlgItemTextA(hwndDlg, IDC_LOGINPW, ppro->m_szPassword, sizeof(ppro->m_szPassword));

				ppro->icq_login(ppro->m_szPassword);

				EndDialog(hwndDlg, IDOK);
				break;

			case IDCANCEL:
				ppro->SetCurrentStatus(ID_STATUS_OFFLINE);
				EndDialog(hwndDlg, IDCANCEL);
				break;
			}
		}
		break;
	}

	return FALSE;
}

void CIcqProto::RequestPassword()
{
	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_LOGINPW), NULL, LoginPasswdDlgProc, LPARAM(this));
}
