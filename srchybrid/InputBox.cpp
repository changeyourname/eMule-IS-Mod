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
#include "resource.h"
#include "eMule.h"
#include "InputBox.h"
#include "OtherFunctions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNAMIC(InputBox, CDialog)

BEGIN_MESSAGE_MAP(InputBox, CDialog)
	ON_BN_CLICKED(IDC_CLEANFILENAME, OnCleanFilename)
	// ismod
	ON_BN_CLICKED(IDC_MOD_RADIO_BEGIN, OnBnClickedRadioBegin)
	ON_BN_CLICKED(IDC_MOD_RADIO_END, OnBnClickedRadioEnd)
END_MESSAGE_MAP()

InputBox::InputBox(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_INPUTBOX, pParent)
{
	m_cancel = true;
	m_bFilenameMode = false;
	m_icMain = NULL;

	// ismod
	m_bMultirename = false;
	m_strBegin = _T("");
	m_strEnd = _T("");
	m_renameMode = IBRM_NORMAL;
}

InputBox::~InputBox()
{
	if (m_icMain)
		VERIFY( DestroyIcon(m_icMain) );
}

void InputBox::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

void InputBox::OnOK()
{
	m_cancel = false;
	GetDlgItemText(IDC_TEXT, m_return);

	if ( IBRM_NORMAL != m_renameMode )
		m_return.Trim();

	CDialog::OnOK();
}

void InputBox::SetLabels(LPCTSTR title, LPCTSTR label, LPCTSTR defaultStr)
{
	m_label = label;
	m_title = title;
	m_default = defaultStr;
}

BOOL InputBox::OnInitDialog()
{
	CDialog::OnInitDialog();
	InitWindowStyles(this);
	SetIcon( m_icMain = theApp.LoadIcon(_T("RENAME")),FALSE);

	m_pwndText = (CEdit*)GetDlgItem(IDC_TEXT);
	
	GetDlgItem(IDC_IBLABEL)->SetWindowText(m_label);
	m_pwndText->SetWindowText(m_default);
	SetWindowText(m_title);

	SetDlgItemText(IDOK, GetResString(IDS_TREEOPTIONS_OK) );
	SetDlgItemText(IDCANCEL, GetResString(IDS_CANCEL) );
	SetDlgItemText(IDC_CLEANFILENAME, GetResString(IDS_CLEANUP));
	GetDlgItem(IDC_CLEANFILENAME)->ShowWindow(m_bFilenameMode ? SW_NORMAL : SW_HIDE);

	// ismod [start]
	if ( m_bMultirename ) {
		SetDlgItemText(IDC_IBLABEL, GetResString(IDS_MOD_MULTIRENAMING));
		GetDlgItem(IDC_MOD_RADIO_RENAME)->EnableWindow(FALSE);
		CheckDlgButton(IDC_MOD_RADIO_BEGIN, BST_CHECKED);
		OnBnClickedRadioBegin();
	} else {
		GetDlgItem(IDC_MOD_RADIO_BEGIN)->EnableWindow(FALSE);
		GetDlgItem(IDC_MOD_RADIO_END)->EnableWindow(FALSE);
		CheckDlgButton(IDC_MOD_RADIO_RENAME, BST_CHECKED);
	}

	if ( m_bFilenameMode )
		SelectOnlyFileName();
	// ismod [end]

	return TRUE;
}

void InputBox::OnCleanFilename()
{
	CString filename;
	m_pwndText->GetWindowText(filename);
	m_pwndText->SetWindowText(CleanupFilename(filename));
}

// ismod [start]
void InputBox::SetMultirename( LPCTSTR strBegin, LPCTSTR strEnd )
{
	m_bMultirename = true;
	m_strBegin = strBegin;
	m_strEnd = strEnd;
}

void InputBox::OnBnClickedRadioBegin()
{
	m_default = m_strBegin;
	m_pwndText->SetWindowText(m_default);
	//GetDlgItem(IDC_RADIO_RENAME)->ShowWindow(SW_NORMAL);
	//GetDlgItem(IDC_RADIO_END)->ShowWindow(SW_NORMAL);
	m_renameMode = IBRM_BEGINIG;
}

void InputBox::OnBnClickedRadioEnd()
{
	m_default = m_strEnd;
	m_pwndText->SetWindowText(m_default);
	//GetDlgItem(IDC_RADIO_RENAME)->ShowWindow(SW_NORMAL);
	//GetDlgItem(IDC_RADIO_BEGIN)->ShowWindow(SW_NORMAL);
	m_renameMode = IBRM_END;
}


BOOL InputBox::PreTranslateMessage(MSG* pMsg)
{
	if ( pMsg->message == WM_KEYDOWN )
	{
		if ( pMsg->wParam == VK_F2 && m_bFilenameMode ) {
			int start, end;
			m_pwndText->GetSel(start, end);
			// none or part selected
			if                         ( (start == end) || (start != end && end != m_pwndText->GetWindowTextLength()) )
				m_pwndText->SetSel(0,-1);
			else
				SelectOnlyFileName();
		}
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void InputBox::SelectOnlyFileName()
{
	CString text;
	m_pwndText->GetWindowText(text);
	int extPos = text.ReverseFind(_T('.'));
	if ( -1 != extPos )
		m_pwndText->PostMessage(EM_SETSEL, (WPARAM)0, (LPARAM)extPos);
}
