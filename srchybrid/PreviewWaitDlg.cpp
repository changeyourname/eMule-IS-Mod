//this file is part of eMule IS Mod
//Copyright (C)2010 IS Mod Team
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
#include "eMule.h"
#include "PartFile.h"
#include "PreviewWaitDlg.h"
#include <CxImage/ximagif.h>

#define PAINT_TIMER_EVENT_ID 1

IMPLEMENT_DYNAMIC(CPreviewWaitDlg, CDialog)

BEGIN_MESSAGE_MAP(CPreviewWaitDlg, CDialog)
	ON_WM_TIMER()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
    ON_WM_DESTROY()
END_MESSAGE_MAP()
	
	CPreviewWaitDlg::CPreviewWaitDlg(CPartFile* file, CWnd* parent /*= NULL*/)
	: CDialog(IDD_MOD_PREVIEWWAIT, parent)
{
	img = new CxImageGIF;
	m_file = file;
}

CPreviewWaitDlg::~CPreviewWaitDlg()
{
	delete img; img = NULL;
}

void CPreviewWaitDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BOOL CPreviewWaitDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	LOGFONT lfDefault;
	GetFont()->GetLogFont(&lfDefault);
	lfDefault.lfWeight = FW_BOLD;
	m_font.CreateFontIndirect(&lfDefault);

	SetDlgItemText(IDCANCEL, GetResString(IDS_CANCEL));

	HRSRC hImg = FindResource(0, MAKEINTRESOURCE(IDR_MOD_PREVIEWWAIT), _T("GIF"));
	img->SetRetreiveAllFrames(true);
	img->LoadResource(hImg, CXIMAGE_FORMAT_GIF);
	total_frames = img->GetNumFrames();
	cur_frame = 0;
	SetTimer(PAINT_TIMER_EVENT_ID, 100, NULL);

	return TRUE;
}

void CPreviewWaitDlg::OnTimer(UINT_PTR nIDEvent)
{
	cur_frame++;
	if(cur_frame>total_frames-1) cur_frame=0;
	CDC* pDC = GetDC();
    if ( pDC ) {
	    DrawAnimation(pDC);
        ReleaseDC(pDC);
    }

	if ( m_file->IsReadyForPreview(true) ) {
		m_file->PreviewFile(true);
		EndDialog(0);
	}

	CDialog::OnTimer(nIDEvent);
}

BOOL CPreviewWaitDlg::OnEraseBkgnd(CDC* pDC)
{
	RECT rc;
	GetClientRect(&rc);
	pDC->FillRect(&rc, &CBrush(RGB(0,0,0)));

	return TRUE;
}

void CPreviewWaitDlg::DrawAnimation(CDC* pDC)
{
	RECT rc;
	GetClientRect(&rc);
	rc.left = rc.left + (rc.right - rc.left)/2 - img->GetWidth()/2;
	rc.right = rc.left + img->GetWidth();
	rc.top = rc.top + (rc.bottom - rc.top)/2 - img->GetHeight()/2;
	LONG tmp = rc.bottom = rc.top + img->GetHeight();
	img->GetFrame(cur_frame)->Draw(pDC->GetSafeHdc(), rc);

	GetClientRect(&rc);
	rc.top = tmp+10;
	pDC->SelectObject((HFONT)m_font);
	pDC->SetTextColor(RGB(255,255,255));
	pDC->SetBkColor(RGB(0,0,0));
	pDC->DrawText(GetResString(IDS_MOD_PRV_PREP), &rc, DT_CENTER|DT_VCENTER);
}

void CPreviewWaitDlg::OnDestroy()
{
    KillTimer(PAINT_TIMER_EVENT_ID);

    CDialog::OnDestroy();
}
