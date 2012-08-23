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

#include "stdafx.h"
#include "Reg2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CReg::CReg(HKEY hKey, LPCTSTR lpszSubKey)
{
    m_bIsOpened = ERROR_SUCCESS == RegCreateKeyEx(hKey, lpszSubKey, 0,0,0, KEY_WRITE|KEY_READ,0, &m_hKey, 0);
}

uint32_t CReg::GetLength(LPCTSTR lpszEntry)
{
    DWORD dwBuffSize = 0;
	if ( m_bIsOpened && ERROR_SUCCESS == RegQueryValueEx(m_hKey, lpszEntry, 0, 0, NULL, &dwBuffSize) )
		return dwBuffSize;
	else
		return 0;
}

LPCTSTR CReg::GetString(LPCTSTR lpszEntry, LPCTSTR lpszDeafultValue /*= NULL*/)
{
    DWORD dwBuffSize = MAX_REG_VALUE_SIZE;
    if (m_bIsOpened && ERROR_SUCCESS == RegQueryValueEx(m_hKey, lpszEntry, 0, 0, (LPBYTE)m_chBuffer, &dwBuffSize)
        && dwBuffSize > 0)
    {
        m_chBuffer[dwBuffSize/sizeof(TCHAR)] = _T('\0');
        return m_chBuffer;
    } else if (lpszDeafultValue == NULL)
        return _T("");
    else
        return lpszDeafultValue;
}

bool CReg::GetBinary(LPCTSTR lpszEntry, BYTE* pData, uint32_t nLength)
{
    DWORD dwSize = nLength;
    if (m_bIsOpened && ERROR_SUCCESS == RegQueryValueEx(m_hKey, lpszEntry, 0, 0, pData, &dwSize))
        return true;
    else
        return false;
}

uint32_t CReg::GetUInt32(LPCTSTR lpszEntry, uint32_t nDefaultValue /*= 0*/)
{
    DWORD dwValue;
    DWORD dwValueSize = sizeof(dwValue);
    if (m_bIsOpened && ERROR_SUCCESS == RegQueryValueEx(m_hKey, lpszEntry, 0, 0, (LPBYTE)&dwValue, &dwValueSize))
    {
        return (uint32_t)(dwValue & _I32_MAX);
    } else
        return nDefaultValue;
}

uint64_t CReg::GetUInt64(LPCTSTR lpszEntry, uint64_t nDefaultValue /*= 0*/)
{
    uint64_t qwValue;
    DWORD dwValueSize = sizeof(qwValue);
    if (m_bIsOpened && ERROR_SUCCESS == RegQueryValueEx(m_hKey, lpszEntry, 0, 0, (LPBYTE)&qwValue, &dwValueSize))
    {
        return qwValue;
    } else
        return nDefaultValue;
}

bool CReg::WriteString(LPCTSTR lpszEntry, LPCTSTR lpszValue)
{
    if (m_bIsOpened && ERROR_SUCCESS == RegSetValueEx(m_hKey, lpszEntry, 0, REG_SZ, (LPBYTE)lpszValue, _tcslen(lpszValue)*sizeof(TCHAR)))
        return true;
    else
        return false;
}

bool CReg::WriteBinary(LPCTSTR lpszEntry, BYTE* pData, uint32_t nLength)
{
    if (m_bIsOpened && ERROR_SUCCESS == RegSetValueEx(m_hKey, lpszEntry, 0, REG_BINARY, pData, nLength))
        return true;
    else
        return false;
}

bool CReg::WriteUInt32(LPCTSTR lpszEntry, uint32_t nValue)
{
    DWORD dwValue = nValue;
    if (m_bIsOpened && ERROR_SUCCESS == RegSetValueEx(m_hKey, lpszEntry, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD)))
        return true;
    else
        return false;
}

bool CReg::WriteUInt64(LPCTSTR lpszEntry, uint64_t nValue)
{
    uint64_t qwValue = nValue;
    if (m_bIsOpened && ERROR_SUCCESS == RegSetValueEx(m_hKey, lpszEntry, 0, REG_DWORD, (LPBYTE)&qwValue, sizeof(uint64_t)))
        return true;
    else
        return false;
}

void CReg::Close()
{
    if(m_bIsOpened)
    {
        m_bIsOpened = false;
        RegCloseKey(m_hKey);
    }
}