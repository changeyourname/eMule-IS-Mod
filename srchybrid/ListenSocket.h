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
#include "EMSocket.h"
#include "TransferRate.h" // netfinity: Transfer Rate Blackbox

class CUpDownClient;
class CPacket;
class CTimerWnd;
enum EDebugLogPriority;

enum SocketState 
{
	SS_Other,		//These are sockets we created that may or may not be used.. Or incoming connections.
	SS_Half,		//These are sockets that we called ->connect(..) and waiting for some kind of response.
	SS_Complete	//These are sockets that have responded with either a connection or error.
};

class CClientReqSocket : public CEMSocket
{
	friend class CListenSocket;
	DECLARE_DYNCREATE(CClientReqSocket)

public:
	CClientReqSocket(CUpDownClient* in_client = NULL);

	void	SetClient(CUpDownClient* pClient);
	void	Disconnect(LPCTSTR pszReason);
	void	WaitForOnConnect();
	void	ResetTimeOutTimer();
	bool	CheckTimeOut();
	virtual UINT GetTimeOut();
	virtual void Safe_Delete();
	
	bool	Create();
	virtual void SendPacket(Packet* packet, bool delpacket = true, bool controlpacket = true, uint32_t actualPayloadSize = 0, bool bForceImmediateSend = false);
    virtual SocketSentBytes SendControlData(uint32_t maxNumberOfBytesToSend, uint32_t overchargeMaxBytesToSend);
    virtual SocketSentBytes SendFileAndControlData(uint32_t maxNumberOfBytesToSend, uint32_t overchargeMaxBytesToSend);

	void	DbgAppendClientInfo(CString& str);
	CString DbgGetClientInfo();

	CUpDownClient*	client;

protected:
	virtual ~CClientReqSocket();
	virtual void Close() { CAsyncSocketEx::Close(); }
	void	Delete_Timed();

	virtual void OnConnect(int nErrorCode);
	void		 OnClose(int nErrorCode);
	void		 OnSend(int nErrorCode);
	void		 OnReceive(int nErrorCode);
	void		 OnError(int nErrorCode);

	virtual bool PacketReceived(Packet* packet);
	int			 PacketReceivedSEH(Packet* packet);
	bool		 PacketReceivedCppEH(Packet* packet);

	bool	ProcessPacket(const BYTE* packet, uint32_t size,UINT opcode);
	bool	ProcessExtPacket(const BYTE* packet, uint32_t size, UINT opcode, UINT uRawSize);
	void	PacketToDebugLogLine(LPCTSTR protocol, const uchar* packet, uint32_t size, UINT opcode);
	void	SetConState(SocketState val);

	uint32_t	timeout_timer;
	bool	deletethis;
	uint32_t	deltimer;
	bool	m_bPortTestCon;
	uint32_t	m_nOnConnect;

	// netfinity: Drop stalled downloads 
	friend class CPartFile; // This is bad, but don't know how to do it otherwise
};


class CListenSocket : public CAsyncSocketEx
{
	friend class CClientReqSocket;

public:
	CListenSocket();
	virtual ~CListenSocket();

	bool	StartListening();
	void	StopListening();
	virtual void OnAccept(int nErrorCode);
	void	Process();
	void	RemoveSocket(CClientReqSocket* todel);
	void	AddSocket(CClientReqSocket* toadd);
	UINT	GetOpenSockets()		{return (UINT) socket_list.GetCount();}
	void	KillAllSockets();
	bool	TooManySockets(bool bIgnoreInterval = false);
	uint32_t	GetMaxConnectionReached()	{return maxconnectionreached;}
	bool    IsValidSocket(CClientReqSocket* totest);
	void	AddConnection();
	void	RecalculateStats();
	void	ReStartListening();
	void	Debug_ClientDeleted(CUpDownClient* deleted);
	bool	Rebind();
	bool	SendPortTestReply(char result,bool disconnect=false);

	void	UpdateConnectionsStatus();
	float	GetMaxConperFiveModifier();
	uint32_t	GetPeakConnections()		{ return peakconnections; }
	uint32_t	GetTotalConnectionChecks()	{ return totalconnectionchecks; }
	float	GetAverageConnections()		{ return averageconnections; }
	uint32_t	GetActiveConnections()		{ return activeconnections; }
	uint16_t	GetConnectedPort()			{ return m_port; }
	uint32_t	GetTotalHalfCon()			{ return m_nHalfOpen; }
	uint32_t	GetTotalComp()				{ return m_nComp; }

private:
	bool bListening;
	CTypedPtrList<CPtrList, CClientReqSocket*> socket_list;
	uint16_t	m_OpenSocketsInterval;
	uint32_t	maxconnectionreached;
	uint16_t	m_ConnectionStates[3];
	int		m_nPendingConnections;
	uint32_t	peakconnections;
	uint32_t	totalconnectionchecks;
	float	averageconnections;
	uint32_t	activeconnections;
	uint16_t  m_port;
	uint32_t	m_nHalfOpen;
	uint32_t	m_nComp;
	
	CTransferRate	m_AvgSocketsPerFive; // netfinity: Precise socket per 5 average
};
