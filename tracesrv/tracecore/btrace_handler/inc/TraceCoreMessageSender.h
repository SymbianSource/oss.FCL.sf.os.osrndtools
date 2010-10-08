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
// Interface used by subscribers to send messages
// 

#ifndef __TRACECOREMESSAGESENDER_H__
#define __TRACECOREMESSAGESENDER_H__


// Forward declarations
class TTraceMessage;


/**
 * Interface used by subscribers to send messages
 */
class MTraceCoreMessageSender
    {
public:

    /**
     * Sends a message
     * 
     * @param aMessage The message data
     */
    virtual TInt SendMessage( TTraceMessage &aMessage ) = 0;
    };

#endif

// End of File
