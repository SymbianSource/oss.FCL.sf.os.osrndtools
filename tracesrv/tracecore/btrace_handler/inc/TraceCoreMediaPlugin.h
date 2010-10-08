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
// Interface class for media plug-ins
//

#ifndef __TRACECOREMEDIAPLUGIN_H__
#define __TRACECOREMEDIAPLUGIN_H__


// Include files
#include <kernel/kernel.h>

// Forward declarations
class DTraceCorePluginIf;
class TTraceMessage;


/**
 * Media plug-in base class
 */
class DTraceCoreMediaPlugin : public DBase
    {
public:

    /**
     * Constructor
     */
    IMPORT_C DTraceCoreMediaPlugin();

    /**
     * Destructor
     */
    IMPORT_C ~DTraceCoreMediaPlugin();

    /**
     * Registers this plug-in to TraceCore
     */
    IMPORT_C TInt Register();
    
    /**
     * Called by the plug-in to send an incoming message to TraceCore
     * 
     * @param aMsg The message that was received
     */
    IMPORT_C void MessageReceived( TTraceMessage &aMsg );
    
    /**
     * Merges the header and data into a single buffer
     * 
     * @param aMsg Message to be sent.
     * @param aMsgBlock The message block where data is merged
     * @return KErrNone if successful
     */
    IMPORT_C TInt MergeHeaderAndData( const TTraceMessage& aMsg, TDes8& aTarget );

    /**
     * Called by TraceCore to send a message out
     * 
     * @param aMsg The message to be sent
     */
    virtual TInt SendMessage( TTraceMessage &aMsg ) = 0;

private:

    /**
     * Unregisters this plug-in from TraceCore. Called from destructor
     */
    void Unregister();
    };

#endif // __TRACECOREMEDIAPLUGIN_H__

// End of File
