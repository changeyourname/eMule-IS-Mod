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

#pragma once

class CWordFilterDlg : public CDialog
{
	DECLARE_DYNAMIC(CWordFilterDlg)

public:
	CWordFilterDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CWordFilterDlg();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedUnlock();
protected:
	virtual void OnOK();
private:
	CString m_strPassword;
	bool m_bFilterLocked;
	void LoadWords();
};
