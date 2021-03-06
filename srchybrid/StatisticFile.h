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
class CKnownFile;

class CStatisticFile
{
public:
	CStatisticFile()
	{
		requested = 0;
		transferred = 0;
		accepted = 0;
		alltimerequested = 0;
		alltimetransferred = 0;
		alltimeaccepted = 0;
	}

	void	MergeFileStats( CStatisticFile* toMerge );
	void	AddRequest();
	void	AddAccepted();
	void	AddTransferred(uint64_t bytes);

	UINT	GetRequests() const				{return requested;}
	UINT	GetAccepts() const				{return accepted;}
	uint64_t	GetTransferred() const			{return transferred;}
	UINT	GetAllTimeRequests() const		{return alltimerequested;}
	UINT	GetAllTimeAccepts() const		{return alltimeaccepted;}
	uint64_t	GetAllTimeTransferred() const	{return alltimetransferred;}
	void	SetAllTimeRequests(uint32_t nVal);
	void	SetAllTimeAccepts(uint32_t nVal);
	void	SetAllTimeTransferred(uint64_t nVal);
	
	CKnownFile* fileParent;

private:
	uint64_t transferred;
	uint64_t alltimetransferred;
    uint32_t requested;
	uint32_t accepted;
	uint32_t alltimerequested;
	uint32_t alltimeaccepted;
};
