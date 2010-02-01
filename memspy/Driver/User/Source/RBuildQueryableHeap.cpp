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

#include "RBuildQueryableHeap.h"

// System includes
#include <e32std.h>
#include <e32debug.h>


TBool RBuildQueryableHeap::IsDebugEUser() const
    {
    User::__DbgSetAllocFail( FALSE, RAllocator::EFailNext, 1);

    TInt* cell = new TInt();
#if defined(_DEBUG) && !defined( __WINS__ )
    RDebug::Printf("RBuildQueryableHeap::IsDebugEUser() - cell: 0x%08x", cell);
#endif

    const TBool debugEUser = ( cell == NULL );
#if defined(_DEBUG) && !defined( __WINS__ )
    RDebug::Printf("RBuildQueryableHeap::IsDebugEUser() - debugEUser: %d", debugEUser);
#endif

    delete cell;
    //
    return debugEUser;
    }


TInt RBuildQueryableHeap::CellHeaderSize( TMemSpyDriverCellType aType ) const
    {
	TInt size = 0;
	//
	switch( aType )
	{
	case EMemSpyDriverGoodAllocatedCell:
	case EMemSpyDriverBadAllocatedCellSize:
	case EMemSpyDriverBadAllocatedCellAddress:
		{
		size = sizeof( TInt ); // Allocated UREL cells contain just a length
		if  ( IsDebugEUser() )
			{
			size = sizeof( RHeap::SDebugCell );
			}
		break;
		}
	case EMemSpyDriverGoodFreeCell:
	case EMemSpyDriverBadFreeCellAddress:
	case EMemSpyDriverBadFreeCellSize:
		size = EFreeCellSize;
		break;
	default:
		break;
	}
    //
    return size;
    }

