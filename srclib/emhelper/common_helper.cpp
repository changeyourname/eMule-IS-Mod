//this file is part of eMule IS Mod
//Copyright (c) 2012 IS Mod Team
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

#include <Windows.h>
#include <Softpub.h>
#include <wintrust.h>
#include <string>
#include "emhelper.h"

#pragma comment(lib, "wintrust.lib")

namespace emUtils {

std::wstring EMHELPER_API decodeHtmlEntities( const std::wstring &text )
{
	std::wstring decodedText;
	decodedText.reserve(text.size());

	for ( size_t x = 0; x < text.size(); ++x )
	{
		if ( TEXT('&') == text[x] && TEXT('#') == text[x+1] && x+3 < text.size() ) {
			size_t y = x + 2;
			while ( (y < text.size()) && (TEXT('0') <= text[y]) && (TEXT('9') >= text[y]) ) {
				++y;
			}

			if( TEXT(';') == text[y] ) {
				std::wstring chrCode = text.substr(x+2, y-x-2);
				// TODO: check bounds?
				wchar_t ch = (wchar_t)wcstoul(chrCode.c_str(), NULL, 10);
				decodedText += ch;
				x = y;
			} else {
				decodedText += text[x];
			}
		} else {
			decodedText += text[x];
		}
	}
	return decodedText;
}

bool EMHELPER_API shutdownWindows()
{
	HANDLE hToken; 
	TOKEN_PRIVILEGES tkp; 

	if ( !OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken) ) 
		return false; 

	LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid); 

	tkp.PrivilegeCount = 1;  // one privilege to set    
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 

	AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0); 
	
	CloseHandle(hToken);

	if ( GetLastError() != ERROR_SUCCESS ) 
		return false; 
	
	bool success = TRUE == ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE, 
		SHTDN_REASON_MAJOR_OPERATINGSYSTEM | SHTDN_REASON_MINOR_UPGRADE |
		SHTDN_REASON_FLAG_PLANNED);

	return success; 
}

bool EMHELPER_API verifyEmbeddedSignature( const std::wstring &file )
{
    LONG lStatus;

    // Initialize the WINTRUST_FILE_INFO structure.

    WINTRUST_FILE_INFO FileData;
    memset(&FileData, 0, sizeof(FileData));
    FileData.cbStruct = sizeof(WINTRUST_FILE_INFO);
    FileData.pcwszFilePath = file.c_str();
    FileData.hFile = NULL;
    FileData.pgKnownSubject = NULL;

    /*
    WVTPolicyGUID specifies the policy to apply on the file
    WINTRUST_ACTION_GENERIC_VERIFY_V2 policy checks:
    
    1) The certificate used to sign the file chains up to a root 
    certificate located in the trusted root certificate store. This 
    implies that the identity of the publisher has been verified by 
    a certification authority.
    
    2) In cases where user interface is displayed (which this example
    does not do), WinVerifyTrust will check for whether the  
    end entity certificate is stored in the trusted publisher store,  
    implying that the user trusts content from this publisher.
    
    3) The end entity certificate has sufficient permission to sign 
    code, as indicated by the presence of a code signing EKU or no 
    EKU.
    */

    GUID WVTPolicyGUID = WINTRUST_ACTION_GENERIC_VERIFY_V2;
    WINTRUST_DATA WinTrustData;

    // Initialize the WinVerifyTrust input data structure.

    // Default all fields to 0.
    memset(&WinTrustData, 0, sizeof(WinTrustData));

    WinTrustData.cbStruct = sizeof(WinTrustData);
    
    // Use default code signing EKU.
    WinTrustData.pPolicyCallbackData = NULL;

    // No data to pass to SIP.
    WinTrustData.pSIPClientData = NULL;

    // Disable WVT UI.
    WinTrustData.dwUIChoice = WTD_UI_NONE;

    // No revocation checking.
    WinTrustData.fdwRevocationChecks = WTD_REVOKE_NONE; 

    // Verify an embedded signature on a file.
    WinTrustData.dwUnionChoice = WTD_CHOICE_FILE;

    // Default verification.
    WinTrustData.dwStateAction = 0;

    // Not applicable for default verification of embedded signature.
    WinTrustData.hWVTStateData = NULL;

    // Not used.
    WinTrustData.pwszURLReference = NULL;

    // Default.
    WinTrustData.dwProvFlags = WTD_SAFER_FLAG;

    // This is not applicable if there is no UI because it changes 
    // the UI to accommodate running applications instead of 
    // installing applications.
    WinTrustData.dwUIContext = 0;

    // Set pFile.
    WinTrustData.pFile = &FileData;

    // WinVerifyTrust verifies signatures as specified by the GUID 
    // and Wintrust_Data.
    lStatus = WinVerifyTrust(NULL, &WVTPolicyGUID, &WinTrustData);

    return ( ERROR_SUCCESS == lStatus );
}

}