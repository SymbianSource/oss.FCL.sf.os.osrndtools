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


#include <kern_priv.h>

#include <piprofiler/ProfilerGenericClassesKrn.h>


/*
 *	
 *	Class CProfilerSamplerBase implementation
 *
 */

inline DProfilerSamplerBase::DProfilerSamplerBase()
    {
	
    }

inline DProfilerSamplerBase::~DProfilerSamplerBase()
    {
	
    }

/*
 *	
 *	Class CProfilerSampleBuffer implementation
 *
 */

inline DProfilerSampleBuffer::DProfilerSampleBuffer(TUint8* aBuffer, 
											TUint8* aDblBuffer, 
											TUint32 aBufferSize )
    {
	LOGSTRING3("CProfilerSampleBuffer::CProfilerSampleBuffer AtFirst: b:0x%x db:0x%x",aBuffer,aDblBuffer);

	// make sure the alignment is right
	if((((TUint32)aBuffer) %4) != 0)
	    aBuffer += (4-(((TUint32)aBuffer)%4));
	if((((TUint32)aDblBuffer) %4) != 0)
	    aDblBuffer += (4-(((TUint32)aDblBuffer)%4));

	LOGSTRING3("CProfilerSampleBuffer::CProfilerSampleBuffer b:0x%x db:0x%x",aBuffer,aDblBuffer);

	iBufStruct = (TProfilerSampleBufStruct*)aBuffer;
	iDblBufStruct = (TProfilerSampleBufStruct*)aDblBuffer;

	LOGSTRING3("CProfilerSampleBuffer::CProfilerSampleBuffer bufStruct rem:0x%x dbuStruct rem:0x%x",
						&iBufStruct->iSampleRemainder,&iDblBufStruct->iSampleRemainder);

	iBufferDataSize = aBufferSize-4;
	iBufferRealSize = aBufferSize;

	ClearBuffer();
    }

inline DProfilerSampleBuffer::~DProfilerSampleBuffer()
    {
	
    }

inline TInt DProfilerSampleBuffer::AddSample(TUint8* aSample, TUint32 aLength)
    {
	TUint32 bytesTotal;
#ifdef __SMP__
	TInt intState(0);
#endif
	// check whether the buffer status is
	switch (iBufferStatus)
	    {
		case DProfilerSampleBuffer::BufferOk:
			// add the data normally to the buffer
			bytesTotal = iBytesWritten+aLength;

			if(bytesTotal < iBufferDataSize)
			    {
#ifdef __SMP__
				__SPIN_LOCK_IRQ(WriteSpinLock);
#endif
				memcpy((&(iBufStruct->iDataStart))+iBytesWritten,aSample,aLength);
#ifdef __SMP__
				__SPIN_UNLOCK_IRQ(WriteSpinLock);
#endif
				iBytesWritten+=aLength;
				return 0;
			    }
			else
			    {

				// the sample does not fit to the buffer
				// first copy as much data as we can fit to the first buffer
				TUint32 fitsToBuffer(iBufferDataSize-iBytesWritten);
				TUint32 remaining(aLength-fitsToBuffer);
				LOGSTRING2("DProfilerSampleBuffer::AddSample sample does not fit to buffer, cpu %d", NKern::CurrentCpu());
#ifdef __SMP__
				__SPIN_LOCK_IRQ(WriteSpinLock);
#endif
				memcpy((&(iBufStruct->iDataStart))+iBytesWritten,aSample,fitsToBuffer);
#ifdef __SMP__
				__SPIN_UNLOCK_IRQ(WriteSpinLock);
#endif
				iBytesWritten = iBufferDataSize;

#ifdef __SMP__
				intState = __SPIN_LOCK_IRQSAVE(BufferStateSpinLock);
#endif
				// ->switch to the double buffer
				iBufferStatus = DProfilerSampleBuffer::BufferCopyAsap;
#ifdef __SMP__
				__SPIN_UNLOCK_IRQRESTORE(BufferStateSpinLock, intState);
#endif
				TProfilerSampleBufStruct* tmpPtr(iBufStruct);
				iBufStruct = iDblBufStruct;
				iDblBufStruct = tmpPtr;
				
				iDblBytesWritten = iBytesWritten;

				// and this is the remainder of a sample
				// that will be copied to the new buffer
				// just in a while
				iBufStruct->iSampleRemainder = remaining;
				
				// now that the buffers have been switched
				// add the rest of the sample to the buffer
				aSample+=fitsToBuffer;

				// there should be room - in case the single sample
				// is smaller than the whole buffer! so we don't
				// bother to check
				//Kern::Printf("DProfilerSampleBuffer::AddSample else 2");
#ifdef __SMP__
				__SPIN_LOCK_IRQ(WriteSpinLock);
#endif 
				memcpy((&(iBufStruct->iDataStart)),aSample,remaining);
#ifdef __SMP__
				__SPIN_UNLOCK_IRQ(WriteSpinLock);
#endif 
				iBytesWritten = remaining;
				return 0;
			    }

		case DProfilerSampleBuffer::BufferCopyAsap:

			// no difference to the BufferOk case
			// unless the double buffer gets filled
			// before the data has been copied
			// add the data normally to the buffer
			bytesTotal = iBytesWritten+aLength;

			if(bytesTotal < iBufferDataSize)
			    {
				LOGSTRING2("DProfilerSampleBuffer::BufferCopyAsap, cpu %d", NKern::CurrentCpu());
#ifdef __SMP__
				__SPIN_LOCK_IRQ(WriteSpinLock);
#endif
				memcpy((&(iBufStruct->iDataStart))+iBytesWritten,aSample,aLength);
#ifdef __SMP__
				__SPIN_UNLOCK_IRQ(WriteSpinLock);
#endif 
				iBytesWritten+=aLength;
				return 0;
			    }
			else
			    {
				// the double buffer is now also full - there is no
				// place to put the data -> we have to waste it!
				// this is an indication of a too small buffer size
#ifdef __SMP__
				intState = __SPIN_LOCK_IRQSAVE(BufferStateSpinLock);
#endif
				iBufferStatus = DProfilerSampleBuffer::BufferFull;
#ifdef __SMP__
				__SPIN_UNLOCK_IRQRESTORE(BufferStateSpinLock, intState);
#endif
				Kern::Printf("DProfilerSampleBuffer::AddSample - double buffer full1!! cpu %d", NKern::CurrentCpu());
				return -1;
			    }

		case DProfilerSampleBuffer::BufferBeingCopied:

			// no difference to the BufferCopyAsap case
			bytesTotal = iBytesWritten+aLength;

			if(bytesTotal < iBufferDataSize)
			    {
				LOGSTRING2("DProfilerSampleBuffer::BufferBeingCopied iBufferDataSize, cpu %d", NKern::CurrentCpu());
#ifdef __SMP__
				__SPIN_LOCK_IRQ(WriteSpinLock);
#endif
				memcpy((&(iBufStruct->iDataStart))+iBytesWritten,aSample,aLength);
#ifdef __SMP__
				__SPIN_UNLOCK_IRQ(WriteSpinLock);
#endif 
				iBytesWritten+=aLength;
				return 0;
			    }
			else
			    {
				// the double buffer is now also full - there is no
				// place to put the data -> we have to waste it!
				// this is an indication of a too small buffer size
                Kern::Printf("DProfilerSampleBuffer::AddSample - double buffer full2!! cpu %d", NKern::CurrentCpu());
				// don't change the state to CProfilerSampleBuffer::BufferFull, since it is
				// already being copied
				return -1;
			    }

		case DProfilerSampleBuffer::BufferFull:
			// the buffer is still full, there is noting we can do
			// about it -> return
		    Kern::Printf("DProfilerSampleBuffer::AddSample - double buffer full3!! cpu %d", NKern::CurrentCpu());
			return -1;

		default:
		    Kern::Printf("DProfilerSampleBuffer::AddSample - wrong switch!!");
			return -1;
	    }
    }

inline void DProfilerSampleBuffer::EndSampling()
    {
#ifdef __SMP__
	TInt intState(0);
#endif
    LOGSTRING("DProfilerSampleBuffer::EndSampling");
	// this will switch to the dbl buffer even though
	// the buffer is not full, so that data can be copied

	// during this operation, no other buffer
	// operations are allowed

	// ensure that the normal buffer is in use and that the
	// buffer is in normal state ( this procedure is performed only once )
	if(iBufferStatus == DProfilerSampleBuffer::BufferOk)
	    {
		// ->switch to the double buffer
        LOGSTRING("DProfilerSampleBuffer::EndSampling - switching to double buffer");
#ifdef __SMP__
		intState = __SPIN_LOCK_IRQSAVE(BufferStateSpinLock);
#endif
		iBufferStatus = DProfilerSampleBuffer::BufferCopyAsap;
#ifdef __SMP__
		__SPIN_UNLOCK_IRQRESTORE(BufferStateSpinLock, intState);
#endif
		TProfilerSampleBufStruct* tmpPtr(iBufStruct);
		iBufStruct = iDblBufStruct;
		iDblBufStruct = tmpPtr;
				
		iDblBytesWritten = iBytesWritten;
		
		// there is no new sample so the remainder is
		// zero, (this shouldn't be used anyway)
		iBufStruct->iSampleRemainder = 0;
	    }
    }

inline TUint32 DProfilerSampleBuffer::GetBufferStatus()
    {
	return iBufferStatus;
    }

inline void DProfilerSampleBuffer::ClearBuffer()
    {
	LOGSTRING2("CProfilerSampleBuffer::ClearBuffer - %d",iBufferDataSize);
#ifdef __SMP__
	TInt intState(0);
#endif
	// the buffers are of same size
	TUint8* ptr1((TUint8*)&(iBufStruct->iDataStart));
	TUint8* ptr2((TUint8*)&(iDblBufStruct->iDataStart));

	for(TUint32 i(0);i<iBufferDataSize;i++)
	    {
		ptr1[i] = 0;
		ptr2[i] = 0;
	    }


	iBufStruct->iSampleRemainder = 0;
	iDblBufStruct->iSampleRemainder = 0;

	// written the dblBufStruct
	iBytesWritten = 0;
	iDblBytesWritten = 0;
	iDblBytesRead = 0;
#ifdef __SMP__
	intState = __SPIN_LOCK_IRQSAVE(BufferStateSpinLock);
#endif
	iBufferStatus = DProfilerSampleBuffer::BufferOk;
#ifdef __SMP__
	__SPIN_UNLOCK_IRQRESTORE(BufferStateSpinLock, intState);
#endif
    }

inline void DProfilerSampleBuffer::DataCopied()
    {
#ifdef __SMP__
	TInt intState(0);
#endif
	iDblBytesRead = 0;
	iDblBytesWritten = 0;
#ifdef __SMP__
	intState = __SPIN_LOCK_IRQSAVE(BufferStateSpinLock);
#endif
	iBufferStatus = DProfilerSampleBuffer::BufferOk;
#ifdef __SMP__
	__SPIN_UNLOCK_IRQRESTORE(BufferStateSpinLock, intState);
#endif
    }

/*
 *
 *	Class DProfilerSampleStream implementation
 *
 */

inline DProfilerSampleStream::DProfilerSampleStream()
    {
	LOGSTRING("DProfilerSampleStream::DProfilerSampleStream");

	iCurrentBuffer = 0;
	iPendingRequest = 0;
	iAddingSamples = 0;
	iClient = 0;
    }

inline DProfilerSampleStream::~DProfilerSampleStream()
    {
	LOGSTRING("DProfilerSampleStream::~DProfilerSampleStream");	
    }

inline void DProfilerSampleStream::InsertCurrentClient(DThread* aClient)
    {
	iClient = aClient;
	LOGSTRING2("DProfilerSampleStream::InsertCurrentClient - iClient is 0x%x",iClient);
    }


inline void DProfilerSampleStream::AddSampleBuffer(TBapBuf* aBuffer,TRequestStatus* aStatus)
    {
	if(iCurrentBuffer != 0 || iPendingRequest != 0)
	    {
		Kern::Printf("DProfilerSampleStream::AddSampleBuffer - ERROR 1");
		return;
	    }

	LOGSTRING3("DProfilerSampleStream::AddSampleBuffer - OK 0x%x,0x%x",aBuffer,aStatus);
	iCurrentBuffer = aBuffer;
	iPendingRequest = aStatus;
	
	LOGSTRING2("DProfilerSampleStream::AddSampleBuffer - Current Client is 0x%x",iClient);	
    }


inline void DProfilerSampleStream::ReleaseIfPending()
    {
    LOGSTRING("DProfilerSampleStream::ReleaseIfPending - entry");

	if(iCurrentBuffer != 0 && iPendingRequest != 0 && iClient != 0)
	    {
		LOGSTRING("DProfilerSampleStream::ReleaseIfPending - release buffer");

		LOGSTRING2("DProfilerSampleStream::AddSamples - completing request 0x%x",iPendingRequest);
		Kern::RequestComplete(iClient,iPendingRequest,KErrNone);
		
		iPendingRequest = 0;
		iCurrentBuffer = 0;
	    }

	LOGSTRING("DProfilerSampleStream::ReleaseIfPending - exit");
    }

inline void DProfilerSampleStream::AddSamples(DProfilerSampleBuffer& aBuffer, TInt aSamplerId)
    {
	LOGSTRING4("DProfilerSampleStream::AddSamples - entry ID: %d, currentbuffer: 0x%x, cpu %d", aSamplerId,iCurrentBuffer, NKern::CurrentCpu());
	if(iCurrentBuffer != 0)
	    {
		// the following will perform simple mutual exclusion
		iAddingSamples++;
		if(iAddingSamples > 1) 
		    {
			// there is someone else adding samples to the buffer
			Kern::Printf("DProfilerSampleStream::AddSamples - mutex in use");
			iAddingSamples--;
			return;
		    }

		LOGSTRING("DProfilerSampleStream::AddSamples - reading TBapBuf");
		
		// use a copy of the client TBapBuf structure during processing
		TBapBuf realBuf;
		TPtr8 ptr((TUint8*)&realBuf,(TInt)sizeof(TBapBuf));
	
		Kern::ThreadRawRead(iClient,(TAny*)(iCurrentBuffer),(TAny*)&realBuf,sizeof(TBapBuf));

		ptr.SetLength(sizeof(TBapBuf));

		LOGSTRING4("DProfilerSampleStream::AddSamples - read %d bytes from 0x%x of thread 0x%x",ptr.Size(),iCurrentBuffer,iClient);

		LOGSTRING5("DProfilerSampleStream::AddSamples - current buffer 0x%x -> b:0x%x s:%d d:%d",
			&realBuf,
			realBuf.iBuffer,
			realBuf.iBufferSize,
			realBuf.iDataSize);

		// get the address of the source buffer data
		TUint8* src((TUint8*)&(aBuffer.iDblBufStruct->iDataStart));
		src += aBuffer.iDblBytesRead;

		// the amount of data to copy is the 4 header bytes +
		// the remaining data in the buffer
		TInt amount(aBuffer.iDblBytesWritten-aBuffer.iDblBytesRead);

		TUint8* dst(realBuf.iBuffer);

		LOGSTRING4("DProfilerSampleStream::AddSamples - s:0x%x d:0x%x a:%d",src,dst,amount);

		if(realBuf.iDataSize == 0)
		    {
			LOGSTRING("DProfilerSampleStream::AddSamples - case 1");

			// the buffer is empty
			if(realBuf.iBufferSize >= (amount+4))
			    {
				LOGSTRING("DProfilerSampleStream::AddSamples - case 1.1");

				// the source buffer is smaller or of equal size than the amount of output data
				PerformCopy((TUint8)aSamplerId,src,realBuf.iBufDes,0,amount);
				realBuf.iDataSize += amount+4;
				// the rest of the source buffer was copied at once, so signal the buffer
				aBuffer.DataCopied();
			    }
			else
			    {
				LOGSTRING("DProfilerSampleStream::AddSamples - case 1.2");

				// only a part of the source buffer will fit to the client side buffer
				amount = realBuf.iBufferSize-4;
				PerformCopy((TUint8)aSamplerId,src,realBuf.iBufDes,0,amount);
				realBuf.iDataSize += amount+4;
				// add the amount of bytes read to the source buffer
				aBuffer.iDblBytesRead+=amount;
			    }
		    }
		else
		    {
			LOGSTRING("DProfilerSampleStream::AddSamples - case 2");

			// there is data in the client buffer
			dst += realBuf.iDataSize;
			TInt remainingSpace(realBuf.iBufferSize-realBuf.iDataSize);

			if( remainingSpace >= (amount+4) )
			    {
				LOGSTRING("DProfilerSampleStream::AddSamples - case 2.1");

				// the source buffer is smaller or of equal size than the amount of output data
				PerformCopy((TUint8)aSamplerId,src,realBuf.iBufDes,realBuf.iDataSize,amount);
				realBuf.iDataSize += (amount+4);
				// the rest of the source buffer was copied at once, so signal the buffer
				aBuffer.DataCopied();
			    }
			else
			    {
				LOGSTRING("DProfilerSampleStream::AddSamples - case 2.2");

				// only a part of the source buffer will fit to the client side buffer
				if(remainingSpace >= 12)
				    {
					LOGSTRING("DProfilerSampleStream::AddSamples - case 2.3");

					amount = remainingSpace-4;
					// there are at least 8 bytes left for data, write it
					PerformCopy((TUint8)aSamplerId,src,realBuf.iBufDes,realBuf.iDataSize,amount);
					realBuf.iDataSize += (amount+4);
					// add the amount of bytes read to the source buffer
					aBuffer.iDblBytesRead+=amount;				
				    }
			    }
		    }
	
		// copy the data in the modified TBapBuf structure back to the client
		LOGSTRING("DProfilerSampleStream::AddSamples - writing TBapBuf");

		Kern::ThreadDesWrite(iClient,(TAny*)(realBuf.iDes),ptr,0,KChunkShiftBy0,iClient);

		// if the client side buffer is full or nearly full, signal the client
		LOGSTRING("DProfilerSampleStream::AddSamples - data copied");

		if(realBuf.iBufferSize-realBuf.iDataSize < 12)
		    {
			LOGSTRING("DProfilerSampleStream::AddSamples - release buffer");
			
			LOGSTRING2("DProfilerSampleStream::AddSamples - completing request 0x%x",iPendingRequest);

			Kern::RequestComplete(iClient,iPendingRequest,KErrNone);

			iPendingRequest = 0;
			iCurrentBuffer = 0;
			//iClient = 0;
		    }

		// free the lock
		iAddingSamples--;
	    }
	LOGSTRING("DProfilerSampleStream::AddSamples - exit");
    }



inline TInt DProfilerSampleStream::EndSampling(DProfilerSampleBuffer& aBuffer,TInt aSamplerId)
    {
    LOGSTRING2("DProfilerSampleStream::EndSampling, sampler ID: %d",aSamplerId);
#ifdef __SMP__
	TInt intState(0);
#endif
	// switch the buffer to double buffer
	// even though it would not be full yet
	// the switch is done only once / end sampling procedure
	// (Only with BufferOk status)
	aBuffer.EndSampling();
	
	LOGSTRING2("DProfilerSampleStream::EndSampling, iClient: 0x%x",iClient);
	
	if(aBuffer.iBufferStatus != DProfilerSampleBuffer::BufferDataEnd)
	    {
		// add these final samples to the client buffer
		AddSamples(aBuffer,aSamplerId);
		
		// if all data was copied to the buffer, the buffer status is now BufferOk

		if(aBuffer.iBufferStatus != DProfilerSampleBuffer::BufferOk)
		    {
            LOGSTRING("DProfilerSampleStream::EndSampling - more data to copy");
			// there is still more data to copy, the pending request should have been
			// completed in AddSamples(), because the client buffer got filled 
			return 1;
		    }
		else
		    {
			// buffer status was changed to BufferOk in AddSamples() - 
			// this means all data from it could be copied
			// now we have to change the status of the buffer to BufferDataEnd, so
			// we know that the particular buffer has no more data to copy
            LOGSTRING("DProfilerSampleStream::EndSampling - switch to BufferDataEnd");
#ifdef __SMP__
			intState = __SPIN_LOCK_IRQSAVE(BufferStateSpinLock);
#endif
			aBuffer.iBufferStatus = DProfilerSampleBuffer::BufferDataEnd;
#ifdef __SMP__
			__SPIN_UNLOCK_IRQRESTORE(BufferStateSpinLock, intState);
#endif
		    }
	    }

	// the buffer was completely emptied to the client buffer, or there was no
	// data to copy to the client side
	LOGSTRING("DProfilerSampleStream::EndSampling - no more data to copy");

	return 0;
    }

inline void DProfilerSampleStream::PerformCopy(TUint8 aSamplerId,TUint8* aSrc,TPtr8* aDst,TInt aOffset,TInt aAmount)
    {
	LOGSTRING2("DProfilerSampleStream::PerformCopy for sampler ID: %d",aSamplerId);	
	LOGSTRING5("DProfilerSampleStream::PerformCopy - 0x%x -> 0x%x - %d - offset: %d",aSrc, aDst, aAmount, aOffset);	
	TUint32 header;
	header = aAmount & 0x00ffffff;
	header += (aSamplerId << 24);
	TPtr8 ptr((TUint8*)&header,4);
	ptr.SetLength(4);

	LOGSTRING2("DProfilerSampleStream::PerformCopy - start header copy HDR = 0x%x",header);	

	// write the header
	TInt err(Kern::ThreadDesWrite(iClient,(TAny*)aDst,ptr,aOffset,KChunkShiftBy0));
	if (err != KErrNone)
		{
		Kern::Printf(("DProfilerSampleStream::PerformCopy() - thread des write error, %d"), err);
		}

	LOGSTRING2("DProfilerSampleStream::PerformCopy - copied header %d bytes",ptr.Size());	
	aOffset+=4;

	LOGSTRING("DProfilerSampleStream::PerformCopy - start copy");	
	// write the data
	ptr.Set(aSrc,aAmount,aAmount);
	ptr.SetLength(aAmount);
	
	err = Kern::ThreadDesWrite(iClient,(TAny*)aDst,ptr,aOffset,KChunkShiftBy0);
	if (err != KErrNone)
		{
		Kern::Printf(("DProfilerSampleStream::PerformCopy() - thread des write error, %d"), err);
		}

	LOGSTRING2("DProfilerSampleStream::PerformCopy - copied data %d bytes",ptr.Size());	

    }
