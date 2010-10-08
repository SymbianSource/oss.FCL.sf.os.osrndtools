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
// Media interface base class
//

#ifndef __TRACECOREMEDIAIF_H__
#define __TRACECOREMEDIAIF_H__


// Include files
#include <kernel/kernel.h>


// Forward declarations
class MTraceCoreMediaIfCallback;
class TTraceMessage;

/**
 * Flag, which tells that this interface supports the Send function
 */
const TUint32 KMediaIfSendSupported        = 0x01;

/**
 * Flag, which tells that this interface supports the SendTrace function
 */
const TUint32 KMediaIfSendTraceSupported   = 0x02;

/**
 * Flag for the plug-in interface
 */
const TUint32 KMediaIfPluginSupported      = 0x04;


/**
 * Media interface base class
 */
NONSHARABLE_CLASS( DTraceCoreMediaIf ) : public DBase
    {
public:

    /**
     * Constructor
     * 
     * @param aFlags the media flags
     */
    DTraceCoreMediaIf( TUint32 aFlags );
    
    /**
     * Initializes this interface
     * 
     * @param aCallback Callback interface which is notified when ISI messages are received
     */
	virtual TInt Init( MTraceCoreMediaIfCallback& aCallback ) = 0;

    /**
     * Gets the properties of this interface
     */
    inline TUint32 GetFlags();
    
	/**
	 * Sends a message
	 *
	 * @param aMsg The message to be sent
	 */
	virtual TInt Send( TTraceMessage& aMsg );
    
    /**
     * Sends a trace
     *
     * @param aMsg The trace to be sent
     */
    virtual TInt SendTrace( const TDesC8& aMsg );
    
protected:

	/**
	 * Callback to TraceCore
	 */
	MTraceCoreMediaIfCallback* iCallback;
    
private:

    /**
     * Media properties
     */
    TUint32 iFlags;
    };

#endif // __TRACECOREISAIF_H__

#include "TraceCoreMediaIf.inl"

// End of File
