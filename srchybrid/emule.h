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

#include "resource.h"
#include "ReadWriteLock.h"	// SLUGFILLER: SafeHash
#ifdef _DEBUG
#include <emhelper/StackWalker.h>
#endif

// BEGIN netfinity: For inline stuff
#include "kademlia/kademlia/Kademlia.h"
#include "Sockets.h"
// END netfinity: For inline stuff

#define	DEFAULT_NICK		thePrefs.GetWindowsUserName()
#define	DEFAULT_TCP_PORT_OLD	4662
#define	DEFAULT_UDP_PORT_OLD	(DEFAULT_TCP_PORT_OLD+10)

#define PORTTESTURL			_T("http://porttest.emule-project.net/connectiontest.php?tcpport=%i&udpport=%i&lang=%i")

#define EMULE_DEFAULT_SKIN  _T("EMULE_DEFAULT_SKIN")
#define EMULE_UPDATE_URL    "http://tcs.is74.ru/update.xml"
#define EMULE_UPDATE_FILE   L"%LOCALAPPDATA%\\eMule IS Mod\\application\\emule-update.exe"
#define EMULE_APPWND_CLASS  L"eMule IS Mod"

class CSearchList;
class CUploadQueue;
class CListenSocket;
class CDownloadQueue;
class CScheduler;
class UploadBandwidthThrottler;
class LastCommonRouteFinder;
class CemuleDlg;
class CClientList;
class CKnownFileList;
class CServerConnect;
class CServerList;
class CSharedFileList;
class CClientCreditsList;
class CFriendList;
class CClientUDPSocket;
class CIPFilter;
class CWebServer;
class CMMServer;
class CAbstractFile;
class CUpDownClient;
//class CPeerCacheFinder;
class CFirewallOpener;
class CUPnPImplWrapper;
#ifdef _DEBUG
class StackWalker;
#endif

struct SLogItem;

enum AppState{
	APP_STATE_RUNNING = 0,
   	APP_STATE_SHUTTINGDOWN,
	APP_STATE_DONE
};

class CemuleApp : public CWinApp
{
public:
	CemuleApp( LPCTSTR lpszAppName = NULL );
	virtual ~CemuleApp();

    UploadBandwidthThrottler* uploadBandwidthThrottler;
	CemuleDlg*			emuledlg;
	CClientList*		clientlist;
	CKnownFileList*		knownfiles;
	CServerConnect*		serverconnect;
	CServerList*		serverlist;	
	CSharedFileList*	sharedfiles;
	CSearchList*		searchlist;
	CListenSocket*		listensocket;
	CUploadQueue*		uploadqueue;
	CDownloadQueue*		downloadqueue;
	CClientCreditsList*	clientcredits;
	CFriendList*		friendlist;
	CClientUDPSocket*	clientudp;
	CIPFilter*			ipfilter;
	CWebServer*			webserver;
	CScheduler*			scheduler;
	CMMServer*			mmserver;
	CUPnPImplWrapper*	m_pUPnPFinder;

	HANDLE				m_hMutexOneInstance;
	CFont				m_fontHyperText;
	CFont				m_fontDefaultBold;
	CFont				m_fontSymbol;
	CFont				m_fontLog;
	CFont				m_fontChatEdit;
	CBrush				m_brushBackwardDiagonal;
	static const UINT	m_nVersionMjr;
	static const UINT	m_nVersionMin;
	static const UINT	m_nVersionUpd;
	static const UINT	m_nVersionBld;
	DWORD				m_dwProductVersionMS;
	DWORD				m_dwProductVersionLS;
	CString				m_strCurVersionLong;
	CString				m_strCurVersionLongDbg;
	UINT				m_uCurVersionShort;
	UINT				m_uCurVersionCheck;
	ULONGLONG			m_ullComCtrlVer;
	AppState			m_app_state; // defines application state for shutdown 
	CMutex				hashing_mut;
	CReadWriteLock		m_threadlock;	// SLUGFILLER: SafeHash - This will ensure eMule goes last
	CString*			pstrPendingLink;
	COPYDATASTRUCT		sendstruct;

	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual BOOL IsIdleMessage(MSG *pMsg);

	// ed2k link functions
	void		AddEd2kLinksToDownload(CString strLinks, int cat);
	void		SearchClipboard();
	void		IgnoreClipboardLinks(CString strLinks) {m_strLastClipboardContents = strLinks;}
	void		PasteClipboard(int cat = 0);
	bool		IsEd2kFileLinkInClipboard();
	bool		IsEd2kServerLinkInClipboard();
	bool		IsEd2kLinkInClipboard(LPCSTR pszLinkType, int iLinkTypeLen);
	LPCTSTR		GetProfileFile()		{ return m_pszProfileName; }

	CString		CreateED2kSourceLink(const CAbstractFile* f);
//	CString		CreateED2kHostnameSourceLink(const CAbstractFile* f);
	CString		CreateKadSourceLink(const CAbstractFile* f);

	// clipboard (text)
	bool		CopyTextToClipboard(CString strText);
	CString		CopyTextFromClipboard();

	void		OnlineSig();
	void		UpdateReceivedBytes(uint32_t bytesToAdd);
	void		UpdateSentBytes(uint32_t bytesToAdd, bool sentToFriend = false);
	INT_PTR			GetFileTypeSystemImageIdx(LPCTSTR pszFilePath, int iLength = -1, bool bNormalsSize = false);
	HIMAGELIST	GetSystemImageList() { return m_hSystemImageList; }
	HIMAGELIST	GetBigSystemImageList() { return m_hBigSystemImageList; }
	CSize		GetSmallSytemIconSize() { return m_sizSmallSystemIcon; }
	CSize		GetBigSytemIconSize() { return m_sizBigSystemIcon; }
	void		CreateBackwardDiagonalBrush();
	void		CreateAllFonts();
	const CString &GetDefaultFontFaceName();
	bool		IsPortchangeAllowed();
	bool		IsConnected() { return (serverconnect->IsConnected() || Kademlia::CKademlia::IsConnected()); }
	bool		IsFirewalled();
	bool		CanDoCallback( CUpDownClient *client );
	uint32_t		GetID();
	uint32_t		GetPublicIP(bool bIgnoreKadIP = false) const;	// return current (valid) public IP or 0 if unknown
	void		SetPublicIP(const uint32_t dwIP);
	void		ResetStandByIdleTimer();

	// because nearly all icons we are loading are 16x16, the default size is specified as 16 and not as 32 nor LR_DEFAULTSIZE
	/* ismod: MoNKi: -Skin Selector-
	HICON		LoadIcon(LPCTSTR lpszResourceName, int cx = 16, int cy = 16, UINT uFlags = LR_DEFAULTCOLOR) const;
	*/
	HICON		LoadIcon(LPCTSTR lpszResourceName, int cx = 16, int cy = 16, UINT uFlags = LR_DEFAULTCOLOR, LPCTSTR pszSkinProfile = NULL) const;

	HICON		LoadIcon(UINT nIDResource) const;
	HBITMAP		LoadImage(LPCTSTR lpszResourceName, LPCTSTR pszResourceType) const;
	HBITMAP		LoadImage(UINT nIDResource, LPCTSTR pszResourceType) const;
	bool		LoadSkinColor(LPCTSTR pszKey, COLORREF& crColor) const;
	bool		LoadSkinColorAlt(LPCTSTR pszKey, LPCTSTR pszAlternateKey, COLORREF& crColor) const;
	CString		GetSkinFileItem(LPCTSTR lpszResourceName, LPCTSTR pszResourceType) const;
	void		ApplySkin(LPCTSTR pszSkinProfile);
	void		EnableRTLWindowsLayout();
	void		DisableRTLWindowsLayout();
	void		UpdateLargeIconSize();
	bool		IsXPThemeActive() const;
	bool		IsVistaThemeActive() const;

	bool		GetLangHelpFilePath(CString& strResult);
	void		SetHelpFilePath(LPCTSTR pszHelpFilePath);

    // thread safe log calls
    void			QueueDebugLogLine(bool bAddToStatusBar, LPCTSTR line,...);
    void			QueueDebugLogLineEx(UINT uFlags, LPCTSTR line,...);
    void			HandleDebugLogQueue();
    void			ClearDebugLogQueue(bool bDebugPendingMsgs = false);

	void			QueueLogLine(bool bAddToStatusBar, LPCTSTR line,...);
    void			QueueLogLineEx(UINT uFlags, LPCTSTR line,...);
    void			HandleLogQueue();
    void			ClearLogQueue(bool bDebugPendingMsgs = false);

	bool			DidWeAutoStart() { return m_bAutoStart; }

protected:
	bool ProcessCommandline();
	void SetTimeOnTransfer();
	static BOOL CALLBACK SearchEmuleWindow(HWND hWnd, LPARAM lParam);

	DECLARE_MESSAGE_MAP()

	HIMAGELIST m_hSystemImageList;
	CMapStringToPtr m_aExtToSysImgIdx;
	CSize m_sizSmallSystemIcon;

	HIMAGELIST m_hBigSystemImageList;
	CMapStringToPtr m_aBigExtToSysImgIdx;
	CSize m_sizBigSystemIcon;

	CString		m_strDefaultFontFaceName;

	/* ismod: new clipboard handler
	bool		m_bGuardClipboardPrompt;
	*/
	CString		m_strLastClipboardContents;

    // thread safe log calls
    CCriticalSection m_queueLock;
    CTypedPtrList<CPtrList, SLogItem*> m_QueueDebugLog;
    CTypedPtrList<CPtrList, SLogItem*> m_QueueLog;

	uint32_t m_dwPublicIP;
	bool m_bAutoStart;

	//////////////////////////////////////////////////
	// IS Mod part
	//
public:
	bool	IsRunning() { return (m_app_state == APP_STATE_RUNNING); }
	uint32_t	GetServerIP() const { return m_dwServerIP; }
	void	SetServerIP(uint32_t dwIP) { m_dwServerIP = dwIP; }
	void	SetAutomaticShutdown(bool in) { m_bAutomaticShutdown = in; }
	bool	IsAutomaticShutdown() const { return m_bAutomaticShutdown; }

	CString		ClipboardText;
	bool		m_bIsConnected;
	time_t		m_LastBlacklistedTime;
	CString		m_strCurVersionLongMod;
	bool		m_bClipboardLink;
	CSafeThread* m_pFlushThread;
	CSafeThread* m_pReadBlockThread;
	CSafeThread* m_pHashThread;
	HANDLE		m_hPartHashWaitEvent;
	HANDLE		m_globalWaitEvent;
#ifdef _DEBUG
	StackWalker *stackWalker;
#endif

private:
    uint32_t m_dwServerIP;
	bool m_bAutomaticShutdown;
};

extern CemuleApp theApp;
