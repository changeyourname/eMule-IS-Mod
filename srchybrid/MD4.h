//
// MD4.h
//
// Copyright (c) Shareaza Development Team, 2002-2004.
// This file is part of SHAREAZA (www.shareaza.com)
//
// Shareaza is free software; you can redistribute it
// and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// Shareaza is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Shareaza; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
#pragma once

#pragma warning(push)
#pragma warning(disable:4244 4100 4189)
#include <cryptopp/md4.h>
#pragma warning(pop)

typedef union
{
	BYTE	n[16];
	BYTE	b[16];
	DWORD	w[4];
} MD4, MD5;

// netfinity: Needed for inlined code
class CMD4;
extern "C" void MD4_Add_p5(CMD4*, LPCVOID pData, DWORD nLength);

class CMD4
{
// Construction
public:
	CMD4();
	virtual ~CMD4();

	static bool VerifyImplementation();

#ifdef _M_IX86
// Attributes
protected:
	// NOTE: if you change this, modify the offsets in MD4_ASM.ASM accordingly
	DWORD	m_nState[4];
	DWORD	m_nCount[2];
	BYTE	m_nBuffer[64];

// Operations
public:
	void	Reset();
	void	Add(LPCVOID pData, DWORD nLength) {MD4_Add_p5(this, pData, nLength);} // netfinity: Made inline
	void	Finish();
	void	GetHash(MD4* pHash) {memcpy(pHash->b, m_nState, 16);} // netfinity: Made inline
	const BYTE* GetHash() const { return (const BYTE*)m_nState; }
// netfinity: Use cryptlib for non X86 platforms
#else
// Attributes
protected:
	CryptoPP::Weak::MD4 m_md4;
	MD4 m_hash;

// Operations
public:
	void	Reset() {m_md4.Restart();}
	void	Add(LPCVOID pData, DWORD nLength) {m_md4.Update((const byte *) pData, nLength);}
	void	Finish() {m_md4.Final(m_hash.b);}
	void	GetHash(MD4* pHash) {memcpy(pHash->b, m_hash.b, 16);}
	const BYTE* GetHash() const { return (const BYTE*) m_hash.b; }
#endif
};

inline bool operator==(const MD4& md4a, const MD4& md4b)
{
    return memcmp( &md4a, &md4b, 16 ) == 0;
}

inline bool operator!=(const MD4& md4a, const MD4& md4b)
{
    return memcmp( &md4a, &md4b, 16 ) != 0;
}
