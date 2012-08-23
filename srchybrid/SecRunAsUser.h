//this file is part of eMule
//Copyright (C)2004-2005 Merkur ( devs@emule-project.net / http://www.emule-project.net )
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

class CSecRunAsUser
{
public:
    enum Result {
        RES_OK_NEED_RESTART = 0,
        RES_OK,
        RES_FAILED
    };

	CSecRunAsUser();
	~CSecRunAsUser();

	Result	RestartSecure();
	bool	IsRunningEmuleAccount()		{return m_bRunningAsEmule;}
	bool	IsRunningRestricted()		{return m_bRunningRestricted;}
	bool	IsRunningSecure()			{return m_bRunningRestricted || m_bRunningAsEmule;}
	CStringW	GetCurrentUserW();

protected:
	Result	PrepareUser();
	Result	RestartAsUser();
	Result	RestartAsRestricted();

	bool	SetDirectoryPermissions();
	CStringW	CreateRandomPW();
	bool	SetObjectPermission( CString strDirFile, DWORD lGrantedAccess );

private:
	CStringW ADSPath;
	CStringW m_strPassword;
	CStringW m_strDomain;
	CStringW m_strCurrentUser;
	bool m_bRunningAsEmule;
	bool m_bRunningRestricted;
};
