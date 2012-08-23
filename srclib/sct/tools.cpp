//this file is part of eMule IS Mod
//Copyright (C)2009 eMule IS Mod
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "stdafx.h"
#include "tools.h"
#include <wininet.h>
#include <atlutil.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Получить пути к дочерним веткам реестра
CStringVector GetChildRegKeys( LPCTSTR szPath )
{
    HKEY hKey;
    if ( RegOpenKey(HKEY_LOCAL_MACHINE, szPath, &hKey) != ERROR_SUCCESS )
        return CStringVector();

    DWORD dwLargestKeySize = 0;
    if ( RegQueryInfoKey(hKey, 0,0,0,0,&dwLargestKeySize,0,0,0,0,0,0) != ERROR_SUCCESS )
        dwLargestKeySize = 1024;
    LPTSTR szKeyBuf = new TCHAR[dwLargestKeySize+2];

    CStringVector arrKeys;

    DWORD i=0;
    while ( RegEnumKey(hKey, i++, szKeyBuf, dwLargestKeySize+1) == ERROR_SUCCESS )
    {
        arrKeys.push_back(szKeyBuf);
    }

    RegCloseKey(hKey);
    delete[] szKeyBuf;

    return arrKeys;
}

// Получить значения ключей в ветке реестра
CStringStringMap GetChildRegValues( LPCTSTR szPath )
{
    HKEY hKey;
    if ( RegOpenKey(HKEY_LOCAL_MACHINE, szPath, &hKey) != ERROR_SUCCESS )
        return CStringStringMap();

    // вычислим подходящие размеры и выделим под них буферы
    DWORD dwLargestNameSize = 0;
    DWORD dwLargestValueSize = 0;
    if ( RegQueryInfoKey(hKey, 0,0,0,0,0,0,0,&dwLargestNameSize,&dwLargestValueSize,0,0) != ERROR_SUCCESS )
    {
        dwLargestNameSize = 1024;
        dwLargestValueSize = 1024;
    }
    LPTSTR szNameBuf = new TCHAR[dwLargestNameSize+10];
    LPTSTR szValueBuf = new TCHAR[dwLargestValueSize+10];

    CStringStringMap mapNamVal;

    DWORD dwNameSize  = dwLargestNameSize += 10;
    DWORD dwValueSize = dwLargestValueSize += 10;

    DWORD i=0;
    LSTATUS lStatus;
    while( (lStatus = RegEnumValue(hKey, i++, szNameBuf, &dwNameSize, 0, 0, (LPBYTE)szValueBuf, &dwValueSize)) == ERROR_SUCCESS )
    {
        mapNamVal[szNameBuf] = szValueBuf;

        // восстановим размеры
        dwNameSize = dwLargestNameSize;
        dwValueSize = dwLargestValueSize;
    }

    RegCloseKey(hKey);
    delete[] szNameBuf;
    delete[] szValueBuf;

    return mapNamVal;
}

CStringVector SplitString( const CString& strString, LPCTSTR szSplitter )
{
    CStringVector arrStrings;

    int curPos = 0;
    CString resToken = strString.Tokenize(szSplitter, curPos);

    while (resToken != _T(""))
    {
        arrStrings.push_back(resToken);
        resToken = strString.Tokenize(szSplitter, curPos);
    };     

    return arrStrings;
}

// резолвим в хостнейм или ИП в машинный формат
uint32_t Resolve( LPCSTR szHost )
{
    uint32_t ret = NULL;
    //This structure should never be released!
    struct hostent * hp = gethostbyname(szHost);
    if (!hp)
    {
        ret = inet_addr(szHost);
    }
    if ((!hp)&&(ret == INADDR_NONE))
    {
        return NULL;
    }
    if (hp != NULL)
    {
        memcpy((void*)&ret, hp->h_addr, hp->h_length);
    }
    return ret;
}
