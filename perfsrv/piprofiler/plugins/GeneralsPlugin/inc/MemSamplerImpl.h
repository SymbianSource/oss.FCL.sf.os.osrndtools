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


#ifndef PROFILER_MEM_SAMPLER_H
#define PROFILER_MEM_SAMPLER_H

#include "GeneralsConfig.h"

#include <kern_priv.h>

#include <piprofiler/ProfilerGenericClassesKrn.h>
#include <piprofiler/ProfilerTraces.h>
#include "GppSamplerImpl.h"
#include "MemoryEventHandler.h"
#include <e32btrace.h>


// constants
// defines the maximum thread amount that is assumed to
// be possible with MEM trace
const TInt KSampleBufferSize = 257;
const TInt KProfilerMaxThreadsAmount = 512;
const TInt KProfilerMaxChunksAmount = 1024;
const TInt KProfilerMaxLibrariesAmount = 1024;
const TInt KProfilerTotalMemorySamplePeriod = 100;

// flags
//#ifndef __SMP__
#define MEM_EVENT_HANDLER
//#define MEM_EVENT_HANDLER_LIBRARY_EVENTS
//#endif

/*
 *	
 *	MEM sampler definition
 *	
 */

class DMemoryEventHandler;

class DMemSamplerImpl //: public DBase
{
public:
	enum EProcessingState
	{
		EStartingToProcess,
		EProcessingNames,
		EProcessingData,
		ENothingToProcess
	};

	enum ESampleType
	{
	    ESampleChunks,
	    ESampleThreads,
	    ESampleLibraries
	};

	DMemSamplerImpl();
	~DMemSamplerImpl();

	TInt	CreateFirstSample();
	TInt	SampleImpl(TUint32 sampleNum);
	TBool	SampleNeeded(TUint32 sampleNum);
	void	Reset();
	TInt	ProcessChunks();
	TInt    ProcessThreads();
    TInt    GatherChunks();
    TInt    GatherThreads();

#ifdef MEM_EVENT_HANDLER_LIBRARY_EVENTS
    TInt    GatherLibraries();
    TInt    ProcessLibraries();
#endif
    
	TInt	EncodeChunkData(DChunk& chunk);
	TInt	EncodeChunkName(DChunk& chunk);
	TInt	EncodeChunkData(DThread& thread);
	TInt	EncodeChunkName(DThread& thread);
#ifdef MEM_EVENT_HANDLER_LIBRARY_EVENTS
	TInt    EncodeChunkData(DLibrary& library);
	TInt    EncodeChunkName(DLibrary& library);
#endif

	TInt 	EncodeTotalMemoryName();
	TInt 	EncodeTotalMemory();
	
	TInt	EncodeNameCode();
	TInt	EncodeDataCode();

	DChunk*		heapChunksToSample[KProfilerMaxChunksAmount];
	DChunk*		heapChunkNamesToReport[KProfilerMaxChunksAmount];
	TUint32		iCount;
	TInt		iChunkCount;
	TInt		iNewChunkCount;
	TBuf8<0x50> name;
	DThread*	threadsToSample[KProfilerMaxThreadsAmount];
	DThread*	threadNamesToReport[KProfilerMaxThreadsAmount];
	TInt		iThreadCount;
	TInt		iNewThreadCount;
	
	TInt        iHandledThreads;
	TInt        iHandledChunks;
	TInt        iHandledLibs;
	
#ifdef MEM_EVENT_HANDLER_LIBRARY_EVENTS
	DLibrary*   librariesToSample[KProfilerMaxLibrariesAmount];
	DLibrary*   libraryNamesToReport[KProfilerMaxLibrariesAmount];
	TInt        iLibraryCount;
	TInt        iNewLibraryCount;
	TInt        iLibrariesProcessing;
#endif

	TInt		iChunksProcessing;
    TInt        iThreadsProcessing;
	TInt		iMemSamplingPeriod;
	//TInt		iMemSamplingPeriodDiv2;
	//TInt        iMemSamplingPeriodDiv3;
	

	ESampleType iSampleType;

	TBool       iTimeToSample;
	TBool 		iTotalMemoryOk;
	TBool		iTotalMemoryNameOk;

	TUint8		sample[KSampleBufferSize];
	TPtr8		sampleDescriptor;
	TInt        iThreadsHandled;
	TInt        iChunksHandled;
#ifdef MEM_EVENT_HANDLER
	TBool      iChunksGathered;
	TBool      iThreadsGathered;
#ifdef MEM_EVENT_HANDLER_LIBRARY_EVENTS
	TBool      iLibrariesGathered;
#endif
#endif
};


template <int BufferSize>
class DProfilerMemSampler : public DProfilerGenericSampler<BufferSize>
{
public:
	DProfilerMemSampler(struct TProfilerGppSamplerData*, TInt id);
	~DProfilerMemSampler();

	void	Sample(TAny* aPtr);
	TInt	Reset(DProfilerSampleStream* aStream, TUint32 aSyncOffset);
	TInt	PostSample();
	TBool	PostSampleNeeded();
	
private:
#ifdef MEM_EVENT_HANDLER
    DMemoryEventHandler*               iEventHandler;
#endif
	DMemSamplerImpl			           memSamplerImpl;
	struct TProfilerGppSamplerData*    gppSamplerData;
	TBool                              sampleNeeded;
    TUint32                         iSyncOffset;
};

/*
 *	
 *	MEM sampler implementation
 *	
 */

template <int BufferSize>
DProfilerMemSampler<BufferSize>::DProfilerMemSampler(struct TProfilerGppSamplerData* gppSamplerDataIn, TInt id) :
	DProfilerGenericSampler<BufferSize>(PROFILER_MEM_SAMPLER_ID)
    {
    LOGSTRING2("DProfilerMemSampler<%d>::CProfilerMemSampler",BufferSize);
	this->gppSamplerData = gppSamplerDataIn;
#ifndef MEM_EVENT_HANDLER
	this->iSamplingPeriod = 3000;	// set default setting
#endif
    iSyncOffset = 0;
    }

template <int BufferSize>
TInt DProfilerMemSampler<BufferSize>::Reset(DProfilerSampleStream* aStream, TUint32 aSyncOffset)
    {
    iSyncOffset = aSyncOffset;
    // check if reset called in stop (by driver)
    if(iSyncOffset != KStateSamplingEnding)
        {
        DProfilerGenericSampler<BufferSize>::Reset(aStream);
        memSamplerImpl.Reset();

#ifdef MEM_EVENT_HANDLER
        // reset member variables
        this->memSamplerImpl.iThreadsGathered = false;
        this->memSamplerImpl.iChunksGathered = false;
#ifdef MEM_EVENT_HANDLER_LIBRARY_EVENTS
        this->memSamplerImpl.iLibrariesGathered = false;
#endif
        
        // memory event handler
        if(iEventHandler)
            {
            // stop previous sampling if still running
            iEventHandler->Stop();
            }
        else
            {
            iEventHandler = new DMemoryEventHandler(this->iSampleBuffer, this->gppSamplerData);
            }
        
        if(iEventHandler)
            {
            TInt err(iEventHandler->Create());
            if(err != KErrNone)
                {
                Kern::Printf("Error in creation of DMemoryEventHandler, error %d", err);
                return err;
                }
            }
        else
            {
            Kern::Printf("Could not initiate DMemoryEventHandler");
            return KErrGeneral;
            }
    
        // set first chunk&thread memory lookup at the 5 ms, should be enough
#ifdef MEM_EVENT_HANDLER_LIBRARY_EVENTS
        this->memSamplerImpl.iMemSamplingPeriod = 10;
#else
        this->memSamplerImpl.iMemSamplingPeriod = 10;
#endif

#else   // ifdef mem event handler
        this->memSamplerImpl.iMemSamplingPeriod = this->iSamplingPeriod;
#endif
//        this->memSamplerImpl.iMemSamplingPeriodDiv2 = (TInt)(this->memSamplerImpl.iMemSamplingPeriod / 2);
//#ifdef MEM_EVENT_HANDLER_LIBRARY_EVENTS
//        this->memSamplerImpl.iMemSamplingPeriodDiv3 = (TInt)(this->memSamplerImpl.iMemSamplingPeriod / 3);
//#endif
	
        LOGSTRING3("CProfilerMemSampler<%d>::Reset - set mem sampling period to %d",
                                BufferSize,this->memSamplerImpl.iMemSamplingPeriod);
        }
	else   // iSyncOffset == KStateSamplingEnding
        {
        LOGSTRING2("DProfilerMemSampler<%d>::Reset - reset in stop", BufferSize);
#ifdef MEM_EVENT_HANDLER
        // destroy memory event handler
        if(iEventHandler)
            {
            // stop previous sampling if still running
            LOGSTRING("Stopping DMemoryEventHandler");
            iEventHandler->Stop();
            }
#endif
        return KErrNone;    // return if reset called in stop
        }

	// add MEM sample header
	TInt length(memSamplerImpl.CreateFirstSample());
	this->iSampleBuffer->AddSample(memSamplerImpl.sample,length);
	
    TUint8 memSamplerId(4);    // mem sampler id
    this->iSampleBuffer->AddSample(&memSamplerId,1);
    this->iSampleBuffer->AddSample((TUint8*)&(gppSamplerData->sampleNumber),4);
	this->sampleNeeded = false;
	LOGSTRING("DProfilerMemSampler::Reset - exit");
	return KErrNone;
    }

template <int BufferSize> 
TInt DProfilerMemSampler<BufferSize>::PostSample()
    {
    this->sampleNeeded = false;

    LOGSTRING3("DProfilerMemSampler<%d>::PostSample - state %d",BufferSize,this->iSampleBuffer->GetBufferStatus());
    
#ifdef MEM_EVENT_HANDLER
    // check if all threads and chunks (and libraries) are gathered
#ifdef MEM_EVENT_HANDLER_LIBRARY_EVENTS
    if(!memSamplerImpl.iThreadsGathered || !memSamplerImpl.iChunksGathered || !memSamplerImpl.iLibrariesGathered)
#else
    if(!memSamplerImpl.iThreadsGathered || !memSamplerImpl.iChunksGathered)
#endif
        {
#endif
        // disable interrupts for checking the kernel containers (EChunk, EThread)
//        TInt interruptLevel(NKern::DisableInterrupts(0));
        // first collect chunk data
        TInt length(this->memSamplerImpl.SampleImpl(this->gppSamplerData->sampleNumber));
        LOGSTRING2("DProfilerMemSampler<>::PostSample - in post sample, clock %d", this->memSamplerImpl.iCount );
        if(length != 0)
            {
            // then, encode the sample to the buffer until no further data available
            while(length > 0)
                {
                TInt ret =this->iSampleBuffer->AddSample(memSamplerImpl.sample,length);
                if (ret != 0)
                            {
                            Kern::Printf(("DProfilerMemSampler<>::PostSample() - add to sample buffer failed, loosing data, error = %d"),ret);                            }
                length = this->memSamplerImpl.SampleImpl(this->gppSamplerData->sampleNumber);
                LOGSTRING("DProfilerMemSampler<>::PostSample - in post sample again");
                
                // indicate that the whole MEM sample ends by having a 0x00 in the end
                if(length == 0)
                    {
                    TUint8 endMark(0);
                    LOGSTRING("MEM sampler PostSample - all samples generated!");
                    
                    this->iSampleBuffer->AddSample(&endMark,1);
                    LOGSTRING2("MEM sampler PostSample - end mark added, time: %d", gppSamplerData->sampleNumber);
                    
                    if (memSamplerImpl.iThreadsGathered && !memSamplerImpl.iChunksGathered)
                        {
                        LOGSTRING("MEM sampler PostSample - creating timestamp for chunks!");
                        // add start marker for chunks
                        TUint8 memSamplerId(4);    // mem sampler id
                        this->iSampleBuffer->AddSample(&memSamplerId,1);
                        this->iSampleBuffer->AddSample((TUint8*)&(gppSamplerData->sampleNumber),4);
                        }
#ifdef MEM_EVENT_HANDLER_LIBRARY_EVENTS
                    if (memSamplerImpl.iThreadsGathered && memSamplerImpl.iChunksGathered)
                        {
                        // add start marker for chunks
                        TUint8 memSamplerId(4);    // mem sampler id
                        this->iSampleBuffer->AddSample(&memSamplerId,1);
                        this->iSampleBuffer->AddSample((TUint8*)&(gppSamplerData->sampleNumber),4);
                        }
#endif
                    }
                } 
            }
        // restore interrupts and continue normal execution
//        NKern::RestoreInterrupts(interruptLevel);
#ifdef MEM_EVENT_HANDLER
    }
    // check if all threads and chunks are gathered
#ifdef MEM_EVENT_HANDLER_LIBRARY_EVENTS
    if(memSamplerImpl.iThreadsGathered && memSamplerImpl.iChunksGathered && memSamplerImpl.iLibrariesGathered)
#else
    if(memSamplerImpl.iThreadsGathered && memSamplerImpl.iChunksGathered)
#endif
        {
        if(iSyncOffset != KStateSamplingEnding)
            {
            // start memory event tracking after checking the current memory status
            if(!iEventHandler->Tracking())
                {
                LOGSTRING2("MEM sampler PostSample - Starting event handler timestamp : %x", gppSamplerData->sampleNumber);
                memSamplerImpl.iTimeToSample = false;
                iEventHandler->Start();
                LOGSTRING2("DProfilerMemSampler<%d>::PostSample - memory event handler started",BufferSize);
                }
            }
        }
#endif
    
    LOGSTRING2("MEM sampler PostSample - finished sampling, time: %d", gppSamplerData->sampleNumber);
    
    // finally perform superclass postsample
	TInt i(this->DProfilerGenericSampler<BufferSize>::PostSample());
    // notify event handler
    //iEventHandler->SampleHandled();
	return i;
    }

template <int BufferSize> 
TBool DProfilerMemSampler<BufferSize>::PostSampleNeeded()
    {
	LOGSTRING3("DProfilerMemSampler<%d>::PostSampleNeeded - state %d",BufferSize,this->iSampleBuffer->GetBufferStatus());

	TUint32 status(this->iSampleBuffer->GetBufferStatus());
    if(iEventHandler)
        {
        if(iEventHandler->Tracking())
            {
            this->sampleNeeded = iEventHandler->SampleNeeded();
            }
        }
    if(status == DProfilerSampleBuffer::BufferCopyAsap || status == DProfilerSampleBuffer::BufferFull || this->sampleNeeded == true)
        {
        return true;
        }
    
    return false;
    }

template <int BufferSize>
void DProfilerMemSampler<BufferSize>::Sample(TAny* aPtr)
    {
    LOGSTRING3("DProfilerMemSampler<%d>::Sample, time %d",BufferSize, this->memSamplerImpl.iCount);	
    if(iEventHandler)
        {
        if(!iEventHandler->Tracking())
            {
            // check if sample is needed, i.e. the sampling interval is met
            if(memSamplerImpl.SampleNeeded(gppSamplerData->sampleNumber)) 
                {
                // set the flag for post sampling
                this->sampleNeeded = true;
                /*
                LOGSTRING2("timestamp : 0x%04x",  gppSamplerData->sampleNumber);
                // start the MEM sample with the sample time
                TUint8 memSamplerId(4);    // mem sampler id
                this->iSampleBuffer->AddSample(&memSamplerId,1);
                this->iSampleBuffer->AddSample((TUint8*)&(gppSamplerData->sampleNumber),4);
                */
                // leave the rest of the processing for PostSample()
                }	
            }
//#ifdef MEM_EVENT_HANDLER
//	// call this to increase the time stamp
//	if(iEventHandler->SampleNeeded())
//	    {
//        // set the flag for post sampling
//        this->sampleNeeded = true;
//	    }
//#endif
        }
	return;
    }

template <int BufferSize>
DProfilerMemSampler<BufferSize>::~DProfilerMemSampler()
    {
	LOGSTRING2("CProfilerMemSampler<%d>::~CProfilerMemSampler",BufferSize);		
#ifdef MEM_EVENT_HANDLER
    // memory event handler
     if(iEventHandler)
         {
         // stop previous sampling if still running
         LOGSTRING("Stopping DMemoryEventHandler");
         iEventHandler->Stop();
         iEventHandler->Close();
         iEventHandler = NULL;
         }
#endif
    }
#endif
