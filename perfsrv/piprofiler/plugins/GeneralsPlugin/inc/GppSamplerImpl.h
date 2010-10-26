/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies). 
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  
*
*/


#ifndef PROFILER_GPP_SAMPLER_H
#define PROFILER_GPP_SAMPLER_H

#include "GeneralsConfig.h"

#include <e32cmn.h>

#include <piprofiler/ProfilerGenericClassesKrn.h>
#include <piprofiler/ProfilerTraces.h>

// for RPropertyRef
#include <sproperty.h> 
#include <e32cmn.h>

/*
 *	
 *	GPP sampler definition
 *	
 */
class DGppSamplerImpl //: public DBase
{
public:

	DGppSamplerImpl();
	~DGppSamplerImpl();

	TUint8* EncodeTag(TUint8* aPtr);
	TUint8* EncodeInt(TUint8* aPtr,TInt aValue);
	TUint8* EncodeUint(TUint8* aPtr,TUint aValue);
	TUint8* EncodeText(TUint8* aPtr, const TDesC& aDes);

	TUint8* EncodeName(TUint8* aPtr, DObject& aObject,TUint32 id);
	TUint8* EncodeThread(TUint8* aPtr, DThread& aThread);

	TBool	IsaTaskKnown(TUint8 task);
	TUint8* EncodeIsaTask(TUint8* aPtr, TUint task);
	TUint8* EncodeIsaName(TUint8* aPtr, TUint task,TBool process);
	
	TUint8* EncodeRepeat(TUint8* aPtr);

	TInt	CreateFirstSample();
	TInt	SampleImpl();
#ifdef __SMP__
    TSpinLock* LockPtr();
#endif
    TInt    iGppSamplingPeriod;
	TUint8	tempBuf[512];
	TUint	iLastPc;
	
	TBool	iIsaSample;
	TInt	knownIsaTasks[256];
	TUint8	knownIsaTaskCount;
	TInt*	isaOsTaskRunningAddr;
	void	Reset();
    TUint32 iSampleCount;
#ifdef __SMP__
    TInt        iCpuNumber;
    TUint32     iStartTime;
#endif	   
	PROFILER_DEFINE_ISA_TASK_NAME_ARRAY

private:
	enum TState {EStop,ERunning,EStopping};

	const TUint* iInterruptStack;
	
	TUint		iLastThread;
	TUint		iRepeat;
#ifndef __SMP__
	TUint32 	iStartTime;
#endif
	RPropertyRef iIsaStartAddr;
	RPropertyRef iIsaEndAddr;
	RPropertyRef iIsaPluginStatus;
	RPropertyRef iIsaOsTaskRunning;
	TInt 		iIsaStatus;
	TInt 		iIsaStart;
	TInt 		iIsaEnd;
	TUint32     iMask;
	TUint32     iCpuSelector;
	TInt        iPrevTS;   // previous sample's timestamp
};

struct TProfilerGppSamplerData
{
//public:
	TUint32		lastPcValue;
	TUint32		sampleNumber;
    TInt        samplingPeriod;
};

template <int BufferSize>
class DProfilerGppSampler : public DProfilerGenericSampler<BufferSize>
{
public:
#ifndef __SMP__
	DProfilerGppSampler();
#else
    DProfilerGppSampler(TInt aCpuNumber);
#endif
	~DProfilerGppSampler();

	struct TProfilerGppSamplerData* GetExportData();
	void	Sample(TAny* aPtr);
	TInt	Reset(DProfilerSampleStream* aStream, TUint32 aSyncOffset);
	TInt 	GetPeriod();
	
private:
	DGppSamplerImpl iGppSamplerImpl;
	struct TProfilerGppSamplerData iExportData;
#ifdef __SMP__
	TInt   iCpuNumber;
#endif
};

/*
 *	
 *	GPP sampler implementation
 *	
 */
#ifndef __SMP__
template <int BufferSize>
DProfilerGppSampler<BufferSize>::DProfilerGppSampler() :
	DProfilerGenericSampler<BufferSize>(PROFILER_GPP_SAMPLER_ID) 
    {
	LOGSTRING2("DProfilerGppSampler<%d>::CProfilerGppSampler",BufferSize);
    }
#else
template <int BufferSize>
DProfilerGppSampler<BufferSize>::DProfilerGppSampler(TInt aCpuNumber) :
    DProfilerGenericSampler<BufferSize>(PROFILER_GPP_SAMPLER_ID+(aCpuNumber*20)), iCpuNumber(aCpuNumber) 
    {
    LOGSTRING3("DProfilerGppSampler<%d>::DProfilerGppSampler, cpu %d",BufferSize, iCpuNumber);
    }
#endif

template <int BufferSize>
DProfilerGppSampler<BufferSize>::~DProfilerGppSampler()
    {
	LOGSTRING2("DProfilerGppSampler<%d>::~CProfilerGppSampler",BufferSize);		
    }

template <int BufferSize>
TInt DProfilerGppSampler<BufferSize>::Reset(DProfilerSampleStream* aStream, TUint32 aSyncOffset)
        {
	LOGSTRING2("DProfilerGppSampler<%d>::Reset - calling superclass reset",BufferSize);
	DProfilerGenericSampler<BufferSize>::Reset(aStream, 0);
	LOGSTRING2("DProfilerGppSampler<%d>::Reset - called superclass reset",BufferSize);

	iGppSamplerImpl.Reset();
	
#ifdef __SMP__
	iGppSamplerImpl.iCpuNumber = iCpuNumber;
	
	// set common start time for all CPU samplers
	iGppSamplerImpl.iStartTime = aSyncOffset;
#endif
	iGppSamplerImpl.iGppSamplingPeriod = DProfilerGenericSampler<BufferSize>::iSamplingPeriod;
	iGppSamplerImpl.iSampleCount = 0;
	iExportData.sampleNumber = 0;
	iExportData.lastPcValue = 0;
    iExportData.samplingPeriod = iGppSamplerImpl.iGppSamplingPeriod;

	TInt length = iGppSamplerImpl.CreateFirstSample();

	LOGSTRING3("DProfilerGPPSampler<%d>::Reset - got first sample, size %d",BufferSize,length);
	
	//DProfilerGenericSampler<BufferSize>::iSampleBuffer->AddSample(iGppSamplerImpl.tempBuf,length);
	this->iSampleBuffer->AddSample(iGppSamplerImpl.tempBuf,length);

	// check if sampling period > 1 ms
	// NOTE: feature supported in Performance Investigator 2.01 and above
	if(iGppSamplerImpl.iGppSamplingPeriod > 1)
	    {
        // For Address/Thread (GPP) version 2.01 or above, the first word is the sampling period in milliseconds
        TUint8* w(iGppSamplerImpl.tempBuf);
        
        (*w++) = (iGppSamplerImpl.iGppSamplingPeriod >> 24) & 0xFF;
        (*w++) = (iGppSamplerImpl.iGppSamplingPeriod >> 16) & 0xFF;
        (*w++) = (iGppSamplerImpl.iGppSamplingPeriod >>  8) & 0xFF;
        (*w++) = (iGppSamplerImpl.iGppSamplingPeriod) & 0xFF;
        
        //DProfilerGenericSampler<BufferSize>::iSampleBuffer->AddSample(iGppSamplerImpl.tempBuf,4);
        this->iSampleBuffer->AddSample(iGppSamplerImpl.tempBuf,4);
	    }
	
	LOGSTRING2("DProfilerGPPSampler<%d>::Reset finished",BufferSize);
	return KErrNone;
    }

template <int BufferSize>
void DProfilerGppSampler<BufferSize>::Sample(TAny* aPtr)
    {
    LOGSTRING2("DProfilerGppSampler<%d>::Sample",BufferSize);

	TInt length(iGppSamplerImpl.SampleImpl());

    iGppSamplerImpl.iSampleCount++;
	iExportData.sampleNumber += iGppSamplerImpl.iGppSamplingPeriod;
	iExportData.lastPcValue = iGppSamplerImpl.iLastPc;

	if(length > 0)
        {
	    TInt ret(this->iSampleBuffer->AddSample(iGppSamplerImpl.tempBuf,length));
        if (ret != 0)
            {
            Kern::Printf(("DProfilerGppSampler<%d>::Sample() - add to sample buffer failed, loosing data, error = %d"),BufferSize,ret);
            }
        }
	LOGSTRING3("DProfilerGppSampler<%d>::Sample - length %d",BufferSize,length);
	return;
    }


template <int BufferSize>
struct TProfilerGppSamplerData* DProfilerGppSampler<BufferSize>::GetExportData()
    {
	LOGSTRING2("DProfilerGppSampler<%d>::GetExportData",BufferSize);
	return &(iExportData);	
    }

template <int BufferSize>
TInt DProfilerGppSampler<BufferSize>::GetPeriod()
    {
	return iGppSamplerImpl.iGppSamplingPeriod;
    }

#endif
