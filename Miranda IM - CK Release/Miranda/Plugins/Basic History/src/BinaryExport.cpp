/*
Basic History plugin for
Miranda IM: the free IM client for Microsoft* Windows*

Author
			Copyright (C) 2011-2012 Krzysztof Kral

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

#include "StdAfx.h"
#include "BinaryExport.h"

#define EXP_FILE (*((std::ofstream*)IExport::stream))
#define IMP_FILE (*((std::ifstream*)IImport::stream))

std::wstring GetProtocolName(HANDLE hContact);
std::wstring GetContactId(HANDLE hContact);

#pragma pack(push, 1)

struct BinaryFileHeader
{
	unsigned char signature[4];
	unsigned char version;
	unsigned char extraFlags;
	unsigned short int reserved;
	unsigned int codepage;
	unsigned short int dataStart;
};

struct BinaryFileMessageHeader
{
	DWORD timestamp;
	WORD eventType;
	WORD flags;
};

#pragma pack(pop)

BinaryExport::~BinaryExport()
{
}

void BinaryExport::WriteString(const std::wstring &str)
{
	int conv = WideCharToMultiByte(codepage, 0, str.c_str(), str.length() + 1, NULL, 0, NULL, NULL);
	char* buf = new char[conv];
	conv = WideCharToMultiByte(codepage, 0, str.c_str(), str.length() + 1, buf, conv, NULL, NULL);
	EXP_FILE.write(buf, conv);
	delete[] buf;
}

bool BinaryExport::ReadString(std::wstring &str)
{	
	std::string buf;
	int size = 1024;
	int pos = 0;
	int totalSize = 0; 
	while(1)
	{
		buf.resize(size);
		if(IMP_FILE.peek() == 0)
		{
			IMP_FILE.get();
			break;
		}

		IMP_FILE.get(((char*)buf.c_str()) + pos, size - pos, 0);
		if(!IMP_FILE.good())
			return false;

		int readed = IMP_FILE.gcount();
		totalSize += readed;
		char end;
		IMP_FILE.get(end);
		if(!IMP_FILE.good())
			return false;
		if(end == 0)
			break;
		if(size - pos - 1 != readed)
			return false;
		buf[size - 1] = end;
		++totalSize;
		size += 1024;
		pos += 1024;
	}

	if(totalSize == 0)
		return true;
	int sizeW = MultiByteToWideChar(codepage, 0, (char*)buf.c_str(), totalSize, NULL, 0);
	str.resize(sizeW);
	MultiByteToWideChar(codepage, 0, (char*)buf.c_str(), totalSize, (wchar_t*)str.c_str(), sizeW);
	return true;
}

void BinaryExport::WriteHeader(const std::wstring &fileName, const std::wstring &filterName, const std::wstring &myName, const std::wstring &myId, const std::wstring &name1, const std::wstring &proto1, const std::wstring &id1, const std::string& baseProto1, const std::wstring& encoding)
{
	BinaryFileHeader header;
	memset(&header, 0, sizeof(BinaryFileHeader));
	memcpy(header.signature, "BHBF", 4);
	header.codepage = codepage = CP_UTF8;
	EXP_FILE.write((char*)&header, sizeof(BinaryFileHeader));
	WriteString(filterName);
	WriteString(myName);
	WriteString(myId);
	WriteString(name1);
	WriteString(proto1);
	WriteString(id1);
	size_t pos = EXP_FILE.tellp();
	header.dataStart = pos;
	EXP_FILE.seekp(offsetof(BinaryFileHeader, dataStart), std::ios_base::beg);
	EXP_FILE.write((char*)&(header.dataStart), sizeof(header.dataStart));
	EXP_FILE.seekp(pos, std::ios_base::beg);
	lTime = 0;
}

void BinaryExport::WriteFooter()
{
}

void BinaryExport::WriteGroup(bool isMe, const std::wstring &time, const std::wstring &user, const std::wstring &eventText)
{
}

void BinaryExport::WriteMessage(bool isMe, const std::wstring &longDate, const std::wstring &shortDate, const std::wstring &user, const std::wstring &message, const DBEVENTINFO& dbei)
{
	if(dbei.timestamp >= lTime)
	{
		BinaryFileMessageHeader header;
		header.eventType = dbei.eventType;
		header.flags = dbei.flags & (~(0x800));
		header.timestamp = dbei.timestamp;
		EXP_FILE.write((char*)&header, sizeof(BinaryFileMessageHeader));
		WriteString(message);
		lTime = dbei.timestamp;
	}
}

bool ReadHeader(BinaryFileHeader& header, std::istream* stream)
{
	stream->read((char*)&header, sizeof(BinaryFileHeader));
	if(!stream->good())
		return false;
	if(memcmp(header.signature, "BHBF", 4) != 0)
		return false;
	if(header.version != 0 || header.codepage == 12000 || header.codepage == 12001)
		return false;

	return true;
}

int BinaryExport::IsContactInFile(const std::vector<HANDLE>& contacts)
{
	BinaryFileHeader header;
	if(!ReadHeader(header, IImport::stream))
		return -2;
	codepage = header.codepage;
	std::wstring filterName;
	std::wstring myName;
	std::wstring myId;
	std::wstring name1;
	std::wstring proto1;
	std::wstring id1;
	if(!ReadString(filterName))
		return -2;
	if(!ReadString(myName))
		return -2;
	if(!ReadString(myId))
		return -2;
	if(!ReadString(name1))
		return -2;
	if(!ReadString(proto1))
		return -2;
	if(!ReadString(id1))
		return -2;

	size_t pos = IMP_FILE.tellg();
	if(header.dataStart < pos)
		return -2;
	
	IMP_FILE.seekg(0, std::ios_base::beg);
	for(int i = 0; i < contacts.size(); ++i)
	{
		std::wstring pn = GetProtocolName(contacts[i]);
		std::wstring id = GetContactId(contacts[i]);
		if(pn == proto1 && id == id1)
		{
			return i;
		}
	}

	return -1;
}

bool BinaryExport::GetEventList(std::vector<IImport::ExternalMessage>& eventList)
{
	BinaryFileHeader header;
	if(!ReadHeader(header, IImport::stream))
		return false;
	codepage = header.codepage;
	IMP_FILE.seekg(header.dataStart, std::ios_base::beg);
	BinaryFileMessageHeader messageHeader;
	while(1)
	{
		IMP_FILE.read((char*)&messageHeader, sizeof(BinaryFileMessageHeader));
		if(IMP_FILE.eof())
			break;
		if(!IMP_FILE.good())
			return false;

		IImport::ExternalMessage exMsg;
		exMsg.eventType = messageHeader.eventType;
		exMsg.flags = messageHeader.flags;
		exMsg.timestamp = messageHeader.timestamp;
		if(!ReadString(exMsg.message))
			return false;
		
		eventList.push_back(exMsg);
	}

	return true;
}
