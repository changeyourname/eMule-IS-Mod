//this file is part of eMule IS Mod
//Copyright (C)2011 eMule IS Mod
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
#include <emhelper/emhelper.h>
#include "resource.h"

#include "XCatalogWnd.h"
#include "Log.h"
#include "StringConversion.h"
#include "emule.h"
#include "emuleDlg.h"
#include "DownloadQueue.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IDC_CONTAINER 31337

class CXCatalogClientWnd: public CWnd
{
protected:
	DECLARE_DYNCREATE(CXCatalogClientWnd)
public:
	CXCatalogClientWnd();
	virtual ~CXCatalogClientWnd();

	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle,
		const RECT& rect, CWnd* pParentWnd, UINT nID, 
		CCreateContext* pContext = NULL)
	{ 
		UNREFERENCED_PARAMETER(pContext);
		UNREFERENCED_PARAMETER(lpszClassName);

		return CreateControl(_T("{45d10b88-8b68-4ee6-940b-983ba8bf5c49}"), lpszWindowName, dwStyle, rect, pParentWnd, nID); 
	}

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		switch ( pMsg->message ) {
		case WM_KEYDOWN:
		case WM_KEYUP:
			switch (pMsg->wParam) {
			case VK_UP:
			case VK_DOWN:
			case VK_LEFT:
			case VK_RIGHT:
			case VK_TAB:
			case VK_ESCAPE:
			case VK_RETURN:
				GetFocus()->SendMessage(pMsg->message, pMsg->wParam, pMsg->lParam);
				return TRUE;
			}
		}

		return FALSE; //__super::PreTranslateMessage(pMsg);
	}

	DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP(CXCatalogClientWnd, CWnd)
END_MESSAGE_MAP()

IMPLEMENT_DYNCREATE(CXCatalogClientWnd, CWnd);

CXCatalogClientWnd::CXCatalogClientWnd()
{
}

CXCatalogClientWnd::~CXCatalogClientWnd()
{
}

/*************************************************************************/
/*************************************************************************/
/*************************************************************************/

IMPLEMENT_DYNAMIC(CXCatalogWnd, CDialog)

BEGIN_MESSAGE_MAP(CXCatalogWnd, CResizableDialog)
	ON_WM_DESTROY()
	ON_WM_SIZE()
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CXCatalogWnd, CResizableDialog)
	ON_EVENT(CXCatalogWnd, IDC_CONTAINER, 4/*ed2kLinkEvent*/, OnEd2kLinkEvent, VTS_WBSTR)
	ON_EVENT(CXCatalogWnd, IDC_CONTAINER, 5/*filePreviewEvent*/, OnFilePreviewEvent, VTS_WBSTR)
END_EVENTSINK_MAP()

CXCatalogWnd::CXCatalogWnd(CWnd* pParent /*=NULL*/) :
	CResizableDialog(IDD_MOD_XCATALOG, pParent),
	m_catalog(NULL)
{
}

CXCatalogWnd::~CXCatalogWnd()
{
	if( m_catalog ) delete m_catalog;
}

BOOL CXCatalogWnd::OnInitDialog()
{
	CResizableDialog::OnInitDialog();

	RECT rc;
	GetClientRect(&rc);
	
	m_catalog = new CXCatalogClientWnd;
	if ( !m_catalog->Create(NULL, _T("xcatalog"), WS_VISIBLE|WS_CHILD, rc, this, IDC_CONTAINER) )
	{
		CWaitCursor waitCursor;
		emActiveX::registerServer(L"xcatalog.dll");

		if ( !m_catalog->Create(NULL, _T("xcatalog"), WS_VISIBLE|WS_CHILD, rc, this, IDC_CONTAINER) ) {
			AddLogLine(true, _T("Unable create xCatalog control"));
			delete m_catalog;
			m_catalog = NULL;
		}
	}
	return TRUE;
}

void CXCatalogWnd::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
}

void CXCatalogWnd::OnDestroy()
{
	if ( m_catalog )
		m_catalog->DestroyWindow();

	CResizableDialog::OnDestroy();
}

void CXCatalogWnd::OnSize(UINT nType, int cx, int cy)
{
	CResizableDialog::OnSize(nType, cx, cy);

	if ( m_catalog ) 
		m_catalog->MoveWindow(0,0, cx, cy, TRUE);
}

BOOL CXCatalogWnd::OnEd2kLinkEvent( BSTR bstrEd2kLink )
{
	theApp.emuledlg->ProcessED2KLink(bstrEd2kLink);

	return TRUE;
}

BOOL CXCatalogWnd::OnFilePreviewEvent( BSTR bstrEd2kLink )
{
	theApp.downloadqueue->PreviewFileByLink(bstrEd2kLink, true);

	return TRUE;
}
