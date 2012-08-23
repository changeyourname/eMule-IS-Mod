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

// CStatistics
#define AVG_SESSION 0
#define AVG_TOTAL 2
#define AVG_TIME 1

enum TBPSTATES {
	STATE_DOWNLOADING = 0x01,
	STATE_ERROROUS    = 0x10
};

class CStatistics
{
public:
	CStatistics();   // standard constructor

	void	Init();
	void	RecordRate();
	float	GetAvgDownloadRate(int averageType);
	float	GetAvgUploadRate(int averageType);

	// -khaos--+++> (2-11-03)
	uint32_t	GetTransferTime()			{ return timeTransfers + time_thisTransfer; }
	uint32_t	GetUploadTime()				{ return timeUploads + time_thisUpload; }
	uint32_t	GetDownloadTime()			{ return timeDownloads + time_thisDownload; }
	uint32_t	GetServerDuration()			{ return timeServerDuration + time_thisServerDuration; }
	void	Add2TotalServerDuration()	{ timeServerDuration += time_thisServerDuration;
										  time_thisServerDuration = 0; }
	void	UpdateConnectionStats(float uploadrate, float downloadrate);


	///////////////////////////////////////////////////////////////////////////
	// Down Overhead
	//
	void	CompDownDatarateOverhead();
	void	ResetDownDatarateOverhead();
	void	AddDownDataOverheadSourceExchange(uint32_t data)	{ m_nDownDataRateMSOverhead += data;
															  m_nDownDataOverheadSourceExchange += data;
															  m_nDownDataOverheadSourceExchangePackets++;}
	void	AddDownDataOverheadFileRequest(uint32_t data)		{ m_nDownDataRateMSOverhead += data;
															  m_nDownDataOverheadFileRequest += data;
															  m_nDownDataOverheadFileRequestPackets++;}
	void	AddDownDataOverheadServer(uint32_t data)			{ m_nDownDataRateMSOverhead += data;
															  m_nDownDataOverheadServer += data;
															  m_nDownDataOverheadServerPackets++;}
	void	AddDownDataOverheadOther(uint32_t data)			{ m_nDownDataRateMSOverhead += data;
															  m_nDownDataOverheadOther += data;
															  m_nDownDataOverheadOtherPackets++;}
	void	AddDownDataOverheadKad(uint32_t data)				{ m_nDownDataRateMSOverhead += data;
															  m_nDownDataOverheadKad += data;
															  m_nDownDataOverheadKadPackets++;}
	void	AddDownDataOverheadCrypt(uint32_t /*data*/)			{;}
	uint32_t	GetDownDatarateOverhead()					{return m_nDownDatarateOverhead;}
	uint64_t	GetDownDataOverheadSourceExchange()			{return m_nDownDataOverheadSourceExchange;}
	uint64_t	GetDownDataOverheadFileRequest()			{return m_nDownDataOverheadFileRequest;}
	uint64_t	GetDownDataOverheadServer()					{return m_nDownDataOverheadServer;}
	uint64_t	GetDownDataOverheadKad()					{return m_nDownDataOverheadKad;}
	uint64_t	GetDownDataOverheadOther()					{return m_nDownDataOverheadOther;}
	uint64_t	GetDownDataOverheadSourceExchangePackets()	{return m_nDownDataOverheadSourceExchangePackets;}
	uint64_t	GetDownDataOverheadFileRequestPackets()		{return m_nDownDataOverheadFileRequestPackets;}
	uint64_t	GetDownDataOverheadServerPackets()			{return m_nDownDataOverheadServerPackets;}
	uint64_t	GetDownDataOverheadKadPackets()				{return m_nDownDataOverheadKadPackets;}
	uint64_t	GetDownDataOverheadOtherPackets()			{return m_nDownDataOverheadOtherPackets;}


	///////////////////////////////////////////////////////////////////////////
	// Up Overhead
	//
	void	CompUpDatarateOverhead();
	void	ResetUpDatarateOverhead();
	void	AddUpDataOverheadSourceExchange(uint32_t data)	{ m_nUpDataRateMSOverhead += data;
															  m_nUpDataOverheadSourceExchange += data;
															  m_nUpDataOverheadSourceExchangePackets++;}
	void	AddUpDataOverheadFileRequest(uint32_t data)		{ m_nUpDataRateMSOverhead += data;
															  m_nUpDataOverheadFileRequest += data;
															  m_nUpDataOverheadFileRequestPackets++;}
	void	AddUpDataOverheadServer(uint32_t data)			{ m_nUpDataRateMSOverhead += data;
															  m_nUpDataOverheadServer += data;
															  m_nUpDataOverheadServerPackets++;}
	void	AddUpDataOverheadKad(uint32_t data)				{ m_nUpDataRateMSOverhead += data;
															  m_nUpDataOverheadKad += data;
															  m_nUpDataOverheadKadPackets++;}
	void	AddUpDataOverheadOther(uint32_t data)				{ m_nUpDataRateMSOverhead += data;
															  m_nUpDataOverheadOther += data;
															  m_nUpDataOverheadOtherPackets++;}
	void	AddUpDataOverheadCrypt(uint32_t /*data*/)				{ ;}

	uint32_t	GetUpDatarateOverhead()						{return m_nUpDatarateOverhead;}
	uint64_t	GetUpDataOverheadSourceExchange()			{return m_nUpDataOverheadSourceExchange;}
	uint64_t	GetUpDataOverheadFileRequest()				{return m_nUpDataOverheadFileRequest;}
	uint64_t	GetUpDataOverheadServer()					{return m_nUpDataOverheadServer;}
	uint64_t	GetUpDataOverheadKad()						{return m_nUpDataOverheadKad;}
	uint64_t	GetUpDataOverheadOther()					{return m_nUpDataOverheadOther;}
	uint64_t	GetUpDataOverheadSourceExchangePackets()	{return m_nUpDataOverheadSourceExchangePackets;}
	uint64_t	GetUpDataOverheadFileRequestPackets()		{return m_nUpDataOverheadFileRequestPackets;}
	uint64_t	GetUpDataOverheadServerPackets()			{return m_nUpDataOverheadServerPackets;}
	uint64_t	GetUpDataOverheadKadPackets()				{return m_nUpDataOverheadKadPackets;}
	uint64_t	GetUpDataOverheadOtherPackets()				{return m_nUpDataOverheadOtherPackets;}

public:
	//	Cumulative Stats
	static float	maxDown;
	static float	maxDownavg;
	static float	cumDownavg;
	static float	maxcumDownavg;
	static float	maxcumDown;
	static float	cumUpavg;
	static float	maxcumUpavg;
	static float	maxcumUp;
	static float	maxUp;
	static float	maxUpavg;
	static float	rateDown;
	static float	rateUp;
	static uint32_t	timeTransfers;
	static uint32_t	timeDownloads;
	static uint32_t	timeUploads;
	static uint32_t	start_timeTransfers;
	static uint32_t	start_timeDownloads;
	static uint32_t	start_timeUploads;
	static uint32_t	time_thisTransfer;
	static uint32_t	time_thisDownload;
	static uint32_t	time_thisUpload;
	static uint32_t	timeServerDuration;
	static uint32_t	time_thisServerDuration;
	static DWORD	m_dwOverallStatus;
	static float	m_fGlobalDone;
	static float	m_fGlobalSize;

	static uint64_t	sessionReceivedBytes;
	static uint64_t	sessionSentBytes;
    static uint64_t	sessionSentBytesToFriend;
	static uint16_t	reconnects;
	static DWORD	transferStarttime;
	static DWORD	serverConnectTime;
	static uint32_t	filteredclients;
	static DWORD	starttime;

private:
	typedef struct TransferredData {
		uint32_t	datalen;
		DWORD	timestamp;
	};
	std::list<TransferredData> uprateHistory;
	std::list<TransferredData> downrateHistory;

	static uint32_t	m_nDownDatarateOverhead;
	static uint32_t	m_nDownDataRateMSOverhead;
	static uint64_t	m_nDownDataOverheadSourceExchange;
	static uint64_t	m_nDownDataOverheadSourceExchangePackets;
	static uint64_t	m_nDownDataOverheadFileRequest;
	static uint64_t	m_nDownDataOverheadFileRequestPackets;
	static uint64_t	m_nDownDataOverheadServer;
	static uint64_t	m_nDownDataOverheadServerPackets;
	static uint64_t	m_nDownDataOverheadKad;
	static uint64_t	m_nDownDataOverheadKadPackets;
	static uint64_t	m_nDownDataOverheadOther;
	static uint64_t	m_nDownDataOverheadOtherPackets;

	static uint32_t	m_nUpDatarateOverhead;
	static uint32_t	m_nUpDataRateMSOverhead;
	static uint64_t	m_nUpDataOverheadSourceExchange;
	static uint64_t	m_nUpDataOverheadSourceExchangePackets;
	static uint64_t	m_nUpDataOverheadFileRequest;
	static uint64_t	m_nUpDataOverheadFileRequestPackets;
	static uint64_t	m_nUpDataOverheadServer;
	static uint64_t	m_nUpDataOverheadServerPackets;
	static uint64_t	m_nUpDataOverheadKad;
	static uint64_t	m_nUpDataOverheadKadPackets;
	static uint64_t	m_nUpDataOverheadOther;
	static uint64_t	m_nUpDataOverheadOtherPackets;

	static uint32_t	m_sumavgDDRO;
	static uint32_t	m_sumavgUDRO;
	CList<TransferredData> m_AvarageDDRO_list;
	CList<TransferredData> m_AvarageUDRO_list;
};

extern CStatistics theStats;

#if !defined(_DEBUG) && !defined(_AFXDLL) && _MFC_VER==0x0710
//#define USE_MEM_STATS
#define	ALLOC_SLOTS	20
extern ULONGLONG g_aAllocStats[ALLOC_SLOTS];
#endif
