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
 
 
This work is based on the java implementation of the Kademlia protocol.
Kademlia: Peer-to-peer routing based on the XOR metric
Copyright (C) 2002  Petar Maymounkov [petar@post.harvard.edu]
http://kademlia.scs.cs.nyu.edu
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
#include "./Maps.h"

namespace Kademlia
{
	class CRoutingBin
	{
	public:
		~CRoutingBin();
		CRoutingBin();
		bool AddContact(CContact* pContact);
		void SetAlive(CContact* pContact);
		void SetTCPPort(uint32_t uIP, uint16_t uUDPPort, uint16_t uTCPPort);
		void RemoveContact(CContact *pContact, bool bNoTrackingAdjust = false);
		CContact *GetContact(const CUInt128 &uID);
		CContact* GetContact(uint32_t uIP, uint16_t nPort, bool bTCPPort);
		CContact *GetOldest();
		UINT GetSize() const;
		void GetNumContacts(uint32_t& nInOutContacts, uint32_t& nInOutFilteredContacts, uint8_t byMinVersion) const;
		UINT GetRemaining() const;
		void GetEntries(ContactList *plistResult, bool bEmptyFirst = true);
		void GetClosestTo(uint32_t uMaxType, const CUInt128 &uTarget, uint32_t uMaxRequired, ContactMap *pmapResult, bool bEmptyFirst = true, bool bSetInUse = false);
		bool ChangeContactIPAddress(CContact* pContact, uint32_t uNewIP);
		void PushToBottom(CContact* pContact); // puts an existing contact from X to the end of the list
		CContact* GetRandomContact(uint32_t nMaxType, uint32_t nMinKadVersion);
		void SetAllContactsVerified();
		static bool CheckGlobalIPLimits(uint32_t uIP, uint16_t uPort, bool bLog);

		bool m_bDontDeleteContacts;

	protected:
		static void			AdjustGlobalTracking(uint32_t uIP, bool bIncrease);
	
	private:
		ContactList m_listEntries;

		static CMap<uint32_t, uint32_t, uint32_t, uint32_t> s_mapGlobalContactIPs;
		static CMap<uint32_t, uint32_t, uint32_t, uint32_t> s_mapGlobalContactSubnets;
	};
}
