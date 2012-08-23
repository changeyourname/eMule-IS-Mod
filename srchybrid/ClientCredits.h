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

#pragma warning(push)
#pragma warning(disable:4244 4100 4189)
#include <cryptopp/rsa.h>
#pragma warning(pop)
#include "MapKey.h"

#define	 MAXPUBKEYSIZE		80

#define CRYPT_CIP_REMOTECLIENT	10
#define CRYPT_CIP_LOCALCLIENT	20
#define CRYPT_CIP_NONECLIENT	30

#pragma pack(1)
struct CreditStruct_29a{
	uchar		abyKey[16];
	uint32_t		nUploadedLo;	// uploaded TO him
	uint32_t		nDownloadedLo;	// downloaded from him
	uint32_t		nLastSeen;
	uint32_t		nUploadedHi;	// upload high 32
	uint32_t		nDownloadedHi;	// download high 32
	uint16_t		nReserved3;
};
struct CreditStruct{
	uchar		abyKey[16];
	uint32_t		nUploadedLo;	// uploaded TO him
	uint32_t		nDownloadedLo;	// downloaded from him
	uint32_t		nLastSeen;
	uint32_t		nUploadedHi;	// upload high 32
	uint32_t		nDownloadedHi;	// download high 32
	uint16_t		nReserved3;
	uint8_t		nKeySize;
	uchar		abySecureIdent[MAXPUBKEYSIZE];
};
#pragma pack()

enum EIdentState{
	IS_NOTAVAILABLE,
	IS_IDNEEDED,
	IS_IDENTIFIED,
	IS_IDFAILED,
	IS_IDBADGUY,
};

class CClientCredits
{
	friend class CClientCreditsList;
public:
	CClientCredits(CreditStruct* in_credits);
	CClientCredits(const uchar* key);
	~CClientCredits();

	const uchar* GetKey() const					{return m_pCredits->abyKey;}
	uchar*	GetSecureIdent()					{return m_abyPublicKey;}
	uint8_t	GetSecIDKeyLen() const				{return m_nPublicKeyLen;}
	CreditStruct* GetDataStruct() const			{return m_pCredits;}
	void	ClearWaitStartTime();
	void	AddDownloaded(uint32_t bytes, uint32_t dwForIP);
	void	AddUploaded(uint32_t bytes, uint32_t dwForIP);
	uint64_t	GetUploadedTotal() const;
	uint64_t	GetDownloadedTotal() const;
	float	GetScoreRatio(uint32_t dwForIP) const;
	float	GetRemoteScoreRatio() const; // netfinity: Remote score
	void	SetLastSeen()					{m_pCredits->nLastSeen = ConvertToTime32(time(NULL));}
	bool	SetSecureIdent(const uchar* pachIdent, uint8_t nIdentLen); // Public key cannot change, use only if there is not public key yet
	EIdentState	GetCurrentIdentState(uint32_t dwForIP) const; // can be != IdentState
	time_t	GetSecureWaitStartTime(uint32_t dwForIP);
	void	SetSecWaitStartTime(uint32_t dwForIP);
	// BEGIN netfinity: Known Client Bonus
	time_t	GetWaitingTimeBonus() const				{return m_dwWaitingTimeBonus;}
	// END netfinity: Known Client Bonus
protected:
	void	Verified(uint32_t dwForIP);
private:
	void			InitalizeIdent();
private: // netfinity: Rearranged for alignment reasons
	CreditStruct*	m_pCredits;
	time_t			m_dwSecureWaitTime;
	time_t			m_dwUnSecureWaitTime;
	time_t			m_dwWaitTimeIP;			   // client IP assigned to the waittime
	// BEGIN netfinity: Known Client Bonus
	time_t			m_dwWaitingTimeBonus;		//! Add this to the waiting time
	// END netfinity: Known Client Bonus
public:
	uint32_t	m_dwCryptRndChallengeFor;
	uint32_t	m_dwCryptRndChallengeFrom;
protected:
	EIdentState IdentState;
private:
	uint32_t			m_dwIdentIP;
	byte			m_abyPublicKey[80];			// even keys which are not verified will be stored here, and - if verified - copied into the struct
	uint8_t			m_nPublicKeyLen;
};

class CClientCreditsList
{
public:
	CClientCreditsList();
	~CClientCreditsList();
	
			// return signature size, 0 = Failed | use sigkey param for debug only
	uint8_t	CreateSignature(CClientCredits* pTarget, uchar* pachOutput, uint8_t nMaxSize, uint32_t ChallengeIP, uint8_t byChaIPKind, CryptoPP::RSASSA_PKCS1v15_SHA_Signer* sigkey = NULL);
	bool	VerifyIdent(CClientCredits* pTarget, const uchar* pachSignature, uint8_t nInputSize, uint32_t dwForIP, uint8_t byChaIPKind);

	CClientCredits* GetCredit(const uchar* key) ;
	void	Process();
	uint8_t	GetPubKeyLen() const			{return m_nMyPublicKeyLen;}
	byte*	GetPublicKey()					{return m_abyMyPublicKey;}
	bool	CryptoAvailable();
protected:
	/* ismod: return load result
	void	LoadList();
	*/
	bool	LoadList();
	void	SaveList();
	void	InitalizeCrypting();
	bool	CreateKeyPair();
#ifdef _DEBUG
	bool	Debug_CheckCrypting();
#endif
private:
	CRBMap<CCKey,CClientCredits*, CCKeyTrait> m_mapClients;
	clock_t			m_nLastSaved;
	CryptoPP::RSASSA_PKCS1v15_SHA_Signer*		m_pSignkey;
	byte			m_abyMyPublicKey[80];
	uint8_t			m_nMyPublicKeyLen;
};
