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

#include "MemSpyDriverLogChanHeapData.h"

// System includes
#include <u32hal.h>
#include <e32rom.h>
#include <memspy/driver/memspydriverobjectsshared.h>
#include <memspy/driver/memspydriverconstants.h>

// Shared includes
#include "MemSpyDriverOpCodes.h"
#include "MemSpyDriverObjectsInternal.h"

// User includes
#include "MemSpyDriverHeap.h"
#include "MemSpyDriverUtils.h"
#include "MemSpyDriverDevice.h"
#include "MemSpyDriverOSAdaption.h"
#include "MemSpyDriverHeapWalker.h"
#include "MemSpyDriverUserEventMonitor.h"
#include "MemSpyDriverSuspensionManager.h"




DMemSpyDriverLogChanHeapData::DMemSpyDriverLogChanHeapData( DMemSpyDriverDevice& aDevice, DThread& aThread )
:	DMemSpyDriverLogChanHeapBase( aDevice, aThread ), iKernelHeap( aDevice.OSAdaption() )
    {
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapData::DMemSpyDriverLogChanHeapData() - this: 0x%08x", this ));
    }


DMemSpyDriverLogChanHeapData::~DMemSpyDriverLogChanHeapData()
	{
	TRACE( Kern::Printf("DMemSpyDriverLogChanHeapData::~DMemSpyDriverLogChanHeapData() - START - this: 0x%08x", this ));

	TRACE( Kern::Printf("DMemSpyDriverLogChanHeapData::~DMemSpyDriverLogChanHeapData() - END - this: 0x%08x", this ));
	}







TInt DMemSpyDriverLogChanHeapData::Request( TInt aFunction, TAny* a1, TAny* a2 )
	{
	TInt r = DMemSpyDriverLogChanHeapBase::Request( aFunction, a1, a2 );
    if  ( r == KErrNone )
        {
	    switch( aFunction )
		    {
        case EMemSpyDriverOpCodeHeapDataGetUser:
            r = GetHeapDataUser( (TMemSpyDriverInternalHeapDataParams*) a1 );
            break;
        case EMemSpyDriverOpCodeHeapDataGetKernelInit:
            r = GetHeapDataKernelInit( (TMemSpyHeapInfo*) a1, (TDes8*) a2 );
            break;
        case EMemSpyDriverOpCodeHeapDataGetKernelFetch:
            r = GetHeapDataKernelFetch( (TDes8*) a1 );
            break;

        default:
            r = KErrNotSupported;
		    break;
		    }
        }
    //
    return r;
	}


TBool DMemSpyDriverLogChanHeapData::IsHandler( TInt aFunction ) const
    {
    return ( aFunction > EMemSpyDriverOpCodeHeapDataBase && aFunction < EMemSpyDriverOpCodeHeapDataEnd );
    }














TInt DMemSpyDriverLogChanHeapData::GetHeapDataUser( TMemSpyDriverInternalHeapDataParams* aParams )
    {
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapData::GetHeapDataUser() - START"));

	TMemSpyDriverInternalHeapDataParams params;
    TInt r = Kern::ThreadRawRead( &ClientThread(), aParams, &params, sizeof(TMemSpyDriverInternalHeapDataParams) );
    //
    if  ( r == KErrNone )
        {
	    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapData::GetHeapDataUser - thread id: %d, remaining: %8d, vtable: 0x%08x", params.iTid, params.iRemaining, params.iRHeapVTable));

        // Work out if we need to read the user or kernel heap data. 
        //
        // NB: The two 'get heap data' functions return either an error ( < KErrNone ) or then
        // return the length of the descriptor data that was written to the client's address
        // space.
        //
        r = GetHeapDataUser( params );

        // Write back to user space if everything went okay. Remember that the 
        // return value above will be the length of data that was written to the
        // client if there was no error.
        if  ( r >= KErrNone )
            {
            const TInt clientDescriptorLength = r;

            // Make sure we update client's remaining data
            r = Kern::ThreadRawWrite( &ClientThread(), aParams, &params, sizeof(TMemSpyDriverInternalHeapDataParams) );
            
            // ... and if that went okay, then we return the length of the descriptor data
            // back to the client.
            if  ( r == KErrNone )
                {
                r = clientDescriptorLength;
                }
            }
        }
    else
        {
    	TRACE( Kern::Printf("DMemSpyDriverLogChanHeapData::GetHeapDataUser - params read error: %d", r));
        }

    // Done
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapData::GetHeapDataUser() - END - ret: %d", r));
    return r;
    }


TInt DMemSpyDriverLogChanHeapData::GetHeapDataKernelInit( TMemSpyHeapInfo* aInfo, TDes8* aFreeCells )
    {
    // First phase is to 
    //
    // a) Open kernel heap
    // b) Make a copy of the heap data
    // c) Walk copy in order to extract statistics (meta data, i.e. TMemSpyHeapInfo)
    //
    // The driver leaves kernel context with the copy of the kernel heap still associated with MemSpy's process.
    // The second driver call will copy the chunk data to user side and release the kernel side chunk.
    //const TBool isInit = iKernelHeap.ChunkIsInitialised();
    //TRACE_KH( Kern::Printf("DMemSpyDriverLogChanHeapData::GetHeapDataKernelInit() - START - isInit: %d", isInit ));
    //__ASSERT_ALWAYS( !isInit, MemSpyDriverUtils::PanicThread( ClientThread(), EPanicKernelHeapDataInitError ) );

    iKernelHeap.Reset();
    NKern::ThreadEnterCS();

    TFullName heapChunkName;
    TInt r = OpenKernelHeap( iKernelHeap, &heapChunkName );
    TRACE_KH( Kern::Printf("DMemSpyDriverLogChanHeapData::GetHeapDataKernelInit() - open err: %d", r));

    if  ( r == KErrNone )
        {
        r = GetHeapInfoKernel( iKernelHeap, aInfo, aFreeCells );
        TRACE_KH( Kern::Printf("DMemSpyDriverLogChanHeapInfo::GetHeapDataKernelInit() - base class get heap info: %d", r) );

        // If everything was okay, we can now return back to user-side, indicating the amount of heap data
        // that they must prepare to read (in the next operation).
        if  ( r == KErrNone )
            {
            // Indicate how big a buffer the user-side must prepare.
            r = OSAdaption().DChunk().GetSize( iKernelHeap.Chunk() );
            TRACE_KH( Kern::Printf("DMemSpyDriverLogChanHeapInfo::GetHeapDataKernelInit() - user side buffer needs to be: %d", r) );
            }
        else
            {
            // Error scenario - must close heap
            iKernelHeap.Close();
            }
        }

    NKern::ThreadLeaveCS();

    TRACE_KH( Kern::Printf("DMemSpyDriverLogChanHeapData::GetHeapDataKernelInit() - END - ret: %d", r));
    return r;
    }


TInt DMemSpyDriverLogChanHeapData::GetHeapDataKernelFetch( TDes8* aSink )
    {
	//TOMSCI TODO this function is fundamentally flawed
	return KErrNotSupported;
	/*
    TRACE_KH( Kern::Printf("DMemSpyDriverLogChanHeapData::GetHeapDataKernelFetch() - START"));

    NKern::ThreadEnterCS();

    // We should already have an initialised copy of the kernel heap
    const TBool isInit = iKernelHeap.Helper() != NULL;
    TRACE_KH( Kern::Printf("DMemSpyDriverLogChanHeapData::GetHeapDataKernelFetch() - isInit: %d", isInit ));
    __ASSERT_ALWAYS( isInit, MemSpyDriverUtils::PanicThread( ClientThread(), EPanicKernelHeapDataFetchError ) );

    // Get user side (MemSpy) descriptor length info
    TInt destLen;
    TInt destMax;
    TUint8* destPtr = NULL;
    TInt r = Kern::ThreadGetDesInfo( &ClientThread(), aSink, destLen, destMax, destPtr, ETrue );
    TRACE_KH( Kern::Printf("DMemSpyDriverLogChanHeapData::GetHeapDataKernelFetch - user side descriptor: 0x%08x (0x%08x), len: %8d, maxLen: %8d, r: %d", aSink, destPtr, destLen, destMax, r ));

    if  ( r == KErrNone )
        {
        // Calculate start of real heap data (skipping over embedded RHeap object)
        const TUint8* startOfHeapOffset = iKernelHeap.Base();
        TRACE_KH( Kern::Printf("DMemSpyDriverLogChanHeapData::GetHeapDataKernelFetch - startOfHeapOffset:    0x%08x", startOfHeapOffset));
        const TUint heapSize = iKernelHeap.Size();
        TRACE_KH( Kern::Printf("DMemSpyDriverLogChanHeapData::GetHeapDataKernelFetch - heapSize:               %8d", heapSize));

        if ( destMax >= heapSize )
            {
            }
        else
            {
            // Not enough space
            r = KErrOverflow;
            TRACE_KH( Kern::Printf("DMemSpyDriverLogChanHeapData::GetHeapDataKernelFetch - not enough space in client descriptor" ));
            }

        // The remaining number of bytes should allow us to calculate the position
        // to read from.
        const TInt amountToRead = Min( heapSize, destMax );
        const TAny* readAddress = startOfHeapOffset;
        TRACE_KH( Kern::Printf("DMemSpyDriverLogChanHeapData::GetHeapDataKernelFetch - amountToRead:         %d", amountToRead));
        TRACE_KH( Kern::Printf("DMemSpyDriverLogChanHeapData::GetHeapDataKernelFetch - readAddress:          0x%08x", readAddress));
        const TPtrC8 pKernelHeapData( (const TUint8*) readAddress, amountToRead );

        // Copy kernel heap data to MemSpy
        TRACE_KH( Kern::Printf("DMemSpyDriverLogChanHeapData::GetHeapDataKernelFetch - about to do write to user-space..."));
        r = Kern::ThreadDesWrite( &ClientThread(), aSink, pKernelHeapData, 0, KChunkShiftBy0 | KTruncateToMaxLength, &ClientThread() );
        TRACE_KH( Kern::Printf("DMemSpyDriverLogChanHeapData::GetHeapDataKernelFetch - write result: %d", r));
        }

    // Free heap resource
    iKernelHeap.DisassociateWithKernelChunk();

    NKern::ThreadLeaveCS();

	TRACE_KH( Kern::Printf("DMemSpyDriverLogChanHeapData::GetHeapDataKernelFetch() - END - ret: %d", r));
    return r;
	*/
    }

















const TInt KPageSize = 4096;

TInt DMemSpyDriverLogChanHeapData::GetHeapDataUser( TMemSpyDriverInternalHeapDataParams& aParams )
    {
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapData::GetHeapDataUser() - START") );

	TInt r = OpenTempObject( aParams.iTid, EThread );
	if  ( r != KErrNone )
		{
    	Kern::Printf("DMemSpyDriverLogChanHeapData::GetHeapDataUser() - END - thread not found");
		return r;
		}
    else 
        {
        const DMemSpyDriverLogChanHeapBase::TDrmMatchType drmMatchType = IsDrmThread( TempObjectAsThread() );

        if  ( drmMatchType != DMemSpyDriverLogChanHeapBase::EMatchTypeNone )
            {
            // Check whether it's a DRM thread...
            DThread* thread = (DThread*) TempObject();
    	    Kern::Printf("DMemSpyDriverLogChanHeapData::GetHeapDataUser() - END - Not allowing dump of DRM heap - matchType: %d, thread: %O", drmMatchType, thread );
            CloseTempObject();
		    return KErrAccessDenied;
            }
        }

    // Check that the process' thread's are suspended
    DThread* thread = (DThread*) TempObject();
    if  ( SuspensionManager().IsSuspended( *thread ) )
        {
        // Open the heap
	    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapData::GetHeapDataUser - thread: %O", thread) );
        RMemSpyDriverRHeapUser heap( OSAdaption() );
		r = heap.OpenUserHeap(*thread, aParams.iDebugAllocator);
		TRACE( Kern::Printf("DMemSpyDriverLogChanHeapData::GetHeapDataUser - opening heap returned: %d", r) );
        if  (r == KErrNone)
            {
            if  ( aParams.iChecksum != 0 )
                {
                TRACE( Kern::Printf("DMemSpyDriverLogChanHeapData::GetHeapDataUser - checksum validation requested - expecting: 0x%08x", aParams.iChecksum ) );
                RMemSpyDriverHeapWalker heapWalker(heap);
                
                TRACE( Kern::Printf("DMemSpyDriverLogChanHeapData::GetHeapDataUser - starting traversal..." ));
#if ( defined( TRACE_TYPE_USERHEAP ) && defined( TRACE_TYPE_HEAPWALK ) )
                heapWalker.SetPrintDebug();
#endif
                r = heapWalker.Traverse();
                const TUint32 calculatedChecksum = heapWalker.Stats().iFreeCellCRC;
                TRACE( Kern::Printf("DMemSpyDriverLogChanHeapData::GetHeapDataUser - finished traversal - err: %d, checksum: 0x%08x", r, calculatedChecksum ));

                TRACE( Kern::Printf("DMemSpyDriverLogChanHeapData::GetHeapDataUser - comparing CALCULATED: 0x%08x vs EXPECTED: 0x%08x", calculatedChecksum, aParams.iChecksum ));
                if  ( calculatedChecksum != aParams.iChecksum )
                    {
                    Kern::Printf("DMemSpyDriverLogChanHeapData::GetHeapDataUser - comparing CALCULATED: 0x%08x vs EXPECTED: 0x%08x for thread %O", calculatedChecksum, aParams.iChecksum, thread );
                    r = KErrCorrupt;
                    }
                }

            // Get user side (MemSpy) descriptor length info
            if  ( r == KErrNone )
                {
                TInt destLen = 0;
                TInt destMax = 0;
                TUint8* destPtr = NULL;
                r = Kern::ThreadGetDesInfo( &ClientThread(), aParams.iDes, destLen, destMax, destPtr, ETrue );
                TRACE( Kern::Printf("DMemSpyDriverLogChanHeapData::GetHeapDataUser - user side descriptor: 0x%08x (0x%08x), len: %8d, maxLen: %8d, r: %d", aParams.iDes, destPtr, destLen, destMax, r ));
				destMax = destMax & ~(KPageSize-1); // Round down dest max to page size
				if (destMax <= 0 || (aParams.iReadAddress & (KPageSize-1))) r = KErrArgument; // If destMax is less than a page or the read address isn't a multiple of page size then we don't want to know

                if  ( r == KErrNone )
                    {
					const TLinAddr chunkBase = (TLinAddr)OSAdaption().DChunk().GetBase(heap.Chunk());
					const TLinAddr chunkMaxAddr = chunkBase + OSAdaption().DChunk().GetMaxSize(heap.Chunk());
        	        TRACE( Kern::Printf("DMemSpyDriverLogChanHeapData::GetHeapDataUser - chunkBase:    0x%08x", chunkBase) );
        
					TLinAddr readAddress = aParams.iReadAddress;
                    if (aParams.iRemaining < 0 )
                        {
                        // Initial case, start from the bottom
                        readAddress = chunkBase;
						aParams.iRemaining = heap.Helper()->CommittedSize();
                        }

                    // The remaining number of bytes should allow us to calculate the position
                    // to read from.
                    TInt amountToRead = Min( aParams.iRemaining, destMax );
        	        TRACE( Kern::Printf("DMemSpyDriverLogChanHeapData::GetHeapDataUser - amountToRead:           %8d", amountToRead) );
        
                    // Do the read from the heap we are spying on into MemSpy's address space
					// TomS: I didn't know you could do this - you live and learn
					do
						{
						r = Kern::ThreadRawRead( thread, (const void*)readAddress, destPtr, amountToRead );
        				TRACE( Kern::Printf("DMemSpyDriverLogChanHeapData::GetHeapDataUser - read result: %d", r) );

						if (r == KErrBadDescriptor)
							{
							// This is not necessarily an error - it could be we've hit an unmapped page
							if (amountToRead > KPageSize)
								{
								// retry reading a single page instead
								amountToRead = KPageSize;
								}
							else
								{
								// Try the next page
								readAddress += KPageSize;
								}
							}
						} while (r == KErrBadDescriptor && readAddress < chunkMaxAddr);
                    //
                    if  (r == KErrNone)
                        {
                        // Client takes care of updating descriptor length.
                        r = amountToRead;
                        }
        
                    // Update remaining bytes
                    aParams.iRemaining -= amountToRead;
                    aParams.iReadAddress = readAddress;
                    }
                }
			}
        else
            {
    	    Kern::Printf("DMemSpyDriverLogChanHeapData::GetHeapDataUser - couldnt open heap for thread %O, err=%d", thread, r);
            r = KErrNotSupported;
            }
		heap.Close();
        }
    else
        {
        Kern::Printf("DMemSpyDriverLogChanHeapData::GetHeapDataUser - parent process not suspended => KErrAccessDenied - thread: %O", thread );
        r = KErrAccessDenied;
        }

	CloseTempObject();

	TRACE( Kern::Printf("DMemSpyDriverLogChanHeapData::GetHeapDataUser() - END - ret: %d", r) );
    return r;
    }



