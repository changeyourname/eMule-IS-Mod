//this file is part of eMule IS Mod
//Copyright (C)2009 eMule IS Mod
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

#ifndef SCT_H
#define SCT_H

#include <ResizableLib\ResizableDialog.h>
#include "resource.h"

class CSctCore;

class AFX_EXT_CLASS CSctWnd : public CResizableDialog
{
    DECLARE_DYNAMIC(CSctWnd)

public:
	CSctWnd(CWnd* pParent = NULL);
	virtual ~CSctWnd();

    void CreateWnd();
	void startCore(void* params);

private:
    virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP();
    afx_msg void OnDestroy();
	afx_msg void OnBnClickedSettings();
	afx_msg void OnBnClickedPayment();
	afx_msg void OnBnClickedChk();

    void RefreshInfo();

    CProgressCtrl progress_ethernet;
    CProgressCtrl progress_is74ru;
    CStatic static_ethernet;
    CStatic static_is74ru;
    CListBox m_DebugLog;

    CSctCore *m_sctCore;
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};

#endif // #ifndef SCT_H