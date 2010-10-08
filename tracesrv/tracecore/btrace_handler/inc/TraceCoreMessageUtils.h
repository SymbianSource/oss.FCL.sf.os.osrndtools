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
// Utility functions for messaging
// 

#ifndef __TRACECOREISAUTILS_H__
#define __TRACECOREISAUTILS_H__


// Include files
#include <kernel/kernel.h>


// Forward declarations
class TTraceMessage;


/**
 * Utility functions for messaging
 */
class TTraceCoreMessageUtils
	{
public:
    /**
     * Get the message length 
     * 
     * @param aMsg Message to be sent.
     * @return length of one of symbian error codes
     */
    static TInt GetMessageLength( const TTraceMessage& aMsg );
    
    /**
     * Merges the header and data into a single buffer
     * 
     * @param aMsg Message to be sent.
     * @param aMsgBlock The message block where data is merged
     * @return Symbian error code
     */
    static TInt MergeHeaderAndData( const TTraceMessage& aMsg, TDes8& aTarget );
	};
    

#endif /*__TRACECOREISAUTILS_H__*/
