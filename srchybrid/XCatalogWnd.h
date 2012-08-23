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
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#pragma once

#include <ResizableLib\ResizableDialog.h>

class CXCatalogClientWnd;

class CXCatalogWnd : public CResizableDialog
{
    DECLARE_DYNAMIC(CXCatalogWnd)

public:
	CXCatalogWnd(CWnd* pParent = NULL);
	virtual ~CXCatalogWnd();

private:
    CWnd *m_catalog;
	
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange( CDataExchange* pDX );

	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);

	DECLARE_EVENTSINK_MAP()
	BOOL OnEd2kLinkEvent( BSTR bstrEd2kLink );
	BOOL OnFilePreviewEvent( BSTR bstrEd2kLink );
};