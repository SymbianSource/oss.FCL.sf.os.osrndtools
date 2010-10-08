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
// TraceCore configuration message subscriber
//

#ifndef __TRACECOREINTERNALMESSAGEHANDLER_H__
#define __TRACECOREINTERNALMESSAGEHANDLER_H__


// Include files
#include "TraceCoreSubscriber.h"


/**
 * Trace configuration. This extends the subscriber base class to receive configuration requests
 */
NONSHARABLE_CLASS( DTraceCoreInternalMessageHandler ) : public DTraceCoreSubscriber
    {
public:

    /**
     * Constructor
     */
    DTraceCoreInternalMessageHandler();
    
    /**
     * Destructor
     */
    ~DTraceCoreInternalMessageHandler();
    
    /**
     * Initializes configuration
     */
    TInt Init();
    
    /**
     * Message received callback
     *
     * @param aMessage The message
     */
    void MessageReceived( TTraceMessage& aMsg );
    
private:

    /**
     * Sends a response message
     * 
     * @param aMsg the incoming message
     * @param aResult the result code
     */
    void SendResponse( TTraceMessage& aMsg, TInt aResult );
    
    /**
     * Handle version query
     * 
     * @param aMsg the incoming message
     * @param aResult the result code
     */
    void HandleOstVersionQuery( TTraceMessage& aMsg, TInt aMessageId );

    };

#endif // __TRACECOREINTERNALMESSAGEHANDLER_H__

// End of File
