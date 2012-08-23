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
#include "../utils/UInt128.h"

namespace Kademlia
{
	struct TrackPackets_Struct{
		uint32_t dwIP;
		uint32_t dwInserted;
		uint8_t  byOpcode;
	};

	struct TrackChallenge_Struct{
		uint32_t uIP;
		uint32_t dwInserted;
		uint8_t  byOpcode;
		CUInt128 uContactID;
		CUInt128 uChallenge;
	};

	struct TrackPacketsIn_Struct{
		struct TrackedRequestIn_Struct{ // netfinity: Rearranged for alignment reasons
			uint32_t	m_nCount;
			uint32_t	m_dwFirstAdded;
			bool	m_bDbgLogged;
			uint8_t	m_byOpcode;
		};

		TrackPacketsIn_Struct() { m_dwLastExpire = 0; m_uIP = 0; }

		// netfinity: Rearranged for alignment reasons
		CArray<TrackedRequestIn_Struct> m_aTrackedRequests;
		uint32_t	m_uIP;
		uint32_t	m_dwLastExpire;	
	};

	class CPacketTracking
	{
		public:
			CPacketTracking();
			virtual ~CPacketTracking();

		protected:
			void AddTrackedOutPacket(uint32_t dwIP, uint8_t byOpcode);
			bool IsOnOutTrackList(uint32_t dwIP, uint8_t byOpcode, bool bDontRemove = false);
			bool InTrackListIsAllowedPacket(uint32_t uIP, uint8_t byOpcode, bool bValidReceiverkey);
			void InTrackListCleanup();
			void AddLegacyChallenge(CUInt128 uContactID, CUInt128 uChallengeID, uint32_t uIP, uint8_t byOpcode);
			bool IsLegacyChallenge(CUInt128 uChallengeID, uint32_t uIP, uint8_t byOpcode, CUInt128& ruContactID);
			bool HasActiveLegacyChallenge(uint32_t uIP) const;

		private:
			bool IsTrackedOutListRequestPacket(uint8_t byOpcode) const;
			CList<TrackPackets_Struct> listTrackedRequests;
			CList<TrackChallenge_Struct> listChallengeRequests;
			CTypedPtrList<CPtrList, TrackPacketsIn_Struct*>					m_liTrackPacketsIn;
			CMap<int, int, TrackPacketsIn_Struct*, TrackPacketsIn_Struct*>	m_mapTrackPacketsIn;
			uint32_t dwLastTrackInCleanup;
	};
}