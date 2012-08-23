//this file is part of eMule
//Copyright (C)2002-2010 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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
#include "EncryptedStreamSocket.h"
#include "OtherFunctions.h"
#include "ThrottledSocket.h" // ZZ:UploadBandWithThrottler (UDP)

class CAsyncProxySocketLayer;
class Packet;

#define ES_DISCONNECTED		0xFF
#define ES_NOTCONNECTED		0x00
#define ES_CONNECTED		0x01

#define PACKET_HEADER_SIZE	6

struct StandardPacketQueueEntry {
	uint32_t actualPayloadSize;
	Packet* packet;
};

class CEMSocket : public CEncryptedStreamSocket, public ThrottledFileSocket // ZZ:UploadBandWithThrottler (UDP)
{
	DECLARE_DYNAMIC(CEMSocket)
public:
	CEMSocket();
	virtual ~CEMSocket();

	virtual void 	SendPacket(Packet* packet, bool delpacket = true, bool controlpacket = true, uint32_t actualPayloadSize = 0, bool bForceImmediateSend = false);
	bool	IsConnected() const {return byConnected == ES_CONNECTED;}
	uint8_t	GetConState() const {return byConnected;}
	virtual bool IsRawDataMode() const { return false; }
// BEGIN netfinity: Advanced download throttling
	static void	SetGlobalDownloadLimit(uint32_t limit);
	static void	DisableGlobalDownloadLimit();
	uint32_t	GetDownloadLimit() const;
// END netfinity: Advanced download throttling
	void	SetDownloadLimit(uint32_t limit);
	void	DisableDownloadLimit();
	BOOL	AsyncSelect(long lEvent);
	virtual bool IsBusy() const			{return m_bBusy;}
    virtual bool HasQueues() const		{return (sendbuffer || standartpacket_queue.GetCount() > 0 || controlpacket_queue.GetCount() > 0);} // not trustworthy threaded? but it's ok if we don't get the correct result now and then
	virtual bool UseBigSendBuffer();

	virtual UINT GetTimeOut() const;
	virtual void SetTimeOut(UINT uTimeOut);

	virtual BOOL Connect(LPCSTR lpszHostAddress, UINT nHostPort);
	virtual BOOL Connect(SOCKADDR* pSockAddr, int iSockAddrLen);

	void InitProxySupport();
	virtual void RemoveAllLayers();
	const CString GetLastProxyError() const { return m_strLastProxyError; }
	bool GetProxyConnectFailed() const { return m_bProxyConnectFailed; }

	CString GetFullErrorMessage(DWORD dwError);

	DWORD GetLastCalledSend() { return lastCalledSend; }
	uint64_t GetSentBytesCompleteFileSinceLastCallAndReset();
	uint64_t GetSentBytesPartFileSinceLastCallAndReset();
	uint64_t GetSentBytesControlPacketSinceLastCallAndReset();
	uint64_t GetSentPayloadSinceLastCallAndReset();
	void TruncateQueues();

    virtual SocketSentBytes SendControlData(uint32_t maxNumberOfBytesToSend, uint32_t minFragSize) { return Send(maxNumberOfBytesToSend, minFragSize, true); };
    virtual SocketSentBytes SendFileAndControlData(uint32_t maxNumberOfBytesToSend, uint32_t minFragSize) { return Send(maxNumberOfBytesToSend, minFragSize, false); };

    uint32_t	GetNeededBytes();
#ifdef _DEBUG
	// Diagnostic Support
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	virtual int	OnLayerCallback(const CAsyncSocketExLayer *pLayer, int nType, int nCode, WPARAM wParam, LPARAM lParam);
	
	virtual void	DataReceived(const BYTE* pcData, UINT uSize);
	virtual bool	PacketReceived(Packet* packet) = 0;
	virtual void	OnError(int nErrorCode) = 0;
	virtual void	OnClose(int nErrorCode);
	virtual void	OnSend(int nErrorCode);
	virtual void	OnReceive(int nErrorCode);
	uint8_t	byConnected;
	UINT	m_uTimeOut;
	bool	m_bProxyConnectFailed;
	CAsyncProxySocketLayer* m_pProxyLayer;
	CString m_strLastProxyError;

private:
    virtual SocketSentBytes Send(uint32_t maxNumberOfBytesToSend, uint32_t minFragSize, bool onlyAllowedToSendControlPacket);
	void	ClearQueues();
	virtual int Receive(void* lpBuf, int nBufLen, int nFlags = 0);

    uint32_t GetNextFragSize(uint32_t current, uint32_t minFragSize);
    bool    HasSent() { return m_hasSent; }

	// Download (pseudo) rate control
// BEGIN netfinity: Advanced download throttling
	static int32_t	globalDownloadLimit;
	static bool		globalDownloadLimitEnable;
	static uint32_t	globalDownloadLimitLastSet;
	int32_t			downloadLimit;
	bool	downloadLimitEnable;
	mutable int32_t	downloadLimitMin;
	mutable uint32_t	downloadLimitLastSet;
// END netfinity: Advanced download throttling
	bool	pendingOnReceive;

	// Download partial header
	char	pendingHeader[PACKET_HEADER_SIZE];	// actually, this holds only 'PACKET_HEADER_SIZE-1' bytes.
	size_t	pendingHeaderSize;

	// Download partial packet
	Packet* pendingPacket;
	size_t	pendingPacketSize;

	// Upload control
	char*	sendbuffer;
	uint32_t	sendblen;
	uint32_t	sent;

	CTypedPtrList<CPtrList, Packet*> controlpacket_queue;
	CList<StandardPacketQueueEntry> standartpacket_queue;
	bool m_currentPacket_is_controlpacket;
	CCriticalSection sendLocker;
	uint64_t m_numberOfSentBytesCompleteFile;
	uint64_t m_numberOfSentBytesPartFile;
	uint64_t m_numberOfSentBytesControlPacket;
	bool m_currentPackageIsFromPartFile;
	bool m_bAccelerateUpload;
	DWORD lastCalledSend;
    DWORD lastSent;
	uint32_t lastFinishedStandard;
	uint32_t m_actualPayloadSize;
	uint32_t m_actualPayloadSizeSent;
    bool m_bBusy;
    bool m_hasSent;
	bool m_bUsesBigSendBuffers;

private:
	// netfinity: Recover partial part packets
	void		RecoverPackets();

// ismod:
public:
	void ReallyDisableDownloadLimit(bool val) { m_bDownloadLimitReallyDisabled = val; }
private:
	bool m_bDownloadLimitReallyDisabled;
};
