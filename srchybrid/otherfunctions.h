//this file is part of eMule
//Copyright (C)2002-2008 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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

#pragma once

class CAbstractFile;
class CKnownFile;
struct Requested_Block_Struct;
class CUpDownClient;
class CPartFile;
class CSafeMemFile;
class CShareableFile;

enum EFileType { 
		FILETYPE_UNKNOWN,
		FILETYPE_EXECUTABLE,
		ARCHIVE_ZIP,
		ARCHIVE_RAR,
		ARCHIVE_ACE,
		IMAGE_ISO,
		AUDIO_MPEG,
		VIDEO_AVI,
		VIDEO_MPG,
		WM,
		PIC_JPG,
		PIC_PNG,
		PIC_GIF,
		DOCUMENT_PDF
};


#define ROUND(x) (floor((float)x+0.5f))

///////////////////////////////////////////////////////////////////////////////
// Low level str
//
__inline char* nstrdup(const char* const todup){
   size_t len = strlen(todup) + 1;
   return (char*)memcpy(new char[len], todup, len);
}

TCHAR *stristr(const TCHAR *str1, const TCHAR *str2);
CString GetNextString(const CString& rstr, LPCTSTR pszTokens, int& riStart);
CString GetNextString(const CString& rstr, TCHAR chToken, int& riStart);


///////////////////////////////////////////////////////////////////////////////
// String conversion
//
CString CastItoXBytes(uint16_t count, bool isK = false, bool isPerSec = false, uint32_t decimal = 2);
CString CastItoXBytes(uint32_t count, bool isK = false, bool isPerSec = false, uint32_t decimal = 2);
CString CastItoXBytes(uint64_t count, bool isK = false, bool isPerSec = false, uint32_t decimal = 2);
CString CastItoXBytes(float count, bool isK = false, bool isPerSec = false, uint32_t decimal = 2);
CString CastItoXBytes(double count, bool isK = false, bool isPerSec = false, uint32_t decimal = 2);
#if defined(_DEBUG) && defined(USE_DEBUG_EMFILESIZE)
CString CastItoXBytes(EMFileSize count, bool isK = false, bool isPerSec = false, uint32_t decimal = 2);
#endif
CString CastItoIShort(uint16_t count, bool isK = false, uint32_t decimal = 2);
CString CastItoIShort(uint32_t count, bool isK = false, uint32_t decimal = 2);
CString CastItoIShort(uint64_t count, bool isK = false, uint32_t decimal = 2);
CString CastItoIShort(float count, bool isK = false, uint32_t decimal = 2);
CString CastItoIShort(double count, bool isK = false, uint32_t decimal = 2);
CString CastSecondsToHM(time_t seconds);
CString	CastSecondsToLngHM(time_t seconds);
CString GetFormatedUInt(ULONG ulVal);
CString GetFormatedUInt64(ULONGLONG ullVal);
void SecToTimeLength(unsigned long ulSec, CStringA& rstrTimeLength);
void SecToTimeLength(unsigned long ulSec, CStringW& rstrTimeLength);
bool RegularExpressionMatch(CString regexpr, CString teststring);

///////////////////////////////////////////////////////////////////////////////
// URL conversion
//
CString URLDecode(const CString& sIn, bool bKeepNewLine = false);
CString URLEncode(const CString& sIn);
CString EncodeURLQueryParam(const CString& rstrQuery);
CString MakeStringEscaped(CString in);
CString RemoveAmbersand(const CString& rstr);
CString	StripInvalidFilenameChars(const CString& strText);
CString	CreateED2kLink(const CAbstractFile* f, bool bEscapeLink = true);
CString	CreateHTMLED2kLink(const CAbstractFile* f);


///////////////////////////////////////////////////////////////////////////////
// Hex conversion
//
CString EncodeBase32(const unsigned char* buffer, unsigned int bufLen);
CString EncodeBase16(const unsigned char* buffer, unsigned int bufLen);
unsigned int DecodeLengthBase16(unsigned int base16Length);
bool DecodeBase16(const TCHAR *base16Buffer, unsigned int base16BufLen, byte *buffer, unsigned int bufflen);
uint32_t DecodeBase32(LPCTSTR pszInput, uchar* paucOutput, uint32_t nBufferLen);
// ismod: no aich

///////////////////////////////////////////////////////////////////////////////
// File/Path string helpers
//
void MakeFoldername(CString &path);
CString RemoveFileExtension(const CString& rstrFilePath);
int CompareDirectories(const CString& rstrDir1, const CString& rstrDir2);
CString StringLimit(CString in, UINT length);
CString CleanupFilename(CString filename, bool bExtension = true);
CString ValidFilename(CString filename);
bool ExpandEnvironmentStrings(CString& rstrStrings);
int CompareLocaleString(LPCTSTR psz1, LPCTSTR psz2);
int CompareLocaleStringNoCase(LPCTSTR psz1, LPCTSTR psz2);
int __cdecl CompareCStringPtrLocaleString(const void* p1, const void* p2);
int __cdecl CompareCStringPtrLocaleStringNoCase(const void* p1, const void* p2);
void Sort(CStringArray& astr, int (__cdecl *pfnCompare)(const void*, const void*) = CompareCStringPtrLocaleStringNoCase);
int __cdecl CompareCStringPtrPtrLocaleString(const void* p1, const void* p2);
int __cdecl CompareCStringPtrPtrLocaleStringNoCase(const void* p1, const void* p2);
void		Sort(CSimpleArray<const CString*>& apstr, int (__cdecl *pfnCompare)(const void*, const void*) = CompareCStringPtrPtrLocaleStringNoCase);
void		StripTrailingCollon(CString& rstr);
bool		IsUnicodeFile(LPCTSTR pszFilePath);
UINT64		GetFreeTempSpace(int tempdirindex);
int			GetPathDriveNumber(CString path);
EFileType	GetFileTypeEx(CShareableFile* kfile, bool checkextention=true, bool checkfileheader=true, bool nocached=false);
CString		GetFileTypeName(EFileType ftype);
int			IsExtensionTypeOf(EFileType type, CString ext);
uint32_t		LevenshteinDistance(const CString& str1, const CString& str2);
bool		_tmakepathlimit(TCHAR *path, const TCHAR *drive, const TCHAR *dir, const TCHAR *fname, const TCHAR *ext);

///////////////////////////////////////////////////////////////////////////////
// GUI helpers
//
void InstallSkin(LPCTSTR pszSkinPackage);
bool CheckFileOpen(LPCTSTR pszFilePath, LPCTSTR pszFileTitle = NULL);
void ShellOpenFile(LPCTSTR name);
void ShellOpenFile(LPCTSTR name, LPCTSTR pszVerb);
bool ShellDeleteFile(LPCTSTR pszFilePath, bool bNoRecBin = false);
CString ShellGetFolderPath(int iCSIDL);
bool SelectDir(HWND hWnd, LPTSTR pszPath, LPCTSTR pszTitle = NULL, LPCTSTR pszDlgTitle = NULL);
BOOL DialogBrowseFile(CString& rstrPath, LPCTSTR pszFilters, LPCTSTR pszDefaultFileName = NULL, DWORD dwFlags = 0,bool openfilestyle=true);
void AddBuddyButton(HWND hwndEdit, HWND hwndButton);
bool SetButtonIcon(HWND hwndButton, HICON &ricoBrowse);
void GetPopupMenuPos(CListCtrl& lv, CPoint& point);
void GetPopupMenuPos(CTreeCtrl& tv, CPoint& point);
void InitWindowStyles(CWnd* pWnd);
CString GetRateString(UINT rate);
HWND ReplaceRichEditCtrl(CWnd* pwndRE, CWnd* pwndParent, CFont* pFont);
int  FontPointSizeToLogUnits(int nPointSize);
bool CreatePointFont(CFont &rFont, int nPointSize, LPCTSTR lpszFaceName);
bool CreatePointFontIndirect(CFont &rFont, const LOGFONT *lpLogFont);


///////////////////////////////////////////////////////////////////////////////
// Resource strings
//
#ifdef USE_STRING_IDS
#define	RESSTRIDTYPE		LPCTSTR
#define	IDS2RESIDTYPE(id)	#id
#define GetResString(id)	_GetResString(#id)
CString _GetResString(RESSTRIDTYPE StringID);
#else//USE_STRING_IDS
#define	RESSTRIDTYPE		UINT
#define	IDS2RESIDTYPE(id)	id
CString GetResString(RESSTRIDTYPE StringID);
#define _GetResString(id)	GetResString(id)
#endif//!USE_STRING_IDS
void InitThreadLocale();


///////////////////////////////////////////////////////////////////////////////
// Error strings, Debugging, Logging
//
int GetSystemErrorString(DWORD dwError, CString &rstrError);
int GetModuleErrorString(DWORD dwError, CString &rstrError, LPCTSTR pszModule);
int GetErrorMessage(DWORD dwError, CString &rstrErrorMsg, DWORD dwFlags = 0);
CString GetErrorMessage(DWORD dwError, DWORD dwFlags = 0);
LPCTSTR	GetShellExecuteErrMsg(DWORD dwShellExecError);
CString DbgGetHexDump(const uint8_t* data, UINT size);
void DbgSetThreadName(LPCSTR szThreadName, ...);
void Debug(LPCTSTR pszFmtMsg, ...);
void DebugHexDump(const uint8_t* data, UINT lenData);
void DebugHexDump(CFile& file);
CString DbgGetFileInfo(const uchar* hash);
CString DbgGetFileStatus(UINT nPartCount, CSafeMemFile* data);
LPCTSTR DbgGetHashTypeString(const uchar* hash);
CString DbgGetClientID(uint32_t nClientID);
int GetHashType(const uchar* hash);
CString DbgGetDonkeyClientTCPOpcode(UINT opcode);
CString DbgGetMuleClientTCPOpcode(UINT opcode);
CString DbgGetClientTCPOpcode(UINT protocol, UINT opcode);
CString DbgGetClientTCPPacket(UINT protocol, UINT opcode, UINT size);
CString DbgGetBlockInfo(const Requested_Block_Struct* block);
CString DbgGetBlockInfo(uint64_t StartOffset, uint64_t EndOffset);
CString DbgGetBlockFileInfo(const Requested_Block_Struct* block, const CPartFile* partfile);
CString DbgGetFileMetaTagName(UINT uMetaTagID);
CString DbgGetFileMetaTagName(LPCSTR pszMetaTagID);
CString DbgGetSearchOperatorName(UINT uOperator);
void DebugRecv(LPCSTR pszMsg, const CUpDownClient* client, const uchar* packet = NULL, uint32_t nIP = 0);
void DebugRecv(LPCSTR pszOpcode, uint32_t ip, uint16_t port);
void DebugSend(LPCSTR pszMsg, const CUpDownClient* client, const uchar* packet = NULL);
void DebugSend(LPCSTR pszOpcode, uint32_t ip, uint16_t port);
void DebugSendF(LPCSTR pszOpcode, uint32_t ip, uint16_t port, LPCTSTR pszMsg, ...);
void DebugHttpHeaders(const CStringAArray& astrHeaders);



///////////////////////////////////////////////////////////////////////////////
// Win32 specifics
//
bool Ask4RegFix(bool checkOnly, bool dontAsk = false, bool bAutoTakeCollections = false); // Barry - Allow forced update without prompt
void BackupReg(void); // Barry - Store previous values
void RevertReg(void); // Barry - Restore previous values
bool DoCollectionRegFix(bool checkOnly);
void AddAutoStart();
void RemAutoStart();
ULONGLONG GetModuleVersion(LPCTSTR pszFilePath);
ULONGLONG GetModuleVersion(HMODULE hModule);

#define _WINVER_XP_		0x0501	// 5.1
#define _WINVER_2003_	0x0502	// 5.2
#define _WINVER_VISTA_	0x0600	// 6.0
#define _WINVER_7_		0x0601	// 6.1
#define	_WINVER_S2008_	0x0601	// 6.1

WORD		DetectWinVersion();
int			IsRunningXPSP2();
uint64_t		GetFreeDiskSpaceX(LPCTSTR pDirectory);
ULONGLONG	GetDiskFileSize(LPCTSTR pszFilePath);
int			GetDesktopColorDepth();
bool		IsFileOnFATVolume(LPCTSTR pszFilePath);
void		ClearVolumeInfoCache(int iDrive = -1);
bool		AddIconGrayscaledToImageList(CImageList& rList, HICON hIcon);


///////////////////////////////////////////////////////////////////////////////
// MD4 helpers
//

__inline BYTE toHex(const BYTE &x){
	return x > 9 ? x + 55: x + 48;
}

// md4cmp -- replacement for memcmp(hash1,hash2,16)
// Like 'memcmp' this function returns 0, if hash1==hash2, and !0, if hash1!=hash2.
// NOTE: Do *NOT* use that function for determining if hash1<hash2 or hash1>hash2.
__inline int md4cmp(const void* const hash1, const void* const hash2) {
	return !(((uint64_t*)hash1)[0] == ((uint64_t*)hash2)[0] &&
		     ((uint64_t*)hash1)[1] == ((uint64_t*)hash2)[1]);
}

__inline bool isnulmd4(const void* const hash) {
	return  (((uint64_t*)hash)[0] == 0ULL &&
		     ((uint64_t*)hash)[1] == 0ULL);
}

// md4clr -- replacement for memset(hash,0,16)
__inline void md4clr(const void* const hash) {
	((uint64_t*)hash)[0] = ((uint64_t*)hash)[1] = 0ULL;
}

// md4cpy -- replacement for memcpy(dst,src,16)
/*
__inline void md4cpy(void* const dst, const void* const src) {
	((uint64*)dst)[0] = ((uint64*)src)[0];
	((uint64*)dst)[1] = ((uint64*)src)[1];
}
*/
typedef struct { BYTE hash[16]; } HashType;
__inline void md4cpy(void* dst, const void* src)
{
	*reinterpret_cast<HashType*>(dst) = *reinterpret_cast<const HashType*>(src);
}
// ismod [end]


#define	MAX_HASHSTR_SIZE (16*2+1)
CString md4str(const uchar* hash);
CStringA md4strA(const uchar* hash);
void md4str(const uchar* hash, TCHAR* pszHash);
void md4strA(const uchar* hash, CHAR* pszHash);
bool strmd4(const char* pszHash, uchar* hash);
bool strmd4(const CString& rstr, uchar* hash);


///////////////////////////////////////////////////////////////////////////////
// Compare helpers
//
__inline int CompareUnsigned(uint32_t uSize1, uint32_t uSize2)
{
	if (uSize1 < uSize2)
		return -1;
	if (uSize1 > uSize2)
		return 1;
	return 0;
}

__inline int CompareUnsignedUndefinedAtBottom(uint32_t uSize1, uint32_t uSize2, bool bSortAscending)
{
	if (uSize1 == 0 && uSize2 == 0)
		return 0;
	if (uSize1 == 0)
		return bSortAscending ? 1 : -1;
	if (uSize2 == 0)
		return bSortAscending ? -1 : 1;
	return CompareUnsigned(uSize1, uSize2);
}

__inline int CompareUnsigned64(uint64_t uSize1, uint64_t uSize2)
{
	if (uSize1 < uSize2)
		return -1;
	if (uSize1 > uSize2)
		return 1;
	return 0;
}

__inline int CompareFloat(float uSize1, float uSize2)
{
	if (uSize1 < uSize2)
		return -1;
	if (uSize1 > uSize2)
		return 1;
	return 0;
}

__inline int CompareOptLocaleStringNoCase(LPCTSTR psz1, LPCTSTR psz2)
{
	if (psz1 && psz2)
		return CompareLocaleStringNoCase(psz1, psz2);
	if (psz1)
		return -1;
	if (psz2)
		return 1;
	return 0;
}

__inline int CompareOptLocaleStringNoCaseUndefinedAtBottom(const CString &str1, const CString &str2, bool bSortAscending)
{
	if (str1.IsEmpty() && str2.IsEmpty())
		return 0;
	if (str1.IsEmpty())
		return bSortAscending ? 1 : -1;
	if (str2.IsEmpty())
		return bSortAscending ? -1 : 1;
	return CompareOptLocaleStringNoCase(str1, str2);
}


///////////////////////////////////////////////////////////////////////////////
// ED2K File Type
//
enum EED2KFileType
{
	ED2KFT_ANY				= 0,
	ED2KFT_AUDIO			= 1,	// ED2K protocol value (eserver 17.6+)
	ED2KFT_VIDEO			= 2,	// ED2K protocol value (eserver 17.6+)
	ED2KFT_IMAGE			= 3,	// ED2K protocol value (eserver 17.6+)
	ED2KFT_PROGRAM			= 4,	// ED2K protocol value (eserver 17.6+)
	ED2KFT_DOCUMENT			= 5,	// ED2K protocol value (eserver 17.6+)
	ED2KFT_ARCHIVE			= 6,	// ED2K protocol value (eserver 17.6+)
	ED2KFT_CDIMAGE			= 7,	// ED2K protocol value (eserver 17.6+)
	ED2KFT_EMULECOLLECTION	= 8
};

CString GetFileTypeByName(LPCTSTR pszFileName);
CString GetFileTypeDisplayStrFromED2KFileType(LPCTSTR pszED2KFileType);
LPCSTR GetED2KFileTypeSearchTerm(EED2KFileType iFileID);
EED2KFileType GetED2KFileTypeSearchID(EED2KFileType iFileID);
EED2KFileType GetED2KFileTypeID(LPCTSTR pszFileName);
bool gotostring(CFile &file, const uchar *find, LONGLONG plen);

///////////////////////////////////////////////////////////////////////////////
// IP/UserID
//
void TriggerPortTest(uint16_t tcp, uint16_t udp);
bool IsGoodIP(uint32_t nIP, bool forceCheck = false);
bool IsGoodIPPort(uint32_t nIP, uint16_t nPort);
bool IsLANIP(uint32_t nIP);
uint8_t GetMyConnectOptions(bool bEncryption = true, bool bCallback = true);
//No longer need seperate lowID checks as we now know the servers just give *.*.*.0 users a lowID
__inline bool IsLowID(uint32_t id){
	return (id < 16777216);
}
CString ipstr(uint32_t nIP);
CString ipstr(uint32_t nIP, uint16_t nPort);
CString ipstr(LPCTSTR pszAddress, uint16_t nPort);
CStringA ipstrA(uint32_t nIP);
void ipstrA(CHAR* pszAddress, int iMaxAddress, uint32_t nIP);
__inline CString ipstr(in_addr nIP){
	return ipstr(*(uint32_t*)&nIP);
}
__inline CStringA ipstrA(in_addr nIP){
	return ipstrA(*(uint32_t*)&nIP);
}


///////////////////////////////////////////////////////////////////////////////
// Date/Time
//
time_t safe_mktime(struct tm* ptm);
bool AdjustNTFSDaylightFileTime(time_t& ruFileDate, LPCTSTR pszFilePath);


///////////////////////////////////////////////////////////////////////////////
// Random Numbers
//
uint16_t GetRandomUInt16();
uint32_t GetRandomUInt32();

///////////////////////////////////////////////////////////////////////////////
// RC4 Encryption
//
struct RC4_Key_Struct{
	uint8_t abyState[256];
	uint8_t byX;
	uint8_t byY;
};

RC4_Key_Struct* RC4CreateKey(const uchar* const pachKeyData, const size_t nLen, RC4_Key_Struct* key = NULL, const bool bSkipDiscard = false);
void RC4Crypt(const uchar* const pachIn, uchar* const pachOut, const size_t nLen, RC4_Key_Struct* const key);

// netfinity: Convert between 32 and 64 bit time values
///////////////////////////////////////////////////////////////////////////////
// 64bit support
//
inline
time_t ConvertFromTime32(const uint32_t uTime)
{
#if defined(_TIME64_T_DEFINED) && !defined(_USE_32BIT_TIME_T)
	const time_t	now = time(NULL);
	time_t	uTimeNew = (now & 0xFFFFFFFF00000000LL) | static_cast<time_t>(static_cast<uint64_t>(uTime));
	// Just in case eMule would still live in the 22nd century
	if (uTimeNew > (now + 0x80000000LL))
		uTimeNew -= 0x100000000LL;
	return uTimeNew;
#else
	return static_cast<time_t>(uTime);
#endif
}

inline
uint32_t ConvertToTime32(const time_t uTime)
{
	return static_cast<uint32_t>(uTime);
}

// ismod [start]
bool HasSubnetMark(const CString& rstr);
CString CreateBBCODEED2kLink(const CAbstractFile* file);
void LogPacket(void* pPacket, uint32_t uPacketSize, uint32_t src, uint32_t dst);

namespace Utils {

void fixNationalChars( CString &rstr );

}
