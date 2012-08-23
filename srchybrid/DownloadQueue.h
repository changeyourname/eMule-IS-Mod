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

#include "TransferRate.h" // netfinity: Transfer rate black box

class CSafeMemFile;
class CSearchFile;
class CUpDownClient;
class CServer;
class CPartFile;
class CSharedFileList;
class CKnownFile;
struct SUnresolvedHostname;

namespace Kademlia 
{
	class CUInt128;
};

class CSourceHostnameResolveWnd : public CWnd
{
// Construction
public:
	CSourceHostnameResolveWnd();
	virtual ~CSourceHostnameResolveWnd();

	void AddToResolve(const uchar* fileid, LPCSTR pszHostname, uint16_t port, LPCTSTR pszURL = NULL);

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnHostnameResolved(WPARAM wParam, LPARAM lParam);

private:
	struct Hostname_Entry {
		uchar fileid[16];
		CStringA strHostname;
		uint16_t port;
		CString strURL;
	};
	CTypedPtrList<CPtrList, Hostname_Entry*> m_toresolve;
	char m_aucHostnameBuffer[MAXGETHOSTSTRUCT];
};


class CDownloadQueue
{
	friend class CAddFileThread;
	friend class CServerSocket;

public:
	CDownloadQueue();
	~CDownloadQueue();

	void	Process();
	void	Init();
	
	// add/remove entries
	void	AddPartFilesToShare() const;
	void	AddDownload(CPartFile* newfile, bool paused);
	void	AddSearchToDownload(CSearchFile* toadd, uint8_t paused = 2, int cat = 0);
	void	AddSearchToDownload(const CString &link, uint8_t paused = 2, int cat = 0);
	void	AddFileLinkToDownload(class CED2KFileLink* pLink, int cat = 0);
	void	RemoveFile(CPartFile* toremove);
	void	DeleteAll();

	int		GetFileCount() const { return filelist.size(); }
	UINT	GetDownloadingFileCount() const;
	UINT	GetPausedFileCount() const;

	bool	IsFileExisting(const uchar* fileid, bool bLogWarnings = true) const;
	bool	IsPartFile(const CKnownFile* file) const;
	bool	IsTempFile(const CString& rstrDirectory, const CString& rstrName) const;	// SLUGFILLER: SafeHash

	CPartFile* GetFileByID(const uchar* filehash) const;
	CPartFile* GetFileByIndex(size_t index) const;
	CPartFile* GetFileByKadFileSearchID(uint32_t ID) const;

    void    StartNextFileIfPrefs(int cat);
	void	StartNextFile(int cat=-1,bool force=false);

	void	RefilterAllComments();	

	// sources
	CUpDownClient* GetDownloadClientByIP(uint32_t dwIP);
	CUpDownClient* GetDownloadClientByIP_UDP(uint32_t dwIP, uint16_t nUDPPort, bool bIgnorePortOnUniqueIP, bool* pbMultipleIPs = NULL);
	bool	IsInList(const CUpDownClient* client) const;

	bool    CheckAndAddSource(CPartFile* sender,CUpDownClient* source);
	bool    CheckAndAddKnownSource(CPartFile* sender,CUpDownClient* source);
	bool	RemoveSource(CUpDownClient* toremove, bool bDoStatsUpdate = true);

	// statistics
	typedef struct{
		int	a[22];
	} SDownloadStats;
	void	GetDownloadSourcesStats(SDownloadStats& results);
	int		GetDownloadFilesStats(uint64_t &ui64TotalFileSize, uint64_t &ui64TotalLeftToTransfer, uint64_t &ui64TotalAdditionalNeededSpace) const;
	uint32_t	GetDatarate() {return datarate;}

	void	AddUDPFileReasks()								{m_nUDPFileReasks++;}
	uint32_t	GetUDPFileReasks() const						{return m_nUDPFileReasks;}
	void	AddFailedUDPFileReasks()						{m_nFailedUDPFileReasks++;}
	uint32_t	GetFailedUDPFileReasks() const					{return m_nFailedUDPFileReasks;}

	// categories
	void	ResetCatParts(UINT cat);
	void	SetCatPrio(UINT cat, uint8_t newprio);
    void    RemoveAutoPrioInCat(UINT cat, uint8_t newprio); // ZZ:DownloadManager
	void	SetCatStatus(UINT cat, int newstatus);
	void	MoveCat(UINT from, UINT to);
	void	SetAutoCat(CPartFile* newfile);

	// searching on local server
	void	SendLocalSrcRequest(CPartFile* sender);
	void	RemoveLocalServerRequest(CPartFile* pFile);
	void	ResetLocalServerRequests();

	// searching in Kad
	void	SetLastKademliaFileRequest()				{lastkademliafilerequest = ::GetTickCount();}
	bool	DoKademliaFileRequest();
	void	KademliaSearchFile(uint32_t searchID, const Kademlia::CUInt128* pcontactID, const Kademlia::CUInt128* pkadID, uint8_t type, uint32_t ip, uint16_t tcp, uint16_t udp, uint32_t dwBuddyIP, uint16_t dwBuddyPort, uint8_t byCryptOptions);

	// searching on global servers
	void	StopUDPRequests();

	// check diskspace
	void	SortByPriority();
	void	CheckDiskspace(bool bNotEnoughSpaceLeft = false);
	void	CheckDiskspaceTimed();

	void	ExportPartMetFilesOverview() const;
	void	OnConnectionState(bool bConnected);

	void	AddToResolved( CPartFile* pFile, SUnresolvedHostname* pUH );

	CString GetOptimalTempDir(UINT nCat, EMFileSize nFileSize);

	CServer* cur_udpserver;

protected:
	bool	SendNextUDPPacket();
	void	ProcessLocalRequests();
	bool	IsMaxFilesPerUDPServerPacketReached(uint32_t nFiles, uint32_t nIncludedLargeFiles) const;
	bool	SendGlobGetSourcesUDPPacket(CSafeMemFile* data, bool bExt2Packet, uint32_t nFiles, uint32_t nIncludedLargeFiles);

private:
	static int __cdecl CompareParts(void *, const void *file1, const void *file2);
	std::vector<CPartFile*> filelist;
	std::list<CPartFile*> m_localServerReqQueue;
	uint16_t	filesrdy;
	uint32_t	datarate;
	
	CPartFile*	lastfile;
	uint32_t		lastcheckdiskspacetime;
	uint32_t		lastudpsearchtime;
	uint32_t		lastudpstattime;
	UINT		udcounter;
	UINT		m_cRequestsSentToServer;
	uint32_t		m_dwNextTCPSrcReq;
	int			m_iSearchedServers;
	uint32_t		lastkademliafilerequest;

	//uint64		m_datarateMS;
	uint32_t		m_nUDPFileReasks;
	uint32_t		m_nFailedUDPFileReasks;

	CSourceHostnameResolveWnd m_srcwnd;

    DWORD       m_dwLastA4AFtime; // ZZ:DownloadManager
// ismod [start]
public:
	void PreviewFileByLink( LPCTSTR pszEd2kLink, bool boost );
};
