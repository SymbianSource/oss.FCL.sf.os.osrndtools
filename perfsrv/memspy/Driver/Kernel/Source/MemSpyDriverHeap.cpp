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
#include "heaputils.h"



RMemSpyDriverRHeapBase::RMemSpyDriverRHeapBase()
	: iHelper(NULL)
    {
    Reset();
    }

LtkUtils::RAllocatorHelper* RMemSpyDriverRHeapBase::Helper()
	{
	return iHelper;
	}

TMemSpyHeapInfo::THeapImplementationType RMemSpyDriverRHeapBase::GetTypeFromHelper() const
	{
	if (iHelper)
		{
		LtkUtils::RAllocatorHelper::TType type = iHelper->GetType();
		switch (type)
			{
			case LtkUtils::RAllocatorHelper::ETypeRHeap:
				return TMemSpyHeapInfo::ETypeRHeap;
			case LtkUtils::RAllocatorHelper::ETypeRHybridHeap:
				return TMemSpyHeapInfo::ETypeRHybridHeap;
			case LtkUtils::RAllocatorHelper::ETypeUnknown:
			default:
				return TMemSpyHeapInfo::ETypeUnknown;
			}
		}
	return TMemSpyHeapInfo::ETypeUnknown;
	}

void RMemSpyDriverRHeapBase::Reset()
    {
	Close();
	}

void RMemSpyDriverRHeapBase::Close()
	{
	if (iHelper)
		{
	    NKern::ThreadEnterCS();
		iHelper->Close();
		delete iHelper;
		iHelper = NULL;
		NKern::ThreadLeaveCS();
		}
    }

void RMemSpyDriverRHeapBase::PrintInfo()
    {
	/* TOMSCI TODO replace with tracing based on latest struct info. See DMemSpyDriverLogChanHeapBase::PrintHeapInfo
	 * Alternatively just call DMemSpyDriverLogChanHeapBase::PrintHeapInfo() somehow?
	 
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
	*/
    }

RMemSpyDriverRHeapReadFromCopy::RMemSpyDriverRHeapReadFromCopy( DMemSpyDriverOSAdaption& aOSAdaption )
:   iOSAdaption( aOSAdaption ), iChunk( NULL ), iChunkAddress( 0 ), iChunkMappingAttributes( 0 ) /*, iClientToKernelDelta( 0 )*/
    {
    }


void RMemSpyDriverRHeapReadFromCopy::Reset()
    {
    RMemSpyDriverRHeapBase::Reset();
	//
    iChunk = NULL;
    iChunkAddress = 0;
    iChunkMappingAttributes = 0;
    //iClientToKernelDelta = 0;
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
    //TOMSCI iClientToKernelDelta = ( (TUint8*) aAddress ) - ( Base() - KRHeapObjectSize );

    TRACE_HEAP( Kern::Printf("RMemSpyDriverRHeapReadFromCopy::AssociateWithKernelChunk() - END - delta between client's user-side base address (base: 0x%08x), kernel-side base address (base: 0x%08x), and kernel-side chunk (base: 0x%08x) is: 0x%08x", Base(), aChunk->iBase, aAddress, iClientToKernelDelta) );
    }


/*void RMemSpyDriverRHeapReadFromCopy::DisassociateWithKernelChunk()
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
*/

DChunk& RMemSpyDriverRHeapReadFromCopy::Chunk()
    {
    return *iChunk;
    }


const DChunk& RMemSpyDriverRHeapReadFromCopy::Chunk() const
    {
    return *iChunk;
    }


/*TLinAddr RMemSpyDriverRHeapReadFromCopy::ChunkKernelAddress() const
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
*/





RMemSpyDriverRHeapUser::RMemSpyDriverRHeapUser( DMemSpyDriverOSAdaption& aOSAdaption )
	: RMemSpyDriverRHeapBase(), iOSAdaption(aOSAdaption)
    {
    }


TInt RMemSpyDriverRHeapUser::OpenUserHeap(DThread& aThread, TBool aEuserUdeb)
	{
	TLinAddr allocatorAddr = (TLinAddr)OSAdaption().DThread().GetAllocator(aThread);
	NKern::ThreadEnterCS();
	LtkUtils::RKernelSideAllocatorHelper* helper = new LtkUtils::RKernelSideAllocatorHelper;
	if (!helper)
		{
		NKern::ThreadLeaveCS();
		return KErrNoMemory;
		}
	TInt err = helper->OpenUserHeap(OSAdaption().DThread().GetId(aThread), allocatorAddr, aEuserUdeb);
	if (!err)
		{
		iChunk = helper->OpenUnderlyingChunk();
		if (!iChunk) err = KErrNotFound;
		}
	if (err)
		{
		delete helper;
		}
	else
		{
		iHelper = helper;
		}
	NKern::ThreadLeaveCS();
	return err;
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


/*
void RMemSpyDriverRHeapKernelFromCopy::DisassociateWithKernelChunk()
    {
    TRACE_KH( Kern::Printf("RMemSpyDriverRHeapKernelFromCopy::DisassociateWithKernelChunk() - START - iKernelHeap: 0x%08x", iKernelHeap ));
    iKernelHeap = NULL;
    RMemSpyDriverRHeapReadFromCopy::DisassociateWithKernelChunk();
    TRACE_KH( Kern::Printf("RMemSpyDriverRHeapKernelFromCopy::DisassociateWithKernelChunk() - END") );
    }
*/

void RMemSpyDriverRHeapKernelFromCopy::Close()
	{
	//TOMSCI TODO close the chunk
	}

RMemSpyDriverRHeapKernelInPlace::RMemSpyDriverRHeapKernelInPlace()
	: iChunk(NULL)
    {
    }

TInt RMemSpyDriverRHeapKernelInPlace::OpenKernelHeap()
	{
	NKern::ThreadEnterCS();
	LtkUtils::RAllocatorHelper* helper = new LtkUtils::RAllocatorHelper;
	if (!helper)
		{
		NKern::ThreadLeaveCS();
		return KErrNoMemory;
		}
	TInt err = helper->OpenKernelHeap();
	if (!err)
		{
		iChunk = helper->OpenUnderlyingChunk();
		if (!iChunk) err = KErrNotFound;
		}

	if (err)
		{
		delete helper;
		}
	else
		{
		iHelper = helper;
		}
	NKern::ThreadLeaveCS();
	return err;
	}

void RMemSpyDriverRHeapKernelInPlace::Close()
    {
	NKern::ThreadEnterCS();
	iChunk->Close(NULL);
	iChunk = NULL;
	RMemSpyDriverRHeapBase::Close();
	NKern::ThreadLeaveCS();
    }

DChunk& RMemSpyDriverRHeapKernelInPlace::Chunk()
    {
    return *iChunk;
    }


const DChunk& RMemSpyDriverRHeapKernelInPlace::Chunk() const
    {
    return *iChunk;
    }

