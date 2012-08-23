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
#pragma once
#include "opcodes.h"
#include "SafeFile.h"

///////////////////////////////////////////////////////////////////////////////
// Packet

class Packet
{
public:
	Packet(uint8_t protocol = OP_EDONKEYPROT);
	Packet(char* header); // only used for receiving packets
	Packet(CMemFile* datafile, uint8_t protocol = OP_EDONKEYPROT, uint8_t ucOpcode = 0x00);
	Packet(const CStringA& str, uint8_t ucProtocol, uint8_t ucOpcode);
	Packet(uint8_t in_opcode, uint32_t in_size, uint8_t protocol = OP_EDONKEYPROT, bool bFromPartFile = true);
	Packet(char* pPacketPart,uint32_t nSize,bool bLast,bool bFromPartFile = true); // only used for splitted packets!
	Packet(Packet* tocopy);
	virtual ~Packet();

	virtual char* GetHeader();
	virtual char* GetUDPHeader();
	virtual char* GetPacket();
	virtual char* DetachPacket();
	virtual uint32_t GetRealPacketSize() const	{return size+6;}
//	bool	IsSplitted() const					{return m_bSplitted;}
//	bool	IsLastSplitted() const				{return m_bLastSplitted;}
	bool	IsFromPF() const					{return m_bFromPF;}
	void	PackPacket();
	bool	UnPackPacket(UINT uMaxDecompressedSize = 50000);

	char*	pBuffer;
	uint32_t	size;
	uint8_t	opcode;
	uint8_t	prot;

protected:
	bool	m_bSplitted;
	bool	m_bLastSplitted;
	bool	m_bPacked;
	bool	m_bFromPF;
	char*	completebuffer;
	char*	tempbuffer;
	char	head[6];
};


///////////////////////////////////////////////////////////////////////////////
// CRawPacket

class CRawPacket : public Packet
{
public:
	CRawPacket(const CStringA& rstr);
	CRawPacket(const char* pcData, UINT uSize, bool bFromPartFile = false);
	virtual ~CRawPacket();

	virtual char*	GetHeader();
	virtual char*	GetUDPHeader();
	virtual char*	GetPacket()					{return pBuffer; }
	virtual void	AttachPacket(char* pcData, UINT uSize, bool bFromPartFile = false);
	virtual char*	DetachPacket();
	virtual uint32_t	GetRealPacketSize() const	{return size;}
};


///////////////////////////////////////////////////////////////////////////////
// CTag

class CTag
#ifdef _DEBUG
	: public CObject
#endif
{
public:
	CTag(LPCSTR pszName, uint64_t uVal, bool bInt64 = false);
	CTag(uint8_t uName, uint64_t uVal, bool bInt64 = false);
	CTag(LPCSTR pszName, LPCTSTR pszVal);
	CTag(uint8_t uName, LPCTSTR pszVal);
	CTag(LPCSTR pszName, const CString& rstrVal);
	CTag(uint8_t uName, const CString& rstrVal);
	CTag(uint8_t uName, const BYTE* pucHash);
	CTag(uint8_t uName, uint32_t nSize, const BYTE* pucData);
	CTag(const CTag& rTag);
	CTag(CFileDataIO* data, bool bOptUTF8);
	~CTag();

	UINT GetType() const			{ return m_uType; }
	UINT GetNameID() const			{ return m_uName; }
	LPCSTR GetName() const			{ return m_pszName; }
	
	bool IsStr() const				{ return m_uType == TAGTYPE_STRING; }
	bool IsInt() const				{ return m_uType == TAGTYPE_UINT32; }
	bool IsFloat() const			{ return m_uType == TAGTYPE_FLOAT32; }
	bool IsHash() const				{ return m_uType == TAGTYPE_HASH; }
	bool IsBlob() const				{ return m_uType == TAGTYPE_BLOB; }
	bool IsInt64(bool bOrInt32 = true) const { return m_uType == TAGTYPE_UINT64 || (bOrInt32 && IsInt()); }
	
	UINT	GetInt() const			{ ASSERT(IsInt());		return (UINT)m_uVal; }
	uint64_t	GetInt64() const		{ ASSERT(IsInt64(true));return m_uVal; }
	const	CString& GetStr() const	{ ASSERT(IsStr());		return *m_pstrVal; }
	float	GetFloat() const		{ ASSERT(IsFloat());	return m_fVal; }
	const	BYTE* GetHash() const	{ ASSERT(IsHash());		return m_pData; }
	uint32_t	GetBlobSize() const		{ ASSERT(IsBlob());		return m_nBlobSize; }
	const	BYTE* GetBlob() const	{ ASSERT(IsBlob());		return m_pData; }

	void SetInt(UINT uVal);
	void SetInt64(uint64_t uVal);
	void SetStr(LPCTSTR pszVal);
	
	CTag* CloneTag()				{ return new CTag(*this); }
	
	bool WriteTagToFile(CFileDataIO* file, EUtf8Str eStrEncode = utf8strNone) const;	// old eD2K tags
	bool WriteNewEd2kTag(CFileDataIO* file, EUtf8Str eStrEncode = utf8strNone) const;	// new eD2K tags
	
	CString GetFullInfo(CString (*pfnDbgGetFileMetaTagName)(UINT uMetaTagID) = NULL) const;

#ifdef _DEBUG
	// Diagnostic Support
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	uint8_t	m_uType;
	uint8_t	m_uName;
	LPSTR	m_pszName;
	uint32_t	m_nBlobSize;
	union{
	  CString*	m_pstrVal;
	  uint64_t	m_uVal;
	  float		m_fVal;
	  BYTE*		m_pData;
	};
};


///////////////////////////////////////////////////////////////////////////////
// CTag and tag string helpers

__inline int CmpED2KTagName(LPCSTR pszTagName1, LPCSTR pszTagName2){
	// string compare is independant from any codepage and/or LC_CTYPE setting.
	return _stricmp(pszTagName1, pszTagName2);
}
void ConvertED2KTag(CTag*& pTag);
