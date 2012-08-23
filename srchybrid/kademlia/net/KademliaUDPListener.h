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
#include "../routing/Maps.h"
#include "./PacketTracking.h"

class CSafeMemFile;
struct SSearchTerm;

namespace Kademlia
{
	class CKadUDPKey;
	class CKadClientSearcher;

	struct FetchNodeID_Struct{
		uint32_t dwIP;
		uint32_t dwTCPPort;
		uint32_t dwExpire;
		CKadClientSearcher* pRequester;
	};

	class CKademliaUDPListener : public CPacketTracking
	{
			friend class CSearch;
		public:
			~CKademliaUDPListener();
			void Bootstrap(LPCTSTR uIP, uint16_t uUDPPort, bool bKad2);
			void Bootstrap(uint32_t uIP, uint16_t uUDPPort, bool bKad2, uint8_t byKadVersion = 0, const CUInt128* uCryptTargetID = NULL);
			void FirewalledCheck(uint32_t uIP, uint16_t uUDPPort, CKadUDPKey senderUDPKey, uint8_t byKadVersion);
			void SendMyDetails(byte byOpcode, uint32_t uIP, uint16_t uUDPPort, uint8_t byKadVersion, CKadUDPKey targetUDPKey, const CUInt128* uCryptTargetID, bool bRequestAckPackage);
			void SendPublishSourcePacket(const CContact* pContact, const CUInt128& uTargetID, const CUInt128& uContactID, const TagList& tags);
			void SendNullPacket(byte byOpcode, uint32_t uIP, uint16_t uUDPPort, CKadUDPKey targetUDPKey, const CUInt128* uCryptTargetID);
			virtual void ProcessPacket(const byte* pbyData, uint32_t uLenData, uint32_t uIP, uint16_t uUDPPort, bool bValidReceiverKey, CKadUDPKey senderUDPKey);
			void SendPacket(const byte* pbyData, uint32_t uLenData, uint32_t uDestinationHost, uint16_t uDestinationPort, CKadUDPKey targetUDPKey, const CUInt128* uCryptTargetID);
			void SendPacket(const byte *pbyData, uint32_t uLenData, byte byOpcode, uint32_t uDestinationHost, uint16_t uDestinationPort, CKadUDPKey targetUDPKey, const CUInt128* uCryptTargetID);
			void SendPacket(CSafeMemFile* pfileData, byte byOpcode, uint32_t uDestinationHost, uint16_t uDestinationPort, CKadUDPKey targetUDPKey, const CUInt128* uCryptTargetID);
			
			bool FindNodeIDByIP(CKadClientSearcher* pRequester, uint32_t dwIP, uint16_t nTCPPort, uint16_t nUDPPort);
			void ExpireClientSearch(CKadClientSearcher* pExpireImmediately = NULL);
		private:
			bool AddContact (const byte* pbyData, uint32_t uLenData, uint32_t uIP, uint16_t uUDPPort, uint16_t uTCPPort, CKadUDPKey cUDPKey, bool& bIPVerified, bool bUpdate, CUInt128* puOutContactID);
			bool AddContact_KADEMLIA2 (const byte* pbyData, uint32_t uLenData, uint32_t uIP, uint16_t& uUDPPort, uint8_t* pnOutVersion, CKadUDPKey cUDPKey, bool& rbIPVerified, bool bUpdate, bool bFromHelloReq, bool* pbOutRequestsACK, CUInt128* puOutContactID);
			void AddContacts(const byte* pbyData, uint32_t uLenData, uint16_t uNumContacts, bool bUpdate);
			void SendLegacyChallenge(uint32_t uIP, uint16_t uUDPPort, const CUInt128& uContactID, bool bKad2);
			static SSearchTerm* CreateSearchExpressionTree(CSafeMemFile& fileIO, int iLevel);
			static void Free(SSearchTerm* pSearchTerms);
			void Process_KADEMLIA_BOOTSTRAP_REQ (const byte* pbyPacketData, uint32_t uLenPacket, uint32_t uIP, uint16_t uUDPPort);
			void Process_KADEMLIA2_BOOTSTRAP_REQ (uint32_t uIP, uint16_t uUDPPort, CKadUDPKey senderUDPKey);
			void Process_KADEMLIA_BOOTSTRAP_RES (const byte* pbyPacketData, uint32_t uLenPacket, uint32_t uIP);
			void Process_KADEMLIA2_BOOTSTRAP_RES (const byte* pbyPacketData, uint32_t uLenPacket, uint32_t uIP, uint16_t uUDPPort, CKadUDPKey senderUDPKey, bool bValidReceiverKey);
			void Process_KADEMLIA_HELLO_REQ (const byte* pbyPacketData, uint32_t uLenPacket, uint32_t uIP, uint16_t uUDPPort);
			void Process_KADEMLIA2_HELLO_REQ (const byte* pbyPacketData, uint32_t uLenPacket, uint32_t uIP, uint16_t uUDPPort, CKadUDPKey senderUDPKey, bool bValidReceiverKey);
			void Process_KADEMLIA_HELLO_RES (const byte* pbyPacketData, uint32_t uLenPacket, uint32_t uIP, uint16_t uUDPPort);
			void Process_KADEMLIA2_HELLO_RES (const byte* pbyPacketData, uint32_t uLenPacket, uint32_t uIP, uint16_t uUDPPort, CKadUDPKey senderUDPKey, bool bValidReceiverKey);
			void Process_KADEMLIA2_HELLO_RES_ACK (const byte* pbyPacketData, uint32_t uLenPacket, uint32_t uIP, bool bValidReceiverKey);
			void Process_KADEMLIA_REQ (const byte* pbyPacketData, uint32_t uLenPacket, uint32_t uIP, uint16_t uUDPPort);
			void Process_KADEMLIA2_REQ (const byte* pbyPacketData, uint32_t uLenPacket, uint32_t uIP, uint16_t uUDPPort, CKadUDPKey senderUDPKey);
			void Process_KADEMLIA_RES (const byte* pbyPacketData, uint32_t uLenPacket, uint32_t uIP, uint16_t uUDPPort);
			void Process_KADEMLIA2_RES (const byte* pbyPacketData, uint32_t uLenPacket, uint32_t uIP, uint16_t uUDPPort, CKadUDPKey senderUDPKey);
			void Process_KADEMLIA_SEARCH_REQ (const byte* pbyPacketData, uint32_t uLenPacket, uint32_t uIP, uint16_t uUDPPort);
			void Process_KADEMLIA2_SEARCH_KEY_REQ (const byte* pbyPacketData, uint32_t uLenPacket, uint32_t uIP, uint16_t uUDPPort, CKadUDPKey senderUDPKey);
			void Process_KADEMLIA2_SEARCH_SOURCE_REQ (const byte* pbyPacketData, uint32_t uLenPacket, uint32_t uIP, uint16_t uUDPPort, CKadUDPKey senderUDPKey);
			void Process_KADEMLIA_SEARCH_RES (const byte* pbyPacketData, uint32_t uLenPacket);
			void Process_KADEMLIA2_SEARCH_RES (const byte* pbyPacketData, uint32_t uLenPacket, CKadUDPKey senderUDPKey);
			void Process_KADEMLIA_PUBLISH_REQ (const byte* pbyPacketData, uint32_t uLenPacket, uint32_t uIP, uint16_t uUDPPort);
			void Process_KADEMLIA2_PUBLISH_KEY_REQ (const byte* pbyPacketData, uint32_t uLenPacket, uint32_t uIP, uint16_t uUDPPort, CKadUDPKey senderUDPKey);
			void Process_KADEMLIA2_PUBLISH_SOURCE_REQ (const byte* pbyPacketData, uint32_t uLenPacket, uint32_t uIP, uint16_t uUDPPort, CKadUDPKey senderUDPKey);
			void Process_KADEMLIA_PUBLISH_RES (const byte* pbyPacketData, uint32_t uLenPacket, uint32_t uIP);
			void Process_KADEMLIA2_PUBLISH_RES (const byte* pbyPacketData, uint32_t uLenPacket, uint32_t uIP, uint16_t uUDPPort, CKadUDPKey senderUDPKey);
			void Process_KADEMLIA_SEARCH_NOTES_REQ (const byte* pbyPacketData, uint32_t uLenPacket, uint32_t uIP, uint16_t uUDPPort);
			void Process_KADEMLIA2_SEARCH_NOTES_REQ (const byte* pbyPacketData, uint32_t uLenPacket, uint32_t uIP, uint16_t uUDPPort, CKadUDPKey senderUDPKey);
			void Process_KADEMLIA_SEARCH_NOTES_RES (const byte* pbyPacketData, uint32_t uLenPacket, uint32_t uIP);
			void Process_KADEMLIA_PUBLISH_NOTES_REQ (const byte* pbyPacketData, uint32_t uLenPacket, uint32_t uIP, uint16_t uUDPPort);
			void Process_KADEMLIA2_PUBLISH_NOTES_REQ (const byte* pbyPacketData, uint32_t uLenPacket, uint32_t uIP, uint16_t uUDPPort, CKadUDPKey senderUDPKey);
			void Process_KADEMLIA_PUBLISH_NOTES_RES (const byte* pbyPacketData, uint32_t uLenPacket, uint32_t uIP);
			void Process_KADEMLIA_FIREWALLED_REQ (const byte* pbyPacketData, uint32_t uLenPacket, uint32_t uIP, uint16_t uUDPPort, CKadUDPKey senderUDPKey);
			void Process_KADEMLIA_FIREWALLED2_REQ (const byte* pbyPacketData, uint32_t uLenPacket, uint32_t uIP, uint16_t uUDPPort, CKadUDPKey senderUDPKey);
			void Process_KADEMLIA_FIREWALLED_RES (const byte* pbyPacketData, uint32_t uLenPacket, uint32_t uIP, CKadUDPKey senderUDPKey);
			void Process_KADEMLIA_FIREWALLED_ACK_RES (uint32_t uLenPacket);
			void Process_KADEMLIA_FINDBUDDY_REQ (const byte* pbyPacketData, uint32_t uLenPacket, uint32_t uIP, uint16_t uUDPPort, CKadUDPKey senderUDPKey);
			void Process_KADEMLIA_FINDBUDDY_RES (const byte* pbyPacketData, uint32_t uLenPacket, uint32_t uIP, uint16_t uUDPPort, CKadUDPKey senderUDPKey);
			void Process_KADEMLIA_CALLBACK_REQ (const byte* pbyPacketData, uint32_t uLenPacket, uint32_t uIP, CKadUDPKey senderUDPKey);
			void Process_KADEMLIA2_PING (uint32_t uIP, uint16_t uUDPPort, CKadUDPKey senderUDPKey);
			void Process_KADEMLIA2_PONG (const byte* pbyPacketData, uint32_t uLenPacket, uint32_t uIP, uint16_t uUDPPort, CKadUDPKey senderUDPKey);
			void Process_KADEMLIA2_FIREWALLUDP(const byte *pbyPacketData, uint32_t uLenPacket,uint32_t uIP, CKadUDPKey senderUDPKey);

			CList<FetchNodeID_Struct> listFetchNodeIDRequests;
			uint32_t	m_nOpenHellos;
			uint32_t	m_nFirewalledHellos;
	};
}
