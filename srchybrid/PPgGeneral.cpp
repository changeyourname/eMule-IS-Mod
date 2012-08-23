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

#include "stdafx.h"
#include "langids.h"
#include "emule.h"
#include "SearchDlg.h"
#include "PreferencesDlg.h"
#include "PPgGeneral.h"
#include "HttpDownloadDlg.h"
#include "Preferences.h"
#include "emuledlg.h"
#include "StatisticsDlg.h"
#include "ServerWnd.h"
#include "TransferDlg.h"
#include "ChatWnd.h"
#include "SharedFilesWnd.h"
#include "KademliaWnd.h"
#include "IrcWnd.h"
#include "WebServices.h"
#include "StringConversion.h"
#include "Log.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CPPgGeneral, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgGeneral, CPropertyPage)
	ON_BN_CLICKED(IDC_STARTMIN, OnSettingsChange)
	ON_BN_CLICKED(IDC_STARTWIN, OnSettingsChange)
	ON_EN_CHANGE(IDC_NICK, OnSettingsChange)
	ON_BN_CLICKED(IDC_EXIT, OnSettingsChange)
	ON_BN_CLICKED(IDC_SPLASHON, OnSettingsChange)
	ON_BN_CLICKED(IDC_BRINGTOFOREGROUND, OnSettingsChange)
	ON_CBN_SELCHANGE(IDC_LANGS, OnLangChange)
	ON_BN_CLICKED(IDC_ED2KFIX, OnBnClickedEd2kfix)
	ON_BN_CLICKED(IDC_WEBSVEDIT , OnBnClickedEditWebservices)
	ON_BN_CLICKED(IDC_ONLINESIG, OnSettingsChange)
	ON_BN_CLICKED(IDC_CHECK4UPDATE, OnBnClickedCheck4Update)
    ON_BN_CLICKED(IDC_ASK4UPDATE, OnBnClickedCheck4Update)
	ON_BN_CLICKED(IDC_MINIMULE, OnSettingsChange)
	ON_BN_CLICKED(IDC_PREVENTSTANDBY, OnSettingsChange)
	ON_WM_HSCROLL()
	ON_WM_HELPINFO()
END_MESSAGE_MAP()

CPPgGeneral::CPPgGeneral()
	: CPropertyPage(CPPgGeneral::IDD)
{
}

CPPgGeneral::~CPPgGeneral()
{
}

void CPPgGeneral::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LANGS, m_language);
}

void CPPgGeneral::LoadSettings(void)
{
	SetDlgItemText(IDC_NICK, thePrefs.GetUserNick());

	for(int i = 0; i < m_language.GetCount(); i++)
		if(m_language.GetItemData(i) == thePrefs.GetLanguageID())
			m_language.SetCurSel(i);
	
	CheckDlgButton(IDC_STARTWIN, thePrefs.m_bAutoStart ? 1 : 0);
	CheckDlgButton(IDC_STARTMIN, thePrefs.startMinimized ? 1 : 0);
	CheckDlgButton(IDC_ONLINESIG, thePrefs.onlineSig? 1 : 0);
	CheckDlgButton(IDC_EXIT, thePrefs.confirmExit ? 1 : 0 );
	CheckDlgButton(IDC_SPLASHON, thePrefs.splashscreen ? 1 : 0);
	CheckDlgButton(IDC_BRINGTOFOREGROUND,thePrefs.bringtoforeground ? 1 : 0);
	CheckDlgButton(IDC_CHECK4UPDATE, thePrefs.m_checkForUpdates ? 1 : 0);
    CheckDlgButton(IDC_ASK4UPDATE, thePrefs.m_askForUpdateInstall ? 1 : 0);
	CheckDlgButton(IDC_MINIMULE, thePrefs.m_bEnableMiniMule ? 1 : 0);
	CheckDlgButton(IDC_PREVENTSTANDBY, thePrefs.GetPreventStandby() ? 1 : 0);

	CString strBuffer;
	strBuffer.Format(_T("%u %s"), thePrefs.m_versionCheckDays, GetResString(IDS_DAYS2));
	SetDlgItemText(IDC_DAYS, strBuffer);
}

BOOL CPPgGeneral::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	InitWindowStyles(this);

	((CEdit*)GetDlgItem(IDC_NICK))->SetLimitText(thePrefs.GetMaxUserNickLength());

	CWordArray aLanguageIDs;
	thePrefs.GetLanguages(aLanguageIDs);
	for (int i = 0; i < aLanguageIDs.GetSize(); i++){
		TCHAR szLang[128];
		int ret=GetLocaleInfo(aLanguageIDs[i], LOCALE_SLANGUAGE, szLang, _countof(szLang));

		if (ret==0)
			switch(aLanguageIDs[i]) {
				case LANGID_UG_CN:
					_tcscpy(szLang,_T("Uyghur") );
					break;
				case LANGID_GL_ES:
					_tcscpy(szLang,_T("Galician") );
					break;
				case LANGID_FR_BR:
					_tcscpy(szLang,_T("Breton (Brezhoneg)") );
					break;
				case LANGID_MT_MT:
					_tcscpy(szLang,_T("Maltese") );
					break;
				case LANGID_ES_AS:
					_tcscpy(szLang,_T("Asturian") );
					break;
				case LANGID_VA_ES:
					_tcscpy(szLang,_T("Valencian") );
					break;
				case LANGID_VA_ES_RACV:
					_tcscpy(szLang, _T("Valencian (RACV)"));
					break;
				default:
					ASSERT(0);
					_tcscpy(szLang,_T("?(unknown language)?") );
			}

		m_language.SetItemData(m_language.AddString(szLang), aLanguageIDs[i]);
	}

	UpdateEd2kLinkFixCtrl();

	CSliderCtrl *sliderUpdate = (CSliderCtrl*)GetDlgItem(IDC_CHECKDAYS);
	sliderUpdate->SetRange(2, 7, true);
	sliderUpdate->SetPos(thePrefs.versionCheckDays());
	
	LoadSettings();
	Localize();
	GetDlgItem(IDC_CHECKDAYS)->ShowWindow( IsDlgButtonChecked(IDC_CHECK4UPDATE) ? SW_SHOW : SW_HIDE );
	GetDlgItem(IDC_DAYS)->ShowWindow( IsDlgButtonChecked(IDC_CHECK4UPDATE) ? SW_SHOW : SW_HIDE );

	return TRUE;
}

void ModifyAllWindowStyles(CWnd* pWnd, DWORD dwRemove, DWORD dwAdd)
{
	CWnd* pWndChild = pWnd->GetWindow(GW_CHILD);
	while (pWndChild)
	{
		ModifyAllWindowStyles(pWndChild, dwRemove, dwAdd);
		pWndChild = pWndChild->GetNextWindow();
	}

	if (pWnd->ModifyStyleEx(dwRemove, dwAdd, SWP_FRAMECHANGED))
	{
		pWnd->Invalidate();
//		pWnd->UpdateWindow();
	}
}

BOOL CPPgGeneral::OnApply()
{
	CString strNick;
	GetDlgItem(IDC_NICK)->GetWindowText(strNick);
	strNick.Trim();
	if (!IsValidEd2kString(strNick))
		strNick.Empty();
	if (strNick.IsEmpty())
	{
		strNick = DEFAULT_NICK;
		GetDlgItem(IDC_NICK)->SetWindowText(strNick);
	}
	thePrefs.SetUserNick(strNick);

	if (m_language.GetCurSel() != CB_ERR)
	{
		WORD wNewLang = (WORD)m_language.GetItemData(m_language.GetCurSel());
		if (thePrefs.GetLanguageID() != wNewLang)
		{
			thePrefs.SetLanguageID(wNewLang);
			thePrefs.SetLanguage();

#ifdef _DEBUG
			// Can't yet be switched on-the-fly, too much unresolved issues..
			if (thePrefs.GetRTLWindowsLayout())
			{
				ModifyAllWindowStyles(theApp.emuledlg, WS_EX_LAYOUTRTL | WS_EX_RTLREADING | WS_EX_RIGHT | WS_EX_LEFTSCROLLBAR, 0);
				ModifyAllWindowStyles(theApp.emuledlg->preferenceswnd, WS_EX_LAYOUTRTL | WS_EX_RTLREADING | WS_EX_RIGHT | WS_EX_LEFTSCROLLBAR, 0);
				theApp.DisableRTLWindowsLayout();
				thePrefs.m_bRTLWindowsLayout = false;
			}
#endif
			theApp.emuledlg->preferenceswnd->Localize();
			theApp.emuledlg->statisticswnd->CreateMyTree();
			theApp.emuledlg->statisticswnd->Localize();
			theApp.emuledlg->statisticswnd->ShowStatistics(true);
			theApp.emuledlg->serverwnd->Localize();
			theApp.emuledlg->transferwnd->Localize();
			theApp.emuledlg->transferwnd->UpdateCatTabTitles();
			theApp.emuledlg->searchwnd->Localize();
			theApp.emuledlg->sharedfileswnd->Localize();
			theApp.emuledlg->chatwnd->Localize();
			theApp.emuledlg->Localize();
			theApp.emuledlg->ircwnd->Localize();
			/* no KAD
			theApp.emuledlg->kademliawnd->Localize();
			*/
		}
	}

	thePrefs.startMinimized = IsDlgButtonChecked(IDC_STARTMIN)!=0;
	thePrefs.m_bAutoStart = IsDlgButtonChecked(IDC_STARTWIN)!=0;
	if( thePrefs.m_bAutoStart )
		AddAutoStart();
	else
		RemAutoStart();
	thePrefs.confirmExit = IsDlgButtonChecked(IDC_EXIT)!=0;
	thePrefs.splashscreen = IsDlgButtonChecked(IDC_SPLASHON)!=0;
	thePrefs.bringtoforeground = IsDlgButtonChecked(IDC_BRINGTOFOREGROUND)!=0;
	thePrefs.m_checkForUpdates = IsDlgButtonChecked(IDC_CHECK4UPDATE)!=0;
    thePrefs.m_askForUpdateInstall = IsDlgButtonChecked(IDC_CHECK4UPDATE)!=0;
	thePrefs.onlineSig = IsDlgButtonChecked(IDC_ONLINESIG)!=0;
	thePrefs.m_versionCheckDays = (uint32_t) static_cast<CSliderCtrl*>(GetDlgItem(IDC_CHECKDAYS))->GetPos();
	thePrefs.m_bEnableMiniMule = IsDlgButtonChecked(IDC_MINIMULE) != 0;
	thePrefs.m_bPreventStandby = IsDlgButtonChecked(IDC_PREVENTSTANDBY) != 0;

	LoadSettings();

	SetModified(FALSE);
	return CPropertyPage::OnApply();
}

void CPPgGeneral::UpdateEd2kLinkFixCtrl()
{
	GetDlgItem(IDC_ED2KFIX)->EnableWindow(Ask4RegFix(true, false, true));
}

BOOL CPPgGeneral::OnSetActive()
{
	UpdateEd2kLinkFixCtrl();
	return __super::OnSetActive();
}

void CPPgGeneral::OnBnClickedEd2kfix()
{
	Ask4RegFix(false, false, true);
	GetDlgItem(IDC_ED2KFIX)->EnableWindow(Ask4RegFix(true));
}

void CPPgGeneral::Localize(void)
{
	if(m_hWnd)
	{
		SetWindowText(GetResString(IDS_PW_GENERAL));
		SetDlgItemText(IDC_NICK_FRM, GetResString(IDS_QL_USERNAME));
		SetDlgItemText(IDC_LANG_FRM, GetResString(IDS_PW_LANG));
		SetDlgItemText(IDC_MISC_FRM, GetResString(IDS_PW_MISC));
		SetDlgItemText(IDC_EXIT, GetResString(IDS_PW_PROMPT));
		SetDlgItemText(IDC_SPLASHON, GetResString(IDS_PW_SPLASH));
		SetDlgItemText(IDC_BRINGTOFOREGROUND, GetResString(IDS_PW_FRONT));
		SetDlgItemText(IDC_ONLINESIG, GetResString(IDS_PREF_ONLINESIG));	
		SetDlgItemText(IDC_STARTMIN, GetResString(IDS_PREF_STARTMIN));	
		SetDlgItemText(IDC_WEBSVEDIT, GetResString(IDS_WEBSVEDIT));
		SetDlgItemText(IDC_ED2KFIX, GetResString(IDS_ED2KLINKFIX));
		SetDlgItemText(IDC_CHECK4UPDATE, GetResString(IDS_CHECK4UPDATE));
		SetDlgItemText(IDC_STARTUP, GetResString(IDS_STARTUP));
		SetDlgItemText(IDC_STARTWIN, GetResString(IDS_STARTWITHWINDOWS));
		SetDlgItemText(IDC_MINIMULE, GetResString(IDS_ENABLEMINIMULE));
		SetDlgItemText(IDC_PREVENTSTANDBY, GetResString(IDS_PREVENTSTANDBY));
        SetDlgItemText(IDC_ASK4UPDATE, LTR(L"Ask before installing new version"));
	}
}

void CPPgGeneral::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	SetModified(TRUE);

	if (GetDlgItem(IDC_CHECKDAYS) == pScrollBar ) {
		CSliderCtrl* slider =(CSliderCtrl*)pScrollBar;
		CString text;
		text.Format(_T("%u %s"), slider->GetPos(), GetResString(IDS_DAYS2));
		SetDlgItemText(IDC_DAYS, text);
	}

	UpdateData(false); 
	CPropertyPage::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CPPgGeneral::OnBnClickedEditWebservices()
{
	theWebServices.Edit();
}

void CPPgGeneral::OnLangChange()
{
	WORD byNewLang = (WORD)m_language.GetItemData(m_language.GetCurSel());
	if ( thePrefs.GetLanguageID() != byNewLang ) {
		if ( !thePrefs.IsLanguageSupported(byNewLang, false) ) {
            AfxMessageBox(_T("Выбранный язык не найден.\r\nПереустановите программу."));
		} else 
			OnSettingsChange();
	}
}

void CPPgGeneral::OnBnClickedCheck4Update()
{
	SetModified();
    UINT checked = IsDlgButtonChecked(IDC_CHECK4UPDATE);
	GetDlgItem(IDC_CHECKDAYS)->ShowWindow( checked ? SW_SHOW : SW_HIDE );
	GetDlgItem(IDC_DAYS)->ShowWindow( checked ? SW_SHOW : SW_HIDE );
    GetDlgItem(IDC_ASK4UPDATE)->ShowWindow( checked ? SW_SHOW : SW_HIDE );
}

BOOL CPPgGeneral::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (ID_HELP == wParam ) {
		OnHelp();
		return TRUE;
	}

	return __super::OnCommand(wParam, lParam);
}

BOOL CPPgGeneral::OnHelpInfo(HELPINFO* /*pHelpInfo*/)
{
	OnHelp();
	return TRUE;
}
