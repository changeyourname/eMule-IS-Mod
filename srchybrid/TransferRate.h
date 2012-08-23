/** @file		TransferRate.h
 *  @brief	Accurate average data rate calculation
 *  @author	netfinity
 */
#pragma once

/**
 *  @brief	Template class for calculating flow/data rates
 */
class CTransferRate
{
private:
	mutable uint64_t				m_bytesPendingMeasure;
	mutable double				m_fSumSamples;
	mutable unsigned int 		m_nSamples;
	mutable uint32_t				m_tickLastRecalc;
	mutable uint32_t	 			m_msSamplePeriod;
	mutable uint32_t				m_rateLastEstAverage;
	struct Sample
	{
		double			m_fSample;
		uint32_t			m_tickSample;
	};
	mutable CList<Sample*>		m_listSamples;
public:
	CTransferRate(const unsigned int Samples = 20, const uint32_t SamplePeriod = 500)
	{
		m_tickLastRecalc = ::GetTickCount();
		m_msSamplePeriod = SamplePeriod;
		m_nSamples = Samples;
		m_bytesPendingMeasure = 0;
		m_rateLastEstAverage = 0;
		m_fSumSamples = 0;
	}
	~CTransferRate()
	{
		while (m_listSamples.GetCount() > 0)
		{
			Sample*	pSample = m_listSamples.RemoveHead();
			delete pSample;
		}
	}
	// Estimate the current rate using calculated values
	uint32_t GetCurrentRate() const
	{
		// Update average if more than one sample period elapsed since last update
		UpdateTransferRate();
		return m_rateLastEstAverage;
	}
	// String conversion 
	//CString ToString() const {return GetCurrentRate().ToString();}
	// Type cast and arithmetic operators
	operator uint32_t () const {return GetCurrentRate();}
//	bool operator ==(const uint32 value) const {return GetCurrentRate() == value;}
//	bool operator !=(const uint32 value) const {return GetCurrentRate() != value;}
//	bool operator >=(const uint32 value) const {return GetCurrentRate() >= value;}
//	bool operator <=(const uint32 value) const {return GetCurrentRate() <= value;}
//	bool operator >(const uint32 value) const {return GetCurrentRate() > value;}
//	bool operator <(const uint32 value) const {return GetCurrentRate() < value;}
//	uint32 operator +(const uint32 value) const {return GetCurrentRate() + value;}
//	uint32 operator -(const uint32 value) const {return GetCurrentRate() - value;}
	// Set calculation parameters
	void SetSamples(const unsigned int Samples) {m_nSamples = Samples; Reset();}
	void SetSamplePeriod(const uint32_t SamplePeriod) {m_msSamplePeriod = SamplePeriod; Reset();}
	// Reset rate calculation
	void Reset()
	{
		m_tickLastRecalc = ::GetTickCount();
		m_bytesPendingMeasure = 0;
		m_rateLastEstAverage = 0;
		m_fSumSamples = 0;
		while (m_listSamples.GetCount() > 0)
		{
			Sample*	pSample = m_listSamples.RemoveHead();
			delete pSample;
		}
	}
	// Update rate calculation with new measure
	void AddTransferred(const uint64_t Measure)
	{
		m_bytesPendingMeasure += Measure;
		UpdateTransferRate();
	}
private:
	// Update rate calculation (make the update check inline)
	void UpdateTransferRate() const
	{
		uint32_t	msElapsedSinceLastRecalc = ::GetTickCount() - m_tickLastRecalc;
		if (msElapsedSinceLastRecalc >= m_msSamplePeriod)
			UpdateTransferRate_Perform(msElapsedSinceLastRecalc);
	}
	// Update rate calculation 
	void UpdateTransferRate_Perform(uint32_t	msElapsedSinceLastRecalc) const
	{
		double	N, secSamplePeriod = (double) m_msSamplePeriod / 1000.0f;
		unsigned int i;
		// Add new sample(s) to tail of sample list
		double			fCurTransferRate = (double) m_bytesPendingMeasure * 1000.0f / (double) msElapsedSinceLastRecalc;
		unsigned int	nSamplesToAdd = msElapsedSinceLastRecalc / m_msSamplePeriod;
		if (nSamplesToAdd > m_nSamples)
		{
			m_tickLastRecalc = m_tickLastRecalc + m_msSamplePeriod * (nSamplesToAdd - m_nSamples);
			nSamplesToAdd = m_nSamples;
		}
		m_bytesPendingMeasure -= (uint64_t) (fCurTransferRate * nSamplesToAdd * secSamplePeriod);
		for (i = 0; i < min(nSamplesToAdd, m_nSamples); ++i)
		{
			Sample*	pNewSample = new Sample();
			pNewSample->m_fSample = fCurTransferRate ;
			pNewSample->m_tickSample = m_tickLastRecalc + m_msSamplePeriod;
			m_listSamples.AddHead(pNewSample);
			m_tickLastRecalc = pNewSample->m_tickSample;
			m_fSumSamples += fCurTransferRate;
		}
		// Trim sample list
		while ((unsigned int) m_listSamples.GetCount() > m_nSamples)
		{
			Sample* pSample = m_listSamples.RemoveTail();
			m_fSumSamples -= pSample->m_fSample;
			delete pSample;
		}
		// Calculate average
		N = static_cast<double>(m_listSamples.GetCount());
		if (N >= 1)
		{
			m_rateLastEstAverage = (uint32_t) (m_fSumSamples / N);
		}
		else
			m_rateLastEstAverage = 0;
	}
};
