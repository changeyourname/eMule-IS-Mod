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
#include "../kademlia/Tag.h"

class CKnownFile;
class CSafeMemFile;
struct SSearchTerm;

namespace Kademlia
{
	void deleteTagListEntries(TagList* plistTag);
	class CByteIO;
	class CKadClientSearcher;
	class CSearch
	{
			friend class CSearchManager;
		public:
			uint32_t		GetSearchID() const;
			uint32_t		GetSearchTypes() const;
			void		SetSearchTypes( uint32_t uVal );
			void		SetTargetID( CUInt128 uVal );
			CUInt128	GetTarget() const;
			uint32_t		GetAnswers() const;
			uint32_t		GetKadPacketSent() const;
			uint32_t		GetRequestAnswer() const;
			uint32_t		GetNodeLoad() const;
			uint32_t		GetNodeLoadResonse() const;
			uint32_t		GetNodeLoadTotal() const;
			const		CKadTagValueString& GetGUIName() const;
			void		SetGUIName(const CKadTagValueString& sGUIName);
			void		SetSearchTermData( uint32_t uSearchTermDataSize, LPBYTE pucSearchTermsData );

			void		AddFileID(const CUInt128& uID);
			void		PreparePacketForTags( CByteIO* pbyPacket, CKnownFile* pFile );
			bool		Stoping() const;
			void		UpdateNodeLoad( uint8_t uLoad );
			
			CKadClientSearcher*	GetNodeSpecialSearchRequester() const						{ return pNodeSpecialSearchRequester; }
			void				SetNodeSpecialSearchRequester(CKadClientSearcher* pNew)		{ pNodeSpecialSearchRequester = pNew; } 
			
			enum {
			    NODE,
			    NODECOMPLETE,
			    FILE,
			    KEYWORD,
			    NOTES,
			    STOREFILE,
			    STOREKEYWORD,
			    STORENOTES,
			    FINDBUDDY,
			    FINDSOURCE,
				NODESPECIAL, // nodesearch request from requester "outside" of kad to find the IP of a given nodeid
				NODEFWCHECKUDP // find new unknown IPs for a UDP firewallcheck
			};

			CSearch();
			~CSearch();

		private:
			void Go();
			void ProcessResponse(uint32_t uFromIP, uint16_t uFromPort, ContactList *plistResults);
			void ProcessResult(const CUInt128 &uAnswer, TagList *listInfo);
			void ProcessResultFile(const CUInt128 &uAnswer, TagList *listInfo);
			void ProcessResultKeyword(const CUInt128 &uAnswer, TagList *listInfo);
			void ProcessResultNotes(const CUInt128 &uAnswer, TagList *listInfo);
			void JumpStart();
			void SendFindValue(CContact* pContact);
			void PrepareToStop();
			void StorePacket();
			uint8_t GetRequestContactCount() const;

			bool m_bStoping;
			time_t m_tCreated;
			uint32_t m_uType;
			uint32_t m_uAnswers;
			uint32_t m_uTotalRequestAnswers;
			uint32_t m_uKadPacketSent; //Used for gui reasons.. May not be needed later..
			uint32_t m_uTotalLoad;
			uint32_t m_uTotalLoadResponses;
			time_t m_uLastResponse;
			uint32_t m_uSearchID;
			uint32_t m_uSearchTermsDataSize;
			LPBYTE m_pucSearchTermsData;
			SSearchTerm* m_pSearchTerm; // cached from m_pucSearchTermsData, used for verifying results lateron
			CKadClientSearcher* pNodeSpecialSearchRequester; // used to callback on result for NODESPECIAL searches
			CUInt128 m_uTarget;
			WordList m_listWords;
			CKadTagValueString m_sGUIName;
			UIntList m_listFileIDs;
			ContactMap m_mapPossible;
			ContactMap m_mapTried;
			ContactMap m_mapResponded;
			ContactMap m_mapBest;
			ContactList m_listDelete;
			ContactMap m_mapInUse;
			CUInt128 m_uClosestDistantFound; // not used for the search itself, but for statistical data collecting
	};
}
void KadGetKeywordHash(const Kademlia::CKadTagValueString& rstrKeywordW, Kademlia::CUInt128* puKadID);
void KadGetKeywordHash(const CStringA& rstrKeywordA, Kademlia::CUInt128* puKadID);
CStringA KadGetKeywordBytes(const Kademlia::CKadTagValueString& rstrKeywordW);
