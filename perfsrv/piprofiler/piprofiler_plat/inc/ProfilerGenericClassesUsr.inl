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


#include <f32file.h>
#include <e32svr.h>

#include <piprofiler/ProfilerTraces.h>

// constants
const TInt KInitialFreeBufferAmount = 4;

/*
 *
 *	Class CProfilerBufferHandler implementation
 *
 */
inline CProfilerBufferHandler* CProfilerBufferHandler::NewL(CProfilerSampleStream& aStream, RPluginSampler& aSampler)
	{
	LOGTEXT(_L("CProfilerBufferHandler::NewL - entry"));
	CProfilerBufferHandler* self = new(ELeave) CProfilerBufferHandler(aStream, aSampler);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;   
	}

inline CProfilerBufferHandler::CProfilerBufferHandler(CProfilerSampleStream& aStream, RPluginSampler& aSampler)
    : CActive(EPriorityStandard),
    iSampler(aSampler),
    iObserver(aStream)
    {
    }

inline CProfilerBufferHandler::~CProfilerBufferHandler()
	{
	LOGTEXT(_L("CProfilerBufferHandler::~CProfilerBufferHandler() - entry"));
	}

inline void CProfilerBufferHandler::ConstructL()
	{
	LOGTEXT(_L("CProfilerBufferHandler::ConstructL - entry"));
	iBufferInProcess = 0;
	iEndOfStreamDetected = false;
	
	iFinished = 0;
	// add the buffer handler to the active scheduler
	CActiveScheduler::Add(this);
	}

inline void CProfilerBufferHandler::StartReceivingData()
	{
	LOGTEXT(_L("CProfilerBufferHandler::StartReceivingData - entry"));

	iEndOfStreamDetected = false;
	// this method initiates receiving data from the sampler
	iBufferInProcess = iObserver.GetNextFreeBuffer();

	LOGSTRING5("CProfilerBufferHandler::StartReceivingData - 0x%x -> b:0x%x s:%d d:%d",
					iBufferInProcess,
					iBufferInProcess->iBuffer,
					iBufferInProcess->iBufferSize,
					iBufferInProcess->iDataSize);

	iSampler.FillThisStreamBuffer(iBufferInProcess,iStatus);

	LOGTEXT(_L("CProfilerBufferHandler::StartReceivingData - SetActive"));
	SetActive();

	LOGTEXT(_L("CProfilerBufferHandler::StartReceivingData - exit"));
	}

inline TInt CProfilerBufferHandler::RunError(TInt aError)
    {
    // handle the error case by stopping the trace
    HandleEndOfStream();
    return aError;
    }

inline void CProfilerBufferHandler::HandleEndOfStream()
    {
    LOGTEXT(_L("CProfilerBufferHandler::HandleEndOfStream - entry"));
    // Cancel has been called, the stream should be about to end now,
    // we will wait for the rest of the buffers to be filled synchronously
    // the end of the stream will be indicated through an empty buffer
    // at first, complete the ongoing request
    if(iStatus == KRequestPending && iBufferInProcess != 0)
        {
        LOGTEXT(_L("CProfilerBufferHandler::HandleEndOfStream - case 1"));

        // wait for the buffer to be filled synchronously
        User::WaitForRequest(iStatus);
        
        // add the received buffer to the list of filled buffers
        iObserver.AddToFilledBuffers(iBufferInProcess);
        // continue writing to output
        iObserver.NotifyWriter();
        
        if(iBufferInProcess->iDataSize == 0)
            {
            // a buffer with size 0 was received
            LOGTEXT(_L("CProfilerBufferHandler::DoCancel - case 1.1"));
            iEndOfStreamDetected = true;
            }

        // there will be no more asynchronous requests
        iBufferInProcess = 0;
        }
    else if (iBufferInProcess != 0)
        {
        LOGTEXT(_L("CProfilerBufferHandler::HandleEndOfStream - case 2"));

        // add the buffer into filled, i.e. ready-to-write buffers
        iObserver.AddToFilledBuffers(iBufferInProcess);
        iObserver.NotifyWriter();
        
        if(iBufferInProcess->iDataSize == 0)
            {
            // a buffer with size 0 was received
            LOGTEXT(_L("CProfilerBufferHandler::HandleEndOfStream - case 2.1"));
            iEndOfStreamDetected = true;
            }       
        // there will be no more asynchronous requests
        iBufferInProcess = 0;
        //delete iBufferInProcess;
        //iBufferInProcess = NULL;    
        //iObserver.AddToFreeBuffers(iBufferInProcess);
        }
    // then, continue until end of stream has been reached
    while(iEndOfStreamDetected == false)
        {
        // the end of stream has not yet been detected, so get more
        // buffers from the sampler, until we get an empty one

        if(iStatus == KRequestPending)
            {
            LOGTEXT(_L("CProfilerBufferHandler::HandleEndOfStream - ERROR 1"));
            }

        LOGTEXT(_L("CProfilerBufferHandler::HandleEndOfStream - case 3"));

        TBapBuf* nextFree = iObserver.GetNextFreeBuffer();  
        iSampler.FillThisStreamBuffer(nextFree,iStatus);
        // wait for the buffer to be filled synchronously
        User::WaitForRequest(iStatus);
        
        // call the writer plugin to write data to output
        iObserver.AddToFilledBuffers(nextFree);
        iObserver.NotifyWriter();
        
        // check if end-of-data message (i.e. data size is 0 sized) received
        if(nextFree->iDataSize == 0)
            {
            LOGTEXT(_L("CProfilerBufferHandler::HandleEndOfStream - case 3.1"));
            // a buffer with size 0 was received
            iEndOfStreamDetected = true;
            nextFree = 0;
            }
        }
    }

inline void CProfilerBufferHandler::RunL()
	{
	LOGTEXT(_L("CProfilerBufferHandler::RunL - entry"));
	// is called by the active scheduler
	// when a buffer has been received

	// buffer with dataSize 0 is returned when the sampling ends
	if(iBufferInProcess->iDataSize != 0)
	    {
	    LOGTEXT(_L("CProfilerBufferHandler::RunL - buffer received"));

		TBapBuf* nextFree = iObserver.GetNextFreeBuffer();
		if(nextFree == 0)
		    {
		    LOGTEXT(_L("CProfilerSampleStream::RunL - GetNextFreeBuffer failed!!"));
		    }
		else{
		    LOGSTRING5("CProfilerBufferHandler::RunL - 0x%x -> b:0x%x s:%d d:%d",
					nextFree,
					nextFree->iBuffer,
					nextFree->iBufferSize,
					nextFree->iDataSize);

            iSampler.FillThisStreamBuffer(nextFree,iStatus);
            SetActive();        
    
            LOGTEXT(_L("CProfilerBufferHandler::RunL - issued new sample command"));
    
            // add the received buffer to the list of filled buffers
            iObserver.AddToFilledBuffers(iBufferInProcess);
            iObserver.NotifyWriter();
    
            // the empty buffer is now the one being processed
            iBufferInProcess = nextFree;
            }
//        LOGTEXT(_L("CProfilerBufferHandler::RunL - SetActive"));
//        SetActive();        
		}
	else
		{
		LOGTEXT(_L("CProfilerBufferHandler::RunL - end of stream detected"));
		iEndOfStreamDetected = true;
		
		// add the empty buffer to the writer so that it will also get the information
		// about the finished stream
		iObserver.AddToFilledBuffers(iBufferInProcess);
		iObserver.NotifyWriter();

		iBufferInProcess = 0;
		Cancel();

		}
	LOGTEXT(_L("CProfilerBufferHandler::RunL - exit"));
	}

inline void CProfilerBufferHandler::DoCancel()
    {
    LOGTEXT(_L("CProfilerBufferHandler::DoCancel - entry"));
    HandleEndOfStream();
    LOGTEXT(_L("CProfilerBufferHandler::DoCancel - exit"));
    }


/*
 *
 *	Class CProfilerSampleStream implementation
 *
 *  - used by Plugin
 **/

inline CProfilerSampleStream* CProfilerSampleStream::NewL(TInt aBufSize)
	{
	LOGTEXT(_L("CProfilerSampleStream::NewL - entry"));
	CProfilerSampleStream* self = new(ELeave) CProfilerSampleStream(aBufSize);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;   
	}

inline CProfilerSampleStream::CProfilerSampleStream(TInt aBufSize) : 
    iBufferSize(aBufSize)
	{
	LOGTEXT(_L("CProfilerSampleStream::CProfilerSampleStream - entry"));
	
    iFilledBuffers = 0;
    iFreeBuffers = 0;
    iFinished = 0;
    
	LOGTEXT(_L("CProfilerSampleStream::CProfilerSampleStream - exit"));	
	}

inline CProfilerSampleStream::~CProfilerSampleStream()
	{
	LOGTEXT(_L("CProfilerSampleStream::~CProfilerSampleStream - entry"));

	// empty all buffers
	EmptyBuffers();
	User::Free(iFilledBuffers);
    	User::Free(iFreeBuffers);
	//delete iFilledBuffers;
	//delete iFreeBuffers;
	    
	LOGTEXT(_L("CProfilerSampleStream::~CProfilerSampleStream - exit"));
	}

inline void CProfilerSampleStream::ConstructL()
	{

	}

inline void CProfilerSampleStream::SetWriter(CWriterPluginInterface& aWriter)
    {
    // set writer plugin
    iWriter = &aWriter;
    }

inline void CProfilerSampleStream::Finalise()
	{
	LOGTEXT(_L("CProfilerSampleStream::Finalise - entry"));
	}

inline void CProfilerSampleStream::ResetBuffers()
    {

    // empty buffers
    EmptyBuffers();

    // re-initialise buffers
    InitialiseBuffers();
    }

inline void CProfilerSampleStream::InitialiseBuffers()
    {
    // re-initialize member variables
    iFilledBuffers = 0;
    iFreeBuffers = 0;
    iFinished = 0;
    
    // create three(orig. two) new TBapBuf objects and add them to the
    // list of free buffers
    for(TInt i(0);i<KInitialFreeBufferAmount;i++)
        {
        // alloc new buffer
        TBapBuf* newBuf = (TBapBuf*)User::Alloc(sizeof(TBapBuf));
        if(newBuf != 0)
            {
            newBuf->iBuffer = (TUint8*)User::Alloc(iBufferSize);
            if(newBuf->iBuffer != 0)
                {
                // initialize the new buffer
                newBuf->iBufferSize = iBufferSize;
                newBuf->iDataSize = 0;
                newBuf->iNext = 0;
                newBuf->iDes = new TPtr8((TUint8*)newBuf,sizeof(TBapBuf));
                newBuf->iDes->SetLength(sizeof(TBapBuf));
                newBuf->iBufDes = new TPtr8((TUint8*)newBuf->iBuffer,iBufferSize);
                newBuf->iBufDes->SetLength(iBufferSize);
                LOGSTRING3("CProfilerSampleStream::InitialiseBuffers - newBuf 0x%x newBuf->iNext 0x%x",newBuf, newBuf->iNext);
                AddToFreeBuffers(newBuf);
                }
            else
                {
                delete newBuf->iBuffer;
                delete newBuf;
                LOGTEXT(_L("CProfilerSampleStream::InitialiseBuffers - Out of memory (1)!!"));
                }
            }
        else
            {
            LOGTEXT(_L("CProfilerSampleStream::InitialiseBuffers - Out of memory (2)!!"));
            delete newBuf;
            }       
        }
    }

inline void CProfilerSampleStream::EmptyBuffers()
    {
	LOGTEXT(_L("CProfilerSampleStream::EmptyBuffers - entry"));

	// delete all free buffers
	while(iFreeBuffers != 0)
	    {
		LOGSTRING2("CProfilerSampleStream::EmptyBuffers - deleting 0x%x",iFreeBuffers);

		// store the next buffer in the list
		TBapBuf* nextFree = iFreeBuffers->iNext;
		// delete the first one in the list
		delete iFreeBuffers->iBufDes;
		delete iFreeBuffers->iDes;
		delete iFreeBuffers->iBuffer;
		//delete iFreeBuffers;
		User::Free(iFreeBuffers);
		// set the list start to the next buffer
		iFreeBuffers = nextFree;
	    }
	iFreeBuffers = 0;
	// delete all filled buffers
	    while(iFilledBuffers != 0)
	        {
	        LOGSTRING2("CProfilerSampleStream::EmptyBuffers - deleting 0x%x",iFilledBuffers);

	        // store the next buffer in the list
	        TBapBuf* nextFilled = iFilledBuffers->iNext;
	        // delete the first one in the list
	        delete iFilledBuffers->iBufDes;
	        delete iFilledBuffers->iDes;
	        delete iFilledBuffers->iBuffer;
	        delete iFilledBuffers;
	        User::Free(iFilledBuffers);
	        // set the list start to the next buffer
	        iFilledBuffers = nextFilled;
	        }
	    iFilledBuffers = 0;
	LOGTEXT(_L("CProfilerSampleStream::EmptyBuffers - exit"));
    }

inline TBapBuf* CProfilerSampleStream::GetNextFreeBuffer()
	{
	LOGTEXT(_L("CProfilerSampleStream::GetNextFreeBuffer - entry"));

	// get a new buffer from the free buffers list
	TBapBuf* nextFree = iFreeBuffers;
	
	// check if we got a buffer or not
	if(nextFree == 0)
	    {
		// if there are no free buffers,
		// create a new one
		LOGTEXT(_L("CProfilerSampleStream::GetNextFreeBuffer - creating new buffer"));
		TBapBuf* newBuf = (TBapBuf*)User::Alloc(sizeof(TBapBuf));
		if(newBuf != 0)
		    {
			newBuf->iBuffer = (TUint8*)User::Alloc(iBufferSize);
			if(newBuf->iBuffer != 0)
			    {
				newBuf->iBufferSize = iBufferSize;
				newBuf->iDataSize = 0;
				newBuf->iNext = 0;
				newBuf->iDes = new TPtr8((TUint8*)newBuf,sizeof(TBapBuf));
				newBuf->iDes->SetLength(sizeof(TBapBuf));
				newBuf->iBufDes = new TPtr8((TUint8*)newBuf->iBuffer,iBufferSize);
				newBuf->iBufDes->SetLength(iBufferSize);
				nextFree = newBuf;
			    }
			else
			    {
			    delete newBuf->iBufDes;
			    delete newBuf->iDes;
			    delete newBuf->iBuffer;
			    delete newBuf;
			    delete nextFree;
				LOGTEXT(_L("CProfilerSampleStream::GetNextFreeBuffer - Out of memory (1)!!"));
				return 0;
			    }
		    }
		else
		    {
			LOGTEXT(_L("CProfilerSampleStream::GetNextFreeBuffer - Out of memory (2)!!"));
			delete newBuf;
			delete nextFree;
			return 0;
		    }		
	    }// nextFree != 0
	else
	    {
		// set the list to point to next free buffer
	    LOGSTRING3("CProfilerSampleStream::GetNextFreeBuffer 0x%x buf 0x%x inext", nextFree, nextFree->iNext);
		iFreeBuffers = nextFree->iNext;
	    }

	LOGTEXT(_L("CProfilerSampleStream::GetNextFreeBuffer - exit"));
	return nextFree;
    }

inline void CProfilerSampleStream::AddToFilledBuffers(TBapBuf* aFilledBuffer)
    {
    LOGSTRING4("CProfilerSampleStream::AddToFilledBuffers - entry, size %d aFilledBuffer* 0x%x iNext 0x%x", aFilledBuffer->iDataSize, aFilledBuffer, aFilledBuffer->iNext);
    //LOGSTRING3("CProfilerSampleStream::AddToFilledBuffers  iFilledBuffers* 0x%x iNext 0x%x",iFilledBuffers, iFilledBuffers->iNext);
    // add this buffer to the list of filled buffers
    if(iFilledBuffers == 0)
        {
        LOGSTRING("CProfilerSampleStream::AddToFilledBuffers iFilledBuffers == 0");
        // the list is empty, so add the the beginning of the list
        // there is no next buffer in the list at the moment
        aFilledBuffer->iNext = 0;
        iFilledBuffers = aFilledBuffer;
        }
    else
        {
        LOGSTRING("CProfilerSampleStream::AddToFilledBuffers iFilledBuffers != 0");
        // there are buffers in the list, add this buffer to the beginning of the list
        if(aFilledBuffer == iFilledBuffers)
            {
            LOGSTRING("CProfilerSampleStream::AddToFilledBuffers iFilledBuffers same");
            }
        else
            {
            LOGSTRING("CProfilerSampleStream::AddToFilledBuffers iFilledBuffers different");
            LOGSTRING3("iFilledBuffers 0x%x iFilledBuffers->iNext 0x%x", iFilledBuffers, iFilledBuffers->iNext);
            if(iFilledBuffers->iNext != 0)
                {
                LOGSTRING3("next buf 0x%x ->iNext 0x%x", iFilledBuffers->iNext,iFilledBuffers->iNext->iNext );
                }
            }
            aFilledBuffer->iNext = iFilledBuffers;
            iFilledBuffers = aFilledBuffer;
        }
    LOGSTRING3("CProfilerSampleStream::AddToFilledBuffers - exit, iFilledBuffer* 0x%x ->iNext 0x%x", iFilledBuffers, iFilledBuffers->iNext);
    //LOGTEXT(_L("CProfilerSampleStream::AddToFilledBuffers - exit "));
    }

TBapBuf* CProfilerSampleStream::GetNextFilledBuffer()
    {
    LOGTEXT(_L("CProfilerSampleStream::GetNextFilledBuffer - entry"));
    if(iFilledBuffers == 0)
        {
        // there are no filled buffers in the list
        LOGTEXT(_L("CProfilerSampleStream::GetNextFilledBuffer - no filled bufs"));
        return 0;
        }
    else
        {   
        // get a buffer from the end of the list
        TBapBuf* buf = iFilledBuffers;
        TBapBuf* prev = 0;
        if(buf)
            {
            LOGSTRING3("CProfilerSampleStream::GetNextFilledBuffer buf 0x%x, inext 0x%x", buf, buf->iNext);
            if(buf->iNext == 0)
                {
                // this was the last (and only) buffer
                iFilledBuffers = 0;
                LOGTEXT(_L("CProfilerSampleStream::GetNextFilledBuffer - last filled"));
                return buf;
                }
            else
                {
                LOGSTRING2("CProfilerSampleStream::GetNextFilledBuffer - searching last filled, inext 0x%x ", buf->iNext);
                while(buf->iNext != 0)
                    {
                    // there are two or more buffers in the list
                    // proceed until the end of the list is found
                    LOGSTRING3("CProfilerSampleStream::GetNextFilledBuffer - searching. buf 0x%x, inext 0x%x", buf, buf->iNext);
                    prev = buf;
                    buf = buf->iNext;
                    }
                // now buf->next is 0, return buf and set the next
                // element of prev to NULL
                prev->iNext = 0;
                LOGSTRING3("CProfilerSampleStream::GetNextFilledBuffer - found last. buf 0x%x, iNext 0x%x", buf, buf->iNext);
                return buf;
                }
            }
        else{
            LOGTEXT(_L("CProfilerSampleStream::GetNextFilledBuffer - nullihan se siellä"));
            }   
        }
    }

inline void CProfilerSampleStream::AddToFreeBuffers(TBapBuf* aFreeBuffer)
    {
	LOGTEXT(_L("CProfilerSampleStream::AddToFreeBuffers - entry"));

	// set data size of the buffer to 0
	aFreeBuffer->iDataSize = 0;

	// add this buffer to the list of free buffers
	if(iFreeBuffers == 0)
	    {
		// this is the first buffer, so set the next to point to NULL
	    aFreeBuffer->iNext = 0;
	    }
	else
	    {
		// otherwise set to point to the beginning of the list
	    aFreeBuffer->iNext = iFreeBuffers;
	    }

	// set this buffer to be the first one in the list
	iFreeBuffers = aFreeBuffer;

	LOGSTRING3("CProfilerSampleStream::AddToFreeBuffers - exit iFreeBuffers 0x%x iFreeBuffers->iNext 0x%x",iFreeBuffers, iFreeBuffers->iNext);
    }

void CProfilerSampleStream::NotifyWriter()
    {
    // notify writer plugin to write data from filled buffer list
    LOGTEXT(_L("CProfilerSampleStream::NotifyWriter() - entry"));
    iWriter->WriteData();
    LOGTEXT(_L("CProfilerSampleStream::NotifyWriter() - exit"));
    }

// end of file



