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

#include "stdafx.h"
#include "resource.h"
#include "options.h"
#include "sct.h"
#include "sctCore.h"
#include "SctWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Sct dialog

IMPLEMENT_DYNAMIC(CSctWnd, CDialog)

BEGIN_MESSAGE_MAP(CSctWnd, CResizableDialog)
    ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_IS74_SETTINGS_HELP, OnBnClickedSettings)
	ON_BN_CLICKED(IDC_BUTTON_IS74_PAYMENT_HELP, OnBnClickedPayment)
	ON_WM_TIMER()
END_MESSAGE_MAP()

// таймеры
#define SCT_TIMER_PIS	0

CSctWnd::CSctWnd(CWnd* pParent /*=NULL*/)
    : CResizableDialog(IDD_SCT, pParent),
    m_sctCore( new CSctCore )
{

}

CSctWnd::~CSctWnd()
{
    delete m_sctCore;
}

BOOL CSctWnd::OnInitDialog()
{
	CResizableDialog::OnInitDialog();

	RefreshInfo();

	// init UI anchors
	AddAnchor(IDC_GROUP_INFO, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_GROUP_TEST, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_STATIC_ETHERNET, TOP_LEFT, TOP_RIGHT);
    AddAnchor(IDC_STATIC_IS74RU, TOP_LEFT, TOP_RIGHT);
    AddAnchor(IDC_PROGRESS_ETHERNET, TOP_RIGHT, TOP_RIGHT);
    AddAnchor(IDC_PROGRESS_IS74RU, TOP_RIGHT, TOP_RIGHT);
	AddAnchor(IDC_BUTTON_IS74_SETTINGS_HELP, TOP_LEFT);
	AddAnchor(IDC_BUTTON_IS74_PAYMENT_HELP, TOP_LEFT);
	AddAnchor(IDC_STATIC_ASK, TOP_LEFT);
	AddAnchor(IDC_STATIC_TELS, TOP_LEFT);
	AddAnchor(IDC_LBL_IFACE, TOP_LEFT);
	AddAnchor(IDC_LBL_AVAL, TOP_LEFT);

	// инициализация PIS
	SetTimer(SCT_TIMER_PIS, SCT_PIS_INTERVAL, NULL);

	return true;
}

void CSctWnd::OnDestroy()
{
}

void CSctWnd::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_PROGRESS_ETHERNET, progress_ethernet);
	DDX_Control(pDX, IDC_PROGRESS_IS74RU, progress_is74ru);
	DDX_Control(pDX, IDC_STATIC_ETHERNET, static_ethernet);
	DDX_Control(pDX, IDC_STATIC_IS74RU, static_is74ru);
}

LRESULT CSctWnd::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		// arrange transferwindow layout
		case WM_PAINT:
			RefreshInfo();
			break;
	}

	return CResizableDialog::DefWindowProc(message, wParam, lParam);
}

// показать справку по настройке
void CSctWnd::OnBnClickedSettings()
{
    ShellExecute(0, _T("open"), _T("http://is74.ru/support/manuals/"), _T(""), NULL, SW_SHOW);
}

// показать способы оплаты
void CSctWnd::OnBnClickedPayment()
{
    ShellExecute(0, _T("open"), _T("http://is74.ru/home/pay/"), _T(""), NULL, SW_SHOW);
}

void CSctWnd::RefreshInfo()
{
    progress_ethernet.SetRange32(0, 100);
	progress_ethernet.SetStep(1);
	progress_ethernet.SetPos(m_sctCore->GetEthernetStatus());
	
    if (m_sctCore->GetEthernetStatus() <= 0)
        static_ethernet.SetWindowText(_T("Нет"));
    else
    {
        CString strMsg;
        strMsg.Format(_T("IP: %s"), m_sctCore->GetOwnIp());
        static_ethernet.SetWindowText(strMsg);
    }
    
    progress_is74ru.SetRange32(0, 100);
	progress_is74ru.SetStep(1);
	progress_is74ru.SetPos(m_sctCore->GetIs74SiteStatus());
	
	if ( m_sctCore->GetIs74SiteStatus() <= 0 )
        static_is74ru.SetWindowText(_T("Недоступен"));
	else
        static_is74ru.SetWindowText(_T("Доступен"));
}

void CSctWnd::CreateWnd()
{
    CResizableDialog::Create(IDD_SCT);
}

void CSctWnd::OnTimer(UINT_PTR nIDEvent)
{
	if ( SCT_TIMER_PIS == nIDEvent )
		m_sctCore->InvokePis();

	CResizableDialog::OnTimer(nIDEvent);
}

void CSctWnd::startCore(void* params)
{
	UNREFERENCED_PARAMETER(params);
	m_sctCore->startThreads();
}
