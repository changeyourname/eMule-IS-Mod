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

class CClientReqSocket;
class CUpDownClient;
namespace Kademlia{
	class CContact;
	class CUInt128;
};
typedef CTypedPtrList<CPtrList, CUpDownClient*> CUpDownClientPtrList;

#define	NUM_CLIENTLIST_STATS	19
#define BAN_CLEANUP_TIME		1200000 // 20 min

//------------CDeletedClient Class----------------------
// this class / list is a bit overkill, but currently needed to avoid any exploit possibtility
// it will keep track of certain clients attributes for 2 hours, while the CUpDownClient object might be deleted already
// currently: IP, Port, UserHash
struct PORTANDHASH{
	uint16_t nPort;
	void* pHash;
};

struct IPANDTICS{
	uint32_t dwIP;
	uint32_t dwInserted;
};
struct CONNECTINGCLIENT{
	CUpDownClient* pClient;
	uint32_t dwInserted;
};


class CDeletedClient{
public:
	CDeletedClient(const CUpDownClient* pClient);
	CArray<PORTANDHASH> m_ItemsList;
	uint32_t				m_dwInserted;
	uint32_t				m_cBadRequest;
};

enum buddyState
{
	Disconnected,
	Connecting,
	Connected
};

// ----------------------CClientList Class---------------
class CClientList
{
	friend class CClientListCtrl;

public:
	CClientList();
	~CClientList();

	// Clients
	void	AddClient(CUpDownClient* toadd,bool bSkipDupTest = false);
	void	RemoveClient(CUpDownClient* toremove, LPCTSTR pszReason = NULL);
	void	GetStatistics(uint32_t& totalclient, int stats[NUM_CLIENTLIST_STATS],
						  CMap<uint32_t, uint32_t, uint32_t, uint32_t>& clientVersionEDonkey,
						  CMap<uint32_t, uint32_t, uint32_t, uint32_t>& clientVersionEDonkeyHybrid,
						  CMap<uint32_t, uint32_t, uint32_t, uint32_t>& clientVersionEMule,
						  CMap<uint32_t, uint32_t, uint32_t, uint32_t>& clientVersionAMule);
	uint32_t	GetClientCount()	{ return (uint32_t) list.GetCount();}
	void	DeleteAll();
	bool	AttachToAlreadyKnown(CUpDownClient** client, CClientReqSocket* sender);
	CUpDownClient* FindClientByIP(uint32_t clientip, UINT port) const;
	CUpDownClient* FindClientByUserHash(const uchar* clienthash, uint32_t dwIP = 0, uint16_t nTCPPort = 0) const;
	CUpDownClient* FindClientByIP(uint32_t clientip) const;
	CUpDownClient* FindClientByIP_UDP(uint32_t clientip, UINT nUDPport) const;
	CUpDownClient* FindClientByServerID(uint32_t uServerIP, uint32_t uUserID) const;
	CUpDownClient* FindClientByUserID_KadPort(uint32_t clientID,uint16_t kadPort) const;
	CUpDownClient* FindClientByIP_KadPort(uint32_t ip, uint16_t port) const;

	// Banned clients
	void	AddBannedClient(uint32_t dwIP);
	bool	IsBannedClient(uint32_t dwIP) const;
	void	RemoveBannedClient(uint32_t dwIP);
	UINT	GetBannedCount() const		{ return (UINT) m_bannedList.GetCount(); }
	void	RemoveAllBannedClients();

	// Tracked clients
	void	AddTrackClient(CUpDownClient* toadd);
	bool	ComparePriorUserhash(uint32_t dwIP, uint16_t nPort, void* pNewHash);
	UINT	GetClientsFromIP(uint32_t dwIP) const;
	void	TrackBadRequest(const CUpDownClient* upcClient, int nIncreaseCounter);
	uint32_t	GetBadRequests(const CUpDownClient* upcClient) const;
	UINT	GetTrackedCount() const		{ return (UINT) m_trackedClientsList.GetCount(); }
	void	RemoveAllTrackedClients();

	// Kad client list, buddy handling
	bool	RequestTCP(Kademlia::CContact* contact, uint8_t byConnectOptions);
	void	RequestBuddy(Kademlia::CContact* contact, uint8_t byConnectOptions);
	bool	IncomingBuddy(Kademlia::CContact* contact, Kademlia::CUInt128* buddyID);
	void	RemoveFromKadList(CUpDownClient* torem);
	void	AddToKadList(CUpDownClient* toadd);
	bool	DoRequestFirewallCheckUDP(const Kademlia::CContact& contact);
	//bool	DebugDoRequestFirewallCheckUDP(uint32 ip, uint16 port);
	uint8_t	GetBuddyStatus()			{ return m_nBuddyStatus; }
	CUpDownClient* GetBuddy()			{ return m_pBuddy; }

	void	AddKadFirewallRequest(uint32_t dwIP);
	bool	IsKadFirewallCheckIP(uint32_t dwIP) const;

	// Direct Callback List
	void	AddTrackCallbackRequests(uint32_t dwIP);
	bool	AllowCalbackRequest(uint32_t dwIP) const;

	// Connecting Clients
	void	AddConnectingClient(CUpDownClient* pToAdd);
	void	RemoveConnectingClient(CUpDownClient* pToRemove);

	void	Process();
	bool	IsValidClient(CUpDownClient* tocheck) const;
	void	Debug_SocketDeleted(CClientReqSocket* deleted) const;

    // ZZ:UploadSpeedSense -->
	/* ismod: no lastCommonRouteFinder
    bool GiveClientsForTraceRoute();
	*/
	// ZZ:UploadSpeedSense <--

    void	ProcessA4AFClients() const; // ZZ:DownloadManager

protected:
	void	CleanUpClientList();
	void	ProcessConnectingClientsList();

private:
	CUpDownClientPtrList list;
	CUpDownClientPtrList m_KadList;
	CMap<uint32_t, uint32_t, uint32_t, uint32_t> m_bannedList;
	CMap<uint32_t, uint32_t, CDeletedClient*, CDeletedClient*> m_trackedClientsList;
	uint32_t	m_dwLastBannCleanUp;
	uint32_t	m_dwLastTrackedCleanUp;
	uint32_t m_dwLastClientCleanUp;
	CUpDownClient* m_pBuddy;
	uint8_t m_nBuddyStatus;
	CList<IPANDTICS> listFirewallCheckRequests;
	CList<IPANDTICS> listDirectCallbackRequests;
	CList<CONNECTINGCLIENT> m_liConnectingClients;
};
