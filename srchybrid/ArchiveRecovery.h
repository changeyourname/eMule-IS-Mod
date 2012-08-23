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
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#pragma once

class CPartFile;
class CShareableFile;
struct Gap_Struct;

#ifndef GapList
typedef std::list<Gap_Struct*> GapList;
#endif

#define ZIP_LOCAL_HEADER_MAGIC		0x04034b50
#define ZIP_LOCAL_HEADER_EXT_MAGIC	0x08074b50
#define ZIP_CD_MAGIC				0x02014b50
#define ZIP_END_CD_MAGIC			0x06054b50
#define ZIP_COMMENT					"Recovered by eMule"

#define RAR_HEAD_FILE 0x74

#define LODWORD(l)           ((DWORD)(((LONG64)(l)) & 0xffffffff))

#pragma pack(1)
struct ZIP_Entry
{
	uint32_t	header;
	uint16_t	versionToExtract;
	uint16_t	generalPurposeFlag;
	uint16_t	compressionMethod;
	uint16_t	lastModFileTime;
	uint16_t	lastModFileDate;
	uint32_t	crc32;
	uint32_t	lenCompressed;
	uint32_t	lenUncompressed;
	uint16_t	lenFilename;
	uint16_t	lenExtraField;
	BYTE	*filename;
	BYTE	*extraField;
	BYTE	*compressedData;
};
#pragma pack()

#pragma pack(1)
struct ZIP_CentralDirectory
{
	ZIP_CentralDirectory() {
		lenFilename = 0;
		filename = NULL;
		lenExtraField = 0;
		extraField = NULL;
		lenComment = 0;
		comment = NULL;
	}
	uint32_t	header;
	uint16_t	versionMadeBy;
	uint16_t	versionToExtract;
	uint16_t	generalPurposeFlag;
	uint16_t	compressionMethod;
	uint16_t	lastModFileTime;
	uint16_t	lastModFileDate;
	uint32_t	crc32;
	uint32_t	lenCompressed;
	uint32_t	lenUncompressed;
	uint16_t	lenFilename;
	uint16_t	lenExtraField;
	uint16_t	lenComment;
	uint16_t	diskNumberStart;
	uint16_t	internalFileAttributes;
	uint32_t	externalFileAttributes;
	uint32_t	relativeOffsetOfLocalHeader;
	BYTE	*filename;
	BYTE	*extraField;
	BYTE	*comment;
};
#pragma pack()

#pragma pack(1)
struct RAR_BlockFile 
{
	RAR_BlockFile()
	{
		EXT_DATE = NULL;
		EXT_DATE_SIZE = 0;
	}
	~RAR_BlockFile()
	{
		delete[] EXT_DATE;
	}

	// This indicates the position in the input file just after the filename
	ULONGLONG offsetData; 
	// This indicates how much of the block is after this offset
	uint32_t dataLength;
    
	uint16_t	HEAD_CRC;
	BYTE	HEAD_TYPE;
	uint16_t	HEAD_FLAGS;
	uint16_t	HEAD_SIZE;
	uint32_t	PACK_SIZE;
	uint32_t	UNP_SIZE;
	BYTE	HOST_OS;
	uint32_t	FILE_CRC;
	uint32_t	FTIME;
	BYTE	UNP_VER;
	BYTE	METHOD;
	uint16_t	NAME_SIZE;
	uint32_t	ATTR;
	uint32_t	HIGH_PACK_SIZE;
	uint32_t	HIGH_UNP_SIZE;
	BYTE	*FILE_NAME;
	BYTE	*EXT_DATE;
	uint32_t	EXT_DATE_SIZE;
	BYTE	SALT[8];
};
#pragma pack()
#pragma pack(1)
struct ACE_ARCHIVEHEADER 
{
	uint16_t	HEAD_CRC;
	uint16_t	HEAD_SIZE;
	BYTE	HEAD_TYPE;
	uint16_t	HEAD_FLAGS;
	BYTE	HEAD_SIGN[7];
	BYTE	VER_EXTRACT;
	BYTE	VER_CREATED;
	BYTE	HOST_CREATED;
	BYTE	VOLUME_NUM;
	uint32_t	FTIME;
	BYTE	RESERVED[8];
	BYTE	AVSIZE;
	//**AV 
	uint16_t	COMMENT_SIZE;

	char*   AV;
	char*	COMMENT;
	char*	DUMP;

	ACE_ARCHIVEHEADER() {
		AV=NULL;
		COMMENT=NULL;
		DUMP=NULL;
		COMMENT_SIZE=0;
	}
	~ACE_ARCHIVEHEADER() {
		if (AV)		{ free(AV);		AV=NULL;}
		if (COMMENT){ free(COMMENT);COMMENT=NULL;}
		if (DUMP)	{ free(DUMP);	DUMP=NULL;}
	}
};
#pragma pack()
#pragma pack(1)
struct ACE_BlockFile 
{
	uint16_t	HEAD_CRC;
	uint16_t	HEAD_SIZE;
	BYTE	HEAD_TYPE;
	uint16_t	HEAD_FLAGS;
	uint32_t  PACK_SIZE;
	uint32_t  ORIG_SIZE;
	uint32_t  FTIME;
	uint32_t  FILE_ATTRIBS;
	uint32_t  CRC32;
	uint32_t  TECHINFO;
	uint16_t  RESERVED;
	uint16_t  FNAME_SIZE;
	// fname
	uint16_t  COMM_SIZE;
	// comment

	char*	FNAME;
	char*	COMMENT;
	uint64_t  data_offset;
	ACE_BlockFile() {
		FNAME=NULL;
		COMMENT=NULL;
		COMM_SIZE=0;
	}
	~ACE_BlockFile() {
		free(FNAME);
		free(COMMENT);
	}
};
#pragma pack()

// ################################
// ISO related
static unsigned char sig_udf_bea[5]  = { 0x42, 0x45, 0x41, 0x30, 0x31 };		// "BEA01"
static unsigned char sig_udf_nsr2[5] = { 0x4e, 0x53, 0x52, 0x30, 0x32 };		// "NSR02"
static unsigned char sig_udf_nsr3[5] = { 0x4e, 0x53, 0x52, 0x30, 0x33 };		// "NSR03"
static unsigned char sig_tea[5]		 = { 0x54, 0x45, 0x41, 0x30, 0x31 };		// "TEA01"
static const    char sElToritoID[]		 = "EL TORITO SPECIFICATION";

enum ISO_ImageType
{
	ISOtype_unknown		= 0,
	ISOtype_9660		= 1,
	ISOtype_joliet		= 2,
	ISOtype_UDF_nsr02	= 4,
	ISOtype_UDF_nsr03	= 8,
};
enum ISO_FileFlags
{
	ISO_HIDDEN		= 1,
	ISO_DIRECTORY	= 2,
	ISO_FILE		= 4,
	ISO_RECORD		= 8,
	ISO_READONLY	= 16
};
#pragma pack(1)
struct ISO_DateTimePVD_s
{ 
	unsigned char year[4];
	unsigned char month[2];
	unsigned char day[2];
	unsigned char hour[2];
	unsigned char minute[2];
	unsigned char second[2];
	unsigned char sechundr[2];
	unsigned char offsetGreenwich;
};
#pragma pack()

#pragma pack(1)
struct ISO_DateTimeFileFolder_s
{ 
	unsigned char year;
	unsigned char month;
	unsigned char day;
	unsigned char hour;
	unsigned char minute;
	unsigned char second;
	unsigned char offsetGreenwich;
};
#pragma pack()

#pragma pack(1)
struct ISO_PVD_s { 
	unsigned char descr_type;
	unsigned char magic[5];
	unsigned char descr_ver;
	unsigned char unused;
	unsigned char sysid[32];
	unsigned char volid[32];
	unsigned char zeros1[8];
	unsigned char seknum[8];
	unsigned char escSeq[32];
	
	UINT32 volsetsize;
	UINT32 volseqnum;
	UINT32 seksize;
	UINT64 pathtablen;

	UINT32 firstSek_LEpathTab1_LE;
	UINT32 firstsek_LEpathtab2_LE;
	UINT32 firstsek_BEpathtab1_BE;
	UINT32 firstsek_BEpathtab2_BE;

	unsigned char rootdir[34];
	unsigned char volsetid[128];
	unsigned char pubid[128];
	unsigned char dataprepid[128];
	unsigned char appid[128];
	unsigned char copyr[37];
	unsigned char abstractfileid[37];
	unsigned char bibliofileid[37];

	ISO_DateTimePVD_s creationdate;
	ISO_DateTimePVD_s modifydate;
	ISO_DateTimePVD_s expiredate;

	unsigned char effective[17];
	unsigned char filestruc_ver;
	unsigned char zero;
	unsigned char app_use[512];
	unsigned char res[653];
};
#pragma pack()

#pragma pack(1)
struct BootDescr
{ 
	unsigned char descr_type;
	unsigned char magic[5];
	unsigned char descr_ver;
	unsigned char sysid[32];
	unsigned char bootid[32];
	unsigned char system_use[1977]; 
};
#pragma pack()

#pragma pack(1)
struct ISO_BootDescr_s
{ 
	unsigned char descr_type;
	unsigned char magic[5];
	unsigned char descr_ver;
	unsigned char sysid[32];
	unsigned char bootid[32];
	unsigned char system_use[1977]; 
};
#pragma pack()

#pragma pack(1)
struct ISO_PathtableEntry
{ 
	BYTE	len;
	BYTE	lenExt;
	unsigned int	sectorOfExtension;
	UINT16	sectorOfParent;
	char*	name;
};
#pragma pack()
#pragma pack(1)
struct ISO_FileFolderEntry
{ 
	BYTE	lenRecord;
	BYTE	nrOfSecInExt;
	UINT64	sector1OfExtension;
	UINT64	dataSize;
	ISO_DateTimeFileFolder_s dateTime;
	BYTE	fileFlags;
	BYTE	fileUnitSize;
	BYTE	interleaveGapSize;
	unsigned int	volSeqNr;
	BYTE	nameLen;
	TCHAR*	name;
	ISO_FileFolderEntry() { name=NULL;};
	~ISO_FileFolderEntry() { if (name) free(name);};
};
#pragma pack()
struct ISOInfos_s
{
	bool	bBootable;
	DWORD	secSize;
	bool	bUDF;
	int		iJolietUnicode;
	DWORD	type;
};

struct ThreadParam
{
	CPartFile *partFile;
	GapList *filled;
	bool preview;
	bool bCreatePartFileCopy;
};


struct archiveinfo_s {
	CTypedPtrList<CPtrList, ZIP_CentralDirectory*> *centralDirectoryEntries;
	CTypedPtrList<CPtrList, RAR_BlockFile*> *RARdir;
	CTypedPtrList<CPtrList, ACE_BlockFile*> *ACEdir;
	CTypedPtrList<CPtrList, ISO_FileFolderEntry*> *ISOdir;
	
	bool bZipCentralDir;
	WORD rarFlags;
	ISOInfos_s isoInfos;
	ACE_ARCHIVEHEADER *ACEhdr;
	archiveinfo_s() { 
		centralDirectoryEntries=NULL;
		RARdir=NULL;
		ACEdir=NULL;
		rarFlags=0;
		bZipCentralDir=false;
		ACEhdr=NULL;
		isoInfos.bBootable=false;
		isoInfos.secSize=false;
		isoInfos.iJolietUnicode=0;
	}
};
struct archiveScannerThreadParams_s {
	CShareableFile*	file;
	archiveinfo_s*	ai;
	GapList *filled;
	int				type;
	HWND			ownerHwnd;
	HWND			progressHwnd;
	int				curProgress;
	bool			m_bIsValid;
};

class CArchiveRecovery
{
public:
	static void recover(CPartFile *partFile, bool preview = false, bool bCreatePartFileCopy = true);
	static bool recoverZip(CFile *zipInput, CFile *zipOutput, archiveScannerThreadParams_s* ai, GapList *filled, bool fullSize);
	static bool recoverRar(CFile *rarInput, CFile *rarOutput, archiveScannerThreadParams_s* ai, GapList *filled);
	static bool recoverAce(CFile *aceInput, CFile *aceOutput, archiveScannerThreadParams_s* ai, GapList *filled);
	static bool recoverISO(CFile *aceInput, CFile *aceOutput, archiveScannerThreadParams_s* ai, GapList *filled);

private:
	CArchiveRecovery(void); // Just use static recover method

	static UINT AFX_CDECL run(LPVOID lpParam);
	static bool performRecovery(CPartFile *partFile, GapList *filled, bool preview, bool bCreatePartFileCopy = true);

	static bool scanForZipMarker(CFile *input, archiveScannerThreadParams_s* aitp, uint32_t marker, uint32_t available);
	static bool processZipEntry(CFile *zipInput, CFile *zipOutput, uint32_t available, CTypedPtrList<CPtrList, ZIP_CentralDirectory*> *centralDirectoryEntries);
	static bool readZipCentralDirectory(CFile *zipInput, CTypedPtrList<CPtrList, ZIP_CentralDirectory*> *centralDirectoryEntries, GapList *filled);

	static RAR_BlockFile *scanForRarFileHeader(CFile *input, archiveScannerThreadParams_s* aitp, UINT64 available);
	static bool validateRarFileBlock(RAR_BlockFile *block);
	static void writeRarBlock(CFile *input, CFile *output, RAR_BlockFile *block);

	static ACE_BlockFile *scanForAceFileHeader(CFile *input, archiveScannerThreadParams_s* aitp, UINT64 available);
	static void writeAceBlock(CFile *input, CFile *output, ACE_BlockFile *block);
	static void CArchiveRecovery::writeAceHeader(CFile *output, ACE_ARCHIVEHEADER* hdr);

	static bool CopyFile(CPartFile *partFile, GapList *filled, CString tempFileName);
	static void DeleteMemory(ThreadParam *tp);
	static bool IsFilled(uint32_t start, uint32_t end, GapList *filled);

	static void ISOReadDirectory(archiveScannerThreadParams_s* aitp, UINT32 startSec, CFile* isoInput, CString currentDirName);

	static void ProcessProgress(archiveScannerThreadParams_s* aitp, UINT64 pos);

	static uint16_t readUInt16(CFile *input);
	static uint32_t readUInt32(CFile *input);
	static uint16_t calcUInt16(BYTE *input);
	static uint32_t calcUInt32(BYTE *input);
	static void writeUInt16(CFile *output, uint16_t val);
	static void writeUInt32(CFile *output, uint32_t val);

};	 // class
