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
* Description:  Definitions for the class TATDllInfo.
*
*/

// INCLUDE FILES
#include "atdllinfo.h"
#include "atlog.h"

// -----------------------------------------------------------------------------
// TATDllInfo::TATDllInfo
// C++ default constructor.
// -----------------------------------------------------------------------------
//
TATDllInfo::TATDllInfo( const TUint32 aStartAddress, const TUint32 aEndAddress,
	const TInt64& aLoadTime, const TDesC8& aDllName  )
 :  iStartAddress( aStartAddress ), iEndAddress( aEndAddress ), 
	iLoadTime( aLoadTime )
    {
    LOGSTR3( "STSE TATDllInfo::TATDllInfo() %x - %x", iStartAddress, iEndAddress );
    
    iName.Copy( aDllName );
    }

// -----------------------------------------------------------------------------
// TATDllInfo::StartAddress
// Returns library start address
// -----------------------------------------------------------------------------
//
TUint32 TATDllInfo::StartAddress()
    {
    LOGSTR2( "STSE TATDllInfo::StartAddress( %x )", iStartAddress );
    
    return iStartAddress;
    }

// -----------------------------------------------------------------------------
// TATDllInfo::EndAddress
// Returns library end address
// -----------------------------------------------------------------------------
//
TUint32 TATDllInfo::EndAddress()
    {
    LOGSTR2( "STSE TATDllInfo::EndAddress( %x )", iEndAddress );
    
    return iEndAddress;
    }

// -----------------------------------------------------------------------------
// TATDllInfo::Name
// Gets specific library name
// -----------------------------------------------------------------------------
//
TDes8& TATDllInfo::Name()
    {
    LOGSTR1( "STSE TATDllInfo::Name()" );
    
    return iName;
    }

// -----------------------------------------------------------------------------
// TATDllInfo::LibraryLoadTime
// Returns library load time
// -----------------------------------------------------------------------------
//
TInt64 TATDllInfo::LibraryLoadTime()
    {
    LOGSTR1( "STSE TATDllInfo::LibraryLoadTime()" );
    
    return iLoadTime;
    }
        
// -----------------------------------------------------------------------------
// TATDllInfo::Match
// Checks if two objects of this class match based on the objects's
// saved library name.
// -----------------------------------------------------------------------------
//
TBool TATDllInfo::Match( const TATDllInfo& aFirst, const TATDllInfo& aSecond )
    {
    LOGSTR1( "STSE TATDllInfo::Match()" );
    
    if ( aFirst.iName.Compare( aSecond.iName ) == 0 )
        {
        return ETrue;
        }
    return EFalse;
    }

// End of File
