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

#ifndef __TRACECOREPLUGINIF_H__
#define __TRACECOREPLUGINIF_H__


// Include files
#include <kernel/kernel.h>
#include "TraceCoreMediaIf.h"


// Forward declarations
class DTraceCoreMediaPlugin;
class TTraceMessage;

/**
 * Interface class for media plug-ins
 */
NONSHARABLE_CLASS( DTraceCorePluginIf ) : public DTraceCoreMediaIf
    {
public:

    /**
     * Constructor
     */
    DTraceCorePluginIf();
    
    /**
     * Destructor
     */
    ~DTraceCorePluginIf();
    
    /**
     * Initializes this interface
     * 
     * @param aCallback Callback interface which is notified when ISI messages are received
     */
	TInt Init( MTraceCoreMediaIfCallback& aCallback );
	
	/**
	 * Sends a message
	 *
	 * @param aMsg The message to be sent
	 */
	TInt Send( TTraceMessage& aMsg );
	
	/**
	 * Registers a media plug-in
	 */
	TInt RegisterPlugin( DTraceCoreMediaPlugin& aPlugin );
	
	/**
	 * Unregisters a media plug-in
	 */
	void UnregisterPlugin( DTraceCoreMediaPlugin& aPlugin );
	
	/**
	 * Message was received by plug-in
     * 
     * @param aMsg The message that was received
	 */
	void MessageReceived( TTraceMessage &aMsg );
	
private:

    /**
     * Media plug-in. Currently only one supported
     */
    DTraceCoreMediaPlugin* iMediaPlugin;
    };

#endif // __TRACECOREPLUGINIF_H__

// End of File
