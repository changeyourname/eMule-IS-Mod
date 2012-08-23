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
#include "resource.h"
#include "AuthDlg.h"
#include "otherfunctions.h"

IMPLEMENT_DYNAMIC(CAuthDlg, CDialog)

CAuthDlg::CAuthDlg(CWnd* pParent /*= NULL*/)
	: CDialog(IDD_MOD_AUTH, pParent)
{
}

CAuthDlg::~CAuthDlg()
{
}

void CAuthDlg::OnOK()
{
	if( ::GetWindowTextLength(::GetDlgItem(GetSafeHwnd(), IDC_MOD_LOGIN)) < 1
	||  ::GetWindowTextLength(::GetDlgItem(GetSafeHwnd(), IDC_MOD_PASSWORD)) < 1)
	{
		MessageBox(_T("Вы не ввели параметры авторизации"), _T("Ошибка"), MB_ICONWARNING);
		return;
	}

	GetDlgItemText(IDC_MOD_LOGIN, m_login);
	GetDlgItemText(IDC_MOD_PASSWORD, m_password);

	CDialog::OnOK();
}

BOOL CAuthDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Localization
	SetWindowText(GetResString(IDS_MOD_AUTH_ERROR));
	SetDlgItemText(IDC_MOD_STATIC_LOGIN, GetResString(IDS_MOD_LOGIN));
	SetDlgItemText(IDC_MOD_STATIC_PASSWORD, GetResString(IDS_MOD_PASSWORD));
	SetDlgItemText(IDC_MOD_AUTHDLGMSG, GetResString(IDS_MOD_AUTHDLGMSG));

	SetDlgItemText(IDC_MOD_LOGIN, m_login);
	SetDlgItemText(IDC_MOD_PASSWORD, m_password);
	
	return TRUE;
}

