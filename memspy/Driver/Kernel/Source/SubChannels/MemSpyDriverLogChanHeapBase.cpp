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

#include "MemSpyDriverLogChanHeapBase.h"

// System includes
#include <u32hal.h>
#include <e32rom.h>
#include <memspy/driver/memspydriverobjectsshared.h>

// Shared includes
#include "MemSpyDriverOpCodes.h"
#include "MemSpyDriverObjectsInternal.h"

// User includes
#include "MemSpyDriverHeap.h"
#include "MemSpyDriverUtils.h"
#include "MemSpyDriverDevice.h"
#include "MemSpyDriverOSAdaption.h"

// Constants
const TInt KMemSpyDriverLogChanHeapBaseXferBufferSize = 1024 * 16;




DMemSpyDriverLogChanHeapBase::DMemSpyDriverLogChanHeapBase( DMemSpyDriverDevice& aDevice, DThread& aThread )
:	DMemSpyDriverLogChanBase( aDevice, aThread )
    {
	TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::DMemSpyDriverLogChanHeapBase() - this: 0x%08x", this ));
    }


DMemSpyDriverLogChanHeapBase::~DMemSpyDriverLogChanHeapBase()
	{
	}


TInt DMemSpyDriverLogChanHeapBase::Construct()
	{
	TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::Construct() - START - this: 0x%08x", this ));
    
    const TInt ret = BaseConstruct( KMemSpyDriverLogChanHeapBaseXferBufferSize );

	TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::Construct() - END - this: 0x%08x, err: %d", this, ret ));
    return ret;
	}











TInt DMemSpyDriverLogChanHeapBase::Request( TInt aFunction, TAny* a1, TAny* a2 )
	{
	const TInt r = DMemSpyDriverLogChanBase::Request( aFunction, a1, a2 );
    return r;
	}













DMemSpyDriverLogChanHeapBase::TDrmMatchType DMemSpyDriverLogChanHeapBase::IsDrmThread( DThread& aThread )
    {
    TDrmMatchType ret = EMatchTypeNone;
    //
    const TUid procUid = aThread.iOwningProcess->iUids.iUid[ 2 ];
    TRACE( Kern::Printf( "DMemSpyDriverLogChanHeapBase::IsDrmThread() - START - aThread: %O, process uid: 0x%08x", &aThread, procUid.iUid ));

    // Some more rudimentary checks based upon process name and
    // known uids.
    TFullName fullName;
    aThread.FullName( fullName );

    // Exclude threads containing "DRM"
    _LIT( KDrmThreadMatchText, "*DRM*" );
    const TInt matchPos = fullName.MatchF( KDrmThreadMatchText );
    if ( matchPos >= 0 )
        {
        TRACE( Kern::Printf( "DMemSpyDriverLogChanHeapBase::IsDrmThread() - found \'DRM\' at pos: %d (%S)", matchPos, &fullName ));
        ret = EMatchTypeName;
        }
    else
        {
        // Some known DRM related process UIDs
        switch( procUid.iUid )
            {
        case 0x10005A22: // DRMEncryptor.exe
        case 0x01105901: // DRMEncryptor.exe
        case 0x101F85C7: // DRMRightsManager.exe
        case 0x10205CA8: // DcfRepSrv.exe
        case 0x101F51F2: // RightsServer.exe
        case 0x101F6DC5: // DRMHelperServer.exe
        case 0x10282F1B: // wmdrmserver.exe
            ret = EMatchTypeUid;
            break;
        default:
            ret = EMatchTypeNone;
            break;
            }
        }

    TRACE( Kern::Printf( "DMemSpyDriverLogChanHeapBase::IsDrmThread() - END - procUid: 0x%08x, matchType: %d", procUid.iUid, ret ));
    return ret;
    }





































void DMemSpyDriverLogChanHeapBase::PrintHeapInfo( const TMemSpyHeapInfo& aInfo )
    {
    const TMemSpyHeapInfoRHeap& rHeapInfo = aInfo.AsRHeap();
    //const TMemSpyHeapObjectDataRHeap& rHeapObjectData = rHeapInfo.ObjectData();
    const TMemSpyHeapStatisticsRHeap& rHeapStats = rHeapInfo.Statistics();
    const TMemSpyHeapMetaDataRHeap& rHeapMetaData = rHeapInfo.MetaData();

    /*
	TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() ---------------------------------------------------" ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - RAllocator                                      -" ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() ---------------------------------------------------" ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - RAllocator::iAccessCount:       %d", rHeapObjectData.iAccessCount ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - RAllocator::iHandleCount:       %d", rHeapObjectData.iHandleCount ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - RAllocator::iHandles:           0x%08x", rHeapObjectData.iHandles ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - RAllocator::iFlags:             0x%08x", rHeapObjectData.iFlags ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - RAllocator::iCellCount:         %d", rHeapObjectData.iCellCount ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - RAllocator::iTotalAllocSize:    %d", rHeapObjectData.iTotalAllocSize ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - " ) );

    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() ---------------------------------------------------" ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - RHeap                                           -" ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() ---------------------------------------------------" ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - RHeap::iMinLength:              %d", rHeapObjectData.iMinLength ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - RHeap::iMaxLength:              %d", rHeapObjectData.iMaxLength ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - RHeap::iOffset:                 %d", rHeapObjectData.iOffset ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - RHeap::iGrowBy:                 %d", rHeapObjectData.iGrowBy ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - RHeap::iChunkHandle:            0x%08x", rHeapObjectData.iChunkHandle ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - RHeap::iBase:                   0x%08x", rHeapObjectData.iBase ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - RHeap::iTop:                    0x%08x", rHeapObjectData.iTop ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - RHeap::iAlign:                  %d", rHeapObjectData.iAlign ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - RHeap::iMinCell:                %d", rHeapObjectData.iAlign ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - RHeap::iPageSize:               %d", rHeapObjectData.iAlign ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - RHeap::iFree.next:              0x%08x", rHeapObjectData.iFree.next ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - RHeap::iFree.len:               %d", rHeapObjectData.iFree.len ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - RHeap::iNestingLevel:           %d", rHeapObjectData.iNestingLevel ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - RHeap::iAllocCount:             %d", rHeapObjectData.iAllocCount ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - RHeap::iFailType:               %d", rHeapObjectData.iFailType ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - RHeap::iFailRate:               %d", rHeapObjectData.iFailRate ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - RHeap::iFailed:                 %d", rHeapObjectData.iFailed ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - RHeap::iFailAllocCount:         %d", rHeapObjectData.iFailAllocCount ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - RHeap::iRand:                   %d", rHeapObjectData.iRand ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - RHeap::iTestData:               0x%08x", rHeapObjectData.iTestData ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - " ) );
	*/
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() ---------------------------------------------------" ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - Stats (Free)                                    -" ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() ---------------------------------------------------" ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - cell count:                     %d", rHeapStats.StatsFree().TypeCount() ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - cell size:                      %d", rHeapStats.StatsFree().TypeSize() ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - cell largest:                   0x%08x", rHeapStats.StatsFree().LargestCellAddress() ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - cell largest size:              %d", rHeapStats.StatsFree().LargestCellSize() ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - slack:                          0x%08x", rHeapStats.StatsFree().SlackSpaceCellAddress() ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - slack size:                     %d", rHeapStats.StatsFree().SlackSpaceCellSize() ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - checksum:                       0x%08x", rHeapStats.StatsFree().Checksum() ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - " ) );

    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() ---------------------------------------------------" ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - Stats (Alloc)                                   -" ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() ---------------------------------------------------" ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - cell count:                     %d", rHeapStats.StatsAllocated().TypeCount() ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - cell size:                      %d", rHeapStats.StatsAllocated().TypeSize() ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - cell largest:                   0x%08x", rHeapStats.StatsAllocated().LargestCellAddress() ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - cell largest size:              %d", rHeapStats.StatsAllocated().LargestCellSize() ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - " ) );

    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() ---------------------------------------------------" ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - Misc. Info                                      -" ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() ---------------------------------------------------" ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - chunk size:                     %d", rHeapMetaData.ChunkSize() ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - chunk handle:                   0x%08x", rHeapMetaData.ChunkHandle() ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - chunk base address:             0x%08x", rHeapMetaData.ChunkBaseAddress() ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - debug allocator:                %d", rHeapMetaData.IsDebugAllocator() ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - shared heap:                    %d", rHeapMetaData.IsSharedHeap() ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - user thread:                    %d", rHeapMetaData.IsUserThread() ) );
    //TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - cell header size (free):        %d", rHeapMetaData.HeaderSizeFree() ) );
    //TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - cell header size (alloc):       %d", rHeapMetaData.HeaderSizeAllocated() ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - heap vTable:                    0x%08x", rHeapMetaData.VTable() ) );
    //TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - heap object size:               %d", rHeapMetaData.ClassSize() ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - heap size:                      %d", rHeapMetaData.iHeapSize ) );
    TRACE( Kern::Printf("DMemSpyDriverLogChanHeapBase::PrintHeapInfo() - allocator address:              0x%08x", rHeapMetaData.iAllocatorAddress ) );
    }

TInt DMemSpyDriverLogChanHeapBase::GetHeapInfoKernel(RMemSpyDriverRHeapBase& aHeap, TMemSpyHeapInfo* aHeapInfo, TDes8* aTransferBuffer )
    {
    TRACE_KH( Kern::Printf("DMemSpyDriverLogChanHeapBase::GetHeapInfoKernel() - START - aTransferBuffer: 0x%08x", aTransferBuffer ) );

    TInt r = KErrNone;
    NKern::ThreadEnterCS();

    // This object holds all of the info we will accumulate for the client.
    TMemSpyHeapInfo masterHeapInfo;
    masterHeapInfo.SetType(aHeap.GetTypeFromHelper());
    masterHeapInfo.SetTid( 2 );
    masterHeapInfo.SetPid( 1 );

    // This is the RHeap-specific object that contains all RHeap info
    TMemSpyHeapInfoRHeap& rHeapInfo = masterHeapInfo.AsRHeap();


    // When walking the kernel heap we must keep track of the free cells
    // without allocating any more memory (on the kernel heap...)
    //
    // Therefore, we start a stream immediately, which is actually already
    // pre-allocated.
    //
    // Start stream and pad with zero count, which we'll repopulate later on
    // once we know the final score.
    RMemSpyMemStreamWriter stream;
    TInt32* pCount = NULL;

    // We must walk the client's heap in order to build statistics
    TRACE_KH( Kern::Printf("DMemSpyDriverLogChanHeapBase::GetHeapInfoKernel - calling heap walker constructor..."));
    RMemSpyDriverHeapWalker heapWalker(aHeap);
    if  ( aTransferBuffer )
        {
        // This will allow us to identify that we're writing directly to the stream
        stream = OpenXferStream();
        iStackStream = &stream;

        // Writer marker value which we'll update after the traversal completes
        pCount = stream.WriteInt32( 0 );

        // Receive cell info as we walk the heap...
        heapWalker.SetObserver( this );
        TRACE_KH( Kern::Printf("DMemSpyDriverLogChanHeapBase::GetHeapInfoKernel - collecting free cells - iStackStream: 0x%08x, isOpen: %d, pCount: 0x%08x", iStackStream, stream.IsOpen(), pCount ));
        }
    else
        {
        iStackStream = NULL;
        TRACE_KH( Kern::Printf("DMemSpyDriverLogChanHeapBase::GetHeapInfoKernel - not collecting free cells"));
        }

    TRACE_KH( Kern::Printf("DMemSpyDriverLogChanHeapBase::GetHeapInfoKernel - locking system..." ));
    NKern::LockSystem();
    TRACE_KH( Kern::Printf("DMemSpyDriverLogChanHeapBase::GetHeapInfoKernel - disabling interrupts..." ));
    const TInt irq = NKern::DisableAllInterrupts();
    TRACE_KH( Kern::Printf("DMemSpyDriverLogChanHeapBase::GetHeapInfoKernel - starting traversal..." ));

#if defined( TRACE_TYPE_KERNELHEAP )
    heapWalker.SetPrintDebug();
#endif
    r = heapWalker.Traverse();

    TRACE_KH( Kern::Printf("DMemSpyDriverLogChanHeapBase::GetHeapInfoKernel - restoring interrupts..." ));
    NKern::RestoreInterrupts( irq );
    TRACE_KH( Kern::Printf("DMemSpyDriverLogChanHeapBase::GetHeapInfoKernel - finished traversal - err: %d, iStackStream: 0x%08x, pCount: 0x%08x, isOpen: %d", r, iStackStream, pCount, ( iStackStream == NULL ? 0 : iStackStream->IsOpen() ) ));
    NKern::UnlockSystem();
    TRACE_KH( Kern::Printf("DMemSpyDriverLogChanHeapBase::GetHeapInfoKernel - unlocked system" ));

    // Write free cells if requested
    if  ( r == KErrNone && iStackStream && iStackStream->IsOpen() && pCount )
        {
        TRACE_KH( Kern::Printf("DMemSpyDriverLogChanHeapBase::GetHeapInfoKernel - final free cell count: %d", iFreeCellCount ));
        *pCount = iFreeCellCount;
        r = stream.WriteAndClose( aTransferBuffer );
        iStackStream = NULL;
        TRACE_KH( Kern::Printf("DMemSpyDriverLogChanHeapBase::GetHeapInfoKernel - stream commit result: %d", r ));
        }

    TMemSpyHeapStatisticsRHeap& rHeapStats = rHeapInfo.Statistics();
    heapWalker.CopyStatsTo( rHeapStats );

    // Get remaining meta data that isn't stored elsewhere
    TMemSpyHeapMetaDataRHeap& rHeapMetaData = rHeapInfo.MetaData();
	TFullName chunkName;
	aHeap.Chunk().FullName(chunkName);
    rHeapMetaData.SetChunkName(chunkName);
    rHeapMetaData.SetChunkSize( (TUint) aHeap.Chunk().Size() );
    rHeapMetaData.SetChunkHandle( &aHeap.Chunk() );
    rHeapMetaData.SetChunkBaseAddress( OSAdaption().DChunk().GetBase(aHeap.Chunk()) );
    rHeapMetaData.SetDebugAllocator(aHeap.Helper()->AllocatorIsUdeb());
    rHeapMetaData.SetUserThread( EFalse );
    rHeapMetaData.SetSharedHeap( ETrue );
	rHeapMetaData.iHeapSize = aHeap.Helper()->CommittedSize();
	rHeapMetaData.iAllocatorAddress = (TAny*)aHeap.Helper()->AllocatorAddress();
	rHeapMetaData.iMinHeapSize = aHeap.Helper()->MinCommittedSize();
	rHeapMetaData.iMaxHeapSize = aHeap.Helper()->MaxCommittedSize();

    PrintHeapInfo( masterHeapInfo );

    // Update info ready for writing back to the user-side
    if  ( r == KErrNone )
        {
        // Write results back to user-side
        TRACE_KH( Kern::Printf("DMemSpyDriverLogChanHeapBase::GetHeapInfoKernel - writing to user-side..."));
        r = Kern::ThreadRawWrite( &ClientThread(), aHeapInfo, &masterHeapInfo, sizeof( TMemSpyHeapInfo ) );
        }

    NKern::ThreadLeaveCS();

	TRACE_KH( Kern::Printf("DMemSpyDriverLogChanHeapBase::GetHeapInfoKernel() - END - ret: %d", r) );
    return r;
    }










    






TBool DMemSpyDriverLogChanHeapBase::HandleHeapCell(TMemSpyDriverCellType aCellType, TAny* aCellAddress, TInt aLength, TInt /*aNestingLevel*/, TInt /*aAllocNumber*/)
    {
    TInt error = KErrNone;
    //
    if  (aCellType & EMemSpyDriverFreeCellMask)
        {
        if  ( iStackStream )
            {
            if  ( !iStackStream->IsFull() )
                {
                ++iFreeCellCount;
                TRACE_KH( Kern::Printf("DMemSpyDriverLogChanHeapBase::HandleHeapCell - writing free cell %d @ 0x%08x, space left: %u", iFreeCellCount, aCellAddress, iStackStream->Remaining() ));
                //
                iStackStream->WriteInt32( aCellType );
                iStackStream->WriteUint32( reinterpret_cast<TUint32>( aCellAddress ) );
                iStackStream->WriteInt32( aLength );
                }
            else
                {
                Kern::Printf( "DMemSpyDriverLogChanHeapBase::HandleHeapCell - Kernel Free Cell stack stream IS FULL!" );
                error = KErrAbort;
                }
            }
       }
    //
    return ( error == KErrNone );
    }


void DMemSpyDriverLogChanHeapBase::HandleHeapWalkInit()
	{
	iFreeCellCount = 0;
	}

TInt DMemSpyDriverLogChanHeapBase::OpenKernelHeap( RHeapK*& aHeap, DChunk*& aChunk, TDes8* aClientHeapChunkName )
    {
    TRACE_KH( Kern::Printf("DMemSpyDriverLogChanHeapBase::OpenKernelHeap() - START") );

    // This is what we're searching for...
    RHeapK* kernelHeap = NULL;
    DChunk* kernelHeapChunk = NULL;

    // Find the SvHeap chunk....
    _LIT( KKernelServerHeapChunkName, "SvHeap" );
 	NKern::ThreadEnterCS();
   
    DObjectCon* chunkContainer = Kern::Containers()[EChunk];
    chunkContainer->Wait();
    NKern::LockSystem();
    const TInt chunkCount = chunkContainer->Count();

    for(TInt i=0; i<chunkCount; i++)
        {
        DChunk* chunk = (DChunk*) (*chunkContainer)[ i ];
        //
        if  ( chunk->NameBuf() )
            {
            const TInt findResult = chunk->NameBuf()->Find( KKernelServerHeapChunkName );
    	    TRACE_KH( Kern::Printf("DMemSpyDriverLogChanHeapBase::OpenKernelHeap - checking chunk: %O against %S => %d", chunk, &KKernelServerHeapChunkName, findResult ) );
            if  ( findResult != KErrNotFound )
                {
                // Found it.
                kernelHeapChunk = chunk;
                TRACE_KH( Kern::Printf( "DMemSpyDriverLogChanHeapBase::OpenKernelHeap - found chunk with base: 0x%08x", chunk->iBase ) );
                break;
                }
            }
        }

    NKern::UnlockSystem();
    chunkContainer->Signal();

    TInt r = KErrNotFound;
    if  ( kernelHeapChunk != NULL )
        {
#ifndef __WINS__
        TRACE_KH( Kern::Printf( "DMemSpyDriverLogChanHeapBase::OpenKernelHeap - kernelHeapChunk: 0x%08x", kernelHeapChunk ) );

        const TRomHeader& romHdr = Epoc::RomHeader();
	    const TRomEntry* primaryEntry = (const TRomEntry*) Kern::SuperPage().iPrimaryEntry;
        TRACE_KH( Kern::Printf( "DMemSpyDriverLogChanHeapBase::OpenKernelHeap - primaryEntry: 0x%08x, primaryEntry->iAddressLin: 0x%08x", primaryEntry, primaryEntry->iAddressLin ) );
	    const TRomImageHeader* primaryImageHeader = (const TRomImageHeader*) primaryEntry->iAddressLin;
        TRACE_KH( Kern::Printf( "DMemSpyDriverLogChanHeapBase::OpenKernelHeap - primaryEntry: 0x%08x", primaryImageHeader ) );

        TRACE_KH( Kern::Printf( "DMemSpyDriverLogChanHeapBase::OpenKernelHeap - TRomImageHeader::iCodeSize:            0x%08x", primaryImageHeader->iCodeSize ) );
        TRACE_KH( Kern::Printf( "DMemSpyDriverLogChanHeapBase::OpenKernelHeap - TRomImageHeader::iTextSize:            0x%08x", primaryImageHeader->iTextSize ) );
        TRACE_KH( Kern::Printf( "DMemSpyDriverLogChanHeapBase::OpenKernelHeap - TRomImageHeader::iDataSize:            0x%08x", primaryImageHeader->iDataSize ) );
        TRACE_KH( Kern::Printf( "DMemSpyDriverLogChanHeapBase::OpenKernelHeap - TRomImageHeader::iBssSize:             0x%08x", primaryImageHeader->iBssSize ) );
        TRACE_KH( Kern::Printf( "DMemSpyDriverLogChanHeapBase::OpenKernelHeap - TRomImageHeader::iHeapSizeMin:         0x%08x", primaryImageHeader->iHeapSizeMin ) );
        TRACE_KH( Kern::Printf( "DMemSpyDriverLogChanHeapBase::OpenKernelHeap - TRomImageHeader::iHeapSizeMax:         0x%08x", primaryImageHeader->iHeapSizeMax ) );
        TRACE_KH( Kern::Printf( "DMemSpyDriverLogChanHeapBase::OpenKernelHeap - TRomImageHeader::iStackSize:           0x%08x", primaryImageHeader->iStackSize ) );

        TRACE_KH( Kern::Printf( "DMemSpyDriverLogChanHeapBase::OpenKernelHeap - romHdr.iKernDataAddress: 0x%08x", romHdr.iKernDataAddress ) );
        TRACE_KH( Kern::Printf( "DMemSpyDriverLogChanHeapBase::OpenKernelHeap - Kern::RoundToPageSize( romHdr.iTotalSvDataSize ): 0x%08x", Kern::RoundToPageSize( romHdr.iTotalSvDataSize ) ) );
        TRACE_KH( Kern::Printf( "DMemSpyDriverLogChanHeapBase::OpenKernelHeap - Kern::RoundToPageSize( kernelProcessCreateInfo.iStackSize ): 0x%08x", Kern::RoundToPageSize( primaryImageHeader->iStackSize ) ) );

        TAny* stack = (TAny*)( romHdr.iKernDataAddress + Kern::RoundToPageSize( romHdr.iTotalSvDataSize ));
        TRACE_KH( Kern::Printf( "DMemSpyDriverLogChanHeapBase::OpenKernelHeap - aStack: 0x%08x", stack ) );
        
        // NB: This is supposed to be Kern::RoundToPageSize( kernelProcessCreateInfo.iStackSize ) but that
        // sometimes returns very dodgy values on ARMv5 Multiple Memory Model when using MemSpy's driver
        // installed via a SIS file. No idea why. Cache problem? 
        TAny* heap = (TAny*)(TLinAddr( stack ) + Kern::RoundToPageSize( primaryImageHeader->iStackSize ));
        TRACE_KH( Kern::Printf( "DMemSpyDriverLogChanHeapBase::OpenKernelHeap - aHeap: 0x%08x", heap ) );

        kernelHeap = (RHeapK*) heap;
#else
        kernelHeap = (RHeapK*) kernelHeapChunk->Base();
#endif
        // Finalise construction of heap 
        if  ( kernelHeap != NULL )
            {
            TRACE_KH( Kern::Printf( "DMemSpyDriverLogChanHeapBase::OpenKernelHeap - kernelHeapChunk->Base(): 0x%08x", kernelHeapChunk->Base() ) );
            aHeap = kernelHeap;
            aChunk = kernelHeapChunk;

            // Get the chunk name (if the caller asked for it)
            if  ( aClientHeapChunkName )
                {
                kernelHeapChunk->FullName( *aClientHeapChunkName );
                }

            // Opened okay
            r = KErrNone;
            }
        else
            {
            TRACE_KH( Kern::Printf("DMemSpyDriverLogChanHeapBase::OpenKernelHeap - kernel heap was NULL..."));
            }
        }
    else
        {
        TRACE_KH( Kern::Printf("DMemSpyDriverLogChanHeapBase::OpenKernelHeap - couldnt find kernel chunk..."));
        r = KErrNotFound;
        }

 	NKern::ThreadLeaveCS();

	TRACE_KH( Kern::Printf("DMemSpyDriverLogChanHeapBase::OpenKernelHeap() - END - ret: %d", r ) );
    return r;
    }

TInt DMemSpyDriverLogChanHeapBase::OpenKernelHeap( RMemSpyDriverRHeapKernelFromCopy& aHeap, TDes8* aClientHeapChunkName )
    {
    TRACE_KH( Kern::Printf("DMemSpyDriverLogChanHeapBase::OpenKernelHeap(CP) - START") );

    RHeapK* heap = NULL;
    DChunk* chunk = NULL;
    TInt r = OpenKernelHeap( heap, chunk, aClientHeapChunkName );
	
    TRACE_KH( Kern::Printf("DMemSpyDriverLogChanHeapBase::OpenKernelHeap(CP) - open err: %d", r ) );
    if  ( r == KErrNone )
        {
#ifdef __SYMBIAN_KERNEL_HYBRID_HEAP__
		// RAllocator::Size() not exported on hybrid heap
		const TInt heapSize = heap->DebugFunction(RAllocator::EGetSize);
#else
        const TInt heapSize = heap->Size();
#endif
        TRACE_KH( Kern::Printf("DMemSpyDriverLogChanHeapBase::OpenKernelHeap(CP) - heapSize: %d, heap: 0x%08x, chunkBase: 0x%08x", heapSize, heap, chunk->Base() ) );

        // Make a new chunk that we can copy the kernel heap into. We cannot lock the system the entire time
        // we need to do this, therefore there is no guarantee that the chunk will be large enough to hold the
        // (current) heap data at the time we need to make the copy. We oversize the chunk by 1mb in the "hope"
        // that it will be enough... :(
        TChunkCreateInfo info;
        info.iType         = TChunkCreateInfo::ESharedKernelSingle;
        info.iMaxSize      = heapSize + ( 1024 * 1024 );
        info.iOwnsMemory   = ETrue; // Use memory from system's free pool
        info.iDestroyedDfc = NULL;
        #ifdef __EPOC32__
        info.iMapAttr      = (TInt)EMapAttrFullyBlocking; // Full caching
        #endif

        // Holds a copy of the client's heap chunk
        DChunk* heapCopyChunk;
        TLinAddr heapCopyChunkAddress;
        TUint32 heapCopyChunkMappingAttributes;
        r = Kern::ChunkCreate( info, heapCopyChunk, heapCopyChunkAddress, heapCopyChunkMappingAttributes );
        TRACE_KH( Kern::Printf("DMemSpyDriverLogChanHeapBase::OpenKernelHeap(CP) - creating chunk returned: %d", r));

        if  ( r == KErrNone )
            {
            TRACE_KH( Kern::Printf("DMemSpyDriverLogChanHeapBase::OpenKernelHeap(CP) - copy chunk base: 0x%08x, heapCopyChunkAddress: 0x%08x", heapCopyChunk->iBase, heapCopyChunkAddress));

            // Commit memory for entire buffer
            TUint32 physicalAddress = 0;
            r = Kern::ChunkCommitContiguous( heapCopyChunk, 0, heapSize, physicalAddress );
            TRACE_KH( Kern::Printf("DMemSpyDriverLogChanHeapBase::OpenKernelHeap(CP) - commiting chunk returned: %d", r));

            if  ( r != KErrNone)
                {
                // On error, throw away the chunk we have created
                Kern::ChunkClose( heapCopyChunk );
                heapCopyChunk = NULL;
                }
            else
                {
                TRACE_KH( Kern::Printf("DMemSpyDriverLogChanHeapBase::OpenKernelHeap(CP) - heapCopyChunk->iSize: 0x%08x, heapCopyChunk->iBase: 0x%08x, heapCopyChunkAddress: 0x%08x, physicalAddress: 0x%08x", heapCopyChunk->iSize, heapCopyChunk->iBase, heapCopyChunkAddress, physicalAddress));

                NKern::LockSystem();
                const TUint32 copyLength = heapSize; // TODO Min( heap->Size(), heapSize );

                TRACE_KH( Kern::Printf("DMemSpyDriverLogChanHeapBase::OpenKernelHeap(CP) - trying to copy %d (vs orig estimate of %d) bytes from kernel allocator address: 0x%08x", copyLength, heapSize, heap));
                memcpy( (TUint8*) heapCopyChunkAddress, heap, copyLength );

                NKern::UnlockSystem();

                TRACE_KH( Kern::Printf("DMemSpyDriverLogChanHeapBase::OpenKernelHeap(CP) - copied kernel heap data" ));

                // Transfer ownership of the copy heap chunk to the heap object. This also calculates the delta
                // beween the heap addresses in the client's address space and the kernel address space.
                TRACE_KH( Kern::Printf("DMemSpyDriverLogChanHeapBase::OpenKernelHeap(CP) - associate chunk and transfer ownership..." ));
                aHeap.SetKernelHeap( *heap );
                aHeap.AssociateWithKernelChunk( heapCopyChunk, heapCopyChunkAddress, heapCopyChunkMappingAttributes );
                }
            }
        else
            {
	        TRACE_KH( Kern::Printf("DMemSpyDriverLogChanHeapBase::OpenKernelHeap(CP) - copy chunk create error: %d", r ) );
            }
        }

	TRACE_KH( Kern::Printf("DMemSpyDriverLogChanHeapBase::OpenKernelHeap(CP) - END - ret: %d", r ) );
    return r;
    }



