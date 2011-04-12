// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
//
// Copyright � 2000,2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright � 2001,2002 Jon Keating, Richard Hughes
// Copyright � 2002,2003,2004 Martin �berg, Sam Kothari, Robert Rainwater
// Copyright � 2004,2005,2006,2007 Joe Kucera
// Copyright � 2006,2007 [sss], chaos.persei, [sin], Faith Healer, Theif, nullbie
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// -----------------------------------------------------------------------------
//
// File name      : $Source$
// Revision       : $Revision: 62 $
// Last change on : $Date: 2007-10-16 14:56:05 +0300 (Вт, 16 окт 2007) $
// Last change by : $Author: chaos.persei $
//
// DESCRIPTION:
//
//  Miranda init file - exported functions of plugin
//
// -----------------------------------------------------------------------------

#include "icqoscar.h"
#include "isee.h"

#include "m_updater.h"
#include "m_folders.h"

HINSTANCE hInst;
PLUGINLINK* pluginLink;
MM_INTERFACE mmi;
UTF8_INTERFACE utfi;
MD5_INTERFACE md5i;
LIST_INTERFACE li;

DWORD MIRANDA_VERSION;

HANDLE hExtraXStatus = NULL;

PLUGININFOEX pluginInfo =
{
	sizeof(PLUGININFOEX),
	"IcqOscarJ " ICQ_MODNAME " Mod Protocol",
	PLUGIN_MAKE_VERSION(1,0,0,69),
	NULL,
	"J.Kucera, Bio, M.berg, R.Hughes, Jon Keating, Faith, BM, S7, sss, cppexpert, chaos.persei, jarvis, ghazan, baloo, nullbie and etc",
	"cppexpert@mail.ru, sss123next@list.ru, chaos.persei@gmail.com, sin@miranda-me.org, jokusoftware@miranda-im.org",
	"(C) 2000-2011 M. berg, R.Hughes, J.Keating, Bio, J.Kucera, Angeli-Ka, Faith Healer, chaos.persei, Se7ven, BM, sss, cppexpert, [sin], nullbie, cppexpert and otc",
	"http://dev.mirandaim.ru/~sss/",
	UNICODE_AWARE,
	0,   //doesn't replace anything built-in
	{ 0xc78614bf, 0x878e, 0x43e4, { 0xa3, 0xf4, 0xa, 0xf9, 0x9a, 0xa5, 0x7a, 0x78 } }
};

// FIXME: Deprecated or duplicates ///////////////////////////////////////
DWORD _mirandaVersion;
extern char *date();
static BOOL bInited = FALSE;

extern "C" PLUGININFOEX __declspec(dllexport) *MirandaPluginInfoEx(DWORD mirandaVersion)
{
	_mirandaVersion = mirandaVersion;

	// Only load for 0.9.0.8 or greater
	// We need the new Unicode aware Contact Search API
	if (_mirandaVersion < PLUGIN_MAKE_VERSION(0, 9, 0, 8))
	{
		MessageBox( NULL, _T("ICQ plugin cannot be loaded. It requires Miranda IM 0.9.0.8 or later."), _T("ICQ Plugin"),
			MB_OK|MB_ICONWARNING|MB_SETFOREGROUND|MB_TOPMOST );
		return NULL;
	}
	else
	{
		// FIXME: Doesn't exists in official plug-in
		static char plugdesc[128];
		strcpy(plugdesc, "Support for ICQ network, enhanced, modified. [Build ");
		strcat(plugdesc, date());
		strcat(plugdesc, " ");
		strcat(plugdesc, __TIME__);
		strcat(plugdesc, "]");
		pluginInfo.description = plugdesc;
		
		// Are we running under Unicode Windows version ?
		gbUnicodeAPI_dep = (GetVersion() & 0x80000000) == 0;
		if (gbUnicodeAPI_dep)
			pluginInfo.flags = 1;
		
		MIRANDA_VERSION = mirandaVersion;
		bInited = TRUE;
	}

	return &pluginInfo;
}

static const MUUID interfaces[] = {MIID_PROTOCOL, MIID_LAST};
extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////

// FIXME: Deprecated or duplicates ///////////////////////////////////////
static int OnSystemModulesLoaded(WPARAM wParam,LPARAM lParam);
static int OnSystemPreShutdown(WPARAM wParam,LPARAM lParam);
static int OnContactSettingChanged(WPARAM wParam, LPARAM lParam);
CRITICAL_SECTION localSeqMutex;
CRITICAL_SECTION connectionHandleMutex;
HANDLE hsmsgrequest;
HANDLE hxstatuschanged;
HANDLE hxstatusiconchanged;
HANDLE hqipstatuschanged;
HANDLE hqipstatusiconchanged;
HANDLE hIconFolder = NULL;
HANDLE hHookIconsChanged = NULL;

// Declarations
void InitVars();
static HANDLE ICQCreateServiceFunction(const char* szService, MIRANDASERVICE serviceProc);
static HANDLE ICQCreateHookableEvent(const char* szEvent);
void RegEventType(int EventType, char* EventDescription);
extern void InitXStatusIcons();
extern void InitQipStatusIcons();
extern void InitTzersIcons();
extern void InitQipStatusEvents();
extern void icq_BuildPrivacyMenu();
static int IconLibIconsChanged(WPARAM wParam, LPARAM lParam);

extern "C" int __declspec(dllexport) Load(PLUGINLINK *link)
{
    pluginLink = link;
	mir_getLI(&li);
    mir_getMMI(&mmi);
	mir_getUTFI(&utfi);
    mir_getMD5I(&md5i);

	ghServerNetlibUser = NULL;

    // Are we running under Unicode Windows version ?
    gbUnicodeAPI_dep = (GetVersion() & 0x80000000) == 0;

    {
        // Are we running under unicode Miranda core ?
        char szVer[MAX_PATH];

        CallService(MS_SYSTEM_GETVERSIONTEXT, MAX_PATH, (LPARAM)szVer);
        _strlwr(szVer);	// make sure it is lowercase

        gbUnicodeCore_dep = (strstr(szVer, "unicode") != NULL);

        if (strstr(szVer, "alpha") != NULL)
            MIRANDA_VERSION |= 0x80000000;	// Are we running under Alpha Core
        else if (MIRANDA_VERSION >= 0x00050000 && strstr(szVer, "preview") == NULL)
            MIRANDA_VERSION &= 0xFFFFFF00;	// for Final Releases of Miranda 0.5+ clear build number

		// Check if _UNICODE matches Miranda's _UNICODE
#if defined( _UNICODE )
		if (strstr(szVer, "unicode") == NULL)
		{
			char szMsg[MAX_PATH], szCaption[100];

			MessageBoxUtf(NULL, ICQTranslateUtfStatic("You cannot use Unicode version of ICQ Protocol plug-in with Ansi version of Miranda IM.", szMsg, MAX_PATH),
				ICQTranslateUtfStatic("ICQ Plugin", szCaption, 100), MB_OK|MB_ICONWARNING|MB_SETFOREGROUND|MB_TOPMOST);
			return 1; // Failure
		}
#else
		if (strstrnull(szVer, "unicode") != NULL)
		{
			MessageBox(NULL, Translate("You cannot use Ansi version of ICQ Protocol plug-in with Unicode version of Miranda IM."), Translate("ICQ Plugin"),
				MB_OK|MB_ICONWARNING|MB_SETFOREGROUND|MB_TOPMOST);
			return 1; // Failure
		}
#endif

        /* WARNING!!! You CANNOT remove this code */
        if (strstr(szVer, "coffee") != NULL)
        {
            // We are running under damn violators
            void (*f)();

            MessageBoxA(0, "Running ICQJ+ Mod is forbidden under license violating products, sorry", "Warning!", MB_OK);

            f = NULL;
            f();
        }
        /* end of protected code */
    }

    srand(time(NULL));
    _tzset();

    ZeroMemory(gpszPassword, sizeof(gpszPassword));

	// Register the module
	PROTOCOLDESCRIPTOR pd = {0};
	pd.cbSize = sizeof(pd);
	pd.szName = ICQ_PROTOCOL_NAME;
	pd.type   = PROTOTYPE_PROTOCOL;
	//pd.fnInit   = icqProtoInit;
	//pd.fnUninit = icqProtoUninit;
	//pd.fnDestroy = icqProtoDestroy;
	CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM)&pd);

    HookEvent(ME_SYSTEM_MODULESLOADED, OnSystemModulesLoaded);
    HookEvent(ME_SYSTEM_PRESHUTDOWN, OnSystemPreShutdown);
    HookEvent(ME_DB_CONTACT_SETTINGCHANGED, OnContactSettingChanged);

    InitializeCriticalSection(&connectionHandleMutex);
    InitializeCriticalSection(&localSeqMutex);
    InitializeCriticalSection(&modeMsgsMutex);

    // Initialize core modules
    InitDB();       // DB interface
    InitVars(); // global variables
    InitCookies();  // cookie utils
    InitCache();    // contacts cache
    InitReguin();   // reguin module

    DBWriteContactSettingDword(NULL, ICQ_PROTOCOL_NAME, "SrvLastUpdate", 0);
    DBWriteContactSettingWord(NULL, ICQ_PROTOCOL_NAME, "SrvRecordCount", 0);

    InitRates();    // rate management

    // Initialize status message struct
    ZeroMemory(&modeMsgs, sizeof(icq_mode_messages));

    // Initialize temporary DB settings
    ICQCreateResidentSetting("Status"); // NOTE: XStatus cannot be temporary
    ICQCreateResidentSetting("ICQStatus");
    ICQCreateResidentSetting("TemporaryVisible");
    ICQCreateResidentSetting("TickTS");
    ICQCreateResidentSetting("IdleTS");
    ICQCreateResidentSetting("LogonTS");
    ICQCreateResidentSetting("CapBuf");
    ICQCreateResidentSetting("DCStatus");
    ICQCreateResidentSetting("TmpContact");

    // Setup services
    ICQCreateServiceFunction(PS_ICQP_SERVER_IGNORE, IcqServerIgnore);
    ICQCreateServiceFunction(PS_GETCAPS, IcqGetCaps);
    ICQCreateServiceFunction(PS_GETNAME, IcqGetName);
    ICQCreateServiceFunction(PS_LOADICON, IcqLoadIcon);
    ICQCreateServiceFunction(PS_SETSTATUS, IcqSetStatus);
    ICQCreateServiceFunction(PS_GETSTATUS, IcqGetStatus);
    ICQCreateServiceFunction(PS_SETAWAYMSG, IcqSetAwayMsg);
    ICQCreateServiceFunction(PS_AUTHALLOW, IcqAuthAllow);
    ICQCreateServiceFunction(PS_AUTHDENY, IcqAuthDeny);

#ifndef _08CORE
    if(_mirandaVersion >= PLUGIN_MAKE_VERSION(0,9,0,7))
        ICQCreateServiceFunction(PS_BASICSEARCHW, IcqBasicSearch_v9);
    else
#endif

	ICQCreateServiceFunction(PS_BASICSEARCH, IcqBasicSearch);

#ifndef _08CORE
    if(_mirandaVersion >= PLUGIN_MAKE_VERSION(0,9,0,7))
        ICQCreateServiceFunction(PS_SEARCHBYEMAILW, IcqSearchByEmail);
    else
#endif
    
	ICQCreateServiceFunction(PS_SEARCHBYEMAIL, IcqSearchByEmail);
    ICQCreateServiceFunction(MS_ICQ_SEARCHBYDETAILS, IcqSearchByDetails);
#ifndef _08CORE
    if(_mirandaVersion >= PLUGIN_MAKE_VERSION(0,9,0,7))
        ICQCreateServiceFunction(PS_SEARCHBYNAMEW, IcqSearchByDetails);
    else
#endif
    
	ICQCreateServiceFunction(PS_SEARCHBYNAME, IcqSearchByDetails);
    ICQCreateServiceFunction(PS_CREATEADVSEARCHUI, IcqCreateAdvSearchUI);
    ICQCreateServiceFunction(PS_SEARCHBYADVANCED, IcqSearchByAdvanced);
    ICQCreateServiceFunction(MS_ICQ_SENDSMS, IcqSendSms);
    ICQCreateServiceFunction(PS_ADDTOLIST, IcqAddToList);
    ICQCreateServiceFunction(PS_ADDTOLISTBYEVENT, IcqAddToListByEvent);
    ICQCreateServiceFunction(PS_FILERESUME, IcqFileResume);
    ICQCreateServiceFunction(PS_SET_NICKNAME, IcqSetNickName);
    ICQCreateServiceFunction(PSS_GETINFO, IcqGetInfo);
    ICQCreateServiceFunction(PSS_MESSAGE, IcqSendMessage);
    ICQCreateServiceFunction(PSS_URL, IcqSendUrl);
    ICQCreateServiceFunction(PSS_CONTACTS, IcqSendContacts);
    ICQCreateServiceFunction(PSS_SETAPPARENTMODE, IcqSetApparentMode);
    ICQCreateServiceFunction(PSS_GETAWAYMSG, IcqGetAwayMsg);
    ICQCreateServiceFunction(PSS_FILEALLOW, IcqFileAllow);
    ICQCreateServiceFunction(PSS_FILEDENY, IcqFileDeny);
    ICQCreateServiceFunction(PSS_FILECANCEL, IcqFileCancel);
    ICQCreateServiceFunction(PSS_FILE, IcqSendFile);
    ICQCreateServiceFunction(PSR_AWAYMSG, IcqRecvAwayMsg);
    ICQCreateServiceFunction(PSR_FILE, IcqRecvFile);
    ICQCreateServiceFunction(PSR_MESSAGE, IcqRecvMessage);
    ICQCreateServiceFunction(PSR_CONTACTS, IcqRecvContacts);
    ICQCreateServiceFunction(PSR_AUTH, IcqRecvAuth);
    ICQCreateServiceFunction(PSS_AUTHREQUEST, IcqSendAuthRequest);
    ICQCreateServiceFunction(PSS_ADDED, IcqSendYouWereAdded);
    ICQCreateServiceFunction(PSS_USERISTYPING, IcqSendUserIsTyping);
    ICQCreateServiceFunction(PS_CREATEACCMGRUI, SvcCreateAccMgrUI);
    // Session password API
    ICQCreateServiceFunction(PS_ICQ_SETPASSWORD, IcqSetPassword);
    // ChangeInfo API
    ICQCreateServiceFunction(PS_CHANGEINFOEX, IcqChangeInfoEx);
    // Avatar API
    ICQCreateServiceFunction(PS_GETAVATARINFO, IcqGetAvatarInfo);
    ICQCreateServiceFunction(PS_GETAVATARCAPS, IcqGetAvatarCaps);
    ICQCreateServiceFunction(PS_GETMYAVATAR, IcqGetMyAvatar);
    ICQCreateServiceFunction(PS_SETMYAVATAR, IcqSetMyAvatar);
    // Custom Status API
    ICQCreateServiceFunction(PS_ICQ_SETCUSTOMSTATUS, IcqSetXStatus); // obsolete (remove in next version)
    ICQCreateServiceFunction(PS_ICQ_GETCUSTOMSTATUS, IcqGetXStatus); // obsolete
    ICQCreateServiceFunction(PS_ICQ_SETCUSTOMSTATUSEX, IcqSetXStatusEx);
    ICQCreateServiceFunction(PS_ICQ_GETCUSTOMSTATUSEX, IcqGetXStatusEx);
    ICQCreateServiceFunction(PS_ICQ_GETCUSTOMSTATUSICON, IcqGetXStatusIcon);
    ICQCreateServiceFunction(PS_ICQ_REQUESTCUSTOMSTATUS, IcqRequestXStatusDetails);
    ICQCreateServiceFunction(PS_ICQ_GETADVANCEDSTATUSICON, IcqRequestAdvStatusIconIdx);
    // Custom caps
    ICQCreateServiceFunction(PS_ICQ_ADDCAPABILITY, IcqAddCapability);
    ICQCreateServiceFunction(PS_ICQ_CHECKCAPABILITY, IcqCheckCapability);



    hsmsgrequest = ICQCreateHookableEvent(ME_ICQ_STATUSMSGREQ);
    hxstatuschanged = ICQCreateHookableEvent(ME_ICQ_CUSTOMSTATUS_CHANGED);
    hxstatusiconchanged = ICQCreateHookableEvent(ME_ICQ_CUSTOMSTATUS_EXTRAICON_CHANGED);
    hqipstatuschanged = ICQCreateHookableEvent(ME_ICQ_CUSTOMSTATUS_CHANGED);//added
    hqipstatusiconchanged = ICQCreateHookableEvent(ME_ICQ_CUSTOMSTATUS_EXTRAICON_CHANGED);//added

    InitDirectConns();
    InitOscarFileTransfer();
    InitServerLists();
    icq_InitInfoUpdate();
    RegEventType(ICQEVENTTYPE_IGNORECHECK_STATUS, "Check ICQ Ignore State");
    RegEventType(ICQEVENTTYPE_CHECK_STATUS, "Check ICQ Status");
    RegEventType(ICQEVENTTYPE_CLIENT_CHANGE, "Chenge ICQ Client");
    RegEventType(ICQEVENTTYPE_SELF_REMOVE, "ICQ Contact SelfRemove");
    RegEventType(ICQEVENTTYPE_AUTH_DENIED, "ICQ Auth Denied");
    RegEventType(ICQEVENTTYPE_AUTH_GRANTED, "ICQ Auth Granted");
    RegEventType(ICQEVENTTYPE_AUTH_REQUESTED, "ICQ Auth Requested");
    RegEventType(ICQEVENTTYPE_YOU_ADDED, "ICQ You Added");
    RegEventType(ICQEVENTTYPE_WAS_FOUND, "Detected via ASD\\PSD");
    RegEventType(ICQEVENTTYPE_CHECK_XSTATUS, "Read X-Status");


    icq_InitISee();

    // Initialize charset conversion routines
    InitI18N();

    UpdateGlobalSettings();

    gnCurrentStatus = ID_STATUS_OFFLINE;

    hIconFolder = FoldersRegisterCustomPath(ICQ_PROTOCOL_NAME, "Icons", MIRANDA_PATH"\\icons");
    if( hIconFolder==(HANDLE)CALLSERVICE_NOTFOUND ) hIconFolder = 0;

    ICQCreateServiceFunction(MS_ICQ_ADDSERVCONTACT, IcqAddServerContact);

    ICQCreateServiceFunction(MS_REQ_AUTH, icq_RequestAuthorization);
    ICQCreateServiceFunction(MS_GRANT_AUTH, IcqGrantAuthorization);
    ICQCreateServiceFunction(MS_REVOKE_AUTH, IcqRevokeAuthorization);
    ICQCreateServiceFunction(MS_SETINVIS, IcqSetInvis);
    ICQCreateServiceFunction(MS_SETVIS, IcqSetVis);
    ICQCreateServiceFunction(MS_INCOGNITO_REQUEST, IncognitoAwayRequest);
    ICQCreateServiceFunction(MS_SEND_TZER, IcqSendtZer);
    ICQCreateServiceFunction(MS_TZER_DIALOG, IcqTzerDlg);
    ICQCreateServiceFunction(MS_ICQ_GET_USER_STATUS, IcqGetUserStatus_service);

    ICQCreateServiceFunction(MS_XSTATUS_SHOWDETAILS, IcqShowXStatusDetails);

    hHookIconsChanged = IconLibHookIconsChanged(IconLibIconsChanged);

    InitXStatusIcons();
    InitQipStatusIcons();
    InitTzersIcons();

    // This must be here - the events are called too early, WTF?
    InitXStatusEvents();
    InitQipStatusEvents();

    if (DBGetContactSettingByte(NULL, ICQ_PROTOCOL_NAME, "PrivacyMenu", DEFAULT_PRIVACY_ENABLED))
        icq_BuildPrivacyMenu();



    //Custom caps
    lstCustomCaps = li.List_Create(0,1);
    lstCustomCaps->sortFunc = NULL;

    {
        char tmp[MAXMODULELABELLENGTH];
        DBCONTACTENUMSETTINGS dbces;
        mir_snprintf(tmp, MAXMODULELABELLENGTH, "%sCaps", ICQ_PROTOCOL_NAME);
        dbces.pfnEnumProc = EnumCustomCapsProc;
        dbces.lParam = (LPARAM)tmp;
        dbces.szModule = tmp;
        CallService(MS_DB_CONTACT_ENUMSETTINGS, 0, (LPARAM)&dbces);
    }

    /*
      {
        ICQ_CUSTOMCAP icqCustomCap;
    	IcqBuildMirandaCap(&icqCustomCap, "Miranda ICQ Capability Test", NULL, "TestTest");
        CallProtoService(gpszICQProtoName, PS_ICQ_ADDCAPABILITY, 0, (LPARAM)&icqCustomCap);
      }
    */

    // Reset a bunch of session specific settings
    ResetSettingsOnLoad();

    return 0;
}

HANDLE hHookUserInfoInit = NULL;
HANDLE hHookOptionInit = NULL;
HANDLE hHookUserMenu = NULL;
HANDLE hHookIdleEvent = NULL;
static HANDLE hUserMenuAddServ = NULL;
static HANDLE hUserMenuAuth = NULL;
static HANDLE hUserMenuGrant = NULL;
static HANDLE hUserMenuRevoke = NULL;
static HANDLE hUserMenuXStatus = NULL;
HANDLE hIconProtocol = NULL;
static HANDLE hIconMenuAuth = NULL;
static HANDLE hIconMenuGrant = NULL;
static HANDLE hIconMenuRevoke = NULL;
static HANDLE hIconMenuAddServ = NULL;
static HANDLE hUserMenuSetVis = NULL;
static HANDLE hUserMenuSetInvis = NULL;
static HANDLE hUserMenuIncognito = NULL;
static HANDLE hUserMenuSendtZer = NULL;
HANDLE hPopUpsList = NULL;
HANDLE hUserMenuStatus = NULL;
HANDLE hExtraDC = NULL, hExtraqipstatus = NULL;
BOOL bAuthIgnore = FALSE;

extern HANDLE hServerConn;
//popup settings
BOOL bPopUpService = FALSE;
BOOL bPopUpsEnabled = FALSE;
BOOL bSpamPopUp = FALSE;
BOOL bUnknownPopUp = FALSE;
BOOL bFoundPopUp = TRUE;
BOOL bScanPopUp = TRUE;
BOOL bClientChangePopUp = FALSE;
BOOL bVisPopUp = TRUE;
BOOL bPopUpForNotOnList = FALSE;
BOOL bPopSelfRem = TRUE;
BOOL bIgnoreCheckPop = TRUE;
BOOL bInfoRequestPopUp = FALSE;
BOOL bAuthPopUp = FALSE;
BOOL bXUpdaterPopUp = FALSE;
BOOL bUinPopup = FALSE;
BOOL bPopupsForHidden = FALSE;
BOOL bPopupsForIgnored = FALSE;
BOOL bCloseWindowPopUp = TRUE;
BOOL bReadXStatusPopUp = FALSE;
//visibility variable
BYTE bVisibility = 0;
BOOL bIncognitoRequest = FALSE;
BOOL bIncognitoGlobal = FALSE;
BOOL bPrivacyMenuPlacement = FALSE;
BOOL bShowAuth = FALSE;
//BOOL bStealthRequest = FALSE;
BOOL bPSD = TRUE;
BOOL bExcludePSD = FALSE;
BOOL bNoPSDForHidden = TRUE;
BOOL bNoStatusReply = FALSE;
//xstatus settings
BOOL bXstatusIconShow = TRUE;
BOOL bQipstatusIconShow = FALSE;
BOOL bQipstatusEnable = FALSE;
//loging options
BOOL bLogSelfRemoveFile = FALSE;
BOOL bLogIgnoreCheckFile = FALSE;
BOOL bLogStatusCheckFile = FALSE;
BOOL bLogASDFile = FALSE;
BOOL bLogClientChangeFile = FALSE;
BOOL bLogAuthFile = FALSE;
BOOL bLogInfoRequestFile = FALSE;
BOOL bLogReadXStatusFile = FALSE;
BOOL bLogSelfRemoveHistory = FALSE;
BOOL bLogIgnoreCheckHistory = FALSE;
BOOL bLogStatusCheckHistory = FALSE;
BOOL bLogASDHistory = FALSE;
BOOL bLogClientChangeHistory = FALSE;
BOOL bLogAuthHistory = FALSE;
BOOL bLogInfoRequestHistory = FALSE;
BOOL bLogReadXStatusHistory = FALSE;
BOOL bHcontactHistory = FALSE;
//ASD settings
BYTE  gbASD = 0;
BOOL bNoASDInInvisible = TRUE;
BOOL bASDForOffline = TRUE;
BOOL bASDViaAwayMsg = FALSE;
BOOL bASDViaXtraz = FALSE;
BOOL bASDViaURL = FALSE;
BOOL bASDUnauthorized = FALSE; //work in progress
BOOL bASDViaAuth = FALSE;
//other settings
BOOL bTmpContacts = FALSE;
BOOL bTmpAuthRequet = TRUE;
BOOL bTmpSendAdded = TRUE;
BOOL gbSecureIM = FALSE, gbExtraIcons = FALSE;
char* TmpGroupName = 0;
BOOL bAddTemp = FALSE;
BOOL bServerAutoChange = TRUE;
WORD wClass;
char szHttpUserAgent[255];

extern int bHideXStatusUI;

extern void UninitQipStatusEvents();



SortedList *lstCustomCaps;


static char UnicodeAware[32];


static int OnModulesLoaded(WPARAM wParam, LPARAM lParam);
static int icq_PrebuildContactMenu(WPARAM wParam, LPARAM lParam);





static HANDLE ICQCreateServiceFunction(const char* szService, MIRANDASERVICE serviceProc)
{
    char str[MAX_PATH + 32];
    strcpy(str, ICQ_PROTOCOL_NAME);
    strcat(str, szService);
    return CreateServiceFunction(str, serviceProc);
}

static HANDLE ICQCreateHookableEvent(const char* szEvent)
{
    char str[MAX_PATH + 32];
    strcpy(str, ICQ_PROTOCOL_NAME);
    strcat(str, szEvent);
    return CreateHookableEvent(str);
}


int EnumCustomCapsProc(const char *szSetting,LPARAM lParam)
{
    DBVARIANT dbv;
    DBCONTACTGETSETTING dbcgs;
    dbcgs.szModule = (char *)lParam;
    dbcgs.szSetting = szSetting;
    dbcgs.pValue = &dbv;
    CallService(MS_DB_CONTACT_GETSETTING, 0, (LPARAM)&dbcgs);
    if (dbv.type == DBVT_BLOB)
    {
        ICQ_CUSTOMCAP icqCustomCap;
        icqCustomCap.cbSize = sizeof(icqCustomCap);
        strncpy(icqCustomCap.name, szSetting, 64);
        memcpy(icqCustomCap.caps, dbv.pbVal, min(0x10, dbv.cpbVal));
        CallProtoService(ICQ_PROTOCOL_NAME, PS_ICQ_ADDCAPABILITY, 0, (LPARAM)&icqCustomCap);
    }
    else if (dbv.type == DBVT_ASCIIZ)
    {
        ICQ_CUSTOMCAP icqCustomCap;
        icqCustomCap.cbSize = sizeof(icqCustomCap);
        strncpy(icqCustomCap.name, szSetting, 64);
        strncpy(icqCustomCap.caps, dbv.pszVal, 0x10);
        CallProtoService(ICQ_PROTOCOL_NAME, PS_ICQ_ADDCAPABILITY, 0, (LPARAM)&icqCustomCap);
    }
    CallService(MS_DB_CONTACT_FREEVARIANT, 0, (LPARAM)&dbv);
    return 0;
}

void RegEventType(int EventType, char* EventDescription)
{
    DBEVENTTYPEDESCR evt= {0};
    evt.cbSize=sizeof(evt);
    evt.module=ICQ_PROTOCOL_NAME;
    evt.eventType=EventType;
    evt.descr=EventDescription;
    CallService(MS_DB_EVENT_REGISTERTYPE, 0, (LPARAM)&evt);
}

void InitVars()
{
    bSpamPopUp = ICQGetContactSettingByte(NULL,"SpamPopUpEnabled",1);
    bUnknownPopUp = ICQGetContactSettingByte(NULL,"UnknownPopUpEnabled",1);
    bFoundPopUp = ICQGetContactSettingByte(NULL,"FoundPopUpEnabled",1);
    bScanPopUp = ICQGetContactSettingByte(NULL,"ScanPopUpEnabled",1);
    bVisPopUp = ICQGetContactSettingByte(NULL,"VisPopUpEnabled",1);
    bClientChangePopUp = ICQGetContactSettingByte(NULL,"ClientChangePopup",0);
    bPopUpForNotOnList = ICQGetContactSettingByte(NULL,"PopUpForNotOnList",0);
    bPopUpsEnabled = ICQGetContactSettingByte(NULL, "PopupsEnabled", 1);
    bXstatusIconShow = ICQGetContactSettingByte(NULL, "XStatusIconShow", 1);
    bQipstatusIconShow = ICQGetContactSettingByte(NULL, "QipStatusShow", 0);
    bAuthIgnore = ICQGetContactSettingByte(NULL, "AuthIgnore", 0);
    bIgnoreCheckPop = ICQGetContactSettingByte(NULL, "IgnoreCheckPop", 1);
    bPopSelfRem = ICQGetContactSettingByte(NULL, "PopSelfRem", 1);
    bInfoRequestPopUp = ICQGetContactSettingByte(NULL, "InfoRequestPopUp", 0);
    bAuthPopUp = ICQGetContactSettingByte(NULL, "AuthPopUp", 0);
    bPopupsForIgnored = ICQGetContactSettingByte(NULL, "PopUpForIgnored", 0);
    bPopupsForHidden = ICQGetContactSettingByte(NULL, "PopUpForHidden", 0);
    bXUpdaterPopUp = ICQGetContactSettingByte(NULL, "XUpdaterPopUp", 0);
    bCloseWindowPopUp = ICQGetContactSettingByte(NULL, "CloseWindow", 1);
    bPSD = ICQGetContactSettingByte(NULL, "PSD", 1);
    bNoASDInInvisible = ICQGetContactSettingByte(NULL, "NoASDInInvisible", 1);
    bASDForOffline = ICQGetContactSettingByte(NULL, "ASDForOffline", 1);
    bLogSelfRemoveFile = ICQGetContactSettingByte(NULL, "LogSelfRemoveFile", 0);
    bLogIgnoreCheckFile = ICQGetContactSettingByte(NULL, "LogIgnoreCheckFile", 0);
    bLogStatusCheckFile = ICQGetContactSettingByte(NULL, "LogStatusCheckFile", 0);
    bLogASDFile = ICQGetContactSettingByte(NULL, "LogASDFile", 0);
    bLogClientChangeFile = ICQGetContactSettingByte(NULL, "LogClientChangeFile", 0);
    bLogAuthFile = ICQGetContactSettingByte(NULL, "LogAuthFile", 0);
    bLogInfoRequestFile = ICQGetContactSettingByte(NULL, "LogRequestFile", 0);
    bLogReadXStatusFile = ICQGetContactSettingByte(NULL, "LogReadXStatusFile", 0);
    bLogSelfRemoveHistory = ICQGetContactSettingByte(NULL, "LogSelfRemoveHistory", 0);
    bLogIgnoreCheckHistory = ICQGetContactSettingByte(NULL, "LogIgnoreCheckHistory", 0);
    bLogStatusCheckHistory = ICQGetContactSettingByte(NULL, "LogStatusCheckHistory", 0);
    bLogASDHistory = ICQGetContactSettingByte(NULL, "LogASDHistory", 0);
    bLogClientChangeHistory = ICQGetContactSettingByte(NULL, "LogClientChangeHistory", 0);
    bLogAuthHistory = ICQGetContactSettingByte(NULL, "LogAuthHistory", 0);
    bLogInfoRequestHistory = ICQGetContactSettingByte(NULL, "LogRequestHistory", 0);
    bLogReadXStatusHistory = ICQGetContactSettingByte(NULL, "LogReadXStatusHistory", 0);
    bHcontactHistory = ICQGetContactSettingByte(NULL,"LogToHcontact",0);
    bTmpContacts = ICQGetContactSettingByte(NULL, "TempContacts", 0);
    TmpGroupName = UniGetContactSettingUtf(NULL,ICQ_PROTOCOL_NAME,"TmpContactsGroup", Translate("General"));
    bAddTemp = ICQGetContactSettingByte(NULL, "AddTemp", 0);
    bTmpAuthRequet = ICQGetContactSettingByte(NULL, "TmpReqAuth", 1);
    bTmpSendAdded = ICQGetContactSettingByte(NULL, "TmpSndAdded", 1);
    bNoStatusReply = ICQGetContactSettingByte(NULL,"NoStatusReply", 0);
    bServerAutoChange = ICQGetContactSettingByte(NULL,"ServerAutoChange", 1);
    bIncognitoGlobal = ICQGetContactSettingByte(NULL, "IncognitoGlobal", 0);
    bPrivacyMenuPlacement = ICQGetContactSettingByte(NULL,"PrivacyPlacement", 1);
    bShowAuth = ICQGetContactSettingByte(NULL, "ShowAuth", 0);
    bUinPopup = ICQGetContactSettingByte(NULL, "UinPopup", 0);
    bReadXStatusPopUp = ICQGetContactSettingByte(NULL, "ReadXStatusPopUp", 0);
    bNoPSDForHidden = ICQGetContactSettingByte(NULL, "NoPSDForHidden", 1);
    gbASD = ICQGetContactSettingByte(NULL, "ASD", 0);
    bASDViaAwayMsg = ICQGetContactSettingByte(NULL, "bASDViaAwayMsg", 0);
    bASDViaXtraz = ICQGetContactSettingByte(NULL, "bASDViaXtraz", 0);
    bASDViaURL = ICQGetContactSettingByte(NULL, "bASDViaURL", 0);
    bASDUnauthorized = ICQGetContactSettingByte(NULL, "bASDUnauthorized", 0);
    bASDViaAuth = ICQGetContactSettingByte(NULL, "bASDViaAuth", 0);
    gbWebAware = ICQGetContactSettingByte(NULL, "WebAware", 0);
    strcpy(szHttpUserAgent, ICQGetContactSettingUtf(NULL, "HttpUserAgent", "Mozilla/4.08 [en] (WinNT; U ;Nav)"));
    m_bSecureConnection = ICQGetContactSettingByte(NULL, "SecureConnection", DEFAULT_SECURE_CONNECTION);
    gbCustomCapEnabled = ICQGetContactSettingByte(NULL, "customcap", 1);
    gbHideIdEnabled = ICQGetContactSettingByte(NULL, "Hide ID", 1);
    gbRTFEnabled = ICQGetContactSettingByte(NULL, "RTF", 0);
    gbVerEnabled = ICQGetContactSettingByte(NULL, "CurrentVer", 0);
    gbTzerEnabled = ICQGetContactSettingByte(NULL, "tZer", 0);
    gbQipStatusEnabled = ICQGetContactSettingByte(NULL, "QipStatusEnable", 0);
//  if(ICQGetContactSettingByte(NULL, "StealthRequest", 0) == 1)
//	  bStealthRequest = TRUE;
}


extern void icq_DestroyPrivacyMenu();

extern "C" int __declspec(dllexport) Unload(void)
{
    if (gbXStatusEnabled) gbXStatusEnabled = 10; // block clist changing

    UninitXStatusEvents();
    UninitQipStatusEvents();

    if (hServerConn)
    {
        icq_sendCloseConnection();

        icq_serverDisconnect(TRUE);
    }

    UninitServerLists();
    UninitOscarFileTransfer();
    UninitDirectConns();
    icq_InfoUpdateCleanup();
    icq_ISeeCleanup();
    icq_DestroyPrivacyMenu();

    NetLib_SafeCloseHandle(&ghDirectNetlibUser);
    NetLib_SafeCloseHandle(&ghServerNetlibUser);
    UninitRates();
    UninitCookies();
    UninitCache();
    DeleteCriticalSection(&modeMsgsMutex);
    DeleteCriticalSection(&localSeqMutex);
    DeleteCriticalSection(&connectionHandleMutex);
    mir_free(modeMsgs.szOffline);
    mir_free(modeMsgs.szOnline);
    mir_free(modeMsgs.szAway);
    mir_free(modeMsgs.szNa);
    mir_free(modeMsgs.szOccupied);
    mir_free(modeMsgs.szDnd);
    mir_free(modeMsgs.szFfc);

    if (hHookIconsChanged)
        UnhookEvent(hHookIconsChanged);

    if (hHookUserInfoInit)
        UnhookEvent(hHookUserInfoInit);

    if (hHookOptionInit)
        UnhookEvent(hHookOptionInit);

    if (hsmsgrequest)
        DestroyHookableEvent(hsmsgrequest);

    if (hxstatuschanged)
        DestroyHookableEvent(hxstatuschanged);

    if (hxstatusiconchanged)
        DestroyHookableEvent(hxstatusiconchanged);

    if (hqipstatuschanged)
        DestroyHookableEvent(hqipstatuschanged);
    if (hqipstatusiconchanged)
        DestroyHookableEvent(hqipstatusiconchanged);
    if (hHookUserMenu)
        UnhookEvent(hHookUserMenu);

    if (hHookIdleEvent)
        UnhookEvent(hHookIdleEvent);

    bVisibility = ICQGetContactSettingByte(NULL, "SrvVisibility", 0);

    return 0;
}






static int OnSystemModulesLoaded(WPARAM wParam,LPARAM lParam)
{
    NETLIBUSER nlu = {0};
    char pszP2PName[MAX_PATH+3];
    char pszGroupsName[MAX_PATH+10];
    char pszSrvGroupsName[MAX_PATH+10];
    char pszCapsName[MAX_PATH+10];
    char szBuffer[MAX_PATH+64];
    char* modules[6] = {0,0,0,0,0,0};

    /* if(ServiceExists(MS_ASSOCMGR_ADDNEWFILETYPE))  //need to be finishaed
     {
      {
    	  FILETYPEDESC ftd;
    	  ftd.cbSize=sizeof(ftd);
    	  ftd.pwszDescription=TranslateW("ICQ link file");
    	  ftd.pszDescription=Translate("ICQ link file");
    	  ftd.ptszDescription=TranslateT("ICQ link file");
    	  ftd.pszFileExt=".icq";
    	  ftd.pszMimeType="application/x-icq";
    	  ftd.pszService=0;
    	  ftd.flags=FTDF_DEFAULTDISABLED;
    	  CallService(MS_ASSOCMGR_ADDNEWFILETYPE,0,(LPARAM)&ftd);
      }
      {
    	  URLTYPEDESC utd;
    	  utd.cbSize=sizeof(utd);
    	  utd.pszDescription=Translate("ICQ Link");
    	  utd.ptszDescription=TranslateT("ICQ Link");
    	  utd.pwszDescription=TranslateW("ICQ Link");
    	  utd.flags=UTDF_DEFAULTDISABLED;
    	  utd.pszService=0;
    	  utd.pszProtoPrefix="http:";
    	  CallService(MS_ASSOCMGR_ADDNEWURLTYPE,0,(LPARAM)&utd);
      }
     }*/


    strcpy(pszP2PName, ICQ_PROTOCOL_NAME);
    strcat(pszP2PName, "P2P");

    strcpy(pszGroupsName, ICQ_PROTOCOL_NAME);
    strcat(pszGroupsName, "Groups");
    strcpy(pszSrvGroupsName, ICQ_PROTOCOL_NAME);
    strcat(pszSrvGroupsName, "SrvGroups");
    strcpy(pszCapsName, ICQ_PROTOCOL_NAME);
    strcat(pszCapsName, "Caps");
    modules[0] = ICQ_PROTOCOL_NAME;
    modules[1] = pszP2PName;
    modules[2] = pszGroupsName;
    modules[3] = pszSrvGroupsName;
    modules[4] = pszCapsName;
    CallService("DBEditorpp/RegisterModule",(WPARAM)modules,(LPARAM)5);


    null_snprintf(szBuffer, sizeof szBuffer, ICQTranslate("%s server connection"), ICQ_PROTOCOL_NAME);
    nlu.cbSize = sizeof(nlu);
//  nlu.flags = NUF_OUTGOING | NUF_HTTPGATEWAY;
    nlu.flags = NUF_OUTGOING | NUF_HTTPCONNS;
    nlu.szDescriptiveName = szBuffer;
    nlu.szSettingsModule = ICQ_PROTOCOL_NAME;
    nlu.szHttpGatewayHello = "http://http.proxy.icq.com/hello";
    nlu.szHttpGatewayUserAgent = szHttpUserAgent;
    nlu.pfnHttpGatewayInit = icq_httpGatewayInit;
    nlu.pfnHttpGatewayBegin = icq_httpGatewayBegin;
    nlu.pfnHttpGatewayWrapSend = icq_httpGatewayWrapSend;
    nlu.pfnHttpGatewayUnwrapRecv = icq_httpGatewayUnwrapRecv;

    ghServerNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);

    null_snprintf(szBuffer, sizeof szBuffer, ICQTranslate("%s client-to-client connections"), ICQ_PROTOCOL_NAME);
    nlu.flags = NUF_OUTGOING | NUF_INCOMING;
    nlu.szDescriptiveName = szBuffer;
    nlu.szSettingsModule = pszP2PName;
    nlu.minIncomingPorts = 1;
    ghDirectNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);

    hHookOptionInit = HookEvent(ME_OPT_INITIALISE, IcqOptInit);
    hHookUserInfoInit = HookEvent(ME_USERINFO_INITIALISE, OnDetailsInit);
	hHookUserMenu  = HookEvent(ME_CLIST_PREBUILDCONTACTMENU, icq_PrebuildContactMenu);
	hHookIdleEvent = HookEvent(ME_IDLE_CHANGED, IcqIdleChanged);

    hPopUpsList = (HANDLE)CallService(MS_UTILS_ALLOCWINDOWLIST,0,0);

    icq_FirstRunCheck();

    InitAvatars();

    // Init extra optional modules
    //InitPopUps();
    InitIconLib();

    hHookIconsChanged = IconLibHookIconsChanged(IconLibIconsChanged);
    // Initialize IconLib icons
    InitXStatusIcons();
    InitQipStatusIcons();
    InitXStatusEvents();
    InitXStatusItems(FALSE);

    InitDCEvents();
    ICQCreateThread(icq_XStatusUpdaterThread, NULL);

    {
        // userinfo
        ICQWriteContactSettingUtf(NULL, "MirVer", Translate("You need to be connected"));
        DBWriteContactSettingDword(NULL, ICQ_PROTOCOL_NAME, "IP", 0);
        if(!DBGetContactSettingByte(NULL, ICQ_PROTOCOL_NAME, "ConstRealIP", 0))
            DBWriteContactSettingDword(NULL, ICQ_PROTOCOL_NAME, "RealIP", 0);
        DBWriteContactSettingWord(NULL, ICQ_PROTOCOL_NAME, "UserPort", 0);
        DBWriteContactSettingDword(NULL, ICQ_PROTOCOL_NAME, "LogonTS", 0);
        DBWriteContactSettingDword(NULL, ICQ_PROTOCOL_NAME, "IdleTS", 0);
        DBWriteContactSettingDword(NULL, ICQ_PROTOCOL_NAME, "Status", ID_STATUS_OFFLINE);
    }
    {
        CLISTMENUITEM mi;
        char pszServiceName[MAX_PATH+30];

        strcpy(pszServiceName, ICQ_PROTOCOL_NAME);
        strcat(pszServiceName, MS_REQ_AUTH);

        ZeroMemory(&mi, sizeof(mi));
        mi.cbSize = sizeof(mi);
        mi.position = 1000030000;
        mi.hIcon = IconLibGetIcon("req_auth");
        mi.pszContactOwner = ICQ_PROTOCOL_NAME;
        mi.pszName = LPGEN("Request authorization");
        mi.pszService = pszServiceName;
        hUserMenuAuth = (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);
        IconLibReleaseIcon("req_auth");

        strcpy(pszServiceName, ICQ_PROTOCOL_NAME);
        strcat(pszServiceName, MS_GRANT_AUTH);

        ZeroMemory(&mi, sizeof(mi));
        mi.cbSize = sizeof(mi);
        mi.position = 1000029999;
        mi.hIcon = IconLibGetIcon("grant_auth");
        mi.pszContactOwner = ICQ_PROTOCOL_NAME;
        mi.pszName = LPGEN("Grant authorization");
        mi.pszService = pszServiceName;
        hUserMenuGrant = (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);
        IconLibReleaseIcon("grant_auth");

        strcpy(pszServiceName, ICQ_PROTOCOL_NAME);
        strcat(pszServiceName, MS_REVOKE_AUTH);
        ZeroMemory(&mi, sizeof(mi));
        mi.cbSize = sizeof(mi);
        mi.position = 1000029998;
        mi.hIcon = IconLibGetIcon("revoke_auth");
        mi.pszContactOwner = ICQ_PROTOCOL_NAME;
        mi.pszName = LPGEN("Revoke authorization");
        mi.pszService = pszServiceName;
        hUserMenuRevoke = (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);
        IconLibReleaseIcon("revoke_auth");


        strcpy(pszServiceName, ICQ_PROTOCOL_NAME);
        strcat(pszServiceName, MS_ICQ_ADDSERVCONTACT);

        ZeroMemory(&mi, sizeof(mi));
        mi.cbSize = sizeof(mi);
        mi.position = -2049999999;
        mi.hIcon = IconLibGetIcon("add_to_server");
        mi.pszContactOwner = ICQ_PROTOCOL_NAME;
        mi.pszName = LPGEN("Add to server list");
        mi.pszService = pszServiceName;
        hUserMenuAddServ = (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);
        IconLibReleaseIcon("add_to_server");

        strcpy(pszServiceName, ICQ_PROTOCOL_NAME);
        strcat(pszServiceName, MS_XSTATUS_SHOWDETAILS);

        ZeroMemory(&mi, sizeof(mi));
        mi.cbSize = sizeof(mi);
        mi.position = -2000004999;
        mi.hIcon = NULL; // dynamically updated
        mi.pszContactOwner = ICQ_PROTOCOL_NAME;
        mi.pszName = LPGEN("Show custom status details");
        mi.pszService = pszServiceName;
        mi.flags=CMIF_NOTOFFLINE;
        hUserMenuXStatus = (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);


        strcpy(pszServiceName, ICQ_PROTOCOL_NAME);
        strcat(pszServiceName, MS_SETVIS);

        ZeroMemory(&mi, sizeof(mi));
        mi.cbSize = sizeof(mi);
        mi.position = 2106000000;
        mi.flags = 0;
        mi.hIcon = IconLibGetIcon("set_vis");
        mi.pszContactOwner = ICQ_PROTOCOL_NAME;
        mi.pszName = LPGEN("Always visible");
        mi.pszService = pszServiceName;
        hUserMenuSetVis=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi);
        IconLibReleaseIcon("set_vis");

        strcpy(pszServiceName, ICQ_PROTOCOL_NAME);
        strcat(pszServiceName, MS_SETINVIS);

        ZeroMemory(&mi, sizeof(mi));
        mi.cbSize = sizeof(mi);
        mi.position = 2107000000;
        mi.flags = 0;
        mi.hIcon = IconLibGetIcon("set_invis");
        mi.pszContactOwner = ICQ_PROTOCOL_NAME;
        mi.pszName = LPGEN("Always invisible");
        mi.pszService = pszServiceName;
        hUserMenuSetInvis=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi);
        IconLibReleaseIcon("set_invis");


        strcpy(pszServiceName, ICQ_PROTOCOL_NAME);
        strcat(pszServiceName, MS_INCOGNITO_REQUEST);

        ZeroMemory(&mi, sizeof(mi));
        mi.cbSize = sizeof(mi);
        mi.position = -2000005000;
        mi.flags = 0;
        mi.hIcon = IconLibGetIcon("incognito_request");
        mi.pszContactOwner = ICQ_PROTOCOL_NAME;
        mi.pszName = LPGEN("Incognito Away-Request");
        mi.pszService = pszServiceName;
        hUserMenuIncognito=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi);
        IconLibReleaseIcon("incognito_request");


        strcpy(pszServiceName, ICQ_PROTOCOL_NAME);
        strcat(pszServiceName, MS_TZER_DIALOG);

        ZeroMemory(&mi, sizeof(mi));
        mi.cbSize = sizeof(mi);
        mi.position = 2108000000;
        mi.flags = 0;
        mi.hIcon = IconLibGetIcon("send_tzer");
        mi.pszContactOwner = ICQ_PROTOCOL_NAME;
        mi.pszName = LPGEN("Send tZer");
        mi.pszService = pszServiceName;
        hUserMenuSendtZer=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi);
        IconLibReleaseIcon("send_tzer");

        strcpy(pszServiceName, ICQ_PROTOCOL_NAME);
        strcat(pszServiceName, PS_ICQP_SERVER_IGNORE);

        ZeroMemory(&mi, sizeof(mi));
        mi.cbSize = sizeof(mi);
        mi.position = -2000005100;
        mi.flags = 0;
        mi.hIcon = IconLibGetIcon("server_ignore");
        mi.pszContactOwner = ICQ_PROTOCOL_NAME;
        mi.pszName = LPGEN("Server ignore");
        mi.pszService = pszServiceName;
        hUserMenuSendtZer=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi);
        IconLibReleaseIcon("server_ignore");

    }
    if (ServiceExists(MS_POPUP_ADDPOPUPEX))
        bPopUpService = TRUE;
    if (ServiceExists("SecureIM/IsContactSecured"))
        gbSecureIM = TRUE;
    if(ServiceExists(MS_EXTRAICON_REGISTER))
    {
        EXTRAICON_INFO ico = {0};
        gbExtraIcons = TRUE;
        ico.type = EXTRAICON_TYPE_ICOLIB;
        ico.cbSize=sizeof(ico);
        ico.name="xstatus";
        ico.description= "XStatus";
        hExtraXStatus=(HANDLE)CallService(MS_EXTRAICON_REGISTER, (WPARAM)&ico, 0);
        ZeroMemory(&ico,sizeof(ico));
        ico.type = EXTRAICON_TYPE_ICOLIB;
        ico.cbSize=sizeof(ico);
        ico.name="icq_dc";
        ico.description="ICQ DC";
        hExtraDC=(HANDLE)CallService(MS_EXTRAICON_REGISTER, (WPARAM)&ico, 0);
        ZeroMemory(&ico,sizeof(ico));
        ico.type = EXTRAICON_TYPE_ICOLIB;
        ico.cbSize=sizeof(ico);
        ico.name="icq_qip_status";
        ico.description="ICQ qip status";
        hExtraqipstatus=(HANDLE)CallService(MS_EXTRAICON_REGISTER, (WPARAM)&ico, 0);
    }

    if(ServiceExists(MS_UPDATE_REGISTER))
    {
        Update upd = {0};
        char szCurrentVersion[30];

        upd.cbSize = sizeof(upd);
        upd.szComponentName = "IcqOscarJ (" ICQ_MODNAME " Mod) Protocol";
        upd.pbVersion = (BYTE*)CreateVersionString(pluginInfo.version, szCurrentVersion);
        upd.cpbVersion = strlen(( char *)upd.pbVersion);

        upd.szBetaUpdateURL = upd.szUpdateURL = "http://sss.chaoslab.ru/icq.zip";
        upd.szBetaVersionURL = upd.szVersionURL = "http://sss.chaoslab.ru/icqpluscurrent";
        upd.pbBetaVersionPrefix = upd.pbVersionPrefix = (BYTE*) "icq ";	// FIXME
        upd.cpbBetaVersionPrefix = upd.cpbVersionPrefix = strlen(( char *)upd.pbVersionPrefix);
        //	  upd.szBetaChangelogURL = "http://dev.mirandaim.ru/~sss/get.php?file=icq_test.txt";

        CallService(MS_UPDATE_REGISTER, 0, (LPARAM)&upd);
    }


    return 0;
}


static int OnSystemPreShutdown(WPARAM wParam,LPARAM lParam)
{
    // all threads should be terminated here
    if (hServerConn)
    {
        icq_sendCloseConnection();

        icq_serverDisconnect(TRUE);
    }
    CheckSelfRemoveShutdown();

    return 0;
}



static int OnContactSettingChanged(WPARAM wParam,LPARAM lParam)
{
    DBCONTACTWRITESETTING *cws = ( DBCONTACTWRITESETTING* )lParam;
    HANDLE hCSCContact = ( HANDLE )wParam;

    if( hCSCContact == NULL || lstrcmpA( cws->szSetting, "Status" ) )
        return 0;
    if ( cws->value.wVal == ID_STATUS_OFFLINE )
    {
        // if contact goes offline, delete xstatus details
        DBDeleteContactSetting( hCSCContact, ICQ_PROTOCOL_NAME, "XStatusId" );
        DBDeleteContactSetting( hCSCContact, ICQ_PROTOCOL_NAME, "XStatusMsg" );
        DBDeleteContactSetting( hCSCContact, ICQ_PROTOCOL_NAME, "XStatusName" );
    }
    return 0;
}



void CListShowMenuItem(HANDLE hMenuItem, BYTE bShow)
{
    CLISTMENUITEM mi = {0};

    mi.cbSize = sizeof(mi);
    if (bShow)
        mi.flags = CMIM_FLAGS;
    else
        mi.flags = CMIM_FLAGS | CMIF_HIDDEN;

    CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuItem, (LPARAM)&mi);
}



void CListSetMenuItemIcon(HANDLE hMenuItem, HICON hIcon)
{
    CLISTMENUITEM mi = {0};

    mi.cbSize = sizeof(mi);
    mi.flags = CMIM_FLAGS | CMIM_ICON;

    mi.hIcon = hIcon;
    CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuItem, (LPARAM)&mi);
}



static int icq_PrebuildContactMenu(WPARAM wParam, LPARAM lParam)
{
    BYTE bXStatus;
    CLISTMENUITEM cli= {0};
    extern HANDLE hStatusMenu;

    CListShowMenuItem(hUserMenuAuth, bShowAuth?1:ICQGetContactSettingByte((HANDLE)wParam, "Auth", 0) && icqOnline);
    CListShowMenuItem(hUserMenuGrant, bShowAuth?1:ICQGetContactSettingByte((HANDLE)wParam, "Grant", 0) && icqOnline);
    CListShowMenuItem(hUserMenuRevoke, bShowAuth?1:!ICQGetContactSettingByte((HANDLE)wParam, "Grant", 0) && icqOnline);
    CListShowMenuItem(hUserMenuSetVis, icqOnline);
    CListShowMenuItem(hUserMenuSetInvis, icqOnline);
    CListShowMenuItem(hUserMenuStatus, icqOnline && gbASD);
//  CListShowMenuItem(hStatusMenu, icqOnline && gbASD); //not woking ? %), looks like we need other method
    CListShowMenuItem(hUserMenuIncognito, (/*bStealthRequest && */!bIncognitoGlobal &&
                      (ICQGetContactSettingWord((HANDLE)wParam, "Status", 0)!= ID_STATUS_OFFLINE) &&
                      (ICQGetContactSettingWord((HANDLE)wParam, "Status", 0)!= ID_STATUS_INVISIBLE)  && icqOnline)); //now away messages for online supported
    CListShowMenuItem(hUserMenuSendtZer, icqOnline && gbTzerEnabled);

    cli.cbSize=sizeof(CLISTMENUITEM);
    cli.flags=CMIM_FLAGS;
    cli.hIcon=NULL;
    cli.pszContactOwner=NULL;

    switch(ICQGetContactSettingWord((HANDLE)wParam, "ApparentMode",0))
    {
    case ID_STATUS_ONLINE:
        cli.flags|=CMIF_CHECKED;
        CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)hUserMenuSetVis,(LPARAM)&cli);
        break;

    case ID_STATUS_OFFLINE:
        cli.flags|=CMIF_CHECKED;
        CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)hUserMenuSetInvis,(LPARAM)&cli);
        break;

    default:
        break;
    }


    if (gbSsiEnabled && !ICQGetContactSettingWord((HANDLE)wParam, "ServerId", 0) && !ICQGetContactSettingWord((HANDLE)wParam, "SrvIgnoreId", 0))
        CListShowMenuItem(hUserMenuAddServ, 1);
    else
        CListShowMenuItem(hUserMenuAddServ, 0);

    bXStatus = ICQGetContactXStatus((HANDLE)wParam);
    CListShowMenuItem(hUserMenuXStatus, (BYTE)(bHideXStatusUI ? 0 : bXStatus));
    if (bXStatus && !bHideXStatusUI)
    {
        CListSetMenuItemIcon(hUserMenuXStatus, GetXStatusIcon(bXStatus, LR_SHARED));
    }

    return 0;
}



static int IconLibIconsChanged(WPARAM wParam, LPARAM lParam)
{
    CListSetMenuItemIcon(hUserMenuAuth, IconLibGetIcon("req_auth"));
    IconLibReleaseIcon("req_auth");
    CListSetMenuItemIcon(hUserMenuGrant, IconLibGetIcon("grant_auth"));
    IconLibReleaseIcon("grant_auth");
    CListSetMenuItemIcon(hUserMenuRevoke, IconLibGetIcon("revoke_auth"));
    IconLibReleaseIcon("revoke_auth");
    CListSetMenuItemIcon(hUserMenuAddServ, IconLibGetIcon("add_to_server"));
    IconLibReleaseIcon("add_to_server");
    CListSetMenuItemIcon(hUserMenuIncognito, IconLibGetIcon("incognito_request"));
    IconLibReleaseIcon("incognito_request");
    CListSetMenuItemIcon(hUserMenuSendtZer, IconLibGetIcon("send_tzer"));
    IconLibReleaseIcon("send_tzer");

    ChangedIconsXStatus();

    return 0;
}



void UpdateGlobalSettings()
{
    if (ghServerNetlibUser)
    {
        NETLIBUSERSETTINGS nlus = {0};

        nlus.cbSize = sizeof(NETLIBUSERSETTINGS);
        if (CallService(MS_NETLIB_GETUSERSETTINGS, (WPARAM)ghServerNetlibUser, (LPARAM)&nlus))
        {
            if (nlus.useProxy && nlus.proxyType == PROXYTYPE_HTTP)
                gbGatewayMode = 1;
            else
                gbGatewayMode = 0;
        }
        else
            gbGatewayMode = 0;
    }

    gbSecureLogin = ICQGetContactSettingByte(NULL, "SecureLogin", DEFAULT_SECURE_LOGIN);
    gbAimEnabled = ICQGetContactSettingByte(NULL, "AimEnabled", DEFAULT_AIM_ENABLED);
    gbUtfEnabled = ICQGetContactSettingByte(NULL, "UtfEnabled", DEFAULT_UTF_ENABLED);
    gwAnsiCodepage = ICQGetContactSettingWord(NULL, "AnsiCodePage", DEFAULT_ANSI_CODEPAGE);
    gbDCMsgEnabled = ICQGetContactSettingByte(NULL, "DirectMessaging", DEFAULT_DCMSG_ENABLED);
    gbTempVisListEnabled = ICQGetContactSettingByte(NULL, "TempVisListEnabled", DEFAULT_TEMPVIS_ENABLED);
    gbSsiEnabled = ICQGetContactSettingByte(NULL, "UseServerCList", DEFAULT_SS_ENABLED);
    gbAvatarsEnabled = ICQGetContactSettingByte(NULL, "AvatarsEnabled", DEFAULT_AVATARS_ENABLED);
    gbXStatusEnabled = ICQGetContactSettingByte(NULL, "XStatusEnabled", DEFAULT_XSTATUS_ENABLED);
}

WORD gwAnsiCodepage;
BYTE gbGatewayMode;
BYTE gbSecureLogin;
BYTE gbXStatusEnabled;
BYTE gbUnicodeCore_dep;	// FIXME: deprecated
BYTE gbWebAware;
icq_mode_messages modeMsgs;
BOOL m_bSecureConnection;
BYTE gbCustomCapEnabled;
BYTE gbHideIdEnabled;
BYTE gbRTFEnabled;
BYTE gbVerEnabled;
BYTE gbTzerEnabled;
BYTE gbUnicodeAPI_dep;	// FIXME: deprecated
BYTE gbUtfEnabled;
BYTE gbQipStatusEnabled;
BYTE gbTempVisListEnabled;
char gpszPassword[16];
BYTE gbSsiEnabled;
BYTE gbAvatarsEnabled;
BYTE gbAimEnabled;
BYTE gbDCMsgEnabled;
//char gpszICQProtoName[MAX_PATH];
CRITICAL_SECTION modeMsgsMutex;
HANDLE ghDirectNetlibUser;
HANDLE ghServerNetlibUser;

// New methods ///////////////////////////////////////////////////////////
/*
static PROTO_INTERFACE* icqProtoInit( const char* pszProtoName, const TCHAR* tszUserName )
{
	CIcqProto *ppro = new CIcqProto(pszProtoName, tszUserName);
	g_Instances.insert(ppro);
	return ppro;
}

static int icqProtoUninit( PROTO_INTERFACE* ppro )
{
	g_Instances.remove(( CIcqProto* )ppro);
	delete ( CIcqProto* )ppro;
	return 0;
}
*/