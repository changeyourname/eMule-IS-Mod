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

#define CS_AUTHFAILED	-6
#define CS_FATALERROR	-5
#define CS_DISCONNECTED	-4
#define CS_SERVERDEAD	-3
#define	CS_ERROR		-2
#define CS_SERVERFULL	-1
#define	CS_NOTCONNECTED	0
#define	CS_CONNECTING	1
#define	CS_CONNECTED	2
#define	CS_WAITFORLOGIN	3
#define CS_WAITFORPROXYLISTENING 4 // deadlake PROXYSUPPORT

#define CS_RETRYCONNECTTIME  30 // seconds

class CServerList;
class CUDPSocket;
class CServerSocket;
class CServer;
class Packet;
class AuthProcess;

class CServerConnect
{
public:
	CServerConnect();
	~CServerConnect();

	void	ConnectionFailed(CServerSocket* sender);
	void	ConnectionEstablished(CServerSocket* sender);
	
	// ismod: use random server to connect
	void	ConnectToAnyServer() { ConnectToAnyServer(UINT_MAX, true, true); }
	void	ConnectToAnyServer(UINT startAt, bool prioSort = false, bool isAuto = true, bool bNoCrypt = false);
	void	ConnectToServer(CServer* toconnect, bool multiconnect = false, bool bNoCrypt = false);
	void	StopConnectionTry();
	static  VOID CALLBACK RetryConnectTimer(HWND hWnd, UINT nMsg, UINT_PTR nId, DWORD dwTime);

	void	CheckForTimeout();
	void	DestroySocket(CServerSocket* pSck);	// safe socket closure and destruction
	bool	SendPacket(Packet* packet,bool delpacket = true, CServerSocket* to = 0);
	bool	IsUDPSocketAvailable() const { return udpsocket != NULL; }
	bool	SendUDPPacket(Packet* packet,CServer* host, bool delpacket = false, uint16_t nSpecialPort = 0, BYTE* pRawPacket = NULL, uint32_t nLen = 0);
	void	KeepConnectionAlive();
	bool	Disconnect();
	bool	IsConnecting()		{ return connecting; }
	bool	IsConnected()		{ return connected; }
	uint32_t	GetClientID()		{ return clientid; }
	CServer* GetCurrentServer();

	uint32_t	clientid;
	uint8_t	pendingConnects;
	uint32_t	m_curuser;

	bool	IsLowID();
	void	SetClientID(uint32_t newid);
	bool	IsLocalServer(uint32_t dwIP, uint16_t nPort);
	void	TryAnotherConnectionRequest();
	bool	IsSingleConnect()	{ return singleconnecting; }
	void	InitLocalIP();
	uint32_t	GetLocalIP()		{ return m_nLocalIP; }

	bool	AwaitingTestFromIP(uint32_t dwIP) const;
	bool	IsConnectedObfuscated() const;
	void	AddConnectionAttemp( CServerSocket* socket ) {
		connectionattemps.SetAt(GetTickCount(), socket);
	}

private:
	bool	connecting;
	bool	singleconnecting;
	bool	connected;
	bool	m_bTryObfuscated;
	uint8_t	max_simcons;
	UINT	m_uStartAutoConnectPos;
	CServerSocket* connectedsocket;
	CUDPSocket* udpsocket;
	// ismod
	//CPtrList m_lstOpenSockets;	// list of currently opened sockets
	CTypedPtrList<CPtrList, CServerSocket*> m_lstOpenSockets;	// list of currently opened sockets
	UINT_PTR	m_idRetryTimer;
	uint32_t	m_nLocalIP;
	CMap<ULONG, ULONG, CServerSocket*, CServerSocket*> connectionattemps;

//ismod
public:
	bool IsSocketOpened(const CServerSocket* auth);
};