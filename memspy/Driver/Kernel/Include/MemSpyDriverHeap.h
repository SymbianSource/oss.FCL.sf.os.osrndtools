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

#ifndef MEMSPYDRIVERHEAP_H
#define MEMSPYDRIVERHEAP_H

// System includes
#include <e32cmn.h>
#include <kern_priv.h>
#include <memspy/driver/memspydriverobjectsshared.h>
#include <memspy/driver/memspydriverenumerationsshared.h>

// User includes
#include "MemSpyDriverObjectsInternal.h"

// Constants
const TUint KRHeapObjectSize = 0x74;
const TUint KRAllocatorAndRHeapMemberDataOffset = 4; // 4 bytes past start of allocator address, i.e. skipping the vtable
const TUint KRHeapMemberDataSize = KRHeapObjectSize - KRAllocatorAndRHeapMemberDataOffset;

// Classes referenced
class DMemSpyDriverOSAdaption;


/**
 * Essentially a mirror of RAllocator and RHeap's layout.
 */
class RMemSpyDriverRHeapBase
	{
public:
	struct SCell
        {
        TInt len; 
        SCell* next;
        };

    struct SDebugCell
        {
        TInt len;
        TInt nestingLevel;
        TInt allocCount;
        };

    struct _s_align {char c; double d;};

    struct SHeapCellInfo { RHeap* iHeap; TInt iTotalAlloc;	TInt iTotalAllocSize; TInt iTotalFree; TInt iLevelAlloc; SDebugCell* iStranded; };
	
    enum {ECellAlignment = sizeof(_s_align)-sizeof(double)};
	enum {EFreeCellSize = sizeof(SCell)};
	enum TDebugOp {EWalk=128};
	enum TCellType
		{EGoodAllocatedCell, EGoodFreeCell, EBadAllocatedCellSize, EBadAllocatedCellAddress,
		EBadFreeCellAddress, EBadFreeCellSize};
	
    enum TDebugHeapId {EUser=0, EKernel=1};

protected:
    RMemSpyDriverRHeapBase();

public: // Inlines
    inline TUint8* Base() const { return iBase; }
    inline TInt Size() const { return iTop - iBase; }
    inline TInt MaxLength() const { return iMaxLength; }

public: // API
    void PrintInfo();
    void CopyObjectDataTo( TMemSpyHeapObjectDataRHeap& aData );

public: // Virtual API
    virtual void Reset();
    virtual void AssociateWithKernelChunk( DChunk* aChunk, TLinAddr aAddress, TUint32 aMappingAttributes ) = 0;
    virtual void DisassociateWithKernelChunk() = 0;
    virtual DChunk& Chunk() = 0;
    virtual const DChunk& Chunk() const = 0;
    virtual TLinAddr ChunkKernelAddress() const = 0;
    virtual TBool ChunkIsInitialised() const = 0;
    virtual TUint ClientToKernelDelta() const = 0;
    virtual void GetHeapSpecificInfo( TMemSpyHeapInfo& /*aInfo*/ ) const { }

public: // Utilities
    TBool CheckCell( TAny* aCellAddress, TInt aLength ) const;
    static TInt AllocatedCellHeaderSize( TBool aDebugLibrary );
    static TInt FreeCellHeaderSize();
    static TInt CellHeaderSize( const TMemSpyDriverInternalWalkHeapParamsCell& aCell, TBool aDebugEUser );

public: // From RAllocator
	TInt iAccessCount;
	TInt iHandleCount;
	TInt* iHandles;
	TUint32 iFlags;
	TInt iCellCount;
	TInt iTotalAllocSize;

public: // From RHeap
	TInt iMinLength;
	TInt iMaxLength;
	TInt iOffset;
	TInt iGrowBy;
	TInt iChunkHandle;
	RFastLock iLock;
	TUint8* iBase;
	TUint8* iTop;
	TInt iAlign;
	TInt iMinCell;
	TInt iPageSize;
	SCell iFree;
	TInt iNestingLevel;
	TInt iAllocCount;
    RAllocator::TAllocFail iFailType;
	TInt iFailRate;
	TBool iFailed;
	TInt iFailAllocCount;
	TInt iRand;
	TAny* iTestData;
    };




class RMemSpyDriverRHeapReadFromCopy : public RMemSpyDriverRHeapBase
	{
protected:
    RMemSpyDriverRHeapReadFromCopy( DMemSpyDriverOSAdaption& aOSAdaption );

public: // New API
    TInt ReadFromUserAllocator( DThread& aThread );

public: // From RMemSpyDriverRHeapBase
    void Reset();
    void AssociateWithKernelChunk( DChunk* aChunk, TLinAddr aAddress, TUint32 aMappingAttributes );
    void DisassociateWithKernelChunk();
    DChunk& Chunk();
    const DChunk& Chunk() const;
    TLinAddr ChunkKernelAddress() const;
    TBool ChunkIsInitialised() const;
    TUint ClientToKernelDelta() const;

protected:
    inline DMemSpyDriverOSAdaption& OSAdaption() { return iOSAdaption; }

private:
    DMemSpyDriverOSAdaption& iOSAdaption;

    // Copy of the client's heap data
    DChunk* iChunk;
    TLinAddr iChunkAddress;
    TUint32 iChunkMappingAttributes;

    // Calculated delta between client's address space values and actual kernel
    // address of the heap chunk.
    TUint iClientToKernelDelta;
    };







class RMemSpyDriverRHeapUser : public RMemSpyDriverRHeapReadFromCopy
	{
public:
    RMemSpyDriverRHeapUser( DMemSpyDriverOSAdaption& aOSAdaption );

public: // New API
    TInt ReadFromUserAllocator( DThread& aThread );
    };



class RMemSpyDriverRHeapKernelFromCopy : public RMemSpyDriverRHeapReadFromCopy
    {
public:
    RMemSpyDriverRHeapKernelFromCopy( DMemSpyDriverOSAdaption& aOSAdaption );
    
public: // API
    void SetKernelHeap( RHeapK& aKernelHeap );

public: // From RMemSpyDriverRHeapBase
    void DisassociateWithKernelChunk();
    void GetHeapSpecificInfo( TMemSpyHeapInfo& aInfo ) const;

private:
    RHeapK* iKernelHeap;
    };



class RMemSpyDriverRHeapKernelInPlace : public RMemSpyDriverRHeapBase
    {
public:
    RMemSpyDriverRHeapKernelInPlace();
    
public: // API
    void FailNext();
    void SetKernelHeap( RHeapK& aKernelHeap );

public: // From RMemSpyDriverRHeapBase
    void Reset();
    void AssociateWithKernelChunk( DChunk* aChunk, TLinAddr aAddress, TUint32 aMappingAttributes );
    void DisassociateWithKernelChunk();
    DChunk& Chunk();
    const DChunk& Chunk() const;
    TLinAddr ChunkKernelAddress() const;
    TBool ChunkIsInitialised() const;
    TUint ClientToKernelDelta() const;
    void GetHeapSpecificInfo( TMemSpyHeapInfo& aInfo ) const;

private: // Internal methods
    void CopyMembersFromKernelHeap();

private: // Internal class

    /**
     * Used when opening the kernel heap
     */
#ifndef __SYMBIAN_KERNEL_HYBRID_HEAP__
    class RHeapKExtended : public RHeapK
        {
    public:
        inline void FailNext()
            {
            SetFailType( RAllocator::EFailNext );
            SetFailRate( 1 );
            ResetFailed();
            ResetFailAllocCount();
            }
        inline void SetFailType( TAllocFail aType ) { iFailType = aType; }
        inline void SetFailRate( TInt aRate ) { iFailRate = aRate; }
        inline void ResetFailed() { iFailed = EFalse; }
        inline void ResetFailAllocCount() { iFailAllocCount = 0; }
        };
#endif
private:
    RHeapK* iKernelHeap;
    DChunk* iChunk;
    };

	
#endif
