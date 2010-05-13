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
* Description:  Definitions for the class CATMemoryEntry
*
*/



// INCLUDE FILES
#include    "atmemoryentry.h"


// ============================ MEMBER FUNCTIONS ===============================


// -----------------------------------------------------------------------------
// CATMemoryEntry::CATMemoryEntry
// C++ default constructor.
// -----------------------------------------------------------------------------
//  
CATMemoryEntry::CATMemoryEntry( TUint32 aMemAddress,
                                const CBufFlat* aCallstackBuf,
                                const TInt64& aAllocTime,
                                TInt aAllocSize ) :
    iMemAddress( aMemAddress ),
    iCallstackBuf( aCallstackBuf ),
    iAllocTime( aAllocTime ),
    iAllocSize( aAllocSize )
    {
    }

// Destructor
CATMemoryEntry::~CATMemoryEntry()
    {
    delete const_cast<CBufFlat*>( iCallstackBuf );
    }

// -----------------------------------------------------------------------------
// CATMemoryEntry::Compare
// -----------------------------------------------------------------------------
//  
TInt CATMemoryEntry::Compare( const CATMemoryEntry& aFirst, const CATMemoryEntry& aSecond )
    {
    if ( aFirst.iAllocTime < aSecond.iAllocTime )
        {
        return -1;
        }
    
    if ( aFirst.iAllocTime > aSecond.iAllocTime )
        {
        return 1;
        }
   
    // else
    return 0;
    }

// -----------------------------------------------------------------------------
// CATMemoryEntry::Match
// -----------------------------------------------------------------------------
//
TBool CATMemoryEntry::Match( const CATMemoryEntry& aFirst, const CATMemoryEntry& aSecond )
    {
    if ( aFirst.iMemAddress == aSecond.iMemAddress )
        {
        return ETrue;
        }
    
    // else
    return EFalse;
    }


