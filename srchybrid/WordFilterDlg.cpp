//this file is part of eMule IS Mod
//Copyright (C)2011 eMule IS Mod Team
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
#include "emule.h"
#include "OtherFunctions.h"
#include "WordFilterDlg.h"
#include "Preferences.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CWordFilterDlg, CDialog)

CWordFilterDlg::CWordFilterDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_MOD_WORDFILTER, pParent)
{
}

CWordFilterDlg::~CWordFilterDlg()
{
}

void CWordFilterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

}

BEGIN_MESSAGE_MAP(CWordFilterDlg, CDialog)
	ON_BN_CLICKED(IDC_MOD_UNLOCK, &CWordFilterDlg::OnBnClickedUnlock)
END_MESSAGE_MAP()

BOOL CWordFilterDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	//localize
	SetWindowText(GetResString(IDS_MOD_WORDFILTER_TITLE));
	SetDlgItemText(IDC_MOD_STATIC_PASSWD, GetResString(IDS_WS_PASS));
	SetDlgItemText(IDC_MOD_WORDHINT, GetResString(IDS_MOD_WORDHINT));
	SetDlgItemText(IDC_MOD_WMSG, GetResString(IDS_MOD_WMSG));
	SetDlgItemText(IDCANCEL, GetResString(IDS_CANCEL));

	m_bFilterLocked = thePrefs.m_bWordFilterLocked;
	m_strPassword = thePrefs.m_strWordFilterPass;
	
	LoadWords();
	
	if(m_bFilterLocked)
	{
		GetDlgItem(IDC_MOD_WORDS)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_MOD_WMSG)->ShowWindow(SW_SHOW);
		SetDlgItemText(IDC_MOD_UNLOCK, GetResString(IDS_MOD_UNLOCK));
	}else{
		GetDlgItem(IDC_MOD_WORDS)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_MOD_WMSG)->ShowWindow(SW_HIDE);
		SetDlgItemText(IDC_MOD_UNLOCK, GetResString(IDS_MOD_LOCK));
	}

	return TRUE;
}

void CWordFilterDlg::OnBnClickedUnlock()
{
	if(m_bFilterLocked)	
	{
		CString strNewPass;
		GetDlgItemText(IDC_MOD_PASS, strNewPass);
		if(m_strPassword != strNewPass)
		{
			MessageBox(GetResString(IDS_MOD_INVPASS));
			return;
		}
		GetDlgItem(IDC_MOD_WORDS)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_MOD_WMSG)->ShowWindow(SW_HIDE);
		SetDlgItemText(IDC_MOD_UNLOCK, GetResString(IDS_MOD_LOCK));
	}else
	{
		CString strNewPass;
		GetDlgItemText(IDC_MOD_PASS, strNewPass);
		m_strPassword = strNewPass;
		GetDlgItem(IDC_MOD_WORDS)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_MOD_WMSG)->ShowWindow(SW_SHOW);
		SetDlgItemText(IDC_MOD_UNLOCK, GetResString(IDS_MOD_UNLOCK));
	}
	m_bFilterLocked = !m_bFilterLocked;
}

void CWordFilterDlg::OnOK()
{
	thePrefs.m_strWordFilterPass = m_strPassword;
	thePrefs.m_bWordFilterLocked = m_bFilterLocked;

	CString strWords;
	GetDlgItemText(IDC_MOD_WORDS, strWords);
	strWords.MakeLower();

	thePrefs.m_strAdultWordsArray.RemoveAll();
	
	int curPos = 0;
	CString resToken = strWords.Tokenize(_T("\r\n"),curPos);
	while (resToken != _T(""))
	{
		thePrefs.m_strAdultWordsArray.Add(resToken);
		resToken = strWords.Tokenize(_T("\r\n"), curPos);
	}

	thePrefs.m_strAdultWordsArray.Sort();

	CDialog::OnOK();
}

void CWordFilterDlg::LoadWords()
{
	CString strWords;
	for(int pos = 0; pos <= thePrefs.m_strAdultWordsArray.GetUpperBound(); ++pos)	
		strWords += thePrefs.m_strAdultWordsArray.GetAt(pos) + _T("\r\n");

	GetDlgItem(IDC_MOD_WORDS)->SetWindowText(strWords);
}

