//this file is part of eMule IS Mod
//Copyright (C)2009 IS Mod Team
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#pragma once

class CReg  
{
public:
    explicit CReg(HKEY hKey, LPCTSTR lpszSubKeyName);
    ~CReg() { Close(); }
    void Close();

    uint32_t GetLength(LPCTSTR lpszEntry);
	LPCTSTR GetString(LPCTSTR lpszEntry, LPCTSTR lpszDeafultValue = NULL);
    bool GetBinary(LPCTSTR lpszEntry, BYTE* pData, uint32_t nLength);
    uint32_t GetUInt32(LPCTSTR lpszEntry, uint32_t nDefaultValue = 0);
    uint64_t GetUInt64(LPCTSTR lpszEntry, uint64_t nDefaultValue = 0);

    bool WriteString(LPCTSTR lpszEntry, LPCTSTR lpszValue);
    bool WriteBinary(LPCTSTR lpszEntry, BYTE* pData, uint32_t nLength);
    bool WriteUInt32(LPCTSTR lpszEntry, uint32_t nValue);
    bool WriteUInt64(LPCTSTR lpszEntry, uint64_t nValue);
    
private:
    CReg();
#define MAX_REG_VALUE_SIZE 256
    TCHAR m_chBuffer[MAX_REG_VALUE_SIZE];
    HKEY m_hKey;
    bool m_bIsOpened;
};
