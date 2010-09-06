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

#include "MemSpyDriverLogChanHeapInfo.h"

// System includes
#include <u32hal.h>
#include <e32rom.h>
#include <memspy/driver/memspydriverconstants.h>
#include <memspy/driver/memspydriverobjectsshared.h>

// Shared includes
#include "MemSpyDriverOpCodes.h"
#include "MemSpyDriverObjectsInternal.h"

// User includes
#include "MemSpyDriverHeap.h"
#include "MemSpyDriverUtils.h"
#include "MemSpyDriverOSAdaption.h"
#include "MemSpyDriverHeapWalker.h"
#include "MemSpyDriverSuspensionManager.h"



DMemSpyDriverLogChanHeapInfo::DMemSpyDriverLogChanHeapInfo( DMemSpyDriverDevice& aDevice, DThread& aThread )
:	DMemSpyDriverLogChanHeapBase( aDevice, aThread )
    {
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::DMemSpyDriverLogChanHeapInfo() - this: 0x%08x", this ));
    }


DMemSpyDriverLogChanHeapInfo::~DMemSpyDriverLogChanHeapInfo()
	{
	TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::~DMemSpyDriverLogChanHeapInfo() - START - this: 0x%08x", this ));
	ReleaseCellList();
	TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::~DMemSpyDriverLogChanHeapInfo() - END - this: 0x%08x", this ));
	}

TInt DMemSpyDriverLogChanHeapInfo::Request( TInt aFunction, TAny* a1, TAny* a2 )
	{
	TInt r = DMemSpyDriverLogChanHeapBase::Request( aFunction, a1, a2 );
    if  ( r == KErrNone )
        {
        if  ( aFunction != EMemSpyDriverOpCodeHeapInfoFetchCellList )
            {
            ReleaseCellList();
            }
        //
        switch( aFunction )
		    {
        case EMemSpyDriverOpCodeHeapInfoGetUser:
            r = GetHeapInfoUser( (TMemSpyDriverInternalHeapRequestParameters*) a1 );
            break;
	    case EMemSpyDriverOpCodeHeapInfoGetKernel:
            r = GetHeapInfoKernel( (TMemSpyDriverInternalHeapRequestParameters*) a1, (TDes8*) a2 );
            break;
        case EMemSpyDriverOpCodeHeapInfoGetIsDebugKernel:
            r = GetIsDebugKernel(a1);
            break;
        case EMemSpyDriverOpCodeHeapInfoFetchCellList:
            r = FetchCellList( (TDes8*) a1 );
            break;

        default:
            r = KErrNotSupported;
		    break;
            }
        }
    //
    return r;
	}


TBool DMemSpyDriverLogChanHeapInfo::IsHandler( TInt aFunction ) const
    {
    return ( aFunction > EMemSpyDriverOpCodeHeapInfoBase && aFunction < EMemSpyDriverOpCodeHeapInfoEnd );
    }













TInt DMemSpyDriverLogChanHeapInfo::GetHeapInfoUser( TMemSpyDriverInternalHeapRequestParameters* aParams )
    {
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::GetHeapInfoUser() - START" ) );

    TInt r = Kern::ThreadRawRead( &ClientThread(), aParams, &iHeapInfoParams, sizeof(TMemSpyDriverInternalHeapRequestParameters) );
    if  ( r != KErrNone )
        {
    	TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::GetHeapInfoUser - params read error: %d", r));
        }
    else
        {
        TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::GetHeapInfoUser - thread id: %d, vtable: 0x%08x, debugAllocator: %d", iHeapInfoParams.iTid, iHeapInfoParams.iRHeapVTable, iHeapInfoParams.iDebugAllocator) );

	    r = OpenTempObject( iHeapInfoParams.iTid, EThread );
	    if  ( r != KErrNone )
		    {
    	    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::GetHeapInfoUser - thread not found") );
            }
        else
            {
            // Check that the process' thread's are suspended
            DThread* thread = (DThread*) TempObject();
            if  ( SuspensionManager().IsSuspended( *thread ) )
                {
                // Open client's heap
                RMemSpyDriverRHeapUser rHeap( OSAdaption() );
				r = rHeap.OpenUserHeap(*thread, iHeapInfoParams.iDebugAllocator);
                TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::GetHeapInfoUser - opening client heap returned: %d", r) );

                if  ( r == KErrNone )
                    {
                    // This object holds all of the info we will accumulate for the client.
                    TMemSpyHeapInfo masterHeapInfo;
                    masterHeapInfo.SetType(rHeap.GetTypeFromHelper());
                    masterHeapInfo.SetTid( iHeapInfoParams.iTid );
                    masterHeapInfo.SetPid( OSAdaption().DThread().GetOwningProcessId( *thread ) );

                    // This is the RHeap-specific object that contains all RHeap info
                    TMemSpyHeapInfoRHeap& rHeapInfo = masterHeapInfo.AsRHeap();


                    // We must walk the client's heap in order to build statistics
                    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::GetHeapInfoUser - calling heap walker constructor..."));
					RMemSpyDriverHeapWalker heapWalker(rHeap);
                    if  (iHeapInfoParams.iBuildFreeCellList || iHeapInfoParams.iBuildAllocCellList)
                        {
                        heapWalker.SetObserver( this );
                        TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::GetHeapInfoUser - collecting cells"));
                        }
                    else
                        {
                        TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::GetHeapInfoUser - not collecting cells"));
                        }

                    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::GetHeapInfoUser - starting traversal openerr: %d...", r));

#if ( defined( TRACE_TYPE_USERHEAP ) && defined( TRACE_TYPE_HEAPWALK ) )
                    heapWalker.SetPrintDebug();
#endif
                    if (r == KErrNone) r = heapWalker.Traverse();
                    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::GetHeapInfoUser - finished traversal - err: %d", r ));

                    TMemSpyHeapStatisticsRHeap& rHeapStats = rHeapInfo.Statistics();
                    heapWalker.CopyStatsTo( rHeapStats );

                    // Get remaining meta data that isn't stored elsewhere
                    TMemSpyHeapMetaDataRHeap& rHeapMetaData = rHeapInfo.MetaData();
					DChunk& userHeapChunk = rHeap.Chunk();
					TFullName chunkName;
					userHeapChunk.FullName(chunkName);
					rHeapMetaData.SetChunkName( chunkName );
		            rHeapMetaData.SetChunkSize( (TUint) OSAdaption().DChunk().GetSize( userHeapChunk ) );
					rHeapMetaData.SetChunkHandle( &userHeapChunk );
					rHeapMetaData.SetChunkBaseAddress( OSAdaption().DChunk().GetBase( userHeapChunk ) );
                    rHeapMetaData.SetDebugAllocator(rHeap.Helper()->AllocatorIsUdeb());
                    rHeapMetaData.SetUserThread( ETrue );
					rHeapMetaData.iHeapSize = rHeap.Helper()->CommittedSize();
					rHeapMetaData.iAllocatorAddress = (TAny*)rHeap.Helper()->AllocatorAddress();
					rHeapMetaData.iMinHeapSize = rHeap.Helper()->MinCommittedSize();
					rHeapMetaData.iMaxHeapSize = rHeap.Helper()->MaxCommittedSize();

                    PrintHeapInfo( masterHeapInfo );

                    // Write free cells if requested
                    if  ( r == KErrNone && (iHeapInfoParams.iBuildFreeCellList || iHeapInfoParams.iBuildAllocCellList))
                        {
                        r = PrepareCellListTransferBuffer();
                        }

                    // Update info ready for writing back to the user-side
                    if  ( r >= KErrNone )
                        {
                        // Write results back to user-side
                        TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::GetHeapInfoUser - writing to user-side..."));
                        TMemSpyHeapInfo* userMasterInfo = iHeapInfoParams.iMasterInfo;
                        const TInt error = Kern::ThreadRawWrite( &ClientThread(), userMasterInfo, &masterHeapInfo, sizeof(TMemSpyHeapInfo) );
                        if  ( error < 0 )
                            {
                            r = error;
                            }
                        }

                    // Release resources
					rHeap.Close();
                    }
                }
            else
                {
                r = KErrAccessDenied;
                TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::GetHeapInfoUser - parent process not suspended => KErrAccessDenied"));
                }

	        CloseTempObject();
            }
        }

	TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::GetHeapInfoUser() - END - ret: %d", r));
    return r;
    }







TInt DMemSpyDriverLogChanHeapInfo::GetHeapInfoKernel( TMemSpyDriverInternalHeapRequestParameters* aParams, TDes8* aTransferBuffer )
    {
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::GetHeapInfoKernel() - START - aTransferBuffer: 0x%08x", aTransferBuffer ) );

    TMemSpyDriverInternalHeapRequestParameters params;
    TInt r = Kern::ThreadRawRead( &ClientThread(), aParams, &params, sizeof(TMemSpyDriverInternalHeapRequestParameters) );
    if  ( r == KErrNone )
        {
        // Open kernel heap
        RMemSpyDriverRHeapKernelInPlace rHeap;
        r = rHeap.OpenKernelHeap();
        TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::GetHeapInfoKernel() - open err: %d", r ) );

        if  ( r == KErrNone )
            {
            r = DMemSpyDriverLogChanHeapBase::GetHeapInfoKernel(rHeap, params.iMasterInfo, aTransferBuffer);
            TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::GetHeapInfoKernel() - base class get heap info: %d", r) );
            }
        else 
            {
            TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::GetHeapInfoKernel() - open err: %d", r ) );
            }
        }
    else
        {
        TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::GetHeapInfoKernel() - params read error: %d", r) );
        }

	TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::GetHeapInfoKernel() - END - ret: %d", r) );
    return r;
    }






TInt DMemSpyDriverLogChanHeapInfo::GetIsDebugKernel(TAny* aResult)
    {
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::GetIsDebugKernel() - START") );
    
    TInt r = KErrNone;
    TBool debugKernel = EFalse;

    NKern::ThreadEnterCS();
    
    RMemSpyDriverRHeapKernelInPlace rHeap;
    r = rHeap.OpenKernelHeap();
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::GetIsDebugKernel() - open kernel heap returned: %d", r) );

    if  ( r == KErrNone )
        {
        debugKernel = rHeap.Helper()->AllocatorIsUdeb();

        // Tidy up
        rHeap.Close();
        }

    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::GetIsDebugKernel() - debugKernel: %d", debugKernel) );

    // Write back to user-land
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::GetIsDebugKernel() - writing to user-side...") );
    r = Kern::ThreadRawWrite( &ClientThread(), aResult, &debugKernel, sizeof(TBool) );

    NKern::ThreadLeaveCS();

 	TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::GetIsDebugKernel() - END - ret: %d", r) );
    return r;
    }

TInt DMemSpyDriverLogChanHeapInfo::PrepareCellListTransferBuffer()
    {
    // Transfer free cells immediately from xfer stream
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::PrepareCellListTransferBuffer() - START - iHeapStream: 0x%08x", iHeapStream ));
    __ASSERT_ALWAYS( !iHeapStream, MemSpyDriverUtils::PanicThread( ClientThread(), EPanicHeapFreeCellStreamNotClosed ) );
    //
    TInt r = KErrNoMemory;
    //
    NKern::ThreadEnterCS();
    //
    iHeapStream = new RMemSpyMemStreamWriter();
    if  ( iHeapStream )
        {
        const TInt requiredMemory = CalculateCellListBufferSize();
        r = OpenXferStream( *iHeapStream, requiredMemory );
        TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::PrepareCellListTransferBuffer() - requested %d bytes for free cell list, r: %d", requiredMemory, r ));

        if  ( r == KErrNone )
            {
            const TInt count = iCellList.Count();
            TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::PrepareCellListTransferBuffer() - cell count: %d", count ));
            //
            iHeapStream->WriteInt32( count );
            for( TInt i=0; i<count; i++ )
                {
                const TMemSpyDriverCell& cell = iCellList[ i ];
                TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::PrepareCellListTransferBuffer() - storing entry: %d", i ));
                //
                iHeapStream->WriteInt32( cell.iType );
                iHeapStream->WriteUint32( reinterpret_cast<TUint32>( cell.iAddress ) );
                iHeapStream->WriteInt32( cell.iLength );
                }

            // Finished with the array now
            iCellList.Reset();

            // We return the amount of client-side memory that needs to be allocated to hold the buffer
            r = requiredMemory;
            }
        }
    //
    NKern::ThreadLeaveCS();
               
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::PrepareCellListTransferBuffer() - END - r: %d", r));
	return r;
    }


TInt DMemSpyDriverLogChanHeapInfo::FetchCellList( TDes8* aBufferSink )
    {
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::FetchCellList() - START - iHeapStream: 0x%08x", iHeapStream ));
    __ASSERT_ALWAYS( iHeapStream, MemSpyDriverUtils::PanicThread( ClientThread(), EPanicHeapFreeCellStreamNotOpen ) );

    TInt r = KErrNone;

    // Write buffer to client
    NKern::ThreadEnterCS();
    r = iHeapStream->WriteAndClose( aBufferSink );

    // Tidy up
    ReleaseCellList();

    NKern::ThreadLeaveCS();
    //
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::FetchCellList() - END - r: %d", r));
	return r;
    }



TInt DMemSpyDriverLogChanHeapInfo::CalculateCellListBufferSize() const
    {
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::CalculateCellListBufferSize() - START" ));

    const TInt count = iCellList.Count();
    const TInt entrySize = sizeof( TInt32 ) + sizeof( TInt32 ) + sizeof( TUint32 );
    const TInt r = ( count * entrySize ) + sizeof( TInt ); // Extra TInt to hold count
                
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::CalculateCellListBufferSize() - END - r: %d, count: %d, entrySize: %d", r, count, entrySize ));
	return r;
    }



void DMemSpyDriverLogChanHeapInfo::ReleaseCellList()
    {
	TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::ReleaseCellList() - START - this: 0x%08x", this ));

    NKern::ThreadEnterCS();
    iCellList.Reset();
    delete iHeapStream;
    iHeapStream = NULL;
    NKern::ThreadLeaveCS();

    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::ReleaseCellList() - END - this: 0x%08x", this ));
    }

TBool DMemSpyDriverLogChanHeapInfo::HandleHeapCell(TMemSpyDriverCellType aCellType, TAny* aCellAddress, TInt aLength, TInt /*aNestingLevel*/, TInt /*aAllocNumber*/)
    {
	TInt err = KErrNone;
    if (((aCellType & EMemSpyDriverFreeCellMask) && iHeapInfoParams.iBuildFreeCellList) || 
        ((aCellType & EMemSpyDriverAllocatedCellMask) && iHeapInfoParams.iBuildAllocCellList))
		{
		TMemSpyDriverCell cell;
		cell.iType = aCellType;
		cell.iAddress = aCellAddress;
		cell.iLength = aLength;

		NKern::ThreadEnterCS();
		err = iCellList.Append(cell);
		NKern::ThreadLeaveCS();
		}
	return err == KErrNone;
	}

void DMemSpyDriverLogChanHeapInfo::HandleHeapWalkInit()
	{
	}
