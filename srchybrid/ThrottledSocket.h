// ZZ:UploadBandWithThrottler (UDP) -->

#pragma once

struct SocketSentBytes {
    bool    success;
	uint32_t	sentBytesStandardPackets;
	uint32_t	sentBytesControlPackets;
};

class ThrottledControlSocket
{
public:
    virtual SocketSentBytes SendControlData(uint32_t maxNumberOfBytesToSend, uint32_t minFragSize) = 0;
};

class ThrottledFileSocket : public ThrottledControlSocket
{
public:
    virtual SocketSentBytes SendFileAndControlData(uint32_t maxNumberOfBytesToSend, uint32_t minFragSize) = 0;
    virtual DWORD GetLastCalledSend() = 0;
    virtual uint32_t	GetNeededBytes() = 0;
	virtual bool	IsBusy() const = 0;
    virtual bool    HasQueues() const = 0;
	virtual bool	UseBigSendBuffer()								{ return false; }
};

// <-- ZZ:UploadBandWithThrottler (UDP)
