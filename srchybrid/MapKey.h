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
#include "otherfunctions.h"
// use this class if the hash is stored somehwere else (and stays valid as long as this object exists)
class CCKey : public CObject{
public:
	CCKey(const uchar* key = 0)	{m_key = key;}
	CCKey(const CCKey& k1)		{m_key = k1.m_key;}

	CCKey& operator=(const CCKey& k1)						{m_key = k1.m_key; return *this; }
    friend bool operator==(const CCKey& k1,const CCKey& k2)  { return !md4cmp(k1.m_key, k2.m_key); }
    friend bool operator<(const CCKey& k1,const CCKey& k2)  { return memcmp(k1.m_key, k2.m_key,16)<0; }
    friend bool operator>(const CCKey& k1,const CCKey& k2)  { return memcmp(k1.m_key, k2.m_key,16)>0; }
	const uchar* m_key;
};

template<> inline UINT AFXAPI HashKey(const CCKey& key){
   uint32_t hash = 1;
   for (int i = 0;i != 16;i++)
	   hash += (key.m_key[i]+1)*((i*i)+1);
   return hash;
};

// ismod: trait for crbmap
class CCKeyTrait: public CElementTraits< CCKey >
{
    static ULONG Hash( const CCKey& key ) throw()
    {
        uint32_t hash = 1;
        for (int i = 0;i != 16;i++)
            hash += (key.m_key[i]+1)*((i*i)+1);
        return hash;
    }
};
// ismod [end]

// use this class if the hash is stored somehwere inside the key (in any case safer but needs more memory)
class CSKey : public CObject{
public:
	CSKey(const uchar* key = 0)	{ if(key) md4cpy(m_key, key); else md4clr(m_key); }
	CSKey(const CSKey& k1)		{ md4cpy(m_key, k1.m_key); }

	CSKey& operator=(const CSKey& k1)						{md4cpy(m_key, k1.m_key); return *this; }
    friend bool operator==(const CSKey& k1,const CSKey& k2) { return !md4cmp(k1.m_key, k2.m_key); }
    friend bool operator<(const CSKey& k1,const CSKey& k2)  { return memcmp(k1.m_key, k2.m_key,16)<0; }
    friend bool operator>(const CSKey& k1,const CSKey& k2)  { return memcmp(k1.m_key, k2.m_key,16)>0; }
	
	uchar m_key[16];
};

template<> inline UINT AFXAPI HashKey(const CSKey& key){
   uint32_t hash = 1;
   for (int i = 0;i != 16;i++)
	   hash += (key.m_key[i]+1)*((i*i)+1);
   return hash;
};

// ismod: trait for crbmap
class CSKeyTrait: public CElementTraits< CSKey >
{
    static ULONG Hash( const CSKey& key ) throw()
    {
        uint32_t hash = 1;
        for (int i = 0;i != 16;i++)
            hash += (key.m_key[i]+1)*((i*i)+1);
        return hash;
    }
};
// ismod [end]