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

#include "stdafx.h"
#include <crashrpt/CrashRpt.h>
#include "SafeThread.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CSafeThread, CWinThread)

CSafeThread::CSafeThread()
	: CWinThread()
{
}

CSafeThread::CSafeThread(AFX_THREADPROC pfnThreadProc, LPVOID pParam)
	: CWinThread(pfnThreadProc, pParam)
{
}

CSafeThread::~CSafeThread()
{
}

BOOL CSafeThread::InitInstance()
{
#ifndef _DEBUG
	crInstallToCurrentThread2(0);
#endif
	
	return CWinThread::InitInstance();
}

int CSafeThread::ExitInstance()
{
#ifndef _DEBUG
	crUninstallFromCurrentThread();
#endif

	return CWinThread::ExitInstance();
}

CSafeThread* CSafeThread::BeginThread(AFX_THREADPROC pfnThreadProc, LPVOID pParam,
		int nPriority, UINT nStackSize, DWORD dwCreateFlags,
		LPSECURITY_ATTRIBUTES lpSecurityAttrs)
{
	ASSERT(pfnThreadProc != NULL);

	CSafeThread* pThread = new CSafeThread(pfnThreadProc, pParam);
	ASSERT_VALID(pThread);

	if (!pThread->CreateThread(dwCreateFlags|CREATE_SUSPENDED, nStackSize,
		lpSecurityAttrs))
	{
		pThread->Delete();
		return NULL;
	}
	VERIFY(pThread->SetThreadPriority(nPriority));
	if (!(dwCreateFlags & CREATE_SUSPENDED))
		VERIFY(pThread->ResumeThread() != (DWORD)-1);

	return pThread;
}

CSafeThread* CSafeThread::BeginThread(CRuntimeClass* pThreadClass,
		int nPriority, UINT nStackSize, DWORD dwCreateFlags,
		LPSECURITY_ATTRIBUTES lpSecurityAttrs)
{
	ASSERT(pThreadClass != NULL);
	ASSERT(pThreadClass->IsDerivedFrom(RUNTIME_CLASS(CSafeThread)));

	CSafeThread* pThread = (CSafeThread*)pThreadClass->CreateObject();
	if (pThread == NULL)
		AfxThrowMemoryException();
	ASSERT_VALID(pThread);

	pThread->m_pThreadParams = NULL;
	if (!pThread->CreateThread(dwCreateFlags|CREATE_SUSPENDED, nStackSize,
		lpSecurityAttrs))
	{
		pThread->Delete();
		return NULL;
	}
	VERIFY(pThread->SetThreadPriority(nPriority));
	if (!(dwCreateFlags & CREATE_SUSPENDED))
	{
		ENSURE(pThread->ResumeThread() != (DWORD)-1);
	}

	return pThread;
}