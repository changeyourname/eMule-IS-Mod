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

#include "ThrottledSocket.h" // ZZ:UploadBandWithThrottler (UDP)
#include "TransferRate.h" // netfinity: Transfer Rate Blackbox

class UploadBandwidthThrottler : public CSafeThread 
{
public:
    UploadBandwidthThrottler(void);
    ~UploadBandwidthThrottler(void);

    uint64_t GetNumberOfSentBytesSinceLastCallAndReset();
    uint64_t GetNumberOfSentBytesOverheadSinceLastCallAndReset();
    uint32_t GetHighestNumberOfFullyActivatedSlotsSinceLastCallAndReset();

    uint32_t GetStandardListSize() { return (uint32_t) m_StandardOrder_list.GetSize(); };

    void AddToStandardList(uint32_t index, ThrottledFileSocket* socket);
    bool RemoveFromStandardList(ThrottledFileSocket* socket);

    void QueueForSendingControlPacket(ThrottledControlSocket* socket, bool hasSent = false); // ZZ:UploadBandWithThrottler (UDP)
    void RemoveFromAllQueues(ThrottledControlSocket* socket) { RemoveFromAllQueues(socket, true); }; // ZZ:UploadBandWithThrottler (UDP)
    void RemoveFromAllQueues(ThrottledFileSocket* socket);

    void EndThread();

    void Pause(bool paused);
    static uint32_t UploadBandwidthThrottler::GetSlotLimit(uint32_t currentUpSpeed);
private:
    static UINT RunProc(LPVOID pParam);
    UINT RunInternal();

    void RemoveFromAllQueues(ThrottledControlSocket* socket, bool lock); // ZZ:UploadBandWithThrottler (UDP)
    bool RemoveFromStandardListNoLock(ThrottledFileSocket* socket);

    uint32_t CalculateChangeDelta(uint32_t numberOfConsecutiveChanges) const;

    CTypedPtrList<CPtrList, ThrottledControlSocket*> m_ControlQueue_list; // a queue for all the sockets that want to have Send() called on them. // ZZ:UploadBandWithThrottler (UDP)
    CTypedPtrList<CPtrList, ThrottledControlSocket*> m_ControlQueueFirst_list; // a queue for all the sockets that want to have Send() called on them. // ZZ:UploadBandWithThrottler (UDP)
    CTypedPtrList<CPtrList, ThrottledControlSocket*> m_TempControlQueue_list; // sockets that wants to enter m_ControlQueue_list // ZZ:UploadBandWithThrottler (UDP)
    CTypedPtrList<CPtrList, ThrottledControlSocket*> m_TempControlQueueFirst_list; // sockets that wants to enter m_ControlQueue_list and has been able to send before // ZZ:UploadBandWithThrottler (UDP)

    CArray<ThrottledFileSocket*, ThrottledFileSocket*> m_StandardOrder_list; // sockets that have upload slots. Ordered so the most prioritized socket is first

    CCriticalSection sendLocker;
    CCriticalSection tempQueueLocker;
	CCriticalSection statsLocker; // netfinity: Separate locker for statistical data to improve performance

    CEvent* threadEndedEvent;
    CEvent* pauseEvent;

    uint64_t m_SentBytesSinceLastCall;
    uint64_t m_SentBytesSinceLastCallOverhead;
    uint32_t m_highestNumberOfFullyActivatedSlots;

    bool doRun;

	// BEGIN netfinity: Upload Throttling Slot Management
public:
	bool			IsMoreSlotsNeeded(void) {return m_bMoreSlotsNeeded;}
	bool			IsMoreSlotsRequired(void) {return m_bMoreSlotsRequired;}
	uint32_t			BandwidthUsed(void) {CSingleLock singleLock(&statsLocker, TRUE); return m_rateUsed;}
	uint32_t			BandwidthToReplenish(void) {CSingleLock singleLock(&statsLocker, TRUE); return m_rateNeededToReplace;}
	uint32_t			BandwidthToFill(void) {CSingleLock singleLock(&statsLocker, TRUE); return m_rateRequiredToFill;}
private:
	bool			m_bMoreSlotsNeeded;
	bool			m_bMoreSlotsRequired;
	CTransferRate	m_rateUsed;
	CTransferRate	m_rateNeededToReplace;
	uint32_t			m_rateRequiredToFill;
	// END netfinity: Upload Throttling Slot Management
};
