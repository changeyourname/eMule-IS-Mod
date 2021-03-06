//this file is part of eMule
//Copyright (C)2002-2008 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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
#define _USE_MATH_DEFINES
#include <math.h>
#include "barshader.h"
#include "Preferences.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define HALF(X) (((X) + 1) / 2)

CBarShader::CBarShader(uint32_t height, uint32_t width) 
: m_Spans(100) // ismod: save cpu?
{
	m_iWidth = width;
	m_iHeight = height;
	m_uFileSize = (uint64_t)1;
	m_Spans.SetAt(0, 0);	// SLUGFILLER: speedBarShader
	m_Modifiers = NULL;
	m_bIsPreview = false;
}

CBarShader::~CBarShader(void) {
	delete[] m_Modifiers;
}

void CBarShader::Reset() {
	Fill(0);
}

void CBarShader::BuildModifiers() {
	delete[] m_Modifiers;
	m_Modifiers = NULL; // 'new' may throw an exception

	if (!m_bIsPreview) 
		m_used3dlevel=thePrefs.Get3DDepth();

	// Barry - New property page slider to control depth of gradient

	// Depth must be at least 2
	// 2 gives greatest depth, the higher the value, the flatter the appearance
	// m_Modifiers[count-1] will always be 1, m_Modifiers[0] depends on the value of depth
	
	int depth = (7-m_used3dlevel);
	int count = HALF(m_iHeight);
	double piOverDepth = M_PI/depth;
	double base = piOverDepth * ((depth / 2.0) - 1);
	double increment = piOverDepth / (count - 1);

	m_Modifiers = new float[count];
	for (int i = 0; i < count; i++)
		m_Modifiers[i] = (float)(sin(base + i * increment));
}

void CBarShader::SetWidth(int width) {
	if(m_iWidth != width) {
		m_iWidth = width;
		if (m_uFileSize > (uint64_t)0)
			m_dPixelsPerByte = (double)m_iWidth / (uint64_t)m_uFileSize;
		else
			m_dPixelsPerByte = 0.0;
		if (m_iWidth)
			m_dBytesPerPixel = (double)m_uFileSize / m_iWidth;
		else
			m_dBytesPerPixel = 0.0;
	}
}

void CBarShader::SetFileSize(uint64_t fileSize) {
	if(m_uFileSize != fileSize) {
		// BEGIN netfinity: Make sure that the end key is where it should be and that there isn't anything beyond
		if (fileSize > m_uFileSize)
		{
			if (m_uFileSize != 0ULL)
				m_Spans.RemoveAt(m_Spans.GetTailPosition());
			m_Spans.SetAt(fileSize, 0ULL);
		}
		else
		{
			for (POSITION pos = m_Spans.FindFirstKeyAfter(fileSize); pos != 0; ) {
				POSITION pos1 = pos;
				m_Spans.GetNext(pos);
				m_Spans.RemoveAt(pos1);
			}
			m_Spans.SetAt(fileSize, 0ULL);
		}
		// END netfinity: Make sure that the end key is where it should be and that there isn't anything beyond

		m_uFileSize = fileSize;

		if (m_uFileSize > 0ULL)
			m_dPixelsPerByte = (double)m_iWidth / (uint64_t)m_uFileSize;
		else
			m_dPixelsPerByte = 0.0;

		if (m_iWidth)
			m_dBytesPerPixel = (double)m_uFileSize / m_iWidth;
		else
			m_dBytesPerPixel = 0.0;
	}
}

void CBarShader::SetHeight(int height) {
	if(m_iHeight != height) {
		m_iHeight = height;

		BuildModifiers();
	}
}

void CBarShader::FillRange(uint64_t start, uint64_t end, COLORREF color) {
	if(end > m_uFileSize)
		end = m_uFileSize;

	if(start >= end)
		return;

	// MOD BEGIN netfinity: Optimized speedBarShader
	// SLUGFILLER: speedBarShader
	POSITION startpos;
	POSITION endpos;
	POSITION pos; // netfinity: We will use this key in the loop further down

	if (start == 0)
	{
		pos = m_Spans.GetHeadPosition();
		startpos = pos;
	}
	else
	{
		pos = m_Spans.FindFirstKeyAfter(start);
		startpos = pos;
		m_Spans.GetPrev(startpos);
	}

	if (end == m_uFileSize)
		endpos = m_Spans.GetTailPosition();
	else
		endpos = m_Spans.FindFirstKeyAfter(end+1);

	ASSERT(startpos != NULL);
	ASSERT(endpos != NULL);

	// Find color values before and after the fill range
	COLORREF startcolor = m_Spans.GetValueAt(startpos);
	POSITION endcolorpos = endpos;
//	if (endpos != startpos)
		m_Spans.GetPrev(endcolorpos);
	COLORREF endcolor = m_Spans.GetValueAt(endcolorpos);

	// Check if range is already set to the specific color, to do an early return (not likely to happen thought)
	if (startpos == endcolorpos && startcolor == color)
		return;

	// BarShader is often filled continuing from the previos end position. Thus we can save an insert operation in that case.
	if (startcolor != color && m_Spans.GetKeyAt(pos) == start)
	{
		m_Spans.SetValueAt(pos, color); // netfinity: Much cheaper than a SetAt()!
		// Clear all keys beyond start to and including end, but not the last key
		m_Spans.GetNext(pos);
		for (; pos != endpos; ) {
			POSITION pos1 = pos;
			m_Spans.GetNext(pos);
			m_Spans.RemoveAt(pos1);
		}
	}
	else
	{
		// Clear all keys from start to and including end, but not the last key
		for (; pos != endpos; ) {
			POSITION pos1 = pos;
			m_Spans.GetNext(pos);
			m_Spans.RemoveAt(pos1);
		}
		
		// Only set color if previous section is different or the key is first
		if (startcolor != color || start == 0)
			m_Spans.SetAt(start, color);
	}
	// Only set color if aftercoming section is different
	if (endcolor != color && end != m_uFileSize)
		m_Spans.SetAt(end, endcolor);
	// SLUGFILLER: speedBarShader
	// MOD END netfinity
}

void CBarShader::Fill(COLORREF color) {
	// SLUGFILLER: speedBarShader
	m_Spans.RemoveAll();
	m_Spans.SetAt(0, color);
	m_Spans.SetAt(m_uFileSize, 0);
	// SLUGFILLER: speedBarShader
}

void CBarShader::Draw(CDC* dc, int iLeft, int iTop, bool bFlat) {
	POSITION pos = m_Spans.GetHeadPosition();	// SLUGFILLER: speedBarShader
	RECT rectSpan;
	rectSpan.top = iTop;
	rectSpan.bottom = iTop + m_iHeight;
	rectSpan.right = iLeft;
	// netfinity: Check clip zones
	RECT clipRect;
	dc->GetClipBox(&clipRect);
	// netfinity: Adjust clip zones
	if (clipRect.left < iLeft)
		clipRect.left = iLeft;
	if (clipRect.right < iLeft + m_iWidth)
		clipRect.right = iLeft + m_iWidth;

	// ismod: save bk color
	COLORREF crOldBk = dc->GetBkColor();
	
	uint64_t uBytesInOnePixel = (uint64_t)(m_dBytesPerPixel + 0.5f);
	uint64_t start = 0;//bsCurrent->start;
	// SLUGFILLER: speedBarShader
	COLORREF color = m_Spans.GetValueAt(pos);
	m_Spans.GetNext(pos);
	// SLUGFILLER: speedBarShader
	while(pos != NULL && rectSpan.right < clipRect.right) {	// SLUGFILLER: speedBarShader
		uint64_t uSpan = m_Spans.GetKeyAt(pos) - start;	// SLUGFILLER: speedBarShader
		uint64_t uPixels = (uint64_t)(uSpan * m_dPixelsPerByte + 0.5f);
		if (uPixels > 0) {
			rectSpan.left = rectSpan.right;
			rectSpan.right += (int)uPixels;
			if (rectSpan.right > clipRect.left)
				FillRect(dc, &rectSpan, color, bFlat);	// SLUGFILLER: speedBarShader
			start += (uint64_t)(uPixels * m_dBytesPerPixel + 0.5f);
		} else if (rectSpan.right > clipRect.left) {
			float fRed = 0;
			float fGreen = 0;
			float fBlue = 0;
			uint64_t iEnd = start + uBytesInOnePixel;
			uint64_t iLast = start;
			// SLUGFILLER: speedBarShader
			do {
				float fWeight = (float)((min(m_Spans.GetKeyAt(pos), iEnd) - iLast) * m_dPixelsPerByte);
				fRed   += GetRValue(color) * fWeight;
				fGreen += GetGValue(color) * fWeight;
				fBlue  += GetBValue(color) * fWeight;
				if(m_Spans.GetKeyAt(pos) > iEnd)
					break;
				iLast = m_Spans.GetKeyAt(pos);
				color = m_Spans.GetValueAt(pos);
				m_Spans.GetNext(pos);
			} while(pos != NULL);
			// SLUGFILLER: speedBarShader
			rectSpan.left = rectSpan.right;
			++rectSpan.right;
			FillRect(dc, &rectSpan, fRed, fGreen, fBlue, bFlat);
			start += uBytesInOnePixel;
		} else {
			rectSpan.left = rectSpan.right;
			++rectSpan.right;
			start += uBytesInOnePixel;
		}
		// SLUGFILLER: speedBarShader
		while(pos != NULL && m_Spans.GetKeyAt(pos) < start) {
			color = m_Spans.GetValueAt(pos);
			m_Spans.GetNext(pos);
		}
		// SLUGFILLER: speedBarShader
	}
	// ismod: restore bk color
	dc->SetBkColor(crOldBk);
}

void CBarShader::FillRect(CDC *dc, LPRECT rectSpan, COLORREF color, bool bFlat) {
	if(!color || bFlat)
		dc->FillSolidRect(rectSpan, color);
	else
		FillRect(dc, rectSpan, GetRValue(color), GetGValue(color), GetBValue(color), false);
}

void CBarShader::FillRect(CDC *dc, LPRECT rectSpan, float fRed, float fGreen,
						  float fBlue, bool bFlat) {
	if(bFlat) {
		COLORREF color = RGB((int)(fRed + .5f), (int)(fGreen + .5f), (int)(fBlue + .5f));
		dc->FillSolidRect(rectSpan, color);
	} else {
		if (m_Modifiers == NULL || (m_used3dlevel!=thePrefs.Get3DDepth() && !m_bIsPreview) )
			BuildModifiers();
		RECT rect = *rectSpan;
		int iTop = rect.top;
		int iBot = rect.bottom;
		int iMax = HALF(m_iHeight);
		for(int i = 0; i < iMax; i++) {
			const COLORREF crNew = RGB((int)(fRed * m_Modifiers[i] + .5f), (int)(fGreen * m_Modifiers[i] + .5f), (int)(fBlue * m_Modifiers[i] + .5f));

			rect.top = iTop + i;
			rect.bottom = iTop + i + 1;
			dc->FillSolidRect(&rect, crNew);

			rect.top = iBot - i - 1;
			rect.bottom = iBot - i;
			dc->FillSolidRect(&rect, crNew);
		}
	}
}

void CBarShader::DrawPreview(CDC* dc, int iLeft, int iTop, UINT previewLevel)		//Cax2 aqua bar
{
	m_bIsPreview=true;
	m_used3dlevel = previewLevel;
	BuildModifiers();
	Draw( dc, iLeft, iTop, (previewLevel==0));
	m_bIsPreview=false;
}
