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
#include "AbstractFile.h"

class CFileDataIO;
class CxImage;

class CSearchFile : public CAbstractFile
{
	DECLARE_DYNAMIC(CSearchFile)

	friend class CPartFile;
	friend class CSearchListCtrl;
public:
	CSearchFile(CFileDataIO* in_data, bool bOptUTF8, uint32_t nSearchID,
				uint32_t nServerIP=0, uint16_t nServerPort=0,
				LPCTSTR pszDirectory = NULL, 
				bool nKademlia = false, bool bServerUDPAnswer = false);
	CSearchFile(const CSearchFile* copyfrom);
	virtual ~CSearchFile();

	bool	IsKademlia() const				{ return m_bKademlia; }
	bool	IsServerUDPAnswer() const		{ return m_bServerUDPAnswer; }
	uint32_t	AddSources(uint32_t count);
	uint32_t	GetSourceCount() const;
	uint32_t	AddCompleteSources(uint32_t count);
	uint32_t	GetCompleteSourceCount() const;
	int		IsComplete() const;
	int		IsComplete(UINT uSources, UINT uCompleteSources) const;
	time_t	GetLastSeenComplete() const;
	uint32_t	GetSearchID() const { return m_nSearchID; }
	LPCTSTR GetDirectory() const { return m_pszDirectory; }

	uint32_t	GetClientID() const				{ return m_nClientID; }
	void	SetClientID(uint32_t nClientID)	{ m_nClientID = nClientID; }
	uint16_t	GetClientPort() const			{ return m_nClientPort; }
	void	SetClientPort(uint16_t nPort)		{ m_nClientPort = nPort; }
	uint32_t	GetClientServerIP() const		{ return m_nClientServerIP; }
	void	SetClientServerIP(uint32_t uIP)   { m_nClientServerIP = uIP; }
	uint16_t	GetClientServerPort() const		{ return m_nClientServerPort; }
	void	SetClientServerPort(uint16_t nPort) { m_nClientServerPort = nPort; }
	int		GetClientsCount() const			{ return ((GetClientID() && GetClientPort()) ? 1 : 0) + m_aClients.GetSize(); }
	void	SetKadPublishInfo(uint32_t dwVal)	{ m_nKadPublishInfo = dwVal; }
	uint32_t	GetKadPublishInfo() const		{ return m_nKadPublishInfo; } // == TAG_PUBLISHINFO

	// Spamfilter
	void	SetNameWithoutKeyword(LPCTSTR strName)	{ m_strNameWithoutKeywords = strName; }
	const CString&	GetNameWithoutKeyword()	const			{ return m_strNameWithoutKeywords; }
	void	SetSpamRating(uint32_t nRating)			{ m_nSpamRating = nRating; }
	uint32_t	GetSpamRating() const					{ return m_nSpamRating; }
	bool	IsConsideredSpam() const;

	virtual void	UpdateFileRatingCommentAvail(bool bForceUpdate = false);

	// GUI helpers
	CSearchFile* GetListParent() const		{ return m_list_parent; }
	void		 SetListParent(CSearchFile* parent) { m_list_parent = parent; }
	UINT		 GetListChildCount() const	{ return m_list_childcount; }
	void		 SetListChildCount(int cnt)	{ m_list_childcount = cnt; }
	void		 AddListChildCount(int cnt) { m_list_childcount += cnt; }
	bool		 IsListExpanded() const		{ return m_list_bExpanded; }
	void		 SetListExpanded(bool val)	{ m_list_bExpanded = val; }

	void		 StoreToFile(CFileDataIO& rFile) const;

	struct SClient {
		SClient() {
			m_nIP = 0;
			m_nPort = 0;
			m_nServerIP = 0;
			m_nServerPort = 0;
		}
		SClient(uint32_t nIP, uint16_t nPort, uint32_t nServerIP, uint16_t nServerPort) {
			m_nIP = nIP;
			m_nPort = nPort;
			m_nServerIP = nServerIP;
			m_nServerPort = nServerPort;
		}
		friend __inline bool __stdcall operator==(const CSearchFile::SClient& c1, const CSearchFile::SClient& c2) {
			return c1.m_nIP==c2.m_nIP && c1.m_nPort==c2.m_nPort &&
				   c1.m_nServerIP==c2.m_nServerIP && c1.m_nServerPort==c2.m_nServerPort;
		}
		uint32_t m_nIP;
		uint32_t m_nServerIP;
		uint16_t m_nPort;
		uint16_t m_nServerPort;
	};
	void AddClient(const SClient& client) { m_aClients.Add(client); }
	const CSimpleArray<SClient>& GetClients() const { return m_aClients; }

	struct SServer {
		SServer() {
			m_nIP = 0;
			m_nPort = 0;
			m_uAvail = 0;
			m_bUDPAnswer = false;
		}
		SServer(uint32_t nIP, uint16_t nPort, bool bUDPAnswer) {
			m_nIP = nIP;
			m_nPort = nPort;
			m_uAvail = 0;
			m_bUDPAnswer = bUDPAnswer;
		}
		friend __inline bool __stdcall operator==(const CSearchFile::SServer& s1, const CSearchFile::SServer& s2) {
			return s1.m_nIP==s2.m_nIP && s1.m_nPort==s2.m_nPort;
		}
		uint32_t m_nIP;
		uint16_t m_nPort;
		UINT   m_uAvail;
		bool   m_bUDPAnswer;
	};
	void AddServer(const SServer& server) { m_aServers.Add(server); }
	const CSimpleArray<SServer>& GetServers() const { return m_aServers; }
	SServer& GetServerAt(int iServer) { return m_aServers[iServer]; }
	
	void	AddPreviewImg(CxImage* img)	{	m_listImages.Add(img); }
	const CSimpleArray<CxImage*>& GetPreviews() const { return m_listImages; }
	bool	IsPreviewPossible() const { return m_bPreviewPossible;}
	void	SetPreviewPossible(bool in)	{ m_bPreviewPossible = in; }

	enum EKnownType
	{
		NotDetermined,
		Shared,
		Downloading,
		Downloaded,
		Cancelled,
		Unknown
	};

	EKnownType GetKnownType() const { return m_eKnown; }
	void SetKnownType(EKnownType eType) { m_eKnown = eType; }

private:
	bool	m_bKademlia;
	bool	m_bServerUDPAnswer;
	uint32_t	m_nClientID;
	uint16_t	m_nClientPort;
	uint32_t	m_nSearchID;
	uint32_t	m_nClientServerIP;
	uint16_t	m_nClientServerPort;
	uint32_t	m_nKadPublishInfo;
	CSimpleArray<SClient> m_aClients;
	CSimpleArray<SServer> m_aServers;
	CSimpleArray<CxImage*> m_listImages;
	LPTSTR m_pszDirectory;
	// spamfilter
	CString	m_strNameWithoutKeywords;
	uint32_t	m_nSpamRating;

	// GUI helpers
	bool		m_bPreviewPossible;
	bool		m_list_bExpanded;
	UINT		m_list_childcount;
	CSearchFile*m_list_parent;
	EKnownType	m_eKnown;
// ismod [start]
public:
    CSearchFile(LPCTSTR szFileName, uint32_t nSearchID);

    bool IsUser() const { return m_bUser; }
    bool IsFolder() const { return m_bFolder; }
    void SetFolder(bool val) { m_bFolder = val; }
private:
    bool m_bUser;
    bool m_bFolder;
// ismod [end]
};

bool IsValidSearchResultClientIPPort(uint32_t nIP, uint16_t nPort);
