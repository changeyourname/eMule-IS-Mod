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

#pragma once

#include "BarShader.h"
#include "StatisticFile.h"
#include "ShareableFile.h"
#include "HashArray.h"

class CxImage;
class CUpDownClient;
class Packet;
class CFileDataIO;
// ismod: no aich
class CCollection;

typedef CTypedPtrList<CPtrList, CUpDownClient*> CUpDownClientPtrList;

class CKnownFile : public CShareableFile
{
	DECLARE_DYNAMIC(CKnownFile)

public:
	CKnownFile();
	virtual ~CKnownFile();

	virtual void SetFileName(LPCTSTR pszFileName, bool bReplaceInvalidFileSystemChars = false, bool bRemoveControlChars = false); // 'bReplaceInvalidFileSystemChars' is set to 'false' for backward compatibility!

	bool	CreateFromFile(LPCTSTR directory, LPCTSTR filename, LPVOID pvProgressParam); // create date, hashset and tags from a file
	bool	LoadFromFile(CFileDataIO* file);	//load date, hashset and tags from a .met file
	bool	WriteToFile(CFileDataIO* file);
	// ismod: no aich

	// last file modification time in (DST corrected, if NTFS) real UTC format
	// NOTE: this value can *not* be compared with NT's version of the UTC time
	CTime	GetUtcCFileDate() const										{ return CTime(m_tUtcLastModified); }
	time_t	GetUtcFileDate() const										{ return m_tUtcLastModified; }

	// Did we not see this file for a long time so that some information should be purged?
	bool	ShouldPartiallyPurgeFile() const;
	void	SetLastSeen()												{ m_timeLastSeen = time(NULL); }

	virtual void	SetFileSize(EMFileSize nFileSize);

	// local available part hashes
	const CHashArray&	hashset() const { return m_hasharray; }
	const uchar*		hashset(uint16_t part) const { return m_hasharray[part]; }
	bool				setHashset( const CHashArray& hashset );

	// SLUGFILLER: SafeHash remove - removed unnececery hash counter
	/*
	// nr. of part hashs according the file size wrt ED2K protocol
	uint16	GetED2KPartHashCount() const { return m_iED2KPartHashCount; }
	*/

	// nr. of 9MB parts (file data)
	uint16_t partCount() const								{ return m_partCount; }

	// nr. of 9MB parts according the file size wrt ED2K protocol (OP_FILESTATUS)
	uint16_t GetED2KPartCount() const { return m_ED2KPartCount; }

	// file upload priority
	uint8_t	GetUpPriority(void) const									{ return m_iUpPriority; }
	void	SetUpPriority(uint8_t iNewUpPriority, bool bSave = true);
	bool	IsAutoUpPriority(void) const								{ return m_bAutoUpPriority; }
	void	SetAutoUpPriority(bool NewAutoUpPriority)					{ m_bAutoUpPriority = NewAutoUpPriority; }
	void	UpdateAutoUpPriority();

	// This has lost it's meaning here.. This is the total clients we know that want this file..
	// Right now this number is used for auto priorities..
	// This may be replaced with total complete source known in the network..
	uint32_t	GetQueuedCount() { return (UINT) m_ClientUploadList.GetCount();}

	bool	LoadHashsetFromFile(CFileDataIO* file, bool checkhash);

	void	AddUploadingClient(CUpDownClient* client);
	void	RemoveUploadingClient(CUpDownClient* client);
	virtual void	UpdatePartsInfo();
	virtual	void	DrawShareStatusBar(CDC* dc, LPCRECT rect, bool onlygreyrect, bool bFlat) const;

	// comment
	void	SetFileComment(LPCTSTR pszComment);

	void	SetFileRating(UINT uRating);

	bool	GetPublishedED2K() const { return m_PublishedED2K; }
	void	SetPublishedED2K(bool val);

	uint32_t	GetKadFileSearchID() const { return kadFileSearchID; }
	void	SetKadFileSearchID(uint32_t id) { kadFileSearchID = id; } //Don't use this unless you know what your are DOING!! (Hopefully I do.. :)

	const Kademlia::WordList& GetKadKeywords() const { return wordlist; }

	time_t	GetLastPublishTimeKadSrc() const { return m_lastPublishTimeKadSrc; }
	void	SetLastPublishTimeKadSrc(time_t time, uint32_t buddyip) { m_lastPublishTimeKadSrc = time; m_lastBuddyIP = buddyip;}
	time_t	GetLastPublishBuddy() const { return m_lastBuddyIP; }
	void	SetLastPublishTimeKadNotes(time_t time) {m_lastPublishTimeKadNotes = time;}
	time_t	GetLastPublishTimeKadNotes() const { return m_lastPublishTimeKadNotes; }

	bool	PublishSrc();
	bool	PublishNotes();

	// file sharing
	virtual Packet* CreateSrcInfoPacket(const CUpDownClient* forClient, uint8_t byRequestedVersion, uint16_t nRequestedOptions) const;
	UINT	GetMetaDataVer() const { return m_uMetaDataVer; }
	void	UpdateMetaDataTags();
	void	RemoveMetaDataTags(UINT uTagType = 0);
	void	RemoveBrokenUnicodeMetaDataTags();

	// preview
	bool	IsMovie() const;
	virtual bool GrabImage(uint8_t nFramesToGrab, double dStartTime, bool bReduceColor, uint16_t nMaxWidth, void* pSender);
	virtual void GrabbingFinished(CxImage** imgResults, uint8_t nFramesGrabbed, void* pSender);

	// ismod: no aich
	// Display / Info / Strings
	virtual CString	GetInfoSummary(bool bNoFormatCommands = false) const;
	CString			GetUpPriorityDisplayString() const;



	// last file modification time in (DST corrected, if NTFS) real UTC format
	// NOTE: this value can *not* be compared with NT's version of the UTC time
	time_t	m_tUtcLastModified;

	CStatisticFile statistic;
	time_t m_nCompleteSourcesTime;
	uint16_t m_nCompleteSourcesCount;
	uint16_t m_nCompleteSourcesCountLo;
	uint16_t m_nCompleteSourcesCountHi;
	CUpDownClientPtrList m_ClientUploadList;
	std::vector<uint16_t> m_AvailPartFrequency;
	CCollection* m_pCollection;

#ifdef _DEBUG
	// Diagnostic Support
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	//preview
	bool	GrabImage(LPCTSTR strFileName, uint8_t nFramesToGrab, double dStartTime, bool bReduceColor, uint16_t nMaxWidth, void* pSender);
	bool	LoadTagsFromFile(CFileDataIO* file);
	bool	LoadDateFromFile(CFileDataIO* file);
	static void	CreateHash(CFile* pFile, uint64_t uSize, uchar* pucHash);
	static bool	CreateHash(FILE* fp, uint64_t uSize, uchar* pucHash);
	static bool	CreateHash(const uchar* pucData, uint64_t uSize, uchar* pucHash);
	virtual void	UpdateFileRatingCommentAvail(bool bForceUpdate = false);

	CHashArray	m_hasharray;

private:
	static CBarShader s_ShareStatusBar;
	uint16_t	m_partCount;
	uint16_t	m_ED2KPartCount;
	uint16_t	m_ED2KPartHashCount;
	uint8_t	m_iUpPriority;
	bool	m_bAutoUpPriority;
	bool	m_PublishedED2K;
	uint32_t	kadFileSearchID;
	time_t	m_lastPublishTimeKadSrc;
	time_t	m_lastPublishTimeKadNotes;
	uint32_t	m_lastBuddyIP;
	Kademlia::WordList wordlist;
	UINT	m_uMetaDataVer;
	time_t	m_timeLastSeen; // we only "see" files when they are in a shared directory

// ismod [start]
public:
	uint16_t GetCompleteSourcesCount() const { return m_nCompleteSourcesCount; }
};
