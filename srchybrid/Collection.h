//this file is part of eMule
//Copyright (C)2002 Merkur ( merkur-@users.sourceforge.net / http://www.emule-project.net )
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

#pragma warning(push)
#pragma warning(disable:4244 4100 4189)
#include <cryptopp/rsa.h>
#include <cryptopp/base64.h>
#include <cryptopp/osrng.h>
#include <cryptopp/files.h>
#include <cryptopp/sha.h>
#pragma warning(pop)
#include "MapKey.h"

#define COLLECTION_FILEEXTENSION	_T(".emulecollection")

class CAbstractFile;
class CCollectionFile;

typedef CMap<CSKey,const CSKey&,CCollectionFile*,CCollectionFile*> CCollectionFilesMap;

class CCollection
{
	friend class CCollectionCreateDialog;
	friend class CCollectionViewDialog;
public:
	CCollection(void);
	CCollection(const CCollection* pCollection);
	~CCollection(void);
	bool InitCollectionFromFile(const CString& sFilePath, CString sFileName);
	CCollectionFile* AddFileToCollection(CAbstractFile* pAbstractFile, bool bCreateClone);
	void RemoveFileFromCollection(CAbstractFile* pAbstractFile);
	void WriteToFileAddShared(CryptoPP::RSASSA_PKCS1v15_SHA_Signer* pSignkey = NULL);
	void SetCollectionAuthorKey(const byte* abyCollectionAuthorKey, uint32_t nSize);
	CString GetCollectionAuthorKeyString();
	static bool HasCollectionExtention(const CString& sFileName);
	CString	GetAuthorKeyHashString();

	CString m_sCollectionName;
	CString m_sCollectionAuthorName;

	// ismod
	CString m_sCollectionFolder;

	bool m_bTextFormat;

private:
	CCollectionFilesMap m_CollectionFilesMap;
	byte*	m_pabyCollectionAuthorKey;
	uint32_t	m_nKeySize;
};
