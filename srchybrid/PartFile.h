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

#include "KnownFile.h"

enum EPartFileStatus{
	PS_READY			= 0,
	PS_EMPTY			= 1,
	PS_WAITINGFORHASH	= 2,
	PS_HASHING			= 3,
	PS_ERROR			= 4,
	PS_INSUFFICIENT		= 5,
	PS_UNKNOWN			= 6,
	PS_PAUSED			= 7,
	PS_COMPLETING		= 8,
	PS_COMPLETE			= 9
};

#define PR_VERYLOW			4 // I Had to change this because it didn't save negative number correctly.. Had to modify the sort function for this change..
#define PR_LOW				0 //*
#define PR_NORMAL			1 // Don't change this - needed for edonkey clients and server!
#define PR_HIGH				2 //*
#define PR_VERYHIGH			3
#define PR_AUTO				5 //UAP Hunter

//#define BUFFER_SIZE_LIMIT 500000 // Max bytes before forcing a flush

#define	PARTMET_BAK_EXT	_T(".bak")
#define	PARTMET_TMP_EXT	_T(".backup")

#define STATES_COUNT		17

enum EPartFileFormat{
	PMT_UNKNOWN			= 0,
	PMT_DEFAULTOLD,
	PMT_SPLITTED,
	PMT_NEWOLD,
	PMT_SHAREAZA,
	PMT_BADFORMAT	
};

enum EPartFileLoadResult{
	PLR_LOADSUCCESS = 1,
	PLR_CHECKSUCCESS = 2,
	PLR_FAILED_METFILE_CORRUPT = -1,
	PLR_FAILED_METFILE_NOACCESS = -2,
	PLR_FAILED_OTHER   = 0
};

#define	FILE_COMPLETION_THREAD_FAILED	0x0000
#define	FILE_COMPLETION_THREAD_SUCCESS	0x0001
#define	FILE_COMPLETION_THREAD_RENAMED	0x0002

enum EPartFileOp{
	PFOP_NONE = 0,
	PFOP_HASHING,
	PFOP_COPYING,
	PFOP_UNCOMPRESSING
};

enum EPartFileHashsetState{
	PFHS_OK = 0,		// like hashsetneeded = false
	PFHS_REQUIRED,		// like hashsetneeded = true
	PFHS_REQUESTING		// like hashsetneeded = true && requesting from some client
};


class CSearchFile;
class CUpDownClient;
enum EDownloadState;
class CxImage;
class CSafeMemFile;

struct Requested_Block_Struct
{
    Requested_Block_Struct() {
        transferred = 0;
        data = NULL;
    };
	uchar	FileID[16];
    uint64_t	StartOffset;
    uint64_t	EndOffset;
    BYTE*   data;
	uint32_t  transferred;
};

struct Gap_Struct
{
	uint64_t start;
	uint64_t end;
};

struct PartFileBufferedData
{
	BYTE*  data;					// Barry - This is the data to be written
	uint64_t start;					// Barry - This is the start offset of the data
	uint64_t end;						// Barry - This is the end offset of the data
};

struct PartStatus_Struct
{
	PartStatus_Struct() {
		isComplete = false;
		isChecked = false;
		frequency = 0;
	}
    bool isComplete;
    bool isChecked;
	uint16_t frequency;
};

enum ePartIoExcPlace
{
	PFIOERR_NONE = 0,
	PFIOERR_ONFLUSH,
	PFIOERR_ONHASH,
	PFIOERR_ONALLOC
};


struct PartFile_Io_Error {
	ePartIoExcPlace where;
	CFileException* exception;
};

typedef CTypedPtrList<CPtrList, CUpDownClient*> CUpDownClientPtrList;

typedef std::list<Gap_Struct*> GapList;
typedef std::deque<PartFileBufferedData*> PartFileBufferedDataList;
typedef std::list<Requested_Block_Struct*> RequestedBlockList;

class CPartFile : public CKnownFile
{
	DECLARE_DYNAMIC(CPartFile)
#ifdef _DEBUG // ismod
	friend class TransferDebugWnd;
#endif
	friend class CPartFileConvert;
	friend class CPartHashThread;
public:
	CPartFile(UINT cat = 0);
	CPartFile(CSearchFile* searchresult, UINT cat = 0);
	CPartFile(LPCTSTR edonkeylink, UINT cat = 0);
	CPartFile(class CED2KFileLink* fileLink, UINT cat = 0);
	virtual ~CPartFile();

	bool	IsPartFile() const { return !(status == PS_COMPLETE); }

	// eD2K filename
	virtual void SetFileName(LPCTSTR pszFileName, bool bReplaceInvalidFileSystemChars = false, bool bRemoveControlChars = false); // 'bReplaceInvalidFileSystemChars' is set to 'false' for backward compatibility!

	// part.met filename (without path!)
	const CString& GetPartMetFileName() const { return m_partmetfilename; }

	// full path to part.met file or completed file
	const CString& GetFullName() const { return m_fullname; }
	void	SetFullName(LPCTSTR name) { m_fullname = name; }
	CString	GetTempPath() const;

	// local file system related properties
	bool	IsNormalFile() const { return (m_dwFileAttributes & (FILE_ATTRIBUTE_COMPRESSED | FILE_ATTRIBUTE_SPARSE_FILE)) == 0; }
	const bool	IsAllocating() const { return m_AllocateThread != NULL; }
	EMFileSize	GetRealFileSize() const;
	void	GetLeftToTransferAndAdditionalNeededSpace(uint64_t &ui64LeftToTransfer, uint64_t &pui32AdditionalNeededSpace) const;
	uint64_t	GetNeededSpace() const;

	// last file modification time (NT's version of UTC), to be used for stats only!
	CTime	GetCFileDate() const { return CTime(m_tLastModified); }
	time_t	GetFileDate() const { return m_tLastModified; }

	// file creation time (NT's version of UTC), to be used for stats only!
	CTime	GetCrCFileDate() const { return CTime(m_tCreated); }
	time_t	GetCrFileDate() const { return m_tCreated; }

	void	InitializeFromLink(CED2KFileLink* fileLink, UINT cat = 0);
	uint32_t	Process(uint32_t reducedownload, UINT icounter);
	EPartFileLoadResult	LoadPartFile(LPCTSTR in_directory, LPCTSTR filename, EPartFileFormat* pOutCheckFileFormat = NULL); //filename = *.part.met
	EPartFileLoadResult	ImportShareazaTempfile(LPCTSTR in_directory,LPCTSTR in_filename, EPartFileFormat* pOutCheckFileFormat = NULL);

	bool	SavePartFile(bool bDontOverrideBak = false);
	void	PartFileHashFinished(CKnownFile* result);
	// BEGIN SLUGFILLER: SafeHash - replaced old handlers, full hash checker remains for file completion
	void	PartHashFinished(uint16_t part, bool corrupt);
    bool	IsPartChecked(uint16_t part) const  { return m_PartsInfo[part].isChecked; }
    bool    IsPartComplete(uint16_t part) const { return m_PartsInfo[part].isComplete; };
	// END SLUGFILLER: SafeHash

	void	AddGap(uint64_t start, uint64_t end);
	void	FillGap(uint64_t start, uint64_t end);
	void	DrawStatusBar(CDC* dc, LPCRECT rect, bool bFlat) /*const*/;
	void    DrawSimpleStatusBar(CDC* dc, LPCRECT rect, bool bFlat) const;
	virtual void	DrawShareStatusBar(CDC* dc, LPCRECT rect, bool onlygreyrect, bool bFlat) const;
	bool	IsComplete(uint64_t start, uint64_t end, bool bIgnoreBufferedData) const;
	bool	IsPureGap(uint64_t start, uint64_t end) const;
	bool	IsAlreadyRequested(uint64_t start, uint64_t end) const;
    bool    ShrinkToAvoidAlreadyRequested(uint64_t& start, uint64_t& end) const;
	bool	IsCorruptedPart(UINT partnumber) const;
	uint64_t	GetTotalGapSizeInRange(uint64_t uRangeStart, uint64_t uRangeEnd) const;
	uint64_t	GetTotalGapSizeInPart(UINT uPart) const;
	void	UpdateCompletedInfos();
	void	UpdateCompletedInfos(uint64_t uTotalGaps);
	virtual void	UpdatePartsInfo();

	bool	GetNextRequestedBlock(CUpDownClient* sender, Requested_Block_Struct** newblocks, uint16_t* count) /*const*/;
	void	WritePartStatus(CSafeMemFile* file) const;
	void	AddSources(CSafeMemFile* sources,uint32_t serverip, uint16_t serverport, bool bWithObfuscationAndHash);
	// ismod: no PeerCache
	static bool CanAddSource(uint32_t userid, uint16_t port, uint32_t serverip, uint16_t serverport, UINT* pdebug_lowiddropped = NULL, bool Ed2kID = true);
	
	EPartFileStatus	GetStatus(bool ignorepause = false) const;
	void	SetStatus(EPartFileStatus eStatus);		// set status and update GUI
	void	_SetStatus(EPartFileStatus eStatus);	// set status and do *not* update GUI
	void	NotifyStatusChange();
	bool	IsStopped() const												{ return stopped; }
	bool	GetCompletionError() const										{ return m_bCompletionError; }
	EMFileSize  GetCompletedSize() const									{ return completedsize; }
	CString getPartfileStatus() const;
	int		getPartfileStatusRang() const;
	void	SetActive(bool bActive);

	uint8_t	GetDownPriority() const											{ return m_iDownPriority; }
	void	SetDownPriority(uint8_t iNewDownPriority, bool resort = true);
	bool	IsAutoDownPriority(void) const									{ return m_bAutoDownPriority; }
	void	SetAutoDownPriority(bool NewAutoDownPriority)					{ m_bAutoDownPriority = NewAutoDownPriority; }
	void	UpdateAutoDownPriority();

	UINT	GetSourceCount() const											{ return srclist.GetCount(); }
	UINT	GetSrcA4AFCount() const											{ return A4AFsrclist.GetCount(); }
	UINT	GetSrcStatisticsValue(EDownloadState nDLState) const;
	UINT	GetTransferringSrcCount() const;
	uint64_t	GetTransferred() const											{ return m_uTransferred; }
	uint32_t	GetDatarate() const												{ return datarate; }
	float	GetPercentCompleted() const										{ return percentcompleted; }
	UINT	GetNotCurrentSourcesCount() const;
	int		GetValidSourcesCount() const;
	bool	IsArchive(bool onlyPreviewable = false) const; // Barry - Also want to preview archives
    bool    IsPreviewableFileType() const;
	time_t	getTimeRemaining() const;
	time_t	getTimeRemainingSimple() const;
	time_t	GetDlActiveTime() const;

	// Barry - Added as replacement for BlockReceived to buffer data before writing to disk
	uint32_t	WriteToBuffer(Requested_Block_Struct *block);
	void	FlushBuffer(bool bForceWait = false);
	// Barry - This will invert the gap list, up to caller to delete gaps when done
	// 'Gaps' returned are really the filled areas, and guaranteed to be in order
	void	GetFilledList( GapList *filled ) const;

	// Barry - Added to prevent list containing deleted blocks on shutdown
	void	RemoveAllRequestedBlocks(void);
	bool	RemoveBlockFromList(uint64_t start, uint64_t end);
	bool	IsInRequestedBlockList(const Requested_Block_Struct* block) const;
	void	RemoveAllSources(bool bTryToSwap);

	bool	CanOpenFile() const;
	bool	IsReadyForPreview(bool real = false) const;
	bool	CanStopFile() const;
	bool	CanPauseFile() const;
	bool	CanResumeFile() const;
	bool	IsPausingOnPreview() const										{ return m_bPauseOnPreview && IsPreviewableFileType() && CanPauseFile(); }

	void	OpenFile() const;
	void	PreviewFile(bool real = false);
	void	DeleteFile();
	void	StopFile(bool bCancel = false, bool resort = true);
	void	PauseFile(bool bInsufficient = false, bool resort = true);
	void	StopPausedFile();
	void	ResumeFile(bool resort = true);
	void	ResumeFileInsufficient();
	void	SetPauseOnPreview(bool bVal)									{ m_bPauseOnPreview = bVal; }

	virtual Packet* CreateSrcInfoPacket(const CUpDownClient* forClient, uint8_t byRequestedVersion, uint16_t nRequestedOptions) const;
	void	AddClientSources(CSafeMemFile* sources, uint8_t sourceexchangeversion, bool bSourceExchange2, const CUpDownClient* pClient = NULL);

	UINT	GetAvailablePartCount() const									{ return availablePartsCount; }
	void	UpdateAvailablePartsCount();

	uint32_t	GetLastAnsweredTime() const										{ return m_ClientSrcAnswered; }
	void	SetLastAnsweredTime() { m_ClientSrcAnswered = ::GetTickCount(); }
	void	SetLastAnsweredTimeTimeout();

	uint64_t	GetCorruptionLoss() const { return m_uCorruptionLoss; }
	uint64_t	GetCompressionGain() const { return m_uCompressionGain; }

	virtual void	UpdateFileRatingCommentAvail(bool bForceUpdate = false);
	virtual void	RefilterFileComments();

	void	AddDownloadingSource(CUpDownClient* client);
	void	RemoveDownloadingSource(CUpDownClient* client);

	CString GetProgressString(uint16_t size) const;
	CString GetInfoSummary(bool bNoFormatCommands = false) const;

	void	UpdateDisplayedInfo(bool force = false);

	UINT	GetCategory() /*const*/;
	void	SetCategory(UINT cat);
	bool	HasDefaultCategory() const;
	bool	CheckShowItemInGivenCat(int inCategory) /*const*/;

	uint8_t*	MMCreatePartStatus();

	//preview
	virtual bool GrabImage(uint8_t nFramesToGrab, double dStartTime, bool bReduceColor, uint16_t nMaxWidth,void* pSender);
	virtual void GrabbingFinished(CxImage** imgResults, uint8_t nFramesGrabbed, void* pSender);

	void	IoExceptionHandler(PartFile_Io_Error *pfioerr);
	void	IoExceptionHandler();

	void	PerformFileCompleteEnd(DWORD dwResult);
    
	void	HashCompleteParts(); // Only hashes completed parts, why hash gaps?

	void	SetFileOp(EPartFileOp eFileOp);
	EPartFileOp GetFileOp() const											{ return m_eFileOp; }
	void	SetFileOpProgress(UINT uProgress);
	UINT	GetFileOpProgress() const										{ return m_uFileOpProgress; }

	uint32_t	m_LastSearchTime;
	uint32_t	m_LastSearchTimeKad;
	uint64_t	m_iAllocinfo;
	CUpDownClientPtrList srclist;
	CUpDownClientPtrList A4AFsrclist;
	CTime	lastseencomplete;
	CFile	m_hpartfile;				// permanent opened handle to avoid write conflicts
	CMutex 	m_FileCompleteMutex;		// mutex for file completion
	uint16_t	src_stats[4];
	uint16_t  net_stats[3];
	volatile bool m_bPreviewing;
	volatile bool m_bRecoveringArchive; // Is archive recovery in progress
	bool	m_bLocalSrcReqQueued;
	bool	srcarevisible;				// used for downloadlistctrl
	uint8_t	m_TotalSearchesKad;
    bool    AllowSwapForSourceExchange()					{ return ::GetTickCount()-lastSwapForSourceExchangeTick > 30*1000; } // ZZ:DownloadManager
    void    SetSwapForSourceExchangeTick()					{ lastSwapForSourceExchangeTick = ::GetTickCount(); } // ZZ:DownloadManager
	
	UINT	SetPrivateMaxSources(uint32_t in)					{ return m_uMaxSources = in; } 
	UINT	GetPrivateMaxSources() const					{ return m_uMaxSources; } 
	UINT	GetMaxSources() const;
	UINT	GetMaxSourcePerFileSoft() const;
	UINT	GetMaxSourcePerFileUDP() const;

    bool    GetPreviewPrio() const							{ return m_bpreviewprio; }
	void    SetPreviewPrio(bool in)							{ m_bpreviewprio=in; }

    static bool RightFileHasHigherPrio(CPartFile* left, CPartFile* right);

#ifdef _DEBUG
	// Diagnostic Support
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	bool	GetNextEmptyBlockInPart(UINT partnumber, Requested_Block_Struct* result, uint32_t blockSize = EMBLOCKSIZE) const;
	void	CompleteFile(bool hashingdone);
	void	CreatePartFile(UINT cat = 0);
	void	Init();

private:
	BOOL 		PerformFileComplete(); // Lord KiRon
	static UINT CompleteThreadProc(LPVOID pvParams); // Lord KiRon - Used as separate thread to complete file
	static UINT AFX_CDECL AllocateSpaceThread(LPVOID lpParam);
	void		CharFillRange(CString* buffer,uint32_t start, uint32_t end, char color) const;

	static CBarShader s_LoadBar;
	static CBarShader s_ChunkBar;
	static CBarShader s_CheckBar;
	time_t	m_iLastPausePurge;
	uint16_t	count;
	UINT	m_anStates[STATES_COUNT];
	EMFileSize	completedsize;
	uint64_t	m_uCorruptionLoss;
	uint64_t	m_uCompressionGain;
	uint32_t	datarate;
	CString m_fullname;
	CString m_partmetfilename;
	uint64_t	m_uTransferred;
	UINT	m_uMaxSources;
	bool	paused;
	bool	m_bPauseOnPreview;
	bool	stopped;
	bool	insufficient;
	bool	m_bCompletionError;
	uint8_t	m_iDownPriority;
	bool	m_bAutoDownPriority;
	EPartFileStatus	status;
	bool	newdate;	// indicates if there was a write access to the .part file
	uint32_t	lastpurgetime;
	uint32_t	m_LastNoNeededCheck;
	GapList gaplist;
	RequestedBlockList requestedblocks_list;
	//CArray<uint16> m_SrcpartFrequency;
	// BEGIN SLUGFILLER: SafeHash
    CArray<PartStatus_Struct> m_PartsInfo; // array with info about parts completion
    uint16_t	m_partsOnHash;
    void	HashSinglePart( uint16_t uPart );
	// END SLUGFILLER: SafeHash
	float	percentcompleted;
	CList<uint16_t, uint16_t> corrupted_list;
	uint32_t	m_ClientSrcAnswered;
	UINT	availablePartsCount;
	CSafeThread* m_AllocateThread;
	DWORD	m_lastRefreshedDLDisplay;
	CUpDownClientPtrList m_downloadingSourceList;
    bool	m_bpreviewprio;
public:    
	uint64_t	m_nTotalBufferData;
private:
	uint32_t	m_nLastBufferFlushTime;
	UINT	m_category;
	DWORD	m_dwFileAttributes;
	time_t	m_tActivated;
	time_t	m_nDlActiveTime;
	time_t	m_tLastModified;	// last file modification time (NT's version of UTC), to be used for stats only!
	time_t	m_tCreated;			// file creation time (NT's version of UTC), to be used for stats only!
    uint32_t	m_random_update_wait;	
	volatile EPartFileOp m_eFileOp;
	volatile UINT m_uFileOpProgress;

    DWORD   lastSwapForSourceExchangeTick; // ZZ:DownloadManaager

// ismod [start]
public:
    uint32_t	calcPartSize(uint16_t part) const { return (part == (partCount()-1)) ? (uint32_t)((uint64_t)m_nFileSize-(partCount()-1)*PARTSIZE) : PARTSIZE; }
    void	FlushDone();
    bool	isFlushing() const { return m_bIsFlushing; };
    void	SetFlushingFinished() { m_bIsFlushing = false; };
	CString getProgressbarStatus() const;
	void	setPreviewProcess(const HANDLE hProcess) { m_hPreviewProcess = hProcess; }
	void	setSparse( bool enable );
	bool	canDelete() const			{ return m_partsOnHash==0 && !m_bIsFlushing && m_AllocateThread==0; }
	bool	pendingOnDelete() const		{ return m_bPendingOnDelete; }
	void	setHashestState( EPartFileHashsetState state ) { m_eHashsetState = state; }
	EPartFileHashsetState hashsetShate() const { return m_eHashsetState; }

private:
	PartFileBufferedDataList m_FlushData_list;
	PartFileBufferedDataList m_BufferedData_list;
	EPartFileHashsetState m_eHashsetState;
	bool	m_bPendingOnDelete;
	bool	m_bAllocateFullSpace;
    bool    m_bIsFlushing;
    HANDLE  m_waitFlushEvent;
	HANDLE	m_hPreviewProcess;
	bool	m_fPreviewSrcTrigger;

	friend  class CPartFileFlushThread;
// ismod [end]
};

//////////////////////////////////////////////////////////////////////////
//
//  PartFile Hash Thread
//

struct HashInfo_Struct
{
	CPartFile*			owner;
	std::vector<uint16_t>	partsToHash;
};

class CPartHashThread : public CSafeThread
{
	DECLARE_DYNCREATE(CPartHashThread)
public:
	virtual	BOOL InitInstance();
	DECLARE_MESSAGE_MAP() 
	afx_msg void OnHashParts(WPARAM wParam, LPARAM lParam);

protected:
	uchar m_localData[PARTSIZE];
};

class CPartFileFlushThread : public CSafeThread
{
	DECLARE_DYNCREATE(CPartFileFlushThread)
public:
	virtual	BOOL InitInstance();
	DECLARE_MESSAGE_MAP() 
	afx_msg void OnFlushFile(WPARAM wParam, LPARAM lParam);
};