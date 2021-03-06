/*
Omegle protocol for
Miranda IM: the free IM client for Microsoft* Windows*

Authors 
			Copyright � 2011-2012 Robert P�sel

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

void OmegleProto::SignOn(void*)
{
	SYSTEMTIME t;
	GetLocalTime( &t );
	Log("[%d.%d.%d] Using Omegle Protocol %s", t.wDay, t.wMonth, t.wYear, __VERSION_STRING);
	
	ScopedLock s(signon_lock_);

	int old_status = m_iStatus;
	m_iStatus = m_iDesiredStatus;
	ProtoBroadcastAck(m_szModuleName,0,ACKTYPE_STATUS,ACKRESULT_SUCCESS,(HANDLE)old_status,m_iStatus);

	setDword( "LogonTS", (DWORD)time(NULL) );
	ClearChat();
	OnJoinChat(0,false);	

	//ToggleStatusMenuItems(true);
}

void OmegleProto::SignOff(void*)
{
	ScopedLock s(signon_lock_);

	int old_status = m_iStatus;
	m_iStatus = ID_STATUS_OFFLINE;

	Netlib_Shutdown(facy.hEventsConnection);

	OnLeaveChat(NULL, NULL);

	deleteSetting( "LogonTS" );	

	ProtoBroadcastAck(m_szModuleName,0,ACKTYPE_STATUS,ACKRESULT_SUCCESS,(HANDLE)old_status,m_iStatus);

	//SetAllContactStatuses( ID_STATUS_OFFLINE );
	//ToggleStatusMenuItems(false);

	if (facy.hEventsConnection)
		Netlib_CloseHandle(facy.hEventsConnection);
	facy.hEventsConnection = NULL;
}

void OmegleProto::StopChat(bool disconnect)
{
	if (facy.state_ == STATE_WAITING) {
		UpdateChat(NULL, TranslateT("Connecting canceled."), false);		
	} else if (facy.state_ == STATE_ACTIVE || facy.state_ == STATE_SPY) {
		bool spy = facy.state_ == STATE_SPY;

		if (disconnect)
		{
			facy.state_ = STATE_DISCONNECTING;
			UpdateChat(NULL, TranslateT("Disconnecting..."), true);

			if (facy.stop())
				LOG("***** Disconnected from stranger %s", facy.chat_id_.c_str());
			else
				LOG("***** Error in disconnecting from stranger %s", facy.chat_id_.c_str());			
		}		

		if (spy) {
			DeleteChatContact(TranslateT("Stranger 1"));
			DeleteChatContact(TranslateT("Stranger 2"));
		} else {
			DeleteChatContact(TranslateT("Stranger"));
		}

		SetTopic(); // reset topic content
	}
	else
	{ // disconnecting or inactive
		return;
	}

	facy.state_ = STATE_INACTIVE;
	facy.chat_id_ = "";
}

void OmegleProto::NewChat()
{
	if (facy.state_ == STATE_WAITING)
	{
		UpdateChat(NULL, TranslateT("We are already waiting for new stranger..."), false);
		return;
	}
	else if (facy.state_ == STATE_ACTIVE || facy.state_ == STATE_SPY)
	{		
		UpdateChat(NULL, TranslateT("Disconnecting..."), true);

		if (facy.stop())
			LOG("***** Disconnected from stranger %s", facy.chat_id_.c_str());
		else
			LOG("***** Error in disconnecting from stranger %s", facy.chat_id_.c_str());

		if (facy.state_ == STATE_SPY) {
			DeleteChatContact(TranslateT("Stranger 1"));
			DeleteChatContact(TranslateT("Stranger 2"));			
		} else {
			DeleteChatContact(TranslateT("Stranger"));
		}

		SetTopic(); // reset topic content

		ClearChat();
		
		UpdateChat(NULL, TranslateT("Connecting..."), true);

		facy.state_ = STATE_WAITING;

		if (facy.start())
		{		
			UpdateChat(NULL, TranslateT("Waiting for Stranger..."), true);
			LOG("***** Waiting for stranger %s", facy.chat_id_.c_str());
		}
	}
	else if (facy.state_ == STATE_DISCONNECTING)
	{
		//UpdateChat(NULL, TranslateT("We are disconnecting now, wait..."), false);
		return;
	}
	else
	{
		ClearChat();
		UpdateChat(NULL, TranslateT("Connecting..."), true);

		facy.state_ = STATE_WAITING;

		if (facy.start())
		{		
			UpdateChat(NULL, TranslateT("Waiting for Stranger..."), true);
			LOG("***** Waiting for stranger %s", facy.chat_id_.c_str());

			ForkThread( &OmegleProto::EventsLoop, this );
		}
	}
	
}

void OmegleProto::EventsLoop(void *)
{
	ScopedLock s(events_loop_lock_);

	time_t tim = ::time(NULL);
	LOG( ">>>>> Entering Omegle::EventsLoop[%d]", tim );

	while ( facy.events() )
	{
		if ( facy.state_ == STATE_INACTIVE || facy.state_ == STATE_DISCONNECTING || !isOnline() )
			break;
		LOG( "***** OmegleProto::EventsLoop[%d] refreshing...", tim );
	}

	ResetEvent(events_loop_lock_);
	LOG( "<<<<< Exiting OmegleProto::EventsLoop[%d]", tim );
}
