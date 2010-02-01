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

#ifndef MEMSPYDRIVERHEAPWALKER_H
#define MEMSPYDRIVERHEAPWALKER_H

// System includes
#include <e32cmn.h>
#include <kern_priv.h>

// Shared includes
#include <memspy/driver/memspydriverenumerationsshared.h>

// User includes
#include "MemSpyDriverHeap.h"
#include "MemSpyDriverHeapStatistics.h"


// Heap walker observer interface - can be used to make a record of each cell
class MMemSpyHeapWalkerObserver
    {
public:
    virtual TBool HandleHeapCell( TInt aCellType, TAny* aCellAddress, TInt aLength, TInt aNestingLevel, TInt aAllocNumber ) = 0;
    virtual void HandleHeapWalkInit() = 0;
    };



// A null observer that is used to collect basic statistics
class TMemSpyHeapWalkerNullObserver : public MMemSpyHeapWalkerObserver
    {
public:
    TBool HandleHeapCell( TInt /*aCellType*/, TAny* /*aCellAddress*/, TInt /*aLength*/, TInt /*aNestingLevel*/, TInt /*aAllocNumber*/ ) { return ETrue; }
    void HandleHeapWalkInit() { }
    };


// Heap walker - allows in-place walking of any heap
class RMemSpyDriverHeapWalker
    {
public:
    RMemSpyDriverHeapWalker( RMemSpyDriverRHeapBase& aHeap, TBool aDebugAllocator );
    RMemSpyDriverHeapWalker( RMemSpyDriverRHeapBase& aHeap, TBool aDebugAllocator, MMemSpyHeapWalkerObserver& aObserver );
		
public: // API
    TInt Traverse();
    void CopyStatsTo( TMemSpyHeapStatisticsRHeap& aStats );
    void SetObserver( MMemSpyHeapWalkerObserver* aObserver );
    inline void SetPrintDebug() { iPrintDebug = ETrue; }
    inline const TMemSpyHeapWalkStatistics& Stats() const { return iStats; }

public: // Utility functions
    static TAny* KernelAddress( TAny* aUserAddress, TUint aDelta );
    static TAny* UserAddress( TAny* aKernelAddress, TUint aDelta );
    static RMemSpyDriverRHeapBase::SCell* CellByUserAddress( TAny* aAddress, TUint aDelta );

private: // Internal methods
    TBool NotifyCell( TMemSpyDriverCellType aType, TAny* aCellAddress, TInt aLength, TInt aNestingLevel = -1, TInt aAllocNumber = -1 );
    //
    void UpdateStats( TMemSpyDriverCellType aType, TAny* aCellAddress, TInt aLength, TInt aNestingLevel, TInt aAllocNumber );
    void InitialiseStats();
    void FinaliseStats();
    void PrintStats();
    //
    TAny* KernelAddress( TAny* aUserAddress ) const;
    TAny* UserAddress( TAny* aKernelAddress ) const;
    //
    inline TBool PrintDebug() const { return iPrintDebug; }

private:
    RMemSpyDriverRHeapBase& iHeap;
    TBool iIsDebugAllocator;
    TBool iPrintDebug;
    MMemSpyHeapWalkerObserver* iObserver;
    TMemSpyHeapWalkStatistics iStats;
    };



#endif
