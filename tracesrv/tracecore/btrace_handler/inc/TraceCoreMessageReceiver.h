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
// Incoming message notifications
// 

#ifndef __TRACECOREMESSAGERECEIVER_H__
#define __TRACECOREMESSAGERECEIVER_H__

//Forward declarations
class TTraceMessage;


/**
 * Incoming message notifications
 */
NONSHARABLE_CLASS( MTraceCoreMessageReceiver )
    {
public:

    /**
     * Message received notification
     * 
     * @param aMsg The message that was received
     * @return KErrNotFound if the message id was not found (not subscribed), KErrNone if found
     */
    virtual TInt MessageReceived( TTraceMessage &aMsg ) = 0;
    };

#endif

// End of File
