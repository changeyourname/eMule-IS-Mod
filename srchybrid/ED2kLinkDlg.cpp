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
#include "emule.h"
#include "ED2kLinkDlg.h"
#include "KnownFile.h"
#include "partfile.h"
#include "preferences.h"
// ismod: no aich
#include "UserMsgs.h"
// ismod
#include "OtherFunctions.h"
#include "Collection.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNAMIC(CED2kLinkDlg, CResizablePage) 

BEGIN_MESSAGE_MAP(CED2kLinkDlg, CResizablePage) 
	ON_BN_CLICKED(IDC_LD_CLIPBOARDBUT, OnBnClickedClipboard)
	ON_BN_CLICKED(IDC_LD_SOURCECHE, OnSettingsChange)
	// ismod: no aich
	/* ismod
	ON_BN_CLICKED(IDC_LD_HTMLCHE, OnSettingsChange)
	*/
	ON_BN_CLICKED(IDC_LD_HOSTNAMECHE, OnSettingsChange)
	ON_BN_CLICKED(IDC_LD_HASHSETCHE, OnSettingsChange)
	ON_MESSAGE(UM_DATA_CHANGED, OnDataChanged)
	// ismod
	ON_BN_CLICKED(IDC_MOD_LD_BBCODECHE, OnClickedBbcodeche)	
	ON_BN_CLICKED(IDC_LD_HTMLCHE, OnClickedHtmlche)
	ON_BN_CLICKED(IDC_MOD_LD_SIZECHE, OnExtendedOptions)
	ON_BN_CLICKED(IDC_MOD_LD_COMMENTCHE, OnExtendedOptions)

END_MESSAGE_MAP() 

CED2kLinkDlg::CED2kLinkDlg() 
   : CResizablePage(CED2kLinkDlg::IDD, IDS_CMT_READALL) 
{ 
	m_paFiles = NULL;
	m_bDataChanged = false;
	m_strCaption = GetResString(IDS_SW_LINK);
	m_psp.pszTitle = m_strCaption;
	m_psp.dwFlags |= PSP_USETITLE;
	m_bReducedDlg = false;
} 

CED2kLinkDlg::~CED2kLinkDlg() 
{ 
} 

void CED2kLinkDlg::DoDataExchange(CDataExchange* pDX) 
{ 
	CResizablePage::DoDataExchange(pDX); 
	DDX_Control(pDX, IDC_LD_LINKEDI, m_ctrlLinkEdit);
} 

BOOL CED2kLinkDlg::OnInitDialog()
{ 
	CResizablePage::OnInitDialog(); 
	InitWindowStyles(this);



	if (!m_bReducedDlg)
	{
		AddAnchor(IDC_LD_BASICGROUP,BOTTOM_LEFT,BOTTOM_RIGHT);
		AddAnchor(IDC_LD_SOURCECHE,BOTTOM_LEFT,BOTTOM_LEFT);
		// ismod: no aich
		AddAnchor(IDC_LD_ADVANCEDGROUP,BOTTOM_LEFT,BOTTOM_RIGHT);

		AddAnchor(IDC_LD_HTMLCHE,BOTTOM_LEFT,BOTTOM_LEFT);
		AddAnchor(IDC_LD_HASHSETCHE,BOTTOM_LEFT,BOTTOM_LEFT);
		AddAnchor(IDC_LD_HOSTNAMECHE,BOTTOM_LEFT,BOTTOM_LEFT);

		// enabled/disable checkbox depending on situation
		if (theApp.IsConnected() && !theApp.IsFirewalled()) {
			GetDlgItem(IDC_LD_SOURCECHE)->EnableWindow(TRUE);
			CheckDlgButton(IDC_LD_SOURCECHE, BST_CHECKED);
		} else {
			GetDlgItem(IDC_LD_SOURCECHE)->EnableWindow(FALSE);
			CheckDlgButton(IDC_LD_SOURCECHE, BST_UNCHECKED);
		}

		if (theApp.IsConnected() && !theApp.IsFirewalled() && !thePrefs.GetYourHostname().IsEmpty() &&
			thePrefs.GetYourHostname().Find(_T('.')) != -1) {
			GetDlgItem(IDC_LD_HOSTNAMECHE)->EnableWindow(TRUE);
			CheckDlgButton(IDC_LD_HOSTNAMECHE, BST_CHECKED);
		} else {
			GetDlgItem(IDC_LD_HOSTNAMECHE)->EnableWindow(FALSE);
			CheckDlgButton(IDC_LD_HOSTNAMECHE, BST_UNCHECKED);

		}
	}
	else
	{
		CRect rcDefault, rcNew;
		GetDlgItem(IDC_LD_LINKGROUP)->GetWindowRect(rcDefault);
		GetDlgItem(IDC_LD_ADVANCEDGROUP)->GetWindowRect(rcNew);
		int nDeltaY = rcNew.bottom - rcDefault.bottom;
		GetDlgItem(IDC_LD_LINKGROUP)->SetWindowPos(NULL, 0, 0, rcDefault.Width(), rcDefault.Height() + nDeltaY, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE);
		GetDlgItem(IDC_LD_LINKEDI)->GetWindowRect(rcDefault);
		GetDlgItem(IDC_LD_LINKEDI)->SetWindowPos(NULL, 0, 0, rcDefault.Width(), rcDefault.Height() + nDeltaY, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE);
		GetDlgItem(IDC_LD_CLIPBOARDBUT)->GetWindowRect(rcDefault);
		ScreenToClient(rcDefault);
		GetDlgItem(IDC_LD_CLIPBOARDBUT)->SetWindowPos(NULL, rcDefault.left, rcDefault.top + nDeltaY, 0, 0, SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE);

		// ismod [start]
		int idItems[] = { IDC_MOD_LD_BBCODECHE, IDC_MOD_LD_SIZECHE, IDC_MOD_LD_COMMENTCHE };
		for (int i=0;i < _countof(idItems); ++i ) {
			GetDlgItem(idItems[i])->GetWindowRect(rcDefault);
			ScreenToClient(rcDefault);
			GetDlgItem(idItems[i])->SetWindowPos(NULL, rcDefault.left, rcDefault.top + nDeltaY, 0, 0, SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE);
		}
		// ismod [end]
		
		GetDlgItem(IDC_LD_BASICGROUP)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_LD_SOURCECHE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_LD_ADVANCEDGROUP)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_LD_HTMLCHE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_LD_HASHSETCHE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_LD_HOSTNAMECHE)->ShowWindow(SW_HIDE);
	}
	// ismod [start]
	AddAnchor(IDC_MOD_LD_BBCODECHE,BOTTOM_LEFT,BOTTOM_LEFT);
	AddAnchor(IDC_MOD_LD_SIZECHE,BOTTOM_LEFT,BOTTOM_LEFT);
	AddAnchor(IDC_MOD_LD_COMMENTCHE,BOTTOM_LEFT,BOTTOM_LEFT);

	CheckDlgButton(IDC_MOD_LD_BBCODECHE, BST_CHECKED);
	CheckDlgButton(IDC_MOD_LD_SIZECHE, BST_CHECKED);
	CheckDlgButton(IDC_MOD_LD_COMMENTCHE, BST_UNCHECKED);
	// ismod [end]

	AddAnchor(IDC_LD_LINKGROUP,TOP_LEFT,BOTTOM_RIGHT);
	AddAnchor(IDC_LD_LINKEDI,TOP_LEFT,BOTTOM_RIGHT);
	AddAnchor(IDC_LD_CLIPBOARDBUT,BOTTOM_RIGHT);
	Localize(); 

	return TRUE; 
} 

BOOL CED2kLinkDlg::OnSetActive()
{
	if (!CResizablePage::OnSetActive())
		return FALSE;

	if (m_bDataChanged)
	{
		//hashsetlink - check if at least one file has a hashset
		BOOL bShowHashset = FALSE;
		// ismod: no aich
		BOOL bShowHTML = FALSE;
		for (int i = 0; i != m_paFiles->GetSize(); i++)
        {
			if (!(*m_paFiles)[i]->IsKindOf(RUNTIME_CLASS(CKnownFile)))
				continue;

			const CKnownFile* file = STATIC_DOWNCAST(CKnownFile, (*m_paFiles)[i]);
			if ( file->hashset().count() > 0 && file->hashset().count() == file->GetED2KPartCount() )
				bShowHashset = TRUE;

			// ismod: no aich
			if (bShowHashset)
				break;
		}
		GetDlgItem(IDC_LD_HASHSETCHE)->EnableWindow(bShowHashset);
		if (!bShowHashset)
			CheckDlgButton(IDC_LD_HASHSETCHE, BST_UNCHECKED);

		// ismod: no aich
		GetDlgItem(IDC_LD_HTMLCHE)->EnableWindow(bShowHTML);

		UpdateLink();
		m_bDataChanged = false;
	}

	return TRUE;
}

LRESULT CED2kLinkDlg::OnDataChanged(WPARAM, LPARAM)
{
	m_bDataChanged = true;
	return 1;
}

void CED2kLinkDlg::Localize(void)
{ 
	GetDlgItem(IDC_LD_LINKGROUP)->SetWindowText(GetResString(IDS_SW_LINK)); 
	GetDlgItem(IDC_LD_CLIPBOARDBUT)->SetWindowText(GetResString(IDS_LD_COPYCLIPBOARD));
	if (!m_bReducedDlg)
	{
		GetDlgItem(IDC_LD_BASICGROUP)->SetWindowText(GetResString(IDS_LD_BASICOPT));
		GetDlgItem(IDC_LD_SOURCECHE)->SetWindowText(GetResString(IDS_LD_ADDSOURCE)); 
		GetDlgItem(IDC_LD_ADVANCEDGROUP)->SetWindowText(GetResString(IDS_LD_ADVANCEDOPT)); 
		GetDlgItem(IDC_LD_HTMLCHE)->SetWindowText(GetResString(IDS_LD_ADDHTML)); 
		GetDlgItem(IDC_LD_HASHSETCHE)->SetWindowText(GetResString(IDS_LD_ADDHASHSET)); 
		GetDlgItem(IDC_LD_HOSTNAMECHE)->SetWindowText(GetResString(IDS_LD_HOSTNAME));
	}

	// ismod
	SetDlgItemText(IDC_MOD_LD_BBCODECHE, GetResString(IDS_MOD_LD_ADDBBCODE)); 
	SetDlgItemText(IDC_MOD_LD_SIZECHE, GetResString(IDS_MOD_LD_ADDSIZE)); 
	SetDlgItemText(IDC_MOD_LD_COMMENTCHE, GetResString(IDS_MOD_LD_ADDCOMMENT)); 
}

void CED2kLinkDlg::UpdateLink()
{
	CString strLinks;
	CString strBuffer;

	// ismod
	const bool bbCode = IsDlgButtonChecked(IDC_MOD_LD_BBCODECHE) != 0;
	const bool bSize = IsDlgButtonChecked(IDC_MOD_LD_SIZECHE) != 0;
	const bool bComment = IsDlgButtonChecked(IDC_MOD_LD_COMMENTCHE) != 0;

	const bool bHashset = IsDlgButtonChecked(IDC_LD_HASHSETCHE) != 0;
	const bool bHTML = IsDlgButtonChecked(IDC_LD_HTMLCHE) != 0;
	const bool bSource = IsDlgButtonChecked(IDC_LD_SOURCECHE) != 0 && theApp.IsConnected() && !theApp.IsFirewalled();
	const bool bHostname = IsDlgButtonChecked(IDC_LD_HOSTNAMECHE) != 0 && theApp.IsConnected() 
		&& !theApp.IsFirewalled() && !thePrefs.GetYourHostname().IsEmpty() 
		&& thePrefs.GetYourHostname().Find(_T('.')) != -1;
	// ismod: no aich
	
	for (int i = 0; i != m_paFiles->GetSize(); i++)
	{
		if (!(*m_paFiles)[i]->IsKindOf(RUNTIME_CLASS(CKnownFile)))
			continue;

		if (!strLinks.IsEmpty())
			strLinks += _T("\r\n\r\n");

		if (bHTML)
			strLinks += _T("<a href=\"");

		// ismod
		if (bbCode) strLinks += _T("[u][b][url=");
		/*const*/ CKnownFile* file = STATIC_DOWNCAST(CKnownFile, (*m_paFiles)[i]);
		strLinks += CreateED2kLink(file, false);
		
		if (bHashset && file->hashset().count() > 0 && file->hashset().count() == file->GetED2KPartCount()){	// SLUGFILLER: SafeHash - use GetED2KPartCount
			strLinks += _T("p=");
			for ( uint16_t j = 0; j < file->hashset().count(); j++ )
			{
				if (j > 0)
					strLinks += _T(':');
				strLinks += EncodeBase16(file->hashset(j), 16);
			}
			strLinks += _T('|');
		}

		// ismod: no aich
		strLinks += _T('/');
		if (bHostname){
			strBuffer.Format(_T("|sources,%s:%i|/"), thePrefs.GetYourHostname(), thePrefs.GetPort() );
			strLinks += strBuffer;
		}
		else if(bSource){
			uint32_t dwID = theApp.GetID();
			strBuffer.Format(_T("|sources,%i.%i.%i.%i:%i|/"),(uint8_t)dwID,(uint8_t)(dwID>>8),(uint8_t)(dwID>>16),(uint8_t)(dwID>>24), thePrefs.GetPort() );
			strLinks += strBuffer;
		}

		if (bHTML)
			strLinks += _T("\">") + StripInvalidFilenameChars(file->GetFileName()) + _T("</a>");

		// ismod [start]
		if ( bbCode )
		{
			if (GetED2KFileTypeID(file->GetFileName()) != ED2KFT_EMULECOLLECTION)
			{
				strLinks += _T("]") + StripInvalidFilenameChars(file->GetFileName()) + _T("[/url][/b][/u] ");
				if (bSize)
					strLinks += CastItoXBytes((uint64_t)file->GetFileSize(), false, false);
			}else
			{
				strLinks += _T("]") + StripInvalidFilenameChars(file->GetFileName().Left(file->GetFileName().GetLength()-CString(COLLECTION_FILEEXTENSION).GetLength())) + _T("[/url][/b][/u]");
				strLinks += _T(" коллекция");
			}

			if (bComment && !file->GetFileComment().IsEmpty())
				strLinks += _T(" [COLOR=Gray][B][I]") + file->GetFileComment() + _T("[/I][/B][/COLOR]");
		}
		// ismod [end]
	}
	m_ctrlLinkEdit.SetWindowText(strLinks);

}

void CED2kLinkDlg::OnBnClickedClipboard()
{
	CString strBuffer;
	m_ctrlLinkEdit.GetWindowText(strBuffer);
	theApp.CopyTextToClipboard(strBuffer);
}

void CED2kLinkDlg::OnSettingsChange()
{
	UpdateLink();
}

BOOL CED2kLinkDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (LOWORD(wParam) == IDCANCEL)
		return ::SendMessage(::GetParent(m_hWnd), WM_COMMAND, wParam, lParam);
	return CResizablePage::OnCommand(wParam, lParam);
}

// ismod [start]
void CED2kLinkDlg::OnClickedBbcodeche()
{
	const bool bbCode = IsDlgButtonChecked(IDC_MOD_LD_BBCODECHE) != 0;
	if ( bbCode )
	{
		CheckDlgButton(IDC_LD_HTMLCHE, BST_UNCHECKED);
		GetDlgItem(IDC_MOD_LD_SIZECHE)->EnableWindow(TRUE);
		GetDlgItem(IDC_MOD_LD_COMMENTCHE)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_MOD_LD_SIZECHE)->EnableWindow(FALSE);
		GetDlgItem(IDC_MOD_LD_COMMENTCHE)->EnableWindow(FALSE);
	}
	UpdateLink();
}

void CED2kLinkDlg::OnClickedHtmlche()
{
	const bool bbHTML = IsDlgButtonChecked(IDC_LD_HTMLCHE) != 0;
	if (bbHTML)
	{
		CheckDlgButton(IDC_MOD_LD_BBCODECHE, BST_UNCHECKED);
		CheckDlgButton(IDC_MOD_LD_SIZECHE, BST_UNCHECKED);
		CheckDlgButton(IDC_MOD_LD_COMMENTCHE, BST_UNCHECKED);
	}
	UpdateLink();
}


void CED2kLinkDlg::OnExtendedOptions()
{
	const bool bSize = IsDlgButtonChecked(IDC_MOD_LD_SIZECHE) != 0;
	const bool bComment = IsDlgButtonChecked(IDC_MOD_LD_COMMENTCHE) != 0;
	if ( bSize || bComment )
	{
		CheckDlgButton(IDC_MOD_LD_BBCODECHE, BST_CHECKED);
		CheckDlgButton(IDC_LD_HTMLCHE, BST_UNCHECKED);
	}
	UpdateLink();
}