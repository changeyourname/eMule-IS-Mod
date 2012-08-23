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

#include "stdafx.h"
#include <math.h>

#pragma warning(push)
#pragma warning(disable:4244 4100 4189)
#include <cryptopp/base64.h>
#include <cryptopp/osrng.h>
#include <cryptopp/files.h>
#include <cryptopp/sha.h>
#pragma warning(pop)

#include "emule.h"
#include "ClientCredits.h"
#include "OtherFunctions.h"
#include "Preferences.h"
#include "SafeFile.h"
#include "Opcodes.h"
#include "Sockets.h"
#include "emuledlg.h"
#include "Log.h"
#include "Reg2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define CLIENTS_MET_FILENAME	_T("clients.met")
#define CRYPTKEY_REG_KEY		_T("CryptKey")

CClientCredits::CClientCredits(CreditStruct* in_credits)
{
	m_pCredits = in_credits;
	InitalizeIdent();
	m_dwUnSecureWaitTime = 0;
	m_dwSecureWaitTime = 0;
	m_dwWaitTimeIP = 0;
	// BEGIN netfinity: Known Client Bonus
	m_dwWaitingTimeBonus = 0;
	// END netfinity: Known Client Bonus
}

CClientCredits::CClientCredits(const uchar* key)
{
	m_pCredits = new CreditStruct;
	memset(m_pCredits, 0, sizeof(CreditStruct));
	md4cpy(m_pCredits->abyKey, key);
	InitalizeIdent();
	m_dwUnSecureWaitTime = ::GetTickCount();
	m_dwSecureWaitTime = ::GetTickCount();
	m_dwWaitTimeIP = 0;
	// BEGIN netfinity: Known Client Bonus
	m_dwWaitingTimeBonus = 0;
	// END netfinity: Known Client Bonus
}

CClientCredits::~CClientCredits()
{
	delete m_pCredits;
}

void CClientCredits::AddDownloaded(uint32_t bytes, uint32_t dwForIP) {
	if ((GetCurrentIdentState(dwForIP) == IS_IDFAILED || GetCurrentIdentState(dwForIP) == IS_IDBADGUY || GetCurrentIdentState(dwForIP) == IS_IDNEEDED) && theApp.clientcredits->CryptoAvailable()) {
		return;
	}

	//encode
	uint64_t current = (((uint64_t)m_pCredits->nDownloadedHi << 32) | m_pCredits->nDownloadedLo) + bytes;

	//recode
	m_pCredits->nDownloadedLo = (uint32_t)current;
	m_pCredits->nDownloadedHi = (uint32_t)(current >> 32);
}

void CClientCredits::AddUploaded(uint32_t bytes, uint32_t dwForIP) {
	if ((GetCurrentIdentState(dwForIP) == IS_IDFAILED || GetCurrentIdentState(dwForIP) == IS_IDBADGUY || GetCurrentIdentState(dwForIP) == IS_IDNEEDED) && theApp.clientcredits->CryptoAvailable()) {
		return;
	}

	//encode
	uint64_t current = (((uint64_t)m_pCredits->nUploadedHi << 32) | m_pCredits->nUploadedLo) + bytes;

	//recode
	m_pCredits->nUploadedLo = (uint32_t)current;
	m_pCredits->nUploadedHi = (uint32_t)(current >> 32);

	m_dwWaitingTimeBonus = 0; // netfinity: Known Client Bonus - Client got what he wanted, so clear the bonus
}

uint64_t CClientCredits::GetUploadedTotal() const {
	return ((uint64_t)m_pCredits->nUploadedHi << 32) | m_pCredits->nUploadedLo;
}

uint64_t CClientCredits::GetDownloadedTotal() const {
	return ((uint64_t)m_pCredits->nDownloadedHi << 32) | m_pCredits->nDownloadedLo;
}

float CClientCredits::GetScoreRatio(uint32_t dwForIP) const
{
	// check the client ident status
	if ( ( GetCurrentIdentState(dwForIP) == IS_IDFAILED || GetCurrentIdentState(dwForIP) == IS_IDBADGUY || GetCurrentIdentState(dwForIP) == IS_IDNEEDED) && theApp.clientcredits->CryptoAvailable() ){
		// bad guy - no credits for you
		return 1.0F;
	}

	// BEGIN netfinity: Anti Shape - Make credit shaping less profitable
	if (thePrefs.TransferFullChunks())
	{
		if (GetDownloadedTotal() < (PARTSIZE / 2))
			return 1.0F;
	}
	else
	{
		if (GetDownloadedTotal() < 2000000)
			return 1.0F;
	}
	// END netfinity: Anti Shape
	float result = 0.0F;
	if (!GetUploadedTotal())
		result = 10.0F;
	else
		result = (float)(((double)GetDownloadedTotal()*2.0)/(double)GetUploadedTotal());
	
	// exponential calcualtion of the max multiplicator based on uploaded data (9.2MB = 3.34, 100MB = 10.0)
	float result2 = 0.0F;
	result2 = (float)(GetDownloadedTotal()/1048576.0);
	result2 += 2.0F;
	result2 = (float)sqrt(result2);

	// linear calcualtion of the max multiplicator based on uploaded data for the first chunk (1MB = 1.01, 9.2MB = 3.34)
	float result3 = 10.0F;
	if (GetDownloadedTotal() < 9646899){
		result3 = (((float)(GetDownloadedTotal() - 1048576) / 8598323.0F) * 2.34F) + 1.0F;
	}

	// take the smallest result
	result = min(result, min(result2, result3));

	if (result < 1.0F)
		return 1.0F;
	else if (result > 10.0F)
		return 10.0F;
	return result;
}

// BEGIN netfinity: Remote score
float CClientCredits::GetRemoteScoreRatio() const
{
	if (GetUploadedTotal() < 1000000)
		return 1.0F;

	float result = 0.0F;
	if (!GetDownloadedTotal())
		result = 10.0F;
	else
		result = (float)(((double)GetUploadedTotal()*2.0)/(double)GetDownloadedTotal());
	float result2 = 0.0F;
	result2 = (float)(GetUploadedTotal()/1048576.0);
	result2 += 2.0F;
	result2 = (float)sqrt(result2);

	// linear calcualtion of the max multiplicator based on uploaded data for the first chunk (1MB = 1.01, 9.2MB = 3.34)
	float result3 = 10.0F;
	if (GetDownloadedTotal() < 9646899){
		result3 = (((float)(GetDownloadedTotal() - 1048576) / 8598323.0F) * 2.34F) + 1.0F;
	}

	// take the smallest result
	result = min(result, min(result2, result3));

	if (result < 1.0F)
		return 1.0F;
	else if (result > 10.0F)
		return 10.0F;
	return result;
}
// END netfinity: Remote score

CClientCreditsList::CClientCreditsList()
{
	m_nLastSaved = ::GetTickCount();
	/* ismod: try to load backup if master is corrupted
	LoadList();
	*/
	if ( !LoadList() )
	{
		CString strFileName = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + CLIENTS_MET_FILENAME;
		CString strBakFileName;
		strBakFileName.Format(_T("%s") CLIENTS_MET_FILENAME _T(".bak"), thePrefs.GetMuleDirectory(EMULE_CONFIGDIR));
		if (::CopyFile(strBakFileName, strFileName, FALSE))
		{
			LoadList();
		}
	}
	
	InitalizeCrypting();
}

CClientCreditsList::~CClientCreditsList()
{
	SaveList();
	CClientCredits* cur_credit;
	CCKey tmpkey(0);
	POSITION pos = m_mapClients.GetHeadPosition();
	while (pos){
		m_mapClients.GetNextAssoc(pos, tmpkey, cur_credit);
		delete cur_credit;
	}
	delete m_pSignkey;
}

/* ismod
void CClientCreditsList::LoadList()
*/
bool CClientCreditsList::LoadList()
{
	// ismod
	bool bCreditsLoaded = false;

	CString strFileName = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + CLIENTS_MET_FILENAME;
	const int iOpenFlags = CFile::modeRead|CFile::osSequentialScan|CFile::typeBinary|CFile::shareDenyWrite;
	CSafeBufferedFile file;
	CFileException fexp;
	if (!file.Open(strFileName, iOpenFlags, &fexp)){
		if (fexp.m_cause != CFileException::fileNotFound){
			CString strError(GetResString(IDS_ERR_LOADCREDITFILE));
			TCHAR szError[MAX_CFEXP_ERRORMSG];
			if (fexp.GetErrorMessage(szError, _countof(szError))){
				strError += _T(" - ");
				strError += szError;
			}
			LogError(LOG_STATUSBAR, _T("%s"), strError);
		}
		return false;
	}
	setvbuf(file.m_pStream, NULL, _IOFBF, 16384);
	
	try{
		uint8_t version = file.ReadUInt8();
		if (version != CREDITFILE_VERSION && version != CREDITFILE_VERSION_29){
			LogWarning(GetResString(IDS_ERR_CREDITFILEOLD));
			file.Close();
			return false;
		}

		/* ismod: crate backup if loaded successfully
		// everything is ok, lets see if the backup exist...
		CString strBakFileName;
		strBakFileName.Format(_T("%s") CLIENTS_MET_FILENAME _T(".bak"), thePrefs.GetMuleDirectory(EMULE_CONFIGDIR));
		
		DWORD dwBakFileSize = 0;
		BOOL bCreateBackup = TRUE;

		HANDLE hBakFile = ::CreateFile(strBakFileName, GENERIC_READ, FILE_SHARE_READ, NULL,
										OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hBakFile != INVALID_HANDLE_VALUE)
		{
			// Ok, the backup exist, get the size
			dwBakFileSize = ::GetFileSize(hBakFile, NULL); //debug
			if (dwBakFileSize > (DWORD)file.GetLength())
			{
				// the size of the backup was larger then the org. file, something is wrong here, don't overwrite old backup..
				bCreateBackup = FALSE;
			}
			//else: backup is smaller or the same size as org. file, proceed with copying of file
			::CloseHandle(hBakFile);
		}
		//else: the backup doesn't exist, create it

		if (bCreateBackup)
		{
			file.Close(); // close the file before copying

			if (!::CopyFile(strFileName, strBakFileName, FALSE))
				LogError(GetResString(IDS_ERR_MAKEBAKCREDITFILE));

			// reopen file
			CFileException fexp;
			if (!file.Open(strFileName, iOpenFlags, &fexp)){
				CString strError(GetResString(IDS_ERR_LOADCREDITFILE));
				TCHAR szError[MAX_CFEXP_ERRORMSG];
				if (fexp.GetErrorMessage(szError, ARRSIZE(szError))){
					strError += _T(" - ");
					strError += szError;
				}
				LogError(LOG_STATUSBAR, _T("%s"), strError);
				return;
			}
			setvbuf(file.m_pStream, NULL, _IOFBF, 16384);
			file.Seek(1, CFile::begin); //set filepointer behind file version byte
		}
		*/
		file.Seek(1, CFile::begin); //set filepointer behind file version byte

		UINT count = file.ReadUInt32();

		const time_t dwExpired = time(NULL) - 12960000; // today - 150 day
		uint32_t cDeleted = 0;
		for (UINT i = 0; i < count; i++){
			CreditStruct* newcstruct = new CreditStruct;
			memset(newcstruct, 0, sizeof(CreditStruct));
			if (version == CREDITFILE_VERSION_29)
				file.Read(newcstruct, sizeof(CreditStruct_29a));
			else
				file.Read(newcstruct, sizeof(CreditStruct));
			
			if (newcstruct->nLastSeen < dwExpired){
				cDeleted++;
				delete newcstruct;
				continue;
			}
			// ismod: file error protection
			if (newcstruct->nKeySize > 76)
			{
				cDeleted++;
				delete newcstruct;
				continue;
			}
			CClientCredits* newcredits = new CClientCredits(newcstruct);
			m_mapClients.SetAt(CCKey(newcredits->GetKey()), newcredits);
		}
		file.Close();

		if (cDeleted>0)
			AddLogLine(false, GetResString(IDS_CREDITFILELOADED) + GetResString(IDS_CREDITSEXPIRED), count-cDeleted,cDeleted);
		else
			AddLogLine(false, GetResString(IDS_CREDITFILELOADED), count);
		// ismod: credits loaded!
		bCreditsLoaded = true;
	}
	catch(CFileException* error){
		if (error->m_cause == CFileException::endOfFile)
			LogError(LOG_STATUSBAR, GetResString(IDS_CREDITFILECORRUPT));
		else{
			TCHAR buffer[MAX_CFEXP_ERRORMSG];
			error->GetErrorMessage(buffer, _countof(buffer));
			LogError(LOG_STATUSBAR, GetResString(IDS_ERR_CREDITFILEREAD), buffer);
		}
		// ismod
		file.Close();
		error->Delete();
	}
	// ismod: create backup
	if (bCreditsLoaded)
	{
		// everything is ok, lets see if the backup exist...
		CString strBakFileName;
		strBakFileName.Format(_T("%s") CLIENTS_MET_FILENAME _T(".bak"), thePrefs.GetMuleDirectory(EMULE_CONFIGDIR));
		
		if (!::CopyFile(strFileName, strBakFileName, FALSE))
		{
			LogError(GetResString(IDS_ERR_MAKEBAKCREDITFILE));
		}
		return true;
	}
	
	return false;
}

void CClientCreditsList::SaveList()
{
	if (thePrefs.GetLogFileSaving())
		AddDebugLogLine(false, _T("Saving clients credit list file \"%s\""), CLIENTS_MET_FILENAME);
	m_nLastSaved = ::GetTickCount();

	CString name = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + CLIENTS_MET_FILENAME;
	CFile file;// no buffering needed here since we swap out the entire array
	CFileException fexp;
	if (!file.Open(name, CFile::modeWrite|CFile::modeCreate|CFile::typeBinary|CFile::shareDenyWrite, &fexp)){
		CString strError(GetResString(IDS_ERR_FAILED_CREDITSAVE));
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		if (fexp.GetErrorMessage(szError, _countof(szError))){
			strError += _T(" - ");
			strError += szError;
		}
		LogError(LOG_STATUSBAR, _T("%s"), strError);
		return;
	}

	uint32_t count = (uint32_t) m_mapClients.GetCount();
	BYTE* pBuffer = new BYTE[count*sizeof(CreditStruct)];
	CClientCredits* cur_credit;
	CCKey tempkey(0);
	POSITION pos = m_mapClients.GetHeadPosition();
	count = 0;
	while (pos)
	{
		m_mapClients.GetNextAssoc(pos, tempkey, cur_credit);
		if (cur_credit->GetUploadedTotal() || cur_credit->GetDownloadedTotal())
		{
			memcpy(pBuffer+(count*sizeof(CreditStruct)), cur_credit->GetDataStruct(), sizeof(CreditStruct));
			count++; 
		}
	}

	try{
		uint8_t version = CREDITFILE_VERSION;
		file.Write(&version, 1);
		file.Write(&count, 4);
		file.Write(pBuffer, count*sizeof(CreditStruct));
		if (thePrefs.GetCommitFiles() >= 2 || (thePrefs.GetCommitFiles() >= 1 && !theApp.IsRunning()))
			file.Flush();
		file.Close();
	}
	catch(CFileException* error){
		CString strError(GetResString(IDS_ERR_FAILED_CREDITSAVE));
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		if (error->GetErrorMessage(szError, _countof(szError))){
			strError += _T(" - ");
			strError += szError;
		}
		LogError(LOG_STATUSBAR, _T("%s"), strError);
		error->Delete();
	}

	delete[] pBuffer;
}

CClientCredits* CClientCreditsList::GetCredit(const uchar* key)
{
	CClientCredits* result;
	CCKey tkey(key);
	if (!m_mapClients.Lookup(tkey, result)){
		result = new CClientCredits(key);
		m_mapClients.SetAt(CCKey(result->GetKey()), result);
	}
	// BEGIN netfinity: Known Client Bonus
	else if (time(NULL) - result->m_pCredits->nLastSeen > HR2S(2))
	{
		// Give 1 minute in bonus on waiting time for every 4 hours away,
		// and with a maximum of 30 minutes after 5 days
		result->m_dwWaitingTimeBonus = min<time_t>(MIN2MS(30), ((time(NULL) - result->m_pCredits->nLastSeen) / HR2S(4)) * MIN2MS(1));
	}
	// END netfinity: Known Client Bonus
	result->SetLastSeen();
	return result;
}

void CClientCreditsList::Process()
{
	if (::GetTickCount() - m_nLastSaved > MIN2MS(13))
		SaveList();
}

void CClientCredits::InitalizeIdent()
{
	if (m_pCredits->nKeySize == 0 ){
		memset(m_abyPublicKey,0,80); // for debugging
		m_nPublicKeyLen = 0;
		IdentState = IS_NOTAVAILABLE;
	}
	else{
		m_nPublicKeyLen = m_pCredits->nKeySize;
		memcpy(m_abyPublicKey, m_pCredits->abySecureIdent, m_nPublicKeyLen);
		IdentState = IS_IDNEEDED;
	}
	m_dwCryptRndChallengeFor = 0;
	m_dwCryptRndChallengeFrom = 0;
	m_dwIdentIP = 0;
}

void CClientCredits::Verified(uint32_t dwForIP)
{
	m_dwIdentIP = dwForIP;
	// client was verified, copy the keyto store him if not done already
	if (m_pCredits->nKeySize == 0){
		m_pCredits->nKeySize = m_nPublicKeyLen; 
		memcpy(m_pCredits->abySecureIdent, m_abyPublicKey, m_nPublicKeyLen);
		if (GetDownloadedTotal() > 0){
			// for security reason, we have to delete all prior credits here
			m_pCredits->nDownloadedHi = 0;
			m_pCredits->nDownloadedLo = 1;
			m_pCredits->nUploadedHi = 0;
			m_pCredits->nUploadedLo = 1; // in order to safe this client, set 1 byte
			if (thePrefs.GetVerbose())
				DEBUG_ONLY(AddDebugLogLine(false, _T("Credits deleted due to new SecureIdent")));
		}
	}
	IdentState = IS_IDENTIFIED;
}

bool CClientCredits::SetSecureIdent(const uchar* pachIdent, uint8_t nIdentLen)  // verified Public key cannot change, use only if there is not public key yet
{
	if (MAXPUBKEYSIZE < nIdentLen || m_pCredits->nKeySize != 0 )
		return false;
	memcpy(m_abyPublicKey,pachIdent, nIdentLen);
	m_nPublicKeyLen = nIdentLen;
	IdentState = IS_IDNEEDED;
	return true;
}

EIdentState	CClientCredits::GetCurrentIdentState(uint32_t dwForIP) const
{
	if (IdentState != IS_IDENTIFIED)
		return IdentState;
	else{
		if (dwForIP == m_dwIdentIP)
			return IS_IDENTIFIED;
		else
			return IS_IDBADGUY; 
			// mod note: clients which just reconnected after an IP change and have to ident yet will also have this state for 1-2 seconds
			//		 so don't try to spam such clients with "bad guy" messages (besides: spam messages are always bad)
	}
}

using namespace CryptoPP;

void CClientCreditsList::InitalizeCrypting()
{
	m_nMyPublicKeyLen = 0;
#ifdef _DEBUG
	memset(m_abyMyPublicKey,0,80); // not really needed; better for debugging tho
#endif
	m_pSignkey = NULL;
	if (!thePrefs.IsSecureIdentEnabled())
		return;

	// ismod [start]
	RSA::PrivateKey privateKey;

	// cryptkey stored in registry
	uint32_t uPrivKeyBufLen;
	CReg reg(HKEY_CURRENT_USER, ISMOD_REGKEY);
	if ( (uPrivKeyBufLen = reg.GetLength(CRYPTKEY_REG_KEY)) == 0 )
	{
		AutoSeededRandomPool rng;
		privateKey.GenerateRandomWithKeySize(rng, RSAKEYSIZE);

		byte abyPrivKeyBuf[512];
		ArraySink privkeysink(abyPrivKeyBuf, _countof(abyPrivKeyBuf));
		privateKey.Save(privkeysink);
		uPrivKeyBufLen = (uint32_t)privkeysink.TotalPutLength();
		privkeysink.MessageEnd();

		reg.WriteBinary(CRYPTKEY_REG_KEY, abyPrivKeyBuf, uPrivKeyBufLen);

		if (thePrefs.GetLogSecureIdent())
			AddDebugLogLine(false, _T("Created new RSA keypair"));
	}else{
		byte *abyPrivKeyBuf = new byte[uPrivKeyBufLen];
		reg.GetBinary(CRYPTKEY_REG_KEY, abyPrivKeyBuf, uPrivKeyBufLen);

		StringStore privkeysink(abyPrivKeyBuf, uPrivKeyBufLen);
		privateKey.Load(privkeysink);
		delete[] abyPrivKeyBuf;
	}
		
	m_pSignkey = new RSASSA_PKCS1v15_SHA_Signer(privateKey);
	RSASSA_PKCS1v15_SHA_Verifier pubkey(*m_pSignkey);
	ArraySink asink(m_abyMyPublicKey, 80);
	pubkey.DEREncode(asink);
	m_nMyPublicKeyLen = (uint8_t)asink.TotalPutLength();
	asink.MessageEnd();

	ASSERT( Debug_CheckCrypting() );
}

bool CClientCreditsList::CreateKeyPair()
{

	return true;
}

uint8_t CClientCreditsList::CreateSignature(CClientCredits* pTarget, uchar* pachOutput, uint8_t nMaxSize, 
										  uint32_t ChallengeIP, uint8_t byChaIPKind, 
										  CryptoPP::RSASSA_PKCS1v15_SHA_Signer* sigkey)
{
	// sigkey param is used for debug only
	if (sigkey == NULL)
		sigkey = m_pSignkey;

	// create a signature of the public key from pTarget
	ASSERT( pTarget );
	ASSERT( pachOutput );
	uint8_t nResult;
	if ( !CryptoAvailable() )
		return 0;
	try{
		
		SecByteBlock sbbSignature(sigkey->SignatureLength());
		AutoSeededRandomPool rng;
		byte abyBuffer[MAXPUBKEYSIZE+9];
		uint32_t keylen = pTarget->GetSecIDKeyLen();
		memcpy(abyBuffer,pTarget->GetSecureIdent(),keylen);
		// 4 additional bytes random data send from this client
		uint32_t challenge = pTarget->m_dwCryptRndChallengeFrom;
		ASSERT ( challenge != 0 );
		PokeUInt32(abyBuffer+keylen, challenge);
		uint16_t ChIpLen = 0;
		if ( byChaIPKind != 0){
			ChIpLen = 5;
			PokeUInt32(abyBuffer+keylen+4, ChallengeIP);
			PokeUInt8(abyBuffer+keylen+4+4, byChaIPKind);
		}
		sigkey->SignMessage(rng, abyBuffer ,keylen+4+ChIpLen , sbbSignature.begin());
		ArraySink asink(pachOutput, nMaxSize);
		asink.Put(sbbSignature.begin(), sbbSignature.size());
		nResult = (uint8_t)asink.TotalPutLength();			
	}
	catch(...)
	{
		ASSERT ( false );
		nResult = 0;
	}
	return nResult;
}

bool CClientCreditsList::VerifyIdent(CClientCredits* pTarget, const uchar* pachSignature, uint8_t nInputSize, 
									 uint32_t dwForIP, uint8_t byChaIPKind)
{
	ASSERT( pTarget );
	ASSERT( pachSignature );
	if ( !CryptoAvailable() ){
		pTarget->IdentState = IS_NOTAVAILABLE;
		return false;
	}
	bool bResult;
	try{
		StringSource ss_Pubkey((byte*)pTarget->GetSecureIdent(),pTarget->GetSecIDKeyLen(),true,0);
		RSASSA_PKCS1v15_SHA_Verifier pubkey(ss_Pubkey);
		// 4 additional bytes random data send from this client +5 bytes v2
		byte abyBuffer[MAXPUBKEYSIZE+9];
		memcpy(abyBuffer,m_abyMyPublicKey,m_nMyPublicKeyLen);
		uint32_t challenge = pTarget->m_dwCryptRndChallengeFor;
		ASSERT ( challenge != 0 );
		PokeUInt32(abyBuffer+m_nMyPublicKeyLen, challenge);
		
		// v2 security improvments (not supported by 29b, not used as default by 29c)
		uint8_t nChIpSize = 0;
		if (byChaIPKind != 0){
			nChIpSize = 5;
			uint32_t ChallengeIP = 0;
			switch (byChaIPKind){
				case CRYPT_CIP_LOCALCLIENT:
					ChallengeIP = dwForIP;
					break;
				case CRYPT_CIP_REMOTECLIENT:
					if (theApp.serverconnect->GetClientID() == 0 || theApp.serverconnect->IsLowID()){
						if (thePrefs.GetLogSecureIdent())
							AddDebugLogLine(false, _T("Warning: Maybe SecureHash Ident fails because LocalIP is unknown"));
						ChallengeIP = theApp.serverconnect->GetLocalIP();
					}
					else
						ChallengeIP = theApp.serverconnect->GetClientID();
					break;
				case CRYPT_CIP_NONECLIENT: // maybe not supported in future versions
					ChallengeIP = 0;
					break;
			}
			PokeUInt32(abyBuffer+m_nMyPublicKeyLen+4, ChallengeIP);
			PokeUInt8(abyBuffer+m_nMyPublicKeyLen+4+4, byChaIPKind);
		}
		//v2 end

		bResult = pubkey.VerifyMessage(abyBuffer, m_nMyPublicKeyLen+4+nChIpSize, pachSignature, nInputSize);
	}
	catch(...)
	{
		if (thePrefs.GetVerbose())
			AddDebugLogLine(false, _T("Error: Unknown exception in %hs"), __FUNCTION__);
		//ASSERT(0);
		bResult = false;
	}
	if (!bResult){
		if (pTarget->IdentState == IS_IDNEEDED)
			pTarget->IdentState = IS_IDFAILED;
	}
	else{
		pTarget->Verified(dwForIP);
	}
	return bResult;
}

bool CClientCreditsList::CryptoAvailable()
{
	return (m_nMyPublicKeyLen > 0 && m_pSignkey != 0 && thePrefs.IsSecureIdentEnabled() );
}


#ifdef _DEBUG
bool CClientCreditsList::Debug_CheckCrypting()
{
	// create random key
	AutoSeededRandomPool rng;

	RSASSA_PKCS1v15_SHA_Signer priv(rng, 384);
	RSASSA_PKCS1v15_SHA_Verifier pub(priv);

	byte abyPublicKey[80];
	ArraySink asink(abyPublicKey, 80);
	pub.DEREncode(asink);
	uint8_t PublicKeyLen = (uint8_t)asink.TotalPutLength();
	asink.MessageEnd();
	uint32_t challenge = rand();
	// create fake client which pretends to be this emule
	CreditStruct* newcstruct = new CreditStruct;
	memset(newcstruct, 0, sizeof(CreditStruct));
	CClientCredits* newcredits = new CClientCredits(newcstruct);
	newcredits->SetSecureIdent(m_abyMyPublicKey,m_nMyPublicKeyLen);
	newcredits->m_dwCryptRndChallengeFrom = challenge;
	// create signature with fake priv key
	uchar pachSignature[200];
	memset(pachSignature,0,200);
	uint8_t sigsize = CreateSignature(newcredits,pachSignature,200,0,false, &priv);


	// next fake client uses the random created public key
	CreditStruct* newcstruct2 = new CreditStruct;
	memset(newcstruct2, 0, sizeof(CreditStruct));
	CClientCredits* newcredits2 = new CClientCredits(newcstruct2);
	newcredits2->m_dwCryptRndChallengeFor = challenge;

	// if you uncomment one of the following lines the check has to fail
	//abyPublicKey[5] = 34;
	//m_abyMyPublicKey[5] = 22;
	//pachSignature[5] = 232;

	newcredits2->SetSecureIdent(abyPublicKey,PublicKeyLen);

	//now verify this signature - if it's true everything is fine
	bool bResult = VerifyIdent(newcredits2,pachSignature,sigsize,0,0);

	delete newcredits;
	delete newcredits2;

	return bResult;
}
#endif
time_t CClientCredits::GetSecureWaitStartTime(uint32_t dwForIP)
{
	if (m_dwUnSecureWaitTime == 0 || m_dwSecureWaitTime == 0)
		SetSecWaitStartTime(dwForIP);

	if (m_pCredits->nKeySize != 0){	// this client is a SecureHash Client
		if (GetCurrentIdentState(dwForIP) == IS_IDENTIFIED){ // good boy
			return m_dwSecureWaitTime - m_dwWaitingTimeBonus; // netfinity: Known Client Bonus - Only give bonus to identified clients, for now!
		}
		else{	// not so good boy
			if (dwForIP == m_dwWaitTimeIP){
				return m_dwUnSecureWaitTime;
			}
			else{	// bad boy
				// this can also happen if the client has not identified himself yet, but will do later - so maybe he is not a bad boy :) .
				CString buffer2, buffer;
				/*for (uint16 i = 0;i != 16;i++){
					buffer2.Format("%02X",this->m_pCredits->abyKey[i]);
					buffer+=buffer2;
				}
				if (thePrefs.GetLogSecureIdent())
					AddDebugLogLine(false,"Warning: WaitTime resetted due to Invalid Ident for Userhash %s", buffer);*/
				
				m_dwUnSecureWaitTime = ::GetTickCount();
				m_dwWaitTimeIP = dwForIP;
				return m_dwUnSecureWaitTime;
			}	
		}
	}
	else{	// not a SecureHash Client - handle it like before for now (no security checks)
		return m_dwUnSecureWaitTime;
	}
}

void CClientCredits::SetSecWaitStartTime(uint32_t dwForIP)
{
	m_dwUnSecureWaitTime = ::GetTickCount()-1;
	m_dwSecureWaitTime = ::GetTickCount()-1;
	m_dwWaitTimeIP = dwForIP;
}

void CClientCredits::ClearWaitStartTime()
{
	m_dwUnSecureWaitTime = 0;
	m_dwSecureWaitTime = 0;
}
