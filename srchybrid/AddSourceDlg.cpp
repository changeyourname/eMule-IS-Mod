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
#include "AddSourceDlg.h"
#include "otherfunctions.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CAddSourceDlg, CDialog)

BEGIN_MESSAGE_MAP(CAddSourceDlg, CResizableDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()

CAddSourceDlg::CAddSourceDlg( CWnd* pParent /*=NULL*/ )	:
	CResizableDialog( IDD_ADDSOURCE, pParent )
{
}

CAddSourceDlg::~CAddSourceDlg()
{
}

void CAddSourceDlg::DoDataExchange( CDataExchange* pDX )
{
	CResizableDialog::DoDataExchange(pDX);
}

BOOL CAddSourceDlg::OnInitDialog()
{
	CResizableDialog::OnInitDialog();
	InitWindowStyles(this);

	AddAnchor(IDC_S_ADDR, TOP_LEFT, TOP_LEFT);
	AddAnchor(IDC_E_ADDRS, TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(IDC_S_PORT, BOTTOM_LEFT, BOTTOM_RIGHT);
	AddAnchor(IDOK, BOTTOM_RIGHT);
	AddAnchor(IDCANCEL, BOTTOM_RIGHT);

    SetDlgItemText(IDOK, LTR(L"OK"));
    SetDlgItemText(IDCANCEL, LTR(L"Cancel"));
    SetDlgItemText(IDC_S_ADDR, LTR(L"IP[:port], one per line"));
    CString strDefaultSourcePort;
    strDefaultSourcePort.Format(LTR(L"Default port %u applied if none specified"), DEFAULT_SOURCE_PORT);
    SetDlgItemText(IDC_S_PORT, strDefaultSourcePort);

    EnableSaveRestore(_T("AddSourceDlg"));

	return TRUE;
}

void CAddSourceDlg::OnBnClickedOk()
{
    CString rawSources;
    GetDlgItemText(IDC_E_SOURCES, rawSources);
    sources.clear();
    
	if ( rawSources.IsEmpty() )
		OnOK();

    int pos = 0;
    CString token = rawSources.Tokenize(_T("\r\n"), pos);
    while ( !token.IsEmpty() ) {
        token.Trim();
        if ( token.Find(_T(':')) == -1 )
            token.AppendFormat(_T(":%u"), DEFAULT_SOURCE_PORT);
        sources.push_back(token);

        token = rawSources.Tokenize(_T("\r\n"), pos);
    }

    OnOK();
}