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

#pragma once

class CPisDialog : public CDialog
{
	DECLARE_DYNAMIC(CPisDialog)

public:
	CPisDialog(CWnd* pParent = NULL);
	virtual ~CPisDialog();

	enum { IDD = IDD_PIS };

	void setInfoText(LPCTSTR szInfoText) { m_strInfoText = szInfoText; }

protected:
	CString m_strInfoText;

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
};
