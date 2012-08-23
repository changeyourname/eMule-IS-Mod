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

class CFriend;
class CFriendListCtrl;
class CUpDownClient;

class CFriendList
{
public:
	CFriendList();
	~CFriendList();

	bool		IsAlreadyFriend(CString strUserHash) const;
	bool		IsValid(CFriend* pToCheck) const;
	void		SaveList();
	bool		LoadList();
	void		RefreshFriend(CFriend* torefresh) const;
	CFriend*	SearchFriend(const uchar* achUserHash, uint32_t dwIP, uint16_t nPort) const;
	void		SetWindow(CFriendListCtrl* NewWnd)	{ m_wndOutput = NewWnd; }
	void		ShowFriends() const;
	bool		AddFriend(CUpDownClient* toadd);
	bool		AddFriend(const uchar* abyUserhash, uint32_t dwLastSeen, uint32_t dwLastUsedIP, uint16_t nLastUsedPort, 
                          uint32_t dwLastChatted, LPCTSTR pszName, uint32_t dwHasHash);
	void		RemoveFriend(CFriend* todel);
	void		RemoveAllFriendSlots();
	void		Process();
	int			GetCount()		{ return (int) m_listFriends.GetCount(); }

private:
	CTypedPtrList<CPtrList, CFriend*>	m_listFriends;
	CFriendListCtrl*					m_wndOutput;
	uint32_t								m_nLastSaved;
};