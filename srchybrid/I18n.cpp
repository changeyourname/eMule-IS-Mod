#include "stdafx.h"
#include <locale.h>
#include "emule.h"
#include "OtherFunctions.h"
#include "Preferences.h"
#include "langids.h"
#include "Version.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static HINSTANCE s_hLangDLL = NULL;

CString GetResString(UINT uStringID, WORD wLanguageID)
{
	CString resString;
	if (s_hLangDLL)
		(void)resString.LoadString(s_hLangDLL, uStringID, wLanguageID);
	if (resString.IsEmpty())
		(void)resString.LoadString(GetModuleHandle(NULL), uStringID, LANGID_EN_US);
	return resString;
}

CString GetResString(UINT uStringID)
{
	CString resString;
	if (s_hLangDLL)
		resString.LoadString(s_hLangDLL, uStringID);
	if (resString.IsEmpty())
		resString.LoadString(GetModuleHandle(NULL), uStringID);
	return resString;
}

struct SLanguage {
	LANGID	lid;
	LPCTSTR pszLocale;
	BOOL	bSupported;
	LPCTSTR	pszISOLocale;
	UINT	uCodepage;
	LPCTSTR	pszHtmlCharset;
};

static SLanguage s_aLanguages[] =
{
	{LANGID_EN_US,	_T("english"),		TRUE,	_T("en_US"),	1252,	_T("windows-1252")},	// English
	{LANGID_RU_RU,	_T("russian"),		FALSE,	_T("ru_RU"),	1251,	_T("windows-1251")},	// Russian
	{0, NULL, 0, 0}
};

/**
 * @brief enumerates all lang dlls in langDir directory
 */
static void InitLanguages( const CString & langDir, bool reInit = false )
{
	static bool _bInitialized = false;
	if (_bInitialized && !reInit)
		return;
	_bInitialized = true;

	CFileFind ff;
	BOOL hasFiles = ff.FindFile(langDir + _T("*.dll"));

	while ( hasFiles )
	{
        hasFiles = ff.FindNextFile();

		if ( ff.IsDirectory() )
			continue;

        TCHAR szLandDLLFileName[_MAX_FNAME];
		_tsplitpath_s(ff.GetFileName(), NULL, 0, NULL, 0, szLandDLLFileName, _countof(szLandDLLFileName), NULL, 0);

		SLanguage* pLangs = s_aLanguages;
		if ( pLangs ) {
			while ( pLangs->lid ) {
				if ( _tcsicmp(pLangs->pszISOLocale, szLandDLLFileName) == 0 ) {
					pLangs->bSupported = TRUE;
					break;
				}
				pLangs++;
			}
		}
	}
	ff.Close();
}

static void FreeLangDLL()
{
	if (s_hLangDLL != NULL && s_hLangDLL != GetModuleHandle(NULL)){
		VERIFY( FreeLibrary(s_hLangDLL) );
		s_hLangDLL = NULL;
	}
}

void CPreferences::GetLanguages(CWordArray& aLanguageIDs)
{
	const SLanguage* pLang = s_aLanguages;
	while (pLang->lid){
		//if (pLang->bSupported)
		//show all languages, offer download if not supported ones later
		aLanguageIDs.Add(pLang->lid);
		pLang++;
	}
}

WORD CPreferences::GetLanguageID()
{
	return m_wLanguageID;
}

void CPreferences::SetLanguageID(WORD lid)
{
	m_wLanguageID = lid;
}

static bool CheckLangDLLVersion(const CString& rstrLangDLL)
{
	bool bResult = false;
	DWORD dwUnused;
	DWORD dwVerInfSize = GetFileVersionInfoSize(const_cast<LPTSTR>((LPCTSTR)rstrLangDLL), &dwUnused);
	if (dwVerInfSize != 0)
	{
		LPBYTE pucVerInf = (LPBYTE)calloc(dwVerInfSize, 1);
		if (pucVerInf)
		{
			if (GetFileVersionInfo(const_cast<LPTSTR>((LPCTSTR)rstrLangDLL), 0, dwVerInfSize, pucVerInf))
			{
				VS_FIXEDFILEINFO* pFileInf = NULL;
				UINT uLen = 0;
				if (VerQueryValue(pucVerInf, _T("\\"), (LPVOID*)&pFileInf, &uLen) && pFileInf && uLen)
				{
					bResult = (pFileInf->dwProductVersionMS == MAKELONG(IS_VERSION_MIN, IS_VERSION_MJR) &&
                               pFileInf->dwProductVersionLS == MAKELONG(IS_VERSION_BLD, IS_VERSION_UPD));
				}
			}
			free(pucVerInf);
		}
	}

	return bResult;
}

static bool LoadLangLib( const CString& langDir, LANGID lid )
{
	const SLanguage* pLangs = s_aLanguages;
	if ( pLangs ) {
		while ( pLangs->lid ) {
			if ( pLangs->bSupported && pLangs->lid == lid ) {
				FreeLangDLL();

				if ( pLangs->lid == LANGID_EN_US ) {
					s_hLangDLL = NULL;
					return true;
				} else {
					CString strLangDLL = langDir;
					strLangDLL += pLangs->pszISOLocale;
					strLangDLL += _T(".dll");
					if ( CheckLangDLLVersion(strLangDLL) ) {
						s_hLangDLL = LoadLibrary(strLangDLL);
						if ( s_hLangDLL )
							return true;
					}
				}
			}
			pLangs++;
		}
	}

	return false;
}

void CPreferences::SetLanguage()
{
	InitLanguages(GetMuleDirectory(EMULE_LANGDIR));

	bool bFoundLang = false;
	if (m_wLanguageID)
		bFoundLang = LoadLangLib(GetMuleDirectory(EMULE_LANGDIR), m_wLanguageID);

	if (!bFoundLang){
		LANGID lidLocale = (LANGID)::GetThreadLocale();
		//LANGID lidLocalePri = PRIMARYLANGID(::GetThreadLocale());
		//LANGID lidLocaleSub = SUBLANGID(::GetThreadLocale());

		bFoundLang = LoadLangLib(GetMuleDirectory(EMULE_LANGDIR), lidLocale);
		if (!bFoundLang){
			LoadLangLib(GetMuleDirectory(EMULE_LANGDIR), LANGID_EN_US);
			m_wLanguageID = LANGID_EN_US;
			CString strLngEnglish = GetResString(IDS_MB_LANGUAGEINFO);
			AfxMessageBox(strLngEnglish, MB_ICONASTERISK);
		} else {
			m_wLanguageID = lidLocale;
        }
	}

    if ( bFoundLang ) {
        CString langFile;
        langFile.Format(_T("%s%u.lng"), GetMuleDirectory(EMULE_LANGDIR), m_wLanguageID);
        l10n::loadLang( langFile );
    }

	// if loading a string fails, set language to English
	if (GetResString(IDS_MB_LANGUAGEINFO).IsEmpty()) {
		LoadLangLib(GetMuleDirectory(EMULE_LANGDIR), LANGID_EN_US);
		m_wLanguageID = LANGID_EN_US;
	}

	InitThreadLocale();
}

bool CPreferences::IsLanguageSupported( LANGID lid, bool bUpdateBefore )
{
	InitLanguages(GetMuleDirectory(EMULE_LANGDIR), bUpdateBefore);

	if (lid == LANGID_EN_US)
		return true;
	const SLanguage* pLang = s_aLanguages;
	for (; pLang->lid; pLang++ ) {
		if ( pLang->lid == lid && pLang->bSupported ) {
			return CheckLangDLLVersion(GetMuleDirectory(EMULE_LANGDIR) + CString(pLang->pszISOLocale) + _T(".dll"));
		}
	}
	return false; 
}

CString CPreferences::GetLangDLLNameByID( LANGID lid )
{
	const SLanguage* pLang = s_aLanguages;
	for (;pLang->lid;pLang++){
		if (pLang->lid == lid)
			return CString(pLang->pszISOLocale) + _T(".dll"); 
	}
	ASSERT ( false );
	return CString(_T(""));
}

void CPreferences::SetRtlLocale( LCID lcid )
{
	const SLanguage* pLangs = s_aLanguages;
	while (pLangs->lid)
	{
		if (pLangs->lid == LANGIDFROMLCID(lcid))
		{
			if (pLangs->uCodepage)
			{
				CString strCodepage;
				strCodepage.Format(_T(".%u"), pLangs->uCodepage);
				_tsetlocale(LC_CTYPE, strCodepage);
			}
			break;
		}
		pLangs++;
	}
}

void CPreferences::InitThreadLocale()
{
	ASSERT( m_wLanguageID != 0 );

	// NOTE: This function is for testing multi language support only.
	// NOTE: This function is *NOT* to be enabled in release builds nor to be offered by any Mod!
	if (theApp.GetProfileInt(_T("eMule"), _T("SetLanguageACP"), 0) != 0)
	{
		//LCID lcidUser = GetUserDefaultLCID();		// Installation, or altered by user in control panel (WinXP)

		// get the ANSI code page which is to be used for all non-Unicode conversions.
		LANGID lidSystem = m_wLanguageID;

		// get user's sorting preferences
		//UINT uSortIdUser = SORTIDFROMLCID(lcidUser);
		//UINT uSortVerUser = SORTVERSIONFROMLCID(lcidUser);
		// we can't use the same sorting paramters for 2 different Languages..
		UINT uSortIdUser = SORT_DEFAULT;
		UINT uSortVerUser = 0;

		// set thread locale, this is used for:
		//	- MBCS->Unicode conversions (e.g. search results).
		//	- Unicode->MBCS conversions (e.g. publishing local files (names) in network, or savint text files on local disk)...
		LCID lcid = MAKESORTLCID(lidSystem, uSortIdUser, uSortVerUser);
		SetThreadLocale(lcid);

		// if we set the thread locale (see comments above) we also have to specify the proper
		// code page for the C-RTL, otherwise we may not be able to store some strings as MBCS
		// (Unicode->MBCS conversion may fail)
		SetRtlLocale(lcid);
	}
}

void InitThreadLocale()
{
	thePrefs.InitThreadLocale();
}

CString GetCodePageNameForLocale(LCID lcid)
{
	CString strCodePage;
	int iResult = GetLocaleInfo(lcid, LOCALE_IDEFAULTANSICODEPAGE, strCodePage.GetBuffer(6), 6);
	strCodePage.ReleaseBuffer();

	if (iResult > 0 && !strCodePage.IsEmpty())
	{
		UINT uCodePage = _tcstoul(strCodePage, NULL, 10);
		if (uCodePage != ULONG_MAX)
		{
			CPINFOEXW CPInfoEx = {0};
			GetCPInfoEx(uCodePage, 0, &CPInfoEx);
			strCodePage = CPInfoEx.CodePageName;
		}
	}
	return strCodePage;
}

CString CPreferences::GetHtmlCharset()
{
	ASSERT( m_wLanguageID != 0 );

	LPCTSTR pszHtmlCharset = NULL;
	const SLanguage* pLangs = s_aLanguages;
	while (pLangs->lid)
	{
		if (pLangs->lid == m_wLanguageID)
		{
			pszHtmlCharset = pLangs->pszHtmlCharset;
			break;
		}
		pLangs++;
	}

	if (pszHtmlCharset == NULL || pszHtmlCharset[0] == _T('\0'))
	{
		ASSERT(0); // should never come here

		// try to get charset from code page
		LPCTSTR pszLcLocale = _tsetlocale(LC_CTYPE, NULL);
		if (pszLcLocale)
		{
			TCHAR szLocaleID[128];
			UINT uCodepage = 0;
			if (_stscanf(pszLcLocale, _T("%[a-zA-Z_].%u"), szLocaleID, &uCodepage) == 2 && uCodepage != 0)
			{
				CString strHtmlCodepage;
				strHtmlCodepage.Format(_T("windows-%u"), uCodepage);
				return strHtmlCodepage;
			}
		}
	}

	return pszHtmlCharset;
}

static HHOOK s_hRTLWindowsLayoutOldCbtFilterHook = NULL;

LRESULT CALLBACK RTLWindowsLayoutCbtFilterHook(int code, WPARAM wParam, LPARAM lParam)
{
	if (code == HCBT_CREATEWND)
	{
		//LPCREATESTRUCT lpcs = ((LPCBT_CREATEWND)lParam)->lpcs;

		//if ((lpcs->style & WS_CHILD) == 0)
		//	lpcs->dwExStyle |= WS_EX_LAYOUTRTL;	// doesn't seem to have any effect, but shouldn't hurt

		HWND hWnd = (HWND)wParam;
		if ((GetWindowLong(hWnd, GWL_STYLE) & WS_CHILD) == 0) {
			SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYOUTRTL);
		}
	}
	return CallNextHookEx(s_hRTLWindowsLayoutOldCbtFilterHook, code, wParam, lParam);
}

void CemuleApp::EnableRTLWindowsLayout()
{
	SetProcessDefaultLayout(LAYOUT_RTL);
	s_hRTLWindowsLayoutOldCbtFilterHook = SetWindowsHookEx(WH_CBT, RTLWindowsLayoutCbtFilterHook, NULL, GetCurrentThreadId());
}

void CemuleApp::DisableRTLWindowsLayout()
{
	if (s_hRTLWindowsLayoutOldCbtFilterHook)
	{
		VERIFY( UnhookWindowsHookEx(s_hRTLWindowsLayoutOldCbtFilterHook) );
		s_hRTLWindowsLayoutOldCbtFilterHook = NULL;
		SetProcessDefaultLayout(0);
	}
}
