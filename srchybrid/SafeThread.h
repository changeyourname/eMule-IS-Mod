//this file is part of eMule IS Mod
//Copyright (C)2011 eMule IS Mod Team
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

class CSafeThread : public CWinThread
{
	DECLARE_DYNAMIC(CSafeThread)
public:
	CSafeThread();
	CSafeThread(AFX_THREADPROC pfnThreadProc, LPVOID pParam);
	virtual ~CSafeThread();

	virtual BOOL InitInstance();
	virtual int ExitInstance();
	
	static CSafeThread* BeginThread(AFX_THREADPROC pfnThreadProc, LPVOID pParam,
		int nPriority = THREAD_PRIORITY_NORMAL, UINT nStackSize = 0,
		DWORD dwCreateFlags = 0, LPSECURITY_ATTRIBUTES lpSecurityAttrs = NULL);

	static CSafeThread* BeginThread(CRuntimeClass* pThreadClass,
		int nPriority = THREAD_PRIORITY_NORMAL, UINT nStackSize = 0,
		DWORD dwCreateFlags = 0, LPSECURITY_ATTRIBUTES lpSecurityAttrs = NULL);
};