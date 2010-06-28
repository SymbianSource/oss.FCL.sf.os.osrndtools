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
// We shouldn't be using any of these any more! -Tomsci
const TUint KRHeapObjectSize = 0x74;
const TUint KRAllocatorAndRHeapMemberDataOffset = 4; // 4 bytes past start of allocator address, i.e. skipping the vtable
const TUint KRHeapMemberDataSize = KRHeapObjectSize - KRAllocatorAndRHeapMemberDataOffset;

// Classes referenced
class DMemSpyDriverOSAdaption;
namespace LtkUtils
	{
	class RAllocatorHelper;
	}

/**
 * Essentially a mirror of RAllocator and RHeap's layout.
 */
class RMemSpyDriverRHeapBase
	{
protected:
    RMemSpyDriverRHeapBase();

public: // API
    void PrintInfo();
	LtkUtils::RAllocatorHelper* Helper();
	TMemSpyHeapInfo::THeapImplementationType GetTypeFromHelper() const;

public: // Virtual API
    virtual void Reset();
	virtual void Close();
    virtual DChunk& Chunk() = 0;
    virtual const DChunk& Chunk() const = 0;

protected:
	LtkUtils::RAllocatorHelper* iHelper;
    };




class RMemSpyDriverRHeapReadFromCopy : public RMemSpyDriverRHeapBase
	{
protected:
    RMemSpyDriverRHeapReadFromCopy( DMemSpyDriverOSAdaption& aOSAdaption );

public: // New API
    void AssociateWithKernelChunk( DChunk* aChunk, TLinAddr aAddress, TUint32 aMappingAttributes );

public: // From RMemSpyDriverRHeapBase
    void Reset();
    DChunk& Chunk();
    const DChunk& Chunk() const;

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
    //TUint iClientToKernelDelta;
    };







class RMemSpyDriverRHeapUser : public RMemSpyDriverRHeapBase
	{
public:
    RMemSpyDriverRHeapUser( DMemSpyDriverOSAdaption& aOSAdaption );
	TInt OpenUserHeap(DThread& aThread, TBool aEuserUdeb);

	DChunk& Chunk() { return *iChunk; }
	const DChunk& Chunk() const { return *iChunk; }

private:
    inline DMemSpyDriverOSAdaption& OSAdaption() { return iOSAdaption; }

private:
    DMemSpyDriverOSAdaption& iOSAdaption;
	DChunk* iChunk;
    };



class RMemSpyDriverRHeapKernelFromCopy : public RMemSpyDriverRHeapReadFromCopy
    {
public:
    RMemSpyDriverRHeapKernelFromCopy( DMemSpyDriverOSAdaption& aOSAdaption );
    
public: // API
    void SetKernelHeap( RHeapK& aKernelHeap );

public: // From RMemSpyDriverRHeapBase
    //void DisassociateWithKernelChunk();
	void Close();

private:
    RHeapK* iKernelHeap;
    };



class RMemSpyDriverRHeapKernelInPlace : public RMemSpyDriverRHeapBase
    {
public:
    RMemSpyDriverRHeapKernelInPlace();
	TInt OpenKernelHeap();
    

public: // From RMemSpyDriverRHeapBase
    void Close();

    DChunk& Chunk();
    const DChunk& Chunk() const;

	// Only important member data is the base class's RAllocatorHelper
	// We do cache the chunk though
private:
	DChunk* iChunk;
    };

	
#endif
