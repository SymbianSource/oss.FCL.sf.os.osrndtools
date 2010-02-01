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
	TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::~DMemSpyDriverLogChanHeapInfo() - END - this: 0x%08x", this ));
	}








TInt DMemSpyDriverLogChanHeapInfo::Request( TInt aFunction, TAny* a1, TAny* a2 )
	{
	TInt r = DMemSpyDriverLogChanHeapBase::Request( aFunction, a1, a2 );
    if  ( r == KErrNone )
        {
        if  ( aFunction != EMemSpyDriverOpCodeHeapInfoFetchFreeCells )
            {
            ReleaseFreeCells();
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
            r = GetIsDebugKernel( (TBool*) a1 );
            break;
        case EMemSpyDriverOpCodeHeapInfoFetchFreeCells:
            r = FetchFreeCells( (TDes8*) a1 );
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
                TFullName chunkName;

                // Open client's heap
                RMemSpyDriverRHeapUser rHeap( OSAdaption() );
                DChunk* userHeapChunk = NULL;
                r = OpenUserHeap( *thread, iHeapInfoParams.iRHeapVTable, rHeap, userHeapChunk, &chunkName );
                TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::GetHeapInfoUser - opening client heap returned: %d", r) );

                if  ( r == KErrNone )
                    {
                    // This object holds all of the info we will accumulate for the client.
                    TMemSpyHeapInfo masterHeapInfo;
                    masterHeapInfo.SetType( TMemSpyHeapInfo::ETypeRHeap );
                    masterHeapInfo.SetTid( iHeapInfoParams.iTid );
                    masterHeapInfo.SetPid( OSAdaption().DThread().GetOwningProcessId( *thread ) );

                    // This is the RHeap-specific object that contains all RHeap info
                    TMemSpyHeapInfoRHeap& rHeapInfo = masterHeapInfo.AsRHeap();

                    // This is the object data for the RHeap instance
                    TMemSpyHeapObjectDataRHeap& rHeapObjectData = rHeapInfo.ObjectData();
                    rHeap.CopyObjectDataTo( rHeapObjectData );

                    // We must walk the client's heap in order to build statistics
                    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::GetHeapInfoUser - calling heap walker constructor..."));
                    TMemSpyHeapWalkerNullObserver observer; 
                    RMemSpyDriverHeapWalker heapWalker( rHeap, iHeapInfoParams.iDebugAllocator );
                    if  ( iHeapInfoParams.iBuildFreeCellList )
                        {
                        heapWalker.SetObserver( this );
                        TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::GetHeapInfoUser - collecting free cells"));
                        }
                    else
                        {
                        heapWalker.SetObserver( &observer );
                        TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::GetHeapInfoUser - not collecting free cells"));
                        }

                    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::GetHeapInfoUser - starting traversal..." ));

#if ( defined( TRACE_TYPE_USERHEAP ) && defined( TRACE_TYPE_HEAPWALK ) )
                    heapWalker.SetPrintDebug();
#endif
                    r = heapWalker.Traverse();
                    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::GetHeapInfoUser - finished traversal - err: %d", r ));

                    TMemSpyHeapStatisticsRHeap& rHeapStats = rHeapInfo.Statistics();
                    heapWalker.CopyStatsTo( rHeapStats );

                    // Get remaining meta data that isn't stored elsewhere
                    TMemSpyHeapMetaDataRHeap& rHeapMetaData = rHeapInfo.MetaData();
                    rHeapMetaData.SetChunkName( chunkName );
                    rHeapMetaData.SetChunkSize( (TUint) OSAdaption().DChunk().GetSize( *userHeapChunk ) );
                    rHeapMetaData.SetChunkHandle( userHeapChunk );
                    rHeapMetaData.SetChunkBaseAddress( OSAdaption().DChunk().GetBase( *userHeapChunk ) );
                    rHeapMetaData.SetDebugAllocator( iHeapInfoParams.iDebugAllocator );
                    rHeapMetaData.SetHeaderSizeFree( RMemSpyDriverRHeapBase::FreeCellHeaderSize() );
                    rHeapMetaData.SetHeaderSizeAllocated( RMemSpyDriverRHeapBase::AllocatedCellHeaderSize( iHeapInfoParams.iDebugAllocator ) );
                    rHeapMetaData.SetUserThread( ETrue );

                    // Get any heap-specific info
                    rHeap.GetHeapSpecificInfo( masterHeapInfo );

                    PrintHeapInfo( masterHeapInfo );

                    // Write free cells if requested
                    if  ( r == KErrNone && iHeapInfoParams.iBuildFreeCellList )
                        {
                        r = PrepareFreeCellTransferBuffer();
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
                    rHeap.DisassociateWithKernelChunk();
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
        TFullName heapChunkName;
        RMemSpyDriverRHeapKernelInPlace rHeap;
        r = OpenKernelHeap( rHeap, &heapChunkName );
        TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::GetHeapInfoKernel() - open err: %d", r ) );

        if  ( r == KErrNone )
            {
            // We must identify if we have a debug kernel allocator
            const TBool debugAllocator = IsDebugKernel( rHeap );
            TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::GetHeapInfoKernel() - debugAllocator: %d", debugAllocator ) );

            r = DMemSpyDriverLogChanHeapBase::GetHeapInfoKernel( rHeap, debugAllocator, heapChunkName, params.iMasterInfo, aTransferBuffer );
            TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::GetHeapInfoKernel() - base class get heap info: %d", r) );
            }
        }
    else
        {
    	Kern::Printf("DMemSpyDriverLogChanHeapInfo::GetHeapInfoKernel() - params read error: %d", r);
        }

	TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::GetHeapInfoKernel() - END - ret: %d", r) );
    return r;
    }






TInt DMemSpyDriverLogChanHeapInfo::GetIsDebugKernel( TBool* aIsDebugKernel )
    {
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::GetIsDebugKernel() - START") );
    
    TInt r = KErrNone;
    TBool debugKernel = EFalse;

    NKern::ThreadEnterCS();
    
    RMemSpyDriverRHeapKernelInPlace rHeap;
    r = OpenKernelHeap( rHeap );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::GetIsDebugKernel() - open kernel heap returned: %d", r) );

    if  ( r == KErrNone )
        {
        debugKernel = IsDebugKernel( rHeap );

        // Tidy up
        rHeap.DisassociateWithKernelChunk();
        }

    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::GetIsDebugKernel() - debugKernel: %d", debugKernel) );

    // Write back to user-land
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::GetIsDebugKernel() - writing to user-side...") );
    r = Kern::ThreadRawWrite( &ClientThread(), aIsDebugKernel, &debugKernel, sizeof(TBool) );

    NKern::ThreadLeaveCS();

 	TRACE( Kern::Printf("DMemSpyDriverLogChanHeapInfo::GetIsDebugKernel() - END - ret: %d", r) );
    return r;
    }





































