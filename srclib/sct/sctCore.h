//this file is part of eMule IS Mod
//Copyright (C)2009 eMule IS Mod
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

#ifndef _SCTCORE_H_
#define _SCTCORE_H_

#pragma once;

#include <vector>

class CSctCore
{
    enum ETaskType
    {
        SCT_TASK_PING=1,	// проверка коннективности с помощью ICMP
        SCT_TASK_TELE,		// проверка связи до телематический площадки
		SCT_TASK_VIDEO
    };

    struct ICMP_TEST_RESULT
    {
        uint32_t uMin;
        uint32_t uMax;
        uint32_t uMid;
        uint8_t  uTtl;
    };

public:
    CSctCore();
    virtual ~CSctCore();

    void startThreads();
    
    inline const CString& GetOwnIp() const { return m_strBestIpAddr; }
    inline uint8_t GetEthernetStatus() const { return m_uEthernetStatus; }
    inline uint8_t GetIs74SiteStatus() const { return m_uConnectionStatusToIs74ru; }

	// Выполнить задание
	void execTask( const CString& hash, const CString& taskData );

	void InvokePis();

private:
    double CalculateBandwidth( LPCTSTR szUrl ) const;
	uint8_t CheckTcpConnection( LPCTSTR szHost, uint16_t uPort, uint32_t uCount ) const;
    uint8_t CheckUdpConnection( LPCTSTR szHost, uint16_t uPort, uint32_t uCount ) const;	
    uint8_t CheckIcmpConnection( LPCTSTR szHost, uint16_t uDataSize, uint32_t uRepeatInterval, uint32_t uRepeadCount, ICMP_TEST_RESULT& itr ) const;
    CString execIcmpCheckOthers( const CString& params ) const;
	CString execTestVideoServer( const CString& params ) const;
    bool runSelfUpdate( LPCTSTR szUpdateLink );
    void LoadSettings();
    void SaveSettings() const;

    // Статус проверки подключения до tcp://is74.ru:80, в процентах
    uint8_t m_uConnectionStatusToIs74ru; // кол-во пакетов дошедших до сервера, в процентах

    // Статус проверки на наличие сетевого интерфейса, в процентах
    uint8_t m_uEthernetStatus;

    bool m_sctAlreadyUpdated;
	time_t m_sctLastUpdateCheckTime;

    CString m_strBestIpAddr;      // best ip-адрес до is74.ru
	uint32_t  m_uDelay;             // Интервал (мс) через который запрашиваются новые задания
    HANDLE m_hWaitEvent;            // эвент для ожидания

    CRITICAL_SECTION m_updateServersCS;
    std::vector<ULONG> m_serverIps;

	CWinThread *m_pisThread;
	static UINT AFX_CDECL pisThreadWrapper( LPVOID lpParam );
	UINT pisWorker();

	CWinThread *m_clientThread;
	static UINT AFX_CDECL clientThreadWrapper( LPVOID lpParam );
	UINT clientWorker();

    CWinThread *m_serverThread;
	static UINT AFX_CDECL serverThreadWrapper( LPVOID lpParam );
	UINT serverWorker();

    // флаг первого запроса,подтверждения к ПИС
    bool m_pisFirstRequest;
    bool m_pisFirstConfirm;
};

#endif // #ifndef _SCTCORE_H_
