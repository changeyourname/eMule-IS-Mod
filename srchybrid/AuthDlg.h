//this file is part of eMule IS Mod
//Copyright (C) 2011 IS Mod Team
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

#pragma once

class CAuthDlg : public CDialog
{
	DECLARE_DYNAMIC(CAuthDlg)

public:
	CAuthDlg(CWnd* pParent = NULL);
	virtual ~CAuthDlg();

public:
	void setCredentials(LPCTSTR login, LPCTSTR password) {
		m_login = login;
		m_password = password;
	}
	void getCredentials(CString& login, CString& password) {
		login = m_login;
		password = m_password;
	}

protected:
	CString m_login;
	CString m_password;

	virtual BOOL OnInitDialog();
	virtual void OnOK();
};
