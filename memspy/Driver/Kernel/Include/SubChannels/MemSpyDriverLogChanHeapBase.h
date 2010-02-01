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

#ifndef MEMSPYDRIVERLOGICALCHANHEAPBASE_H
#define MEMSPYDRIVERLOGICALCHANHEAPBASE_H

// System includes
#include <kern_priv.h>
#include <memspy/driver/memspydriverobjectsshared.h>
#include <memspy/driver/memspydriverenumerationsshared.h>

// User includes
#include "MemSpyDriverOpCodes.h"
#include "MemSpyDriverLogChanBase.h"
#include "MemSpyDriverObjectsInternal.h"
#include "MemSpyDriverHeapWalker.h"
#include "MemSpyDriverEnumerationsInternal.h"

// Classes referenced
class TMemSpyHeapInfo;
class DMemSpyDriverDevice;
class RMemSpyDriverRHeapUser;
class RMemSpyMemStreamWriter;
class RMemSpyDriverRHeapKernelInPlace;
class RMemSpyDriverRHeapKernelFromCopy;


class DMemSpyDriverLogChanHeapBase : public DMemSpyDriverLogChanBase, public MMemSpyHeapWalkerObserver
	{
public:
    enum TDrmMatchType
        {
        EMatchTypeNone = 0,
        EMatchTypeName,
        EMatchTypeUid
        };
public:
	~DMemSpyDriverLogChanHeapBase();

protected:
	DMemSpyDriverLogChanHeapBase( DMemSpyDriverDevice& aDevice, DThread& aThread );
    TInt Construct();

protected: // From DMemSpyDriverLogChanBase
    TInt Request( TInt aFunction, TAny* a1, TAny* a2 );

protected: // Capability checks for heap access
    TDrmMatchType IsDrmThread( DThread& aThread );

private: // From MHeapWalkerObserver
    void HandleHeapWalkInit();
    TBool HandleHeapCell( TInt aCellType, TAny* aCellAddress, TInt aLength, TInt aNestingLevel, TInt aAllocNumber );

protected: // Heap utility functions
    TInt OpenKernelHeap( RHeapK*& aHeap, DChunk*& aChunk, TDes8* aClientHeapChunkName = NULL );
    TInt OpenKernelHeap( RMemSpyDriverRHeapKernelInPlace& aHeap, TDes8* aClientHeapChunkName = NULL );
    TInt OpenKernelHeap( RMemSpyDriverRHeapKernelFromCopy& aHeap, TDes8* aClientHeapChunkName = NULL );
    TInt OpenUserHeap( DThread& aClientThread, TUint aExpectedHeapVTable, RMemSpyDriverRHeapUser& aHeap, DChunk*& aUserHeapChunk, TDes8* aClientHeapChunkName = NULL );
    TBool GetUserHeapHandle( DThread& aThread, RMemSpyDriverRHeapUser& aHeap, TUint aExpectedVTable );
    TBool IsDebugKernel();
    TBool IsDebugKernel( RMemSpyDriverRHeapKernelInPlace& aHeap );
    TInt GetHeapInfoKernel( RMemSpyDriverRHeapBase& aHeap, TBool aIsDebugAllocator, const TDesC8& aChunkName, TMemSpyHeapInfo* aHeapInfo, TDes8* aTransferBuffer );
    void PrintHeapInfo( const TMemSpyHeapInfo& aInfo );

protected: // Free cells
    void ReleaseFreeCells();
    TInt PrepareFreeCellTransferBuffer();
	TInt FetchFreeCells( TDes8* aBufferSink );
    TInt CalculateFreeCellBufferSize() const;

private: // Data members
	RArray< TMemSpyDriverFreeCell > iFreeCells;

    // Points to stack-based object whilst walking in progress
    RMemSpyMemStreamWriter* iStackStream;
    RMemSpyMemStreamWriter* iHeapStream;
    TInt iFreeCellCount;
	};


#endif
