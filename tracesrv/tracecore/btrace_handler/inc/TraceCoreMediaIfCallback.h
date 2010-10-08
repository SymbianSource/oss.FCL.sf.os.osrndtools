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
// Callback interface for incoming messages from media interface
//

#ifndef __TRACECOREMEDIAIFCALLBACK_H__
#define __TRACECOREMEDIAIFCALLBACK_H__

// Forward declarations
class TTraceMessage;
class DTraceCoreMediaIf;

/**
 * Callback interface for incoming messages from media interface
 */
NONSHARABLE_CLASS( MTraceCoreMediaIfCallback )
    {
public:

    /**
     * Message was received
     * 
     * @param aMsg The message that was received
     * @return KErrNotFound if the message id was not found (not subscribed), KErrNone if found
     */
    virtual TInt MessageReceived( TTraceMessage &aMsg ) = 0;
    
    /**
     * Set sender media
     * 
     * @param aSenderMedia Pointer to the new sender
     * @return KErrNone if media change ok
     */
    virtual TInt SetSenderMedia( DTraceCoreMediaIf* aSenderMedia ) = 0;
    };

#endif

// End of File
