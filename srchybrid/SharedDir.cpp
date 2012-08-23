//this file is part of eMule IS Mod
//Copyright (C)2009 IS Mod Team
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

#include "stdafx.h"
#include "KnownFile.h"
#include "SharedDir.h"
#include "Preferences.h"
#include "Log.h"
#include "OtherFunctions.h"
#include "emule.h"
#include "SharedFileList.h"

SharedDir::SharedDir( LPCTSTR dirPath )
{
	m_uTotalSize = 0ui64;
	m_strDirPath = theApp.sharedfiles->GetPseudoDirName(dirPath);
	m_bCollectionCreated = false;
}

void SharedDir::addFile( CKnownFile* file )
{
	if ( m_files.find(file) == m_files.end() )
	{
		m_files.insert(file);
		m_uTotalSize += (uint64_t)file->GetFileSize();
		m_bCollectionCreated = false;
	}
}

void SharedDir::removeFile( CKnownFile* file )
{
	if ( m_files.find(file) != m_files.end() )
	{
		m_files.erase(file);
		m_uTotalSize -= (uint64_t)file->GetFileSize();
		m_bCollectionCreated = false;
	}
}

void SharedDir::createCollection()
{
	if ( m_bCollectionCreated )
		return;

	m_strCollFileName.Format(_T("%s\\%d.emulecollection"), m_strDirPath, m_files.size());
	m_strCollFileName.Replace(_T('\\'),_T('-'));
	m_strCollFilePath = thePrefs.GetMuleDirectory(EMULE_COLLECTIONDIR, true) + _T("\\") + m_strCollFileName;

	CFile collFile;
	if ( !collFile.Open(m_strCollFilePath, CFile::modeWrite|CFile::modeCreate) ) {
		AddDebugLogLine(false, _T("Unable to create collection for %s"), m_strDirPath);
		return;
	}

	char szCreationTime[] = "#CreationTime 1111111111\r\n";
	collFile.Write(szCreationTime, _countof(szCreationTime)-1);

	KnownFileSet::iterator iter = m_files.begin();
	while ( iter != m_files.end() )
	{
		CString strED2KLink = CreateED2kLink(*iter, true);
		collFile.Write(CT2CA(strED2KLink), strED2KLink.GetLength());
		collFile.Write("\r\n", 2);

		++iter;
	}

	collFile.Close();

	CTime fileTime(2000,1,1,1,1,1); // constant time for all files!!!
	CFileStatus status;
	CFile::GetStatus(m_strCollFilePath, status);
	status.m_ctime = status.m_mtime = fileTime;
	CFile::SetStatus(m_strCollFilePath, status);


	m_bCollectionCreated = true;
}

void SharedDir::deleteCollection()
{
	if ( m_bCollectionCreated )
		DeleteFile(m_strCollFilePath);
}
