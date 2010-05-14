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

#ifndef MEMSPYDRIVEROBJECTSSHAREDRHEAP_H
#define MEMSPYDRIVEROBJECTSSHAREDRHEAP_H

// System includes
#include <u32std.h>
#ifndef __KERNEL_MODE__
#include <e32debug.h>
#endif

// User includes
#include <memspy/driver/memspydriverenumerationsshared.h>

// Defines
//#define __CHECK_CHUNK_NAME

// Classes referenced
class CActiveScheduler;
class CTrapCleanup;



class TMemSpyHeapObjectDataRHeap
    {
public:
    inline TMemSpyHeapObjectDataRHeap()
        : iAccessCount( 0 ),
          iHandleCount( 0 ),
          iHandles( NULL ),
          iFlags( 0 ),
          iCellCount( 0 ),
          iTotalAllocSize ( 0 ),
          //
          iMinLength( 0 ),
          iMaxLength( 0 ),
          iOffset ( 0 ),
          iGrowBy( 0 ),
          iChunkHandle ( 0 ),
          iBase( NULL ),
          iTop( NULL ),
          iAlign( 0 ),
          iMinCell( 0 ),
          iPageSize( 0 ),
          iNestingLevel( 0 ),
          iAllocCount( 0 ),
          iFailRate( 0 ),
          iFailed( EFalse ),
          iFailAllocCount( 0 ),
          iRand( 0 ),
          iTestData( NULL )
        {
        }

public: // API
    inline TUint8* Base() const { return iBase; }
    inline TUint Size() const { return iTop - iBase; }

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
#ifdef __SYMBIAN_KERNEL_HYBRID_HEAP__
	struct SCell { TInt len; SCell* next; };
    SCell iFree;
#else
    RHeap::SCell iFree;
#endif
	TInt iNestingLevel;
	TInt iAllocCount;
    RAllocator::TAllocFail iFailType;
	TInt iFailRate;
	TBool iFailed;
	TInt iFailAllocCount;
	TInt iRand;
	TAny* iTestData;
    };



/**
 * Base class for MemSpy RHeap statistics
 */
class TMemSpyHeapStatisticsRHeapBase
    {
public: // Type
    enum TType
        {
        ETypeAllocated = 0,
        ETypeFree
        };

public: // Constructors & destructor
    inline TMemSpyHeapStatisticsRHeapBase( TType aType )
        : iType( aType ), 
          iTypeCount( 0 ), 
          iTypeSize( 0 ),
          iLargestCellAddress( NULL ),
          iLargestCellSize( 0 )
        {
        }

public: // API
    inline TType Type() const { return iType; }
    //
    inline TUint TypeCount() const { return iTypeCount; }
    inline void SetTypeCount( TUint aValue ) { iTypeCount = aValue; }
    //
    inline TUint TypeSize() const { return iTypeSize; }
    inline void SetTypeSize( TUint aValue ) { iTypeSize = aValue; }
    //
    inline TAny* LargestCellAddress() const { return iLargestCellAddress; }
    inline void SetLargestCellAddress( TAny* aAddress ) { iLargestCellAddress = aAddress; }
    //
    inline TUint LargestCellSize() const { return iLargestCellSize; }
    inline void SetLargestCellSize( TUint aSize ) { iLargestCellSize = aSize; }

private: // Data members
    TType iType;
    TUint iTypeCount;
    TUint iTypeSize;
    TAny* iLargestCellAddress;
    TUint iLargestCellSize;
    };





/**
 * RHeap statistics for Allocated cell types
 */
class TMemSpyHeapStatisticsRHeapAllocated : public TMemSpyHeapStatisticsRHeapBase
    {
public: // Constructors
    inline TMemSpyHeapStatisticsRHeapAllocated()
        : TMemSpyHeapStatisticsRHeapBase( TMemSpyHeapStatisticsRHeapBase::ETypeAllocated )
        {
        }

public:
    };






/**
 * RHeap statistics for Free cell types
 */
class TMemSpyHeapStatisticsRHeapFree : public TMemSpyHeapStatisticsRHeapBase
    {
public: // Constructors
    inline TMemSpyHeapStatisticsRHeapFree()
        : TMemSpyHeapStatisticsRHeapBase( TMemSpyHeapStatisticsRHeapBase::ETypeFree ),
          iSlackSpace( 0 ), iSlackSpaceCellAddress( NULL ), iChecksum( 0 )
        {
        }

public: // API
    inline TUint SlackSpaceCellSize() const { return iSlackSpace; }
    inline void SetSlackSpaceCellSize( TUint aValue ) { iSlackSpace = aValue; }
    //
    inline TAny* SlackSpaceCellAddress() const { return iSlackSpaceCellAddress; }
    inline void SetSlackSpaceCellAddress( TAny* aAddress ) { iSlackSpaceCellAddress = aAddress; }
    //
    inline TUint32 Checksum() const { return iChecksum; }
    inline void SetChecksum( TUint32 aValue ) { iChecksum = aValue; }

public:
    TUint iSlackSpace;
    TAny* iSlackSpaceCellAddress;
    TUint32 iChecksum;
    };





/**
 * RHeap statistics for common cell types
 */
class TMemSpyHeapStatisticsRHeapCommon
    {
public: // Constructors
    inline TMemSpyHeapStatisticsRHeapCommon()
        : iTotalCellCount( 0 )
        {
        }

public:
    inline TUint TotalCellCount() const { return iTotalCellCount; }
    inline void SetTotalCellCount( TUint aValue ) { iTotalCellCount = aValue; }

private:
    TUint iTotalCellCount;
    };





/**
 * RHeap statistics class
 */
class TMemSpyHeapStatisticsRHeap
    {
public: // Constructors
    inline TMemSpyHeapStatisticsRHeap()
        {
        }

public: // API
    inline TMemSpyHeapStatisticsRHeapFree& StatsFree() { return iStatisticsFree; }
    inline const TMemSpyHeapStatisticsRHeapFree& StatsFree() const { return iStatisticsFree; }
    //
    inline TMemSpyHeapStatisticsRHeapAllocated& StatsAllocated() { return iStatisticsAllocated; }
    inline const TMemSpyHeapStatisticsRHeapAllocated& StatsAllocated() const { return iStatisticsAllocated; }
    //
    inline TMemSpyHeapStatisticsRHeapCommon& StatsCommon() { return iStatisticsCommon; }
    inline const TMemSpyHeapStatisticsRHeapCommon& StatsCommon() const { return iStatisticsCommon; }

private: // Data members
    TMemSpyHeapStatisticsRHeapCommon iStatisticsCommon;
    TMemSpyHeapStatisticsRHeapFree iStatisticsFree;
    TMemSpyHeapStatisticsRHeapAllocated iStatisticsAllocated;
    };







/**
 *
 */
class TMemSpyHeapMetaDataRHeap
    {
public: // Constructors
    inline TMemSpyHeapMetaDataRHeap()
        : iChunkSize( 0 ),
          iChunkHandle( NULL ),
          iChunkBaseAddress( NULL ),
          iDebugAllocator( EFalse ),
          iHeaderSizeFree( 0 ),
          iHeaderSizeAllocated( 0 ),
          iIsUserThread( ETrue ),
          iSharedHeap( EFalse )
        {
        }

public: // API
#ifndef __KERNEL_MODE__
    inline TPtrC ChunkName() const
        {
        TBool alreadyExpanded = ETrue;

#ifdef __CHECK_CHUNK_NAME
        {
        RDebug::Printf( "TMemSpyHeapMetaDataRHeap::ChunkName() - BEF - length: %d", iChunkName.Length() );
        const TUint8* data2 = iChunkName.Ptr();
        const TInt count = iChunkName.Length();
        for( TInt i=0;i<count;i++)
            {
            RDebug::Printf( "TMemSpyHeapMetaDataRHeap::ChunkName() - BEF - data[%03d] = 0x%08x = %02x (%c)", i, data2, *data2, *data2 );
            ++data2;
            }
        }
#endif

        // Check if it already has been converted to unicode
        const TUint8* data = iChunkName.Ptr();
        if ( iChunkName.Length() >= 1 )
            {
            const TUint8 highByte = *(data + 1);
            alreadyExpanded = ( highByte == 0 ); 
            }

        // Once expanded, the descriptor is twice the length
        TPtrC ret( (TUint16*) iChunkName.Ptr(), iChunkName.Length() / 2 );
        //
        if ( !alreadyExpanded )
            {
            const TInt halfLength = iChunkName.Length() / 2;
            TMemSpyHeapMetaDataRHeap* self = const_cast< TMemSpyHeapMetaDataRHeap* >( this );
            ret.Set( self->iChunkName.Expand() );
            }

#ifdef __CHECK_CHUNK_NAME
        {
        RDebug::Printf( "TMemSpyHeapMetaDataRHeap::ChunkName() - AFT - length: %d", iChunkName.Length() );
        const TUint8* data2 = iChunkName.Ptr();
        const TInt count = iChunkName.Length();
        for( TInt i=0;i<count;i++)
            {
            RDebug::Printf( "TMemSpyHeapMetaDataRHeap::ChunkName() - AFT - data[%03d] = 0x%08x = %02x (%c)", i, data2, *data2, *data2 );
            ++data2;
            }
        const TPtrC pNextRet( (TUint16*) iChunkName.Ptr(), iChunkName.Length() / 2 );
        RDebug::Print( _L("TMemSpyHeapMetaDataRHeap::ChunkName() - ret: [%S], nextRet: [%S]"), &ret, &pNextRet );
        }
#endif
        //
        return ret;
        }

    inline void SetChunkName( const TDesC16& aValue )
        {
        iChunkName.Copy( aValue );
        iChunkName.Collapse();
        }
#else
    inline void SetChunkName( const TDesC8& aValue )
        {
        iChunkName.Copy( aValue );
        }
#endif
    //
    inline TUint ChunkSize() const { return iChunkSize; }
    inline void SetChunkSize( TUint aValue ) { iChunkSize = aValue; }
    //
    inline TAny* ChunkHandle() const { return iChunkHandle; }
    inline void SetChunkHandle( TAny* aValue ) { iChunkHandle = aValue; }
    //
    inline TAny* ChunkBaseAddress() const { return iChunkBaseAddress; }
    inline void SetChunkBaseAddress( TAny* aValue ) { iChunkBaseAddress = aValue; }
    //
    inline TUint HeaderSizeFree() const { return iHeaderSizeFree; }
    inline void SetHeaderSizeFree( TUint aValue ) { iHeaderSizeFree = aValue; }
    //
    inline TUint HeaderSizeAllocated() const { return iHeaderSizeAllocated; }
    inline void SetHeaderSizeAllocated( TUint aValue ) { iHeaderSizeAllocated = aValue; }
    //
    inline TBool IsDebugAllocator() const { return iDebugAllocator; }
    inline void SetDebugAllocator( TBool aValue ) { iDebugAllocator = aValue; }
    //
    inline TBool IsUserThread() const { return iIsUserThread; }
    inline void SetUserThread( TBool aValue ) { iIsUserThread = aValue; }
    //
    inline TBool IsSharedHeap() const { return iSharedHeap; }
    inline void SetSharedHeap( TBool aValue ) { iSharedHeap = aValue; }
    //
    inline TUint VTable() const { return iVTable; }
    inline void SetVTable( TUint aValue ) { iVTable = aValue; }
    //
    inline TUint ClassSize() const { return iClassSize; }
    inline void SetClassSize( TUint aValue ) { iClassSize = aValue; }

private: // Data members
    TBuf8< KMaxFullName * 2 > iChunkName;
    TUint iChunkSize;
    TAny* iChunkHandle;
    TAny* iChunkBaseAddress;
    TBool iDebugAllocator;
    TUint iHeaderSizeFree;
    TUint iHeaderSizeAllocated;
    TBool iSharedHeap;
    TBool iIsUserThread;
    TUint iVTable;
    TUint iClassSize;
    };







/**
 *
 */
class TMemSpyHeapInfoRHeap
    {
public: // Constructors
    inline TMemSpyHeapInfoRHeap()
        {
        }

public: // API
    inline TMemSpyHeapMetaDataRHeap& MetaData() { return iMetaData; }
    inline const TMemSpyHeapMetaDataRHeap& MetaData() const { return iMetaData; }
    //
    inline TMemSpyHeapObjectDataRHeap& ObjectData() { return iObjectData; }
    inline const TMemSpyHeapObjectDataRHeap& ObjectData() const { return iObjectData; }
    //
    inline TMemSpyHeapStatisticsRHeap& Statistics() { return iStatistics; }
    inline const TMemSpyHeapStatisticsRHeap& Statistics() const { return iStatistics; }

private: // Data members
    TMemSpyHeapMetaDataRHeap iMetaData;
    TMemSpyHeapObjectDataRHeap iObjectData;
    TMemSpyHeapStatisticsRHeap iStatistics;
    };





#endif
