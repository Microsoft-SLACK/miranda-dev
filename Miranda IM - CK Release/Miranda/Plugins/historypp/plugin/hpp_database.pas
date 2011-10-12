(*
    History++ plugin for Miranda IM: the free IM client for Microsoft* Windows*

    Copyright (C) 2006-2009 theMIROn, 2003-2006 Art Fedorov.
    History+ parts (C) 2001 Christian Kastner

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
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*)

{-----------------------------------------------------------------------------
 hpp_database (historypp project)

 Version:   1.0
 Created:   31.03.2003
 Author:    Oxygen

 [ Description ]

 Helper routines for database use

 [ History ]
 1.0 (31.03.2003) - Initial version

 [ Modifications ]

 [ Knows Inssues ]
 None

 Contributors: theMIROn, Art Fedorov
-----------------------------------------------------------------------------}


unit hpp_database;

interface

uses m_globaldefs, m_api, windows, hpp_global;

procedure SetSafetyMode(Safe: Boolean);

function DBGetContactSettingString(hContact: THandle; const szModule: PChar; const szSetting: PChar; ErrorValue: PChar): AnsiString;
function DBGetContactSettingWideString(hContact: THandle; const szModule: PChar; const szSetting: PChar; ErrorValue: PWideChar): WideString;
function DBWriteContactSettingWideString(hContact: THandle; const szModule: PChar; const szSetting: PChar; const val: PWideChar): Integer;

function DBDelete(const Module, Param: String): Boolean; overload;
function DBDelete(const hContact: THandle; const Module, Param: String): Boolean; overload;
function DBExists(const Module, Param: String): Boolean; overload;
function DBExists(const hContact: THandle; const Module, Param: String): Boolean; overload;

function GetDBBlob(const Module,Param: String; var Value: Pointer; var Size: Integer): Boolean; overload;
function GetDBBlob(const hContact: THandle; const Module,Param: String; var Value: Pointer; var Size: Integer): Boolean; overload;
function GetDBStr(const Module,Param: String; Default: String): String; overload;
function GetDBStr(const hContact: THandle; const Module,Param: String; Default: String): String; overload;
function GetDBWideStr(const Module,Param: String; Default: WideString): WideString; overload;
function GetDBWideStr(const hContact: THandle; const Module,Param: String; Default: WideString): WideString; overload;
function GetDBInt(const Module,Param: String; Default: Integer): Integer; overload;
function GetDBInt(const hContact: THandle; const Module,Param: String; Default: Integer): Integer; overload;
function GetDBWord(const Module,Param: String; Default: Word): Word; overload;
function GetDBWord(const hContact: THandle; const Module,Param: String; Default: Word): Word; overload;
function GetDBDWord(const Module,Param: String; Default: DWord): DWord; overload;
function GetDBDWord(const hContact: THandle; const Module,Param: String; Default: DWord): DWord; overload;
function GetDBByte(const Module,Param: String; Default: Byte): Byte; overload;
function GetDBByte(const hContact: THandle; const Module,Param: String; Default: Byte): Byte; overload;
function GetDBBool(const Module,Param: String; Default: Boolean): Boolean; overload;
function GetDBBool(const hContact: THandle; const Module,Param: String; Default: Boolean): Boolean; overload;
function GetDBDateTime(const hContact: THandle; const Module,Param: String; Default: TDateTime): TDateTime; overload;
function GetDBDateTime(const Module,Param: String; Default: TDateTime): TDateTime; overload;

function WriteDBBlob(const Module,Param: String; Value: Pointer; Size: Integer): Integer; overload;
function WriteDBBlob(const hContact: THandle; const Module,Param: String; Value: Pointer; Size: Integer): Integer; overload;
function WriteDBByte(const Module,Param: String; Value: Byte): Integer; overload;
function WriteDBByte(const hContact: THandle; const Module,Param: String; Value: Byte): Integer; overload;
function WriteDBWord(const Module,Param: String; Value: Word): Integer; overload;
function WriteDBWord(const hContact: THandle; const Module,Param: String; Value: Word): Integer; overload;
function WriteDBDWord(const Module,Param: String; Value: DWord): Integer; overload;
function WriteDBDWord(const hContact: THandle; const Module,Param: String; Value: DWord): Integer; overload;
function WriteDBInt(const Module,Param: String; Value: Integer): Integer; overload;
function WriteDBInt(const hContact: THandle; const Module,Param: String; Value: Integer): Integer; overload;
function WriteDBStr(const Module,Param: String; Value: String): Integer; overload;
function WriteDBStr(const hContact: THandle; const Module,Param: String; Value: String): Integer; overload;
function WriteDBWideStr(const Module,Param: String; Value: WideString): Integer; overload;
function WriteDBWideStr(const hContact: THandle; const Module,Param: String; Value: WideString): Integer; overload;
function WriteDBBool(const Module,Param: String; Value: Boolean): Integer; overload;
function WriteDBBool(const hContact: THandle; const Module,Param: String; Value: Boolean): Integer; overload;
function WriteDBDateTime(const hContact: THandle; const Module,Param: String; Value: TDateTime): Integer; overload;
function WriteDBDateTime(const Module,Param: String; Value: TDateTime): Integer; overload;

implementation

procedure SetSafetyMode(Safe: Boolean);
begin
  PluginLink^.CallService(MS_DB_SETSAFETYMODE,WPARAM(Safe),0);
end;

function DBExists(const Module, Param: String): Boolean;
begin
  Result := DBExists(0,Module,Param);
end;

function DBExists(const hContact: THandle; const Module, Param: String): Boolean;
var
  dbv: TDBVARIANT;
  cgs: TDBCONTACTGETSETTING;
begin
  cgs.szModule := PChar(Module);
  cgs.szSetting := PChar(Param);
  cgs.pValue := @dbv;
  Result := (PluginLink^.CallService(MS_DB_CONTACT_GETSETTING, hContact, lParam(@cgs)) = 0);
  if Result then
    DBFreeVariant(@dbv);
end;

function DBDelete(const Module, Param: String): Boolean;
begin
  Result := DBDelete(0,Module,Param);
end;

function DBDelete(const hContact: THandle; const Module, Param: String): Boolean;
begin
  Result := (DBDeleteContactSetting(hContact,PChar(Module),PChar(Param)) = 0);
end;

function WriteDBBool(const Module,Param: String; Value: Boolean): Integer;
begin
  Result := WriteDBBool(0,Module,Param,Value);
end;

function WriteDBBool(const hContact: THandle; const Module,Param: String; Value: Boolean): Integer;
begin
  Result := WriteDBByte(hContact,Module,Param,Byte(Value));
end;

function WriteDBByte(const Module,Param: String; Value: Byte): Integer;
begin
  Result := WriteDBByte(0,Module,Param,Value);
end;

function WriteDBByte(const hContact: THandle; const Module,Param: String; Value: Byte): Integer;
begin
  Result := DBWriteContactSettingByte(hContact,PChar(Module), PChar(Param), Value);
end;

function WriteDBWord(const Module,Param: String; Value: Word): Integer;
begin
  Result := WriteDBWord(0,Module,Param,Value);
end;

function WriteDBWord(const hContact: THandle; const Module,Param: String; Value: Word): Integer;
begin
  Result := DBWriteContactSettingWord(hContact,PChar(Module),PChar(Param),Value);
end;

function WriteDBDWord(const Module,Param: String; Value: DWord): Integer;
begin
  Result := WriteDBWord(0,Module,Param,Value);
end;

function WriteDBDWord(const hContact: THandle; const Module,Param: String; Value: DWord): Integer;
begin
  Result := DBWriteContactSettingDWord(hContact,PChar(Module),PChar(Param),Value);
end;

function WriteDBInt(const Module,Param: String; Value: Integer): Integer;
begin
  Result := WriteDBInt(0,Module,Param,Value);
end;

function WriteDBInt(const hContact: THandle; const Module,Param: String; Value: Integer): Integer;
var
  cws: TDBCONTACTWRITESETTING;
begin
  cws.szModule := PChar(Module);
  cws.szSetting := PChar(Param);
  cws.value.type_ := DBVT_DWORD;
  cws.value.dVal := Value;
  Result := PluginLink^.CallService(MS_DB_CONTACT_WRITESETTING, hContact, lParam(@cws));
end;

function WriteDBStr(const Module,Param: String; Value: String): Integer;
begin
  Result := WriteDBStr(0,Module,Param,Value);
end;

function WriteDBStr(const hContact: THandle; const Module,Param: String; Value: String): Integer;
begin
  Result := DBWriteContactSettingString(hContact,PChar(Module),PChar(Param),PChar(Value));
end;

function WriteDBWideStr(const Module,Param: String; Value: WideString): Integer;
begin
  Result := WriteDBWideStr(0,Module,Param,Value);
end;

function WriteDBWideStr(const hContact: THandle; const Module,Param: String; Value: WideString): Integer;
begin
  Result := DBWriteContactSettingWideString(hContact,PChar(Module),PChar(Param),PWideChar(Value));
end;

function DBWriteContactSettingWideString(hContact: THandle; const szModule: PChar; const szSetting: PChar; const val: PWideChar): Integer;
var
  cws: TDBCONTACTWRITESETTING;
begin
  cws.szModule := szModule;
  cws.szSetting := szSetting;
  if hppCoreUnicode then begin
    cws.value.type_ := DBVT_WCHAR;
    cws.value.pwszVal := val;
  end else begin
    cws.value.type_ := DBVT_ASCIIZ;
    cws.value.pszVal := PChar(WideToAnsiString(val,hppCodepage));
  end;
  Result := PluginLink^.CallService(MS_DB_CONTACT_WRITESETTING, hContact, lParam(@cws));
end;

function WriteDBBlob(const Module,Param: String; Value: Pointer; Size: Integer): Integer;
begin
  Result := WriteDBBlob(0,Module,Param,Value,Size);
end;

function WriteDBBlob(const hContact: THandle; const Module,Param: String; Value: Pointer; Size: Integer): Integer;
var
  cws: TDBContactWriteSetting;
begin
  ZeroMemory(@cws,SizeOf(cws));
  cws.szModule := PChar(Module);
  cws.szSetting := PChar(Param);
  cws.value.type_ := DBVT_BLOB;
  cws.value.pbVal := Value;
  cws.value.cpbVal := Word(Size);
  Result := PluginLink^.CallService(MS_DB_CONTACT_WRITESETTING,hContact,lParam(@cws));
end;

function WriteDBDateTime(const hContact: THandle; const Module,Param: String; Value: TDateTime): Integer; overload;
var
  p: PDateTime;
begin
  GetMem(p,SizeOf(TDateTime));
  p^ := Value;
  Result := WriteDBBlob(hContact,Module,Param,p,SizeOf(TDateTime));
  FreeMem(p,SizeOf(TDateTime));
end;

function WriteDBDateTime(const Module,Param: String; Value: TDateTime): Integer; overload;
begin
  Result := WriteDBDateTime(0,Module,Param,Value);
end;

function GetDBBlob(const Module,Param: String; var Value: Pointer; var Size: Integer): Boolean;
begin
  Result := GetDBBlob(0,Module,Param,Value,Size);
end;

function GetDBBlob(const hContact: THandle; const Module,Param: String; var Value: Pointer; var Size: Integer): Boolean;
var
  cgs: TDBContactGetSetting;
  dbv: TDBVARIANT;
begin
  Result := False;
  ZeroMemory(@cgs,SizeOf(cgs));
  cgs.szModule := PChar(Module);
  cgs.szSetting := PChar(Param);
  cgs.pValue := @dbv;
  if PluginLink^.CallService(MS_DB_CONTACT_GETSETTING, hContact, lParam(@cgs)) <> 0 then exit;
  Size := dbv.cpbVal;
  Value := nil;
  if dbv.cpbVal = 0 then exit;
  GetMem(Value,dbv.cpbVal);
  Move(dbv.pbVal^,PByte(Value)^,dbv.cpbVal);
  DBFreeVariant(@dbv);
  Result := True;
end;

function GetDBBool(const Module,Param: String; Default: Boolean): Boolean;
begin
  Result := GetDBBool(0,Module,Param,Default);
end;

function GetDBBool(const hContact: THandle; const Module,Param: String; Default: Boolean): Boolean;
begin
  Result := Boolean(GetDBByte(hContact,Module,Param,Byte(Default)));
end;

function GetDBByte(const Module,Param: String; Default: Byte): Byte;
begin
  Result := GetDBByte(0,Module,Param,Default);
end;

function GetDBByte(const hContact: THandle; const Module,Param: String; Default: Byte): Byte;
begin
  Result := DBGetContactSettingByte(hContact,PChar(Module),PChar(Param),Default);
end;

function GetDBWord(const Module,Param: String; Default: Word): Word;
begin
  Result := GetDBWord(0,Module,Param,Default);
end;

function GetDBWord(const hContact: THandle; const Module,Param: String; Default: Word): Word;
begin
  Result := DBGetContactSettingWord(hContact,PChar(Module),PChar(Param),Default);
end;

function GetDBDWord(const Module,Param: String; Default: DWord): DWord;
begin
  Result := GetDBDWord(0,Module,Param,Default);
end;

function GetDBDWord(const hContact: THandle; const Module,Param: String; Default: DWord): DWord;
begin
  Result := DBGetContactSettingDWord(hContact,PChar(Module),PChar(Param),Default);
end;

function GetDBInt(const Module,Param: String; Default: Integer): Integer;
begin
  Result := GetDBInt(0,Module,Param,Default);
end;

function GetDBInt(const hContact: THandle; const Module,Param: String; Default: Integer): Integer;
var
  cws:TDBCONTACTGETSETTING;
  dbv:TDBVariant;
begin
  dbv.type_ := DBVT_DWORD;
  dbv.dVal:=Default;
  cws.szModule:=PChar(Module);
  cws.szSetting:=PChar(Param);
  cws.pValue:=@dbv;
  if PluginLink^.CallService(MS_DB_CONTACT_GETSETTING,hContact,DWord(@cws))<>0 then
    Result:=default
  else
    Result:=dbv.dval;
end;

function GetDBStr(const Module,Param: String; Default: String): String;
begin
  Result := GetDBStr(0,Module,Param,Default);
end;

function GetDBStr(const hContact: THandle; const Module,Param: String; Default: String): String;
begin
  Result := DBGetContactSettingString(hContact,PChar(Module),PChar(Param),PChar(Default));
end;

function DBGetContactSettingString(hContact: THandle; const szModule: PChar; const szSetting: PChar; ErrorValue: PChar): AnsiString;
var
  dbv: TDBVARIANT;
  cgs: TDBCONTACTGETSETTING;
  tmp: WideString;
begin
  cgs.szModule := szModule;
  cgs.szSetting := szSetting;
  cgs.pValue := @dbv;
  if PluginLink^.CallService(MS_DB_CONTACT_GETSETTING, hContact, lParam(@cgs)) <> 0 then
    Result := ErrorValue
  else begin
    case dbv.type_ of
      DBVT_ASCIIZ:
        Result := AnsiString(dbv.pszVal);
      DBVT_UTF8: begin
        tmp := AnsiToWideString(dbv.pszVal,CP_UTF8);
	      Result := WideToAnsiString(tmp,hppCodepage);
        end;
      DBVT_WCHAR:
        Result := WideToAnsiString(dbv.pwszVal,hppCodepage);
    end;
    // free variant
    DBFreeVariant(@dbv);
  end;
end;

function GetDBWideStr(const Module,Param: String; Default: WideString): WideString;
begin
  Result := GetDBWideStr(0,Module,Param,Default);
end;

function GetDBWideStr(const hContact: THandle; const Module,Param: String; Default: WideString): WideString;
begin
  Result := DBGetContactSettingWideString(hContact,PChar(Module),PChar(Param),PWideChar(Default));
end;

function DBGetContactSettingWideString(hContact: THandle; const szModule: PChar; const szSetting: PChar; ErrorValue: PWideChar): WideString;
var
  dbv: TDBVARIANT;
  cgs: TDBCONTACTGETSETTING;
begin
  cgs.szModule := szModule;
  cgs.szSetting := szSetting;
  cgs.pValue := @dbv;
  if PluginLink^.CallService(MS_DB_CONTACT_GETSETTING, hContact, lParam(@cgs)) <> 0 then
    Result := ErrorValue
  else begin
    case dbv.type_ of
      DBVT_ASCIIZ:
        Result := AnsiToWideString(dbv.pszVal,hppCodepage);
      DBVT_UTF8:
        Result := AnsiToWideString(dbv.pszVal,CP_UTF8);
      DBVT_WCHAR:
        Result := WideString(dbv.pwszVal);
    end;
    // free variant
    DBFreeVariant(@dbv);
  end;
end;

function GetDBDateTime(const hContact: THandle; const Module,Param: String; Default: TDateTime): TDateTime; overload;
var
  p: Pointer;
  s: Integer;
begin
  Result := Default;
  if not GetDBBlob(hContact,Module,Param,p,s) then exit;
  if s <> SizeOf(TDateTime) then begin
    FreeMem(p,s);
    exit;
  end;
  Result := PDateTime(p)^;
  FreeMem(p,s);
end;

function GetDBDateTime(const Module,Param: String; Default: TDateTime): TDateTime; overload;
begin
  Result := GetDBDateTime(0,Module,Param,Default);
end;

end.
