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
#include "BarShader.h"
#include "ClientStateDefs.h"
#include "TransferRate.h"	// netfinity: Transfer Rate Blackbox
        
class CClientReqSocket;
// ismod: no PeerCache
class CFriend;
class CPartFile;
class CClientCredits;
class CAbstractFile;
class CKnownFile;
class Packet;
class CxImage;
struct Requested_Block_Struct;
class CSafeMemFile;
class CEMSocket;
// ismod: no aich
// ismod
class CSearchFile;

enum EUtf8Str;

// ismod: mod version flags
#define MOD_VERSION_OLDER    0x01
#define MOD_VERSION_SAME     0x02
#define MOD_VERSION_NEWER    0x04
#define MOD_VERSION_UNSTABLE 0x08

struct Pending_Block_Struct{
	Pending_Block_Struct()
	{
		block = NULL;
		zStream = NULL;
		totalUnzipped = 0;
		fZStreamError = 0;
		fRecovered = 0;
		fQueued = 0;
	}
	Requested_Block_Struct*	block;
	struct z_stream_s*      zStream;       // Barry - Used to unzip packets
	UINT					totalUnzipped; // Barry - This holds the total unzipped bytes for all packets so far
	UINT					fZStreamError : 1,
							fRecovered    : 1,
							fQueued		  : 3;
};

//#pragma pack(1)
struct Requested_File_Struct{
	uchar	  fileid[16];
	uint32_t	  lastasked;
	uint8_t	  badrequests;
};
//#pragma pack()

struct PartFileStamp{
	CPartFile*	file;
	DWORD		timestamp;
};

#define	MAKE_CLIENT_VERSION(mjr, min, upd) \
	((UINT)(mjr)*100U*10U*100U + (UINT)(min)*100U*10U + (UINT)(upd)*100U)

//#pragma pack(2)
class CUpDownClient : public CObject
{
	DECLARE_DYNAMIC(CUpDownClient)

	friend class CUploadQueue;
public:
    void PrintUploadStatus();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Base
	CUpDownClient(CClientReqSocket* sender = 0);
	CUpDownClient(CPartFile* in_reqfile, uint16_t in_port, uint32_t in_userid, uint32_t in_serverup, uint16_t in_serverport, bool ed2kID = false);
	virtual ~CUpDownClient();

	void			StartDownload();
	virtual void	CheckDownloadTimeout();
	virtual void	SendCancelTransfer(Packet* packet = NULL);
	virtual bool	IsEd2kClient() const							{ return true; }
	virtual bool	Disconnected(LPCTSTR pszReason, bool bFromSocket = false);
	virtual bool	TryToConnect(bool bIgnoreMaxCon = false, bool bNoCallbacks = false, CRuntimeClass* pClassSocket = NULL);
	virtual void	Connect();
	virtual void	ConnectionEstablished();
	virtual void	OnSocketConnected(int nErrorCode);
	bool			CheckHandshakeFinished() const;
	void			CheckFailedFileIdReqs(const uchar* aucFileHash);
	uint32_t			GetUserIDHybrid() const							{ return m_nUserIDHybrid; }
	void			SetUserIDHybrid(uint32_t val)						{ m_nUserIDHybrid = val; }
	LPCTSTR			GetUserName() const								{ return m_pszUsername; }
	void			SetUserName(LPCTSTR pszNewName);
	uint32_t			GetIP() const									{ return m_dwUserIP; }
	void			SetIP( uint32_t val ) //Only use this when you know the real IP or when your clearing it.
						{
							m_dwUserIP = val;
							m_nConnectIP = val;
						}
	__inline bool	HasLowID() const								{ return (m_nUserIDHybrid < 16777216); }
	uint32_t			GetConnectIP() const							{ return m_nConnectIP; }
	uint16_t			GetUserPort() const								{ return m_nUserPort; }
	void			SetUserPort(uint16_t val)							{ m_nUserPort = val; }
	UINT			GetTransferredUp() const						{ return m_nTransferredUp; }
	UINT			GetTransferredDown() const						{ return m_nTransferredDown; }
	uint32_t			GetServerIP() const								{ return m_dwServerIP; }
	void			SetServerIP(uint32_t nIP)							{ m_dwServerIP = nIP; }
	uint16_t			GetServerPort() const							{ return m_nServerPort; }
	void			SetServerPort(uint16_t nPort)						{ m_nServerPort = nPort; }
	const uchar*	GetUserHash() const								{ return (uchar*)m_achUserHash; }
	void			SetUserHash(const uchar* pUserHash);
	bool			HasValidHash() const
						{
							return ((const uint32_t*)m_achUserHash)[0] != 0 || ((const uint32_t*)m_achUserHash)[1] != 0 || ((const uint32_t*)m_achUserHash)[2] != 0 || ((const uint32_t*)m_achUserHash)[3] != 0;
						}
	int				GetHashType() const;
	const uchar*	GetBuddyID() const								{ return (uchar*)m_achBuddyID; }
	void			SetBuddyID(const uchar* m_achTempBuddyID);
	bool			HasValidBuddyID() const							{ return m_bBuddyIDValid; }
	void			SetBuddyIP( uint32_t val )						{ m_nBuddyIP = val; }
	uint32_t			GetBuddyIP() const								{ return m_nBuddyIP; }
	void			SetBuddyPort( uint16_t val )						{ m_nBuddyPort = val; }
	uint16_t			GetBuddyPort() const							{ return m_nBuddyPort; }
	EClientSoftware	GetClientSoft() const							{ return (EClientSoftware)m_clientSoft; }
	const CString&	GetClientSoftVer() const						{ return m_strClientSoftware; }
	const CString&	GetClientModVer() const							{ return m_strModVersion; }
	void			InitClientSoftwareVersion();
	UINT			GetVersion() const								{ return m_nClientVersion; }
	uint8_t			GetMuleVersion() const							{ return m_byEmuleVersion; }
	bool			ExtProtocolAvailable() const					{ return m_bEmuleProtocol; }
	bool			SupportMultiPacket() const						{ return m_bMultiPacket; }
	bool			SupportExtMultiPacket() const					{ return m_fExtMultiPacket; }
	bool			SupportPeerCache() const						{ return m_fPeerCache; }
	bool			SupportsLargeFiles() const						{ return m_fSupportsLargeFiles; }
	bool			IsEmuleClient() const							{ return m_byEmuleVersion!=0; }
	uint8_t			GetSourceExchange1Version() const				{ return m_bySourceExchange1Ver; }
	bool			SupportsSourceExchange2() const					{ return m_fSupportsSourceEx2; }
	CClientCredits* Credits() const									{ return credits; }
	bool			IsBanned() const;
	const CString&	GetClientFilename() const						{ return m_strClientFilename; }
	void			SetClientFilename(const CString& fileName)		{ m_strClientFilename = fileName; }
	uint16_t			GetUDPPort() const								{ return m_nUDPPort; }
	void			SetUDPPort(uint16_t nPort)						{ m_nUDPPort = nPort; }
	uint8_t			GetUDPVersion() const							{ return m_byUDPVer; }
	bool			SupportsUDP() const								{ return GetUDPVersion() != 0 && m_nUDPPort != 0; }
	uint16_t			GetKadPort() const								{ return m_nKadPort; }
	void			SetKadPort(uint16_t nPort)						{ m_nKadPort = nPort; }
	uint8_t			GetExtendedRequestsVersion() const				{ return m_byExtendedRequestsVer; }
	void			RequestSharedFileList();
	void			ProcessSharedFileList(const uchar* pachPacket, UINT nSize, LPCTSTR pszDirectory = NULL);
	EConnectingState GetConnectingState() const						{ return (EConnectingState)m_nConnectingState; }

	void			ClearHelloProperties();
	bool			ProcessHelloAnswer(const uchar* pachPacket, UINT nSize);
	bool			ProcessHelloPacket(const uchar* pachPacket, UINT nSize);
	void			SendHelloAnswer();
	virtual void	SendHelloPacket();
	void			SendMuleInfoPacket(bool bAnswer);
	void			ProcessMuleInfoPacket(const uchar* pachPacket, UINT nSize);
	void			ProcessMuleCommentPacket(const uchar* pachPacket, UINT nSize);
	void			ProcessEmuleQueueRank(const uchar* packet, UINT size);
	void			ProcessEdonkeyQueueRank(const uchar* packet, UINT size);
	void			CheckQueueRankFlood();
	bool			Compare(const CUpDownClient* tocomp, bool bIgnoreUserhash = false) const;
	void			ResetFileStatusInfo();
	uint32_t			GetLastSrcReqTime() const						{ return m_dwLastSourceRequest; }
	void			SetLastSrcReqTime()								{ m_dwLastSourceRequest = ::GetTickCount(); }
	uint32_t			GetLastSrcAnswerTime() const					{ return m_dwLastSourceAnswer; }
	void			SetLastSrcAnswerTime()							{ m_dwLastSourceAnswer = ::GetTickCount(); }
	uint32_t			GetLastAskedForSources() const					{ return m_dwLastAskedForSources; }
	void			SetLastAskedForSources()						{ m_dwLastAskedForSources = ::GetTickCount(); }
	bool			GetFriendSlot() const;
	void			SetFriendSlot(bool bNV)							{ m_bFriendSlot = bNV; }
	bool			IsFriend() const								{ return m_Friend != NULL; }
	CFriend*		GetFriend() const;
	void			SetCommentDirty(bool bDirty = true)				{ m_bCommentDirty = bDirty; }
	bool			GetSentCancelTransfer() const					{ return m_fSentCancelTransfer; }
	void			SetSentCancelTransfer(bool bVal)				{ m_fSentCancelTransfer = bVal; }
	void			ProcessPublicIPAnswer(const BYTE* pbyData, UINT uSize);
	void			SendPublicIPRequest();
	uint8_t			GetKadVersion()	const							{ return m_byKadVersion; }
	bool			SendBuddyPingPong()								{ return m_dwLastBuddyPingPongTime < ::GetTickCount(); }
	bool			AllowIncomeingBuddyPingPong()					{ return m_dwLastBuddyPingPongTime < (::GetTickCount()-(3*60*1000)); }
	void			SetLastBuddyPingPongTime()						{ m_dwLastBuddyPingPongTime = (::GetTickCount()+(10*60*1000)); }
	void			ProcessFirewallCheckUDPRequest(CSafeMemFile* data);
	void			SendSharedDirectories();

	// secure ident
	void			SendPublicKeyPacket();
	void			SendSignaturePacket();
	void			ProcessPublicKeyPacket(const uchar* pachPacket, UINT nSize);
	void			ProcessSignaturePacket(const uchar* pachPacket, UINT nSize);
	uint8_t			GetSecureIdentState() const						{ return (uint8_t)m_SecureIdentState; }
	void			SendSecIdentStatePacket();
	void			ProcessSecIdentStatePacket(const uchar* pachPacket, UINT nSize);
	uint8_t			GetInfoPacketsReceived() const					{ return m_byInfopacketsReceived; }
	void			InfoPacketsReceived();
	bool			HasPassedSecureIdent(bool bPassIfUnavailable) const;
	// preview
	void			SendPreviewRequest(const CAbstractFile* pForFile);
	void			SendPreviewAnswer(const CKnownFile* pForFile, CxImage** imgFrames, uint8_t nCount);
	void			ProcessPreviewReq(const uchar* pachPacket, UINT nSize);
	void			ProcessPreviewAnswer(const uchar* pachPacket, UINT nSize);
	bool			GetPreviewSupport() const						{ return m_fSupportsPreview && GetViewSharedFilesSupport(); }
	bool			GetViewSharedFilesSupport() const				{ return m_fNoViewSharedFiles==0; }
	bool			SafeConnectAndSendPacket(Packet* packet);
	bool			SendPacket(Packet* packet, bool bDeletePacket, bool bVerifyConnection = false);
	void			CheckForGPLEvilDoer();
	// Encryption / Obfuscation / Connectoptions
	bool			SupportsCryptLayer() const						{ return m_fSupportsCryptLayer; }
	bool			RequestsCryptLayer() const						{ return SupportsCryptLayer() && m_fRequestsCryptLayer; }
	bool			RequiresCryptLayer() const						{ return RequestsCryptLayer() && m_fRequiresCryptLayer; }
	bool			SupportsDirectUDPCallback() const				{ return m_fDirectUDPCallback != 0 && HasValidHash() && GetKadPort() != 0; }
	void			SetCryptLayerSupport(bool bVal)					{ m_fSupportsCryptLayer = bVal ? 1 : 0; }
	void			SetCryptLayerRequest(bool bVal)					{ m_fRequestsCryptLayer = bVal ? 1 : 0; }
	void			SetCryptLayerRequires(bool bVal)				{ m_fRequiresCryptLayer = bVal ? 1 : 0; }
	void			SetDirectUDPCallbackSupport(bool bVal)			{ m_fDirectUDPCallback = bVal ? 1 : 0; }
	void			SetConnectOptions(uint8_t byOptions, bool bEncryption = true, bool bCallback = true); // shortcut, sets crypt, callback etc based from the tagvalue we recieve
	bool			IsObfuscatedConnectionEstablished() const;
	bool			ShouldReceiveCryptUDPPackets() const;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Upload
	EUploadState	GetUploadState() const							{ return (EUploadState)m_nUploadState; }
	void			SetUploadState(EUploadState news);
	time_t			GetWaitStartTime() const;
	void 			SetWaitStartTime();
	void 			ClearWaitStartTime();
	time_t			GetWaitTime() const								{ return max(time_t(0), m_dwUploadTime - GetWaitStartTime()); }
	bool			IsDownloading() const							{ return (m_nUploadState == US_UPLOADING); }
	bool			HasBlocks() const								{ return !m_BlockRequests_queue.IsEmpty(); }
    UINT            GetNumberOfRequestedBlocksInQueue() const       { return (UINT) m_BlockRequests_queue.GetCount(); }
	UINT			GetDatarate() const								{ return m_nUpDatarate; }	
	UINT			GetScore(bool sysvalue, bool isdownloading = false, bool onlybasevalue = false) const;
	void			AddReqBlock(Requested_Block_Struct* reqblock);
	void			CreateNextBlockPackage(bool bBigBuffer = false);
	uint32_t			GetUpStartTimeDelay() const						{ return (uint32_t) max(0, (int32_t) (::GetTickCount() - m_dwUploadTime)); }
	void 			SetUpStartTime()								{ (uint32_t) max(0, (int32_t) (m_dwUploadTime = ::GetTickCount())); }
	void			SendHashsetPacket(const uchar* fileid);
	const uchar*	GetUploadFileID() const							{ return requpfileid; }
	void			SetUploadFileID(CKnownFile* newreqfile);
	UINT			SendBlockData();
	void			ClearUploadBlockRequests();
	void			SendRankingInfo();
	void			SendCommentInfo(/*const*/ CKnownFile *file);
	void			AddRequestCount(const uchar* fileid);
	void			UnBan();
	void			Ban(LPCTSTR pszReason = NULL);
	UINT			GetAskedCount() const							{ return m_cAsked; }
	void			AddAskedCount()									{ m_cAsked++; }
	void			SetAskedCount(UINT m_cInAsked)					{ m_cAsked = m_cInAsked; }
	void			FlushSendBlocks(); // call this when you stop upload, or the socket might be not able to send
	time_t			GetLastUpRequest() const						{ return m_dwLastUpRequest; }
	void			SetLastUpRequest()								{ m_dwLastUpRequest = ::GetTickCount(); }
	void			SetCollectionUploadSlot(bool bValue);
	bool			HasCollectionUploadSlot() const					{ return m_bCollectionUploadSlot; }

	UINT			GetSessionUp() const							{ return m_nTransferredUp - m_nCurSessionUp; }
	void			ResetSessionUp() {
						m_nCurSessionUp = m_nTransferredUp;
						m_addedPayloadQueueSession = 0;
						m_nCurQueueSessionPayloadUp = 0;
					}

	UINT			GetSessionDown() const							{ return m_nTransferredDown - m_nCurSessionDown; }
    UINT            GetSessionPayloadDown() const                   { return m_nCurSessionPayloadDown; }
	void			ResetSessionDown() {
						m_nCurSessionDown = m_nTransferredDown;
                        m_nCurSessionPayloadDown = 0;
					}
	UINT			GetQueueSessionPayloadUp() const				{ return m_nCurQueueSessionPayloadUp; }
    UINT			GetPayloadInBuffer() const						{ return m_addedPayloadQueueSession - GetQueueSessionPayloadUp(); }

	bool			ProcessExtendedInfo(CSafeMemFile* packet, CKnownFile* tempreqfile);
	uint32_t			GetUpPartCount() const							{ return m_nUpPartCount; }
	void			DrawUpStatusBar(CDC* dc, RECT* rect, bool onlygreyrect, bool  bFlat) const;
	bool			IsUpPartAvailable(UINT iPart) const {
						return (iPart >= m_nUpPartCount || !m_abyUpPartStatus) ? false : m_abyUpPartStatus[iPart] != 0;
					}
	uint8_t*			GetUpPartStatus() const							{ return m_abyUpPartStatus; }
    float           GetCombinedFilePrioAndCredit();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Download
	UINT			GetAskedCountDown() const						{ return m_cDownAsked; }
	void			AddAskedCountDown()								{ m_cDownAsked++; }
	void			SetAskedCountDown(UINT cInDownAsked)			{ m_cDownAsked = cInDownAsked; }
	EDownloadState	GetDownloadState() const						{ return (EDownloadState)m_nDownloadState; }
	void			SetDownloadState(EDownloadState nNewState, LPCTSTR pszReason = _T("Unspecified"));
	uint32_t			GetLastAskedTime(const CPartFile* partFile = NULL) const;
    void            SetLastAskedTime()								{ m_fileReaskTimes.SetAt(reqfile, ::GetTickCount()); }
	bool			IsPartAvailable(UINT iPart) const {
						return (iPart >= m_partCount || !m_abyPartStatus) ? false : m_abyPartStatus[iPart] != 0;
					}
	uint8_t*			GetPartStatus() const							{ return m_abyPartStatus; }
	uint32_t			GetPartCount() const							{ return m_partCount; }
	UINT			GetDownloadDatarate() const						{ return m_nDownDatarate; }
	UINT			GetRemoteQueueRank() const						{ return m_nRemoteQueueRank; }
	void			SetRemoteQueueRank(UINT nr, bool bUpdateDisplay = false);
	bool			IsRemoteQueueFull() const						{ return m_bRemoteQueueFull; }
	void			SetRemoteQueueFull(bool flag)					{ m_bRemoteQueueFull = flag; }
	void			DrawStatusBar(CDC* dc, LPCRECT rect, bool onlygreyrect, bool  bFlat) const;
	bool			AskForDownload();
	virtual void	SendFileRequest();
	void			SendStartupLoadReq();
	void			ProcessFileInfo(CSafeMemFile* data, CPartFile* file);
	void			ProcessFileStatus(bool bUdpPacket, CSafeMemFile* data, CPartFile* file);
	void			ProcessHashSet(const uchar* data, UINT size);
	void			ProcessAcceptUpload();
	bool			AddRequestForAnotherFile(CPartFile* file);
	void			CreateBlockRequests(int iMaxBlocks);
	virtual void	SendBlockRequests();
	// ismod: no PeerCache
	virtual void	ProcessBlockPacket(const uchar* packet, UINT size, bool packed, bool bI64Offsets);
	// ismod: no PeerCache
	void			ClearDownloadBlockRequests();
	void			SendOutOfPartReqsAndAddToWaitingQueue();
	//UINT			CalculateDownloadRate(); // netfinity: Transfer Rate BlackBox - Replaced by CTransferRate class
	uint16_t			GetAvailablePartCount() const;
	bool			SwapToAnotherFile(LPCTSTR pszReason, bool bIgnoreNoNeeded, bool ignoreSuspensions, bool bRemoveCompletely, CPartFile* toFile = NULL, bool allowSame = true, bool isAboutToAsk = false, bool debug = false); // ZZ:DownloadManager
	void			DontSwapTo(/*const*/ CPartFile* file);
	bool			IsSwapSuspended(const CPartFile* file, const bool allowShortReaskTime = false, const bool fileIsNNP = false) /*const*/; // ZZ:DownloadManager
	uint32_t          GetTimeUntilReask() const						{return GetTimeUntilReask(reqfile);}
	uint32_t          GetTimeUntilReask(const CPartFile* file) const	{return GetTimeUntilReask(file, false);}
    uint32_t			GetTimeUntilReask(const CPartFile* file, const bool allowShortReaskTime, const bool useGivenNNP = false, const bool givenNNP = false) const;
	void			UDPReaskACK(uint16_t nNewQR);
	void			UDPReaskFNF();
	void			UDPReaskForDownload();
	bool			UDPPacketPending() const						{ return m_bUDPPending; }
	bool			IsSourceRequestAllowed() const;
    bool            IsSourceRequestAllowed(CPartFile* partfile, bool sourceExchangeCheck = false) const; // ZZ:DownloadManager

	bool			IsValidSource() const;
	ESourceFrom		GetSourceFrom() const							{ return (ESourceFrom)m_nSourceFrom; }
	void			SetSourceFrom(ESourceFrom val)					{ m_nSourceFrom = (_ESourceFrom)val; }

	void			SetDownStartTime()								{ m_dwDownStartTime = ::GetTickCount(); }
	uint32_t			GetDownTimeDifference(boolean clear = true)	{
						uint32_t myTime = m_dwDownStartTime;
						if(clear) m_dwDownStartTime = 0;
						return ::GetTickCount() - myTime;
					}
	bool			GetTransferredDownMini() const					{ return m_bTransferredDownMini; }
	void			SetTransferredDownMini()						{ m_bTransferredDownMini = true; }
	void			InitTransferredDownMini()						{ m_bTransferredDownMini = false; }
	UINT			GetA4AFCount() const							{ return (UINT) m_OtherRequests_list.GetCount(); }

	uint16_t			GetUpCompleteSourcesCount() const				{ return m_nUpCompleteSourcesCount; }
	void			SetUpCompleteSourcesCount(uint16_t n)				{ m_nUpCompleteSourcesCount = n; }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Chat
	EChatState		GetChatState() const							{ return (EChatState)m_nChatstate; }
	void			SetChatState(EChatState nNewS)					{ m_nChatstate = (_EChatState)nNewS; }
	EChatCaptchaState GetChatCaptchaState() const					{ return (EChatCaptchaState)m_nChatCaptchaState; }
	void			SetChatCaptchaState(EChatCaptchaState nNewS)	{ m_nChatCaptchaState = (_EChatCaptchaState)nNewS; }
	void			ProcessChatMessage(CSafeMemFile* data, uint32_t nLength);
	void			SendChatMessage(CString strMessage);
	void			ProcessCaptchaRequest(CSafeMemFile* data);
	void			ProcessCaptchaReqRes(uint8_t nStatus);
	// message filtering
	uint8_t			GetMessagesReceived() const						{ return m_cMessagesReceived; }
	void			SetMessagesReceived(uint8_t nCount)				{ m_cMessagesReceived = nCount; }
	void			IncMessagesReceived()							{ m_cMessagesReceived < 255 ? ++m_cMessagesReceived : 255; }
	uint8_t			GetMessagesSent() const							{ return m_cMessagesSent; }
	void			SetMessagesSent(uint8_t nCount)					{ m_cMessagesSent = nCount; }
	void			IncMessagesSent()								{ m_cMessagesSent < 255 ? ++m_cMessagesSent : 255; }
	bool			IsSpammer() const								{ return m_fIsSpammer; }
	void			SetSpammer(bool bVal);
	bool			GetMessageFiltered() const						{ return m_fMessageFiltered; }
	void			SetMessageFiltered(bool bVal);


	//KadIPCheck
	EKadState		GetKadState() const								{ return (EKadState)m_nKadState; }
	void			SetKadState(EKadState nNewS)					{ m_nKadState = (_EKadState)nNewS; }

	//File Comment
	bool			HasFileComment() const							{ return !m_strFileComment.IsEmpty(); }
    const CString&	GetFileComment() const							{ return m_strFileComment; } 
    void			SetFileComment(LPCTSTR pszComment)				{ m_strFileComment = pszComment; }

	bool			HasFileRating() const							{ return m_uFileRating > 0; }
    uint8_t			GetFileRating() const							{ return m_uFileRating; }
    void			SetFileRating(uint8_t uRating)					{ m_uFileRating = uRating; }

	// Barry - Process zip file as it arrives, don't need to wait until end of block
	int				unzip(Pending_Block_Struct *block, const BYTE *zipped, UINT lenZipped, BYTE **unzipped, UINT *lenUnzipped, int iRecursion = 0);
	void			UpdateDisplayedInfo(bool force = false);
	int             GetFileListRequested() const					{ return m_iFileListRequested; }
    void            SetFileListRequested(int iFileListRequested)	{ m_iFileListRequested = iFileListRequested; }

	virtual void	SetRequestFile(CPartFile* pReqFile);
	CPartFile*		GetRequestFile() const							{ return reqfile; }

	// ismod: no aich 
	EUtf8Str		GetUnicodeSupport() const;

	CString			GetDownloadStateDisplayString() const;
	CString			GetUploadStateDisplayString() const;

	LPCTSTR			DbgGetDownloadState() const;
	LPCTSTR			DbgGetUploadState() const;
	LPCTSTR			DbgGetKadState() const;
	CString			DbgGetClientInfo(bool bFormatIP = false) const;
	CString			DbgGetFullClientSoftVer() const;
	const CString&	DbgGetHelloInfo() const							{ return m_strHelloInfo; }
	const CString&	DbgGetMuleInfo() const							{ return m_strMuleInfo; }

// ZZ:DownloadManager -->
    const bool      IsInNoNeededList(const CPartFile* fileToCheck) const;
    const bool      SwapToRightFile(CPartFile* SwapTo, CPartFile* cur_file, bool ignoreSuspensions, bool SwapToIsNNPFile, bool isNNPFile, bool& wasSkippedDueToSourceExchange, bool doAgressiveSwapping = false, bool debug = false);
    const DWORD     getLastTriedToConnectTime() { return m_dwLastTriedToConnect; }
// <-- ZZ:DownloadManager

#ifdef _DEBUG
	// Diagnostic Support
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	CClientReqSocket* socket;
	CClientCredits*	credits;
	CFriend*		m_Friend;
	uint8_t*			m_abyUpPartStatus;
	CTypedPtrList<CPtrList, CPartFile*> m_OtherRequests_list;
	CTypedPtrList<CPtrList, CPartFile*> m_OtherNoNeeded_list;
	uint16_t			m_lastPartAsked;
	bool			m_bAddNextConnect;

    void			SetSlotNumber(UINT newValue)					{ m_slotNumber = newValue; }
    UINT			GetSlotNumber() const							{ return m_slotNumber; }
    CEMSocket*		GetFileUploadSocket(bool log = false);

	// ismod: no PeerCache

	// ismod [start]
	// BEGIN SiRoB: ReadBlockFromFileThread
public:
	void	SetReadBlockFromFileBuffer(byte* pdata) {filedata = pdata;};
	// END SiRoB: ReadBlockFromFileThread
	// ismod [end]
protected:
	// base
	void	Init();
	bool	ProcessHelloTypePacket(CSafeMemFile* data);
	void	SendHelloTypePacket(CSafeMemFile* data);
	void	CreateStandartPackets(byte* data, UINT togo, Requested_Block_Struct* currentblock, bool bFromPF = true);
	void	CreatePackedPackets(byte* data, UINT togo, Requested_Block_Struct* currentblock, bool bFromPF = true);
	void	SendFirewallCheckUDPRequest();
	void	SendHashSetRequest();

	uint32_t	m_nConnectIP;		// holds the supposed IP or (after we had a connection) the real IP
	uint32_t	m_dwUserIP;			// holds 0 (real IP not yet available) or the real IP (after we had a connection)
	uint32_t	m_dwServerIP;
	uint32_t	m_nUserIDHybrid;
	uint16_t	m_nUserPort;
	uint16_t	m_nServerPort;
	UINT	m_nClientVersion;
	//--group to aligned int32
	uint8_t	m_byEmuleVersion;
	uint8_t	m_byDataCompVer;
	bool	m_bEmuleProtocol;
	bool	m_bIsHybrid;
	//--group to aligned int32
	TCHAR*	m_pszUsername;
	uchar	m_achUserHash[16];
	uint16_t	m_nUDPPort;
	uint16_t	m_nKadPort;
	//--group to aligned int32
	uint8_t	m_byUDPVer;
	uint8_t	m_bySourceExchange1Ver;
	uint8_t	m_byAcceptCommentVer;
	uint8_t	m_byExtendedRequestsVer;
	//--group to aligned int32
	uint8_t	m_byCompatibleClient;
	bool	m_bFriendSlot;
	bool	m_bCommentDirty;
	bool	m_bIsML;
	//--group to aligned int32
	bool	m_bGPLEvildoer;
	bool	m_bHelloAnswerPending;
	uint8_t	m_byInfopacketsReceived;	// have we received the edonkeyprot and emuleprot packet already (see InfoPacketsReceived() )
	uint8_t	m_bySupportSecIdent;
	//--group to aligned int32
	uint32_t	m_dwLastSignatureIP;
	CString m_strClientSoftware;
	CString m_strModVersion;
	uint32_t	m_dwLastSourceRequest;
	uint32_t	m_dwLastSourceAnswer;
	uint32_t	m_dwLastAskedForSources;
    int     m_iFileListRequested;
	CString	m_strFileComment;
	//--group to aligned int32
	uint8_t	m_uFileRating;
	uint8_t	m_cMessagesReceived;		// count of chatmessages he sent to me
	uint8_t	m_cMessagesSent;			// count of chatmessages I sent to him
	bool	m_bMultiPacket;
	//--group to aligned int32
	bool	m_bUnicodeSupport;
	bool	m_bBuddyIDValid;
	uint16_t	m_nBuddyPort;
	//--group to aligned int32
	uint8_t	m_byKadVersion;
	uint8_t	m_cCaptchasSent;

	uint32_t	m_nBuddyIP;
	uint32_t	m_dwLastBuddyPingPongTime;
	uchar	m_achBuddyID[16];
	CString m_strHelloInfo;
	CString m_strMuleInfo;
	CString m_strCaptchaChallenge;
	CString m_strCaptchaPendingMsg;
	

	// States
	_EClientSoftware	m_clientSoft;
	_EChatState			m_nChatstate;
	_EKadState			m_nKadState;
	_ESecureIdentState	m_SecureIdentState;
	_EUploadState		m_nUploadState;
	_EDownloadState		m_nDownloadState;
	_ESourceFrom		m_nSourceFrom;
	_EChatCaptchaState	m_nChatCaptchaState;
	_EConnectingState	m_nConnectingState;

	CTypedPtrList<CPtrList, Packet*> m_WaitingPackets_list;
	CList<PartFileStamp> m_DontSwap_list;

	////////////////////////////////////////////////////////////////////////
	// Upload
	//
    int GetFilePrioAsNumber() const;

	UINT		m_nTransferredUp;
	time_t		m_dwUploadTime;
	UINT		m_cAsked;
	time_t		m_dwLastUpRequest;
	UINT		m_nCurSessionUp;
	UINT		m_nCurSessionDown;
    UINT		m_nCurQueueSessionPayloadUp;
    UINT		m_addedPayloadQueueSession;
	uint16_t		m_nUpPartCount;
	uint16_t		m_nUpCompleteSourcesCount;
	uchar		requpfileid[16];
    UINT		m_slotNumber;
	bool		m_bCollectionUploadSlot;

	typedef struct TransferredData {
		UINT	datalen;
		DWORD	timestamp;
	};
	CTypedPtrList<CPtrList, Requested_Block_Struct*> m_BlockRequests_queue;
	CTypedPtrList<CPtrList, Requested_Block_Struct*> m_DoneBlocks_list;
	CTypedPtrList<CPtrList, Requested_File_Struct*>	 m_RequestedFiles_list;

	// BEGIN SiRoB: ReadBlockFromFileThread
	byte* filedata;
	// END SiRoB: ReadBlockFromFileThread
	//////////////////////////////////////////////////////////
	// Download
	//
	CPartFile*	reqfile;
	// ismod: no aich
	UINT		m_cDownAsked;
	uint8_t*		m_abyPartStatus;
	CString		m_strClientFilename;
	UINT		m_nTransferredDown;
    UINT        m_nCurSessionPayloadDown;
	uint32_t		m_dwDownStartTime;
	uint64_t		m_nLastBlockOffset;
	uint32_t		m_dwLastBlockReceived;
	UINT		m_nTotalUDPPackets;
	UINT		m_nFailedUDPPackets;
	UINT		m_nRemoteQueueRank;
	//--group to aligned int32
	bool		m_bRemoteQueueFull;
	bool		m_bCompleteSource;
	uint32_t		m_partCount;
	//--group to aligned int32
	uint16_t		m_cShowDR;
	bool		m_bReaskPending;
	bool		m_bUDPPending;
	bool		m_bTransferredDownMini;

	// Download from URL
	CStringA	m_strUrlPath;
	uint64_t		m_uReqStart;
	uint64_t		m_uReqEnd;
	uint64_t		m_nUrlStartPos;


	//////////////////////////////////////////////////////////
	// Upload data rate computation
	//
	UINT		m_nUpDatarate;
	UINT		m_nSumForAvgUpDataRate;
	CList<TransferredData> m_AvarageUDR_list;

	//////////////////////////////////////////////////////////
	// Download data rate computation
	//
	/*UINT		m_nDownDatarate;
	UINT		m_nDownDataRateMS;
	UINT		m_nSumForAvgDownDataRate;
	CList<TransferredData> m_AvarageDDR_list;*/
	CTransferRate	m_nDownDatarate; // netfinity: Transfer Rate BlackBox

	//////////////////////////////////////////////////////////
	// GUI helpers
	//
	static CBarShader s_StatusBar;
	static CBarShader s_UpStatusBar;
	DWORD		m_lastRefreshedDLDisplay;
    DWORD		m_lastRefreshedULDisplay;
    uint32_t      m_random_update_wait;

	// using bitfield for less important flags, to save some bytes
	UINT m_fHashsetRequesting : 1, // we have sent a hashset request to this client in the current connection
		 m_fSharedDirectories : 1, // client supports OP_ASKSHAREDIRS opcodes
		 m_fSentCancelTransfer: 1, // we have sent an OP_CANCELTRANSFER in the current connection
		 m_fNoViewSharedFiles : 1, // client has disabled the 'View Shared Files' feature, if this flag is not set, we just know that we don't know for sure if it is enabled
		 m_fSupportsPreview   : 1,
		 m_fPreviewReqPending : 1,
		 m_fPreviewAnsPending : 1,
		 m_fIsSpammer		  : 1,
		 m_fMessageFiltered   : 1,
		 m_fPeerCache		  : 1,
		 m_fQueueRankPending  : 1,
		 m_fUnaskQueueRankRecv: 2,
		 m_fFailedFileIdReqs  : 4, // nr. of failed file-id related requests per connection
		 m_fNeedOurPublicIP	  : 1, // we requested our IP from this client
		 // ismod: no aich
		 m_fSentOutOfPartReqs : 1,
		 m_fSupportsLargeFiles: 1,
		 m_fExtMultiPacket	  : 1,
		 m_fRequestsCryptLayer: 1,
	     m_fSupportsCryptLayer: 1,
		 m_fRequiresCryptLayer: 1,
		 m_fSupportsSourceEx2 : 1,
		 m_fSupportsCaptcha	  : 1,
		 m_fDirectUDPCallback : 1,	// 1 bits left
		 m_fFailedDownload    : 1; // SiRoB: Download Collision fix

	CTypedPtrList<CPtrList, Pending_Block_Struct*>	 m_PendingBlocks_list;
	CTypedPtrList<CPtrList, Requested_Block_Struct*> m_DownloadBlocks_list;

    bool    m_bSourceExchangeSwapped; // ZZ:DownloadManager
    DWORD   lastSwapForSourceExchangeTick; // ZZ:DownloadManaager
    bool    DoSwap(CPartFile* SwapTo, bool bRemoveCompletely, LPCTSTR reason); // ZZ:DownloadManager
    CMap<CPartFile*, CPartFile*, DWORD, DWORD> m_fileReaskTimes; // ZZ:DownloadManager (one resk timestamp for each file)
    DWORD   m_dwLastTriedToConnect; // ZZ:DownloadManager (one resk timestamp for each file)
    bool    RecentlySwappedForSourceExchange() { return ::GetTickCount()-lastSwapForSourceExchangeTick < 30*1000; } // ZZ:DownloadManager
    void    SetSwapForSourceExchangeTick() { lastSwapForSourceExchangeTick = ::GetTickCount(); } // ZZ:DownloadManager

	//////////////////////////////////////////////////////////
	// IS Mod part
	//
public:
	uint8_t			GetModVersionFlags() const { return m_ModVersionFlags; }
	bool			IsSameMod() const { return m_bSameMod; }
	void            SetSearchFile(CSearchFile* pFile) { m_pSearchFile = pFile; }
	int             GetDirContentsRequested() const { return m_bDirContentsRequested; }
	COLORREF 		GetClientColor() const;
	void            ProcessDirContentsList(const uchar* pachPacket, uint32_t nSize);

private:
	bool			m_bSameMod;
	uint32_t			m_ModExtVersion;	
	uint8_t			m_ModVersionFlags;
	CSearchFile*    m_pSearchFile;
	bool            m_bDirContentsRequested; // флаг запроса содержимого директории
};

// ismod [start]
// BEGIN SiRoB: ReadBlockFromFileThread

struct BlockInfo_Struct
{
    CKnownFile* srcfile;
    uint32_t togo;
    uint64_t startOffset;
};

class CReadBlockFromFileThread : public CSafeThread
{
	DECLARE_DYNCREATE(CReadBlockFromFileThread)
public:
	virtual	BOOL InitInstance();
    DECLARE_MESSAGE_MAP()
    afx_msg void OnReadBlockFromFile(WPARAM wParam, LPARAM lParam);
};
// END SiRoB: ReadBlockFromFileThread
