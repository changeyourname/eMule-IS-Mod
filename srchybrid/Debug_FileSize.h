//this file is part of eMule
//Copyright (C)2002-2005 Merkur ( devs@emule-project.net / http://www.emule-project.net )
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

enum E_DebugFSAtion {
	DFSA_ADD = 0,
	DFSA_SUB,
	DFSA_MUL,
	DFSA_DIV
};

class CEMFileSize {
public:
	CEMFileSize()															{m_nSize = (uint64_t)(-1);}
	CEMFileSize(uint64_t nSize)												{m_nSize = nSize; Check();}
	__declspec(deprecated) CEMFileSize(uint32_t nSize)						{m_nSize = nSize; Check();}

	CEMFileSize& operator=(const CEMFileSize& k1)							{m_nSize = k1.m_nSize; Check(); return *this; }
	CEMFileSize& operator=(uint64_t k1)										{m_nSize = k1; Check(); return *this; }
	__declspec(deprecated) CEMFileSize& operator=(int64_t k1)				{m_nSize = k1; Check(); return *this; }
	__declspec(deprecated) CEMFileSize& operator=(uint32_t k1)				{m_nSize = k1; Check(); return *this; }
	__declspec(deprecated) CEMFileSize& operator=(int32_t k1)				{m_nSize = k1; Check(); return *this; }

	CEMFileSize& operator-=(const CEMFileSize& k1)							{Check(); m_nSize -= k1.m_nSize; Check(); return *this; }
	CEMFileSize& operator-=(uint64_t k1)										{Check(); m_nSize -= k1; Check(); return *this; }
	__declspec(deprecated) CEMFileSize& operator-=(int64_t k1)				{Check(); m_nSize -= k1; Check(); return *this; }
	__declspec(deprecated) CEMFileSize& operator-=(uint32_t k1)				{Check(); m_nSize -= k1; Check(); return *this; }
	__declspec(deprecated) CEMFileSize& operator-=(int32_t k1)				{Check(); m_nSize -= k1; Check(); return *this; }

	CEMFileSize& operator+=(const CEMFileSize& k1)							{Check(); m_nSize += k1.m_nSize; Check(); return *this; }
	CEMFileSize& operator+=(uint64_t k1)										{Check(); m_nSize += k1; Check(); return *this; }
	__declspec(deprecated) CEMFileSize& operator+=(int64_t k1)				{Check(); m_nSize += k1; Check(); return *this; }
	__declspec(deprecated) CEMFileSize& operator+=(uint32_t k1)				{Check(); m_nSize += k1; Check(); return *this; }
	__declspec(deprecated) CEMFileSize& operator+=(int32_t k1)				{Check(); m_nSize += k1; Check(); return *this; }
    

	operator uint64_t() const													{return m_nSize;}
	operator double() const													{return (double)m_nSize;}
	/*__declspec(deprecated)*/ operator float() const						{return (float)m_nSize;}
	/*__declspec(deprecated)*/ operator int64_t() const						{return (int64_t)m_nSize;}
	__declspec(deprecated) operator uint32_t() const							{ASSERT( m_nSize < 0xFFFFFFFF ); return (uint32_t)m_nSize;}
	__declspec(deprecated) operator int32_t() const							{ASSERT( m_nSize < 0x7FFFFFFF ); return (int32_t)m_nSize;}

	friend bool operator==(const CEMFileSize& k1,const CEMFileSize& k2)		{return k1.m_nSize == k2.m_nSize;}
	friend bool operator==(const CEMFileSize& k1,uint64_t k2)					{return k1.m_nSize == k2;}
	friend bool operator==(uint64_t k1,const CEMFileSize& k2)					{return k1 == k2.m_nSize;}
	__declspec(deprecated) friend bool operator==(uint32_t k1,const CEMFileSize& k2)	{return k1 == k2.m_nSize;}
	__declspec(deprecated) friend bool operator==(const CEMFileSize& k1,uint32_t k2)	{return k1.m_nSize == k2;}

	friend bool operator!=(const CEMFileSize& k1,const CEMFileSize& k2)		{return k1.m_nSize != k2.m_nSize;}
	friend bool operator!=(const CEMFileSize& k1,uint64_t k2)					{return k1.m_nSize != k2;}
	friend bool operator!=(uint64_t k1,const CEMFileSize& k2)					{return k1 != k2.m_nSize;}
	__declspec(deprecated) friend bool operator!=(uint32_t k1,const CEMFileSize& k2)	{return k1 != k2.m_nSize;}
	__declspec(deprecated) friend bool operator!=(const CEMFileSize& k1,uint32_t k2)	{return k1.m_nSize != k2;}

	friend bool operator>(const CEMFileSize& k1,const CEMFileSize& k2)		{return k1.m_nSize > k2.m_nSize;}
	friend bool operator>(const CEMFileSize& k1,uint64_t k2)					{return k1.m_nSize > k2;}
	friend bool operator>(uint64_t k1,const CEMFileSize& k2)					{return k1 > k2.m_nSize;}
	__declspec(deprecated) friend bool operator>(uint32_t k1,const CEMFileSize& k2)	{return k1 > k2.m_nSize;}
	__declspec(deprecated) friend bool operator>(const CEMFileSize& k1,uint32_t k2)	{return k1.m_nSize > k2;}

	friend bool operator<(const CEMFileSize& k1,const CEMFileSize& k2)		{return k1.m_nSize < k2.m_nSize;}
	friend bool operator<(const CEMFileSize& k1,uint64_t k2)					{return k1.m_nSize < k2;}
	friend bool operator<(uint64_t k1,const CEMFileSize& k2)					{return k1 < k2.m_nSize;}
	__declspec(deprecated) friend bool operator<(uint32_t k1,const CEMFileSize& k2)	{return k1 < k2.m_nSize;}
	__declspec(deprecated) friend bool operator<(const CEMFileSize& k1,uint32_t k2)	{return k1.m_nSize < k2;}

	friend bool operator>=(const CEMFileSize& k1,const CEMFileSize& k2)		{return k1.m_nSize >= k2.m_nSize;}
	friend bool operator>=(const CEMFileSize& k1,uint64_t k2)					{return k1.m_nSize >= k2;}
	friend bool operator>=(uint64_t k1,const CEMFileSize& k2)					{return k1 >= k2.m_nSize;}
	__declspec(deprecated) friend bool operator>=(uint32_t k1,const CEMFileSize& k2)	{return k1 >= k2.m_nSize;}
	__declspec(deprecated) friend bool operator>=(const CEMFileSize& k1,uint32_t k2)	{return k1.m_nSize >= k2;}

	friend bool operator<=(const CEMFileSize& k1,const CEMFileSize& k2)		{return k1.m_nSize <= k2.m_nSize;}
	friend bool operator<=(const CEMFileSize& k1,uint64_t k2)					{return k1.m_nSize <= k2;}
	friend bool operator<=(uint64_t k1,const CEMFileSize& k2)					{return k1 <= k2.m_nSize;}
	__declspec(deprecated) friend bool operator<=(uint32_t k1,const CEMFileSize& k2)	{return k1 <= k2.m_nSize;}
	__declspec(deprecated) friend bool operator<=(const CEMFileSize& k1,uint32_t k2)	{return k1.m_nSize <= k2;}

	friend CEMFileSize operator+(const CEMFileSize& k1,const CEMFileSize& k2)		{return CEMFileSize(k1.m_nSize, k2.m_nSize, DFSA_ADD);}
	friend CEMFileSize operator+(const CEMFileSize& k1,uint64_t k2)					{return CEMFileSize(k1.m_nSize, k2, DFSA_ADD);}
	friend CEMFileSize operator+(uint64_t k1,const CEMFileSize& k2)					{return CEMFileSize(k1, k2.m_nSize, DFSA_ADD);}

	friend CEMFileSize operator-(const CEMFileSize& k1,const CEMFileSize& k2)		{return CEMFileSize(k1.m_nSize, k2.m_nSize, DFSA_SUB);}
	friend CEMFileSize operator-(const CEMFileSize& k1,uint64_t k2)					{return CEMFileSize(k1.m_nSize, k2, DFSA_SUB);}
	friend CEMFileSize operator-(uint64_t k1,const CEMFileSize& k2)					{return CEMFileSize(k1, k2.m_nSize, DFSA_SUB);}
	__declspec(deprecated) friend CEMFileSize operator-(uint32_t k1,const CEMFileSize& k2)	{return CEMFileSize(k1, k2.m_nSize, DFSA_SUB);}

	friend CEMFileSize operator*(const CEMFileSize& k1,const CEMFileSize& k2)		{return CEMFileSize(k1.m_nSize, k2.m_nSize, DFSA_MUL);}
	friend CEMFileSize operator*(const CEMFileSize& k1,uint64_t k2)					{return CEMFileSize(k1.m_nSize, k2, DFSA_MUL);}
	friend CEMFileSize operator*(uint64_t k1,const CEMFileSize& k2)					{return CEMFileSize(k1, k2.m_nSize, DFSA_MUL);}

	friend CEMFileSize operator/(const CEMFileSize& k1,const CEMFileSize& k2)		{return CEMFileSize(k1.m_nSize, k2.m_nSize, DFSA_DIV);}
	friend CEMFileSize operator/(const CEMFileSize& k1,uint64_t k2)					{return CEMFileSize(k1.m_nSize, k2, DFSA_DIV);}
	friend CEMFileSize operator/(uint64_t k1,const CEMFileSize& k2)					{return CEMFileSize(k1, k2.m_nSize, DFSA_DIV);}

private:
	CEMFileSize(uint64_t nSize1, uint64_t nSize2, E_DebugFSAtion edfsAction)	{ 
		if (edfsAction == DFSA_ADD){
			m_nSize = nSize1 + nSize2;
			ASSERT( m_nSize >= nSize1 && m_nSize >= nSize2 /*&& m_nSize <= 0x4000000000*/ );
		}
		else if (edfsAction == DFSA_SUB){
			m_nSize = nSize1 - nSize2;
			ASSERT( m_nSize <= nSize1 /*&& m_nSize <= 0x4000000000*/ );
		}
		else if (edfsAction == DFSA_DIV){
			if ( nSize2 != 0 )
				m_nSize = nSize1 / nSize2;
			else
				ASSERT( false );
		}
		else if (edfsAction == DFSA_MUL){
			m_nSize = nSize1 * nSize2;
			ASSERT( m_nSize >= nSize1 && m_nSize >= nSize2 /*&& m_nSize <= 0x4000000000*/ );
		}
	}
	void	Check()	{ ASSERT( m_nSize != (uint64_t)(-1) /*&& m_nSize <= 0x4000000000*/ ); }
	uint64_t	m_nSize;
};

// ismod [start]
// hello there! i've removed /*&& m_nSize <= 0x4000000000*/ because it crashes user and folder search