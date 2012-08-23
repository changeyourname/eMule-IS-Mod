//this file is part of eMule IS Mod
//Copyright (C)2009 IS Mod Team
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

#ifndef __HASHARRAY_H__
#define __HASHARRAY_H__

class CHashArray
{
public:
	CHashArray(uint32_t hashSize = 16)
	{
		m_data = NULL;
		m_count = 0;
		m_hashSize = hashSize;
	}
	~CHashArray() { release(); }

	uint32_t	hashSize() const { return m_hashSize; }
	uint32_t	count() const { return m_count; }
	void	setCount(uint32_t count) { m_count = count; }
	bool	empty() const { return count()==0; }
	uchar*	data() const { return m_data; }
	void	append(const uchar* hash) { memcpy(m_data+m_count++*m_hashSize, hash, m_hashSize); }
	uchar*	append_v() { return m_data + m_count++ * m_hashSize; }
	void	allocate(uint32_t count) {
		release();
		m_data = new uchar[count*m_hashSize];
	}

	void release() {
		if ( m_data )
			delete[] m_data;
		m_data = NULL;
		m_count = 0;
	}

	void copy( const CHashArray &hasharray ) {
		m_hashSize = hasharray.hashSize();
		allocate(hasharray.count());
		m_count = hasharray.count();
		memcpy(m_data, hasharray.data(), m_count*m_hashSize);
	}

	const uchar* operator[]( uint32_t index ) const { return m_data+index*m_hashSize; }


private:
	uchar *m_data;
	uint32_t m_count;
	uint32_t m_hashSize;
};

#endif // __HASHARRAY_H__