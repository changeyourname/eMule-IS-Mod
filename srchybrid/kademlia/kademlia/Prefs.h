/*
Copyright (C)2003 Barry Dunne (http://www.emule-project.net)
 
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.
 
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
 
You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// Note To Mods //
/*
Please do not change anything here and release it..
There is going to be a new forum created just for the Kademlia side of the client..
If you feel there is an error or a way to improve something, please
post it in the forum first and let us look at it.. If it is a real improvement,
it will be added to the offical client.. Changing something without knowing
what all it does can cause great harm to the network if released in mass form..
Any mod that changes anything within the Kademlia side will not be allowed to advertise
there client on the eMule forum..
*/

#pragma once
#include "../utils/UInt128.h"

namespace Kademlia
{
	class CPrefs
	{
		public:
			CPrefs();
			~CPrefs();

			void		GetKadID(CUInt128 *puID) const;
			void		GetKadID(CString *psID) const;
			void		SetKadID(const CUInt128 &puID);
			CUInt128	GetKadID() const;
			void		GetClientHash(CUInt128 *puID) const;
			void		GetClientHash(CString *psID) const;
			void		SetClientHash(const CUInt128 &puID);
			CUInt128	GetClientHash() const;
			uint32_t		GetIPAddress() const;
			void		SetIPAddress(uint32_t uVal);
			bool		GetRecheckIP() const;
			void		SetRecheckIP();
			void		IncRecheckIP();
			bool		HasHadContact() const;
			void		SetLastContact();
			bool		HasLostConnection() const;
            time_t		GetLastContact() const { return m_tLastContact; }
			bool		GetFirewalled() const;
			void		SetFirewalled();
			void		IncFirewalled();

            uint8_t		GetTotalFile() const { return m_uTotalFile; } 
			void		SetTotalFile(uint8_t uVal);
			uint8_t		GetTotalStoreSrc() const;
			void		SetTotalStoreSrc(uint8_t uVal);
			uint8_t		GetTotalStoreKey() const;
			void		SetTotalStoreKey(uint8_t uVal);
			uint8_t		GetTotalSource() const;
			void		SetTotalSource(uint8_t uVal);
			uint8_t		GetTotalNotes() const;
			void		SetTotalNotes(uint8_t uVal);
			uint8_t		GetTotalStoreNotes() const;
			void		SetTotalStoreNotes(uint8_t uVal);
			uint32_t		GetKademliaUsers() const;
			void		SetKademliaUsers(uint32_t uVal);
			uint32_t		GetKademliaFiles() const;
			void		SetKademliaFiles();
			bool		GetPublish() const;
			void		SetPublish(bool bVal);
			bool		GetFindBuddy();
			void		SetFindBuddy(bool bVal = true);
			bool		GetUseExternKadPort() const;
			void		SetUseExternKadPort(bool bVal);
			uint16_t		GetExternalKadPort() const;
			void		SetExternKadPort(uint16_t uVal, uint32_t nFromIP);
			bool		FindExternKadPort(bool bReset = false);
			uint16_t		GetInternKadPort() const;
			uint8_t		GetMyConnectOptions(bool bEncryption = true, bool bCallback = true);
			void		StatsIncUDPFirewalledNodes(bool bFirewalled);
			void		StatsIncTCPFirewalledNodes(bool bFirewalled);
			float		StatsGetFirewalledRatio(bool bUDP) const;
			float		StatsGetKadV8Ratio();

			static uint32_t GetUDPVerifyKey(uint32_t dwTargetIP);
		private:
			void Init(LPCTSTR szFilename);
			void Reset();
			void SetDefaults();
			void ReadFile();
			void WriteFile();
			CString	m_sFilename;
			time_t m_tLastContact;
			CUInt128 m_uClientID;
			CUInt128 m_uClientHash;
			uint32_t m_uIP;
			uint32_t m_uIPLast;
			uint32_t m_uRecheckip;
			uint32_t m_uFirewalled;
			uint32_t m_uKademliaUsers;
			uint32_t m_uKademliaFiles;
			uint8_t m_uTotalFile;
			uint8_t m_uTotalStoreSrc;
			uint8_t m_uTotalStoreKey;
			uint8_t m_uTotalSource;
			uint8_t m_uTotalNotes;
			uint8_t m_uTotalStoreNotes;
			bool m_bPublish;
			bool m_bFindBuddy;
			bool m_bLastFirewallState;
			bool m_bUseExternKadPort;
			uint16_t m_nExternKadPort;
			CArray<uint32_t> m_anExternPortIPs;
			CArray<uint16_t> m_anExternPorts;
			uint32_t m_nStatsUDPOpenNodes;
			uint32_t m_nStatsUDPFirewalledNodes;
			uint32_t m_nStatsTCPOpenNodes;
			uint32_t m_nStatsTCPFirewalledNodes;
			time_t m_nStatsKadV8LastChecked;
			float  m_fKadV8Ratio;
	};
}
