
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
// ismod: shared dirs
#include "SharedDir.h"

class CKnownFileList;
class CServerConnect;
class CPartFile;
class CKnownFile;
class CPublishKeywordList;
class CSafeMemFile;
class CServer;
class CCollection;

struct UnknownFile_Struct{
	CString strName;
	CString strDirectory;
	CString strSharedDirectory;
};

class CSharedFileList
{
	friend class CSharedFilesCtrl;
	friend class CClientReqSocket;

public:
	CSharedFileList(CServerConnect* in_server);
	~CSharedFileList();

	void	SendListToServer();
	void	Reload();
	void	Save() const;
	void	Process();
	void	Publish();
	void	RebuildMetaData();
	void	DeletePartFileInstances() const;
	void	PublishNextTurn()													{ m_lastPublishED2KFlag=true;	}
	void	ClearED2KPublishInfo();
	void	ClearKadSourcePublishInfo();

	void	CreateOfferedFilePacket(CKnownFile* cur_file, CSafeMemFile* files, CServer* pServer, CUpDownClient* pClient = NULL);

	bool	SafeAddKFile(CKnownFile* toadd, bool bOnlyAdd = false);
	void	RepublishFile(CKnownFile* pFile);
	void	SetOutputCtrl(CSharedFilesCtrl* in_ctrl);	
	bool	RemoveFile(CKnownFile* toremove, bool bDeleted = false);	// removes a specific shared file from the list
	void	UpdateFile(CKnownFile* toupdate);
	void	AddFileFromNewlyCreatedCollection(const CString& rstrFilePath)		{ CheckAndAddSingleFile(rstrFilePath); }

	// GUI is not initially updated 
	bool	AddSingleSharedFile(const CString& rstrFilePath, bool bNoUpdate = false); // includes updating sharing preferences, calls CheckAndAddSingleSharedFile afterwards
	bool	AddSingleSharedDirectory(const CString& rstrFilePath, bool bNoUpdate = false); 
	bool	ExcludeFile(CString strFilePath);	// excludes a specific file from being shared and removes it from the list if it exists
	
	void	AddKeywords(CKnownFile* pFile);
	void	RemoveKeywords(CKnownFile* pFile);

	void	CopySharedFileMap(CRBMap<CCKey,CKnownFile*,CCKeyTrait> &Files_Map);	
	
	CKnownFile* GetFileByID(const uchar* filehash) const;
	CKnownFile*	GetFileByIndex(int index);
	bool	IsFilePtrInList(const CKnownFile* file) const; // slow
	bool	IsUnsharedFile(const uchar* auFileHash) const;
	bool	ShouldBeShared(const CString &strPath, const CString &strFilePath, bool bMustBeShared) const;
	bool	ContainsSingleSharedFiles(CString strDirectory) const; // includes subdirs
	CString GetPseudoDirName(const CString& strDirectoryName);
	CString GetDirNameByPseudo(const CString& strPseudoName) const;

	uint64_t	GetDatasize(uint64_t &pbytesLargest) const;
	int		GetCount()	{return (int) m_Files_map.GetCount(); }
	size_t	GetHashingCount()													{ return m_waitingForHashList.size() + (m_currentlyHashingFile==NULL?0:1); }
	bool	ProbablyHaveSingleSharedFiles() const								{ return bHaveSingleSharedFiles && !m_liSingleSharedFiles.IsEmpty(); } // might not be always up-to-date, could give false "true"s, not a problem currently

	void	HashFailed(UnknownFile_Struct* hashed);		// SLUGFILLER: SafeHash
	void	FileHashingFinished(CKnownFile* file);

	bool	GetPopularityRank(const CKnownFile* pFile, uint32_t& rnOutSession, uint32_t& rnOutTotal) const;
	CMutex	m_mutWriteList;

	// ismod: moved from proteced
	void	AddFilesFromDirectory(const CString& rstrDirectory);

protected:
	bool	AddFile(CKnownFile* pFile);
	// ismod: moved to public
	void	FindSharedFiles();
	
	void	HashNextFile();
	bool	IsHashing(const CString& rstrDirectory, const CString& rstrName);
	void	RemoveFromHashing(CKnownFile* hashed);
	void	LoadSingleSharedFilesList();

	void	CheckAndAddSingleFile(const CFileFind& ff);
	bool	CheckAndAddSingleFile(const CString& rstrFilePath); // add specific files without editing sharing preferences

private:
	/* ismod
    CMap<CCKey,const CCKey&,CKnownFile*,CKnownFile*> m_Files_map;
    */
    CRBMap<CCKey,CKnownFile*, CCKeyTrait>	m_Files_map; 
	CRBMap<CSKey,bool, CSKeyTrait>	m_UnsharedFiles_map;
	CRBMap<CString,CString> m_mapPseudoDirNames;
	CPublishKeywordList* m_keywords;
	std::list<UnknownFile_Struct*> m_waitingForHashList;
	UnknownFile_Struct* m_currentlyHashingFile;
	CServerConnect*		server;
	CSharedFilesCtrl*	output;
	CStringList			m_liSingleSharedFiles;
	CStringList			m_liSingleExcludedFiles;

	time_t m_lastPublishED2K;
	bool	 m_lastPublishED2KFlag;
	int m_currFileSrc;
	int m_currFileNotes;
	int m_currFileKey;
	time_t m_lastPublishKadSrc;
	time_t m_lastPublishKadNotes;
	bool bHaveSingleSharedFiles;

// ismod [start]
public:
	SharedDir* GetSharedDirByCollName(const CString& collName) { return sharedDirsPseudo[collName]; }
	uint64_t GetTotalShareSize() const { return m_uTotalShareSize; }
	SharedDirsMap sharedDirs;
private:
    void CreateOfferedUserPacket(CSafeMemFile* files, CServer* pServer) const;
    bool m_bUserPublished;
	uint64_t m_uTotalShareSize;

	SharedDirsMap sharedDirsPseudo;
// ismod [end]
};

class CAddFileThread : public CSafeThread
{
	DECLARE_DYNCREATE(CAddFileThread)
protected:
	CAddFileThread();
public:
	virtual BOOL InitInstance();
	virtual int	Run();
	void	SetValues(CSharedFileList* pOwner, LPCTSTR directory, LPCTSTR filename, LPCTSTR strSharedDir, CPartFile* partfile = NULL);

private:
	CSharedFileList* m_pOwner;
	CString			 m_strDirectory;
	CString			 m_strFilename;
	CString			 m_strSharedDir;
	CPartFile*		 m_partfile;
};
