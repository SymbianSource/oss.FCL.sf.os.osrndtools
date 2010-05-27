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
* Description:  Definitions for the class CATDynProcessInfo
*
*/


// INCLUDE FILES
#include    "atdynprocessinfo.h"

// ============================ MEMBER FUNCTIONS ===============================


// -----------------------------------------------------------------------------
// CATDynProcessInfo::CATDynProcessInfo
// -----------------------------------------------------------------------------
//  
CATDynProcessInfo::CATDynProcessInfo() :
    iProcessId( KNullProcessId ),
    iSessionObject( NULL )
    {
    }

// -----------------------------------------------------------------------------
// CATDynProcessInfo::CATDynProcessInfo
// -----------------------------------------------------------------------------
//  
CATDynProcessInfo::CATDynProcessInfo( TUint aProcessId,
                                      CATStorageServerSession* aSessionObject,
                                      const RArray<TATDllInfo>& aDlls ) :
    iProcessId( aProcessId ),
    iSessionObject( aSessionObject ),
    iDlls( aDlls )
    {
    }

// -----------------------------------------------------------------------------
// CATDynProcessInfo::CATDynProcessInfo
// -----------------------------------------------------------------------------
//  
CATDynProcessInfo::CATDynProcessInfo( TUint aProcessId,
                                      CATStorageServerSession* aSessionObject ) :
    iProcessId( aProcessId ), iSessionObject( aSessionObject )
    {
    }

// -----------------------------------------------------------------------------
// CATDynProcessInfo::CATDynProcessInfo
// -----------------------------------------------------------------------------
//  
CATDynProcessInfo::CATDynProcessInfo( TUint aProcessId ) :
    iProcessId( aProcessId ), iSessionObject( NULL )
    {
    }

// Destructor
CATDynProcessInfo::~CATDynProcessInfo()
    {
    iDlls.Close();
    }

// -----------------------------------------------------------------------------
// CATDynProcessInfo::Compare
// -----------------------------------------------------------------------------
//  
TInt CATDynProcessInfo::Compare( const CATDynProcessInfo& aFirst, 
	const CATDynProcessInfo& aSecond )
    {
    if ( aFirst.iProcessId < aSecond.iProcessId )
        {
        return -1;
        }
    
    if ( aFirst.iProcessId > aSecond.iProcessId )
        {
        return 1;
        }
   
    // else
    return 0;
    }

// End of File
