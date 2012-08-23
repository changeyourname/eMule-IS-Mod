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

class CTag;
class CFileDataIO;

#pragma pack(1)
struct ServerMet_Struct {
	uint32_t	ip;
	uint16_t	port;
	uint32_t	tagcount;
};
#pragma pack()

#define SRV_PR_LOW			2
#define SRV_PR_NORMAL		0
#define SRV_PR_HIGH			1


// Server TCP flags
#define	SRV_TCPFLG_COMPRESSION		0x00000001
#define	SRV_TCPFLG_NEWTAGS			0x00000008
#define	SRV_TCPFLG_UNICODE			0x00000010
#define SRV_TCPFLG_RELATEDSEARCH	0x00000040
#define SRV_TCPFLG_TYPETAGINTEGER	0x00000080
#define SRV_TCPFLG_LARGEFILES		0x00000100
#define SRV_TCPFLG_TCPOBFUSCATION	0x00000400

// Server UDP flags
#define	SRV_UDPFLG_EXT_GETSOURCES	0x00000001
#define	SRV_UDPFLG_EXT_GETFILES		0x00000002
#define	SRV_UDPFLG_NEWTAGS			0x00000008
#define	SRV_UDPFLG_UNICODE			0x00000010
#define	SRV_UDPFLG_EXT_GETSOURCES2	0x00000020
#define SRV_UDPFLG_LARGEFILES		0x00000100
#define SRV_UDPFLG_UDPOBFUSCATION	0x00000200
#define SRV_UDPFLG_TCPOBFUSCATION	0x00000400

class CServer{
public:
	CServer(const ServerMet_Struct* in_data);
	CServer(uint16_t in_port, LPCTSTR i_addr);
	CServer(const CServer* pOld);
	~CServer();

	bool	AddTagFromFile(CFileDataIO* servermet);

	const CString& GetListName() const				{return m_strName;}
	void	SetListName(LPCTSTR newname);

	const CString& GetDescription() const			{return m_strDescription;}
	void	SetDescription(LPCTSTR newdescription);

	uint32_t	GetIP() const							{return ip;}
	void	SetIP(uint32_t newip);

	const CString& GetDynIP() const					{return m_strDynIP;}
	bool	HasDynIP() const						{return !m_strDynIP.IsEmpty();}
	void	SetDynIP(LPCTSTR newdynip);

	LPCTSTR	GetFullIP() const						{return ipfull;}
	LPCTSTR	GetAddress() const;
	uint16_t	GetPort() const							{return port;}

	uint32_t	GetFiles() const						{return files;}
	void	SetFileCount(uint32_t in_files)			{files = in_files;}

	uint32_t	GetUsers() const						{return users;}
	void	SetUserCount(uint32_t in_users)			{users = in_users;}

	UINT	GetPreference() const					{return m_uPreference;}
	void	SetPreference(UINT uPreference)			{m_uPreference = uPreference;}

	uint32_t	GetPing() const							{return ping;}
	void	SetPing(uint32_t in_ping)					{ping = in_ping;}

	uint32_t	GetMaxUsers() const						{return maxusers;}
	void	SetMaxUsers(uint32_t in_maxusers) 		{maxusers = in_maxusers;}

	uint32_t	GetFailedCount() const					{return failedcount;}
	void	SetFailedCount(uint32_t nCount)			{failedcount = nCount;}
	void	AddFailedCount()						{failedcount++;} 
	void	ResetFailedCount()						{failedcount = 0;}

	uint32_t	GetLastPingedTime() const				{return lastpingedtime;}
	void	SetLastPingedTime(uint32_t in_lastpingedtime)	{lastpingedtime = in_lastpingedtime;}

	uint32_t	GetRealLastPingedTime() const					{return m_dwRealLastPingedTime;} // last pinged time without any random modificator
	void	SetRealLastPingedTime(uint32_t in_lastpingedtime)	{m_dwRealLastPingedTime = in_lastpingedtime;}

	uint32_t	GetLastPinged() const					{return lastpinged;}
	void	SetLastPinged(uint32_t in_lastpinged)		{lastpinged = in_lastpinged;}

	UINT	GetLastDescPingedCount() const			{return lastdescpingedcout;}
	void	SetLastDescPingedCount(bool reset);

	bool	IsStaticMember() const					{return staticservermember;}
	void	SetIsStaticMember(bool in)				{staticservermember = in;}

	uint32_t	GetChallenge() const					{return challenge;}
	void	SetChallenge(uint32_t in_challenge)		{challenge = in_challenge;}

	uint32_t	GetDescReqChallenge() const				{return m_uDescReqChallenge;}
	void	SetDescReqChallenge(uint32_t uDescReqChallenge) {m_uDescReqChallenge = uDescReqChallenge;}

	uint32_t	GetSoftFiles() const					{return softfiles;}
	void	SetSoftFiles(uint32_t in_softfiles)		{softfiles = in_softfiles;}

	uint32_t	GetHardFiles() const					{return hardfiles;}
	void	SetHardFiles(uint32_t in_hardfiles)		{hardfiles = in_hardfiles;}

	const CString& GetVersion() const				{return m_strVersion;}
	void	SetVersion(LPCTSTR pszVersion)			{m_strVersion = pszVersion;}

	uint32_t	GetTCPFlags() const						{return m_uTCPFlags;}
	void	SetTCPFlags(uint32_t uFlags)				{m_uTCPFlags = uFlags;}

	uint32_t	GetUDPFlags() const						{return m_uUDPFlags;}
	void	SetUDPFlags(uint32_t uFlags)				{m_uUDPFlags = uFlags;}

	uint32_t	GetLowIDUsers() const					{return m_uLowIDUsers;}
	void	SetLowIDUsers(uint32_t uLowIDUsers)		{m_uLowIDUsers = uLowIDUsers;}

	uint16_t	GetObfuscationPortTCP() const			{return m_nObfuscationPortTCP;}
	void	SetObfuscationPortTCP(uint16_t nPort)		{m_nObfuscationPortTCP = nPort;}

	uint16_t	GetObfuscationPortUDP() const			{return m_nObfuscationPortUDP;}
	void	SetObfuscationPortUDP(uint16_t nPort)		{m_nObfuscationPortUDP = nPort;}

	uint32_t	GetServerKeyUDP(bool bForce = false) const;
	void	SetServerKeyUDP(uint32_t dwServerKeyUDP);

	bool	GetCryptPingReplyPending() const		{return m_bCryptPingReplyPending;}
	void	SetCryptPingReplyPending(bool bVal)		{m_bCryptPingReplyPending = bVal;}

	uint32_t	GetServerKeyUDPIP() const				{return m_dwIPServerKeyUDP;}

	bool	GetUnicodeSupport() const				{return (GetTCPFlags() & SRV_TCPFLG_UNICODE)!=0;}
	bool	GetRelatedSearchSupport() const			{return (GetTCPFlags() & SRV_TCPFLG_RELATEDSEARCH)!=0;}
	bool	SupportsLargeFilesTCP() const			{return (GetTCPFlags() & SRV_TCPFLG_LARGEFILES)!=0;}
	bool	SupportsLargeFilesUDP() const			{return (GetUDPFlags() & SRV_UDPFLG_LARGEFILES)!=0;}
	bool	SupportsObfuscationUDP() const			{return (GetUDPFlags() & SRV_UDPFLG_UDPOBFUSCATION)!=0;}
	bool	SupportsObfuscationTCP() const			{return GetObfuscationPortTCP() != 0 && ((GetUDPFlags() & SRV_UDPFLG_TCPOBFUSCATION)!=0 || (GetTCPFlags() & SRV_TCPFLG_TCPOBFUSCATION)!=0);}
	bool	SupportsGetSourcesObfuscation() const	{return (GetTCPFlags() & SRV_TCPFLG_TCPOBFUSCATION)!=0;} // mapped to TCPFLAG_TCPOBFU

	bool	IsEqual(const CServer* pServer) const;

private:
	uint32_t		challenge;
	uint32_t		m_uDescReqChallenge;
	uint32_t		lastpinged; //This is to get the ping delay.
	uint32_t		lastpingedtime; //This is to decided when we retry the ping.
	uint32_t		lastdescpingedcout;
	uint32_t		files;
	uint32_t		users;
	uint32_t		maxusers;
	uint32_t		softfiles;
	uint32_t		hardfiles;
	UINT		m_uPreference;
	uint32_t		ping;
	CString		m_strDescription;
	CString		m_strName;
	CString		m_strDynIP;
	TCHAR		ipfull[3+1+3+1+3+1+3+1]; // 16
	uint32_t		ip;
	uint16_t		port;
	bool		staticservermember;
	bool		m_bCryptPingReplyPending;
	uint32_t		failedcount; 
	CString		m_strVersion;
	uint32_t		m_uTCPFlags;
	uint32_t		m_uUDPFlags;
	uint32_t		m_uLowIDUsers;
	uint32_t		m_dwServerKeyUDP;
	uint32_t		m_dwIPServerKeyUDP;
	uint16_t		m_nObfuscationPortTCP;
	uint16_t		m_nObfuscationPortUDP;
	uint32_t		m_dwRealLastPingedTime;
};