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

#include "MemSpyDriverHeapWalker.h"

// User includes
#include "MemSpyDriverUtils.h"

// Defines
#define __NEXT_CELL(p)				((RMemSpyDriverRHeapBase::SCell*)(((TUint8*)p)+p->len))
#define PRINTDEBUG( a ) { if ( PrintDebug() ) a; }


RMemSpyDriverHeapWalker::RMemSpyDriverHeapWalker( RMemSpyDriverRHeapBase& aHeap, TBool aDebugAllocator )
:   iHeap( aHeap ), iIsDebugAllocator( aDebugAllocator ), iPrintDebug( EFalse ), iObserver( NULL )
    {
    InitialiseStats();
    }


RMemSpyDriverHeapWalker::RMemSpyDriverHeapWalker( RMemSpyDriverRHeapBase& aHeap, TBool aDebugAllocator, MMemSpyHeapWalkerObserver& aObserver )
:   iHeap( aHeap ), iIsDebugAllocator( aDebugAllocator ), iPrintDebug( EFalse ), iObserver( &aObserver )
    {
    InitialiseStats();
    }


TInt RMemSpyDriverHeapWalker::Traverse()
//
// Walk the heap calling the info function.
//
	{
    PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::Traverse() - START - delta: 0x%08x", iHeap.ClientToKernelDelta() ));
    InitialiseStats();
    if  ( iObserver )
        {
        PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::Traverse() - heap walk init..." ));
        iObserver->HandleHeapWalkInit();
        }

    PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::Traverse() - heap walk init complete" ));
    TAny* heapBase = KernelAddress( iHeap.iBase );
    TAny* heapTop = KernelAddress( iHeap.iTop );
	PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::Traverse() - kernel-side chunk address: 0x%08x, chunkBase: 0x%08x, heapBase: 0x%08x, heapTop: 0x%08x", iHeap.ChunkKernelAddress(), iHeap.Chunk().iBase, heapBase, heapTop));

    TRACE_DATA( MemSpyDriverUtils::DataDump("%lS", (TUint8*) iHeap.ChunkKernelAddress(), iHeap.Chunk().iSize, iHeap.Chunk().iSize ) );
   
	TInt nestingLevel = 0;
	TInt allocationNumber = 0;
	//
	RMemSpyDriverRHeapBase::SCell* pC = (RMemSpyDriverRHeapBase::SCell*) heapBase;		// allocated cells
	RMemSpyDriverRHeapBase::SCell* pF = &iHeap.iFree;				            // free cells
	PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::Traverse() - before while loop entry - pC: 0x%08x, pF: 0x%08x, heapBase: 0x%08x, heapTop: 0x%08x", pC, pF, heapBase, heapTop));
    //
    while( ( pF == &iHeap.iFree ) || ( pF >= heapBase && pF < heapTop ) )
		{
        pF = (RMemSpyDriverRHeapBase::SCell*) KernelAddress( pF->next );				// next free cell
	    PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::Traverse() - pC: 0x%08x, pF: 0x%08x, heapBase: 0x%08x, heapTop: 0x%08x", pC, pF, heapBase, heapTop));

        if  ( pF )
        	{
            PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::Traverse() - freeCell:       0x%08x", pF ));

            if  ( pF >= heapBase && pF < heapTop )
                {
                PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::Traverse() - freeCell->next: 0x%08x", pF->next ));
                PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::Traverse() - freeCell->len:  0x%08x", pF->len ));
                }
            else
                {
                PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::Traverse() - FATAL ERROR - freeCell:  0x%08x is outside heap bounds!", pF ));
                }

            PRINTDEBUG( Kern::Printf(" "));
            }
		
        if  (!pF)
            {
            PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::Traverse() - next free cell address is NULL"));
			pF = (RMemSpyDriverRHeapBase::SCell*) heapTop;		// to make size checking work
            }
		else if (  (TUint8*) pF < heapBase || (TUint8*) pF >= heapTop || (KernelAddress( pF->next ) && KernelAddress( pF->next ) <= pF ) )
			{
			// free cell pointer off the end or going backwards
            PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::Traverse() - EBadFreeCellAddress: 0x%08x", pF ));
            NotifyCell( EMemSpyDriverBadFreeCellAddress, UserAddress(pF), 0 );
			return KErrAbort;
			}
		else
			{
			TInt l = pF->len;
			if ( l< iHeap.iMinCell || (l & (iHeap.iAlign-1)))
				{
				// free cell length invalid
                PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::Traverse() - EBadFreeCellSize: 0x%08x", pF ));
		        NotifyCell( EMemSpyDriverBadFreeCellSize, UserAddress(pF), l );
			    return KErrAbort;
				}
			}

        while ( pC != pF )				// walk allocated cells up to next free cell
			{
    	    if  ( pC )
        	    {
                // The 'next' cell field is only applicable if the cell is a 'free' cell, hence we only print the cell's
                // address, its length, and its _calculated_ next cell (based upon address + length). Calc length is done
                // a bit later on...
                PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::Traverse() - allocCell:       0x%08x", pC ));
                PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::Traverse() - allocCell->len:  0x%08x", pC->len ));
                PRINTDEBUG( Kern::Printf(" "));
                }
            
            TInt l = pC->len;
			if (l<iHeap.iMinCell || (l & (iHeap.iAlign-1)))
				{
				// allocated cell length invalid
                PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::Traverse() - EBadAllocatedCellSize: 0x%08x", pC ));
		        NotifyCell( EMemSpyDriverBadAllocatedCellSize, UserAddress(pC), l );
			    return KErrAbort;
				}

            // ALLOCATED CELL
            if  ( iIsDebugAllocator )
                {
                RMemSpyDriverRHeapBase::SDebugCell* debugCell = (RMemSpyDriverRHeapBase::SDebugCell*) pC;
                nestingLevel = debugCell->nestingLevel;
                allocationNumber = debugCell->allocCount;
                }

            PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::Traverse() - EGoodAllocatedCell: 0x%08x", pC ));
	        if  ( NotifyCell( EMemSpyDriverGoodAllocatedCell, UserAddress(pC), l, nestingLevel, allocationNumber ) == EFalse )
                {
                PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::Traverse() - END1 - KErrAbort on NotifyCell..."));
			    return KErrAbort;
                }

			RMemSpyDriverRHeapBase::SCell* pN = (RMemSpyDriverRHeapBase::SCell*) __NEXT_CELL( pC );
            PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::Traverse() - allocCell next:  0x%08x", pN ));
			if (pN > pF)
				{
				// cell overlaps next free cell
                PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::Traverse() - EBadAllocatedCellAddress: 0x%08x", pC ));
		        NotifyCell( EMemSpyDriverBadAllocatedCellAddress, UserAddress(pC), l );
			    return KErrAbort;
				}

            pC = pN;
			}

        PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::Traverse() - freeCell before exit check is: 0x%08x", pF ));
        if  ((TUint8*) pF >= heapTop )
            {
            PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::Traverse() - freeCell reached top of heap -> done"));
			break;		// reached end of heap
            }
		
        pC = (RMemSpyDriverRHeapBase::SCell*) __NEXT_CELL(pF);	// step to next allocated cell

        // FREE CELL
        PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::Traverse() - EGoodFreeCell: 0x%08x", pF ));
        if  ( NotifyCell( EMemSpyDriverGoodFreeCell, UserAddress(pF), pF->len ) == EFalse )
            {
            PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::Traverse() - END2 - KErrAbort on NotifyCell..."));
			return KErrAbort;
            }
		}

    FinaliseStats();
    PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::Traverse() - END - pF: 0x%08x, pC: 0x%08x, heapBase: 0x%08x, heapTop: 0x%08x", pF, pC, heapBase, heapTop));
    return KErrNone;
	}


void RMemSpyDriverHeapWalker::CopyStatsTo( TMemSpyHeapStatisticsRHeap& aStats )
    {
	PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::CopyStatsTo() - START"));

    // Copy free cell info
    TMemSpyHeapStatisticsRHeapFree& free = aStats.StatsFree();
    free.SetTypeCount( iStats.iFreeCellCount );
    free.SetTypeSize( iStats.iTotalFreeSpace );

    // If the last cell was a free cell, and it was also the largest cell
    // then we use the prior largest free cell instead. This is because
    // slack space is already reported separately.
    TAny* largestFreeCellAddress = (TAny*) iStats.iLargestCellAddressFree;
    TUint largestFreeCellSize = iStats.iLargestCellSizeFree;
    if ( iStats.iLastCellWasFreeCell && iStats.iLargestCellSizeFree == iStats.iSlackSpace && iStats.iSpackSpaceCellAddress == iStats.iLargestCellAddressFree )
        {
	    PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::CopyStatsTo() - using previous max free cell stats, since largest free cell is slack cell at end of heap..."));
        largestFreeCellAddress = (TAny*) iStats.iLargestCellAddressFreePrevious;
        largestFreeCellSize = iStats.iLargestCellSizeFreePrevious;
        }

    free.SetLargestCellAddress( largestFreeCellAddress );
    free.SetLargestCellSize( largestFreeCellSize );
    free.SetSlackSpaceCellSize( iStats.iSlackSpace );
    free.SetSlackSpaceCellAddress( (TAny*) iStats.iSpackSpaceCellAddress );
    free.SetChecksum( iStats.iFreeCellCRC );

    // Copy allocated cell info
    TMemSpyHeapStatisticsRHeapAllocated& alloc = aStats.StatsAllocated();
    alloc.SetTypeCount( iStats.iAllocCellCount );
    alloc.SetTypeSize( iStats.iTotalAllocSpace );
    alloc.SetLargestCellAddress( (TAny*) iStats.iLargestCellAddressAlloc );
    alloc.SetLargestCellSize( iStats.iLargestCellSizeAlloc );

    // Copy common info
    TMemSpyHeapStatisticsRHeapCommon& common = aStats.StatsCommon();
    common.SetTotalCellCount( iStats.iNumberOfWalkedCells );

	PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::CopyStatsTo() - END"));
    }


void RMemSpyDriverHeapWalker::SetObserver( MMemSpyHeapWalkerObserver* aObserver )
    {
    PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::SetObserver() - aObserver: 0x%08x", aObserver ));
    iObserver = aObserver;
    }


TAny* RMemSpyDriverHeapWalker::KernelAddress( TAny* aUserAddress, TUint aDelta )
    {
    TAny* ret = NULL;
    //
    if  ( aUserAddress )
        {
	    TRACE_HEAP( Kern::Printf("RMemSpyDriverHeapWalker::KernelAddress() - aUserAddress: 0x%08x", aUserAddress));
        ret = (TUint8*) aUserAddress + aDelta;
        }
    //
	TRACE_HEAP( Kern::Printf("RMemSpyDriverHeapWalker::KernelAddress() - ret: 0x%08x", ret));
    return ret;
    }

 
TAny* RMemSpyDriverHeapWalker::UserAddress( TAny* aKernelAddress, TUint aDelta )
    {
    TAny* ret = NULL;
    //
    if  ( aKernelAddress )
        {
	    TRACE_HEAP( Kern::Printf("RMemSpyDriverHeapWalker::UserAddress() - aKernelAddress: 0x%08x", aKernelAddress));
        ret = (TUint8*) aKernelAddress - aDelta;
        }
    //
	TRACE_HEAP( Kern::Printf("RMemSpyDriverHeapWalker::UserAddress() - ret: 0x%08x", ret));
    return ret;
    }


TAny* RMemSpyDriverHeapWalker::KernelAddress( TAny* aUserAddress) const
    {
    return KernelAddress( aUserAddress, iHeap.ClientToKernelDelta() );
    }


TAny* RMemSpyDriverHeapWalker::UserAddress( TAny* aKernelAddress ) const
    {
    return UserAddress( aKernelAddress, iHeap.ClientToKernelDelta() );
    }


RMemSpyDriverRHeapBase::SCell* RMemSpyDriverHeapWalker::CellByUserAddress( TAny* aAddress, TUint aDelta )
    {
    RMemSpyDriverRHeapBase::SCell* ret = (RMemSpyDriverRHeapBase::SCell*) KernelAddress( aAddress, aDelta );
    return ret;
    }


TBool RMemSpyDriverHeapWalker::NotifyCell( TMemSpyDriverCellType aType, TAny* aCellAddress, TInt aLength, TInt aNestingLevel, TInt aAllocNumber )
    {
    // Update stats first
    UpdateStats( aType, aCellAddress, aLength, aNestingLevel, aAllocNumber );
	
    // Notify observer
    TBool continueTraversal = ETrue;
    if  ( iObserver )
        {
        continueTraversal = iObserver->HandleHeapCell( aType, aCellAddress, aLength, aNestingLevel, aAllocNumber );
        }
    //
    return continueTraversal;
    }


void RMemSpyDriverHeapWalker::UpdateStats( TMemSpyDriverCellType aCellType, TAny* aCellAddress, TInt aLength, TInt aNestingLevel, TInt aAllocNumber )
    {
    switch( aCellType )
        {
    case EMemSpyDriverGoodAllocatedCell:
        PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::UpdateStats - EGoodAllocatedCell       - 0x%08x, len: %8d, nestingLev: %8d, allocNum: %8d", aCellAddress, aLength, aNestingLevel, aAllocNumber ));
        break;
    case EMemSpyDriverGoodFreeCell:
        PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::UpdateStats - EGoodFreeCell            - 0x%08x, len: %8d, nestingLev: %8d, allocNum: %8d", aCellAddress, aLength, aNestingLevel, aAllocNumber ));
        break;
    case EMemSpyDriverBadAllocatedCellSize:
        Kern::Printf("RMemSpyDriverHeapWalker::UpdateStats - EBadAllocatedCellSize    - 0x%08x, len: %8d, nestingLev: %8d, allocNum: %8d", aCellAddress, aLength, aNestingLevel, aAllocNumber );
        break;
    case EMemSpyDriverBadAllocatedCellAddress:
        Kern::Printf("RMemSpyDriverHeapWalker::UpdateStats - EBadAllocatedCellAddress - 0x%08x, len: %8d, nestingLev: %8d, allocNum: %8d", aCellAddress, aLength, aNestingLevel, aAllocNumber );
        break;
    case EMemSpyDriverBadFreeCellAddress:
        Kern::Printf("RMemSpyDriverHeapWalker::UpdateStats - EBadFreeCellAddress      - 0x%08x, len: %8d, nestingLev: %8d, allocNum: %8d", aCellAddress, aLength, aNestingLevel, aAllocNumber );
        break;
    case EMemSpyDriverBadFreeCellSize:
        Kern::Printf("RMemSpyDriverHeapWalker::UpdateStats - EBadFreeCellSize         - 0x%08x, len: %8d, nestingLev: %8d, allocNum: %8d", aCellAddress, aLength, aNestingLevel, aAllocNumber );
        break;
    default:
        Kern::Printf("RMemSpyDriverHeapWalker::UpdateStats - UHANDLED TYPE!           - 0x%08x, len: %8d, nestingLev: %8d, allocNum: %8d, type: %d", aCellAddress, aLength, aNestingLevel, aAllocNumber, aCellType );
        break;
        }

    if  ( aCellType == EMemSpyDriverGoodFreeCell )
        {
        // Update checksum
        iStats.iFreeCellCRC = iStats.iFreeCellCRC ^ reinterpret_cast<TUint32>( aCellAddress );

        // Track cell counts and length
        ++iStats.iFreeCellCount;
        iStats.iTotalFreeSpace += aLength;
        iStats.iLastFreeCellLength = aLength;

        PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::UpdateStats - WAS FREE CELL - iFreeCellCRC: 0x%08x, iFreeCellCount: %d, iTotalFreeSpace: %d, iLastFreeCellLength: %d", iStats.iFreeCellCRC, iStats.iFreeCellCount, iStats.iTotalFreeSpace, iStats.iLastFreeCellLength));
        
        // Identify biggest cell
        if  ( (TUint) aLength > iStats.iLargestCellSizeFree )
            {
            PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::UpdateStats - this cell (%d bytes big) is bigger than previous largested FREE cell (%d bytes) => making it the new largest FREE cell", aLength, iStats.iLargestCellSizeFree));
            iStats.iLargestCellSizeFreePrevious = iStats.iLargestCellSizeFree;
            iStats.iLargestCellSizeFree = aLength;
            iStats.iLargestCellAddressFreePrevious = iStats.iLargestCellAddressFree;
            iStats.iLargestCellAddressFree = (TLinAddr) aCellAddress;
            }

        // Identify first cell
        if  ( iStats.iFirstFreeCellAddress == 0 )
            {
            iStats.iFirstFreeCellLength = aLength;
            iStats.iFirstFreeCellAddress = (TLinAddr) aCellAddress;
            }
        }
    else if ( aCellType == EMemSpyDriverGoodAllocatedCell )
        {
        // Track cell counts and length
        ++iStats.iAllocCellCount;
        iStats.iTotalAllocSpace += aLength;
        iStats.iLastFreeCellLength = 0; 

        PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::UpdateStats - WAS ALLOC CELL - iAllocCellCount: %d, iTotalAllocSpace: %d", iStats.iAllocCellCount, iStats.iTotalAllocSpace));

        // Identify biggest cell
        if  ( (TUint) aLength > iStats.iLargestCellSizeAlloc )
            {
            PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::UpdateStats - this cell (%d bytes big) is bigger than previous largested ALLOC cell (%d bytes) => making it the new largest ALLOC cell", aLength, iStats.iLargestCellSizeAlloc));
            iStats.iLargestCellSizeAlloc = aLength;
            iStats.iLargestCellAddressAlloc = (TLinAddr) aCellAddress;
            }
        }
    else
        {
        iStats.iLastFreeCellLength = aLength;
        }

    iStats.iLastCellType = aCellType;
    iStats.iLastCellAddress = (TLinAddr) aCellAddress;
    iStats.iLastCellWasFreeCell = ( aCellType == EMemSpyDriverGoodFreeCell );
    ++iStats.iNumberOfWalkedCells;
    }


void RMemSpyDriverHeapWalker::InitialiseStats()
    {
    iStats.iFreeCellCRC = 0;
    iStats.iNumberOfWalkedCells = 0;
    iStats.iFirstFreeCellAddress = 0;
    iStats.iFirstFreeCellLength = 0;
    iStats.iLastCellType = EMemSpyDriverGoodAllocatedCell;
    iStats.iLastCellWasFreeCell = EFalse;
    iStats.iLastFreeCellLength = 0;
    iStats.iTotalFreeSpace = 0;
    iStats.iTotalAllocSpace = 0;
    iStats.iSlackSpace = 0;
    iStats.iFreeCellCount = 0;
    iStats.iAllocCellCount = 0;
    iStats.iLargestCellSizeFree = 0;
    iStats.iLargestCellSizeAlloc = 0;
    iStats.iLargestCellAddressFree = 0;
    iStats.iLargestCellAddressAlloc = 0;
    iStats.iLargestCellSizeFreePrevious = 0;
    iStats.iLargestCellAddressFreePrevious = 0;
    iStats.iSpackSpaceCellAddress = 0;
    iStats.iLastCellAddress = 0;

    // These two can be identified up front
    iStats.iFreeCellOverheadHeaderLength = RMemSpyDriverRHeapBase::FreeCellHeaderSize();
    iStats.iAllocCellOverheadHeaderLength = RMemSpyDriverRHeapBase::AllocatedCellHeaderSize( iIsDebugAllocator );
    }


void RMemSpyDriverHeapWalker::FinaliseStats()
    {
    if  ( iStats.iLastCellWasFreeCell )
        {
        iStats.iSlackSpace = iStats.iLastFreeCellLength;
        iStats.iSpackSpaceCellAddress = iStats.iLastCellAddress;
        }

    PrintStats();
    }


void RMemSpyDriverHeapWalker::PrintStats()
    {
    PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::PrintStats - HEAP SUMMARY FOR THREAD:" ) );
    PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::PrintStats - ------------------------------------------------------------" ) );
    PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::PrintStats - iNumberOfWalkedCells         : %10d", iStats.iNumberOfWalkedCells ) );
    PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::PrintStats - iFirstFreeCellAddress        : 0x%08x", iStats.iFirstFreeCellAddress ) );
    PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::PrintStats - iFirstFreeCellLength         : %10d", iStats.iFirstFreeCellLength ) );
    PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::PrintStats - iLastCellWasFreeCell         : %10d", iStats.iLastCellWasFreeCell ) );
    PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::PrintStats - iLastCellType                : %10d", iStats.iLastCellType ) );
    PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::PrintStats - iLastFreeCellLength          : %10d", iStats.iLastFreeCellLength ) );
    PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::PrintStats - iTotalFreeSpace              : %10d", iStats.iTotalFreeSpace ) );
    PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::PrintStats - iTotalAllocSpace             : %10d", iStats.iTotalAllocSpace ) );
    PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::PrintStats - iSlackSpace                  : %10d", iStats.iSlackSpace ) );
    PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::PrintStats - iFreeCellCount               : %10d", iStats.iFreeCellCount ) );
    PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::PrintStats - iAllocCellCount              : %10d", iStats.iAllocCellCount ) );
    PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::PrintStats - iLargestCellSizeFree         : %10d", iStats.iLargestCellSizeFree ) );
    PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::PrintStats - iLastFreeCellLength          : %10d", iStats.iLastFreeCellLength ) );
    PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::PrintStats - iLargestCellSizeAlloc        : %10d", iStats.iLargestCellSizeAlloc ) );
    PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::PrintStats - iLargestCellAddressFree      : 0x%08x", iStats.iLargestCellAddressFree ) );
    PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::PrintStats - iLargestCellAddressAlloc     : 0x%08x", iStats.iLargestCellAddressAlloc ) );
    PRINTDEBUG( Kern::Printf("RMemSpyDriverHeapWalker::PrintStats - iFreeCellCRC                 : 0x%08x", iStats.iFreeCellCRC ) );
    }

