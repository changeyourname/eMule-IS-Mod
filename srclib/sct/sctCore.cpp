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

#include "stdafx.h"
#include <sstream>
#include <atlutil.h>
#include <IPHlpApi.h>
#include <Icmpapi.h>

#include <openssl/err.h>
#include <openssl/conf.h>
#include <emhelper/emhelper.h>

#include "resource.h"
#include "sct.h"
#include "options.h"
#include "SecureSocket.h"
#include "tools.h"
#include "MD5Sum.h"
#include "sctCore.h"
#include "IpfUpdater.h"
#include "PisDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

RSA* LoadPublicKey();

// Конструктор
CSctCore::CSctCore() :
	m_sctAlreadyUpdated(false),
	m_uDelay(SCT_FIRST_DELAY),
	m_clientThread(NULL),
	m_serverThread(NULL),
	m_pisThread(NULL),
	m_sctLastUpdateCheckTime(0),
    m_pisFirstRequest(true),
    m_pisFirstConfirm(true)
{
    LoadSettings();
    SecureSocket::loadSsl();
    m_hWaitEvent = CreateEvent(0,0,0,0);
    InitializeCriticalSection(&m_updateServersCS);

	// обновим ipfilter
	AfxBeginThread(CIpfUpdater::UpdateIpfThread, NULL);

}

CSctCore::~CSctCore()
{
    if(m_clientThread)
    {
        TerminateThread(m_clientThread->m_hThread, 0);
        m_clientThread->Delete();
    }
    if(m_serverThread)
    {
        TerminateThread(m_serverThread->m_hThread, 0);
        m_serverThread->Delete();
    }

    CloseHandle(m_hWaitEvent);
    SaveSettings();
    SecureSocket::unloadSsl();
}

void CSctCore::startThreads()
{
    m_clientThread = AfxBeginThread(clientThreadWrapper, this);
    m_serverThread = AfxBeginThread(serverThreadWrapper, this);

	InvokePis();
}

void CSctCore::InvokePis() {
	if ( NULL == m_pisThread ) {
		m_pisThread = AfxBeginThread(pisThreadWrapper, static_cast<LPVOID>(this));
	}
}

UINT AFX_CDECL CSctCore::pisThreadWrapper(LPVOID lpParam)
{
	CSctCore* self = static_cast<CSctCore*>(lpParam);
	UINT uReturnCode = self->pisWorker();
	self->m_pisThread = NULL;

	return uReturnCode;
}

UINT CSctCore::pisWorker()
{
	std::string url = SCT_PIS_URL;

	if ( m_pisFirstRequest ) {
		m_pisFirstRequest = false;
		url += "&firstrun=1";
	}

	std::ostringstream data;
	int code = emCurl::httpRequest(url, "", data);
	if ( 0 != code) {
		return 1;
	}
	
	std::wstring message = (wchar_t*)data.str().c_str();

	if ( message.size() < 3 ) {
		return 1;
	}

	CPisDialog pisDlg;
	pisDlg.setInfoText(message.c_str());
	
	pisDlg.DoModal();

	url = SCT_PIS_URL_ACCEPT;

	if ( m_pisFirstConfirm ) {
		m_pisFirstConfirm = false;
		url += "&firstrun=1";
	}

    emCurl::httpRequest(url, "", data);
    m_pisThread = NULL;

	return 0;
}

UINT AFX_CDECL CSctCore::clientThreadWrapper(LPVOID lpParam)
{
	CSctCore* self = static_cast<CSctCore*>(lpParam);
	UINT uReturnCode = self->clientWorker();
	self->m_clientThread = NULL;

	return uReturnCode;
}

// поток клиента, который ходит на сервер скт и забирает оттуда задания
UINT CSctCore::clientWorker()
{
    m_uEthernetStatus = CheckTcpConnection(SCT_AUTH_SRV, 80, 10);

    // проверка подключения к is74.ru
    m_uConnectionStatusToIs74ru = CheckTcpConnection(SCT_IS74_ADDR, 80, 10);

    for(;;)
    {
        MD5Sum sum;
        sum.Calculate((uchar*)(LPSTR)CT2CA(*g_pstrPassword), g_pstrPassword->GetLength());

		CString sctUrl;
		sctUrl.Format(L"https://%s%s?page=get_task&login=%s&hash=%s&version=%s&emver=%s&ownip=%s", SCT_SERVER_ADDR, SCT_WEB_PATH, 
			*g_pstrLogin, sum.GetHash(), g_szVersion, g_szEmuleVersion,	GetOwnIp());
		
		CString strPostData;
		strPostData.Format(L"page=get_task&login=%s&hash=%s&version=%s&emver=%s&ownip=%s", 
			*g_pstrLogin, sum.GetHash(), g_szVersion, g_szEmuleVersion,	GetOwnIp());

		std::ostringstream data;
        std::string url = CW2A(sctUrl);
        std::string postData = CW2A(strPostData);
		int code = emCurl::httpRequest(url, postData, data);

		if ( 0 == code ) {
			CString utfRawData = CA2W(data.str().c_str());
			execTask(sum.GetHash(), utfRawData);	
		}
		
        WaitForSingleObject(m_hWaitEvent, m_uDelay); // sleep
    }
    
    return 0;
}

UINT AFX_CDECL CSctCore::serverThreadWrapper( LPVOID lpParam )
{
	CSctCore* self = static_cast<CSctCore*>(lpParam);
	UINT uReturnCode = self->serverWorker();
	self->m_serverThread = NULL;

	return uReturnCode;
}

UINT CSctCore::serverWorker()
{
	{
		in_addr ip = SecureSocket::resolve(SCT_IS74_ADDR_A);
		DWORD dwBestIf;
		GetBestInterface(static_cast<IPAddr>(ip.S_un.S_addr), &dwBestIf);

		MIB_IPADDRTABLE *pIpAddrTable = (MIB_IPADDRTABLE *)malloc(sizeof(MIB_IPADDRTABLE));
		ULONG uSize = sizeof(MIB_IPADDRTABLE);
		if( ERROR_INSUFFICIENT_BUFFER == GetIpAddrTable(pIpAddrTable, &uSize, FALSE) ) {
			free(pIpAddrTable);
			pIpAddrTable = (MIB_IPADDRTABLE *)malloc(uSize);
		}
		GetIpAddrTable(pIpAddrTable, &uSize, FALSE);
		for ( DWORD i=0; i < (int) pIpAddrTable->dwNumEntries; i++) {
			if ( pIpAddrTable->table[i].dwIndex == dwBestIf ) {
				ip.S_un.S_addr = static_cast<ULONG>(pIpAddrTable->table[i].dwAddr);
				m_strBestIpAddr = inet_ntoa(ip);
				break;
			}
		}
		free(pIpAddrTable);
	}

    // load rsa public key
    RSA* rsaPubKey = LoadPublicKey();
    if ( !rsaPubKey )
        return 0;

    for(;;)
    {
        if ( !m_strBestIpAddr.IsEmpty() )
        {
            // create socket
            SOCKET srvSock = socket(AF_INET, SOCK_STREAM, 0);
            if ( srvSock != INVALID_SOCKET )
            {
                struct sockaddr_in bind_addr;        
                memset(&bind_addr, 0, sizeof(bind_addr));
                bind_addr.sin_family = AF_INET;
                bind_addr.sin_port = htons(SCT_LISTEN_PORT);
                bind_addr.sin_addr.s_addr = inet_addr(CT2CA(m_strBestIpAddr));

                // bind to addr and port
                if ( bind_addr.sin_addr.s_addr && bind(srvSock, (sockaddr*)&bind_addr, sizeof(bind_addr)) == 0 )
                {
                    // listen socket
                    if ( listen(srvSock, 5) == 0 ) 
                    {
                        for(;;)
                        {
                            SOCKET cliSock;
                            struct sockaddr_in out_addr;
                            int sock_len = sizeof(out_addr);

                            // accept connection
                            if( (cliSock = accept(srvSock, (sockaddr*)&out_addr, &sock_len)) == INVALID_SOCKET )
                                break;

                            // validate peer address
                            bool isValidPeer = false;
                            EnterCriticalSection(&m_updateServersCS);
                            for( size_t i = 0; i<m_serverIps.size(); ++i)
                            {
                                if ( out_addr.sin_addr.s_addr == m_serverIps[i] )
                                {
                                    isValidPeer = true;
                                    break;
                                }
                            }
                            LeaveCriticalSection(&m_updateServersCS);
                            if ( !isValidPeer )
                            {
                                closesocket(cliSock);
                                continue;
                            }

                            // read all available data
                            CMemFile memFile(1024);
                            char pTempBuf[128];
                            int len;
                            while ( (len = recv(cliSock, pTempBuf, _countof(pTempBuf), 0)) > 0 )
                            {
                                memFile.Write(pTempBuf, len);
                            }
                            closesocket(cliSock);
                            int in_len = (int)memFile.GetLength();

                            if ( in_len <= 0 )
                                continue;

                            uchar* in_data = memFile.Detach();
                            uchar* out_data = new uchar[in_len+1];

                            // try to decrypt
                            int out_len = RSA_public_decrypt(in_len, in_data, out_data, rsaPubKey, RSA_PKCS1_PADDING);
                            free(in_data);
                            if( out_len <= 0)
                            {
                                delete[] out_data;
                                continue;
                            }
                            out_data[out_len] = '\0';

                            // calculate password md5
                            MD5Sum sum;
                            sum.Calculate((uchar*)(LPSTR)CT2CA(*g_pstrPassword), g_pstrPassword->GetLength());

                            // prepare task text
                            CString task(CA2CT((LPSTR)out_data));
                            delete[] out_data;

                            // execute task
                            execTask(sum.GetHash(), task);
                        }
                    }
                }
                closesocket(srvSock);
            }
        }
        WaitForSingleObject(m_hWaitEvent, 5000); // 5 sec sleep
    }

    // free public key
    RSA_free(rsaPubKey);
 
    return 0;
}

// Получить задание
void CSctCore::execTask( const CString& hash,  const CString& taskData )
{
    // ничего не получили
    if ( taskData.IsEmpty() )
        return;
    
    CStringVector arrLines = SplitString(taskData, _T("\r\n"));

    // мнимум 2 строки
    if( arrLines.size() < 2)
        return;
    
    // первая строка - информация всякая
    if ( !arrLines[0].IsEmpty() && (arrLines[0] != _T("NO")) )
    {
        CStringVector arrPairsList = SplitString(arrLines[0], _T(";"));
        for( UINT i = 0; i< arrPairsList.size(); ++i )
        {
            CStringVector arrParamPair = SplitString(arrPairsList[i], _T(":"));

            if ( arrParamPair.size() != 2 )
                continue;
        
            // задержка
            if (arrParamPair[0] == _T("delay"))
                m_uDelay = _tstoi(arrParamPair[1]);
            else if (arrParamPair[0] == _T("servers"))
            {
                EnterCriticalSection(&m_updateServersCS);
                CStringVector ips = SplitString(arrParamPair[1], _T(","));
                m_serverIps.clear();
                for(UINT j = 0; j<ips.size(); ++j )
                    m_serverIps.push_back(inet_addr(CT2A(ips[j])));
                LeaveCriticalSection(&m_updateServersCS);
            }

        }
    }

    // вторая строка - ссылка на обновление если требуется
    // обновляемся если только автообновление включено и еще не обновлялись
	// каждые 24 часа предлагаем
    if ( (arrLines[1].Find(_T("http://")) == 0) && (time(NULL) - m_sctLastUpdateCheckTime > 60*60*24) )
    {
		m_sctAlreadyUpdated = runSelfUpdate(arrLines[1]);
		m_sctLastUpdateCheckTime = time(NULL);
    }

	// 3rd line link to new emule, if newer version is avaliable (obsolete)

    // по всем заданиям, начиная со второй строки
    for( UINT i = 3; i < arrLines.size(); ++i )
    {
        CStringVector arrTask = SplitString(arrLines[i], _T(" "));
        //0 - type, 1 - id, 2 - аргументы
        // (пример - "from:78.29.0.254;to:78.29.0.254;size:1;count:1;interval_repeat:1")
    
        // минимум 3 поля
        if(arrTask.size() < 3)
            continue;
        
        CString strResult;
		switch( _tstoi(arrTask[0]) )
        {
        case SCT_TASK_PING:
            strResult = execIcmpCheckOthers(arrTask[2]);
            break;
		case SCT_TASK_VIDEO:
			strResult = execTestVideoServer(arrTask[2]);
			break;
		default:
			continue;
        }

		// отправим результат выполнения задания
		CString sctUrl;
		sctUrl.Format(L"https://%s:%u/%s?page=set_result&login=%s&hash=%s&tid=%s&version=%s&emver=%s",
			SCT_SERVER_ADDR, SCT_SERVER_PORT, SCT_WEB_PATH, *g_pstrLogin, hash, arrTask[1], g_szVersion, g_szEmuleVersion);

		std::ostringstream data;
        std::string url = CW2A(sctUrl);
        std::string postData = CW2A(strResult);
		emCurl::httpRequest(url, postData, data);
    }
}

CString CSctCore::execTestVideoServer(const CString& params ) const 
{
	CString result;
	double fSpeed = CalculateBandwidth(params);
	if ( fSpeed > 0 )
	{
		if (fSpeed < 1024.0)
			result.Format(_T("%.0f Bytes/sec"), fSpeed);
		else if (fSpeed < 1024000.0)
			result.Format(_T("%.2f KBytes/sec"), fSpeed/1024.0);
		else if (fSpeed < 1048576000.0)
			result.Format(_T("%.2f MBytes/sec"), fSpeed/1048576.0);
		else if (fSpeed < 1073741824000.0)
			result.Format(_T("%.2f GBytes/sec"), fSpeed/1073741824.0);
		else 
			result.Format(_T("%.2f TBytes/sec"), fSpeed/1099511627776.0);
	}else
		result = _T("CONNECTION_ERROR");

	return result;
}

/*
    Проверка коннективности с другими абонентами
    Параметры передаваемые в строке:
    iplist - список ip адресов через запятую
    size - размер пакета
    interval_repeat - интервал повторения отсылки пакета (мс)
    count - кол-во пакетов для отправки
*/
CString CSctCore::execIcmpCheckOthers(const CString& params) const
{
    // ассоциативный массив параметров для пинга
    CStringStringMap mParams;

    // заполняем мультикарту(ассоц масив) параметров
    CStringVector arrTaskParams = SplitString(params, _T(";"));
    for ( UINT j = 0; j< arrTaskParams.size(); ++j )
    {
        CStringVector arrPair = SplitString(arrTaskParams[j], _T(":"));
        if ( arrPair.size() != 2 )
            continue;

        mParams[arrPair[0]] = arrPair[1];
    }

    uint32_t uRepeatCount = (uint32_t)_tstoi(mParams[_T("count")]); // кол-во пакетов?
    uint16_t uDataSize = (uint16_t)_tstoi(mParams[_T("size")]); // размер отправляемых данных
    uint32_t uRepeatInterval = (uint32_t)_tstoi(mParams[_T("interval_repeat")]); // интервал повторения пинга

    // валидация
    if (uRepeatCount < 1) uRepeatCount = 1;
    else if (uRepeatCount > 5000) uRepeatCount = 5000;
	if (uDataSize < 1) uDataSize = 1;
	else if (uDataSize > 65000) uDataSize = 65000;
    if (uRepeatInterval > 600000) uRepeatInterval = 600000; // 10 мин

    // циклом по всем адресам
    int curPos = 0;
    CString resToken = mParams[_T("iplist")].Tokenize(_T(","), curPos);
    CString strResult = _T("result=");
    while ( resToken != _T("") )
    {
        ICMP_TEST_RESULT itr;
        uint8_t uPercent = CheckIcmpConnection(resToken, uDataSize, uRepeatInterval, uRepeatCount, itr);

        CString strResultLine;
        strResultLine.Format(
            _T("ip%s+sta%d+min%d+mid%d+max%d+TTL%d;\n"),
            resToken, uPercent, itr.uMin, itr.uMid, itr.uMax, itr.uTtl);
        strResult += strResultLine;

        resToken = mParams[_T("iplist")].Tokenize(_T(","), curPos);
    }

    return strResult;
}

/*
@brief Проверка TCP соединения до хоста и заданного порта
@return кол-во удачных соединений процентах от общего числа попыток
*/
uint8_t CSctCore::CheckTcpConnection(LPCTSTR szHost, uint16_t uPort, uint32_t uCount) const
{
    uint32_t uOk = 0;

    for(uint32_t i = 0; i < uCount; i++)
    {
        CSocket tcpSocket;
        tcpSocket.Create(0, SOCK_STREAM);
        if ( tcpSocket.Connect(szHost, uPort)  )
        {
            tcpSocket.Close();
            uOk++;
        }
    }

    return uint8_t(float(uOk)/float(uCount) * 100);
}

/*
@brief Проверка UDP соединения до хоста и заданного порта
@return кол-во удачно отправленных датаграммв процентах
*/
uint8_t CSctCore::CheckUdpConnection( LPCTSTR szHost, uint16_t uPort, uint32_t uCount ) const
{
    uint32_t uOk = 0;

    for(uint32_t i = 0; i < uCount; i++)
    {
        CSocket udpSocket;
        udpSocket.Create(0, SOCK_DGRAM);
        if ( udpSocket.Connect(szHost, uPort) )
        {
            if ( udpSocket.Send(SCT_TEST_DATA, _countof(SCT_TEST_DATA)) )
                uOk++;;
            udpSocket.Close();
        }
    }

    return uint8_t(float(uOk)/float(uCount) * 100);
}

/*
@brief Подсчитать скорость скачивания файла с сервера по HTTP
*/
double CSctCore::CalculateBandwidth(LPCTSTR szUrl) const
{
	double fResultSpeed;
	CUrl url;
	url.CrackUrl(szUrl);

	CSocket httpSocket;
	if ( httpSocket.Create(0, SOCK_STREAM) && httpSocket.Connect(url.GetHostName(), url.GetPortNumber())  )
	{
		CString strRequest;
		strRequest.Format(
			_T("GET %s HTTP/1.0\r\n")
			_T("Host: %s:%u\r\n")
			_T("Connection: Close\r\n\r\n"),
			url.GetUrlPath(),
			url.GetHostName(),
			url.GetPortNumber());
		httpSocket.Send(CT2CA(strRequest), strRequest.GetLength());
		DWORD dwStartTime = GetTickCount();

		double uTotalSize = 0;
		uint32_t uRetSize;
		BYTE pTempBuf[512];
		while( ( uRetSize = httpSocket.Receive(pTempBuf, _countof(pTempBuf)) ) > 0)
		{
			uTotalSize += uRetSize;
		}
		DWORD dwEndTime = GetTickCount();
		httpSocket.Close();

		double fTotalTime = (dwEndTime-dwStartTime)/1000.0;
		if ( fTotalTime < 0.1 ) fTotalTime = 0.1;
		fResultSpeed = uTotalSize / fTotalTime;
	}else
		fResultSpeed = -1.;

	return fResultSpeed;
}


// ICMP Проверка соединения до хоста
uint8_t CSctCore::CheckIcmpConnection(LPCTSTR szHost, uint16_t uDataSize, uint32_t uRepeatInterval,
                                    uint32_t uRepeadCount, ICMP_TEST_RESULT& itr) const
{
    ZeroMemory(&itr, sizeof(ICMP_TEST_RESULT));
    itr.uMin = 1000;

    HANDLE hIcmp = IcmpCreateFile();
    if(hIcmp == INVALID_HANDLE_VALUE)
        return 0;

    // резолвим хост
    IPAddr ipaHost = Resolve(CT2CA(szHost));
    if ( ipaHost == 0 )
        return 0;

    // данные, которые будет отправлять
    LPBYTE lpSendData = new BYTE[uDataSize];
    FillMemory(lpSendData, uDataSize, 0x2A);

    // готовим всякие данные
    DWORD dwReplySize = sizeof(ICMP_ECHO_REPLY) + uDataSize;
    LPVOID lpReplyBuffer = new BYTE[dwReplySize];

    uint32_t uOk = 0; // счетчик удачных попыток
    for ( uint32_t i = 0; i < uRepeadCount; ++i )
    {
        DWORD dwReplyCount = IcmpSendEcho(hIcmp, ipaHost, lpSendData, uDataSize, NULL, 
            lpReplyBuffer, dwReplySize, SCT_ICMP_TIMEOUT);

        if ( dwReplyCount < 1 )
            continue;

        PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY)lpReplyBuffer;
        if ( pEchoReply->Status != IP_SUCCESS )
            continue;

        ++uOk;

        // среднее
        itr.uMid += pEchoReply->RoundTripTime;

        // максимальное время
        if ( itr.uMax < pEchoReply->RoundTripTime ) 
            itr.uMax = pEchoReply->RoundTripTime;

        // минимальное время
        if ( itr.uMin > pEchoReply->RoundTripTime ) 
            itr.uMin = pEchoReply->RoundTripTime;

        // TTL пакета
        itr.uTtl = pEchoReply->Options.Ttl;

        WaitForSingleObject(m_hWaitEvent, uRepeatInterval);
    }

    if(uOk!=0)
        itr.uMid /= uOk;
    else
        itr.uMin = 0;

    IcmpCloseHandle(hIcmp);

    delete[] lpSendData;
    delete[] lpReplyBuffer;

    return uint8_t(float(uOk)/float(uRepeadCount)*100);
}

EXTERN_C IMAGE_DOS_HEADER __ImageBase; // читерство
bool CSctCore::runSelfUpdate(LPCTSTR szUpdateLink)
{
    // get self file path
    CString strCurDllPath;
    GetModuleFileName((HINSTANCE)&__ImageBase, strCurDllPath.GetBufferSetLength(1000), 999);

    // dll stored in user folder: don't use updater tool
	if ( strCurDllPath.Find(_T("\\AppData")) > 0 || strCurDllPath.Find(_T("\\Local Settings")) > 0 )
	{
		std::ostringstream data;
        std::string url = CW2A(szUpdateLink);
		int code = emCurl::httpRequest(url, "", data);
		if ( 0 != code ) {
			return false;
		}
        std::string dataArray = data.str();
        if ( dataArray.size() < 1024 ) {
            return false;
        }

		CString strNewDllPath;
		strNewDllPath.Format(_T("%s.new"), strCurDllPath);
		CString strOldDllPath;
		strOldDllPath.Format(_T("%s.old"), strCurDllPath);

		// save new dll
		CFile newDllFile;

		if ( newDllFile.Open(strNewDllPath, CFile::modeWrite|CFile::modeCreate|CFile::shareExclusive) == FALSE )
		{
			return false;
		}

		newDllFile.Write(dataArray.c_str(), dataArray.size());
		newDllFile.Close();

		// текущую в старую
		if ( MoveFileEx(strCurDllPath, strOldDllPath, MOVEFILE_REPLACE_EXISTING) == FALSE )
		{
			return false;
		}

		// новую в текущую
		if ( MoveFileEx(strNewDllPath, strCurDllPath, MOVEFILE_REPLACE_EXISTING) == FALSE )
		{
			// fallback!
			MoveFileEx(strOldDllPath, strCurDllPath, MOVEFILE_REPLACE_EXISTING);
			return false;
		}

		return true;

	} else { 
		
		// dll in app folder, use update tool

		if ( AfxMessageBox(_T("Обновить модуль \"Ваша сеть\"?"), MB_YESNO|MB_SYSTEMMODAL|MB_ICONQUESTION, 0) == IDNO )
		{
			// пользователь отказался, больше не надоедаем
			return true;
		}

		// get full path of update tool
		CString strUpdaterExePath = strCurDllPath.Left(strCurDllPath.ReverseFind(_T('\\'))+1) + _T("sctupd.exe");

		// format parameters
		CString strCmdLine;
		strCmdLine.Format(_T("\"%s\" \"%s\""), szUpdateLink, strCurDllPath);

		SHELLEXECUTEINFO   sei;
		ZeroMemory(&sei, sizeof(sei));

		sei.cbSize          = sizeof(SHELLEXECUTEINFO);
		sei.fMask           = SEE_MASK_FLAG_DDEWAIT | SEE_MASK_FLAG_NO_UI | SEE_MASK_NOCLOSEPROCESS;
		if ( LOBYTE(GetVersion()) >= 0x06 )
	        sei.lpVerb = _T("runas");
	    else 
			sei.lpVerb = _T("open");
		sei.lpFile          = strUpdaterExePath;
		sei.lpParameters    = strCmdLine;
		sei.nShow           = SW_HIDE;

		if ( !ShellExecuteEx(&sei) ) {
			AfxMessageBox(_T("Не удалось выполнить обновление модуля СКТ."), MB_ICONSTOP);
	        return false;
		}

		WaitForSingleObject(sei.hProcess, INFINITE);
		DWORD dwExitCode;
		GetExitCodeProcess(sei.hProcess, &dwExitCode);
		if ( dwExitCode != 0 ) {
			AfxMessageBox(_T("Не удалось выполнить обновление модуля СКТ."), MB_ICONSTOP);
	        return false;
		}

		CloseHandle( sei.hProcess );

		AfxMessageBox(
			_T("Обновление выполнено успешно.\n")
			_T("Изменения вступят в силу при следующем запуске программы."), MB_ICONINFORMATION);
		
		return true;
	}
}

// сохраняем настройки
void CSctCore::SaveSettings() const
{
    CRegKey key;
    LONG res = key.Open(HKEY_CURRENT_USER, SCT_OPTIONS_REG_LOCATION);
    if ( ERROR_SUCCESS!= res )
    {
        key.Close();
        res = key.Create(HKEY_CURRENT_USER, SCT_OPTIONS_REG_LOCATION);
        if ( ERROR_SUCCESS == res )
            return;
    }
    key.SetQWORDValue(SCT_OPTIONS_SCT_LAST_UPDATE_CHECK_TIME, m_sctLastUpdateCheckTime);
}

// загружаем настройки
void CSctCore::LoadSettings()
{
    CRegKey key;
    LONG res = key.Open(HKEY_CURRENT_USER, SCT_OPTIONS_REG_LOCATION);
    if ( ERROR_SUCCESS == res )
    {
		ULONGLONG tmpVal64;

		res = key.QueryQWORDValue(SCT_OPTIONS_SCT_LAST_UPDATE_CHECK_TIME, tmpVal64);
		if( ERROR_SUCCESS == res ) {
			m_sctLastUpdateCheckTime = tmpVal64;
		} else {
			m_sctLastUpdateCheckTime = 0;
		}
    }
}

RSA* LoadPublicKey()
{
    // load public key from resources and check
    HMODULE hSctDll = GetModuleHandle(_T("sct"));
    HRSRC hResourcePtr = FindResource(hSctDll, MAKEINTRESOURCE(IDR_PUBLIC_KEY), _T("RT_PEM"));
    HGLOBAL hResource = (BYTE*)LoadResource(hSctDll, hResourcePtr);
    char* rsaPubKey = (char*)LockResource(hResource);
    if ( !rsaPubKey )
        return 0;

    // create bio
    BIO* keyBio = BIO_new_mem_buf(rsaPubKey, strlen(rsaPubKey)); 
    FreeResource(hResource);
    if ( !keyBio )
        return 0;

    // read pubkey
    EVP_PKEY* public_key = PEM_read_bio_PUBKEY(keyBio, 0, 0, 0);
    BIO_free(keyBio);
    if ( !public_key )
        return 0;
    
    // get rsa key
    RSA* rsa = EVP_PKEY_get1_RSA(public_key);
    EVP_PKEY_free(public_key);
    if ( !rsa )
        return 0;

    return rsa;
};