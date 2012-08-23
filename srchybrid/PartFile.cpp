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
#include "stdafx.h"
#include <sys/stat.h>
#include <io.h>
#include <winioctl.h>
#ifdef _DEBUG
#include "DebugHelpers.h"
#endif
#include "emule.h"
#include "PartFile.h"
#include "UpDownClient.h"
// ismod: no Peercache
#include "ED2KLink.h"
#include "Preview.h"
#include "ArchiveRecovery.h"
#include "SearchFile.h"
#include "Kademlia/Kademlia/Kademlia.h"
#include "kademlia/kademlia/search.h"
#include "kademlia/kademlia/SearchManager.h"
#include "kademlia/utils/MiscUtils.h"
#include "kademlia/kademlia/prefs.h"
#include "kademlia/kademlia/Entry.h"
#include "DownloadQueue.h"
#include "IPFilter.h"
#include "MMServer.h"
#include "OtherFunctions.h"
#include "Packets.h"
#include "Preferences.h"
#include "SafeFile.h"
#include "SharedFileList.h"
#include "ListenSocket.h"
#include "Sockets.h"
#include "Server.h"
#include "KnownFileList.h"
#include "emuledlg.h"
#include "TransferDlg.h"
#include "TaskbarNotifier.h"
#include "ClientList.h"
#include "Statistics.h"
// ismod: no aich
// ismod: no Peercache
#include "Log.h"
#include "CollectionViewDialog.h"
#include "Collection.h"
// ismod
#include "UserMsgs.h"
#include "DesktopIntegration.h"
#include "PreviewWaitDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define PROGRESS_HEIGHT 3

CBarShader CPartFile::s_LoadBar(PROGRESS_HEIGHT);
CBarShader CPartFile::s_ChunkBar(16);
CBarShader CPartFile::s_CheckBar(PROGRESS_HEIGHT); 

IMPLEMENT_DYNAMIC(CPartFile, CKnownFile)

CPartFile::CPartFile( UINT ucat )
{
	Init();
	m_category = ucat;
}

CPartFile::CPartFile(CSearchFile* searchresult, UINT cat)
{
	Init();

	const CTypedPtrList<CPtrList, Kademlia::CEntry*>& list = searchresult->getNotes();
	for ( POSITION pos = list.GetHeadPosition(); pos != NULL; )
	{
			Kademlia::CEntry* entry = list.GetNext(pos);
			m_kadNotes.AddTail(entry->Copy());
	}
	UpdateFileRatingCommentAvail();

	md4cpy(m_abyFileHash, searchresult->GetFileHash());
	for (int i = 0; i < searchresult->taglist.GetCount();i++){
		const CTag* pTag = searchresult->taglist[i];
		switch (pTag->GetNameID()){
			case FT_FILENAME:{
				ASSERT( pTag->IsStr() );
				if (pTag->IsStr()){
					if (GetFileName().IsEmpty())
						SetFileName(pTag->GetStr(), true, true);
				}
				break;
			}
			case FT_FILESIZE:{
				ASSERT( pTag->IsInt64(true) );
				if (pTag->IsInt64(true))
					SetFileSize(pTag->GetInt64());
				break;
			}
			default:{
				bool bTagAdded = false;
				if (pTag->GetNameID() != 0 && pTag->GetName() == NULL && (pTag->IsStr() || pTag->IsInt()))
				{
					static const struct
					{
						uint8_t	nName;
						uint8_t	nType;
					} _aMetaTags[] = 
					{
						{ FT_MEDIA_ARTIST,  2 },
						{ FT_MEDIA_ALBUM,   2 },
						{ FT_MEDIA_TITLE,   2 },
						{ FT_MEDIA_LENGTH,  3 },
						{ FT_MEDIA_BITRATE, 3 },
						{ FT_MEDIA_CODEC,   2 },
						{ FT_FILETYPE,		2 },
						{ FT_FILEFORMAT,	2 }
					};
					for (int t = 0; t < _countof(_aMetaTags); t++)
					{
						if (pTag->GetType() == _aMetaTags[t].nType && pTag->GetNameID() == _aMetaTags[t].nName)
						{
							// skip string tags with empty string values
							if (pTag->IsStr() && pTag->GetStr().IsEmpty())
								break;

							// skip integer tags with '0' values
							if (pTag->IsInt() && pTag->GetInt() == 0)
								break;

							TRACE(_T("CPartFile::CPartFile(CSearchFile*): added tag %s\n"), pTag->GetFullInfo(DbgGetFileMetaTagName));
							CTag* newtag = new CTag(*pTag);
							taglist.Add(newtag);
							bTagAdded = true;
							break;
						}
					}
				}

				if (!bTagAdded)
					TRACE(_T("CPartFile::CPartFile(CSearchFile*): ignored tag %s\n"), pTag->GetFullInfo(DbgGetFileMetaTagName));
			}
		}
	}
	CreatePartFile(cat);
	m_category=cat;
}

CPartFile::CPartFile(LPCTSTR edonkeylink, UINT cat)
{
	CED2KLink* pLink = 0;
	try {
		pLink = CED2KLink::CreateLinkFromUrl(edonkeylink);
		_ASSERT( pLink != 0 );
		CED2KFileLink* pFileLink = pLink->GetFileLink();
		if (pFileLink==0) 
			throw GetResString(IDS_ERR_NOTAFILELINK);
		InitializeFromLink(pFileLink,cat);
	} catch (CString error) {
		CString strMsg;
		strMsg.Format(GetResString(IDS_ERR_INVALIDLINK), error);
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_LINKERROR), strMsg);
		SetStatus(PS_ERROR);
	}
	delete pLink;
}

void CPartFile::InitializeFromLink(CED2KFileLink* fileLink, UINT cat)
{
	Init();
	try{
		SetFileName(fileLink->GetName(), true, true);
		SetFileSize(fileLink->GetSize());
		md4cpy(m_abyFileHash, fileLink->GetHashKey());
		if (!theApp.downloadqueue->IsFileExisting(m_abyFileHash))
		{
			if (fileLink->m_hashset && fileLink->m_hashset->GetLength() > 0)
			{
				try
				{
					if (!LoadHashsetFromFile(fileLink->m_hashset, true))
					{
						ASSERT( m_hasharray.count() == 0 );
						AddDebugLogLine(false, _T("eD2K link \"%s\" specified with invalid hashset"), fileLink->GetName());
					}
					else
						m_eHashsetState = PFHS_OK;
						//hashsetneeded = false;
				}
				catch (CFileException* e)
				{
					TCHAR szError[MAX_CFEXP_ERRORMSG];
					e->GetErrorMessage(szError, _countof(szError));
					AddDebugLogLine(false, _T("Error: Failed to process hashset for eD2K link \"%s\" - %s"), fileLink->GetName(), szError);
					e->Delete();
				}
			}
			CreatePartFile(cat);
			m_category=cat;
		}
		else
			SetStatus(PS_ERROR);
	}
	catch(CString error){
		CString strMsg;
		strMsg.Format(GetResString(IDS_ERR_INVALIDLINK), error);
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_LINKERROR), strMsg);
		SetStatus(PS_ERROR);
	}
}

CPartFile::CPartFile(CED2KFileLink* fileLink, UINT cat)
{
	InitializeFromLink(fileLink,cat);
}

void CPartFile::Init(){
	newdate = true;
	m_LastSearchTime = 0;
	m_LastSearchTimeKad = 0;
	m_TotalSearchesKad = 0;
	lastpurgetime = ::GetTickCount();
	paused = false;
	stopped= false;
	status = PS_EMPTY;
	insufficient = false;
	m_bCompletionError = false;
	m_uTransferred = 0;
	m_iLastPausePurge = time(NULL);
	m_AllocateThread=NULL;
	m_iAllocinfo = 0;
	if(thePrefs.GetNewAutoDown()){
		m_iDownPriority = PR_HIGH;
		m_bAutoDownPriority = true;
	}
	else{
		m_iDownPriority = PR_NORMAL;
		m_bAutoDownPriority = false;
	}
	srcarevisible = false;
	memset(m_anStates,0,sizeof(m_anStates));
	datarate = 0;
	m_uMaxSources = 0;
	m_eHashsetState = PFHS_OK;
	//hashsetneeded = true;
	count = 0;
	percentcompleted = 0;
	completedsize = (uint64_t)0;
	m_bPreviewing = false;
	lastseencomplete = NULL;
	availablePartsCount=0;
	m_ClientSrcAnswered = 0;
	m_LastNoNeededCheck = 0;
	m_uRating = 0;
	(void)m_strComment;
	m_nTotalBufferData = 0;
	m_nLastBufferFlushTime = ::GetTickCount(); // netfinity: In case of rollover!
	m_bRecoveringArchive = false;
	m_uCompressionGain = 0;
	m_uCorruptionLoss = 0;
	m_category=0;
	m_lastRefreshedDLDisplay = 0;
	m_bLocalSrcReqQueued = false;
	memset(src_stats,0,sizeof(src_stats));
	memset(net_stats,0,sizeof(net_stats));
	m_nCompleteSourcesTime = time(NULL);
	m_nCompleteSourcesCount = 0;
	m_nCompleteSourcesCountLo = 0;
	m_nCompleteSourcesCountHi = 0;
	m_dwFileAttributes = 0;
	m_bPendingOnDelete=false;
	m_tActivated = 0;
	m_nDlActiveTime = 0;
	m_tLastModified = (UINT)-1;
	m_tUtcLastModified = (UINT)-1;
	m_tCreated = 0;
	m_eFileOp = PFOP_NONE;
	m_uFileOpProgress = 0;
    m_bpreviewprio = false;
    m_random_update_wait = (uint32_t)(rand()/(RAND_MAX/1000));
    lastSwapForSourceExchangeTick = ::GetTickCount();
	m_bPauseOnPreview = false;
	// ismod [start]
	m_partsOnHash = 0;
	m_bIsFlushing = false;
	m_bAllocateFullSpace = false;
    m_waitFlushEvent = CreateEvent(0,TRUE,TRUE,0);
	m_hPreviewProcess = NULL;
}

CPartFile::~CPartFile()
{
	// terminate allocation thread
	if ( NULL != m_AllocateThread ) {
		HANDLE hThread = m_AllocateThread->m_hThread;
		TerminateThread(hThread, 100);
	}

	// wait for current and next flush
	if ( INVALID_HANDLE_VALUE != m_hpartfile.m_hFile ) {
		WaitForSingleObject(m_waitFlushEvent, INFINITE);

		// commit file and directory entry
		m_hpartfile.Close();
		// Update met file (with current directory entry)
		SavePartFile();
	}
	
	CloseHandle(m_waitFlushEvent);
	
	// clear gap list
	for ( GapList::iterator it = gaplist.begin(); it != gaplist.end(); ++it ) {
		delete *it;
	}
	gaplist.clear();

	// clear buffered data list
	for( PartFileBufferedDataList::iterator it = m_BufferedData_list.begin(); it != m_BufferedData_list.end(); ++it )
	{
		delete[] (*it)->data;
		delete *it;
	}
	m_BufferedData_list.clear();

	// clear flush list
	for( PartFileBufferedDataList::iterator it = m_FlushData_list.begin(); it != m_FlushData_list.end(); ++it )
	{
		if( NULL != (*it)->data )
			delete[] (*it)->data;
		delete *it;
	}
	m_FlushData_list.clear();

}

#ifdef _DEBUG
void CPartFile::AssertValid() const
{
	CKnownFile::AssertValid();

	(void)m_LastSearchTime;
	(void)m_LastSearchTimeKad;
	(void)m_TotalSearchesKad;
	srclist.AssertValid();
	A4AFsrclist.AssertValid();
	(void)lastseencomplete;
	m_hpartfile.AssertValid();
	m_FileCompleteMutex.AssertValid();
	(void)src_stats;
	(void)net_stats;
	CHECK_BOOL(m_bPreviewing);
	CHECK_BOOL(m_bRecoveringArchive);
	CHECK_BOOL(m_bLocalSrcReqQueued);
	CHECK_BOOL(srcarevisible);
	(void)m_eHashsetState;
	//CHECK_BOOL(hashsetneeded);
	(void)m_iLastPausePurge;
	(void)count;
	(void)m_anStates;
	ASSERT( completedsize <= m_nFileSize );
	(void)m_uCorruptionLoss;
	(void)m_uCompressionGain;
	(void)datarate;
	(void)m_fullname;
	(void)m_partmetfilename;
	(void)m_uTransferred;
	CHECK_BOOL(paused);
	CHECK_BOOL(stopped);
	CHECK_BOOL(insufficient);
	CHECK_BOOL(m_bCompletionError);
	ASSERT( m_iDownPriority == PR_LOW || m_iDownPriority == PR_NORMAL || m_iDownPriority == PR_HIGH );
	CHECK_BOOL(m_bAutoDownPriority);
	ASSERT( status == PS_READY || status == PS_EMPTY || status == PS_WAITINGFORHASH || status == PS_ERROR || status == PS_COMPLETING || status == PS_COMPLETE );
	CHECK_BOOL(newdate);
	(void)lastpurgetime;
	(void)m_LastNoNeededCheck;
	// ismod
	ASSERT( percentcompleted >= 0.0F && percentcompleted <= 100.0F );
	corrupted_list.AssertValid();
	(void)availablePartsCount;
	(void)m_ClientSrcAnswered;
	(void)s_LoadBar;
	(void)s_ChunkBar;
	(void)m_lastRefreshedDLDisplay;
	m_downloadingSourceList.AssertValid();
	(void)m_nTotalBufferData;
	(void)m_nLastBufferFlushTime;
	(void)m_category;
	(void)m_dwFileAttributes;
}

void CPartFile::Dump(CDumpContext& dc) const
{
	CKnownFile::Dump(dc);
}
#endif


void CPartFile::CreatePartFile(UINT cat)
{
	if ( MAX_EMULE_FILE_SIZE < m_nFileSize ) {
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_CREATEPARTFILE));
		SetStatus(PS_ERROR);
		return;
	}

	// decide which tempfolder to use
	CString tempdirtouse = theApp.downloadqueue->GetOptimalTempDir(cat, GetFileSize());

	// use lowest free partfilenumber for free file
	CString filename; 
	uint32_t i = 0; 
	do {
		i++; 
		filename.Format(_T("%s\\%03u.part"), tempdirtouse, i); 
	} while ( PathFileExists(filename) );
	m_partmetfilename.Format(_T("%03u.part.met"), i); 
	SetPath(tempdirtouse);
	m_fullname.Format(_T("%s\\%s"), tempdirtouse, m_partmetfilename);

	CTag* partnametag = new CTag(FT_PARTFILENAME, RemoveFileExtension(m_partmetfilename));
	taglist.Add(partnametag);
	
	Gap_Struct* gap = new Gap_Struct;
	gap->start = 0ui64;
	gap->end = m_nFileSize - 1ui64;
	gaplist.push_back(gap);

	CString partfull(RemoveFileExtension(m_fullname));
	SetFilePath(partfull);
	if ( !m_hpartfile.Open(partfull,CFile::modeCreate|CFile::modeReadWrite|CFile::shareDenyWrite|CFile::osSequentialScan) ) {
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_CREATEPARTFILE));
		SetStatus(PS_ERROR);
	} else {
		if ( thePrefs.GetSpaceAllocPolicy() == SPACE_ALLOC_SPARSE ) {
			setSparse(true);
		}

		struct _stat fileinfo;
		if (_tstat(partfull, &fileinfo) == 0){
			m_tLastModified = fileinfo.st_mtime;
			m_tCreated = fileinfo.st_ctime;
		}
		else
			AddDebugLogLine(false, _T("Failed to get file date for \"%s\" - %s"), partfull, _tcserror(errno));
	}
	m_dwFileAttributes = GetFileAttributes(partfull);
	if ( INVALID_FILE_ATTRIBUTES == m_dwFileAttributes )
		m_dwFileAttributes = 0;

	if ( 0 == m_hasharray.count() ) {
		if (1 < GetED2KPartCount() )
			m_eHashsetState = PFHS_REQUIRED;
		else {
			m_eHashsetState = PFHS_OK;
			m_hasharray.allocate(1);
			md4cpy(m_hasharray.append_v(), m_abyFileHash);
		}
	}

	// prepare array about parts info
    m_PartsInfo.SetSize(partCount());

	paused = false;

	if ( thePrefs.AutoFilenameCleanup() )
		SetFileName(CleanupFilename(GetFileName()));

    SavePartFile();
	SetActive(theApp.IsConnected());
}

/* 
* David: Lets try to import a Shareaza download ...
*
* The first part to get filename size and hash is easy 
* the secund part to get the hashset and the gap List
* is much more complicated.
*
* We could parse the whole *.sd file but I chose a other tricky way:
* To find the hashset we will search for the ed2k hash, 
* it is repeated on the begin of the hashset
* To get the gap list we will process analog 
* but now we will search for the file size.
*
*
* The *.sd file format for version 32
* [S][D][L] <-- File ID
* [20][0][0][0] <-- Version
* [FF][FE][FF][BYTE]NAME <-- len;Name 
* [QWORD] <-- Size
* [BYTE][0][0][0]SHA(20)[BYTE][0][0][0] <-- SHA Hash
* [BYTE][0][0][0]TIGER(24)[BYTE][0][0][0] <-- TIGER Hash
* [BYTE][0][0][0]MD5(16)[BYTE][0][0][0] <-- MD4 Hash
* [BYTE][0][0][0]ED2K(16)[BYTE][0][0][0] <-- ED2K Hash
* [...] <-- Saved Sources
* [QWORD][QWORD][DWORD]GAP(QWORD:QWORD)<-- Gap List: Total;Left;count;gap1(begin:length),gap2,Gap3,...
* [...] <-- Bittorent Info
* [...] <-- Tiger Tree
* [DWORD]ED2K(16)HASH1(16)HASH2(16)... <-- ED2K Hash Set: count;ed2k hash;hash1,hash2,hash3,...
* [...] <-- Comments
*/
EPartFileLoadResult CPartFile::ImportShareazaTempfile(LPCTSTR in_directory,LPCTSTR in_filename, EPartFileFormat* pOutCheckFileFormat) 
{
	CString fullname;
	fullname.Format(_T("%s\\%s"), in_directory, in_filename);

	// open the file
	CFile sdFile;
	CFileException fexpMet;
	if (!sdFile.Open(fullname, CFile::modeRead|CFile::osSequentialScan|CFile::typeBinary|CFile::shareDenyWrite, &fexpMet)){
		CString strError;
		strError.Format(GetResString(IDS_ERR_OPENMET), in_filename, _T(""));
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		if (fexpMet.GetErrorMessage(szError, _countof(szError))){
			strError += _T(" - ");
			strError += szError;
		}
		LogError(LOG_STATUSBAR, _T("%s"), strError);
		return PLR_FAILED_METFILE_NOACCESS;
	}
	//	setvbuf(sdFile.m_pStream, NULL, _IOFBF, 16384);

	try{
		CArchive ar( &sdFile, CArchive::load );

		// Is it a valid Shareaza temp file?
		CHAR szID[3];
		ar.Read( szID, 3 );
		if ( strncmp( szID, "SDL", 3 ) ){ 
			ar.Close();
			sdFile.Close();
			if (pOutCheckFileFormat != NULL)
				*pOutCheckFileFormat = PMT_UNKNOWN;
			return PLR_FAILED_OTHER;
		}

		// Get the version
		int nVersion;
		ar >> nVersion;

		// Get the File Name
		CString sRemoteName;
		ar >> sRemoteName;
		SetFileName(sRemoteName);

		// Get the File Size
		unsigned __int64 lSize;
		EMFileSize nSize;
		/*if ( nVersion >= 29 ){
			ar >> lSize;
			nSize = lSize;
		}else
			ar >> nSize;*/
		ar >> lSize;
		nSize = lSize;
		SetFileSize(nSize);

		// Get the ed2k hash
		BOOL bSHA1, bTiger, bMD5, bED2K, Trusted; bMD5 = false; bED2K = false;
		BYTE pSHA1[20];
		BYTE pTiger[24];
		BYTE pMD5[16];
		BYTE pED2K[16];

		ar >> bSHA1;
		if ( bSHA1 ) ar.Read( pSHA1, sizeof(pSHA1) );
		if ( nVersion >= 31 ) ar >> Trusted;

		ar >> bTiger;
		if ( bTiger ) ar.Read( pTiger, sizeof(pTiger) );
		if ( nVersion >= 31 ) ar >> Trusted;

		if ( nVersion >= 22 ) ar >> bMD5;
		if ( bMD5 ) ar.Read( pMD5, sizeof(pMD5) );
		if ( nVersion >= 31 ) ar >> Trusted;

		if ( nVersion >= 13 ) ar >> bED2K;
		if ( bED2K ) ar.Read( pED2K, sizeof(pED2K) );
		if ( nVersion >= 31 ) ar >> Trusted;

		ar.Close();

		if(bED2K){
			md4cpy(m_abyFileHash, pED2K);
		}else{
			Log(LOG_ERROR,GetResString(IDS_X_SHAREAZA_IMPORT_NO_HASH),in_filename);
			sdFile.Close();
			return PLR_FAILED_OTHER;
		}

		if (pOutCheckFileFormat != NULL){
			*pOutCheckFileFormat = PMT_SHAREAZA;
			return PLR_CHECKSUCCESS;
		}

		// Now the tricky part
		LONGLONG basePos = sdFile.GetPosition();

		// Try to to get the gap list
		if(gotostring(sdFile,nVersion >= 29 ? (uchar*)&lSize : (uchar*)&nSize,nVersion >= 29 ? 8 : 4)) // search the gap list
		{
			sdFile.Seek(sdFile.GetPosition()-(nVersion >= 29 ? 8 : 4),CFile::begin); // - file size
			CArchive ar( &sdFile, CArchive::load );

			bool badGapList = false;

			if( nVersion >= 29 )
			{
				__int64 nTotal, nRemaining;
				DWORD nFragments;
				ar >> nTotal >> nRemaining >> nFragments;

				if(nTotal >= nRemaining){
					__int64 begin, length;
					for (; nFragments--; ){
						ar >> begin >> length;
						if(begin + length > nTotal){
							badGapList = true;
							break;
						}
						AddGap((uint32_t)begin, (uint32_t)(begin+length-1));
					}
				}else
					badGapList = true;
			}
			else
			{
				DWORD nTotal, nRemaining;
				DWORD nFragments;
				ar >> nTotal >> nRemaining >> nFragments;

				if(nTotal >= nRemaining){
					DWORD begin, length;
					for (; nFragments--; ){
						ar >> begin >> length;
						if(begin + length > nTotal){
							badGapList = true;
							break;
						}
						AddGap(begin,begin+length-1);
					}
				}else
					badGapList = true;
			}

			if ( badGapList ) {
				for ( GapList::iterator it = gaplist.begin(); it != gaplist.end(); ++it ) {
					delete *it;
				}
				gaplist.clear();
				Log(LOG_WARNING,GetResString(IDS_X_SHAREAZA_IMPORT_GAP_LIST_CORRUPT),in_filename);
			}

			ar.Close();
		}
		else{
			Log(LOG_WARNING,GetResString(IDS_X_SHAREAZA_IMPORT_NO_GAP_LIST),in_filename);
			sdFile.Seek(basePos,CFile::begin); // not found, reset start position
		}

		// Try to get the complete hashset
		if(gotostring(sdFile,m_abyFileHash,16)) // search the hashset
		{
			sdFile.Seek(sdFile.GetPosition()-16-4,CFile::begin); // - list size - hash length
			CArchive ar( &sdFile, CArchive::load );

			DWORD nCount;
			ar >> nCount;

			BYTE pMD4[16];
			ar.Read( pMD4, sizeof(pMD4) ); // read the hash again

			// read the hashset
			m_hasharray.allocate(nCount);
			for (DWORD i = 0; i < nCount; i++){
				ar.Read( m_hasharray.append_v(), 16 );
			}

			
			if ( !m_hasharray.empty() )
			{
				uchar checkhash[16];
				CreateHash(m_hasharray.data(), m_hasharray.count()*m_hasharray.hashSize(), checkhash);

				if ( md4cmp(pMD4, checkhash) )
				{
					m_hasharray.release();
					Log(LOG_WARNING,GetResString(IDS_X_SHAREAZA_IMPORT_HASH_SET_CORRUPT),in_filename);
				}
			}
			ar.Close();
		}
		else{
			Log(LOG_WARNING,GetResString(IDS_X_SHAREAZA_IMPORT_NO_HASH_SET),in_filename);
			//sdFile.Seek(basePos,CFile::begin); // not found, reset start position
		}

		// Close the file
		sdFile.Close();
	}
	catch(CArchiveException* error){
		TCHAR buffer[MAX_CFEXP_ERRORMSG];
		error->GetErrorMessage(buffer,_countof(buffer));
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_FILEERROR), in_filename, GetFileName(), buffer);
		error->Delete();
		return PLR_FAILED_OTHER;
	}
	catch(CFileException* error){
		if (error->m_cause == CFileException::endOfFile){
			LogError(LOG_STATUSBAR, GetResString(IDS_ERR_METCORRUPT), in_filename, GetFileName());
		}else{
			TCHAR buffer[MAX_CFEXP_ERRORMSG];
			error->GetErrorMessage(buffer,_countof(buffer));
			LogError(LOG_STATUSBAR, GetResString(IDS_ERR_FILEERROR), in_filename, GetFileName(), buffer);
		}
		error->Delete();
		return PLR_FAILED_OTHER;
	}
#ifndef _DEBUG
	catch(...){
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_METCORRUPT), in_filename, GetFileName());
		ASSERT(0);
		return PLR_FAILED_OTHER;
	}
#endif

	// The part below would be a copy of the CPartFile::LoadPartFile, 
	// so it is smarter to save and reload the file insta dof dougling the whole stuff
	if(!SavePartFile())
		return PLR_FAILED_OTHER;

	m_hasharray.release();
	for ( GapList::iterator it = gaplist.begin(); it != gaplist.end(); ++it ) {
		delete *it;
	}
	gaplist.clear();

	return LoadPartFile(in_directory, in_filename);
}

EPartFileLoadResult CPartFile::LoadPartFile( LPCTSTR in_directory, LPCTSTR in_filename, EPartFileFormat* pOutCheckFileFormat )
{
	bool isnewstyle;
	uint8_t version;
	EPartFileFormat partmettype = PMT_UNKNOWN;

	CMap<UINT, UINT, Gap_Struct*, Gap_Struct*> gap_map;
	m_uTransferred = 0;
	m_partmetfilename = in_filename;
	SetPath(in_directory);
	m_fullname.Format(_T("%s\\%s"), GetPath(), m_partmetfilename);

	// readfile data form part.met file
	CSafeBufferedFile metFile;
	CFileException fexpMet;
	if ( !metFile.Open(m_fullname, CFile::modeRead|CFile::osSequentialScan|CFile::typeBinary|CFile::shareDenyWrite, &fexpMet) ) {
		CString strError;
		strError.Format(GetResString(IDS_ERR_OPENMET), m_partmetfilename, _T(""));
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		if (fexpMet.GetErrorMessage(szError, _countof(szError))){
			strError += _T(" - ");
			strError += szError;
		}
		LogError(LOG_STATUSBAR, _T("%s"), strError);
		return PLR_FAILED_METFILE_NOACCESS;
	}
	setvbuf(metFile.m_pStream, NULL, _IOFBF, 16384);

	try{
		version = metFile.ReadUInt8();
		
		if (version != PARTFILE_VERSION && version != PARTFILE_SPLITTEDVERSION && version != PARTFILE_VERSION_LARGEFILE){
			metFile.Close();
			if (version==83) {				
				return ImportShareazaTempfile(in_directory, in_filename, pOutCheckFileFormat);
			}
			LogError(LOG_STATUSBAR, GetResString(IDS_ERR_BADMETVERSION), m_partmetfilename, GetFileName());
			return PLR_FAILED_METFILE_CORRUPT;
		}
		
		isnewstyle = (version == PARTFILE_SPLITTEDVERSION);
		partmettype = isnewstyle ? PMT_SPLITTED : PMT_DEFAULTOLD;
		if (!isnewstyle) {
			uint8_t test[4];
			metFile.Seek(24, CFile::begin);
			metFile.Read(&test[0], 1);
			metFile.Read(&test[1], 1);
			metFile.Read(&test[2], 1);
			metFile.Read(&test[3], 1);

			metFile.Seek(1, CFile::begin);

			if (test[0]==0 && test[1]==0 && test[2]==2 && test[3]==1) {
				isnewstyle = true;	// edonkeys so called "old part style"
				partmettype = PMT_NEWOLD;
			}
		}

		if (isnewstyle) {
			uint32_t temp;
			metFile.Read(&temp,4);

			if (temp == 0) {	// 0.48 partmets - different again
				LoadHashsetFromFile(&metFile, false);
			}
			else {
				uchar gethash[16];
				metFile.Seek(2, CFile::begin);
				LoadDateFromFile(&metFile);
				metFile.Read(gethash, 16);
				md4cpy(m_abyFileHash, gethash);
			}
		}
		else {
			LoadDateFromFile(&metFile);
			LoadHashsetFromFile(&metFile, false);
		}

		UINT tagcount = metFile.ReadUInt32();
		for (UINT j = 0; j < tagcount; j++){
			CTag* newtag = new CTag(&metFile, false);
			if (pOutCheckFileFormat == NULL || (pOutCheckFileFormat != NULL && (newtag->GetNameID()==FT_FILESIZE || newtag->GetNameID()==FT_FILENAME))){
			    switch (newtag->GetNameID()){
				    case FT_FILENAME:{
					    if (!newtag->IsStr()) {
						    LogError(LOG_STATUSBAR, GetResString(IDS_ERR_METCORRUPT), m_partmetfilename, GetFileName());
						    delete newtag;
						    return PLR_FAILED_METFILE_CORRUPT;
					    }
						if (GetFileName().IsEmpty())
							SetFileName(newtag->GetStr());
					    delete newtag;
					    break;
				    }
				    case FT_LASTSEENCOMPLETE:{
						ASSERT( newtag->IsInt() );
						if (newtag->IsInt())
						    lastseencomplete = newtag->GetInt();
					    delete newtag;
					    break;
				    }
				    case FT_FILESIZE:{
						ASSERT( newtag->IsInt64(true) );
						if (newtag->IsInt64(true))
						    SetFileSize(newtag->GetInt64());
					    delete newtag;
					    break;
				    }
				    case FT_TRANSFERRED:{
						ASSERT( newtag->IsInt64(true) );
						if (newtag->IsInt64(true))
						    m_uTransferred = newtag->GetInt64();
					    delete newtag;
					    break;
				    }
				    case FT_COMPRESSION:{
						ASSERT( newtag->IsInt64(true) );
						if (newtag->IsInt64(true))
							m_uCompressionGain = newtag->GetInt64();
					    delete newtag;
					    break;
				    }
				    case FT_CORRUPTED:{
						ASSERT( newtag->IsInt64() );
						if (newtag->IsInt64())
							m_uCorruptionLoss = newtag->GetInt64();
					    delete newtag;
					    break;
				    }
				    case FT_FILETYPE:{
						ASSERT( newtag->IsStr() );
						if (newtag->IsStr())
						    SetFileType(newtag->GetStr());
					    delete newtag;
					    break;
				    }
				    case FT_CATEGORY:{
						ASSERT( newtag->IsInt() );
						if (newtag->IsInt())
							m_category = newtag->GetInt();
					    delete newtag;
					    break;
				    }
					case FT_MAXSOURCES: {
						ASSERT( newtag->IsInt() );
						if (newtag->IsInt())
							m_uMaxSources = newtag->GetInt();
					    delete newtag;
					    break;
				    }
				    case FT_DLPRIORITY:{
						ASSERT( newtag->IsInt() );
						if (newtag->IsInt()){
							if (!isnewstyle){
								m_iDownPriority = (uint8_t)newtag->GetInt();
								if( m_iDownPriority == PR_AUTO ){
									m_iDownPriority = PR_HIGH;
									SetAutoDownPriority(true);
								}
								else{
									if (m_iDownPriority != PR_LOW && m_iDownPriority != PR_NORMAL && m_iDownPriority != PR_HIGH)
										m_iDownPriority = PR_NORMAL;
									SetAutoDownPriority(false);
								}
							}
						}
						delete newtag;
						break;
				    }
				    case FT_STATUS:{
						ASSERT( newtag->IsInt() );
						if (newtag->IsInt()){
						    paused = newtag->GetInt()!=0;
						    stopped = paused;
						}
					    delete newtag;
					    break;
				    }
				    case FT_ULPRIORITY:{
						ASSERT( newtag->IsInt() );
						if (newtag->IsInt()){
							if (!isnewstyle){
								int iUpPriority = newtag->GetInt();
								if( iUpPriority == PR_AUTO ){
									SetUpPriority(PR_HIGH, false);
									SetAutoUpPriority(true);
								}
								else{
									if (iUpPriority != PR_VERYLOW && iUpPriority != PR_LOW && iUpPriority != PR_NORMAL && iUpPriority != PR_HIGH && iUpPriority != PR_VERYHIGH)
										iUpPriority = PR_NORMAL;
									SetUpPriority((uint8_t)iUpPriority, false);
									SetAutoUpPriority(false);
								}
							}
						}
						delete newtag;
					    break;
				    }
				    case FT_KADLASTPUBLISHSRC:{
						ASSERT( newtag->IsInt() );
						if (newtag->IsInt())
						{
						    SetLastPublishTimeKadSrc(newtag->GetInt(), 0);
							if(GetLastPublishTimeKadSrc() > (uint32_t)time(NULL)+KADEMLIAREPUBLISHTIMES)
							{
								//There may be a posibility of an older client that saved a random number here.. This will check for that..
								SetLastPublishTimeKadSrc(0,0);
							}
						}
					    delete newtag;
					    break;
				    }
				    case FT_KADLASTPUBLISHNOTES:{
						ASSERT( newtag->IsInt() );
						if (newtag->IsInt())
						{
						    SetLastPublishTimeKadNotes(newtag->GetInt());
						}
					    delete newtag;
					    break;
				    }
                    case FT_DL_PREVIEW:{
                        ASSERT( newtag->IsInt() );
						SetPreviewPrio(((newtag->GetInt() >>  0) & 0x01) == 1);
						SetPauseOnPreview(((newtag->GetInt() >>  1) & 0x01) == 1);
                        delete newtag;
                        break;
                    }

				   // statistics
					case FT_ATTRANSFERRED:{
						ASSERT( newtag->IsInt() );
						if (newtag->IsInt())
							statistic.SetAllTimeTransferred(newtag->GetInt());
						delete newtag;
						break;
					}
					case FT_ATTRANSFERREDHI:{
						ASSERT( newtag->IsInt() );
						if (newtag->IsInt())
						{
							uint32_t hi,low;
							low = (UINT)statistic.GetAllTimeTransferred();
							hi = newtag->GetInt();
							uint64_t hi2;
							hi2=hi;
							hi2=hi2<<32;
							statistic.SetAllTimeTransferred(low+hi2);
						}
						delete newtag;
						break;
					}
					case FT_ATREQUESTED:{
						ASSERT( newtag->IsInt() );
						if (newtag->IsInt())
							statistic.SetAllTimeRequests(newtag->GetInt());
						delete newtag;
						break;
					}
 					case FT_ATACCEPTED:{
						ASSERT( newtag->IsInt() );
						if (newtag->IsInt())
							statistic.SetAllTimeAccepts(newtag->GetInt());
						delete newtag;
						break;
					}

					// old tags: as long as they are not needed, take the chance to purge them
					case FT_PERMISSIONS:
						ASSERT( newtag->IsInt() );
						delete newtag;
						break;
					case FT_KADLASTPUBLISHKEY:
						ASSERT( newtag->IsInt() );
						delete newtag;
						break;
					case FT_DL_ACTIVE_TIME:
						ASSERT( newtag->IsInt() );
						if (newtag->IsInt())
							m_nDlActiveTime = newtag->GetInt();
						delete newtag;
						break;
					case FT_CORRUPTEDPARTS:
						ASSERT( newtag->IsStr() );
						if (newtag->IsStr())
						{
							ASSERT( corrupted_list.GetHeadPosition() == NULL );
							CString strCorruptedParts(newtag->GetStr());
							int iPos = 0;
							CString strPart = strCorruptedParts.Tokenize(_T(","), iPos);
							while (!strPart.IsEmpty())
							{
								UINT uPart;
								if (_stscanf_s(strPart, _T("%u"), &uPart) == 1)
								{
									if (uPart < partCount() && !IsCorruptedPart(uPart))
										corrupted_list.AddTail((uint16_t)uPart);
								}
								strPart = strCorruptedParts.Tokenize(_T(","), iPos);
							}
						}
						delete newtag;
						break;
					case FT_AICH_HASH:{
                        // ismod: no aich
						delete newtag;
						break;
					}
				    default:{
					    if (newtag->GetNameID()==0 && (newtag->GetName()[0]==FT_GAPSTART || newtag->GetName()[0]==FT_GAPEND))
						{
							ASSERT( newtag->IsInt64(true) );
							if (newtag->IsInt64(true))
							{
								Gap_Struct* gap;
								UINT gapkey = atoi(&newtag->GetName()[1]);
								if (!gap_map.Lookup(gapkey, gap))
								{
									gap = new Gap_Struct;
									gap_map.SetAt(gapkey, gap);
									gap->start = (uint64_t)-1;
									gap->end = (uint64_t)-1;
								}
								if (newtag->GetName()[0] == FT_GAPSTART)
									gap->start = newtag->GetInt64();
								if (newtag->GetName()[0] == FT_GAPEND)
									gap->end = newtag->GetInt64() - 1;
							}
						    delete newtag;
					    }
					    else
						    taglist.Add(newtag);
				    }
				}
			}
			else
				delete newtag;
		}

		// load the hashsets from the hybridstylepartmet
		if (isnewstyle && pOutCheckFileFormat == NULL && (metFile.GetPosition()<metFile.GetLength()) ) {
			uint8_t temp;
			metFile.Read(&temp,1);
			
			UINT parts = partCount();	// assuming we will get all hashsets
			
			m_hasharray.allocate(parts);

			for (UINT i = 0; i < parts && (metFile.GetPosition() + 16 < metFile.GetLength()); i++){
				metFile.Read(m_hasharray.append_v(), 16);
			}
			
			if ( !m_hasharray.empty() )
			{
				uchar checkhash[16];
				CreateHash(m_hasharray.data(), m_hasharray.count()*m_hasharray.hashSize(), checkhash);

				if ( md4cmp(m_abyFileHash, checkhash) ) // m_abyFileHash != checkhash
				{
					m_hasharray.release();
				}
			}
		}

		metFile.Close();
	}
	catch(CFileException* error){
		if (error->m_cause == CFileException::endOfFile)
			LogError(LOG_STATUSBAR, GetResString(IDS_ERR_METCORRUPT), m_partmetfilename, GetFileName());
		else{
			TCHAR buffer[MAX_CFEXP_ERRORMSG];
			error->GetErrorMessage(buffer,_countof(buffer));
			LogError(LOG_STATUSBAR, GetResString(IDS_ERR_FILEERROR), m_partmetfilename, GetFileName(), buffer);
		}
		error->Delete();
		return PLR_FAILED_METFILE_CORRUPT;
	}
#ifndef _DEBUG
	catch(...){
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_METCORRUPT), m_partmetfilename, GetFileName());
		ASSERT(0);
		return PLR_FAILED_METFILE_CORRUPT;
	}
#endif

	if (m_nFileSize > (uint64_t)MAX_EMULE_FILE_SIZE) {
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_FILEERROR), m_partmetfilename, GetFileName(), _T("File size exceeds supported limit"));
		return PLR_FAILED_OTHER;
	}

	if (pOutCheckFileFormat != NULL)
	{
		// AAARGGGHH!!!....
		*pOutCheckFileFormat = partmettype;
		return PLR_CHECKSUCCESS;
	}

	// ismod: prepare parts info
	m_PartsInfo.SetSize(partCount());
	
	// Now to flush the map into the list (Slugfiller)
	for (POSITION pos = gap_map.GetStartPosition(); pos != NULL; ){
		Gap_Struct* gap;
		UINT gapkey;
		gap_map.GetNextAssoc(pos, gapkey, gap);
		// BEGIN SLUGFILLER: SafeHash - revised code, and extra safety
		if (gap->start != -1 && gap->end != -1 && gap->start <= gap->end && gap->start < m_nFileSize){
			if (gap->end >= m_nFileSize)
				gap->end = m_nFileSize - (uint64_t)1; // Clipping
			AddGap(gap->start, gap->end); // All tags accounted for, use safe adding
		}
		delete gap;
		// END SLUGFILLER: SafeHash
	}

    // mark completed parts
	for (uint16_t i = 0; i < partCount(); i++) {
		if (IsComplete((uint64_t)i*PARTSIZE,(uint64_t)(i+1)*PARTSIZE-1, true)) {
            m_PartsInfo[i].isComplete = true;
			m_PartsInfo[i].isChecked = true;
		}
	}

	// verify corrupted parts list
	POSITION posCorruptedPart = corrupted_list.GetHeadPosition();
	while (posCorruptedPart)
	{
		POSITION posLast = posCorruptedPart;
		uint16_t uCorruptedPart = corrupted_list.GetNext(posCorruptedPart);
		if ( IsPartChecked(uCorruptedPart) )
			corrupted_list.RemoveAt(posLast);
	}

	//check if this is a backup
	// BEGIN SLUGFILLER: SafeHash - also update the partial name
	if(_tcsicmp(_tcsrchr(m_fullname, _T('.')), PARTMET_TMP_EXT) == 0 || _tcsicmp(_tcsrchr(m_fullname, _T('.')), PARTMET_BAK_EXT) == 0)
	{
		m_fullname = RemoveFileExtension(m_fullname);
		m_partmetfilename = RemoveFileExtension(m_partmetfilename);
	}
	// END SLUGFILLER: SafeHash

	// open permanent handle
	CString searchpath(RemoveFileExtension(m_fullname));
	ASSERT( searchpath.Right(5) == _T(".part") );
	CFileException fexpPart;
	if (!m_hpartfile.Open(searchpath, CFile::modeReadWrite|CFile::shareDenyWrite|CFile::osSequentialScan, &fexpPart)){
		CString strError;
		strError.Format(GetResString(IDS_ERR_FILEOPEN), searchpath, GetFileName());
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		if (fexpPart.GetErrorMessage(szError, _countof(szError))){
			strError += _T(" - ");
			strError += szError;
		}
		LogError(LOG_STATUSBAR, _T("%s"), strError);
		return PLR_FAILED_OTHER;
	}

	// read part file creation time
	struct _stat fileinfo;
	if (_tstat(searchpath, &fileinfo) == 0){
		m_tLastModified = fileinfo.st_mtime;
		m_tCreated = fileinfo.st_ctime;
	}
	else
		AddDebugLogLine(false, _T("Failed to get file date for \"%s\" - %s"), searchpath, _tcserror(errno));

	try{
		SetFilePath(searchpath);
		m_dwFileAttributes = GetFileAttributes(GetFilePath());
		if (m_dwFileAttributes == INVALID_FILE_ATTRIBUTES)
			m_dwFileAttributes = 0;

		// BEGIN SLUGFILLER: SafeHash - final safety, make sure any missing part of the file is gap
		if (m_hpartfile.GetLength() < m_nFileSize)
			AddGap(m_hpartfile.GetLength(), m_nFileSize - (uint64_t)1);
		// Goes both ways - Partfile should never be too large
		if (m_hpartfile.GetLength() > m_nFileSize){
			TRACE(_T("Partfile \"%s\" is too large! Truncating %I64u bytes.\n"), GetFileName(), m_hpartfile.GetLength() - m_nFileSize);
			m_hpartfile.SetLength(m_nFileSize);
		}
		// END SLUGFILLER: SafeHash

		// BEGIN SLUGFILLER: SafeHash - ignore loaded hash for 1-chunk files
		if (GetED2KPartCount() <= 1) {
			m_hasharray.allocate(1);
			md4cpy(m_hasharray.append_v(), m_abyFileHash);
		}

		SetStatus(PS_EMPTY);
		// check hashcount, filesatus etc
		if (hashset().count() != GetED2KPartCount()){	// SLUGFILLER: SafeHash - use GetED2KPartCount
			ASSERT( hashset().count() == 0 );
			m_eHashsetState = PFHS_REQUIRED;
			//hashsetneeded = true;
			return PLR_LOADSUCCESS;
		}
		else {
			m_eHashsetState = PFHS_OK;
			//hashsetneeded = false;
			for (uint16_t i = 0; i < (uint16_t)m_hasharray.count(); i++){
				if (i < partCount() && IsPartChecked(i)){
					SetStatus(PS_READY);
					break;
				}
			}
		}

		if ( gaplist.empty() ) {	// is this file complete already?
			CompleteFile(false);
			return PLR_LOADSUCCESS;
		}

		if (!isnewstyle) // not for importing
		{
			// check date of .part file - if its wrong, rehash file
			CFileStatus filestatus;
			try{
				m_hpartfile.GetStatus(filestatus); // this; "...returns m_attribute without high-order flags" indicates a known MFC bug, wonder how many unknown there are... :)
			}
			catch(CException* ex){
				ex->Delete();
			}
			time_t fdate = filestatus.m_mtime.GetTime();
			if (fdate == 0)
				fdate = -1;
			if (fdate == -1){
				if (thePrefs.GetVerbose())
					AddDebugLogLine(false, _T("Failed to get file date of \"%s\" (%s)"), filestatus.m_szFullName, GetFileName());
			}
			else
				AdjustNTFSDaylightFileTime(fdate, filestatus.m_szFullName);

			if (m_tUtcLastModified != fdate){
				CString strFileInfo;
				strFileInfo.Format(_T("%s (%s)"), GetFilePath(), GetFileName());
				LogError(LOG_STATUSBAR, GetResString(IDS_ERR_REHASH), strFileInfo);
				// rehash
				// BEGIN SLUGFILLER: SafeHash
				SetStatus(PS_EMPTY);	// no need to wait for hashes with the new system
				HashCompleteParts();	// Only hashes completed parts, why hash gaps?
				// END SLUGFILLER: SafeHash
		    }
		}
	}
	catch(CFileException* error){
		CString strError;
		strError.Format(_T("Failed to initialize part file \"%s\" (%s)"), m_hpartfile.GetFilePath(), GetFileName());
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		if (error->GetErrorMessage(szError, _countof(szError))){
			strError += _T(" - ");
			strError += szError;
		}
		LogError(LOG_STATUSBAR, _T("%s"), strError);
		error->Delete();
		return PLR_FAILED_OTHER;
	}

	UpdateCompletedInfos();
	return PLR_LOADSUCCESS;
}

bool CPartFile::SavePartFile(bool bDontOverrideBak)
{
	switch (status){
		case PS_WAITINGFORHASH:
		case PS_HASHING:
			return false;
	}

	// search part file
	CFileFind ff;
	CString searchpath(RemoveFileExtension(m_fullname));
	bool end = !ff.FindFile(searchpath,0);
	if (!end)
		ff.FindNextFile();
	if (end || ff.IsDirectory()){
		LogError(GetResString(IDS_ERR_SAVEMET) + _T(" - %s"), m_partmetfilename, GetFileName(), GetResString(IDS_ERR_PART_FNF));
		return false;
	}

	//get filedate
	CTime lwtime;
	try{
		ff.GetLastWriteTime(lwtime);
	}
	catch(CException* ex){
		ex->Delete();
	}
	m_tLastModified = (UINT)lwtime.GetTime();
	if (m_tLastModified == 0)
		m_tLastModified = (UINT)-1;
	m_tUtcLastModified = m_tLastModified;
	if (m_tUtcLastModified == -1){
		if (thePrefs.GetVerbose())
			AddDebugLogLine(false, _T("Failed to get file date of \"%s\" (%s)"), m_partmetfilename, GetFileName());
	}
	else
		AdjustNTFSDaylightFileTime(m_tUtcLastModified, ff.GetFilePath());

	ff.Close();

	CString strTmpFile(m_fullname);
	strTmpFile += PARTMET_TMP_EXT;

	// save file data to part.met file
	CSafeBufferedFile file;
	CFileException fexp;
	if (!file.Open(strTmpFile, CFile::modeWrite|CFile::modeCreate|CFile::typeBinary|CFile::shareDenyWrite, &fexp)){
		CString strError;
		strError.Format(GetResString(IDS_ERR_SAVEMET), m_partmetfilename, GetFileName());
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		if (fexp.GetErrorMessage(szError, _countof(szError))){
			strError += _T(" - ");
			strError += szError;
		}
		LogError(_T("%s"), strError);
		return false;
	}
	setvbuf(file.m_pStream, NULL, _IOFBF, 16384);

	try{
		//version
		// only use 64 bit tags, when PARTFILE_VERSION_LARGEFILE is set!
		file.WriteUInt8( IsLargeFile()? PARTFILE_VERSION_LARGEFILE : PARTFILE_VERSION);

		//date
		file.WriteUInt32(ConvertToTime32(m_tUtcLastModified));

		//hash
		file.WriteHash16(m_abyFileHash);
		file.WriteUInt16(static_cast<uint16_t>(m_hasharray.count()));
		if ( m_hasharray.count() != 0 )
			file.Write(m_hasharray.data(), m_hasharray.hashSize()*m_hasharray.count());

		UINT uTagCount = 0;
		ULONG uTagCountFilePos = (ULONG)file.GetPosition();
		file.WriteUInt32(uTagCount);

		CTag nametag(FT_FILENAME, GetFileName());
		nametag.WriteTagToFile(&file, utf8strOptBOM);
		uTagCount++;

		CTag sizetag(FT_FILESIZE, m_nFileSize, IsLargeFile());
		sizetag.WriteTagToFile(&file);
		uTagCount++;

		if (m_uTransferred){
			CTag transtag(FT_TRANSFERRED, m_uTransferred, IsLargeFile());
			transtag.WriteTagToFile(&file);
			uTagCount++;
		}
		if (m_uCompressionGain){
			CTag transtag(FT_COMPRESSION, m_uCompressionGain, IsLargeFile());
			transtag.WriteTagToFile(&file);
			uTagCount++;
		}
		if (m_uCorruptionLoss){
			CTag transtag(FT_CORRUPTED, m_uCorruptionLoss, IsLargeFile());
			transtag.WriteTagToFile(&file);
			uTagCount++;
		}

		if (paused){
			CTag statustag(FT_STATUS, 1);
			statustag.WriteTagToFile(&file);
			uTagCount++;
		}

		CTag prioritytag(FT_DLPRIORITY, IsAutoDownPriority() ? PR_AUTO : m_iDownPriority);
		prioritytag.WriteTagToFile(&file);
		uTagCount++;

		CTag ulprioritytag(FT_ULPRIORITY, IsAutoUpPriority() ? PR_AUTO : GetUpPriority());
		ulprioritytag.WriteTagToFile(&file);
		uTagCount++;

		if (lastseencomplete.GetTime()){
			CTag lsctag(FT_LASTSEENCOMPLETE, (UINT)lastseencomplete.GetTime());
			lsctag.WriteTagToFile(&file);
			uTagCount++;
		}

		if (m_category){
			CTag categorytag(FT_CATEGORY, m_category);
			categorytag.WriteTagToFile(&file);
			uTagCount++;
		}

		if (GetLastPublishTimeKadSrc()){
			CTag kadLastPubSrc(FT_KADLASTPUBLISHSRC, GetLastPublishTimeKadSrc());
			kadLastPubSrc.WriteTagToFile(&file);
			uTagCount++;
		}

		if (GetLastPublishTimeKadNotes()){
			CTag kadLastPubNotes(FT_KADLASTPUBLISHNOTES, GetLastPublishTimeKadNotes());
			kadLastPubNotes.WriteTagToFile(&file);
			uTagCount++;
		}

		if (GetDlActiveTime()){
			CTag tagDlActiveTime(FT_DL_ACTIVE_TIME, GetDlActiveTime());
			tagDlActiveTime.WriteTagToFile(&file);
			uTagCount++;
		}

        if (GetPreviewPrio() || IsPausingOnPreview()){
			UINT uTagValue = ((IsPausingOnPreview() ? 1 : 0) <<  1) | ((GetPreviewPrio() ? 1 : 0) <<  0);
            CTag tagDlPreview(FT_DL_PREVIEW, uTagValue);
			tagDlPreview.WriteTagToFile(&file);
			uTagCount++;
		}

		// statistics
		if (statistic.GetAllTimeTransferred()){
			CTag attag1(FT_ATTRANSFERRED, (uint32_t)statistic.GetAllTimeTransferred());
			attag1.WriteTagToFile(&file);
			uTagCount++;
			
			CTag attag4(FT_ATTRANSFERREDHI, (uint32_t)(statistic.GetAllTimeTransferred() >> 32));
			attag4.WriteTagToFile(&file);
			uTagCount++;
		}

		if (statistic.GetAllTimeRequests()){
			CTag attag2(FT_ATREQUESTED, statistic.GetAllTimeRequests());
			attag2.WriteTagToFile(&file);
			uTagCount++;
		}
		
		if (statistic.GetAllTimeAccepts()){
			CTag attag3(FT_ATACCEPTED, statistic.GetAllTimeAccepts());
			attag3.WriteTagToFile(&file);
			uTagCount++;
		}

		if (m_uMaxSources){
			CTag attag3(FT_MAXSOURCES, m_uMaxSources);
			attag3.WriteTagToFile(&file);
			uTagCount++;
		}

		// currupt part infos
        POSITION posCorruptedPart = corrupted_list.GetHeadPosition();
		if (posCorruptedPart)
		{
			CString strCorruptedParts;
			while (posCorruptedPart)
			{
				UINT uCorruptedPart = corrupted_list.GetNext(posCorruptedPart);
				if (!strCorruptedParts.IsEmpty())
					strCorruptedParts += _T(",");
				strCorruptedParts.AppendFormat(_T("%u"), (UINT)uCorruptedPart);
			}
			ASSERT( !strCorruptedParts.IsEmpty() );
			CTag tagCorruptedParts(FT_CORRUPTEDPARTS, strCorruptedParts);
			tagCorruptedParts.WriteTagToFile(&file);
			uTagCount++;
		}

        // ismod: no aich
		for (int j = 0; j < taglist.GetCount(); j++){
			if (taglist[j]->IsStr() || taglist[j]->IsInt()){
				taglist[j]->WriteTagToFile(&file, utf8strOptBOM);
				uTagCount++;
			}
		}

		//gaps
		char namebuffer[10];
		char* number = &namebuffer[1];
		UINT i_pos = 0;
		for ( GapList::iterator it = gaplist.begin(); it != gaplist.end(); ++it )
		{
			Gap_Struct* gap = *it;
			_itoa_s(i_pos, number, _countof(namebuffer) - 1, 10);
			namebuffer[0] = FT_GAPSTART;
			CTag gapstarttag(namebuffer,gap->start, IsLargeFile());
			gapstarttag.WriteTagToFile(&file);
			uTagCount++;

			// gap start = first missing byte but gap ends = first non-missing byte in edonkey
			// but I think its easier to user the real limits
			namebuffer[0] = FT_GAPEND;
			CTag gapendtag(namebuffer,gap->end+1, IsLargeFile());
			gapendtag.WriteTagToFile(&file);
			uTagCount++;
			
			i_pos++;
		}

		file.Seek(uTagCountFilePos, CFile::begin);
		file.WriteUInt32(uTagCount);
		file.SeekToEnd();

		if (thePrefs.GetCommitFiles() >= 2 || (thePrefs.GetCommitFiles() >= 1 && !theApp.IsRunning())){
			file.Flush(); // flush file stream buffers to disk buffers
			if (_commit(_fileno(file.m_pStream)) != 0) // commit disk buffers to disk
				AfxThrowFileException(CFileException::hardIO, GetLastError(), file.GetFileName());
		}
		file.Close();
	}
	catch(CFileException* error){
		CString strError;
		strError.Format(GetResString(IDS_ERR_SAVEMET), m_partmetfilename, GetFileName());
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		if (error->GetErrorMessage(szError, _countof(szError))){
			strError += _T(" - ");
			strError += szError;
		}
		LogError(_T("%s"), strError);
		error->Delete();

		// remove the partially written or otherwise damaged temporary file
		file.Abort(); // need to close the file before removing it. call 'Abort' instead of 'Close', just to avoid an ASSERT.
		(void)_tremove(strTmpFile);
		return false;
	}

	// after successfully writing the temporary part.met file...
	if (_tremove(m_fullname) != 0 && errno != ENOENT){
		if (thePrefs.GetVerbose())
			DebugLogError(_T("Failed to remove \"%s\" - %s"), m_fullname, _tcserror(errno));
	}

	if (_trename(strTmpFile, m_fullname) != 0){
		int iErrno = errno;
		if (thePrefs.GetVerbose())
			DebugLogError(_T("Failed to move temporary part.met file \"%s\" to \"%s\" - %s"), strTmpFile, m_fullname, _tcserror(iErrno));

		CString strError;
		strError.Format(GetResString(IDS_ERR_SAVEMET), m_partmetfilename, GetFileName());
		strError += _T(" - ");
		strError += _tcserror(iErrno);
		LogError(_T("%s"), strError);
		return false;
	}

	// create a backup of the successfully written part.met file
	CString BAKName(m_fullname);
	BAKName.Append(PARTMET_BAK_EXT);
	if (!::CopyFile(m_fullname, BAKName, bDontOverrideBak ? TRUE : FALSE)){
		if (!bDontOverrideBak)
			DebugLogError(_T("Failed to create backup of %s (%s) - %s"), m_fullname, GetFileName(), GetErrorMessage(GetLastError()));
	}

	return true;
}

void CPartFile::PartFileHashFinished( CKnownFile* result )
{
	newdate = true;
    bool errorfound = false;

	// file < PARTSIZE
	if ( GetED2KPartCount() == 0 ) {
		if ( IsPartComplete(0) ) {
			if ( md4cmp(result->GetFileHash(), GetFileHash()) ) {
				LogWarning(GetResString(IDS_ERR_FOUNDCORRUPTION), 1, GetFileName());
				AddGap(0, m_nFileSize - 1ui64);
				errorfound = true;
			} else {
				if ( GetED2KPartCount() != hashset().count() ) {
					ASSERT( result->GetED2KPartCount() == GetED2KPartCount() );
					if ( setHashset(result->hashset()) )
						m_eHashsetState = PFHS_OK;
				}
			}
		}
	} else {
		for ( uint16_t i = 0; i < hashset().count(); i++ ) {
			if ( i < partCount() && IsPartComplete(i) ) {
				if ( md4cmp(result->hashset(i), hashset(i)) ) {
					LogWarning(GetResString(IDS_ERR_FOUNDCORRUPTION), i+1, GetFileName());
					AddGap((uint64_t)i*PARTSIZE, ((uint64_t)((uint64_t)(i + 1)*PARTSIZE - 1) >= m_nFileSize) ? ((uint64_t)m_nFileSize - 1) : ((uint64_t)(i + 1)*PARTSIZE - 1) );
					errorfound = true;
				}
			}
		}
	}
	// ismod: no acih

	delete result;
	if ( !errorfound ) {
		if ( status == PS_COMPLETING ) {
			if (thePrefs.GetVerbose())
				AddDebugLogLine(true, _T("Completed file-hashing for \"%s\""), GetFileName());
			if ( theApp.sharedfiles->GetFileByID(GetFileHash()) == NULL )
				theApp.sharedfiles->SafeAddKFile(this);
			CompleteFile(true);
			return;
		} else
			AddLogLine(false, GetResString(IDS_HASHINGDONE), GetFileName());
	} else {
		SetStatus(PS_READY);
		if ( thePrefs.GetVerbose() )
			DebugLogError(LOG_STATUSBAR, _T("File-hashing failed for \"%s\""), GetFileName());
		SavePartFile();
		return;
	}

	if (thePrefs.GetVerbose())
		AddDebugLogLine(true, _T("Completed file-hashing for \"%s\""), GetFileName());
	SetStatus(PS_READY);
	SavePartFile();
	theApp.sharedfiles->SafeAddKFile(this);
}

void CPartFile::AddGap(uint64_t start, uint64_t end)
{
	ASSERT( start <= end );

	for ( GapList::iterator it = gaplist.begin(); it != gaplist.end(); ) {
		Gap_Struct* cur_gap = *it;
		if ( cur_gap->start >= start && cur_gap->end <= end ) { // this gap is inside the new gap - delete
			it = gaplist.erase(it);
			delete cur_gap;
		}
		else if ( cur_gap->start >= start && cur_gap->start <= end ) { // a part of this gap is in the new gap - extend limit and delete
			end = cur_gap->end;
			it = gaplist.erase(it);
			delete cur_gap;
		}
		else if ( cur_gap->end <= end && cur_gap->end >= start ) { // a part of this gap is in the new gap - extend limit and delete
			start = cur_gap->start;
			it = gaplist.erase(it);
			delete cur_gap;
		}
		else if (start >= cur_gap->start && end <= cur_gap->end ) { // new gap is already inside this gap - return
			return;
		} else {
			++it;
		}
	}
	Gap_Struct* new_gap = new Gap_Struct;
	new_gap->start = start;
	new_gap->end = end;
	gaplist.push_back(new_gap);
	UpdateDisplayedInfo();
	newdate = true;
    // ismod: update part info [start]
	uint16_t startPart = (uint16_t)(start/PARTSIZE);
	uint16_t endPart = (uint16_t)(end/PARTSIZE);
	for (uint16_t i=startPart; i<=endPart; ++i) {
		m_PartsInfo[i].isComplete = false;
		m_PartsInfo[i].isChecked = false;
	}
}

bool CPartFile::IsComplete(uint64_t start, uint64_t end, bool bIgnoreBufferedData) const
{
	ASSERT( start <= end );

	if ( end >= m_nFileSize ) {
		end = (uint64_t)m_nFileSize - 1;
	}

	for ( GapList::const_iterator it = gaplist.begin(); it != gaplist.end(); ++it )
	{
		const Gap_Struct* cur_gap = *it;
		if (   (cur_gap->start >= start          && cur_gap->end   <= end)
			|| (cur_gap->start >= start          && cur_gap->start <= end)
			|| (cur_gap->end   <= end            && cur_gap->end   >= start)
			|| (start          >= cur_gap->start && end            <= cur_gap->end)
		   )
		{
			return false;	
		}
	}

	if ( bIgnoreBufferedData )
		return true;

	for ( PartFileBufferedDataList::const_iterator it = m_BufferedData_list.begin();  it != m_BufferedData_list.end(); ++it )
	{
		const PartFileBufferedData* cur_gap = *it;
		if (   (cur_gap->start >= start          && cur_gap->end   <= end)
			|| (cur_gap->start >= start          && cur_gap->start <= end)
			|| (cur_gap->end   <= end            && cur_gap->end   >= start)
			|| (start          >= cur_gap->start && end            <= cur_gap->end)
		)
		{
			return false;	
		}
	}

	return true;
}

bool CPartFile::IsPureGap(uint64_t start, uint64_t end) const
{
	ASSERT( start <= end );
	ASSERT( m_nFileSize > end );

	for ( GapList::const_iterator it = gaplist.begin(); it != gaplist.end(); ++it ) {
		const Gap_Struct* cur_gap = *it;
		if (start >= cur_gap->start  && end <= cur_gap->end ){
			return true;
		}
	}
	return false;
}

bool CPartFile::IsAlreadyRequested( uint64_t start, uint64_t end ) const
{
	ASSERT( start <= end );
	ASSERT( m_nFileSize > end );

	// check our requestlist
	for ( RequestedBlockList::const_iterator it = requestedblocks_list.begin(); it != requestedblocks_list.end(); ++it ) {
		const Requested_Block_Struct* cur_block = *it;
		if ((start <= cur_block->EndOffset) && (end >= cur_block->StartOffset))
			return true;
	}

	return false;
}

bool CPartFile::ShrinkToAvoidAlreadyRequested( uint64_t &start, uint64_t &end ) const
{
	ASSERT( start <= end );
#ifdef _DEBUG
    uint64_t startOrig = start;
    uint64_t endOrig = end;
#endif
	for ( RequestedBlockList::const_iterator it = requestedblocks_list.begin(); it != requestedblocks_list.end(); ++it ) {
		const Requested_Block_Struct* cur_block = *it;
        if ( (start <= cur_block->EndOffset) && (end >= cur_block->StartOffset) ) {
            if( start < cur_block->StartOffset ) {
                end = cur_block->StartOffset - 1;
            } else if ( end > cur_block->EndOffset ) {
                start = cur_block->EndOffset + 1;
            } else {
                return false;
            }
        }
	}

    ASSERT(start >= startOrig && start <= endOrig);
    ASSERT(end >= startOrig && end <= endOrig);

	return true;
}

uint64_t CPartFile::GetTotalGapSizeInRange(uint64_t uRangeStart, uint64_t uRangeEnd) const
{
	ASSERT( uRangeStart <= uRangeEnd );

	uint64_t uTotalGapSize = 0;

	if (uRangeEnd >= m_nFileSize)
		uRangeEnd = m_nFileSize - (uint64_t)1;

	for ( GapList::const_iterator it = gaplist.begin(); it != gaplist.end(); )
	{
		const Gap_Struct* pGap = *it;

		if (pGap->start < uRangeStart && pGap->end > uRangeEnd)
		{
			uTotalGapSize += uRangeEnd - uRangeStart + 1;
			break;
		}

		if (pGap->start >= uRangeStart && pGap->start <= uRangeEnd)
		{
			uint64_t uEnd = (pGap->end > uRangeEnd) ? uRangeEnd : pGap->end;
			uTotalGapSize += uEnd - pGap->start + 1;
		}
		else if (pGap->end >= uRangeStart && pGap->end <= uRangeEnd)
		{
			uTotalGapSize += pGap->end - uRangeStart + 1;
		}
	}

	ASSERT( uTotalGapSize <= uRangeEnd - uRangeStart + 1 );

	return uTotalGapSize;
}

uint64_t CPartFile::GetTotalGapSizeInPart(UINT uPart) const
{
	uint64_t uRangeStart = (uint64_t)uPart * PARTSIZE;
	uint64_t uRangeEnd = uRangeStart + PARTSIZE - 1;
	if (uRangeEnd >= m_nFileSize)
		uRangeEnd = m_nFileSize;
	return GetTotalGapSizeInRange(uRangeStart, uRangeEnd);
}

bool CPartFile::GetNextEmptyBlockInPart(UINT partNumber, Requested_Block_Struct *result, uint32_t blockSize) const
{
	Gap_Struct *firstGap;
	Gap_Struct *currentGap;
	uint64_t end;
	uint64_t blockLimit;

	// Find start of this part
	uint64_t partStart = PARTSIZE * (uint64_t)partNumber;
	uint64_t start = partStart;

	// What is the end limit of this block, i.e. can't go outside part (or filesize)
	uint64_t partEnd = PARTSIZE * (uint64_t)(partNumber + 1) - 1;
	if (partEnd >= GetFileSize())
		partEnd = GetFileSize() - (uint64_t)1;
	ASSERT( partStart <= partEnd );

	// Loop until find a suitable gap and return true, or no more gaps and return false
	for (;;)
	{
		firstGap = NULL;

		// Find the first gap from the start position
		for ( GapList::const_iterator it = gaplist.begin(); it != gaplist.end(); ++it )
		{
			currentGap = *it;
			// Want gaps that overlap start<->partEnd
			if ((currentGap->start <= partEnd) && (currentGap->end >= start))
			{
				// Is this the first gap?
				if ((firstGap == NULL) || (currentGap->start < firstGap->start))
					firstGap = currentGap;
			}
		}

		// If no gaps after start, exit
		if (firstGap == NULL)
			return false;

		// Update start position if gap starts after current pos
		if (start < firstGap->start)
			start = firstGap->start;

		// If this is not within part, exit
		if (start > partEnd)
			return false;

		// Find end, keeping within the max block size and the part limit
		end = firstGap->end;

        blockLimit = partStart + (uint64_t)((UINT)(start - partStart)/EMBLOCKSIZE + 1)*EMBLOCKSIZE - 1;
		if (end > blockLimit)
			end = blockLimit;

        if (end > partEnd)
			end = partEnd;
    
		// ismod: dynamic block size [start]
		if ( (end-start) > blockSize )
			end = start + blockSize - 1;
		// ismod: dynamic block size [start]

		// If this gap has not already been requested, we have found a valid entry
		if (!IsAlreadyRequested(start, end))
		{
			// Was this block to be returned
			if (result != NULL)
			{
				result->StartOffset = start;
				result->EndOffset = end;
				md4cpy(result->FileID, GetFileHash());
				result->transferred = 0;
			}
			return true;
		}
		else
		{
        	uint64_t tempStart = start;
        	uint64_t tempEnd = end;

			bool shrinkSucceeded = ShrinkToAvoidAlreadyRequested(tempStart, tempEnd);
			if(shrinkSucceeded) {
				//if (thePrefs.GetVerbose())
				//	AddDebugLogLine(false, _T("Shrunk interval to prevent collision with already requested block: Old interval %I64u-%I64u. New interval: %I64u-%I64u. File %s."), start, end, tempStart, tempEnd, GetFileName());

                // Was this block to be returned
			    if (result != NULL)
			    {
				    result->StartOffset = tempStart;
				    result->EndOffset = tempEnd;
				    md4cpy(result->FileID, GetFileHash());
			    }
			    return true;
            } else {
			    // Reposition to end of that gap
			    start = end + 1;
		    }
		}

		// If tried all gaps then break out of the loop
		if (end == partEnd)
			break;
	}

	// No suitable gap found
	return false;
}

void CPartFile::FillGap( uint64_t start, uint64_t end )
{
	ASSERT( start <= end );

	for ( GapList::iterator it = gaplist.begin(); it != gaplist.end(); ) {
		Gap_Struct* cur_gap = *it;
		if ( cur_gap->start >= start && cur_gap->end <= end ) { // our part fills this gap completly
			it = gaplist.erase(it);
			delete cur_gap;
		}
		else if ( cur_gap->start >= start && cur_gap->start <= end ) { // a part of this gap is in the part - set limit
			cur_gap->start = end+1;
			++it;
		}
		else if ( cur_gap->end <= end && cur_gap->end >= start ) { // a part of this gap is in the part - set limit
			cur_gap->end = start-1;
			++it;
		}
		else if ( start >= cur_gap->start && end <= cur_gap->end ) {
			uint64_t buffer = cur_gap->end;
			cur_gap->end = start-1;
			cur_gap = new Gap_Struct;
			cur_gap->start = end+1;
			cur_gap->end = buffer;
			gaplist.insert(it, cur_gap);
			break; // for
		} else {
			++it;
		}
	}

	UpdateCompletedInfos();
	UpdateDisplayedInfo();
	newdate = true;
}

void CPartFile::UpdateCompletedInfos()
{
   	uint64_t allgaps = 0; 

	for ( GapList::const_iterator it = gaplist.begin(); it != gaplist.end(); ++it) { 
		const Gap_Struct* cur_gap = *it;
		allgaps += cur_gap->end - cur_gap->start + 1;
	}

	UpdateCompletedInfos(allgaps);
}

void CPartFile::UpdateCompletedInfos(uint64_t uTotalGaps)
{
	if ( uTotalGaps > m_nFileSize ) {
		ASSERT(0);
		uTotalGaps = m_nFileSize;
	}

	if ( !gaplist.empty() || !requestedblocks_list.empty() ) { 
		// 'percentcompleted' is only used in GUI, round down to avoid showing "100%" in case 
		// we actually have only "99.9%"
		percentcompleted = (float)(floor((1.0 - (double)uTotalGaps/(uint64_t)m_nFileSize) * 1000.0) / 10.0);
		completedsize = m_nFileSize - uTotalGaps;
	} 
	else{
		percentcompleted = 100.0F;
		completedsize = m_nFileSize;
	}
}

void CPartFile::DrawShareStatusBar(CDC* dc, LPCRECT rect, bool onlygreyrect, bool bFlat) const
{
	if( !IsPartFile() )
	{
		CKnownFile::DrawShareStatusBar( dc, rect, onlygreyrect, bFlat );
		return;
	}

    const COLORREF crNotShared = RGB(224, 224, 224);
	s_ChunkBar.SetFileSize(GetFileSize());
	s_ChunkBar.SetHeight(rect->bottom - rect->top);
	s_ChunkBar.SetWidth(rect->right - rect->left);
	s_ChunkBar.Fill(crNotShared);

	if (!onlygreyrect){
    	const COLORREF crMissing = RGB(255, 0, 0);
		COLORREF crProgress;
		COLORREF crHave;
		COLORREF crPending;
        COLORREF crNooneAsked;
		if(bFlat) { 
			crProgress = RGB(0, 150, 0);
			crHave = RGB(0, 0, 0);
			crPending = RGB(255,208,0);
		    crNooneAsked = RGB(0, 0, 0);
		} else { 
			crProgress = RGB(0, 224, 0);
			crHave = RGB(104, 104, 104);
			crPending = RGB(255, 208, 0);
		    crNooneAsked = RGB(104, 104, 104);
		}
		for (UINT i = 0; i < partCount(); i++){
            if(IsComplete((uint64_t)i*PARTSIZE,((uint64_t)(i+1)*PARTSIZE)-1, true)) {
                if(GetStatus() != PS_PAUSED || m_ClientUploadList.GetSize() > 0 || m_nCompleteSourcesCountHi > 0) {
                    uint32_t frequency;
                    if(GetStatus() != PS_PAUSED && !m_PartsInfo.IsEmpty()) {
                        frequency = m_PartsInfo[i].frequency;
                    } else if(!m_AvailPartFrequency.empty()) {
                        frequency = max(m_AvailPartFrequency[i], m_nCompleteSourcesCountLo);
                    } else {
                        frequency = m_nCompleteSourcesCountLo;
                    }

    			    if(frequency > 0 ){
				        COLORREF color = RGB(0, (22*(frequency-1) >= 210) ? 0 : 210-(22*(frequency-1)), 255);
				        s_ChunkBar.FillRange(PARTSIZE*(uint64_t)(i),PARTSIZE*(uint64_t)(i+1),color);
                    } else {
			            s_ChunkBar.FillRange(PARTSIZE*(uint64_t)(i),PARTSIZE*(uint64_t)(i+1),crMissing);
                    }
                } else {
				    s_ChunkBar.FillRange(PARTSIZE*(uint64_t)(i),PARTSIZE*(uint64_t)(i+1),crNooneAsked);
                }
			}
		}
	}
   	s_ChunkBar.Draw(dc, rect->left, rect->top, bFlat); 
} 

void CPartFile::DrawSimpleStatusBar(CDC* dc, LPCRECT rect, bool /*bFlat*/) const
{
	EPartFileStatus eVirtualState = GetStatus();
	CRect rc(rect);
	COLORREF color;

	//draw black rect
	dc->SetDCPenColor(RGB(255,255,255)); // black
	dc->Rectangle(rc);
	rc.left++;
	rc.right--;
	rc.top++;
	rc.bottom--;

	if (status == PS_COMPLETE || status == PS_COMPLETING) {
		dc->FillSolidRect(rc, RGB(46,184,115));
	} else if (eVirtualState == PS_INSUFFICIENT || status == PS_ERROR) {
		int iOldBkColor = dc->SetBkColor(RGB(255, 255, 0));
		if (theApp.m_brushBackwardDiagonal.m_hObject)
			dc->FillRect(rect, &theApp.m_brushBackwardDiagonal);
		else
			dc->FillSolidRect(rect, RGB(255, 255, 0));
		dc->SetBkColor(iOldBkColor);
	} else {
		dc->FillSolidRect(rc, RGB(110,110,110));
		if( PS_PAUSED == status || srclist.GetCount() == 0 ) {
			color = RGB(224,0,0);
		} else {
			color = RGB(0,
						(210 - 22*(srclist.GetCount() - 1) <  0) ?  0 : 210 - 22*(srclist.GetCount() - 1),
						255);
		}
		rc.right = rc.left + (LONG)((double)rc.Width()*(double)completedsize/(double)m_nFileSize);
		dc->FillSolidRect(rc, color);
	}

	if (GetFileOp() != PFOP_NONE)
	{
		float blockpixel = (float)(rect->right - rect->left)/100.0F;
		CRect rcFileOpProgress;
		rcFileOpProgress.top = rect->top+1;
		rcFileOpProgress.bottom = rcFileOpProgress.top + 1;
		rcFileOpProgress.left = rect->left+1;
		rcFileOpProgress.right = rect->right-1;
		rcFileOpProgress.right = rcFileOpProgress.left + (UINT)(GetFileOpProgress()*blockpixel + 0.5F);
		dc->FillRect(&rcFileOpProgress, &CBrush(RGB(255,208,0)));
	}
}

void CPartFile::DrawStatusBar(CDC* dc, LPCRECT rect, bool bFlat) /*const*/
{
	COLORREF crProgress;
	COLORREF crProgressBk;
	COLORREF crHave;
	COLORREF crPending;
	COLORREF crMissing;
	EPartFileStatus eVirtualState = GetStatus();
	bool notgray = eVirtualState == PS_EMPTY || eVirtualState == PS_READY;

	if (bFlat)
		crProgress = RGB(0, 150, 0);
	else
		crProgress = RGB(0, 224, 0);
	crProgressBk = RGB(224, 224, 224);
	if (notgray) {
		crMissing = RGB(255, 0, 0);
		if (bFlat) {
			crHave = RGB(0, 0, 0);
			crPending = RGB(255, 208, 0);
		} else {
			crHave = RGB(104, 104, 104);
			crPending = RGB(255, 208, 0);
		}
	} else {
		crMissing = RGB(191, 64, 64);
		if (bFlat) {
			crHave = RGB(64, 64, 64);
			crPending = RGB(191, 168, 64);
		} else {
			crHave = RGB(116, 116, 116);
			crPending = RGB(191, 168, 64);
		}
	}

	s_ChunkBar.SetHeight(rect->bottom - rect->top);
	s_ChunkBar.SetWidth(rect->right - rect->left);
	s_ChunkBar.SetFileSize(m_nFileSize);
	s_ChunkBar.Fill(crHave);

	if (status == PS_COMPLETE || status == PS_COMPLETING)
	{
		s_ChunkBar.FillRange(0, m_nFileSize, crProgress);
		s_ChunkBar.Draw(dc, rect->left, rect->top, bFlat);
		percentcompleted = 100.0F;
		completedsize = m_nFileSize;
	}
	else if (eVirtualState == PS_INSUFFICIENT || status == PS_ERROR)
	{
		int iOldBkColor = dc->SetBkColor(RGB(255, 255, 0));
		if (theApp.m_brushBackwardDiagonal.m_hObject)
			dc->FillRect(rect, &theApp.m_brushBackwardDiagonal);
		else
			dc->FillSolidRect(rect, RGB(255, 255, 0));
		dc->SetBkColor(iOldBkColor);

		UpdateCompletedInfos();
	}
	else
	{
	    // red gaps
	    uint64_t allgaps = 0;
	    for ( GapList::const_iterator it = gaplist.begin(); it != gaplist.end(); ++it ) {
		    const Gap_Struct* cur_gap = *it;
		    allgaps += cur_gap->end - cur_gap->start + 1;
		    bool gapdone = false;
		    uint64_t gapstart = cur_gap->start;
		    uint64_t gapend = cur_gap->end;
		    for (UINT i = 0; i < partCount(); i++){
			    if (gapstart >= (uint64_t)i*PARTSIZE && gapstart <= (uint64_t)(i+1)*PARTSIZE - 1){ // is in this part?
				    if (gapend <= (uint64_t)(i+1)*PARTSIZE - 1)
					    gapdone = true;
				    else
					    gapend = (uint64_t)(i+1)*PARTSIZE - 1; // and next part
    
				    // paint
				    COLORREF color;
				    if (m_PartsInfo.GetCount() >= (INT_PTR)i && m_PartsInfo[(uint16_t)i].frequency)
				    {
						if (notgray)
							color = RGB(0,
										(210 - 22*(m_PartsInfo[(uint16_t)i].frequency - 1) <  0) ?  0 : 210 - 22*(m_PartsInfo[(uint16_t)i].frequency - 1),
										255);
						else
							color = RGB(64,
										(169 - 11*(m_PartsInfo[(uint16_t)i].frequency - 1) < 64) ? 64 : 169 - 11*(m_PartsInfo[(uint16_t)i].frequency - 1),
										191);
				    }
				    else
					    color = crMissing;
				    s_ChunkBar.FillRange(gapstart, gapend + 1, color);
    
				    if (gapdone) // finished?
					    break;
				    else{
					    gapstart = gapend + 1;
					    gapend = cur_gap->end;
				    }
			    }
		    }
	    }
    
	    // yellow pending parts
		for ( RequestedBlockList::const_iterator it = requestedblocks_list.begin(); it != requestedblocks_list.end(); ++it ) {
		    const Requested_Block_Struct* block = *it;
		    s_ChunkBar.FillRange(block->StartOffset + block->transferred, block->EndOffset + 1, crPending);
	    }
    
	    s_ChunkBar.Draw(dc, rect->left, rect->top, bFlat);
    
	    // green progress
	    float blockpixel = (float)(rect->right - rect->left)/(float)m_nFileSize;
	    RECT gaprect;
	    gaprect.top = rect->top;
	    gaprect.bottom = gaprect.top + PROGRESS_HEIGHT;
	    gaprect.left = rect->left;
    
	    if (!bFlat) {
		    s_LoadBar.SetWidth((int)( (uint64_t)(m_nFileSize - allgaps)*blockpixel + 0.5F));
		    s_LoadBar.Fill(crProgress);
		    s_LoadBar.Draw(dc, gaprect.left, gaprect.top, false);
	    } else {
		    gaprect.right = rect->left + (uint32_t)((uint64_t)(m_nFileSize - allgaps)*blockpixel + 0.5F);
		    dc->FillRect(&gaprect, &CBrush(crProgress));
		    //draw gray progress only if flat
		    gaprect.left = gaprect.right;
		    gaprect.right = rect->right;
		    dc->FillRect(&gaprect, &CBrush(crProgressBk));
	    }

		// draw checked parts line
		s_CheckBar.Reset();
		s_CheckBar.SetFileSize(m_nFileSize);
		s_CheckBar.SetWidth(rect->right - rect->left);

		for (uint16_t i=0; i < partCount(); ++i) {
			if ( !m_PartsInfo[i].isChecked ) {
					s_CheckBar.FillRange(i*PARTSIZE, (i+1)*PARTSIZE, RGB(255,0,0));
			}
		}
		s_CheckBar.Draw(dc, rect->left, rect->bottom-PROGRESS_HEIGHT+1, false);

    
	    UpdateCompletedInfos(allgaps);
    }

	// additionally show any file op progress (needed for PS_COMPLETING and PS_WAITINGFORHASH)
	if (GetFileOp() != PFOP_NONE)
	{
		float blockpixel = (float)(rect->right - rect->left)/100.0F;
		CRect rcFileOpProgress;
		rcFileOpProgress.top = rect->top;
		rcFileOpProgress.bottom = rcFileOpProgress.top + PROGRESS_HEIGHT;
		rcFileOpProgress.left = rect->left;
		if (!bFlat)
		{
			s_LoadBar.SetWidth((int)(GetFileOpProgress()*blockpixel + 0.5F));
			s_LoadBar.Fill(RGB(255,208,0));
			s_LoadBar.Draw(dc, rcFileOpProgress.left, rcFileOpProgress.top, false);
		}
		else
		{
			rcFileOpProgress.right = rcFileOpProgress.left + (UINT)(GetFileOpProgress()*blockpixel + 0.5F);
			dc->FillRect(&rcFileOpProgress, &CBrush(RGB(255,208,0)));
			rcFileOpProgress.left = rcFileOpProgress.right;
			rcFileOpProgress.right = rect->right;
			dc->FillRect(&rcFileOpProgress, &CBrush(crProgressBk));
		}
	}
}

void CPartFile::WritePartStatus(CSafeMemFile* file) const
{
	uint16_t uED2KPartCount = GetED2KPartCount();
	file->WriteUInt16((uint16_t)uED2KPartCount);
	
	uint16_t uPart = 0;
	while (uPart != uED2KPartCount)
	{
		uint8_t towrite = 0;
		for (uint16_t i = 0; i < 8; i++)
		{
			if (uPart < partCount() && IsPartChecked(uPart))	// SLUGFILLER: SafeHash
				towrite |= (1 << i);
			uPart++;
			if (uPart == uED2KPartCount)
				break;
		}
		file->WriteUInt8(towrite);
	}
}

int CPartFile::GetValidSourcesCount() const
{
	int counter = 0;
	for (POSITION pos = srclist.GetHeadPosition(); pos != NULL;){
		EDownloadState nDLState = srclist.GetNext(pos)->GetDownloadState();
		if (nDLState==DS_ONQUEUE || nDLState==DS_DOWNLOADING || nDLState==DS_CONNECTED || nDLState==DS_REMOTEQUEUEFULL)
			++counter;
	}
	return counter;
}

UINT CPartFile::GetNotCurrentSourcesCount() const
{
	UINT counter = 0;
	for (POSITION pos = srclist.GetHeadPosition(); pos != NULL;){
		EDownloadState nDLState = srclist.GetNext(pos)->GetDownloadState();
		if (nDLState!=DS_ONQUEUE && nDLState!=DS_DOWNLOADING)
			counter++;
	}
	return counter;
}

uint64_t CPartFile::GetNeededSpace() const
{
	if (m_hpartfile.GetLength() > GetFileSize())
		return 0;	// Shouldn't happen, but just in case
	return GetFileSize() - m_hpartfile.GetLength();
}

EPartFileStatus CPartFile::GetStatus(bool ignorepause) const
{
	if ((!paused && !insufficient) || status == PS_ERROR || status == PS_COMPLETING || status == PS_COMPLETE || ignorepause)
		return status;
	else if (paused)
		return PS_PAUSED;
	else
		return PS_INSUFFICIENT;
}

void CPartFile::AddDownloadingSource(CUpDownClient* client){
	POSITION pos = m_downloadingSourceList.Find(client); // to be sure
	if(pos == NULL){
		m_downloadingSourceList.AddTail(client);
		theApp.emuledlg->transferwnd->GetDownloadClientsList()->AddClient(client);
	}
}

void CPartFile::RemoveDownloadingSource(CUpDownClient* client){
	POSITION pos = m_downloadingSourceList.Find(client); // to be sure
	if(pos != NULL){
		m_downloadingSourceList.RemoveAt(pos);
		theApp.emuledlg->transferwnd->GetDownloadClientsList()->RemoveClient(client);
	}
}

uint32_t CPartFile::Process(uint32_t reducedownload, UINT icounter/*in percent*/)
{
#ifdef _DEBUG
	theApp.emuledlg->updateTransferDebug(this);
#endif

	if (thePrefs.m_iDbgHeap >= 2)
		ASSERT_VALID(this);

	UINT nOldTransSourceCount = GetSrcStatisticsValue(DS_DOWNLOADING);
	DWORD dwCurTick = ::GetTickCount();
	if (dwCurTick < m_nLastBufferFlushTime)
	{
		ASSERT( false );
		m_nLastBufferFlushTime = dwCurTick;
	}

	// If buffer size exceeds limit, or if not written within time limit, flush data
	if ((m_nTotalBufferData > thePrefs.GetFileBufferSize()) || ((dwCurTick - (DWORD) m_nLastBufferFlushTime) > thePrefs.GetFileBufferTimeLimit()))
	{
		// Avoid flushing while copying preview file
		if (!m_bPreviewing)
			FlushBuffer();
	}

	datarate = 0;

	// calculate datarate, set limit etc.
	if(icounter < 10)
	{
		uint32_t cur_datarate;
		for(POSITION pos = m_downloadingSourceList.GetHeadPosition();pos!=0;)
		{
			CUpDownClient* cur_src = m_downloadingSourceList.GetNext(pos);
			if (thePrefs.m_iDbgHeap >= 2)
				ASSERT_VALID( cur_src );
			if(cur_src && cur_src->GetDownloadState() == DS_DOWNLOADING)
			{
				ASSERT( cur_src->socket );
				if (cur_src->socket)
				{
					cur_src->CheckDownloadTimeout();
					// netfinity: Transfer Rate BlackBox - Calculation is done automaticaly
					cur_datarate = cur_src->GetDownloadDatarate();
					cur_src->UpdateDisplayedInfo();
					datarate+=cur_datarate;
					if(reducedownload)
					{
// BEGIN netfinity: Advanced download throttling
						//uint32 limit = reducedownload*cur_datarate/1000;
						uint32_t limit = max<uint32_t>(1000, reducedownload * cur_datarate / 100); // Always try 1 kB/s if possible
// END netfinity: Advanced download throttling
					cur_src->socket->SetDownloadLimit(limit);
					// ismod: no PeerCache
					}
				}
			}
		}
	}
	else
	{
		bool downloadingbefore=m_anStates[DS_DOWNLOADING]>0;
		// -khaos--+++> Moved this here, otherwise we were setting our permanent variables to 0 every tenth of a second...
		memset(m_anStates,0,sizeof(m_anStates));
		memset(src_stats,0,sizeof(src_stats));
		memset(net_stats,0,sizeof(net_stats));
		UINT nCountForState;

		for (POSITION pos = srclist.GetHeadPosition(); pos != NULL;)
		{
			CUpDownClient* cur_src = srclist.GetNext(pos);
			if (thePrefs.m_iDbgHeap >= 2)
				ASSERT_VALID( cur_src );

			// BEGIN -rewritten- refreshing statistics (no need for temp vars since it is not multithreaded)
			nCountForState = cur_src->GetDownloadState();
			//special case which is not yet set as downloadstate
			if (nCountForState == DS_ONQUEUE)
			{
				if( cur_src->IsRemoteQueueFull() )
					nCountForState = DS_REMOTEQUEUEFULL;
			}

			// this is a performance killer -> avoid calling 'IsBanned' for gathering stats
			//if (cur_src->IsBanned())
			//	nCountForState = DS_BANNED;
			if (cur_src->GetUploadState() == US_BANNED) // not as accurate as 'IsBanned', but way faster and good enough for stats.
				nCountForState = DS_BANNED;

			if (cur_src->GetSourceFrom() >= SF_SERVER && cur_src->GetSourceFrom() <= SF_PASSIVE)
				++src_stats[cur_src->GetSourceFrom()];

			if (cur_src->GetServerIP() && cur_src->GetServerPort())
			{
				net_stats[0]++;
				if(cur_src->GetKadPort())
					net_stats[2]++;
			}
			if (cur_src->GetKadPort())
				net_stats[1]++;

			ASSERT( nCountForState < _countof(m_anStates) );
			m_anStates[nCountForState]++;

			switch (cur_src->GetDownloadState())
			{
				case DS_DOWNLOADING:{
					ASSERT( cur_src->socket );
					if (cur_src->socket)
					{
						cur_src->CheckDownloadTimeout();
						// netfinity: Transfer Rate BlackBox - Calculation is done automaticaly
						uint32_t cur_datarate = cur_src->GetDownloadDatarate();
						cur_src->UpdateDisplayedInfo();
						datarate += cur_datarate;
						if (reducedownload && cur_src->GetDownloadState() == DS_DOWNLOADING)
						{
// BEGIN netfinity: Advanced download throttling
							//uint32 limit = reducedownload*cur_datarate/1000;
							uint32_t limit = max<uint32_t>(1000, reducedownload * cur_datarate / 100); // Always try 1 kB/s if possible
// END netfinity: Advanced download throttling
							cur_src->socket->SetDownloadLimit(limit);
							// ismod: no PeerCache

						}
						else{
							cur_src->socket->DisableDownloadLimit();
							// ismod: no PeerCache
						}
					}
					break;
				}
				// Do nothing with this client..
				case DS_BANNED:
					break;
				// Check if something has changed with our or their ID state..
				case DS_LOWTOLOWIP:
				{
					// To Mods, please stop instantly removing these sources..
					// This causes sources to pop in and out creating extra overhead!
					//Make sure this source is still a LowID Client..
					if( cur_src->HasLowID() )
					{
						//Make sure we still cannot callback to this Client..
						if( !theApp.CanDoCallback( cur_src ) )
						{
							//If we are almost maxed on sources, slowly remove these client to see if we can find a better source.
							if( ((dwCurTick - lastpurgetime) > SEC2MS(30)) && (this->GetSourceCount() >= (GetMaxSources()*.8 )) )
							{
								theApp.downloadqueue->RemoveSource( cur_src );
								lastpurgetime = dwCurTick;
							}
							break;
						}
					}
					// This should no longer be a LOWTOLOWIP..
					cur_src->SetDownloadState(DS_ONQUEUE);
					break;
				}
				case DS_NONEEDEDPARTS:
				{ 
					// To Mods, please stop instantly removing these sources..
					// This causes sources to pop in and out creating extra overhead!
					if( (dwCurTick - lastpurgetime) > SEC2MS(40) ){
						lastpurgetime = dwCurTick;
						// we only delete them if reaching the limit
						if (GetSourceCount() >= (GetMaxSources()*.8 )){
							theApp.downloadqueue->RemoveSource( cur_src );
							break;
						}			
					}
					// doubled reasktime for no needed parts - save connections and traffic
                    if (cur_src->GetTimeUntilReask() > 0)
						break; 

                    cur_src->SwapToAnotherFile(_T("A4AF for NNP file. CPartFile::Process()"), true, false, false, NULL, true, true); // ZZ:DownloadManager
					// Recheck this client to see if still NNP.. Set to DS_NONE so that we force a TCP reask next time..
    				cur_src->SetDownloadState(DS_NONE);
					break;
				}
				case DS_ONQUEUE:
				{
					// To Mods, please stop instantly removing these sources..
					// This causes sources to pop in and out creating extra overhead!
					if( cur_src->IsRemoteQueueFull() ) 
					{
						if( ((dwCurTick - lastpurgetime) > MIN2MS(1)) && (GetSourceCount() >= (GetMaxSources()*.8 )) )
						{
							theApp.downloadqueue->RemoveSource( cur_src );
							lastpurgetime = dwCurTick;
							break;
						}
					}
					//Give up to 1 min for UDP to respond.. If we are within one min of TCP reask, do not try..
					if ( theApp.IsConnected() && cur_src->GetTimeUntilReask() < MIN2MS(2) && cur_src->GetTimeUntilReask() > SEC2MS(1) &&
						 (::GetTickCount()-cur_src->getLastTriedToConnectTime() > FILEREASKTIME)
					   ) // ZZ:DownloadManager (one resk timestamp for each file)
						cur_src->UDPReaskForDownload();
				}
				case DS_CONNECTING:
				case DS_TOOMANYCONNS:
				case DS_TOOMANYCONNSKAD:
				case DS_NONE:
				case DS_WAITCALLBACK:
				case DS_WAITCALLBACKKAD:
				{
					if ( ( theApp.IsConnected() && cur_src->GetTimeUntilReask() == 0 ) &&
						 (::GetTickCount()-cur_src->getLastTriedToConnectTime() > FILEREASKTIME)
					   ) // ZZ:DownloadManager (one resk timestamp for each file)
					{
						cur_src->AskForDownload(); // NOTE: This may *delete* the client!!
					}
					break;
				}
			}
		}
		if (downloadingbefore!=(m_anStates[DS_DOWNLOADING]>0))
			NotifyStatusChange();
 
		if( GetMaxSourcePerFileUDP() > GetSourceCount()){
			if (theApp.downloadqueue->DoKademliaFileRequest() && (Kademlia::CKademlia::GetTotalFile() < KADEMLIATOTALFILE) && (dwCurTick > m_LastSearchTimeKad) &&  Kademlia::CKademlia::IsConnected() && theApp.IsConnected() && !stopped){ //Once we can handle lowID users in Kad, we remove the second IsConnected
				//Kademlia
				theApp.downloadqueue->SetLastKademliaFileRequest();
				if (!GetKadFileSearchID())
				{
					Kademlia::CSearch* pSearch = Kademlia::CSearchManager::PrepareLookup(Kademlia::CSearch::FILE, true, GetFileHash());
					if (pSearch)
					{
						if(m_TotalSearchesKad < 7)
							m_TotalSearchesKad++;
						m_LastSearchTimeKad = dwCurTick + (KADEMLIAREASKTIME*m_TotalSearchesKad);
						pSearch->SetGUIName(GetFileName());
						SetKadFileSearchID(pSearch->GetSearchID());
					}
					else
						SetKadFileSearchID(0);
				}
			}
		}
		else{
			if(GetKadFileSearchID())
			{
				Kademlia::CSearchManager::StopSearch(GetKadFileSearchID(), true);
			}
		}


		// check if we want new sources from server
		if ( !m_bLocalSrcReqQueued && ((!m_LastSearchTime) || (dwCurTick - m_LastSearchTime) > SERVERREASKTIME) && theApp.serverconnect->IsConnected()
			&& GetMaxSourcePerFileSoft() > GetSourceCount() && !stopped
			&& (!IsLargeFile() || (theApp.serverconnect->GetCurrentServer() != NULL && theApp.serverconnect->GetCurrentServer()->SupportsLargeFilesTCP())))
		{
			m_bLocalSrcReqQueued = true;
			theApp.downloadqueue->SendLocalSrcRequest(this);
		}

		count++;
		if (count == 3){
			count = 0;
			UpdateAutoDownPriority();
			UpdateDisplayedInfo();
			UpdateCompletedInfos();
		}
	}

	if ( GetSrcStatisticsValue(DS_DOWNLOADING) != nOldTransSourceCount ){
		if (theApp.emuledlg->transferwnd->GetDownloadList()->curTab == 0)
			theApp.emuledlg->transferwnd->GetDownloadList()->ChangeCategory(0); 
		else
			UpdateDisplayedInfo(true);
		if (thePrefs.ShowCatTabInfos() )
			theApp.emuledlg->transferwnd->UpdateCatTabTitles();
	}

	return datarate;
}

bool CPartFile::CanAddSource(uint32_t userid, uint16_t port, uint32_t serverip, uint16_t serverport, UINT* pdebug_lowiddropped, bool Ed2kID)
{
	//The incoming ID could have the userid in the Hybrid format.. 
	uint32_t hybridID = 0;
	if( Ed2kID )
	{
		if(IsLowID(userid))
			hybridID = userid;
		else
			hybridID = ntohl(userid);
	}
	else
	{
		hybridID = userid;
		if(!IsLowID(userid))
			userid = ntohl(userid);
	}

	// MOD Note: Do not change this part - Merkur
	if (theApp.serverconnect->IsConnected())
	{
		if(theApp.serverconnect->IsLowID())
		{
			if(theApp.serverconnect->GetClientID() == userid && theApp.serverconnect->GetCurrentServer()->GetIP() == serverip && theApp.serverconnect->GetCurrentServer()->GetPort() == serverport )
				return false;
			if(theApp.serverconnect->GetLocalIP() == userid)
				return false;
		}
		else
		{
			if(theApp.serverconnect->GetClientID() == userid && thePrefs.GetPort() == port)
				return false;
		}
	}
	if (Kademlia::CKademlia::IsConnected())
	{
		if(!Kademlia::CKademlia::IsFirewalled())
			if(Kademlia::CKademlia::GetIPAddress() == hybridID && thePrefs.GetPort() == port)
				return false;
	}

	//This allows *.*.*.0 clients to not be removed if Ed2kID == false
	if ( IsLowID(hybridID) && theApp.IsFirewalled() )
	{
		if (pdebug_lowiddropped)
			(*pdebug_lowiddropped)++;
		return false;
	}
	// MOD Note - end
	return true;
}

void CPartFile::AddSources(CSafeMemFile* sources, uint32_t serverip, uint16_t serverport, bool bWithObfuscationAndHash)
{
	UINT count = sources->ReadUInt8();

	UINT debug_lowiddropped = 0;
	UINT debug_possiblesources = 0;
	uchar achUserHash[16];
	bool bSkip = false;
	for (UINT i = 0; i < count; i++)
	{
		uint32_t userid = sources->ReadUInt32();
		uint16_t port = sources->ReadUInt16();
		uint8_t byCryptOptions = 0;
		if (bWithObfuscationAndHash){
			byCryptOptions = sources->ReadUInt8();
			if ((byCryptOptions & 0x80) > 0)
				sources->ReadHash16(achUserHash);

			if ((thePrefs.IsClientCryptLayerRequested() && (byCryptOptions & 0x01/*supported*/) > 0 && (byCryptOptions & 0x80) == 0)
				|| (thePrefs.IsClientCryptLayerSupported() && (byCryptOptions & 0x02/*requested*/) > 0 && (byCryptOptions & 0x80) == 0))
				DebugLogWarning(_T("Server didn't provide UserHash for source %u, even if it was expected to (or local obfuscationsettings changed during serverconnect"), userid);
			else if (!thePrefs.IsClientCryptLayerRequested() && (byCryptOptions & 0x02/*requested*/) == 0 && (byCryptOptions & 0x80) != 0)
				DebugLogWarning(_T("Server provided UserHash for source %u, even if it wasn't expected to (or local obfuscationsettings changed during serverconnect"), userid);
		}
		
		// since we may received multiple search source UDP results we have to "consume" all data of that packet
		if (stopped || bSkip)
			continue;

		// check the HighID(IP) - "Filter LAN IPs" and "IPfilter" the received sources IP addresses
		if (!IsLowID(userid))
		{
			if (!IsGoodIP(userid))
			{ 
				// check for 0-IP, localhost and optionally for LAN addresses
				//if (thePrefs.GetLogFilteredIPs())
				//	AddDebugLogLine(false, _T("Ignored source (IP=%s) received from server - bad IP"), ipstr(userid));
				continue;
			}
			if (theApp.ipfilter->IsFiltered(userid))
			{
				if (thePrefs.GetLogFilteredIPs())
					AddDebugLogLine(false, _T("Ignored source (IP=%s) received from server - IP filter (%s)"), ipstr(userid), theApp.ipfilter->GetLastHit());
				continue;
			}
			if (theApp.clientlist->IsBannedClient(userid)){
#ifdef _DEBUG
				if (thePrefs.GetLogBannedClients()){
					CUpDownClient* pClient = theApp.clientlist->FindClientByIP(userid);
					AddDebugLogLine(false, _T("Ignored source (IP=%s) received from server - banned client %s"), ipstr(userid), pClient->DbgGetClientInfo());
				}
#endif
				continue;
			}
		}

		// additionally check for LowID and own IP
		if (!CanAddSource(userid, port, serverip, serverport, &debug_lowiddropped))
		{
			//if (thePrefs.GetLogFilteredIPs())
			//	AddDebugLogLine(false, _T("Ignored source (IP=%s) received from server"), ipstr(userid));
			continue;
		}

		if( GetMaxSources() > this->GetSourceCount() )
		{
			debug_possiblesources++;
			CUpDownClient* newsource = new CUpDownClient(this,port,userid,serverip,serverport,true);
			newsource->SetConnectOptions(byCryptOptions, true, false);

			if ((byCryptOptions & 0x80) != 0)
				newsource->SetUserHash(achUserHash);
			theApp.downloadqueue->CheckAndAddSource(this,newsource);
		}
		else
		{
			// since we may received multiple search source UDP results we have to "consume" all data of that packet
			bSkip = true;
			if(GetKadFileSearchID())
				Kademlia::CSearchManager::StopSearch(GetKadFileSearchID(), false);
			continue;
		}
	}
	if (thePrefs.GetDebugSourceExchange())
		AddDebugLogLine(false, _T("SXRecv: Server source response; Count=%u, Dropped=%u, PossibleSources=%u, File=\"%s\""), count, debug_lowiddropped, debug_possiblesources, GetFileName());
}

// ismod: no PeerCache

// SLUGFILLER: heapsortCompletesrc
static void HeapSort(CArray<uint16_t, uint16_t>& count, UINT first, UINT last){
	UINT r;
	for ( r = first; !(r & (UINT)INT_MIN) && (r<<1) < last; ){
		UINT r2 = (r<<1)+1;
		if (r2 != last)
			if (count[r2] < count[r2+1])
				r2++;
		if (count[r] < count[r2]){
			uint16_t t = count[r2];
			count[r2] = count[r];
			count[r] = t;
			r = r2;
		}
		else
			break;
	}
}
// SLUGFILLER: heapsortCompletesrc

void CPartFile::UpdatePartsInfo()
{
	if( !IsPartFile() )
	{
		CKnownFile::UpdatePartsInfo();
		return;
	}

	// Cache part count
	UINT partcount = partCount();
	bool flag = (time(NULL) - m_nCompleteSourcesTime > 0); 

	// Reset part counters
	/*
	if (static_cast<UINT>(m_SrcpartFrequency.GetSize()) < partcount)
		m_SrcpartFrequency.SetSize(partcount);
	*/
	for (UINT i = 0; i < partcount; i++)
		m_PartsInfo[i].frequency = 0;
	
	CArray<uint16_t, uint16_t> count;
	if (flag)
		count.SetSize(0, srclist.GetSize());
	for (POSITION pos = srclist.GetHeadPosition(); pos != 0; )
	{
		CUpDownClient* cur_src = srclist.GetNext(pos);
		if( cur_src->GetPartStatus() )
		{		
			for (UINT i = 0; i < partcount; i++)
			{
				if (cur_src->IsPartAvailable(i))
					m_PartsInfo[i].frequency += 1;
			}
			if ( flag )
			{
				count.Add(cur_src->GetUpCompleteSourcesCount());
			}
		}
	}

	if (flag)
	{
		m_nCompleteSourcesCount = m_nCompleteSourcesCountLo = m_nCompleteSourcesCountHi = 0;
	
		for (UINT i = 0; i < partcount; i++)
		{
			if (!i)
				m_nCompleteSourcesCount = m_PartsInfo[i].frequency;
			else if( m_nCompleteSourcesCount > m_PartsInfo[i].frequency)
				m_nCompleteSourcesCount = m_PartsInfo[i].frequency;
		}
	
		count.Add(m_nCompleteSourcesCount);
	
		int n = static_cast<int>(count.GetSize());
		if (n > 0)
		{
			// SLUGFILLER: heapsortCompletesrc
			int r;
			for (r = n/2; r--; )
				HeapSort(count, r, n-1);
			for (r = n; --r; ){
				uint16_t t = count[r];
				count[r] = count[0];
				count[0] = t;
				HeapSort(count, 0, r-1);
			}
			// SLUGFILLER: heapsortCompletesrc

			// calculate range
			int i = n >> 1;			// (n / 2)
			int j = (n * 3) >> 2;	// (n * 3) / 4
			int k = (n * 7) >> 3;	// (n * 7) / 8

			//When still a part file, adjust your guesses by 20% to what you see..

			//Not many sources, so just use what you see..
			if (n < 5)
			{
//				m_nCompleteSourcesCount;
				m_nCompleteSourcesCountLo= m_nCompleteSourcesCount;
				m_nCompleteSourcesCountHi= m_nCompleteSourcesCount;
			}
			//For low guess and normal guess count
			//	If we see more sources then the guessed low and normal, use what we see.
			//	If we see less sources then the guessed low, adjust network accounts for 80%, we account for 20% with what we see and make sure we are still above the normal.
			//For high guess
			//  Adjust 80% network and 20% what we see.
			else if (n < 20)
			{
				if ( count.GetAt(i) < m_nCompleteSourcesCount )
					m_nCompleteSourcesCountLo = m_nCompleteSourcesCount;
				else
					m_nCompleteSourcesCountLo = (uint16_t)((float)(count.GetAt(i)*.8)+(float)(m_nCompleteSourcesCount*.2));
				m_nCompleteSourcesCount= m_nCompleteSourcesCountLo;
				m_nCompleteSourcesCountHi= (uint16_t)((float)(count.GetAt(j)*.8)+(float)(m_nCompleteSourcesCount*.2));
				if( m_nCompleteSourcesCountHi < m_nCompleteSourcesCount )
					m_nCompleteSourcesCountHi = m_nCompleteSourcesCount;
			}
			else
			//Many sources..
			//For low guess
			//	Use what we see.
			//For normal guess
			//	Adjust network accounts for 80%, we account for 20% with what we see and make sure we are still above the low.
			//For high guess
			//  Adjust network accounts for 80%, we account for 20% with what we see and make sure we are still above the normal.
			{
				m_nCompleteSourcesCountLo= m_nCompleteSourcesCount;
				m_nCompleteSourcesCount= (uint16_t)((float)(count.GetAt(j)*.8)+(float)(m_nCompleteSourcesCount*.2));
				if( m_nCompleteSourcesCount < m_nCompleteSourcesCountLo )
					m_nCompleteSourcesCount = m_nCompleteSourcesCountLo;
				m_nCompleteSourcesCountHi= (uint16_t)((float)(count.GetAt(k)*.8)+(float)(m_nCompleteSourcesCount*.2));
				if( m_nCompleteSourcesCountHi < m_nCompleteSourcesCount )
					m_nCompleteSourcesCountHi = m_nCompleteSourcesCount;
			}
		}
		m_nCompleteSourcesTime = time(NULL) + (60);
	}
	UpdateDisplayedInfo();
}	

bool CPartFile::RemoveBlockFromList(uint64_t start, uint64_t end)
{
	ASSERT( start <= end );

	for ( RequestedBlockList::iterator it = requestedblocks_list.begin(); it != requestedblocks_list.end(); ++it ) {
		const Requested_Block_Struct *block = *it;
		if ( block->StartOffset == start && block->EndOffset == end ) {
			it = requestedblocks_list.erase(it);
			return true;
		}
	}
	return false;
}

bool CPartFile::IsInRequestedBlockList( const Requested_Block_Struct* block ) const
{
	return std::find(requestedblocks_list.begin(), requestedblocks_list.end(), block) != requestedblocks_list.end();
}

void CPartFile::RemoveAllRequestedBlocks(void)
{
	requestedblocks_list.clear();
}

void CPartFile::CompleteFile(bool bIsHashingDone)
{
	theApp.downloadqueue->RemoveLocalServerRequest(this);
	if( GetKadFileSearchID() )
		Kademlia::CSearchManager::StopSearch(GetKadFileSearchID(), false);

	if ( srcarevisible )
		theApp.emuledlg->transferwnd->GetDownloadList()->HideSources(this);
	
	if ( !bIsHashingDone ) {
		SetStatus(PS_COMPLETING);
		datarate = 0;
		CAddFileThread* addfilethread = (CAddFileThread*) CSafeThread::BeginThread(RUNTIME_CLASS(CAddFileThread), THREAD_PRIORITY_BELOW_NORMAL,0, CREATE_SUSPENDED);
		if ( addfilethread ) {
			SetFileOp(PFOP_HASHING);
			SetFileOpProgress(0);
			TCHAR mytemppath[MAX_PATH];
			_tcscpy_s(mytemppath, _countof(mytemppath), m_fullname);
			mytemppath[ _tcslen(mytemppath)-_tcslen(m_partmetfilename)-1] = 0;
			addfilethread->SetValues(NULL, mytemppath, RemoveFileExtension(m_partmetfilename), _T(""), this);
			addfilethread->ResumeThread();	
		} else {
			LogError(LOG_STATUSBAR, GetResString(IDS_ERR_FILECOMPLETIONTHREAD));
			SetStatus(PS_ERROR);
		}
		return;
	} else {
		StopFile();
		SetStatus(PS_COMPLETING);
		CSafeThread *pThread = CSafeThread::BeginThread(CompleteThreadProc, this, THREAD_PRIORITY_BELOW_NORMAL, 0, CREATE_SUSPENDED);
		if ( pThread ) {
			SetFileOp(PFOP_COPYING);
			SetFileOpProgress(0);
			pThread->ResumeThread();
		} else {
			LogError(LOG_STATUSBAR, GetResString(IDS_ERR_FILECOMPLETIONTHREAD));
			SetStatus(PS_ERROR);
			return;
		}
	}
	theApp.emuledlg->transferwnd->GetDownloadList()->ShowFilesCount();
	if (thePrefs.ShowCatTabInfos())
		theApp.emuledlg->transferwnd->UpdateCatTabTitles();
	UpdateDisplayedInfo(true);
}

UINT CPartFile::CompleteThreadProc(LPVOID pvParams) 
{ 
	DbgSetThreadName("PartFileComplete");
	InitThreadLocale();
	// BEGIN SLUGFILLER: SafeHash
	CReadWriteLock lock(&theApp.m_threadlock);
	if (!lock.ReadLock(0))
		return 0;
	// END SLUGFILLER: SafeHash
	CPartFile* pFile = (CPartFile*)pvParams;
	if (!pFile)
		return (UINT)-1; 
	CoInitialize(NULL);
   	pFile->PerformFileComplete();
	CoUninitialize();
   	return 0; 
}

void UncompressFile(LPCTSTR pszFilePath, CPartFile* pPartFile)
{
	// check, if it's a compressed file
	DWORD dwAttr = GetFileAttributes(pszFilePath);
	if (dwAttr == INVALID_FILE_ATTRIBUTES || (dwAttr & FILE_ATTRIBUTE_COMPRESSED) == 0)
		return;

	CString strDir = pszFilePath;
	PathRemoveFileSpec(strDir.GetBuffer());
	strDir.ReleaseBuffer();

	// If the directory of the file has the 'Compress' attribute, do not uncomress the file
	dwAttr = GetFileAttributes(strDir);
	if (dwAttr == INVALID_FILE_ATTRIBUTES || (dwAttr & FILE_ATTRIBUTE_COMPRESSED) != 0)
		return;

	HANDLE hFile = CreateFile(pszFilePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE){
		if (thePrefs.GetVerbose())
			theApp.QueueDebugLogLine(true, _T("Failed to open file \"%s\" for decompressing - %s"), pszFilePath, GetErrorMessage(GetLastError(), 1));
		return;
	}
	
	if (pPartFile)
		pPartFile->SetFileOp(PFOP_UNCOMPRESSING);

	USHORT usInData = COMPRESSION_FORMAT_NONE;
	DWORD dwReturned = 0;
	if (!DeviceIoControl(hFile, FSCTL_SET_COMPRESSION, &usInData, sizeof usInData, NULL, 0, &dwReturned, NULL)){
		if (thePrefs.GetVerbose())
			theApp.QueueDebugLogLine(true, _T("Failed to decompress file \"%s\" - %s"), pszFilePath, GetErrorMessage(GetLastError(), 1));
	}
	CloseHandle(hFile);
}

#ifndef __IZoneIdentifier_INTERFACE_DEFINED__
MIDL_INTERFACE("cd45f185-1b21-48e2-967b-ead743a8914e")
IZoneIdentifier : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE GetId(DWORD *pdwZone) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetId(DWORD dwZone) = 0;
    virtual HRESULT STDMETHODCALLTYPE Remove(void) = 0;
};
#endif //__IZoneIdentifier_INTERFACE_DEFINED__

#ifdef HAVE_VISTA_SDK
EXTERN_C const IID CLSID_PersistentZoneIdentifier;
#else
const GUID CLSID_PersistentZoneIdentifier = { 0x0968E258, 0x16C7, 0x4DBA, { 0xAA, 0x86, 0x46, 0x2D, 0xD6, 0x1E, 0x31, 0xA3 } };
#endif

void SetZoneIdentifier(LPCTSTR pszFilePath)
{
	if (!thePrefs.GetCheckFileOpen())
		return;
	CComPtr<IZoneIdentifier> pZoneIdentifier;
	HRESULT hr = pZoneIdentifier.CoCreateInstance(CLSID_PersistentZoneIdentifier, NULL, CLSCTX_INPROC_SERVER);
	if (SUCCEEDED(hr))
	{
		CComQIPtr<IPersistFile> pPersistFile = pZoneIdentifier;
		if (pPersistFile)
		{
			// Specify the 'zone identifier' which has to be commited with 'IPersistFile::Save'
			hr = pZoneIdentifier->SetId(URLZONE_INTERNET);
			if (SUCCEEDED(hr))
			{
				// Save the 'zone identifier'
				// NOTE: This does not modify the file content in any way, 
				// *but* it modifies the "Last Modified" file time!
				VERIFY( SUCCEEDED(hr = pPersistFile->Save(pszFilePath, FALSE)) );
			}
		}
	}
}

DWORD CALLBACK CopyProgressRoutine(LARGE_INTEGER TotalFileSize, LARGE_INTEGER TotalBytesTransferred,
								   LARGE_INTEGER /*StreamSize*/, LARGE_INTEGER /*StreamBytesTransferred*/, DWORD /*dwStreamNumber*/,
								   DWORD /*dwCallbackReason*/, HANDLE /*hSourceFile*/, HANDLE /*hDestinationFile*/, 
								   LPVOID lpData)
{
	CPartFile* pPartFile = (CPartFile*)lpData;
	if (TotalFileSize.QuadPart && pPartFile && pPartFile->IsKindOf(RUNTIME_CLASS(CPartFile)))
	{
		UINT uProgress = (UINT)(TotalBytesTransferred.QuadPart * 100 / TotalFileSize.QuadPart);
		if (uProgress != pPartFile->GetFileOpProgress())
		{
			ASSERT( uProgress <= 100 );
			VERIFY( PostMessage(theApp.emuledlg->GetSafeHwnd(), TM_FILEOPPROGRESS, uProgress, (LPARAM)pPartFile) );
		}
	}
	else
		ASSERT(0);

	return PROGRESS_CONTINUE;
}

DWORD MoveCompletedPartFile(LPCTSTR pszPartFilePath, LPCTSTR pszNewPartFilePath, CPartFile* pPartFile)
{
	DWORD dwMoveResult = ERROR_INVALID_FUNCTION;

	if (MoveFileWithProgress(pszPartFilePath, pszNewPartFilePath, CopyProgressRoutine, pPartFile, MOVEFILE_COPY_ALLOWED))
		dwMoveResult = ERROR_SUCCESS;
	else
		dwMoveResult = GetLastError();


	return dwMoveResult;
}

// Lord KiRon - using threads for file completion
// NOTE: This function is executed within a seperate thread, do *NOT* use any lists/queues of the main thread without
// synchronization. Even the access to couple of members of the CPartFile (e.g. filename) would need to be properly
// synchronization to achive full multi threading compliance.
BOOL CPartFile::PerformFileComplete() 
{
	// If that function is invoked from within the file completion thread, it's ok if we wait (and block) the thread.
	CSingleLock sLock(&m_FileCompleteMutex, TRUE);

	CString strPartfilename(RemoveFileExtension(m_fullname));
	TCHAR* newfilename = _tcsdup(GetFileName());
	_tcscpy(newfilename, (LPCTSTR)StripInvalidFilenameChars(newfilename));

	CString strNewname;
	CString indir;

	if (PathFileExists(thePrefs.GetCategory(GetCategory())->strIncomingPath)){
		indir = thePrefs.GetCategory(GetCategory())->strIncomingPath;
		strNewname.Format(_T("%s\\%s"), indir, newfilename);
	}
	else{
		indir = thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR);
		strNewname.Format(_T("%s\\%s"), indir, newfilename);
	}

	// close permanent handle
	try{
		if (m_hpartfile.m_hFile != INVALID_HANDLE_VALUE)
			m_hpartfile.Close();
	}
	catch(CFileException* error){
		TCHAR buffer[MAX_CFEXP_ERRORMSG];
		error->GetErrorMessage(buffer, _countof(buffer));
		theApp.QueueLogLine(true, GetResString(IDS_ERR_FILEERROR), m_partmetfilename, GetFileName(), buffer);
		error->Delete();
		//return false;
	}

	bool renamed = false;
	if(PathFileExists(strNewname))
	{
		renamed = true;
		int namecount = 0;

		size_t length = _tcslen(newfilename);
		ASSERT(length != 0); //name should never be 0

		//the file extension
		TCHAR *ext = _tcsrchr(newfilename, _T('.'));
		if(ext == NULL)
			ext = newfilename + length;

		TCHAR *last = ext;  //new end is the file name before extension
		last[0] = 0;  //truncate file name

		//search for matching ()s and check if it contains a number
		if((ext != newfilename) && (_tcsrchr(newfilename, _T(')')) + 1 == last)) {
			TCHAR *first = _tcsrchr(newfilename, _T('('));
			if(first != NULL) {
				first++;
				bool found = true;
				for(TCHAR *step = first; step < last - 1; step++)
					if(*step < _T('0') || *step > _T('9')) {
						found = false;
						break;
					}
				if(found) {
					namecount = _tstoi(first);
					last = first - 1;
					last[0] = 0;  //truncate again
				}
			}
		}

		CString strTestName;
		do {
			namecount++;
			strTestName.Format(_T("%s\\%s(%d).%s"), indir, newfilename, namecount, min(ext + 1, newfilename + length));
		}
		while (PathFileExists(strTestName));
		strNewname = strTestName;
	}
	free(newfilename);

	// ismod: wait for preview completion
	if( m_hPreviewProcess != NULL) {
		// set text status
		WaitForSingleObject(m_hPreviewProcess, INFINITE);
		CloseHandle(m_hPreviewProcess);
		m_hPreviewProcess = NULL;
	}


	DWORD dwMoveResult;
	if ((dwMoveResult = MoveCompletedPartFile(strPartfilename, strNewname, this)) != ERROR_SUCCESS)
	{
		theApp.QueueLogLine(true,GetResString(IDS_ERR_COMPLETIONFAILED) + _T(" - \"%s\": ") + GetErrorMessage(dwMoveResult), GetFileName(), strNewname);
		// If the destination file path is too long, the default system error message may not be helpful for user to know what failed.
		if (strNewname.GetLength() >= MAX_PATH)
			theApp.QueueLogLine(true,GetResString(IDS_ERR_COMPLETIONFAILED) + _T(" - \"%s\": Path too long"),GetFileName(), strNewname);

		paused = true;
		stopped = true;
		SetStatus(PS_ERROR);
		m_bCompletionError = true;
		SetFileOp(PFOP_NONE);
		if (theApp.IsRunning())
			VERIFY( PostMessage(theApp.emuledlg->m_hWnd, TM_FILECOMPLETED, FILE_COMPLETION_THREAD_FAILED, (LPARAM)this) );
		return FALSE;
	}

	UncompressFile(strNewname, this);
	SetZoneIdentifier(strNewname);		// may modify the file's "Last Modified" time

	// to have the accurate date stored in known.met we have to update the 'date' of a just completed file.
	// if we don't update the file date here (after commiting the file and before adding the record to known.met), 
	// that file will be rehashed at next startup and there would also be a duplicate entry (hash+size) in known.met
	// because of different file date!
	ASSERT( m_hpartfile.m_hFile == INVALID_HANDLE_VALUE ); // the file must be closed/commited!
	struct _stat st;
	if (_tstat(strNewname, &st) == 0)
	{
		m_tLastModified = st.st_mtime;
		m_tUtcLastModified = m_tLastModified;
		AdjustNTFSDaylightFileTime(m_tUtcLastModified, strNewname);
	}

	// remove part.met file
	if (_tremove(m_fullname))
		theApp.QueueLogLine(true, GetResString(IDS_ERR_DELETEFAILED) + _T(" - ") + CString(_tcserror(errno)), m_fullname);

	// remove backup files
	CString BAKName(m_fullname);
	BAKName.Append(PARTMET_BAK_EXT);
	if (_taccess(BAKName, 0) == 0 && !::DeleteFile(BAKName))
		theApp.QueueLogLine(true,GetResString(IDS_ERR_DELETE) + _T(" - ") + GetErrorMessage(GetLastError()), BAKName);

	BAKName = m_fullname;
	BAKName.Append(PARTMET_TMP_EXT);
	if (_taccess(BAKName, 0) == 0 && !::DeleteFile(BAKName))
		theApp.QueueLogLine(true,GetResString(IDS_ERR_DELETE) + _T(" - ") + GetErrorMessage(GetLastError()), BAKName);
	
	// initialize 'this' part file for being a 'complete' file, this is to be done *before* releasing the file mutex.
	m_fullname = strNewname;
	SetPath(indir);
	SetFilePath(m_fullname);
	_SetStatus(PS_COMPLETE); // set status of CPartFile object, but do not update GUI (to avoid multi-thread problems)
	paused = false;
	SetFileOp(PFOP_NONE);

    // ismod: no aich

	// explicitly unlock the file before posting something to the main thread.
	sLock.Unlock();

	if (theApp.IsRunning())
		VERIFY( PostMessage(theApp.emuledlg->m_hWnd, TM_FILECOMPLETED, FILE_COMPLETION_THREAD_SUCCESS | (renamed ? FILE_COMPLETION_THREAD_RENAMED : 0), (LPARAM)this) );
	return TRUE;
}

// 'End' of file completion, to avoid multi threading synchronization problems, this is to be invoked from within the
// main thread!
void CPartFile::PerformFileCompleteEnd(DWORD dwResult)
{
	if (dwResult & FILE_COMPLETION_THREAD_SUCCESS)
	{
		SetStatus(PS_COMPLETE); // (set status and) update status-modification related GUI elements
		theApp.knownfiles->SafeAddKFile(this);
		theApp.downloadqueue->RemoveFile(this);
		theApp.mmserver->AddFinishedFile(this);
		if (thePrefs.GetRemoveFinishedDownloads())
			theApp.emuledlg->transferwnd->GetDownloadList()->RemoveFile(this);
		else
			UpdateDisplayedInfo(true);

		theApp.emuledlg->transferwnd->GetDownloadList()->ShowFilesCount();

		thePrefs.Add2DownCompletedFiles();
		thePrefs.Add2DownSessionCompletedFiles();
		thePrefs.SaveCompletedDownloadsStat();

		// 05-J‰n-2004 [bc]: ed2k and Kad are already full of totally wrong and/or not properly attached meta data. Take
		// the chance to clean any available meta data tags and provide only tags which were determined by us.
		UpdateMetaDataTags();

		// republish that file to the ed2k-server to update the 'FT_COMPLETE_SOURCES' counter on the server.
		theApp.sharedfiles->RepublishFile(this);

		// give visual response
		Log(LOG_SUCCESS | LOG_STATUSBAR, GetResString(IDS_DOWNLOADDONE), GetFileName());
		theApp.emuledlg->ShowNotifier(GetResString(IDS_TBN_DOWNLOADDONE) + _T('\n') + GetFileName(), TBN_DOWNLOADFINISHED, GetFilePath());
		if (dwResult & FILE_COMPLETION_THREAD_RENAMED)
		{
			CString strFilePath(GetFullName());
			PathStripPath(strFilePath.GetBuffer());
			strFilePath.ReleaseBuffer();
			Log(LOG_STATUSBAR, GetResString(IDS_DOWNLOADRENAMED), strFilePath);
		}
		if(!m_pCollection && CCollection::HasCollectionExtention(GetFileName()))
		{
			m_pCollection = new CCollection();
			if(!m_pCollection->InitCollectionFromFile(GetFilePath(), GetFileName()))
			{
				delete m_pCollection;
				m_pCollection = NULL;
			}
		}
	}

	theApp.downloadqueue->StartNextFileIfPrefs(GetCategory());
}

void  CPartFile::RemoveAllSources(bool bTryToSwap)
{
	for( POSITION pos = srclist.GetHeadPosition(); pos != NULL; /**/) {
		CUpDownClient *src = srclist.GetNext(pos);
		if ( bTryToSwap ) {
			if ( !src->SwapToAnotherFile(_T("Removing source. CPartFile::RemoveAllSources()"), true, true, true, NULL, false, false) ) // ZZ:DownloadManager
				theApp.downloadqueue->RemoveSource(src, false);
		} else
			theApp.downloadqueue->RemoveSource(src, false);
	}
	UpdatePartsInfo(); 
	UpdateAvailablePartsCount();

	//[enkeyDEV(Ottavio84) -A4AF-]
	// remove all links A4AF in sources to this file
	if ( !A4AFsrclist.IsEmpty() )
	{
		POSITION pos1, pos2;
		for(pos1 = A4AFsrclist.GetHeadPosition();(pos2=pos1)!=NULL;)
		{
			A4AFsrclist.GetNext(pos1);
			
			POSITION pos3 = A4AFsrclist.GetAt(pos2)->m_OtherRequests_list.Find(this); 
			if(pos3)
			{ 
				A4AFsrclist.GetAt(pos2)->m_OtherRequests_list.RemoveAt(pos3);
				theApp.emuledlg->transferwnd->GetDownloadList()->RemoveSource(this->A4AFsrclist.GetAt(pos2),this);
			}
			else{
				pos3 = A4AFsrclist.GetAt(pos2)->m_OtherNoNeeded_list.Find(this); 
				if(pos3)
				{ 
					A4AFsrclist.GetAt(pos2)->m_OtherNoNeeded_list.RemoveAt(pos3);
					theApp.emuledlg->transferwnd->GetDownloadList()->RemoveSource(A4AFsrclist.GetAt(pos2),this);
				}
			}
		}
		A4AFsrclist.RemoveAll();
	}
	
	UpdateFileRatingCommentAvail();
}

void CPartFile::DeleteFile()
{
	ASSERT ( !m_bPreviewing );

	// Barry - Need to tell any connected clients to stop sending the file
	StopFile(true);

	// feel free to implement a runtime handling mechanism!
	if ( !canDelete() ) {
		LogWarning(LOG_STATUSBAR, GetResString(IDS_DELETEAFTERALLOC), GetFileName());
		m_bPendingOnDelete = true;
		return;
	}

	theApp.sharedfiles->RemoveFile(this, true);
	theApp.downloadqueue->RemoveFile(this);
	theApp.emuledlg->transferwnd->GetDownloadList()->RemoveFile(this);
	theApp.knownfiles->AddCancelledFileID(GetFileHash());

	if (m_hpartfile.m_hFile != INVALID_HANDLE_VALUE)
		m_hpartfile.Close();

	if (_tremove(m_fullname))
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_DELETE) + _T(" - ") + CString(_tcserror(errno)), m_fullname);
	CString partfilename(RemoveFileExtension(m_fullname));
	if (_tremove(partfilename))
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_DELETE) + _T(" - ") + CString(_tcserror(errno)), partfilename);

	CString BAKName(m_fullname);
	BAKName.Append(PARTMET_BAK_EXT);
	if (_taccess(BAKName, 0) == 0 && !::DeleteFile(BAKName))
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_DELETE) + _T(" - ") + GetErrorMessage(GetLastError()), BAKName);

	BAKName = m_fullname;
	BAKName.Append(PARTMET_TMP_EXT);
	if (_taccess(BAKName, 0) == 0 && !::DeleteFile(BAKName))
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_DELETE) + _T(" - ") + GetErrorMessage(GetLastError()), BAKName);

	delete this;
}

// SLUGFILLER: SafeHash remove - removed HashSinglePart completely.
/*
bool CPartFile::HashSinglePart(UINT partnumber)
{
	if ((GetHashCount() <= partnumber) && (GetPartCount() > 1)){
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_HASHERRORWARNING), GetFileName());
		hashsetneeded = true;
		return true;
	}
	else if (!GetPartHash(partnumber) && GetPartCount() != 1){
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_INCOMPLETEHASH), GetFileName());
		hashsetneeded = true;
		return true;		
	}
	else{
		uchar hashresult[16];
		m_hpartfile.Seek((LONGLONG)PARTSIZE*(uint64)partnumber,0);
		uint32 length = PARTSIZE;
		if ((ULONGLONG)PARTSIZE*(uint64)(partnumber+1) > m_hpartfile.GetLength()){
			length = (UINT)(m_hpartfile.GetLength() - ((ULONGLONG)PARTSIZE*(uint64)partnumber));
			ASSERT( length <= PARTSIZE );
		}
		CreateHash(&m_hpartfile, length, hashresult);

		if (GetPartCount()>1 || GetFileSize()== (uint64)PARTSIZE){
			if (md4cmp(hashresult,GetPartHash(partnumber)))
				return false;
			else
				return true;
		}
		else{
			if (md4cmp(hashresult,m_abyFileHash))
				return false;
			else
				return true;
		}
	}
}
*/
bool CPartFile::IsCorruptedPart(UINT partnumber) const
{
	return (corrupted_list.Find((uint16_t)partnumber) != NULL);
}

// Barry - Also want to preview zip/rar files
bool CPartFile::IsArchive(bool onlyPreviewable) const
{
	if (onlyPreviewable && NULL == m_AllocateThread ) {
		EFileType ftype=GetFileTypeEx((CKnownFile*)this);
		return (ftype==ARCHIVE_RAR || ftype==ARCHIVE_ZIP || ftype==ARCHIVE_ACE);
	}

	return (ED2KFT_ARCHIVE == GetED2KFileTypeID(GetFileName()));
}

bool CPartFile::IsPreviewableFileType() const {
    return IsArchive(true) || IsMovie();
}

void CPartFile::SetDownPriority(uint8_t np, bool resort)
{
	//Changed the default resort to true. As it is was, we almost never sorted the download list when a priority changed.
	//If we don't keep the download list sorted, priority means nothing in downloadqueue.cpp->process().
	//Also, if we call this method with the same priority, don't do anything to help use less CPU cycles.
	if ( m_iDownPriority != np )
	{
		//We have a new priority
		if (np != PR_LOW && np != PR_NORMAL && np != PR_HIGH){
			//This should never happen.. Default to Normal.
			ASSERT(0);
			np = PR_NORMAL;
		}
	
		m_iDownPriority = np;
		//Some methods will change a batch of priorites then call these methods. 
	    if(resort) {
			//Sort the downloadqueue so contacting sources work correctly.
			theApp.downloadqueue->SortByPriority();
			theApp.downloadqueue->CheckDiskspaceTimed();
	    }
		//Update our display to show the new info based on our new priority.
		UpdateDisplayedInfo(true);
		//Save the partfile. We do this so that if we restart eMule before this files does
		//any transfers, it will remember the new priority.
		SavePartFile();
	}
}

bool CPartFile::CanOpenFile() const
{
	return (GetStatus()==PS_COMPLETE);
}

void CPartFile::OpenFile() const
{
	if(m_pCollection)
	{
		CCollectionViewDialog dialog;
		dialog.SetCollection(m_pCollection);
		dialog.DoModal();
	}
	else
		ShellOpenFile(GetFullName(), NULL);
}

bool CPartFile::CanStopFile() const
{
	bool bFileDone = (GetStatus()==PS_COMPLETE || GetStatus()==PS_COMPLETING);
	return (!IsStopped() && GetStatus()!=PS_ERROR && !bFileDone);
}

void CPartFile::StopFile(bool bCancel, bool resort)
{
	// Barry - Need to tell any connected clients to stop sending the file
	PauseFile(false, resort);
	m_LastSearchTimeKad = 0;
	m_TotalSearchesKad = 0;
	RemoveAllSources(true);
	paused = true;
	stopped = true;
	insufficient = false;
	datarate = 0;
	memset(m_anStates,0,sizeof(m_anStates));
	memset(src_stats,0,sizeof(src_stats));
	memset(net_stats,0,sizeof(net_stats));

	if (!bCancel)
		FlushBuffer();
    if(resort) {
	    theApp.downloadqueue->SortByPriority();
	    theApp.downloadqueue->CheckDiskspace();
    }
	UpdateDisplayedInfo(true);
}

void CPartFile::StopPausedFile()
{
	//Once an hour, remove any sources for files which are no longer active downloads
	EPartFileStatus uState = GetStatus();
	if( (uState==PS_PAUSED || uState==PS_INSUFFICIENT || uState==PS_ERROR) && !stopped && time(NULL) - m_iLastPausePurge > (60*60) )
	{
		StopFile();
	}
	else
	{
		if ( pendingOnDelete() && canDelete() )
		{
			DeleteFile();
			return;
		}
	}
}

bool CPartFile::CanPauseFile() const
{
	bool bFileDone = (GetStatus()==PS_COMPLETE || GetStatus()==PS_COMPLETING);
	return (GetStatus()!=PS_PAUSED && GetStatus()!=PS_ERROR && !bFileDone);
}

void CPartFile::PauseFile(bool bInsufficient, bool resort)
{
	// if file is already in 'insufficient' state, don't set it again to insufficient. this may happen if a disk full
	// condition is thrown before the automatically and periodically check free diskspace was done.
	if (bInsufficient && insufficient)
		return;

	// if file is already in 'paused' or 'insufficient' state, do not refresh the purge time
	if (!paused && !insufficient)
		m_iLastPausePurge = time(NULL);
	theApp.downloadqueue->RemoveLocalServerRequest(this);

	if(GetKadFileSearchID())
	{
		Kademlia::CSearchManager::StopSearch(GetKadFileSearchID(), true);
		m_LastSearchTimeKad = 0; //If we were in the middle of searching, reset timer so they can resume searching.
	}

	SetActive(false);

	if (status==PS_COMPLETE || status==PS_COMPLETING)
		return;

	Packet* packet = new Packet(OP_CANCELTRANSFER,0);
	for( POSITION pos = srclist.GetHeadPosition(); pos != NULL; )
	{
		CUpDownClient* cur_src = srclist.GetNext(pos);
		if (cur_src->GetDownloadState() == DS_DOWNLOADING)
		{
			cur_src->SendCancelTransfer(packet);
			cur_src->SetDownloadState(DS_ONQUEUE, _T("You cancelled the download. Sending OP_CANCELTRANSFER"));
		}
	}
	delete packet;

	if (bInsufficient)
	{
		LogError(LOG_STATUSBAR, _T("Insufficient diskspace - pausing download of \"%s\""), GetFileName());
		insufficient = true;
	}
	else
	{
		paused = true;
		insufficient = false;
	}
	NotifyStatusChange();
	datarate = 0;
	m_anStates[DS_DOWNLOADING] = 0; // -khaos--+++> Renamed var.
	if (!bInsufficient)
	{
        if(resort) {
		    theApp.downloadqueue->SortByPriority();
		    theApp.downloadqueue->CheckDiskspace();
        }
		SavePartFile();
	}
	UpdateDisplayedInfo(true);
}

bool CPartFile::CanResumeFile() const
{
	return (GetStatus()==PS_PAUSED || GetStatus()==PS_INSUFFICIENT || (GetStatus()==PS_ERROR && GetCompletionError()));
}

void CPartFile::ResumeFile(bool resort)
{
	if (status==PS_COMPLETE || status==PS_COMPLETING)
		return;
	if (status==PS_ERROR && m_bCompletionError){
		ASSERT( gaplist.empty() );
		if ( gaplist.empty() && !m_nTotalBufferData && !isFlushing() ) {
			// rehashing the file could probably be avoided, but better be in the safe side..
			m_bCompletionError = false;
			CompleteFile(false);
		}
		return;
	}
	paused = false;
	stopped = false;
	SetActive(theApp.IsConnected());
	m_LastSearchTime = 0;
    if(resort) {
	    theApp.downloadqueue->SortByPriority();
	    theApp.downloadqueue->CheckDiskspace();
    }
	SavePartFile();
	NotifyStatusChange();

	UpdateDisplayedInfo(true);
}

void CPartFile::ResumeFileInsufficient()
{
	if (status==PS_COMPLETE || status==PS_COMPLETING)
		return;
	if (!insufficient)
		return;
	AddLogLine(false, _T("Resuming download of \"%s\""), GetFileName());
	insufficient = false;
	SetActive(theApp.IsConnected());
	m_LastSearchTime = 0;
	UpdateDisplayedInfo(true);
}

CString CPartFile::getPartfileStatus() const
{
	switch(GetStatus()){
		case PS_HASHING:
            return GetResString(IDS_HASHING);
		case PS_WAITINGFORHASH:
            return GetResString(IDS_HASHING) + _T("(Queued)");

		case PS_COMPLETING:{
			CString strState = GetResString(IDS_COMPLETING);
			if (GetFileOp() == PFOP_HASHING)
				strState += _T(" (") + GetResString(IDS_HASHING) + _T(")");
			else if (GetFileOp() == PFOP_COPYING)
				if(m_hPreviewProcess != NULL)
					strState = _T("«¿ –Œ…“≈ œ–≈ƒœ–Œ—ÃŒ“–");
				else
					strState += _T(" (Copying)");
			else if (GetFileOp() == PFOP_UNCOMPRESSING)
				strState += _T(" (Uncompressing)");
			return strState;
		}

		case PS_COMPLETE:
			return GetResString(IDS_COMPLETE);

		case PS_PAUSED:
			if (stopped)
				return GetResString(IDS_STOPPED);
			return GetResString(IDS_PAUSED);

		case PS_INSUFFICIENT:
			return GetResString(IDS_INSUFFICIENT);

		case PS_ERROR:
			if (m_bCompletionError)
				return GetResString(IDS_INSUFFICIENT);
			return GetResString(IDS_ERRORLIKE);
	}

	if (GetSrcStatisticsValue(DS_DOWNLOADING) > 0)
		return GetResString(IDS_DOWNLOADING);
	else
		return GetResString(IDS_WAITING);
} 

int CPartFile::getPartfileStatusRang() const
{
	switch (GetStatus()) {
		case PS_HASHING: 
		case PS_WAITINGFORHASH:
			return 7;

		case PS_COMPLETING:
			return 1;

		case PS_COMPLETE:
			return 0;

		case PS_PAUSED:
			if (IsStopped())
				return 6;
			else
				return 5;
		case PS_INSUFFICIENT:
			return 4;

		case PS_ERROR:
			return 8;
	}
	if (GetSrcStatisticsValue(DS_DOWNLOADING) == 0)
		return 3; // waiting?
	return 2; // downloading?
} 

time_t CPartFile::getTimeRemainingSimple() const
{
	if (GetDatarate() == 0)
		return -1;
	return (time_t)((uint64_t)(GetFileSize() - GetCompletedSize()) / (uint64_t)GetDatarate());
}

time_t CPartFile::getTimeRemaining() const
{
	EMFileSize completesize = GetCompletedSize();
	time_t simple = -1;
	time_t estimate = -1;
	if( GetDatarate() > 0 )
	{
		simple = (time_t)((uint64_t)(GetFileSize() - completesize) / (uint64_t)GetDatarate());
	}
	if( GetDlActiveTime() && completesize >= (uint64_t)512000 )
		estimate = (time_t)((uint64_t)(GetFileSize() - completesize) / ((double)completesize / (double)GetDlActiveTime()));

	if( simple == -1 )
	{
		//We are not transferring at the moment.
		if( estimate == -1 )
			//We also don't have enough data to guess
			return -1;
		else if( estimate > HR2S(24*15) )
			//The estimate is too high
			return -1;
		else
			return estimate;
	}
	else if( estimate == -1 )
	{
		//We are transferring but estimate doesn't have enough data to guess
		return simple;
	}
	if( simple < estimate )
		return simple;
	if( estimate > HR2S(24*15) )
		//The estimate is too high..
		return -1;
	return estimate;
}

void CPartFile::PreviewFile(bool real)
{
	if (thePreviewApps.Preview(this))
		return;

	if (IsArchive(true)){
		if (!m_bRecoveringArchive && !m_bPreviewing)
			CArchiveRecovery::recover(this, true, thePrefs.GetPreviewCopiedArchives());
		return;
	}

	if ( !IsReadyForPreview(!real) ) {
		if ( !real ) {
			CPreviewWaitDlg dlg(this);
			dlg.DoModal();
		}
		return;
	}

	if (thePrefs.IsMoviePreviewBackup()){
		if (!CheckFileOpen(GetFilePath(), GetFileName()))
			return;
		m_bPreviewing = true;
		CPreviewThread* pThread = (CPreviewThread*) CSafeThread::BeginThread(RUNTIME_CLASS(CPreviewThread), THREAD_PRIORITY_NORMAL,0, CREATE_SUSPENDED);
		if (pThread == NULL)
			throw CString(_T(__FUNCTION__) _T(": Failed to create worker thread!"));
		pThread->SetValues(this, thePrefs.GetVideoPlayer(), thePrefs.GetVideoPlayerArgs());
		pThread->ResumeThread();
	}
	else{
		ExecutePartFile(this, thePrefs.GetVideoPlayer(), thePrefs.GetVideoPlayerArgs());
	}
}

bool CPartFile::IsReadyForPreview(bool real) const
{
	CPreviewApps::ECanPreviewRes ePreviewAppsRes = thePreviewApps.CanPreview(this);
	if (ePreviewAppsRes != CPreviewApps::NotHandled)
		return (ePreviewAppsRes == CPreviewApps::Yes);

	// Barry - Allow preview of archives of any length > 1k
	if (IsArchive(true))
	{
		//if (GetStatus() != PS_COMPLETE && GetStatus() != PS_COMPLETING 
		//	&& GetFileSize()>1024 && GetCompletedSize()>1024 
		//	&& !m_bRecoveringArchive 
		//	&& GetFreeDiskSpaceX(thePrefs.GetTempDir())+100000000 > 2*GetFileSize())
		//	return true;

		// check part file state
	    EPartFileStatus uState = GetStatus();
		if (uState == PS_COMPLETE || uState == PS_COMPLETING)
			return false;

		// check part file size(s)
		if (GetFileSize() < (uint64_t)1024 || GetCompletedSize() < (uint64_t)1024)
			return false;

		// check if we already trying to recover an archive file from this part file
		if (m_bRecoveringArchive)
			return false;

		// check free disk space
		uint64_t uMinFreeDiskSpace = (thePrefs.IsCheckDiskspaceEnabled() && thePrefs.GetMinFreeDiskSpace() > 0)
									? thePrefs.GetMinFreeDiskSpace()
									: 20*1024*1024;
		if (thePrefs.GetPreviewCopiedArchives())
			uMinFreeDiskSpace += (uint64_t)(GetFileSize() * (uint64_t)2);
		else
			uMinFreeDiskSpace += (uint64_t)(GetCompletedSize() + (uint64_t)16*1024);
		if (GetFreeDiskSpaceX(GetTempPath()) < uMinFreeDiskSpace)
			return false;
		return true; 
	}

	if (thePrefs.IsMoviePreviewBackup())
	{
		return !( (GetStatus() != PS_READY && GetStatus() != PS_PAUSED) 
				|| m_bPreviewing || partCount() < 5 || !IsMovie() || (GetFreeDiskSpaceX(GetTempPath()) + 100000000) < GetFileSize()
				|| ( !IsPartComplete(0) || !IsPartComplete(partCount()-1)));
	}
	else
	{
		if(real) {
			TCHAR szVideoPlayerFileName[_MAX_FNAME];
			_tsplitpath_s(thePrefs.GetVideoPlayer(), NULL, 0, NULL, 0, szVideoPlayerFileName, _countof(szVideoPlayerFileName), NULL, 0);

			// enable the preview command if the according option is specified 'PreviewSmallBlocks' 
			// or if VideoLAN client is specified
			if (thePrefs.GetPreviewSmallBlocks() || !_tcsicmp(szVideoPlayerFileName, _T("vlc")))
			{
				if (m_bPreviewing)
					return false;

				EPartFileStatus uState = GetStatus();
				if (!(uState == PS_READY || uState == PS_EMPTY || uState == PS_PAUSED || uState == PS_INSUFFICIENT || uState == PS_COMPLETE))
					return false;

				// default: check the ED2K file format to be of type audio, video or CD image.
				// but because this could disable the preview command for some file types which eMule does not know,
				// this test can be avoided by specifying 'PreviewSmallBlocks=2'
				if (thePrefs.GetPreviewSmallBlocks() <= 1)
				{
					// check the file extension
					EED2KFileType eFileType = GetED2KFileTypeID(GetFileName());
					if (!(eFileType == ED2KFT_VIDEO || eFileType == ED2KFT_AUDIO || eFileType == ED2KFT_CDIMAGE))
					{
						// check the ED2K file type
						const CString& rstrED2KFileType = GetStrTagValue(FT_FILETYPE);
						if (rstrED2KFileType.IsEmpty() || !(!_tcscmp(rstrED2KFileType, _T(ED2KFTSTR_AUDIO)) || !_tcscmp(rstrED2KFileType, _T(ED2KFTSTR_VIDEO))))
							return false;
					}
				}

				// If it's an MPEG file, VLC is even capable of showing parts of the file if the beginning of the file is missing!
				bool bMPEG = false;
				LPCTSTR pszExt = _tcsrchr(GetFileName(), _T('.'));
				if (pszExt != NULL){
					CString strExt(pszExt);
					strExt.MakeLower();
					bMPEG = (strExt==_T(".mpg") || strExt==_T(".mpeg") || strExt==_T(".mpe") || strExt==_T(".mp3") || strExt==_T(".mp2") || strExt==_T(".mpa"));
				}

				if (bMPEG){
					// TODO: search a block which is at least 16K (Audio) or 256K (Video)
					if (GetCompletedSize() < (uint64_t)16*1024)
						return false;
				}
				else{
					// For AVI files it depends on the used codec..
					if (thePrefs.GetPreviewSmallBlocks() >= 2) {
						if ( GetCompletedSize() < (uint64_t)256*1024 )
							return false;
					} else {
					    if ( !IsPartComplete(0) || !IsPartComplete(partCount()-1) )
						    return false;
					}
				}
    
				return true;
			}
			else{
				return !((GetStatus() != PS_READY && GetStatus() != PS_PAUSED) 
						|| m_bPreviewing || partCount() < 2 || !IsMovie() || !IsPartComplete(0)); 
			}
		} else {
			return true;
		}
	}
}

void CPartFile::UpdateAvailablePartsCount()
{
	UINT availablecounter = 0;
	UINT iPartCount = partCount();
	for (UINT ixPart = 0; ixPart < iPartCount; ixPart++){
		for (POSITION pos = srclist.GetHeadPosition(); pos; ){
			if (srclist.GetNext(pos)->IsPartAvailable(ixPart)){
				availablecounter++; 
				break;
			}
		}
	}
	if (iPartCount == availablecounter && availablePartsCount < iPartCount)
		lastseencomplete = CTime::GetCurrentTime();
	availablePartsCount = availablecounter;
}

Packet* CPartFile::CreateSrcInfoPacket(const CUpDownClient* forClient, uint8_t byRequestedVersion, uint16_t nRequestedOptions) const
{
	if (!IsPartFile() || srclist.IsEmpty())
		return CKnownFile::CreateSrcInfoPacket(forClient, byRequestedVersion, nRequestedOptions);

	if (md4cmp(forClient->GetUploadFileID(), GetFileHash()) != 0) {
		// should never happen
		DEBUG_ONLY( DebugLogError(_T("*** %hs - client (%s) upload file \"%s\" does not match file \"%s\""), __FUNCTION__, forClient->DbgGetClientInfo(), DbgGetFileInfo(forClient->GetUploadFileID()), GetFileName()) );
		ASSERT(0);
		return NULL;
	}

	// check whether client has either no download status at all or a download status which is valid for this file
	if (!(forClient->GetUpPartCount() == 0 && forClient->GetUpPartStatus() == NULL)
		&& !(forClient->GetUpPartCount() == partCount() && forClient->GetUpPartStatus() != NULL))
	{
		// should never happen
		DEBUG_ONLY( DebugLogError(_T("*** %hs - part count (%u) of client (%s) does not match part count (%u) of file \"%s\""), __FUNCTION__, forClient->GetUpPartCount(), forClient->DbgGetClientInfo(), partCount(), GetFileName()) );
		ASSERT(0);
		return NULL;
	}

	if (!(GetStatus() == PS_READY || GetStatus() == PS_EMPTY))
		return NULL;

	CSafeMemFile data(1024);
	
	uint8_t byUsedVersion;
	bool bIsSX2Packet;
	if (forClient->SupportsSourceExchange2() && byRequestedVersion > 0){
		// the client uses SourceExchange2 and requested the highest version he knows
		// and we send the highest version we know, but of course not higher than his request
		byUsedVersion = min(byRequestedVersion, (uint8_t)SOURCEEXCHANGE2_VERSION);
		bIsSX2Packet = true;
		data.WriteUInt8(byUsedVersion);

		// we don't support any special SX2 options yet, reserved for later use
		if (nRequestedOptions != 0)
			DebugLogWarning(_T("Client requested unknown options for SourceExchange2: %u (%s)"), nRequestedOptions, forClient->DbgGetClientInfo());
	}
	else{
		byUsedVersion = forClient->GetSourceExchange1Version();
		bIsSX2Packet = false;
		if (forClient->SupportsSourceExchange2())
			DebugLogWarning(_T("Client which announced to support SX2 sent SX1 packet instead (%s)"), forClient->DbgGetClientInfo());
	}

	UINT nCount = 0;
	data.WriteHash16(m_abyFileHash);
	data.WriteUInt16((uint16_t)nCount);
	
	bool bNeeded;
	const uint8_t* reqstatus = forClient->GetUpPartStatus();
	for (POSITION pos = srclist.GetHeadPosition();pos != 0;){
		bNeeded = false;
		const CUpDownClient* cur_src = srclist.GetNext(pos);
		if (cur_src->HasLowID() || !cur_src->IsValidSource())
			continue;
		const uint8_t* srcstatus = cur_src->GetPartStatus();
		if (srcstatus){
			if (cur_src->GetPartCount() == partCount()){
				if (reqstatus){
					ASSERT( forClient->GetUpPartCount() == partCount() );
					// only send sources which have needed parts for this client
					for (UINT x = 0; x < partCount(); x++){
						if (srcstatus[x] && !reqstatus[x]){
							bNeeded = true;
							break;
						}
					}
				}
				else{
					// We know this client is valid. But don't know the part count status.. So, currently we just send them.
					for (UINT x = 0; x < partCount(); x++){
						if (srcstatus[x]){
							bNeeded = true;
							break;
						}
					}
				}
			}
			else{
				// should never happen
				if (thePrefs.GetVerbose())
					DEBUG_ONLY(DebugLogError(_T("*** %hs - found source (%s) with wrong partcount (%u) attached to partfile \"%s\" (partcount=%u)"), __FUNCTION__, cur_src->DbgGetClientInfo(), cur_src->GetPartCount(), GetFileName(), partCount()));
			}
		}

		if (bNeeded){
			nCount++;
			uint32_t dwID;
			if (byUsedVersion >= 3)
				dwID = cur_src->GetUserIDHybrid();
			else
				dwID = ntohl(cur_src->GetUserIDHybrid());
			data.WriteUInt32(dwID);
			data.WriteUInt16(cur_src->GetUserPort());
			data.WriteUInt32(cur_src->GetServerIP());
			data.WriteUInt16(cur_src->GetServerPort());
			if (byUsedVersion >= 2)
				data.WriteHash16(cur_src->GetUserHash());
			if (byUsedVersion >= 4){
				// ConnectSettings - SourceExchange V4
				// 4 Reserved (!)
				// 1 DirectCallback Supported/Available 
				// 1 CryptLayer Required
				// 1 CryptLayer Requested
				// 1 CryptLayer Supported
				const uint8_t uSupportsCryptLayer	= cur_src->SupportsCryptLayer() ? 1 : 0;
				const uint8_t uRequestsCryptLayer	= cur_src->RequestsCryptLayer() ? 1 : 0;
				const uint8_t uRequiresCryptLayer	= cur_src->RequiresCryptLayer() ? 1 : 0;
				//const uint8 uDirectUDPCallback	= cur_src->SupportsDirectUDPCallback() ? 1 : 0;
				const uint8_t byCryptOptions = /*(uDirectUDPCallback << 3) |*/ (uRequiresCryptLayer << 2) | (uRequestsCryptLayer << 1) | (uSupportsCryptLayer << 0);
				data.WriteUInt8(byCryptOptions);
			}
			if (nCount > 500)
				break;
		}
	}
	if (!nCount)
		return 0;
	data.Seek(bIsSX2Packet ? 17 : 16, SEEK_SET);
	data.WriteUInt16((uint16_t)nCount);

	Packet* result = new Packet(&data, OP_EMULEPROT);
	result->opcode = bIsSX2Packet ? OP_ANSWERSOURCES2 : OP_ANSWERSOURCES;
	// (1+)16+2+501*(4+2+4+2+16+1) = 14547 (14548) bytes max.
	if (result->size > 354)
		result->PackPacket();
	if (thePrefs.GetDebugSourceExchange())
		AddDebugLogLine(false, _T("SXSend: Client source response SX2=%s, Version=%u; Count=%u, %s, File=\"%s\""), bIsSX2Packet ? _T("Yes") : _T("No"), byUsedVersion, nCount, forClient->DbgGetClientInfo(), GetFileName());
	return result;
}

void CPartFile::AddClientSources(CSafeMemFile* sources, uint8_t uClientSXVersion, bool bSourceExchange2, const CUpDownClient* pClient)
{
	if (stopped)
		return;

	UINT nCount = 0;

	if (thePrefs.GetDebugSourceExchange()) {
		CString strDbgClientInfo;
		if (pClient)
			strDbgClientInfo.Format(_T("%s, "), pClient->DbgGetClientInfo());
		AddDebugLogLine(false, _T("SXRecv: Client source response; SX2=%s, Ver=%u, %sFile=\"%s\""), bSourceExchange2 ? _T("Yes") : _T("No"), uClientSXVersion, strDbgClientInfo, GetFileName());
	}

	UINT uPacketSXVersion = 0;
	if (!bSourceExchange2){
		// for SX1 (deprecated):
		// Check if the data size matches the 'nCount' for v1 or v2 and eventually correct the source
		// exchange version while reading the packet data. Otherwise we could experience a higher
		// chance in dealing with wrong source data, userhashs and finally duplicate sources.
		nCount = sources->ReadUInt16();
		UINT uDataSize = (UINT)(sources->GetLength() - sources->GetPosition());
		// Checks if version 1 packet is correct size
		if (nCount*(4+2+4+2) == uDataSize)
		{
			// Received v1 packet: Check if remote client supports at least v1
			if (uClientSXVersion < 1) {
				if (thePrefs.GetVerbose()) {
					CString strDbgClientInfo;
					if (pClient)
						strDbgClientInfo.Format(_T("%s, "), pClient->DbgGetClientInfo());
					DebugLogWarning(_T("Received invalid SX packet (v%u, count=%u, size=%u), %sFile=\"%s\""), uClientSXVersion, nCount, uDataSize, strDbgClientInfo, GetFileName());
				}
				return;
			}
			uPacketSXVersion = 1;
		}
		// Checks if version 2&3 packet is correct size
		else if (nCount*(4+2+4+2+16) == uDataSize)
		{
			// Received v2,v3 packet: Check if remote client supports at least v2
			if (uClientSXVersion < 2) {
				if (thePrefs.GetVerbose()) {
					CString strDbgClientInfo;
					if (pClient)
						strDbgClientInfo.Format(_T("%s, "), pClient->DbgGetClientInfo());
					DebugLogWarning(_T("Received invalid SX packet (v%u, count=%u, size=%u), %sFile=\"%s\""), uClientSXVersion, nCount, uDataSize, strDbgClientInfo, GetFileName());
				}
				return;
			}
			if (uClientSXVersion == 2)
				uPacketSXVersion = 2;
			else
				uPacketSXVersion = 3;
		}
		// v4 packets
		else if (nCount*(4+2+4+2+16+1) == uDataSize)
		{
			// Received v4 packet: Check if remote client supports at least v4
			if (uClientSXVersion < 4) {
				if (thePrefs.GetVerbose()) {
					CString strDbgClientInfo;
					if (pClient)
						strDbgClientInfo.Format(_T("%s, "), pClient->DbgGetClientInfo());
					DebugLogWarning(_T("Received invalid SX packet (v%u, count=%u, size=%u), %sFile=\"%s\""), uClientSXVersion, nCount, uDataSize, strDbgClientInfo, GetFileName());
				}
				return;
			}
			uPacketSXVersion = 4;
		}
		else
		{
			// If v5+ inserts additional data (like v2), the above code will correctly filter those packets.
			// If v5+ appends additional data after <count>(<Sources>)[count], we are in trouble with the 
			// above code. Though a client which does not understand v5+ should never receive such a packet.
			if (thePrefs.GetVerbose()) {
				CString strDbgClientInfo;
				if (pClient)
					strDbgClientInfo.Format(_T("%s, "), pClient->DbgGetClientInfo());
				DebugLogWarning(_T("Received invalid SX packet (v%u, count=%u, size=%u), %sFile=\"%s\""), uClientSXVersion, nCount, uDataSize, strDbgClientInfo, GetFileName());
			}
			return;
		}
		ASSERT( uPacketSXVersion != 0 );
	}
	else{
		// for SX2:
		// We only check if the version is known by us and do a quick sanitize check on known version
		// other then SX1, the packet will be ignored if any error appears, sicne it can't be a "misunderstanding" anymore
		if (uClientSXVersion > SOURCEEXCHANGE2_VERSION || uClientSXVersion == 0){
			if (thePrefs.GetVerbose()) {
				CString strDbgClientInfo;
				if (pClient)
					strDbgClientInfo.Format(_T("%s, "), pClient->DbgGetClientInfo());

				DebugLogWarning(_T("Received invalid SX2 packet - Version unknown (v%u), %sFile=\"%s\""), uClientSXVersion, strDbgClientInfo, GetFileName());
			}
			return;
		}
		// all known versions use the first 2 bytes as count and unknown version are already filtered above
		nCount = sources->ReadUInt16();
		UINT uDataSize = (UINT)(sources->GetLength() - sources->GetPosition());	
		bool bError = false;
		switch (uClientSXVersion){
			case 1:
				bError = nCount*(4+2+4+2) != uDataSize;
				break;
			case 2:
			case 3:
				bError = nCount*(4+2+4+2+16) != uDataSize;
				break;
			case 4:
				bError = nCount*(4+2+4+2+16+1) != uDataSize;
				break;
			default:
				ASSERT( false );
		}

		if (bError){
			ASSERT( false );
			if (thePrefs.GetVerbose()) {
				CString strDbgClientInfo;
				if (pClient)
					strDbgClientInfo.Format(_T("%s, "), pClient->DbgGetClientInfo());
				DebugLogWarning(_T("Received invalid/corrupt SX2 packet (v%u, count=%u, size=%u), %sFile=\"%s\""), uClientSXVersion, nCount, uDataSize, strDbgClientInfo, GetFileName());
			}
			return;
		}
		uPacketSXVersion = uClientSXVersion;
	}

	for (UINT i = 0; i < nCount; i++)
	{
		uint32_t dwID = sources->ReadUInt32();
		uint16_t nPort = sources->ReadUInt16();
		uint32_t dwServerIP = sources->ReadUInt32();
		uint16_t nServerPort = sources->ReadUInt16();

		uchar achUserHash[16];
		if (uPacketSXVersion >= 2)
			sources->ReadHash16(achUserHash);

		uint8_t byCryptOptions = 0;
		if (uPacketSXVersion >= 4)
			byCryptOptions = sources->ReadUInt8();

		// Clients send ID's in the Hyrbid format so highID clients with *.*.*.0 won't be falsely switched to a lowID..
		if (uPacketSXVersion >= 3)
		{
			uint32_t dwIDED2K = ntohl(dwID);

			// check the HighID(IP) - "Filter LAN IPs" and "IPfilter" the received sources IP addresses
			if (!IsLowID(dwID))
			{
				if (!IsGoodIP(dwIDED2K))
				{
					// check for 0-IP, localhost and optionally for LAN addresses
					//if (thePrefs.GetLogFilteredIPs())
					//	AddDebugLogLine(false, _T("Ignored source (IP=%s) received via source exchange - bad IP"), ipstr(dwIDED2K));
					continue;
				}
				if (theApp.ipfilter->IsFiltered(dwIDED2K))
				{
					if (thePrefs.GetLogFilteredIPs())
						AddDebugLogLine(false, _T("Ignored source (IP=%s) received via source exchange - IP filter (%s)"), ipstr(dwIDED2K), theApp.ipfilter->GetLastHit());
					continue;
				}
				if (theApp.clientlist->IsBannedClient(dwIDED2K)){
#ifdef _DEBUG
					if (thePrefs.GetLogBannedClients()){
						CUpDownClient* pClient = theApp.clientlist->FindClientByIP(dwIDED2K);
						AddDebugLogLine(false, _T("Ignored source (IP=%s) received via source exchange - banned client %s"), ipstr(dwIDED2K), pClient->DbgGetClientInfo());
					}
#endif
					continue;
				}
			}

			// additionally check for LowID and own IP
			if (!CanAddSource(dwID, nPort, dwServerIP, nServerPort, NULL, false))
			{
				//if (thePrefs.GetLogFilteredIPs())
				//	AddDebugLogLine(false, _T("Ignored source (IP=%s) received via source exchange"), ipstr(dwIDED2K));
				continue;
			}
		}
		else
		{
			// check the HighID(IP) - "Filter LAN IPs" and "IPfilter" the received sources IP addresses
			if (!IsLowID(dwID))
			{
				if (!IsGoodIP(dwID))
				{ 
					// check for 0-IP, localhost and optionally for LAN addresses
					//if (thePrefs.GetLogFilteredIPs())
					//	AddDebugLogLine(false, _T("Ignored source (IP=%s) received via source exchange - bad IP"), ipstr(dwID));
					continue;
				}
				if (theApp.ipfilter->IsFiltered(dwID))
				{
					if (thePrefs.GetLogFilteredIPs())
						AddDebugLogLine(false, _T("Ignored source (IP=%s) received via source exchange - IP filter (%s)"), ipstr(dwID), theApp.ipfilter->GetLastHit());
					continue;
				}
				if (theApp.clientlist->IsBannedClient(dwID)){
#ifdef _DEBUG
					if (thePrefs.GetLogBannedClients()){
						CUpDownClient* pClient = theApp.clientlist->FindClientByIP(dwID);
						AddDebugLogLine(false, _T("Ignored source (IP=%s) received via source exchange - banned client %s"), ipstr(dwID), pClient->DbgGetClientInfo());
					}
#endif
					continue;
				}
			}

			// additionally check for LowID and own IP
			if (!CanAddSource(dwID, nPort, dwServerIP, nServerPort))
			{
				//if (thePrefs.GetLogFilteredIPs())
				//	AddDebugLogLine(false, _T("Ignored source (IP=%s) received via source exchange"), ipstr(dwID));
				continue;
			}
		}

		if (GetMaxSources() > GetSourceCount())
		{
			CUpDownClient* newsource;
			if (uPacketSXVersion >= 3)
				newsource = new CUpDownClient(this, nPort, dwID, dwServerIP, nServerPort, false);
			else
				newsource = new CUpDownClient(this, nPort, dwID, dwServerIP, nServerPort, true);
			if (uPacketSXVersion >= 2)
				newsource->SetUserHash(achUserHash);
			if (uPacketSXVersion >= 4) {
				newsource->SetConnectOptions(byCryptOptions, true, false);
				//if (thePrefs.GetDebugSourceExchange()) // remove this log later
				//	AddDebugLogLine(false, _T("Received CryptLayer aware (%u) source from V4 Sourceexchange (%s)"), byCryptOptions, newsource->DbgGetClientInfo());
			}
			newsource->SetSourceFrom(SF_SOURCE_EXCHANGE);
			theApp.downloadqueue->CheckAndAddSource(this, newsource);
		} 
		else
			break;
	}
}

// making this function return a higher when more sources have the extended
// protocol will force you to ask a larger variety of people for sources
/*int CPartFile::GetCommonFilePenalty() const
{
	//TODO: implement, but never return less than MINCOMMONPENALTY!
	return MINCOMMONPENALTY;
}
*/
/* Barry - Replaces BlockReceived() 

           Originally this only wrote to disk when a full 180k block 
           had been received from a client, and only asked for data in 
		   180k blocks.

		   This meant that on average 90k was lost for every connection
		   to a client data source. That is a lot of wasted data.

		   To reduce the lost data, packets are now written to a buffer
		   and flushed to disk regularly regardless of size downloaded.
		   This includes compressed packets.

		   Data is also requested only where gaps are, not in 180k blocks.
		   The requests will still not exceed 180k, but may be smaller to
		   fill a gap.
*/
uint32_t CPartFile::WriteToBuffer( Requested_Block_Struct* block )
{
	// Increment transferred bytes counter for this file
	m_uTransferred += block->transferred;

	// This is needed a few times
	uint32_t lenData = (uint32_t)(block->EndOffset - block->StartOffset + 1);

	if ( lenData > block->transferred ) {
		m_uCompressionGain += lenData - block->transferred;
		thePrefs.Add2SavedFromCompression(lenData - block->transferred);
	}

	// security sanitize check to make sure we do not write anything into an already hashed complete chunk
	ASSERT( !IsPartComplete((uint16_t)(block->StartOffset / PARTSIZE)) );
	ASSERT( !IsComplete(block->StartOffset, block->EndOffset, false) ); // collision check

	// Create a new buffered queue entry
	PartFileBufferedData *item = new PartFileBufferedData;
	item->start = block->StartOffset;
	item->end = block->EndOffset;
	item->data = block->data;
	block->data = NULL;
	
	m_BufferedData_list.push_back(item);

	// Increment buffer size marker
	m_nTotalBufferData += lenData;

	// Mark this small section of the file as filled
	FillGap(block->StartOffset, block->EndOffset);

	if ( gaplist.empty() ) {
		FlushBuffer();
	}

	// Return the length of data written to the buffer
	return lenData;
}

void CPartFile::FlushBuffer( bool forcewait )
{
	// wait here for flush finish
    if( isFlushing() ) {
        if( forcewait )
            WaitForSingleObject(m_waitFlushEvent, INFINITE);
        else
            return;
    }

    bool bIncreasedFile = false;
	m_nLastBufferFlushTime = GetTickCount();

	if ( 0 == m_nTotalBufferData )
        return;

	// diskspace is being allocated right now, so don't write and keep the data in the buffer for later.
	if ( NULL != m_AllocateThread ) {
		return;
	} else if ( m_iAllocinfo > 0 ) {
		bIncreasedFile = true;
		m_iAllocinfo = 0;
	}

	try
	{
		bool bCheckDiskspace = thePrefs.IsCheckDiskspaceEnabled() && thePrefs.GetMinFreeDiskSpace() > 0;
		ULONGLONG uFreeDiskSpace = bCheckDiskspace ? GetFreeDiskSpaceX(GetTempPath()) : 0;

		// Check free diskspace for compressed/sparse files before possibly increasing the file size
		if ( bCheckDiskspace && !IsNormalFile() )
		{
			// Compressed/sparse files; regardless whether the file is increased in size, 
			// check the amount of data which will be written
			// would need to use disk cluster sizes for more accuracy
			if ( m_nTotalBufferData + thePrefs.GetMinFreeDiskSpace() >= uFreeDiskSpace )
				AfxThrowFileException(CFileException::diskFull, 0, m_hpartfile.GetFileName());
		}

		// Ensure file is big enough to write data to (the last item will be the furthest from the start)
		if ( m_hpartfile.GetLength() < GetFileSize() )
		{
			uint64_t newsize = GetFileSize();
			ULONGLONG uIncrease = newsize - m_hpartfile.GetLength();

			// Check free diskspace for normal files before increasing the file size
			if ( bCheckDiskspace && IsNormalFile() )
			{
				// Normal files; check if increasing the file would reduce the amount of min. free space beyond the limit
				// would need to use disk cluster sizes for more accuracy
				if ( uIncrease + thePrefs.GetMinFreeDiskSpace() >= uFreeDiskSpace )
					AfxThrowFileException(CFileException::diskFull, 0, m_hpartfile.GetFileName());
			}

			// Allocate space for files>20mb and full space allocation policy
			if ( (thePrefs.GetSpaceAllocPolicy() == SPACE_ALLOC_FULL) && (GetFileSize() > 20*1024*1024ui64) ) {
				m_AllocateThread = CSafeThread::BeginThread(AllocateSpaceThread, this, THREAD_PRIORITY_BELOW_NORMAL, 0, CREATE_SUSPENDED);
				if ( m_AllocateThread == NULL )
					throw _T("Failed to create alloc thread! -> allocate blocking");
				m_iAllocinfo = newsize;
				m_AllocateThread->ResumeThread();
				return;
			}
		}

		ASSERT( m_hpartfile.GetLength() <= GetFileSize() );

	} catch ( CFileException* ex ) {
		PartFile_Io_Error *pfioerr = new PartFile_Io_Error;
		pfioerr->where = PFIOERR_NONE;
		pfioerr->exception = ex;
		IoExceptionHandler(pfioerr);	
	}

	for ( PartFileBufferedDataList::iterator it = m_BufferedData_list.begin(); it != m_BufferedData_list.end(); ++it ) {
		m_FlushData_list.push_back(*it);
	}
	m_BufferedData_list.clear();
	ResetEvent(m_waitFlushEvent);
	m_bIsFlushing = true;
	theApp.m_pFlushThread->PostThreadMessage(UM_FLUSHFILE, (WPARAM)this, (LPARAM)0);
}

void CPartFile::FlushDone()
{
	// Remember which parts need to be checked at the end of the flush
	bool* changedPart = (bool*)_alloca(partCount()*sizeof(bool));
	ZeroMemory(changedPart, partCount()*sizeof(bool));

	for ( PartFileBufferedDataList::iterator it = m_FlushData_list.begin();
		  it != m_FlushData_list.end(); ++it )
	{
        PartFileBufferedData* item = *it;
		
		if (  NULL != item->data )
			continue;

		// could be more than one part
		uint16_t lastpart = (uint16_t)(item->end/PARTSIZE);
                
		for ( uint16_t curpart = (uint16_t)(item->start/PARTSIZE); curpart <= lastpart; curpart++ )
			changedPart[curpart] = true;
                
		uint32_t lenData = (uint32_t)(item->end - item->start + 1);
		m_nTotalBufferData -= lenData;
		delete item;
	}
	m_FlushData_list.clear();

	// Check each changed part of the file only if hashset is full
	if ( hashset().count() == GetED2KPartCount() )
	{
		// Check each part of the file
		for ( uint16_t partNumber = 0; partNumber < partCount(); ++partNumber )
		{
			if ( !changedPart[partNumber] )
				continue;

			// Is this 9MB part complete and check if all data written to disk
			if ( IsComplete(PARTSIZE * partNumber, (PARTSIZE * (partNumber + 1)) - 1, false) )
			{
				m_PartsInfo[partNumber].isComplete = true;
                // Is part corrupt? let's check in another thread
				HashSinglePart(partNumber);
			}
		}
	}
	else {
		ASSERT(GetED2KPartCount() > 1);	// Files with only 1 chunk should have a forced hashset
		if ( m_eHashsetState != PFHS_REQUESTING )
			m_eHashsetState = PFHS_REQUIRED;
	}

	// Update met file
	SavePartFile();

	if (theApp.IsRunning()) // may be called during shutdown!
	{
		// Don't perform file completion here

		if (m_bPauseOnPreview && IsReadyForPreview())
		{
			m_bPauseOnPreview = false;
			PauseFile();
		}
	}
	SetFlushingFinished();

	// flush buffers again if gap list empty
	if ( gaplist.empty() && 0 != m_nTotalBufferData )
		FlushBuffer();
}


void CPartFile::IoExceptionHandler( PartFile_Io_Error *pfioerr )
{
	if ( pfioerr->where == PFIOERR_ONFLUSH ) {
		SetEvent(m_waitFlushEvent);
		m_bIsFlushing = false;
	} else if ( pfioerr->where == PFIOERR_ONHASH ) {
		m_partsOnHash--;
	} else if ( pfioerr->where == PFIOERR_ONALLOC ) {
		m_AllocateThread = NULL;
	}

	CFileException *error = pfioerr->exception;
	delete pfioerr;

	if ( thePrefs.IsCheckDiskspaceEnabled() && CFileException::diskFull == error->m_cause )
	{
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_OUTOFSPACE), GetFileName());
		if ( theApp.IsRunning() && thePrefs.GetNotifierOnImportantError() ) {
			CString msg;
			msg.Format(GetResString(IDS_ERR_OUTOFSPACE), GetFileName());
			theApp.emuledlg->ShowNotifier(msg, TBN_IMPORTANTEVENT);
		}

		// 'CFileException::diskFull' is also used for 'not enough min. free space'
		if ( theApp.IsRunning() )
		{
			if ( thePrefs.IsCheckDiskspaceEnabled() && thePrefs.GetMinFreeDiskSpace() == 0 )
				theApp.downloadqueue->CheckDiskspace(true);
			else
				PauseFile(true/*bInsufficient*/);
		}
	} else {
		if ( thePrefs.IsErrorBeepEnabled() )
			Beep(800, 200);

		if ( CFileException::diskFull == error->m_cause )
		{
			LogError(LOG_STATUSBAR, GetResString(IDS_ERR_OUTOFSPACE), GetFileName());
			// may be called during shutdown!
			if ( theApp.IsRunning() && thePrefs.GetNotifierOnImportantError() ) {
				CString msg;
				msg.Format(GetResString(IDS_ERR_OUTOFSPACE), GetFileName());
				theApp.emuledlg->ShowNotifier(msg, TBN_IMPORTANTEVENT);
			}
		}
		else
		{
			TCHAR buffer[MAX_CFEXP_ERRORMSG];
			error->GetErrorMessage(buffer,_countof(buffer));
			LogError(LOG_STATUSBAR, GetResString(IDS_ERR_WRITEERROR), GetFileName(), buffer);
			SetStatus(PS_ERROR);
		}
		paused = true;
		m_iLastPausePurge = time(NULL);
		theApp.downloadqueue->RemoveLocalServerRequest(this);
		datarate = 0;
		m_anStates[DS_DOWNLOADING] = 0;
	}

	if ( theApp.IsRunning() ) // may be called during shutdown!
		UpdateDisplayedInfo();

	error->Delete();
}

UINT AFX_CDECL CPartFile::AllocateSpaceThread( LPVOID lpParam )
{
	DbgSetThreadName("Partfile-Allocate Space");
	InitThreadLocale();

	CReadWriteLock lock(&theApp.m_threadlock);
	if ( !lock.ReadLock(0) )
		return 0;

    CPartFile* owner = (CPartFile*)lpParam;
	theApp.QueueDebugLogLine(false, _T("ALLOC:Start (%s) (%s)"), owner->GetFileName(), CastItoXBytes(owner->m_iAllocinfo, false, false) );

	try {
        CSingleLock hashing_lock(&theApp.hashing_mut, TRUE);

		// If this is a NTFS compressed file and the current block is the 1st one to be written and there is not 
		// enough free disk space to hold the entire *uncompressed* file, windows throws a 'diskFull'!?
		owner->m_hpartfile.SetLength(owner->m_iAllocinfo); // allocate disk space (may throw 'diskFull')

		// force the alloc, by temporary writing a non zero to the file end
		byte x = 255;
		owner->m_hpartfile.Seek(-1, CFile::end);
		owner->m_hpartfile.Write(&x, 1);
		owner->m_hpartfile.Flush();
	} catch ( CFileException* ex ) {
		PartFile_Io_Error *pfioerr = new PartFile_Io_Error;
		pfioerr->where = PFIOERR_ONALLOC;
		pfioerr->exception = ex;
		theApp.emuledlg->PostMessage(TM_PARTFILE_IOERR, (WPARAM)owner, (LPARAM)pfioerr);
		return 1;
	}
#ifndef _DEBUG
	catch(...) {
		ASSERT( FALSE );
	}
#endif

	owner->m_AllocateThread = NULL;
	theApp.QueueDebugLogLine(false, _T("ALLOC:End (%s)"), owner->GetFileName());
	return 0;
}

// Barry - This will invert the gap list, up to caller to delete gaps when done
// 'Gaps' returned are really the filled areas, and guaranteed to be in order
void CPartFile::GetFilledList( GapList *filled ) const
{
	if ( gaplist.empty() )
		return;

	Gap_Struct *gap = NULL;
	Gap_Struct *best = NULL;
	uint64_t start = 0;
	uint64_t bestEnd = 0;

	// Loop until done
	bool finished = false;
	while (!finished)
	{
		finished = true;
		// Find first gap after current start pos
		bestEnd = m_nFileSize;
		for ( GapList::const_iterator it = gaplist.begin(); it != gaplist.end(); ++it ) {
			gap = *it;
			if ( (gap->start >= start) && (gap->end < bestEnd) )
			{
				best = gap;
				bestEnd = best->end;
				finished = false;
			}
		}

		// TODO: here we have a problem - it occurred that eMule crashed because of "best==NULL" while
		// recovering an archive which was currently in "completing" state...
		if ( best == NULL ) {
			ASSERT(0);
			return;
		}

		if ( !finished )
		{
			if ( best->start > 0 ) {
				// Invert this gap
				gap = new Gap_Struct;
				gap->start = start;
				gap->end = best->start - 1;
				filled->push_back(gap);
				start = best->end + 1;
			} else 				
				start = best->end + 1;

		}
		else if ( best->end+1 < m_nFileSize )
		{
			gap = new Gap_Struct;
			gap->start = best->end + 1;
			gap->end = m_nFileSize;
			filled->push_back(gap);
		}
	}
}

void CPartFile::UpdateFileRatingCommentAvail( bool bForceUpdate )
{
	bool bOldHasComment = m_bHasComment;
	UINT uOldUserRatings = m_uUserRating;

	m_bHasComment = false;
	UINT uRatings = 0;
	UINT uUserRatings = 0;

	for ( POSITION pos = srclist.GetHeadPosition(); pos != NULL; )
	{
		const CUpDownClient* cur_src = srclist.GetNext(pos);
		if (!m_bHasComment && cur_src->HasFileComment())
			m_bHasComment = true;
		if (cur_src->HasFileRating())
		{
			uRatings++;
			uUserRatings += cur_src->GetFileRating();
		}
	}
	for( POSITION pos = m_kadNotes.GetHeadPosition(); pos != NULL; )
	{
		Kademlia::CEntry* entry = m_kadNotes.GetNext(pos);
		if (!m_bHasComment && !entry->GetStrTagValue(TAG_DESCRIPTION).IsEmpty())
			m_bHasComment = true;
		UINT rating = (UINT)entry->GetIntTagValue(TAG_FILERATING);
		if (rating != 0)
		{
			uRatings++;
			uUserRatings += rating;
		}
	}

	if (uRatings)
		m_uUserRating = (uint32_t)ROUND((float)uUserRatings / uRatings);
	else
		m_uUserRating = 0;

	if (bOldHasComment != m_bHasComment || uOldUserRatings != m_uUserRating || bForceUpdate)
		UpdateDisplayedInfo(true);
}

void CPartFile::UpdateDisplayedInfo(bool force)
{
	if (theApp.IsRunning()){
		DWORD curTick = ::GetTickCount();

        if(force || curTick-m_lastRefreshedDLDisplay > MINWAIT_BEFORE_DLDISPLAY_WINDOWUPDATE+m_random_update_wait) {
			theApp.emuledlg->transferwnd->GetDownloadList()->UpdateItem(this);
			m_lastRefreshedDLDisplay = curTick;
		}
	}
}

void CPartFile::UpdateAutoDownPriority(){
	if( !IsAutoDownPriority() )
		return;
	if ( GetSourceCount() > 100 ){
		SetDownPriority( PR_LOW );
		return;
	}
	if ( GetSourceCount() > 20 ){
		SetDownPriority( PR_NORMAL );
		return;
	}
	SetDownPriority( PR_HIGH );
}

UINT CPartFile::GetCategory() /*const*/
{
	if (m_category > (UINT)(thePrefs.GetCatCount() - 1))
		m_category = 0;
	return m_category;
}

bool CPartFile::HasDefaultCategory() const // extra function for const 
{
	return m_category == 0 || m_category > (UINT)(thePrefs.GetCatCount() - 1);
}

// Ornis: Creating progressive presentation of the partfilestatuses - for webdisplay
CString CPartFile::GetProgressString(uint16_t size) const
{
	char crProgress = '0';//green
	char crHave = '1';	// black
	char crPending='2';	// yellow
	char crMissing='3';  // red
	
	char crWaiting[6];
	crWaiting[0]='4'; // blue few source
	crWaiting[1]='5';
	crWaiting[2]='6';
	crWaiting[3]='7';
	crWaiting[4]='8';
	crWaiting[5]='9'; // full sources

	CString my_ChunkBar;
	for (uint16_t i=0;i<=size+1;i++) my_ChunkBar.AppendChar(crHave);	// one more for safety

	float unit= (float)size/(float)m_nFileSize;

	if(GetStatus() == PS_COMPLETE || GetStatus() == PS_COMPLETING) {
		CharFillRange(&my_ChunkBar,0,(uint32_t)((uint64_t)m_nFileSize*unit), crProgress);
	} else
	    // red gaps
	    for ( GapList::const_iterator it = gaplist.begin(); it != gaplist.end(); ++it ) {
		    Gap_Struct* cur_gap = *it;
		    bool gapdone = false;
		    uint64_t gapstart = cur_gap->start;
		    uint64_t gapend = cur_gap->end;
		    for (UINT i = 0; i < partCount(); i++){
			    if (gapstart >= (uint64_t)i*PARTSIZE && gapstart <=  (uint64_t)(i+1)*PARTSIZE){ // is in this part?
				    if (gapend <= (uint64_t)(i+1)*PARTSIZE)
					    gapdone = true;
				    else{
					    gapend = (uint64_t)(i+1)*PARTSIZE; // and next part
				    }
				    // paint
				    uint8_t color;
				    if (m_PartsInfo.GetCount() >= (INT_PTR)i && m_PartsInfo[(uint16_t)i].frequency)  // frequency?
					    //color = crWaiting;
					    color = m_PartsInfo[(uint16_t)i].frequency <  10 ? crWaiting[m_PartsInfo[(uint16_t)i].frequency/2]:crWaiting[5];
				    else
					    color = crMissing;
    
				    CharFillRange(&my_ChunkBar,(uint32_t)(gapstart*unit), (uint32_t)(gapend*unit + 1),  color);
    
				    if (gapdone) // finished?
					    break;
				    else{
					    gapstart = gapend;
					    gapend = cur_gap->end;
				    }
			    }
		    }
	    }

	// yellow pending parts
	for ( RequestedBlockList::const_iterator it = requestedblocks_list.begin(); it != requestedblocks_list.end(); ++it ) {
		Requested_Block_Struct* block =  *it;
		CharFillRange(&my_ChunkBar, (uint32_t)((block->StartOffset + block->transferred)*unit), (uint32_t)(block->EndOffset*unit),  crPending);
	}

	return my_ChunkBar;
}

void CPartFile::CharFillRange(CString* buffer, uint32_t start, uint32_t end, char color) const
{
	for (uint32_t i = start; i <= end;i++)
		buffer->SetAt(i, color);
}

void CPartFile::SetCategory(UINT cat)
{
	m_category=cat;
	
// ZZ:DownloadManager -->
	// set new prio
	if (IsPartFile()){
		SavePartFile();
	}
// <-- ZZ:DownloadManager
}

void CPartFile::_SetStatus(EPartFileStatus eStatus)
{
	// NOTE: This function is meant to be used from *different* threads -> Do *NOT* call
	// any GUI functions from within here!!
	ASSERT( eStatus != PS_PAUSED && eStatus != PS_INSUFFICIENT );
	status = eStatus;
}

void CPartFile::SetStatus(EPartFileStatus eStatus)
{
	_SetStatus(eStatus);
	if (theApp.IsRunning())
	{
		NotifyStatusChange();
		UpdateDisplayedInfo(true);
		if (thePrefs.ShowCatTabInfos())
			theApp.emuledlg->transferwnd->UpdateCatTabTitles();
	}
}

void CPartFile::NotifyStatusChange()
{
	if (theApp.IsRunning())
		theApp.emuledlg->transferwnd->GetDownloadList()->UpdateCurrentCategoryView(this);
}

EMFileSize CPartFile::GetRealFileSize() const
{
	return ::GetDiskFileSize(GetFilePath());
}

uint8_t* CPartFile::MMCreatePartStatus(){
	// create partstatus + info in mobilemule protocol specs
	// result needs to be deleted[] | slow, but not timecritical
	uint8_t* result = new uint8_t[partCount()+1];
	for (uint16_t i = 0; i < partCount(); i++){
		result[i] = 0;
		if (IsPartComplete(i)){
			result[i] = 1;
			continue;
		}
		else{
			if (IsComplete((uint64_t)i*PARTSIZE + (0*(PARTSIZE/3)), (((uint64_t)i*PARTSIZE)+(1*(PARTSIZE/3)))-1, false))
				result[i] += 2;
			if (IsComplete((uint64_t)i*PARTSIZE+ (1*(PARTSIZE/3)), (((uint64_t)i*PARTSIZE)+(2*(PARTSIZE/3)))-1, false))
				result[i] += 4;
			if (IsComplete((uint64_t)i*PARTSIZE+ (2*(PARTSIZE/3)), (((uint64_t)i*PARTSIZE)+(3*(PARTSIZE/3)))-1, false))
				result[i] += 8;
			uint8_t freq;
			if (m_PartsInfo.GetCount() > (signed)i)
				freq = (uint8_t)m_PartsInfo[i].frequency;
			else
				freq = 0;

			if (freq > 44)
				freq = 44;
			freq = (uint8_t)ceilf((float)freq/3);
			freq = (uint8_t)(freq << 4);
			result[i] = (uint8_t)(result[i] + freq);
		}

	}
	return result;
};

UINT CPartFile::GetSrcStatisticsValue(EDownloadState nDLState) const
{
	ASSERT( nDLState < _countof(m_anStates) );
	return m_anStates[nDLState];
}

UINT CPartFile::GetTransferringSrcCount() const
{
	return GetSrcStatisticsValue(DS_DOWNLOADING);
}

// [Maella -Enhanced Chunk Selection- (based on jicxicmic)]

//#pragma pack(1)
struct Chunk {
	uint16_t part;			// Index of the chunk
	union {
		uint16_t frequency;	// Availability of the chunk
		uint16_t rank;		// Download priority factor (highest = 0, lowest = 0xffff)
	};
};
//#pragma pack()

bool CPartFile::GetNextRequestedBlock(CUpDownClient* sender, 
                                      Requested_Block_Struct** newblocks, 
									  uint16_t* count) /*const*/
{
	// The purpose of this function is to return a list of blocks (~180KB) to
	// download. To avoid a prematurely stop of the downloading, all blocks that 
	// are requested from the same source must be located within the same 
	// chunk (=> part ~9MB).
	//  
	// The selection of the chunk to download is one of the CRITICAL parts of the 
	// edonkey network. The selection algorithm must insure the best spreading
	// of files.
	//  
	// The selection is based on several criteria:
	//  -   Frequency of the chunk (availability), very rare chunks must be downloaded 
	//      as quickly as possible to become a new available source.
	//  -   Parts used for preview (first + last chunk), preview or check a 
	//      file (e.g. movie, mp3)
	//  -   Completion (shortest-to-complete), partially retrieved chunks should be 
	//      completed before starting to download other one.
	//  
	// The frequency criterion defines several zones: very rare, rare, almost rare,
	// and common. Inside each zone, the criteria have a specific ëweightí, used 
	// to calculate the priority of chunks. The chunk(s) with the highest 
	// priority (highest=0, lowest=0xffff) is/are selected first.
	//  
	// This algorithm usually selects first the rarest chunk(s). However, partially
	// complete chunk(s) that is/are close to completion may overtake the priority 
	// (priority inversion). For common chunks, it also tries to put the transferring
    // clients on the same chunk, to complete it sooner.
	//
	// Check input parameters
	if(count == 0)
		return false;
	if(sender->GetPartStatus() == NULL)
		return false;

    //AddDebugLogLine(DLP_VERYLOW, false, _T("Evaluating chunks for file: \"%s\" Client: %s"), GetFileName(), sender->DbgGetClientInfo());
    
	// Define and create the list of the chunks to download
	CList<Chunk> chunksList(partCount());

    uint16_t tempLastPartAsked = (uint16_t)-1;
    if(sender->m_lastPartAsked != ((uint16_t)-1) && sender->GetClientSoft() == SO_EMULE && sender->GetVersion() < MAKE_CLIENT_VERSION(0, 43, 1)){
        tempLastPartAsked = sender->m_lastPartAsked;
    }

	// ismod: stream media preview
	// criterions:
	// 1) src count > 10
	// 2) speed more than 1mb
	// 3) trigger state
	const bool isStreamMedia = 
		(thePrefs.GetPreviewPrio() || thePrefs.IsExtControlsEnabled() && GetPreviewPrio())&&
		IsMovie() &&
		((this->GetSourceCount()) > 10) &&
		(this->GetDatarate() > 1*1024*1024) && // 1mbyte/sec
		!m_fPreviewSrcTrigger;
	m_fPreviewSrcTrigger = !m_fPreviewSrcTrigger;

	// ismod: dynamic block size [start]
	uint32_t blockSize = EMBLOCKSIZE;
	if ( blockSize > sender->GetDownloadDatarate()*5 ) { // block size = datarate*5secs
		blockSize = sender->GetDownloadDatarate()*5;
		if ( (blockSize < 10*1024) )
			blockSize = 10*1024;
	}
	// ismod [end]
	
	// Main loop
	uint16_t newBlockCount = 0;
	while(newBlockCount != *count){
		// Create a request block structure if a chunk has been previously selected
		if(tempLastPartAsked != (uint16_t)-1){
			Requested_Block_Struct* pBlock = new Requested_Block_Struct;
			if(GetNextEmptyBlockInPart(tempLastPartAsked, pBlock, blockSize) == true){
                //AddDebugLogLine(false, _T("Got request block. Interval %i-%i. File %s. Client: %s"), pBlock->StartOffset, pBlock->EndOffset, GetFileName(), sender->DbgGetClientInfo());
				// Keep a track of all pending requested blocks
				requestedblocks_list.push_back(pBlock);
				// Update list of blocks to return
				newblocks[newBlockCount++] = pBlock;
				// Skip end of loop (=> CPU load)
				continue;
			}
			// All blocks for this chunk have been already requested
			delete pBlock;
			// => Try to select another chunk
			sender->m_lastPartAsked = tempLastPartAsked = (uint16_t)-1;
		}

		// Check if a new chunk must be selected (e.g. download starting, previous chunk complete)
		if(tempLastPartAsked == (uint16_t)-1){

			// Quantify all chunks (create list of chunks to download) 
			// This is done only one time and only if it is necessary (=> CPU load)
			if(chunksList.IsEmpty()){
				// ismod: check two last parts for stream media
				if ( isStreamMedia && (partCount() > 2) && !IsPartComplete(partCount()-1) 
					&& sender->IsPartAvailable(partCount()-1) && GetNextEmptyBlockInPart(partCount()-1, NULL)) {
					// Create a new entry for this chunk and add it to the list
					Chunk newEntry;
					newEntry.part = partCount()-1;
					newEntry.frequency = m_PartsInfo[partCount()-1].frequency;
					chunksList.AddTail(newEntry);
				} else if ( isStreamMedia && (partCount() > 3) && !IsPartComplete(partCount()-2) 
							&& sender->IsPartAvailable(partCount()-2) && GetNextEmptyBlockInPart(partCount()-2, NULL)) {
					// Create a new entry for this chunk and add it to the list
					Chunk newEntry;
					newEntry.part = partCount()-2;
					newEntry.frequency = m_PartsInfo[partCount()-2].frequency;
					chunksList.AddTail(newEntry);
				} else {
					// Identify the locally missing part(s) that this source has
					for(uint16_t i = 0; i < partCount(); i++){
						// ismod: do not search in completed parts
						if ( !IsPartComplete(i) && sender->IsPartAvailable(i) && GetNextEmptyBlockInPart(i, NULL) ) {
							// Create a new entry for this chunk and add it to the list
							Chunk newEntry;
							newEntry.part = i;
							newEntry.frequency = m_PartsInfo[i].frequency;
							chunksList.AddTail(newEntry);
							// ismod: stream media
							if ( isStreamMedia ) {
								break;
							}
						}
					}
				}

				// ismod: 
				if (chunksList.GetCount() == 1)
					continue;

				// Check if any block(s) could be downloaded
				if(chunksList.IsEmpty() == TRUE){
					break; // Exit main loop while()
				}

                // Define the bounds of the zones (very rare, rare etc)
				// more depending on available sources
				uint16_t limit = (uint16_t)ceil(GetSourceCount()/ 10.0);
				if (limit<3) limit=3;

				const uint16_t veryRareBound = limit;
				const uint16_t rareBound = 2*limit;
				const uint16_t almostRareBound = 4*limit;

				// Cache Preview state (Criterion 2)
				const bool isPreviewEnable = (thePrefs.GetPreviewPrio() || thePrefs.IsExtControlsEnabled() && GetPreviewPrio()) && IsPreviewableFileType();

				// Collect and calculate criteria for all chunks
				for(POSITION pos = chunksList.GetHeadPosition(); pos != NULL; ){
					Chunk& cur_chunk = chunksList.GetNext(pos);

					// Offsets of chunk
					UINT uCurChunkPart = cur_chunk.part; // help VC71...
					const uint64_t uStart = (uint64_t)uCurChunkPart * PARTSIZE;
					const uint64_t uEnd  = ((GetFileSize() - (uint64_t)1) < (uStart + PARTSIZE - 1)) ? 
										  (GetFileSize() - (uint64_t)1) : (uStart + PARTSIZE - 1);
					ASSERT( uStart <= uEnd );

					// Criterion 2. Parts used for preview
					// Remark: - We need to download the first part and the last part(s).
					//        - When the last part is very small, it's necessary to 
					//          download the two last parts.
					bool critPreview = false;
					if(isPreviewEnable == true){
						if(cur_chunk.part == 0){
							critPreview = true; // First chunk
						}
						else if(cur_chunk.part == partCount()-1){
							critPreview = true; // Last chunk 
						}
						else if(cur_chunk.part == partCount()-2){
							// Last chunk - 1 (only if last chunk is too small)
							if( (GetFileSize() - uEnd) < (uint64_t)PARTSIZE/3){
								critPreview = true; // Last chunk - 1
							}
						}
					}

					// Criterion 3. Request state (downloading in process from other source(s))
					//const bool critRequested = IsAlreadyRequested(uStart, uEnd);
                    bool critRequested = false; // <--- This is set as a part of the second critCompletion loop below

					// Criterion 4. Completion
					uint64_t partSize = uEnd - uStart + 1; //If all is covered by gaps, we have downloaded PARTSIZE, or possibly less for the last chunk;
                    ASSERT(partSize <= PARTSIZE);
					for ( GapList::iterator it = gaplist.begin(); it != gaplist.end(); ++it ) {
						const Gap_Struct* cur_gap = *it;
						// Check if Gap is into the limit
						if(cur_gap->start < uStart) {
							if(cur_gap->end > uStart && cur_gap->end < uEnd) {
                                ASSERT(partSize >= (cur_gap->end - uStart + 1));
								partSize -= cur_gap->end - uStart + 1;
							}
							else if(cur_gap->end >= uEnd) {
								partSize = 0;
								break; // exit loop for()
							}
						}
						else if(cur_gap->start <= uEnd) {
							if(cur_gap->end < uEnd) {
                                ASSERT(partSize >= (cur_gap->end - cur_gap->start + 1));
								partSize -= cur_gap->end - cur_gap->start + 1;
							}
							else {
                                ASSERT(partSize >= (uEnd - cur_gap->start + 1));
								partSize -= uEnd - cur_gap->start + 1;
							}
						}
					}
                    //ASSERT(partSize <= PARTSIZE && partSize <= (uEnd - uStart + 1));

                    // requested blocks from sources we are currently downloading from is counted as if already downloaded
                    // this code will cause bytes that has been requested AND transferred to be counted twice, so we can end
                    // up with a completion number > PARTSIZE. That's ok, since it's just a relative number to compare chunks.
                    for ( RequestedBlockList::const_iterator it = requestedblocks_list.begin(); it != requestedblocks_list.end(); ++it ) {
                        const Requested_Block_Struct* reqBlock = *it;
                        if(reqBlock->StartOffset < uStart) {
                            if(reqBlock->EndOffset > uStart) {
                                if(reqBlock->EndOffset < uEnd) {
                                    //ASSERT(partSize + (reqBlock->EndOffset - uStart + 1) <= (uEnd - uStart + 1));
								    partSize += reqBlock->EndOffset - uStart + 1;
                                    critRequested = true;
                                } else if(reqBlock->EndOffset >= uEnd) {
                                    //ASSERT(partSize + (uEnd - uStart + 1) <= uEnd - uStart);
                                    partSize += uEnd - uStart + 1;
                                    critRequested = true;
                                }
							}
                        } else if(reqBlock->StartOffset <= uEnd) {
							if(reqBlock->EndOffset < uEnd) {
                                //ASSERT(partSize + (reqBlock->EndOffset - reqBlock->StartOffset + 1) <= (uEnd - uStart + 1));
								partSize += reqBlock->EndOffset - reqBlock->StartOffset + 1;
                                critRequested = true;
							} else {
                                //ASSERT(partSize +  (uEnd - reqBlock->StartOffset + 1) <= (uEnd - uStart + 1));
								partSize += uEnd - reqBlock->StartOffset + 1;
                                critRequested = true;
							}
						}
                    }
                    //Don't check this (see comment above for explanation): ASSERT(partSize <= PARTSIZE && partSize <= (uEnd - uStart + 1));

                    if(partSize > PARTSIZE) partSize = PARTSIZE;

                    uint16_t critCompletion = (uint16_t)ceil((double)(partSize*100)/PARTSIZE); // in [%]. Last chunk is always counted as a full size chunk, to not give it any advantage in this comparison due to smaller size. So a 1/3 of PARTSIZE downloaded in last chunk will give 33% even if there's just one more byte do download to complete the chunk.
                    if(critCompletion > 100) critCompletion = 100;

                    // Criterion 5. Prefer to continue the same chunk
                    const bool sameChunk = (cur_chunk.part == sender->m_lastPartAsked);

                    // Criterion 6. The more transferring clients that has this part, the better (i.e. lower).
                    uint16_t transferringClientsScore = (uint16_t)m_downloadingSourceList.GetSize();

                    // Criterion 7. Sooner to completion (how much of a part is completed, how fast can be transferred to this part, if all currently transferring clients with this part are put on it. Lower is better.)
                    uint16_t bandwidthScore = 2000;

                    // Calculate criterion 6 and 7
                    if(m_downloadingSourceList.GetSize() > 1) {
                        UINT totalDownloadDatarateForThisPart = 1;
                        for(POSITION downloadingClientPos = m_downloadingSourceList.GetHeadPosition(); downloadingClientPos != NULL; ) {
                            const CUpDownClient* downloadingClient = m_downloadingSourceList.GetNext(downloadingClientPos);
                            if(downloadingClient->IsPartAvailable(cur_chunk.part)) {
                                transferringClientsScore--;
                                totalDownloadDatarateForThisPart += downloadingClient->GetDownloadDatarate() + 500; // + 500 to make sure that a unstarted chunk available at two clients will end up just barely below 2000 (max limit)
                            }
                        }

                        bandwidthScore = min<uint16_t>((uint16_t)((PARTSIZE-partSize)/(totalDownloadDatarateForThisPart*5)), 2000);
                        //AddDebugLogLine(DLP_VERYLOW, false,
                        //    _T("BandwidthScore for chunk %i: bandwidthScore = %u = min((PARTSIZE-partSize)/(totalDownloadDatarateForThisChunk*5), 2000) = min((PARTSIZE-%I64u)/(%u*5), 2000)"),
                        //    cur_chunk.part, bandwidthScore, partSize, totalDownloadDatarateForThisChunk);
                    }

                    //AddDebugLogLine(DLP_VERYLOW, false, _T("Evaluating chunk number: %i, SourceCount: %u/%i, critPreview: %s, critRequested: %s, critCompletion: %i%%, sameChunk: %s"), cur_chunk.part, cur_chunk.frequency, GetSourceCount(), ((critPreview == true) ? _T("true") : _T("false")), ((critRequested == true) ? _T("true") : _T("false")), critCompletion, ((sameChunk == true) ? _T("true") : _T("false")));

					// Calculate priority with all criteria
                    if(partSize > 0 && GetSourceCount() <= GetSrcA4AFCount()) {
						// If there are too many a4af sources, the completion of blocks have very high prio
						cur_chunk.rank = (cur_chunk.frequency) +                      // Criterion 1
							             ((critPreview == true) ? 0 : 200) +          // Criterion 2
										 ((critRequested == true) ? 0 : 1) +          // Criterion 3
										 (100 - critCompletion) +                     // Criterion 4
                                         ((sameChunk == true) ? 0 : 1) +              // Criterion 5
                                         bandwidthScore;                              // Criterion 7
                    } else if(cur_chunk.frequency <= veryRareBound){
						// 3000..xxxx unrequested + requested very rare chunks
						cur_chunk.rank = (75 * cur_chunk.frequency) +                 // Criterion 1
							             ((critPreview == true) ? 0 : 1) +            // Criterion 2
										 ((critRequested == true) ? 3000 : 3001) +    // Criterion 3
										 (100 - critCompletion) +                     // Criterion 4
                                         ((sameChunk == true) ? 0 : 1) +              // Criterion 5
                                         transferringClientsScore;                    // Criterion 6
					}
					else if(critPreview == true){
						// 10000..10100  unrequested preview chunks
						// 20000..20100  requested preview chunks
						cur_chunk.rank = ((critRequested == true &&
                                           sameChunk == false) ? 20000 : 10000) +     // Criterion 3
										 (100 - critCompletion);                      // Criterion 4
					}
					else if(cur_chunk.frequency <= rareBound){
						// 10101..1xxxx  requested rare chunks
						// 10102..1xxxx  unrequested rare chunks
                        //ASSERT(cur_chunk.frequency >= veryRareBound);

                        cur_chunk.rank = (25 * cur_chunk.frequency) +                 // Criterion 1 
										 ((critRequested == true) ? 10101 : 10102) +  // Criterion 3
										 (100 - critCompletion) +                     // Criterion 4
                                         ((sameChunk == true) ? 0 : 1) +              // Criterion 5
                                         transferringClientsScore;                    // Criterion 6
					}
					else if(cur_chunk.frequency <= almostRareBound){
						// 20101..1xxxx  requested almost rare chunks
						// 20150..1xxxx  unrequested almost rare chunks
                        //ASSERT(cur_chunk.frequency >= rareBound);

                        // used to slightly lessen the imporance of frequency
                        uint16_t randomAdd = 1 + (uint16_t)((((uint32_t)rand()*(almostRareBound-rareBound))+(RAND_MAX/2))/RAND_MAX);
                        //AddDebugLogLine(DLP_VERYLOW, false, _T("RandomAdd: %i, (%i-%i=%i)"), randomAdd, rareBound, almostRareBound, almostRareBound-rareBound);

                        cur_chunk.rank = (cur_chunk.frequency) +                      // Criterion 1
										 ((critRequested == true) ? 20101 : (20201+almostRareBound-rareBound)) +  // Criterion 3
                                         ((partSize > 0) ? 0 : 500) +                 // Criterion 4
										 (5*100 - (5*critCompletion)) +               // Criterion 4
                                         ((sameChunk == true) ? (uint16_t)0 : randomAdd) +  // Criterion 5
                                         bandwidthScore;                              // Criterion 7
					}
					else { // common chunk
						// 30000..30100  requested common chunks
						// 30001..30101  unrequested common chunks
						cur_chunk.rank = ((critRequested == true) ? 30000 : 30001) +  // Criterion 3
										 (100 - critCompletion) +                     // Criterion 4
                                         ((sameChunk == true) ? 0 : 1) +              // Criterion 5
                                         bandwidthScore;                              // Criterion 7
					}

                    //AddDebugLogLine(DLP_VERYLOW, false, _T("Rank: %u"), cur_chunk.rank);
				}
			}

			// Select the next chunk to download
			if(chunksList.IsEmpty() == FALSE){
				// Find and count the chunck(s) with the highest priority
				uint16_t count = 0; // Number of found chunks with same priority
				uint16_t rank = 0xffff; // Highest priority found
				for(POSITION pos = chunksList.GetHeadPosition(); pos != NULL; ){
					const Chunk& cur_chunk = chunksList.GetNext(pos);
					if(cur_chunk.rank < rank){
						count = 1;
						rank = cur_chunk.rank;
					}
					else if(cur_chunk.rank == rank){
						count++;
					}
				}

				// Use a random access to avoid that everybody tries to download the 
				// same chunks at the same time (=> spread the selected chunk among clients)
				uint16_t randomness = 1 + (uint16_t)((((uint32_t)rand()*(count-1))+(RAND_MAX/2))/RAND_MAX);
				for(POSITION pos = chunksList.GetHeadPosition(); ; ){
					POSITION cur_pos = pos;
					const Chunk& cur_chunk = chunksList.GetNext(pos);
					if(cur_chunk.rank == rank){
						randomness--; 
						if(randomness == 0){
							// Selection process is over 
                            sender->m_lastPartAsked = tempLastPartAsked = cur_chunk.part;
                            //AddDebugLogLine(DLP_VERYLOW, false, _T("Chunk number %i selected. Rank: %u"), cur_chunk.part, cur_chunk.rank);

							// Remark: this list might be reused up to ë*countí times
							chunksList.RemoveAt(cur_pos);
							break; // exit loop for()
						}  
					}
				}
			}
			else {
				// There is no remaining chunk to download
				break; // Exit main loop while()
			}
		}
	}
	// Return the number of the blocks 
	*count = newBlockCount;
	
	// Return
	return (newBlockCount > 0);
}
// Maella end


CString CPartFile::GetInfoSummary(bool bNoFormatCommands) const
{
	if (!IsPartFile())
		return CKnownFile::GetInfoSummary();

	CString Sbuffer, lsc, compl, buffer, lastdwl;

	lsc.Format(_T("%s"), CastItoXBytes(GetCompletedSize(), false, false));
	compl.Format(_T("%s"), CastItoXBytes(GetFileSize(), false, false));
	buffer.Format(_T("%s/%s"), lsc, compl);
	compl.Format(_T("%s: %s (%.1f%%)\n"), GetResString(IDS_DL_TRANSFCOMPL), buffer, GetPercentCompleted());

	if (lastseencomplete == NULL)
		lsc.Format(_T("%s"), GetResString(IDS_NEVER));
	else
		lsc.Format(_T("%s"), lastseencomplete.Format(thePrefs.GetDateTimeFormat()));

	float availability = 0.0F;
	if (partCount() != 0)
		availability = (float)(GetAvailablePartCount() * 100.0 / partCount());
	
	CString avail;
	avail.Format(GetResString(IDS_AVAIL), partCount(), GetAvailablePartCount(), availability);

	if (GetCFileDate() != NULL)
		lastdwl.Format(_T("%s"), GetCFileDate().Format(thePrefs.GetDateTimeFormat()));
	else
		lastdwl = GetResString(IDS_NEVER);
	
	CString sourcesinfo;
	sourcesinfo.Format(GetResString(IDS_DL_SOURCES) + _T(": ") + GetResString(IDS_SOURCESINFO) + _T('\n'), GetSourceCount(), GetValidSourcesCount(), GetSrcStatisticsValue(DS_NONEEDEDPARTS), GetSrcA4AFCount());
		
	// always show space on disk
	CString sod = _T("  (") + GetResString(IDS_ONDISK) + CastItoXBytes(GetRealFileSize(), false, false) + _T(")");

	CString status;
	if (GetTransferringSrcCount() > 0)
		status.Format(GetResString(IDS_PARTINFOS2) + _T("\n"), GetTransferringSrcCount());
	else 
		status.Format(_T("%s\n"), getPartfileStatus());

	CString strHeadFormatCommand = bNoFormatCommands ? _T("") : _T("<br_head>");
	CString info;
	info.Format(_T("%s\n")
		+ GetResString(IDS_FD_HASH) + _T(" %s\n")
		+ GetResString(IDS_FD_SIZE) + _T(" %s  %s\n") + strHeadFormatCommand + _T("\n")
		+ GetResString(IDS_FD_MET)+ _T(" %s\n")
		+ GetResString(IDS_STATUS) + _T(": ") + status
		+ _T("%s")
		+ sourcesinfo
		+ _T("%s")
		+ GetResString(IDS_LASTSEENCOMPL) + _T(' ') + lsc + _T('\n')
		+ GetResString(IDS_FD_LASTCHANGE) + _T(' ') + lastdwl,
		GetFileName(),
		md4str(GetFileHash()),
		CastItoXBytes(GetFileSize(), false, false),	sod,
		GetPartMetFileName(),
		compl,
		avail);
	return info;
}

bool CPartFile::GrabImage(uint8_t nFramesToGrab, double dStartTime, bool bReduceColor, uint16_t nMaxWidth, void* pSender)
{
	if (!IsPartFile()){
		return CKnownFile::GrabImage(GetPath() + CString(_T("\\")) + GetFileName(),nFramesToGrab, dStartTime, bReduceColor, nMaxWidth, pSender);
	}
	else{
		if ( ((GetStatus() != PS_READY && GetStatus() != PS_PAUSED) || m_bPreviewing || partCount() < 2 || !IsPartComplete(0))  )
			return false;
		CString strFileName = RemoveFileExtension(GetFullName());
		if (m_FileCompleteMutex.Lock(100)){
			m_bPreviewing = true; 
			try{
				if (m_hpartfile.m_hFile != INVALID_HANDLE_VALUE){
					m_hpartfile.Close();
				}
			}
			catch(CFileException* exception){
				exception->Delete();
				m_FileCompleteMutex.Unlock();
				m_bPreviewing = false; 
				return false;
			}
		}
		else
			return false;

		return CKnownFile::GrabImage(strFileName,nFramesToGrab, dStartTime, bReduceColor, nMaxWidth, pSender);
	}
}

void CPartFile::GrabbingFinished(CxImage** imgResults, uint8_t nFramesGrabbed, void* pSender)
{
	// unlock and reopen the file
	if (IsPartFile()){
		CString strFileName = RemoveFileExtension(GetFullName());
		if (!m_hpartfile.Open(strFileName, CFile::modeReadWrite|CFile::shareDenyWrite|CFile::osSequentialScan)){
			// uhuh, that's really bad
			LogError(LOG_STATUSBAR, GetResString(IDS_FAILEDREOPEN), RemoveFileExtension(GetPartMetFileName()), GetFileName());
			SetStatus(PS_ERROR);
			StopFile();
		}
		m_bPreviewing = false;
		m_FileCompleteMutex.Unlock();
		// continue processing
	}
	CKnownFile::GrabbingFinished(imgResults, nFramesGrabbed, pSender);
}

void CPartFile::GetLeftToTransferAndAdditionalNeededSpace(uint64_t &rui64LeftToTransfer, 
														  uint64_t &rui64AdditionalNeededSpace) const
{
	uint64_t uSizeLastGap = 0;
	for ( GapList::const_iterator it = gaplist.begin(); it != gaplist.end(); ++it )
	{
		const Gap_Struct* cur_gap = *it;
		uint64_t uGapSize = cur_gap->end - cur_gap->start + 1;
		rui64LeftToTransfer += uGapSize;
		if (cur_gap->end == GetFileSize() - (uint64_t)1)
			uSizeLastGap = uGapSize;
	}

	if (IsNormalFile())
	{
		// File is not NTFS-Compressed nor NTFS-Sparse
		if (GetFileSize() == GetRealFileSize()) // already fully allocated?
			rui64AdditionalNeededSpace = 0;
		else
			rui64AdditionalNeededSpace = uSizeLastGap;
	}
	else
	{
		// File is NTFS-Compressed or NTFS-Sparse
		rui64AdditionalNeededSpace = rui64LeftToTransfer;
	}
}

void CPartFile::SetLastAnsweredTimeTimeout()
{
	m_ClientSrcAnswered = 2 * CONNECTION_LATENCY + ::GetTickCount() - SOURCECLIENTREASKS;
}

/*Checks, if a given item should be shown in a given category
AllcatTypes:
	0	all
	1	all not assigned
	2	not completed
	3	completed
	4	waiting
	5	transferring
	6	errorous
	7	paused
	8	stopped
	10	Video
	11	Audio
	12	Archive
	13	CDImage
	14  Doc
	15  Pic
	16  Program
*/
bool CPartFile::CheckShowItemInGivenCat(int inCategory) /*const*/
{
	int myfilter=thePrefs.GetCatFilter(inCategory);

	// common cases
	if (((UINT)inCategory == GetCategory() && myfilter == 0))
		return true;
	if (inCategory>0 && GetCategory()!=(UINT)inCategory && !thePrefs.GetCategory(inCategory)->care4all )
		return false;


	bool ret=true;
	if ( myfilter > 0)
	{
		if (myfilter>=4 && myfilter<=8 && !IsPartFile())
			ret=false;
		else switch (myfilter)
		{
			case 1 : ret=(GetCategory() == 0);break;
			case 2 : ret= (IsPartFile());break;
			case 3 : ret= (!IsPartFile());break;
			case 4 : ret= ((GetStatus()==PS_READY || GetStatus()==PS_EMPTY) && GetTransferringSrcCount()==0);break;
			case 5 : ret= ((GetStatus()==PS_READY || GetStatus()==PS_EMPTY) && GetTransferringSrcCount()>0);break;
			case 6 : ret= (GetStatus()==PS_ERROR);break;
			case 7 : ret= (GetStatus()==PS_PAUSED || IsStopped() );break;
			case 8 : ret=  lastseencomplete!=NULL ;break;
			case 10 : ret= IsMovie();break;
			case 11 : ret= (ED2KFT_AUDIO == GetED2KFileTypeID(GetFileName()));break;
			case 12 : ret= IsArchive();break;
			case 13 : ret= (ED2KFT_CDIMAGE == GetED2KFileTypeID(GetFileName()));break;
			case 14 : ret= (ED2KFT_DOCUMENT == GetED2KFileTypeID(GetFileName()));break;
			case 15 : ret= (ED2KFT_IMAGE == GetED2KFileTypeID(GetFileName()));break;
			case 16 : ret= (ED2KFT_PROGRAM == GetED2KFileTypeID(GetFileName()));break;
			case 18 : ret= RegularExpressionMatch(thePrefs.GetCategory(inCategory)->regexp ,GetFileName());break;
			case 20 : ret= (ED2KFT_EMULECOLLECTION == GetED2KFileTypeID(GetFileName()));break;
		}
	}

	return (thePrefs.GetCatFilterNeg(inCategory))?!ret:ret;
}



void CPartFile::SetFileName(LPCTSTR pszFileName, bool bReplaceInvalidFileSystemChars, bool bRemoveControlChars)
{
	CKnownFile::SetFileName(pszFileName, bReplaceInvalidFileSystemChars, bRemoveControlChars);

	UpdateDisplayedInfo(true);
	theApp.emuledlg->transferwnd->GetDownloadList()->UpdateCurrentCategoryView(this);
}

void CPartFile::SetActive(bool bActive)
{
	time_t tNow = time(NULL);
	if (bActive)
	{
		if (theApp.IsConnected())
		{
			if (m_tActivated == 0)
				m_tActivated = tNow;
		}
	}
	else
	{
		if (m_tActivated != 0)
		{
			m_nDlActiveTime += tNow - m_tActivated;
			m_tActivated = 0;
		}
	}
}

time_t CPartFile::GetDlActiveTime() const
{
	time_t nDlActiveTime = m_nDlActiveTime;
	if (m_tActivated != 0)
		nDlActiveTime += time(NULL) - m_tActivated;
	return nDlActiveTime;
}

void CPartFile::SetFileOp(EPartFileOp eFileOp)
{
	m_eFileOp = eFileOp;
}

void CPartFile::SetFileOpProgress(UINT uProgress)
{
	ASSERT( uProgress <= 100 );
	m_uFileOpProgress = uProgress;
}

bool CPartFile::RightFileHasHigherPrio(CPartFile* left, CPartFile* right)
{
    if(!right) {
        return false;
    }

    if(!left ||
       thePrefs.GetCategory(right->GetCategory())->prio > thePrefs.GetCategory(left->GetCategory())->prio ||
       thePrefs.GetCategory(right->GetCategory())->prio == thePrefs.GetCategory(left->GetCategory())->prio &&
       (
           right->GetDownPriority() > left->GetDownPriority() ||
           right->GetDownPriority() == left->GetDownPriority() &&
           (
               right->GetCategory() == left->GetCategory() && right->GetCategory() != 0 &&
               (thePrefs.GetCategory(right->GetCategory())->downloadInAlphabeticalOrder && thePrefs.IsExtControlsEnabled()) && 
               right->GetFileName() && left->GetFileName() &&
               right->GetFileName().CompareNoCase(left->GetFileName()) < 0
           )
       )
    ) {
        return true;
    } else {
        return false;
    }
}

// ismod: no aich

UINT CPartFile::GetMaxSources() const
{
	// Ignore any specified 'max sources' value if not in 'extended mode' -> don't use a parameter which was once
	// specified in GUI but can not be seen/modified any longer..
	return (!thePrefs.IsExtControlsEnabled() || m_uMaxSources == 0) ? thePrefs.GetMaxSourcePerFileDefault() : m_uMaxSources;
}

UINT CPartFile::GetMaxSourcePerFileSoft() const
{
	UINT temp = ((UINT)GetMaxSources() * 9L) / 10;
	if (temp > MAX_SOURCES_FILE_SOFT)
		return MAX_SOURCES_FILE_SOFT;
	return temp;
}

UINT CPartFile::GetMaxSourcePerFileUDP() const
{	
	UINT temp = ((UINT)GetMaxSources() * 3L) / 4;
	if (temp > MAX_SOURCES_FILE_UDP)
		return MAX_SOURCES_FILE_UDP;
	return temp;
}

CString CPartFile::GetTempPath() const
{
	return m_fullname.Left(m_fullname.ReverseFind(_T('\\'))+1);
}

void CPartFile::RefilterFileComments()
{
	// check all available comments against our filter again
	if ( thePrefs.GetCommentFilter().IsEmpty() )
		return;

	for ( POSITION pos = srclist.GetHeadPosition(); pos != NULL; )
	{
		CUpDownClient* cur_src = srclist.GetNext(pos);
		if ( cur_src->HasFileComment() )
		{
			CString strCommentLower(cur_src->GetFileComment());
			strCommentLower.MakeLower();

			int iPos = 0;
			CString strFilter(thePrefs.GetCommentFilter().Tokenize(_T("|"), iPos));
			while ( !strFilter.IsEmpty() ) {
				// comment filters are already in lowercase, compare with temp. lowercased received comment
				if ( strCommentLower.Find(strFilter) >= 0 ) {
					cur_src->SetFileComment(_T(""));
					cur_src->SetFileRating(0);
					break;
				}
				strFilter = thePrefs.GetCommentFilter().Tokenize(_T("|"), iPos);
			}		
		}
	}
	RefilterKadNotes();
	UpdateFileRatingCommentAvail();
}

void CPartFile::PartHashFinished( uint16_t part, bool corrupt )
{
	if ( partCount() <= part )
		return;
	
	m_partsOnHash--;
    uint32_t partRange = calcPartSize(part)-1;

	if ( corrupt ) {
		LogWarning(LOG_STATUSBAR, GetResString(IDS_ERR_PARTCORRUPT), part, GetFileName());

		// add part to corrupted list, if not already there
		if ( !IsCorruptedPart(part) )
			corrupted_list.AddTail(part);

		// Only create gap if recovery data could not be requested (might start download already good data otherwise)
		AddGap(PARTSIZE*part, PARTSIZE*part + partRange);

		// update stats
		m_uCorruptionLoss += (partRange + 1);
		thePrefs.Add2LostFromCorruption(partRange + 1);

		// Update met file - gaps data changed
		SavePartFile();

		// update sources download state
		for (POSITION pos = srclist.GetHeadPosition(); pos != NULL;) {
			CUpDownClient *src = srclist.GetNext(pos);
			if ( src->GetDownloadState() == DS_NONEEDEDPARTS && src->IsPartAvailable(part) )
				src->AskForDownload();
		}
	} else {
		if (thePrefs.GetVerbose())
			AddDebugLogLine(DLP_VERYLOW, false, _T("Finished part %u of \"%s\""), part, GetFileName());

		// if this part was successfully completed, remove from corrupted list
		POSITION posCorrupted = corrupted_list.Find(part);
		if (posCorrupted)
			corrupted_list.RemoveAt(posCorrupted);

		// Successfully completed part, make it available for sharing
		m_PartsInfo[part].isChecked = true;
		if ( status == PS_EMPTY )
		{
			SetStatus(PS_READY);
			if (theApp.IsRunning())	// may be called during shutdown!
				theApp.sharedfiles->SafeAddKFile(this);
		}

		if ( !m_partsOnHash ) {
			// Update met file - file fully hashed
			SavePartFile();
		}

		if (theApp.IsRunning())	{ // may be called during shutdown!
			// Is this file finished?
			if (!m_partsOnHash && gaplist.empty() && !m_nTotalBufferData && !isFlushing())
				CompleteFile(false); // Recheck all hashes, because loaded data is trusted based on file-date
		}
	}
}

void CPartFile::HashSinglePart( uint16_t part )
{
    if ( !theApp.IsRunning() || pendingOnDelete() )	// Don't start any last-minute hashing
        return;

    if ( partCount() <= part ) {	// Out of bounds, no point in even trying
        return;
    }

	if ( hashset().count() < GetED2KPartCount() ) {
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_HASHERRORWARNING), GetFileName());
		return;
	}
	
	HashInfo_Struct* pHashInfo = new HashInfo_Struct;
    pHashInfo->owner = this;
    pHashInfo->partsToHash.push_back(part);

	m_partsOnHash++;

    theApp.m_pHashThread->PostThreadMessage(UM_HASHPARTS, (WPARAM)0, (LPARAM)pHashInfo);
}

void CPartFile::HashCompleteParts()
{
    if ( !theApp.IsRunning() || pendingOnDelete() )
        return;

    HashInfo_Struct* pHashInfo = new HashInfo_Struct;
    
    pHashInfo->owner = this;

    uint16_t partsSentToHash = 0;
	
	for ( uint16_t i = 0; i < partCount(); i++ ) {
        if ( IsPartComplete(i) ) {
            pHashInfo->partsToHash.push_back(i);
			partsSentToHash++;
        }
	}

	m_partsOnHash += partsSentToHash;

    theApp.m_pHashThread->PostThreadMessage(UM_HASHPARTS, (WPARAM)0, (LPARAM)pHashInfo);
}

CString CPartFile::getProgressbarStatus() const
{
	switch( GetStatus() ) {
		case PS_HASHING:
			return GetResString(IDS_HASHING);
		case PS_WAITINGFORHASH:
			return GetResString(IDS_HASHING) + _T("(Queued)");

		case PS_COMPLETING:{
			CString strState = GetResString(IDS_COMPLETING);
			if (GetFileOp() == PFOP_HASHING)
				strState += _T(" (") + GetResString(IDS_HASHING) + _T(")");
			else if (GetFileOp() == PFOP_COPYING)
				if(m_hPreviewProcess != NULL)
					strState = _T("«¿ –Œ…“≈ œ–≈ƒœ–Œ—ÃŒ“–");
				else
					strState += _T(" (Copying)");
			else if (GetFileOp() == PFOP_UNCOMPRESSING)
				strState += _T(" (Uncompressing)");
			return strState;
						   }

		case PS_COMPLETE:
			return GetResString(IDS_COMPLETE);

		case PS_PAUSED:
			if (stopped)
				return GetResString(IDS_STOPPED);
			return GetResString(IDS_PAUSED);

		case PS_INSUFFICIENT:
			return GetResString(IDS_INSUFFICIENT);

		case PS_ERROR:
			if (m_bCompletionError)
				return GetResString(IDS_INSUFFICIENT);
			return GetResString(IDS_ERRORLIKE);
	}

	if(IsAllocating())
		return GetResString(IDS_MOD_ALLOCATING);
	else {
		CString str;
		str.Format(_T("%.1f%%"), GetPercentCompleted());
		return str;
	}
}

void CPartFile::setSparse( bool enable )
{
	if ( m_hpartfile.m_hFile == NULL ) 
		return;

	DWORD dwAttr = GetFileAttributes(m_hpartfile.GetFilePath());
	FILE_SET_SPARSE_BUFFER fssb;
	
	if ( enable && !(dwAttr&FILE_ATTRIBUTE_SPARSE_FILE) ) {
		fssb.SetSparse = TRUE;
	} else if ( !enable && dwAttr&FILE_ATTRIBUTE_SPARSE_FILE ) {
		fssb.SetSparse = FALSE;
	}

	DWORD dwReturnedBytes = 0;
	if ( !DeviceIoControl(m_hpartfile.m_hFile, FSCTL_SET_SPARSE,  &fssb, sizeof(fssb), NULL, 0, &dwReturnedBytes, NULL) )
	{
		// Errors:
		// ERROR_INVALID_FUNCTION	returned by WinXP when attempting to create a sparse file on a FAT32 partition
		DWORD dwError = GetLastError();
		if ( dwError != ERROR_INVALID_FUNCTION && thePrefs.GetVerboseLogPriority() <= DLP_VERYLOW )
			DebugLogError(_T("Failed to apply NTFS sparse file attribute to file \"%s\" - %s"), m_hpartfile.GetFilePath(), GetErrorMessage(dwError, 1));
		// ismod: set full allocation flag
		m_bAllocateFullSpace = true;
	}
}

//////////////////////////////////////////////////////////////////////////
//
//  PartFile Hash Thread
//

IMPLEMENT_DYNCREATE(CPartHashThread, CSafeThread)

BEGIN_MESSAGE_MAP(CPartHashThread, CSafeThread)
    ON_THREAD_MESSAGE ( UM_HASHPARTS, OnHashParts ) 
END_MESSAGE_MAP()

BOOL CPartHashThread::InitInstance()
{
	CSafeThread::InitInstance();
    DbgSetThreadName("CPartHashThread");
    SetThreadPriority(THREAD_PRIORITY_NORMAL);
    InitThreadLocale();
    return TRUE;
}

void CPartHashThread::OnHashParts(WPARAM /*wParam*/, LPARAM lParam)
{
	CReadWriteLock lock(&theApp.m_threadlock);
	if (!lock.ReadLock(0))
		AfxEndThread(0, FALSE);

	ResetEvent(theApp.m_hPartHashWaitEvent);

	HashInfo_Struct* pHashInfo = (HashInfo_Struct*)lParam;
	CPartFile* owner = pHashInfo->owner;

	CFile file;
	CString partFilePath = RemoveFileExtension(owner->m_fullname);
	CFileException *fexp = new CFileException;
	if ( file.Open(partFilePath, CFile::modeRead|CFile::osSequentialScan|CFile::shareDenyNone, fexp) )
	{
		for ( auto it = pHashInfo->partsToHash.cbegin(); it != pHashInfo->partsToHash.cend(); ++it )
		{
			uint16_t part = *it;
			uchar hashresult[16];
			uint32_t length = owner->calcPartSize(part);

			if ( !theApp.IsRunning() )	// in case of shutdown while still hashing
				break;

			try {
				file.Seek((LONGLONG)PARTSIZE*part, CFile::begin);
				file.Read(m_localData, length);
			} catch ( CFileException* e ) {
				PartFile_Io_Error *pfioerr = new PartFile_Io_Error;
				pfioerr->where = PFIOERR_ONHASH;
				pfioerr->exception = e;
				theApp.emuledlg->PostMessage(TM_PARTFILE_IOERR, (WPARAM)owner, (LPARAM)pfioerr);
				continue;
			}

			CKnownFile::CreateHash(m_localData, length, hashresult);

			if ( md4cmp(hashresult, owner->hashset(part)) ) {
				theApp.emuledlg->PostMessage(TM_PARTHASHEDCORRUPT, (WPARAM)owner, (LPARAM)part);
			} else {
				theApp.emuledlg->PostMessage(TM_PARTHASHEDOK, (WPARAM)owner, (LPARAM)part);
			}
		}

		file.Close();
		fexp->Delete();
	} else {
		PartFile_Io_Error *pfioerr = new PartFile_Io_Error;
		pfioerr->where = PFIOERR_ONHASH;
		pfioerr->exception = fexp;
		theApp.emuledlg->PostMessage(TM_PARTFILE_IOERR, (WPARAM)owner, (LPARAM)pfioerr);
	}

	delete pHashInfo;

	SetEvent(theApp.m_hPartHashWaitEvent);
}

//////////////////////////////////////////////////////////////////////////
//
// PartFile Flush Thread
//

IMPLEMENT_DYNCREATE(CPartFileFlushThread, CSafeThread)

BEGIN_MESSAGE_MAP(CPartFileFlushThread, CSafeThread)
	ON_THREAD_MESSAGE ( UM_FLUSHFILE, OnFlushFile ) 
END_MESSAGE_MAP()

BOOL CPartFileFlushThread::InitInstance()
{
	CSafeThread::InitInstance();
	DbgSetThreadName("CPartFileFlushThread");
	InitThreadLocale();
	SetThreadPriority(THREAD_PRIORITY_NORMAL);
	return TRUE;
}

bool ComparePartFileBufferedData(const PartFileBufferedData* left, const PartFileBufferedData* right )
{
	return left->start < right->start;
}

void CPartFileFlushThread::OnFlushFile( WPARAM wParam, LPARAM /*lParam*/ )
{
	CPartFile* owner = (CPartFile*)wParam;

	// sort data list
	std::sort(owner->m_FlushData_list.begin(), owner->m_FlushData_list.end(), ComparePartFileBufferedData);

	// Loop through queue
	for ( PartFileBufferedDataList::iterator it = owner->m_FlushData_list.begin();
		it != owner->m_FlushData_list.end(); ++it )
	{
		PartFileBufferedData* item = *it;

		if ( NULL == item->data )
			continue;

		uint32_t length = (uint32_t)(item->end - item->start + 1);

		try {
			owner->m_hpartfile.Seek(item->start, CFile::begin);
			owner->m_hpartfile.Write(item->data, length);
		} catch ( CFileException* e ) {
			PartFile_Io_Error *pfioerr = new PartFile_Io_Error;
			pfioerr->where = PFIOERR_ONFLUSH;
			pfioerr->exception = e;
			theApp.emuledlg->PostMessage(TM_PARTFILE_IOERR, (WPARAM)owner, (LPARAM)pfioerr);

			SetEvent(owner->m_waitFlushEvent);
			return;
		}

		delete[] item->data;
		item->data = NULL; // Important!!!
	}

	// Flush to disk
	owner->m_hpartfile.Flush();

	SetEvent(owner->m_waitFlushEvent);
	theApp.emuledlg->PostMessage(TM_FLUSHDONE, (WPARAM)owner, (LPARAM)0);
}