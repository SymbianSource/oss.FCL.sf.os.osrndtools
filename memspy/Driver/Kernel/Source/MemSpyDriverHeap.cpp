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

#include "MemSpyDriverHeap.h"

// System includes
#include <kern_priv.h>

// User includes
#include "MemSpyDriverOSAdaption.h"
#include "MemSpyDriverUtils.h"

// Defines
#define __NEXT_CELL(p)				((RMemSpyDriverRHeapBase::SCell*)(((TUint8*)p)+p->len))
#define __NEXT_CELL2(p,l)			((RMemSpyDriverRHeapBase::SCell*)(((TUint8*)p)+l))


RMemSpyDriverRHeapBase::RMemSpyDriverRHeapBase()
    {
    Reset();
    }


void RMemSpyDriverRHeapBase::Reset()
    {
	iAccessCount = 0;
	iHandleCount = 0;
	iHandles = NULL;
	iFlags = 0;
	iCellCount = 0;
	iTotalAllocSize = 0;
    
    iMinLength = 0;
	iMaxLength = 0;
	iOffset = 0;
	iGrowBy = 0;
	iChunkHandle = 0;
	// iLock needs no initialisation due to default ctor
	iBase = NULL;
	iTop = NULL;
	iAlign = 0;
	iMinCell = 0;
	iPageSize = 0;
	iFree.len = 0;
	iFree.next = NULL;
	iNestingLevel = 0;
	iAllocCount = 0;
    iFailType = RAllocator::EReset;
	iFailRate = 0;
	iFailed = EFalse;
	iFailAllocCount = 0;
	iRand = 0;
	iTestData = NULL;
    }


TBool RMemSpyDriverRHeapBase::CheckCell( TAny* aCellAddress, TInt aLength ) const
	{
	const TLinAddr m = TLinAddr(iAlign - 1);
    TRACE_HEAP( Kern::Printf("RMemSpyDriverRHeapBase::CheckCell() - cell: 0x%08x, len: %8d, iAlign: %d, m: %d", aCellAddress, aLength, iAlign, m) );

    TBool isValid = ETrue;
    //
    if ( isValid && (aLength & m) )
        {
    	TRACE_HEAP( Kern::Printf("RMemSpyDriverRHeapBase::CheckCell() - ERROR - length is odd: %d, iAlign: %d, m: %d", aLength, iAlign, m) );
        isValid = EFalse;
        }
    if ( isValid && aLength < iMinCell )
        {
    	TRACE_HEAP( Kern::Printf("RMemSpyDriverRHeapBase::CheckCell() - ERROR - length: %d, is less than min cell size (%d)", aLength, iMinCell) );
        isValid = EFalse;
        }
    if ( isValid && (TUint8*)aCellAddress < iBase )
        {
    	TRACE_HEAP( Kern::Printf("RMemSpyDriverRHeapBase::CheckCell() - ERROR - cell address: 0x%08x, is before start address: 0x%08x", (TUint8*) aCellAddress, iBase) );
        isValid = EFalse;
        }

    if  ( isValid )
        {
        const TUint8* nextCell = (TUint8*)__NEXT_CELL2(aCellAddress, aLength);
        if  ( nextCell > iTop )
            {
        	TRACE_HEAP( Kern::Printf("RMemSpyDriverRHeapBase::CheckCell() - ERROR - nextCell: 0x%08x is after the top of the heap: 0x%08x", nextCell, iTop) );
            isValid = EFalse;
            }
        }
    //
    return isValid;
	}


TInt RMemSpyDriverRHeapBase::AllocatedCellHeaderSize( TBool aDebugLibrary )
    {
    // Allocated cells are only 4 bytes in UREL, but 12 bytes in UDEB.
    TInt size = sizeof(SCell*);
    //
    if  ( aDebugLibrary )
        {
        size = sizeof(SDebugCell);
        }
    //
    return size;
    }


TInt RMemSpyDriverRHeapBase::FreeCellHeaderSize()
    {
    // Free cells remain the same size in UREL and UDEB builds.
    const TInt size = sizeof(SCell);
    return size; 
    }


TInt RMemSpyDriverRHeapBase::CellHeaderSize( const TMemSpyDriverInternalWalkHeapParamsCell& aCell, TBool aDebugLibrary )
    {
    TInt size = 0;
    //
    if  ( aCell.iCellType == EMemSpyDriverGoodAllocatedCell )
        {
        size = AllocatedCellHeaderSize( aDebugLibrary );
        }
    else if ( aCell.iCellType == EMemSpyDriverGoodFreeCell ) 
        {
        size = FreeCellHeaderSize();
        }
    //
    return size;
    }


void RMemSpyDriverRHeapBase::PrintInfo()
    {
#if defined(TRACE_TYPE_KERNELHEAP) || defined(TRACE_TYPE_USERHEAP)
    Kern::Printf(" " );
    Kern::Printf("RMemSpyDriverRHeapBase::PrintInfo - RAllocator - iAccessCount:    0x%08x", iAccessCount );
    Kern::Printf("RMemSpyDriverRHeapBase::PrintInfo - RAllocator - iHandleCount:    0x%08x", iHandleCount );
    Kern::Printf("RMemSpyDriverRHeapBase::PrintInfo - RAllocator - iHandles:        0x%08x", iHandles );
    Kern::Printf("RMemSpyDriverRHeapBase::PrintInfo - RAllocator - iFlags:          0x%08x", iFlags );
    Kern::Printf("RMemSpyDriverRHeapBase::PrintInfo - RAllocator - iCellCount:      0x%08x", iCellCount );
    Kern::Printf("RMemSpyDriverRHeapBase::PrintInfo - RAllocator - iTotalAllocSize: 0x%08x", iTotalAllocSize );

    Kern::Printf("RMemSpyDriverRHeapBase::PrintInfo - RHeap -      iMinLength:      0x%08x", iMinLength );
    Kern::Printf("RMemSpyDriverRHeapBase::PrintInfo - RHeap -      iMaxLength:      0x%08x", iMaxLength );
    Kern::Printf("RMemSpyDriverRHeapBase::PrintInfo - RHeap -      iOffset:         0x%08x", iOffset);
    Kern::Printf("RMemSpyDriverRHeapBase::PrintInfo - RHeap -      iGrowBy:         0x%08x", iGrowBy);
    Kern::Printf("RMemSpyDriverRHeapBase::PrintInfo - RHeap -      iChunkHandle:    0x%08x", iChunkHandle);
    Kern::Printf("RMemSpyDriverRHeapBase::PrintInfo - RHeap -      iBase:           0x%08x", Base());
    Kern::Printf("RMemSpyDriverRHeapBase::PrintInfo - RHeap -      iTop:            0x%08x", iTop );
    Kern::Printf("RMemSpyDriverRHeapBase::PrintInfo - RHeap -      iAlign:          0x%08x", iAlign);
    Kern::Printf("RMemSpyDriverRHeapBase::PrintInfo - RHeap -      iMinCell:        0x%08x", iMinCell);
    Kern::Printf("RMemSpyDriverRHeapBase::PrintInfo - RHeap -      iPageSize:       0x%08x", iPageSize);
    Kern::Printf("RMemSpyDriverRHeapBase::PrintInfo - RHeap -      iFree len:       0x%08x", iFree.len);
    Kern::Printf("RMemSpyDriverRHeapBase::PrintInfo - RHeap -      iFree next:      0x%08x", iFree.next);
    Kern::Printf("RMemSpyDriverRHeapBase::PrintInfo - RHeap -      iNestingLevel:   0x%08x", iNestingLevel);
    Kern::Printf("RMemSpyDriverRHeapBase::PrintInfo - RHeap -      iAllocCount:     0x%08x", iAllocCount);
    Kern::Printf("RMemSpyDriverRHeapBase::PrintInfo - RHeap -      size:              %8d",  Size() );
    Kern::Printf(" " );
    Kern::Printf(" " );
#endif
    }


void RMemSpyDriverRHeapBase::CopyObjectDataTo( TMemSpyHeapObjectDataRHeap& aData )
    {
    TRACE_HEAP( Kern::Printf("RMemSpyDriverRHeapBase::CopyObjectDataTo() - START" ) );

    TUint8* sourceAddress = reinterpret_cast< TUint8* >( this );
    sourceAddress += KRAllocatorAndRHeapMemberDataOffset;
    memcpy( &aData, sourceAddress, KRHeapObjectSize );

    TRACE_HEAP( Kern::Printf("RMemSpyDriverRHeapBase::CopyObjectDataTo() - END") );
    }

















































RMemSpyDriverRHeapReadFromCopy::RMemSpyDriverRHeapReadFromCopy( DMemSpyDriverOSAdaption& aOSAdaption )
:   iOSAdaption( aOSAdaption ), iChunk( NULL ), iChunkAddress( 0 ), iChunkMappingAttributes( 0 ), iClientToKernelDelta( 0 )
    {
    }


void RMemSpyDriverRHeapReadFromCopy::Reset()
    {
    RMemSpyDriverRHeapBase::Reset();
	//
    iChunk = NULL;
    iChunkAddress = 0;
    iChunkMappingAttributes = 0;
    iClientToKernelDelta = 0;
    }


void RMemSpyDriverRHeapReadFromCopy::AssociateWithKernelChunk( DChunk* aChunk, TLinAddr aAddress, TUint32 aMappingAttributes )
    {
    TRACE_HEAP( Kern::Printf("RMemSpyDriverRHeapReadFromCopy::AssociateWithKernelChunk() - START - aChunk: %O, aChunk base: 0x%08x, aAddress: 0x%08x, clients heap base: 0x%08x, aChunk size: %8d", aChunk, aChunk->iBase, aAddress, Base(), aChunk->iSize ) );

    iChunk = aChunk;
    iChunkAddress = aAddress;
    iChunkMappingAttributes = aMappingAttributes;

    // Calculate start of real heap data (skipping over embedded RHeap object)
    // Since we must operate with kernel-side addressing into our cloned heap chunk,
    // we must use aAddress (the kernel address of the chunk) rather than aChunk->iBase
    iClientToKernelDelta = ( (TUint8*) aAddress ) - ( Base() - KRHeapObjectSize );

    TRACE_HEAP( Kern::Printf("RMemSpyDriverRHeapReadFromCopy::AssociateWithKernelChunk() - END - delta between client's user-side base address (base: 0x%08x), kernel-side base address (base: 0x%08x), and kernel-side chunk (base: 0x%08x) is: 0x%08x", Base(), aChunk->iBase, aAddress, iClientToKernelDelta) );
    }


void RMemSpyDriverRHeapReadFromCopy::DisassociateWithKernelChunk()
    {
    TRACE_HEAP( Kern::Printf("RMemSpyDriverRHeapReadFromCopy::DisassociateWithKernelChunk() - START - iChunk: 0x%08x", iChunk ) );

    NKern::ThreadEnterCS();
    if  ( iChunk != NULL )
        {
        Kern::ChunkClose( iChunk );
        iChunk = NULL;
        }
    NKern::ThreadLeaveCS();

    TRACE_HEAP( Kern::Printf("RMemSpyDriverRHeapReadFromCopy::DisassociateWithKernelChunk() - END") );
    }


DChunk& RMemSpyDriverRHeapReadFromCopy::Chunk()
    {
    return *iChunk;
    }


const DChunk& RMemSpyDriverRHeapReadFromCopy::Chunk() const
    {
    return *iChunk;
    }


TLinAddr RMemSpyDriverRHeapReadFromCopy::ChunkKernelAddress() const
    {
    return iChunkAddress;
    }


TBool RMemSpyDriverRHeapReadFromCopy::ChunkIsInitialised() const
    {
    return iChunk != NULL;
    }


TUint RMemSpyDriverRHeapReadFromCopy::ClientToKernelDelta() const
    {
    return iClientToKernelDelta;
    }

















































RMemSpyDriverRHeapUser::RMemSpyDriverRHeapUser( DMemSpyDriverOSAdaption& aOSAdaption )
:   RMemSpyDriverRHeapReadFromCopy( aOSAdaption )
    {
    }


TInt RMemSpyDriverRHeapUser::ReadFromUserAllocator( DThread& aThread )
    {
    TBuf8<KRHeapMemberDataSize> memberData;
    memberData.SetMax();

    NKern::ThreadEnterCS();
    NKern::LockSystem();
    RAllocator* allocator = OSAdaption().DThread().GetAllocator( aThread );
    NKern::UnlockSystem();
  	NKern::ThreadLeaveCS();

    TUint8* memberDataAddress = (TUint8*) allocator + KRAllocatorAndRHeapMemberDataOffset;
	TRACE_HEAP( Kern::Printf("RMemSpyDriverRHeapUser::ReadFromUserAllocator() - START - allocator addr: 0x%08x, therefore going to read %d bytes from address 0x%08x within client thread (0x%08x + %4d bytes)", allocator, KRHeapMemberDataSize, memberDataAddress, allocator, KRAllocatorAndRHeapMemberDataOffset ) );

    const TInt error = Kern::ThreadRawRead( &aThread, memberDataAddress, (TAny*) memberData.Ptr(), KRHeapMemberDataSize );
    TRACE_DATA( MemSpyDriverUtils::DataDump("%lS", memberData.Ptr(), KRHeapMemberDataSize, KRHeapMemberDataSize ) );

    if  ( error == KErrNone )
        {
        TUint8* destinationAddress = reinterpret_cast< TUint8* >( this );

        // Skip over our vTable too...
        destinationAddress += KRAllocatorAndRHeapMemberDataOffset;

        // Now copy data into this object
        TPtr8 self( destinationAddress, KRHeapMemberDataSize, KRHeapMemberDataSize );
        self.Copy( memberData );

        PrintInfo();
        }
    else
        {
        }

	TRACE_HEAP( Kern::Printf("RMemSpyDriverRHeapUser::ReadFromUserAllocator() - END - read error: %d", error ) );
    return error;
    }
















RMemSpyDriverRHeapKernelFromCopy::RMemSpyDriverRHeapKernelFromCopy( DMemSpyDriverOSAdaption& aOSAdaption )
:   RMemSpyDriverRHeapReadFromCopy( aOSAdaption )
    {
    }


void RMemSpyDriverRHeapKernelFromCopy::SetKernelHeap( RHeapK& aKernelHeap )
    {
    TRACE_KH( Kern::Printf("RMemSpyDriverRHeapKernelFromCopy::SetKernelHeap() - START" ) );

    // Perform a copy operation in order to populate base class with a duplicate of the kernel's heap info.
    iKernelHeap = &aKernelHeap;

    // Source address
    TUint8* sourceAddress = (TUint8*) iKernelHeap + KRAllocatorAndRHeapMemberDataOffset;
    TUint8* destinationAddress = (TUint8*) this + KRAllocatorAndRHeapMemberDataOffset;

    // Copy 
    memcpy( destinationAddress, sourceAddress, KRHeapMemberDataSize );

    // And print info in debug builds for verification...
    PrintInfo();

    TRACE_KH( Kern::Printf("RMemSpyDriverRHeapKernelFromCopy::SetKernelHeap() - END" ) );
    }


void RMemSpyDriverRHeapKernelFromCopy::DisassociateWithKernelChunk()
    {
    TRACE_KH( Kern::Printf("RMemSpyDriverRHeapKernelFromCopy::DisassociateWithKernelChunk() - START - iKernelHeap: 0x%08x", iKernelHeap ));
    iKernelHeap = NULL;
    RMemSpyDriverRHeapReadFromCopy::DisassociateWithKernelChunk();
    TRACE_KH( Kern::Printf("RMemSpyDriverRHeapKernelFromCopy::DisassociateWithKernelChunk() - END") );
    }


void RMemSpyDriverRHeapKernelFromCopy::GetHeapSpecificInfo( TMemSpyHeapInfo& aInfo ) const
    {
    TRACE_KH( Kern::Printf("RMemSpyDriverRHeapKernelFromCopy::GetHeapSpecificInfo() - START - iKernelHeap: 0x%08x", iKernelHeap ));
    //
    if  ( iKernelHeap )
        {
        const TUint32* pHeap = reinterpret_cast< TUint32* >( iKernelHeap );
        //
        TMemSpyHeapInfoRHeap& rHeapInfo = aInfo.AsRHeap();
        TMemSpyHeapMetaDataRHeap& rHeapMetaData = rHeapInfo.MetaData();
        rHeapMetaData.SetVTable( *pHeap );
        rHeapMetaData.SetClassSize( KRHeapObjectSize );
        //
        TRACE_KH( Kern::Printf("RMemSpyDriverRHeapKernelFromCopy::GetHeapSpecificInfo() - RHeapK vtable is: 0x%08x", *pHeap ));
        }
    //
    TRACE_KH( Kern::Printf("RMemSpyDriverRHeapKernelFromCopy::GetHeapSpecificInfo() - END") );
    }





















RMemSpyDriverRHeapKernelInPlace::RMemSpyDriverRHeapKernelInPlace()
:   iKernelHeap( NULL ), iChunk( NULL )
    {
    }


void RMemSpyDriverRHeapKernelInPlace::SetKernelHeap( RHeapK& aKernelHeap )
    {
    iKernelHeap = &aKernelHeap;
    CopyMembersFromKernelHeap();
    }


void RMemSpyDriverRHeapKernelInPlace::FailNext()
    {
    RMemSpyDriverRHeapKernelInPlace::RHeapKExtended* heap = reinterpret_cast< RMemSpyDriverRHeapKernelInPlace::RHeapKExtended* >( iKernelHeap );
    heap->FailNext();
    }


void RMemSpyDriverRHeapKernelInPlace::Reset()
    {
    RMemSpyDriverRHeapBase::Reset();
	//
    iChunk = NULL;
    }


void RMemSpyDriverRHeapKernelInPlace::AssociateWithKernelChunk( DChunk* aChunk, TLinAddr /*aAddress*/, TUint32 /*aMappingAttributes*/ )
    {
    TRACE_KH( Kern::Printf("RMemSpyDriverRHeapKernelInPlace::AssociateWithKernelChunk() - START - aChunk: %O, aChunk base: 0x%08x", aChunk, aChunk->iBase ) );
    iChunk = aChunk;
    }


void RMemSpyDriverRHeapKernelInPlace::DisassociateWithKernelChunk()
    {
    TRACE_KH( Kern::Printf("RMemSpyDriverRHeapKernelInPlace::DisassociateWithKernelChunk() - START - iChunk: 0x%08x", iChunk ));
    iChunk = NULL;
    iKernelHeap = NULL;
    TRACE_KH( Kern::Printf("RMemSpyDriverRHeapKernelInPlace::DisassociateWithKernelChunk() - END") );
    }


DChunk& RMemSpyDriverRHeapKernelInPlace::Chunk()
    {
    return *iChunk;
    }


const DChunk& RMemSpyDriverRHeapKernelInPlace::Chunk() const
    {
    return *iChunk;
    }


TLinAddr RMemSpyDriverRHeapKernelInPlace::ChunkKernelAddress() const
    {
    const TLinAddr ret = reinterpret_cast< TLinAddr >( iChunk->iBase );
    return ret;
    }


TBool RMemSpyDriverRHeapKernelInPlace::ChunkIsInitialised() const
    {
    return iChunk != NULL;
    }


TUint RMemSpyDriverRHeapKernelInPlace::ClientToKernelDelta() const
    {
    // We're operating in kernel address space, there is no delta.
    return 0;
    }


void RMemSpyDriverRHeapKernelInPlace::CopyMembersFromKernelHeap()
    {
    TRACE_KH( Kern::Printf("RMemSpyDriverRHeapKernelInPlace::CopyMembersFromKernelHeap() - START" ) );

    // Perform a copy operation in order to populate base class with a duplicate of the kernel's heap info.
    RHeapK* kernelHeap = iKernelHeap;

    // Source address
    TUint8* sourceAddress = (TUint8*) kernelHeap + KRAllocatorAndRHeapMemberDataOffset;
    TUint8* destinationAddress = (TUint8*) this + KRAllocatorAndRHeapMemberDataOffset;

    // Copy 
    memcpy( destinationAddress, sourceAddress, KRHeapMemberDataSize );

    // And print info in debug builds for verification...
    PrintInfo();

    TRACE_KH( Kern::Printf("RMemSpyDriverRHeapKernelInPlace::CopyMembersFromKernelHeap() - END" ) );
    }


void RMemSpyDriverRHeapKernelInPlace::GetHeapSpecificInfo( TMemSpyHeapInfo& aInfo ) const
    {
    TRACE_KH( Kern::Printf("RMemSpyDriverRHeapKernelInPlace::GetHeapSpecificInfo() - START - iKernelHeap: 0x%08x", iKernelHeap ));
    //
    if  ( iKernelHeap )
        {
        const TUint32* pHeap = reinterpret_cast< TUint32* >( iKernelHeap );
        //
        TMemSpyHeapInfoRHeap& rHeapInfo = aInfo.AsRHeap();
        TMemSpyHeapMetaDataRHeap& rHeapMetaData = rHeapInfo.MetaData();
        rHeapMetaData.SetVTable( *pHeap );
        rHeapMetaData.SetClassSize( KRHeapObjectSize );
        //
        TRACE_KH( Kern::Printf("RMemSpyDriverRHeapKernelInPlace::GetHeapSpecificInfo() - RHeapK vtable is: 0x%08x", *pHeap ));
        }
    //
    TRACE_KH( Kern::Printf("RMemSpyDriverRHeapKernelInPlace::GetHeapSpecificInfo() - END") );
    }





