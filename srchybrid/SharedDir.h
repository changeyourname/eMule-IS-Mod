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

#pragma once

class CKnowFile;

typedef std::set<CKnownFile*> KnownFileSet; 

class SharedDir
{
public:
	explicit SharedDir(LPCTSTR dirPath);

	void	addFile(CKnownFile* file);
	void	removeFile(CKnownFile* file);
	void	createCollection();
	void	deleteCollection();
	CString	collectionFileName() const { return m_strCollFileName;  }
	uint64_t	totalSize() const { return m_uTotalSize; }
	size_t	filesCount() const { return m_files.size(); }
	KnownFileSet::const_iterator getConstIteratorBegin() const { return m_files.begin(); }
	KnownFileSet::const_iterator getConstIteratorEnd() const { return m_files.end(); }

private:
	bool m_bCollectionCreated;
	CString m_strDirPath;
	CString m_strCollFilePath;
	CString m_strCollFileName;
	KnownFileSet m_files;
	uint64_t m_uTotalSize;
};

typedef std::map<CString,SharedDir*> SharedDirsMap;