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

namespace Kademlia
{
	class CDataIO
	{
		public:
			byte ReadByte();
			uint8_t ReadUInt8();
			uint16_t ReadUInt16();
			uint32_t ReadUInt32();
			uint64_t ReadUInt64();
			void ReadUInt128(CUInt128* puValue);
			void ReadHash(BYTE* pbyValue);
			BYTE* ReadBsob(uint8_t* puSize);
			float ReadFloat();
			CStringW ReadStringUTF8(bool bOptACP = false);
			void	 WriteString(const CStringW& strVal);
			CKadTag* ReadTag(bool bOptACP = false);
			void ReadTagList(TagList* pTaglist, bool bOptACP = false);
			void WriteByte(byte byVal);
			void WriteUInt8(uint8_t uVal);
			void WriteUInt16(uint16_t uVal);
			void WriteUInt32(uint32_t uVal);
			void WriteUInt64(uint64_t uVal);
			void WriteUInt128(const CUInt128& uVal);
			void WriteHash( const BYTE* pbyVal);
			void WriteBsob( const BYTE* pbyVal, uint8_t uSize);
			void WriteFloat(float fVal);
			void WriteTag(const CKadTag* pTag);
			void WriteTag(LPCSTR szName, uint8_t uValue);
			void WriteTag(LPCSTR szName, uint16_t uValue);
			void WriteTag(LPCSTR szName, uint32_t uValue);
			void WriteTag(LPCSTR szName, uint64_t uValue);
			void WriteTag(LPCSTR szName, float fValue);
			void WriteTagList(const TagList& tagList);
			virtual void ReadArray(LPVOID lpResult, uint32_t uByteCount) = 0;
			virtual void WriteArray(LPCVOID lpVal, uint32_t uByteCount) = 0;
			virtual UINT GetAvailable() const = 0;
	};
}
