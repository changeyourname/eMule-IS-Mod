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
#include "UploadBandwidthThrottler.h" // ZZ:UploadBandWithThrottler (UDP)
#include "EncryptedDatagramSocket.h"

class Packet;

struct UDPPack // netfinity: Rearranged for alignment reasons
{
	Packet* packet;
	uint32_t	dwIP;
	uint16_t	nPort;
	uint32_t	dwTime;
	bool	bEncrypt;
	bool	bKad;
	uint32_t  nReceiverVerifyKey;
	uchar	pachTargetClientHashORKadID[16];
	//uint16 nPriority; We could add a priority system here to force some packets.
};

class CClientUDPSocket : public CAsyncSocket, public CEncryptedDatagramSocket, public ThrottledControlSocket // ZZ:UploadBandWithThrottler (UDP)
{
public:
	CClientUDPSocket();
	virtual ~CClientUDPSocket();

	bool	Create();
	bool	Rebind();
	uint16_t	GetConnectedPort()			{ return m_port; }
	bool	SendPacket(Packet* packet, uint32_t dwIP, uint16_t nPort, bool bEncrypt, const uchar* pachTargetClientHashORKadID, bool bKad, uint32_t nReceiverVerifyKey);
    SocketSentBytes  SendControlData(uint32_t maxNumberOfBytesToSend, uint32_t minFragSize); // ZZ:UploadBandWithThrottler (UDP)

protected:
	bool	ProcessPacket(const BYTE* packet, UINT size, uint8_t opcode, uint32_t ip, uint16_t port);
	
	virtual void	OnSend(int nErrorCode);	
	virtual void	OnReceive(int nErrorCode);
	virtual void	OnClose(int nErrorCode); // netfinity: 

private:
	int		SendTo(char* lpBuf,int nBufLen,uint32_t dwIP, uint16_t nPort);
    bool	IsBusy() const { return m_bWouldBlock; }
	bool	m_bWouldBlock;
	uint16_t	m_port;

	CTypedPtrList<CPtrList, UDPPack*> controlpacket_queue;

    CCriticalSection sendLocker; // ZZ:UploadBandWithThrottler (UDP)
};
