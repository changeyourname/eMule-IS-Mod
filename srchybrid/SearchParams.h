#pragma once

#include "SafeFile.h"

///////////////////////////////////////////////////////////////////////////////
// ESearchType

enum ESearchType
{
	//NOTE: The numbers are *equal* to the entries in the comboxbox -> TODO: use item data
	SearchTypeAutomatic = 0,
	SearchTypeEd2kServer,
	SearchTypeEd2kGlobal,
	SearchTypeKademlia,
	SearchTypeFileDonkey
};


#define	MAX_SEARCH_EXPRESSION_LEN	512

///////////////////////////////////////////////////////////////////////////////
// SSearchParams

struct SSearchParams
{
	SSearchParams()
	{
		dwSearchID = ~uint32_t(0);
		eType = SearchTypeEd2kGlobal;
		bClientSharedFiles = false;
		ullMinSize = 0;
		ullMaxSize = 0;
		uAvailability = 0;
		uComplete = 0;
		ulMinBitrate = 0;
		ulMinLength = 0;
		bMatchKeywords = false;
        // ismod [start]
        bShowOwnFiles = false;
        bSearchUsers = false;
        bSearchFolders = false;
        bRestoringSearch = false;
        bFilterPPPlus = false;
        // ismod [end]
	}

	SSearchParams(CFileDataIO& rFile)
	{
		dwSearchID = rFile.ReadUInt32();
		eType = (ESearchType)rFile.ReadUInt8();
		bClientSharedFiles = rFile.ReadUInt8() > 0;
		strSpecialTitle = rFile.ReadString(true);
		strExpression = rFile.ReadString(true);
		strFileType = rFile.ReadString(true);
		ullMinSize = 0;
		ullMaxSize = 0;
		uAvailability = 0;
		uComplete = 0;
		ulMinBitrate = 0;
		ulMinLength = 0;
		bMatchKeywords = false;
        // ismod [start]
        bShowOwnFiles = rFile.ReadUInt8() > 0;
        bSearchUsers = rFile.ReadUInt8() > 0;
        bSearchFolders = rFile.ReadUInt8() > 0;
        bRestoringSearch = false;
        bFilterPPPlus = false;
        // ismod [end]
	}
	uint32_t dwSearchID;
	bool bClientSharedFiles;
	CString strSearchTitle;
	CString strExpression;
	CString strKeyword;
	CString strBooleanExpr;
	ESearchType eType;
	CStringA strFileType;
	CString strMinSize;
	uint64_t ullMinSize;
	CString strMaxSize;
	uint64_t ullMaxSize;
	UINT uAvailability;
	CString strExtension;
	UINT uComplete;
	CString strCodec;
	ULONG ulMinBitrate;
	ULONG ulMinLength;
	CString strTitle;
	CString strAlbum;
	CString strArtist;
	CString strSpecialTitle;
	bool bMatchKeywords;
    // ismod [start]
    bool bShowOwnFiles;
    bool bSearchUsers;
    bool bSearchFolders;
    bool bRestoringSearch;
    CString strSysInfoPart;
    bool bFilterPPPlus;
    // ismod [end]

	void StorePartially(CFileDataIO& rFile) const
	{
		rFile.WriteUInt32(dwSearchID);
		rFile.WriteUInt8((uint8_t)eType);
		rFile.WriteUInt8(bClientSharedFiles ? 1 : 0);
		rFile.WriteString(strSpecialTitle, utf8strRaw);
		rFile.WriteString(strExpression, utf8strRaw);
		rFile.WriteString(CString(strFileType), utf8strRaw);
        // ismod [start]
        rFile.WriteUInt8(bShowOwnFiles ? 1 : 0);
        rFile.WriteUInt8(bSearchUsers ? 1 : 0);
        rFile.WriteUInt8(bSearchFolders ? 1 : 0);
        // ismod [end]
	}
};

bool GetSearchPacket(CSafeMemFile* data, SSearchParams* pParams, bool bTargetSupports64Bit, bool* pbPacketUsing64Bit);
