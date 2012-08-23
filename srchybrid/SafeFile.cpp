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
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#include "stdafx.h"
#include "SafeFile.h"
#include "Packets.h"
#include "StringConversion.h"
#include "kademlia/utils/UInt128.h"
#include <atlenc.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


///////////////////////////////////////////////////////////////////////////////
// CFileDataIO

uint8_t CFileDataIO::ReadUInt8()
{
	uint8_t nVal;
	Read(&nVal, sizeof nVal);
	return nVal;
}

uint16_t CFileDataIO::ReadUInt16()
{
	uint16_t nVal;
	Read(&nVal, sizeof nVal);
	return nVal;
}

uint32_t CFileDataIO::ReadUInt32()
{
	uint32_t nVal;
	Read(&nVal, sizeof nVal);
	return nVal;
}

uint64_t CFileDataIO::ReadUInt64()
{
	uint64_t nVal;
	Read(&nVal, sizeof nVal);
	return nVal;
}

void CFileDataIO::ReadUInt128(Kademlia::CUInt128 *pVal)
{
	Read(pVal->GetDataPtr(), 16);
}

Kademlia::CUInt128 CFileDataIO::ReadUInt128()
{
	Kademlia::CUInt128	val;
	Read(val.GetDataPtr(), 16);
	return val;
}

void CFileDataIO::ReadHash16(uchar* pVal)
{
	Read(pVal, 16);
}

CString CFileDataIO::ReadString(bool bOptUTF8, UINT uRawSize)
{
	const UINT uMaxShortRawSize = SHORT_RAW_ED2K_UTF8_STR;
	if (uRawSize <= uMaxShortRawSize)
	{
		char acRaw[uMaxShortRawSize];
		Read(acRaw, uRawSize);
		if (uRawSize >= 3 && (UCHAR)acRaw[0] == 0xEFU && (UCHAR)acRaw[1] == 0xBBU && (UCHAR)acRaw[2] == 0xBFU)
		{
			WCHAR awc[uMaxShortRawSize];
			int iChars = ByteStreamToWideChar(acRaw + 3, uRawSize - 3, awc, _countof(awc));
			if (iChars >= 0)
				return CStringW(awc, iChars);
		}
		else if (bOptUTF8)
		{
			WCHAR awc[uMaxShortRawSize];
			//int iChars = ByteStreamToWideChar(acRaw, uRawSize, awc, ARRSIZE(awc));
			int iChars = utf8towc(acRaw, uRawSize, awc, _countof(awc));
			if (iChars >= 0)
				return CStringW(awc, iChars);
		}
		return CStringW(acRaw, uRawSize); // use local codepage
	}
	else
	{
		Array<char> acRaw(uRawSize);
		Read(acRaw, uRawSize);
		if (uRawSize >= 3 && (UCHAR)acRaw[0] == 0xEFU && (UCHAR)acRaw[1] == 0xBBU && (UCHAR)acRaw[2] == 0xBFU)
		{
			Array<WCHAR> awc(uRawSize);
			int iChars = ByteStreamToWideChar(acRaw + 3, uRawSize - 3, awc, uRawSize);
			if (iChars >= 0)
				return CStringW(awc, iChars);
		}
		else if (bOptUTF8)
		{
			Array<WCHAR> awc(uRawSize);
			//int iChars = ByteStreamToWideChar(acRaw, uRawSize, awc, uRawSize);
			int iChars = utf8towc(acRaw, uRawSize, awc, uRawSize);
			if (iChars >= 0)
				return CStringW(awc, iChars);
		}
		return CStringW(acRaw, uRawSize); // use local codepage
	}
}

CString CFileDataIO::ReadString(bool bOptUTF8)
{
	UINT uLen = ReadUInt16();
	return ReadString(bOptUTF8, uLen);
}

CStringW CFileDataIO::ReadStringUTF8()
{
	UINT uRawSize = ReadUInt16();
	const UINT uMaxShortRawSize = SHORT_RAW_ED2K_UTF8_STR;
	if (uRawSize <= uMaxShortRawSize)
	{
		char acRaw[uMaxShortRawSize];
		Read(acRaw, uRawSize);
		WCHAR awc[uMaxShortRawSize];
		int iChars = ByteStreamToWideChar(acRaw, uRawSize, awc, _countof(awc));
		if (iChars >= 0)
			return CStringW(awc, iChars);
		return CStringW(acRaw, uRawSize); // use local codepage
	}
	else
	{
		Array<char> acRaw(uRawSize);
		Read(acRaw, uRawSize);
		Array<WCHAR> awc(uRawSize);
		int iChars = ByteStreamToWideChar(acRaw, uRawSize, awc, uRawSize);
		if (iChars >= 0)
			return CStringW(awc, iChars);
		return CStringW(acRaw, uRawSize); // use local codepage;
	}
}

void CFileDataIO::WriteUInt8(uint8_t nVal)
{
	Write(&nVal, sizeof nVal);
}

void CFileDataIO::WriteUInt16(uint16_t nVal)
{
	Write(&nVal, sizeof nVal);
}

void CFileDataIO::WriteUInt32(uint32_t nVal)
{
	Write(&nVal, sizeof nVal);
}

void CFileDataIO::WriteUInt64(uint64_t nVal)
{
	Write(&nVal, sizeof nVal);
}

void CFileDataIO::WriteUInt128(const Kademlia::CUInt128 *pVal)
{
	Write(pVal->GetData(), 16);
}

void CFileDataIO::WriteUInt128(const Kademlia::CUInt128 &rVal)
{
	Write(rVal.GetData(), 16);
}

void CFileDataIO::WriteHash16(const uchar* pVal)
{
	Write(pVal, 16);
}

void CFileDataIO::WriteString(const CString& rstr, EUtf8Str eEncode)
{
#define	WRITE_STR_LEN(n)	WriteUInt16((uint16_t)(n))
	if (eEncode == utf8strRaw)
	{
		CUnicodeToUTF8 utf8(rstr);
		WRITE_STR_LEN(utf8.GetLength());
		Write((LPCSTR)utf8, utf8.GetLength());
	}
	else if (eEncode == utf8strOptBOM)
	{
		/* Borschtsch - love Unicode
		if (NeedUTF8String(rstr))
		{
		*/
			CUnicodeToBOMUTF8 bomutf8(rstr);
			WRITE_STR_LEN(bomutf8.GetLength());
			Write((LPCSTR)bomutf8, bomutf8.GetLength());
		/* Borschtsch - love Unicode
		}
		else
		{
			CUnicodeToMultiByte mb(rstr);
			WRITE_STR_LEN(mb.GetLength());
			Write((LPCSTR)mb, mb.GetLength());
		}
		*/
	}
	else
	{
		CUnicodeToMultiByte mb(rstr);
		WRITE_STR_LEN(mb.GetLength());
		Write((LPCSTR)mb, mb.GetLength());
	}
#undef WRITE_STR_LEN
}

void CFileDataIO::WriteString(LPCSTR psz)
{
	size_t uLen = strlen(psz);
	ASSERT( uLen <= std::numeric_limits<uint16_t>::max() );
	WriteUInt16(static_cast<uint16_t>(uLen));
	Write(psz, uLen);
}

void CFileDataIO::WriteLongString(const CString& rstr, EUtf8Str eEncode)
{
#define	WRITE_STR_LEN(n)	WriteUInt32(n)
	if (eEncode == utf8strRaw)
	{
		CUnicodeToUTF8 utf8(rstr);
		WRITE_STR_LEN(utf8.GetLength());
		Write((LPCSTR)utf8, utf8.GetLength());
	}
	else if (eEncode == utf8strOptBOM)
	{
		/* Borschtsch - love Unicode
		if (NeedUTF8String(rstr))
		{
		*/
			CUnicodeToBOMUTF8 bomutf8(rstr);
			WRITE_STR_LEN(bomutf8.GetLength());
			Write((LPCSTR)bomutf8, bomutf8.GetLength());
		/* Borschtsch - love Unicode
		}
		else
		{
			CUnicodeToMultiByte mb(rstr);
			WRITE_STR_LEN(mb.GetLength());
			Write((LPCSTR)mb, mb.GetLength());
		}
		*/
	}
	else
	{
		CUnicodeToMultiByte mb(rstr);
		WRITE_STR_LEN(mb.GetLength());
		Write((LPCSTR)mb, mb.GetLength());
	}
#undef WRITE_STR_LEN
}

void CFileDataIO::WriteLongString(LPCSTR psz)
{
	UINT uLen = static_cast<UINT>(strlen(psz));
	WriteUInt32(uLen);
	Write(psz, uLen);
}

///////////////////////////////////////////////////////////////////////////////
// CSafeFile

UINT CSafeFile::Read(void* lpBuf, UINT nCount)
{
	if (GetPosition() + nCount > GetLength())
		AfxThrowFileException(CFileException::endOfFile, 0, GetFileName());
	return CFile::Read(lpBuf, nCount);
}

void CSafeFile::Write(const void* lpBuf, UINT nCount)
{
	CFile::Write(lpBuf, nCount);
}

ULONGLONG CSafeFile::Seek(LONGLONG lOff, UINT nFrom)
{
	return CFile::Seek(lOff, nFrom);
}

ULONGLONG CSafeFile::GetPosition() const
{
	return CFile::GetPosition();
}

ULONGLONG CSafeFile::GetLength() const {
	return CFile::GetLength();
}


///////////////////////////////////////////////////////////////////////////////
// CSafeMemFile

UINT CSafeMemFile::Read(void* lpBuf, UINT nCount)
{
	if (m_nPosition + nCount > m_nFileSize)
		AfxThrowFileException(CFileException::endOfFile, 0, GetFileName());
	return CMemFile::Read(lpBuf, nCount);
}

void CSafeMemFile::Write(const void* lpBuf, UINT nCount)
{
	CMemFile::Write(lpBuf, nCount);
}

ULONGLONG CSafeMemFile::Seek(LONGLONG lOff, UINT nFrom)
{
	return CMemFile::Seek(lOff, nFrom);
}

uint8_t CSafeMemFile::ReadUInt8()
{
	if (m_nPosition + sizeof(uint8_t) > m_nFileSize)
		AfxThrowFileException(CFileException::endOfFile, 0, GetFileName());
	return *(m_lpBuffer + m_nPosition++);
}

uint16_t CSafeMemFile::ReadUInt16()
{
	if (m_nPosition + sizeof(uint16_t) > m_nFileSize)
		AfxThrowFileException(CFileException::endOfFile, 0, GetFileName());
	uint16_t nResult = *((uint16_t*)(m_lpBuffer + m_nPosition));
	m_nPosition += sizeof(uint16_t);
	return nResult;
}

uint32_t CSafeMemFile::ReadUInt32()
{
	if (m_nPosition + sizeof(uint32_t) > m_nFileSize)
		AfxThrowFileException(CFileException::endOfFile, 0, GetFileName());
	uint32_t nResult = *((uint32_t*)(m_lpBuffer + m_nPosition));
	m_nPosition += sizeof(uint32_t);
	return nResult;
}

uint64_t CSafeMemFile::ReadUInt64()
{
	if (m_nPosition + sizeof(uint64_t) > m_nFileSize)
		AfxThrowFileException(CFileException::endOfFile, 0, GetFileName());
	uint64_t nResult = *((uint64_t*)(m_lpBuffer + m_nPosition));
	m_nPosition += sizeof(uint64_t);
	return nResult;
}

//Borschtsch
float CSafeMemFile::ReadFloat32()
{
	if (m_nPosition + sizeof(float) > m_nFileSize)
		AfxThrowFileException(CFileException::endOfFile, 0, GetFileName());
	float nResult = *((float*)(m_lpBuffer + m_nPosition));
	m_nPosition += sizeof(float);
	return nResult;
}

void CSafeMemFile::ReadUInt128(Kademlia::CUInt128* pVal)
{
	if (m_nPosition + sizeof(uint32_t)*4 > m_nFileSize)
		AfxThrowFileException(CFileException::endOfFile, 0, GetFileName());
	uint32_t* pUInt32Val = (uint32_t*)pVal->GetDataPtr();
	const uint32_t* pUInt32 = (uint32_t*)(m_lpBuffer + m_nPosition);
	pUInt32Val[0] = pUInt32[0];
	pUInt32Val[1] = pUInt32[1];
	pUInt32Val[2] = pUInt32[2];
	pUInt32Val[3] = pUInt32[3];
	m_nPosition += sizeof(uint32_t)*4;
}

Kademlia::CUInt128 CSafeMemFile::ReadUInt128()
{
	if (m_nPosition + sizeof(uint32_t)*4 > m_nFileSize)
		AfxThrowFileException(CFileException::endOfFile, 0, GetFileName());
	const uint32_t* pUInt32 = (uint32_t*)(m_lpBuffer + m_nPosition);
	m_nPosition += sizeof(uint32_t)*4;
	return Kademlia::CUInt128(pUInt32[0], pUInt32[1], pUInt32[2], pUInt32[3]);
}

void CSafeMemFile::ReadHash16(uchar* pVal)
{
	if (m_nPosition + sizeof(uint32_t)*4 > m_nFileSize)
		AfxThrowFileException(CFileException::endOfFile, 0, GetFileName());
	const uint32_t* pUInt32 = (uint32_t*)(m_lpBuffer + m_nPosition);
	((uint32_t*)pVal)[0] = pUInt32[0];
	((uint32_t*)pVal)[1] = pUInt32[1];
	((uint32_t*)pVal)[2] = pUInt32[2];
	((uint32_t*)pVal)[3] = pUInt32[3];
	m_nPosition += sizeof(uint32_t)*4;
}

void CSafeMemFile::WriteUInt8(uint8_t nVal)
{
	if (m_nPosition + sizeof(uint8_t) > m_nBufferSize)
		GrowFile(m_nPosition + sizeof(uint8_t));
	*(m_lpBuffer + m_nPosition++) = nVal;
	if (m_nPosition > m_nFileSize)
		m_nFileSize = m_nPosition;
}

void CSafeMemFile::WriteUInt16(uint16_t nVal)
{
	if (m_nPosition + sizeof(uint16_t) > m_nBufferSize)
		GrowFile(m_nPosition + sizeof(uint16_t));
	*((uint16_t*)(m_lpBuffer + m_nPosition)) = nVal;
	m_nPosition += sizeof(uint16_t);
	if (m_nPosition > m_nFileSize)
		m_nFileSize = m_nPosition;
}

void CSafeMemFile::WriteUInt32(uint32_t nVal)
{
	if (m_nPosition + sizeof(uint32_t) > m_nBufferSize)
		GrowFile(m_nPosition + sizeof(uint32_t));
	*((uint32_t*)(m_lpBuffer + m_nPosition)) = nVal;
	m_nPosition += sizeof(uint32_t);
	if (m_nPosition > m_nFileSize)
		m_nFileSize = m_nPosition;
}

void CSafeMemFile::WriteUInt64(uint64_t nVal)
{
	if (m_nPosition + sizeof(uint64_t) > m_nBufferSize)
		GrowFile(m_nPosition + sizeof(uint64_t));
	*((uint64_t*)(m_lpBuffer + m_nPosition)) = nVal;
	m_nPosition += sizeof(uint64_t);
	if (m_nPosition > m_nFileSize)
		m_nFileSize = m_nPosition;
}

//Borschtsch
void CSafeMemFile::WriteFloat32(float nVal)
{
	if (m_nPosition + sizeof(float) > m_nBufferSize)
		GrowFile(m_nPosition + sizeof(float));
	*((float*)(m_lpBuffer + m_nPosition)) = nVal;
	m_nPosition += sizeof(float);
	if (m_nPosition > m_nFileSize)
		m_nFileSize = m_nPosition;
}

void CSafeMemFile::WriteUInt128(const Kademlia::CUInt128* pVal)
{
	if (m_nPosition + sizeof(uint32_t)*4 > m_nBufferSize)
		GrowFile(m_nPosition + sizeof(uint32_t)*4);
	uint32_t* pUInt32 = (uint32_t*)(m_lpBuffer + m_nPosition);
	const uint32_t* pUInt32Val = (uint32_t*)pVal->GetData();
	pUInt32[0] = pUInt32Val[0];
	pUInt32[1] = pUInt32Val[1];
	pUInt32[2] = pUInt32Val[2];
	pUInt32[3] = pUInt32Val[3];
	m_nPosition += sizeof(uint32_t)*4;
	if (m_nPosition > m_nFileSize)
		m_nFileSize = m_nPosition;
}

void CSafeMemFile::WriteUInt128(const Kademlia::CUInt128& pVal)
{
	if (m_nPosition + sizeof(uint32_t)*4 > m_nBufferSize)
		GrowFile(m_nPosition + sizeof(uint32_t)*4);
	uint32_t* pUInt32 = (uint32_t*)(m_lpBuffer + m_nPosition);
	const uint32_t* pUInt32Val = (uint32_t*)pVal.GetData();
	pUInt32[0] = pUInt32Val[0];
	pUInt32[1] = pUInt32Val[1];
	pUInt32[2] = pUInt32Val[2];
	pUInt32[3] = pUInt32Val[3];
	m_nPosition += sizeof(uint32_t)*4;
	if (m_nPosition > m_nFileSize)
		m_nFileSize = m_nPosition;
}

void CSafeMemFile::WriteHash16(const uchar* pVal)
{
	if (m_nPosition + sizeof(uint32_t)*4 > m_nBufferSize)
		GrowFile(m_nPosition + sizeof(uint32_t)*4);
	uint32_t* pUInt32 = (uint32_t*)(m_lpBuffer + m_nPosition);
	pUInt32[0] = ((uint32_t*)pVal)[0];
	pUInt32[1] = ((uint32_t*)pVal)[1];
	pUInt32[2] = ((uint32_t*)pVal)[2];
	pUInt32[3] = ((uint32_t*)pVal)[3];
	m_nPosition += sizeof(uint32_t)*4;
	if (m_nPosition > m_nFileSize)
		m_nFileSize = m_nPosition;
}

ULONGLONG CSafeMemFile::GetPosition() const
{
	return CMemFile::GetPosition();
}

ULONGLONG CSafeMemFile::GetLength() const {
	return CMemFile::GetLength();
}


///////////////////////////////////////////////////////////////////////////////
// CSafeBufferedFile

UINT CSafeBufferedFile::Read(void* lpBuf, UINT nCount)
{
	// that's terrible slow
//	if (GetPosition()+nCount > this->GetLength())
//		AfxThrowFileException(CFileException::endOfFile, 0, GetFileName());
	UINT uRead = CStdioFile::Read(lpBuf,nCount);
	if (uRead != nCount)
		AfxThrowFileException(CFileException::endOfFile, 0, GetFileName());
	return uRead;
}

void CSafeBufferedFile::Write(const void* lpBuf, UINT nCount)
{
	CStdioFile::Write(lpBuf, nCount);
}

ULONGLONG CSafeBufferedFile::Seek(LONGLONG lOff, UINT nFrom)
{
	return CStdioFile::Seek(lOff, nFrom);
}

ULONGLONG CSafeBufferedFile::GetPosition() const
{
	return CStdioFile::GetPosition();
}

ULONGLONG CSafeBufferedFile::GetLength() const {
	return CStdioFile::GetLength();
}

int CSafeBufferedFile::printf(LPCTSTR pszFmt, ...)
{
	va_list args;
	va_start(args, pszFmt);
	int iResult = _vftprintf(m_pStream, pszFmt, args);
	va_end(args);
	if (iResult < 0) {
#if _MFC_VER>=0x0800
		AfxThrowFileException(CFileException::genericException, _doserrno, m_strFileName);
#else
		AfxThrowFileException(CFileException::generic, _doserrno, m_strFileName);
#endif
	}
	return iResult;
}
