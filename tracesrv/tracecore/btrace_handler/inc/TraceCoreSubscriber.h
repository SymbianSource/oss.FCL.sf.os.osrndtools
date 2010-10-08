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
// Base class for subscribers of TraceCore
//

#ifndef __TRACECORESUBSCRIBER_H__
#define __TRACECORESUBSCRIBER_H__

#ifdef __KERNEL_MODE__
// Include files
#include <kernel/kernel.h>
#endif // __KERNEL_MODE__

/**
 * Header format
 */	
enum TMessageHeaderFormat
	{
	/**
	 * Proprietary Format (Header Format 0 is also considered as proprietary)
	 */
	EMessageHeaderFormatProprietary = 1,
	EMessageHeaderFormatOst         = 2
	
	};


/**
 * Trace Message
 */
class TTraceMessage
	{
public:
	TMessageHeaderFormat iMsgFormat;
	TDesC8 *iHeader;
	TDesC8 *iData;
	TUint8 iMessageId;
	};

#ifdef __KERNEL_MODE__

// Forward declarations
class MTraceCoreMessageSender;

/**
 * Base class for subscribers of TraceCore
 */
class DTraceCoreSubscriber : public DBase
    {
public:

    /**
     * Constructor
     */
    IMPORT_C DTraceCoreSubscriber();

    /**
     * Destructor
     */
    IMPORT_C ~DTraceCoreSubscriber();
    
    /**
     * Subscribes to a message by message ID / protocol ID
     * depending of the header format. First byte of aMessageID is always used
     * for header format.
     * 
     * @param aMessageId The message ID to be subscribed
     * @param aMsgFormat Message format e.g. EMessageHeaderFormatOst
     */
    IMPORT_C TInt Subscribe( TUint32 aMessageID, TMessageHeaderFormat aHeaderFormat );
    
    /**
     * Subscribes to message ID
     * 
     * @param aMessageId The ID to subscribe
     */
    IMPORT_C TInt Subscribe( TUint32 aMessageID );

    /**
     * Unsubscribes from message ID
     * 
     * @param aMessageId The ID to unsubscribe
     */
    IMPORT_C void Unsubscribe( TUint32 aMessageID );
    
    /**
     * Callback function for messages
     */
    virtual void MessageReceived( TTraceMessage &aMsg ) = 0;

    /**
     * Sets the message sender interface. This is called during Subscribe
     */
    void SetMessageSender( MTraceCoreMessageSender& aMessageSender );
    
private:

    /**
     * Unsubscribes all message ID's of this subscriber. Called from destructor
     */
    void Unsubscribe();

protected:

    /**
     * Message sender interface
     */
    MTraceCoreMessageSender* iMessageSender;
    };

#endif // __KERNEL_MODE__

#endif // __TRACECORESUBSCRIBER_H__

// End of File
