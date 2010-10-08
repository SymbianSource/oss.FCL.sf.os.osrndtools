// Copyright (c) 2007-2010 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
// Trace Core
//

#ifndef __TRACECORECOMMON_H__
#define __TRACECORECOMMON_H__

#include <opensystemtrace.h>

/**
 * Checks whether a given Group ID is valid or not
 *
 * @param aGroupId The group ID
 * @return ETrue if group ID is valid, EFalse if not
 */
inline TBool GroupIdIsValid( TUint32 aGroupId )
    {
    return aGroupId <= KMaxGroupId ;
    }

/**
 * Checks whether a given Component ID is valid or not
 *
 * @param aComponentId The component ID
 * @return ETrue if group ID is valid, EFalse if not
 */
inline TBool ComponentIdIsValid( TUint32 aComponentId )
    {
    return  aComponentId >0 ;
    }

/**
 * Converts aGroupId from a trace word to a group ID:
 * If aGroupId is 16-bit it is returned unchanged
 * If aGroupId is 32-bit the top 16-bits are returned
 * @param aGroupId The group ID
 * @return The part of the input parameter containing the group ID
 */
inline TUint32 FixGroupId( TUint32 aGroupId )
    {
    TUint32 groupId = aGroupId;
    // Get top 16 bits of groupId
    TUint32 top16bits = (groupId & 0xffff0000) >> GROUPIDSHIFT;
    // If top 16 bits are non-zero, assume they contain the Group ID
    if ( top16bits != 0 )
        {
        groupId = top16bits;
        }
    return groupId;
    }

#endif

// End of File
