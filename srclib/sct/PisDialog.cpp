//this file is part of eMule IS Mod
//Copyright (C)2012 eMule IS Mod Team
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
#include "PisDialog.h"

IMPLEMENT_DYNAMIC(CPisDialog, CDialog)

CPisDialog::CPisDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CPisDialog::IDD, pParent)
{
}

CPisDialog::~CPisDialog()
{
}

BEGIN_MESSAGE_MAP(CPisDialog, CDialog)
END_MESSAGE_MAP()

BOOL CPisDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetDlgItemText(IDC_INFO, m_strInfoText);

	return TRUE;
}