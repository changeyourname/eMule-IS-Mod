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
#include "eMule.h"
#include "TransferDebugWnd.h"
#include "PartFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(TransferDebugWnd, CWnd)

BEGIN_MESSAGE_MAP(TransferDebugWnd, CWnd)
	ON_WM_PAINT()
END_MESSAGE_MAP()

TransferDebugWnd::TransferDebugWnd()
{
}

TransferDebugWnd::~TransferDebugWnd()
{
}

void TransferDebugWnd::updateInfo( CPartFile *file )
{
	m_infoString.Format(
		_T("File: %s\n")
		_T("Size: %I64u Complete: %I64u (%.2f%%)\n")
		_T("Sources: %d (complete: %u)\n")
		_T("DlSpeed: %u\n")
		_T("Parts: %u OnHash: %u\n")
		_T("BufferedData: %I64u\n")
		,
		file->GetFileName(),
		(uint64_t)file->GetFileSize(), (uint64_t)file->GetCompletedSize(), file->GetPercentCompleted(),
		(uint32_t)file->GetSourceCount(), (uint32_t)file->GetCompleteSourcesCount(),
		file->GetDatarate(),
		(uint32_t)file->partCount(), (uint32_t)file->m_partsOnHash,
		file->m_nTotalBufferData
	);

	Invalidate(TRUE);
	UpdateWindow();
}

void TransferDebugWnd::OnPaint()
{
	CPaintDC dc(this);

	RECT rc;
	GetClientRect(&rc);
	dc.FillSolidRect(&rc, RGB(255,255,255));
	dc.DrawText(m_infoString, -1, &rc, DT_LEFT);
}
