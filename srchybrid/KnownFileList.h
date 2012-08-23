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
#include "MapKey.h"
// ismod: no aich

class CKnownFile;
typedef CRBMap<CCKey,CKnownFile*,CCKeyTrait> CKnownFilesMap;
typedef CRBMap<CSKey,int,CSKeyTrait> CancelledFilesMap;
// ismod: no aich

class CKnownFileList 
{
	friend class CFileDetailDlgStatistics;
	friend class CStatisticFile;
public:
	CKnownFileList();
	~CKnownFileList();

	bool	SafeAddKFile(CKnownFile* toadd);
	bool	Init();
	void	Save();
	void	Clear();
	void	Process();

	CKnownFile* FindKnownFile(LPCTSTR filename, time_t date, uint64_t size) const;
	CKnownFile* FindKnownFileByID(const uchar* hash) const;
	CKnownFile* FindKnownFileByPath(LPCTSTR sFilePath) const;
	bool	IsKnownFile(const CKnownFile* file) const;
	bool	IsFilePtrInList(const CKnownFile* file) const;

	void	AddCancelledFileID(const uchar* hash);
	bool	IsCancelledFileByID(const uchar* hash) const;

	const CKnownFilesMap& GetKnownFiles() const { return m_Files_map; }
	void	CopyKnownFileMap(CRBMap<CCKey,CKnownFile*,CCKeyTrait> &Files_Map);

	// ismod: no aich
	uint32_t 	m_nRequestedTotal;
	uint32_t 	m_nAcceptedTotal;
	uint64_t 	m_nTransferredTotal;

private:
	bool	LoadKnownFiles();
	bool	LoadCancelledFiles();

	uint16_t 	requested;
	uint16_t 	accepted;
	uint64_t 	transferred;
	time_t 	m_nLastSaved;
	CKnownFilesMap		m_Files_map;
	CancelledFilesMap	m_mapCancelledFiles;
	// ismod: no aich
	uint32_t	m_dwCancelledFilesSeed;
};
