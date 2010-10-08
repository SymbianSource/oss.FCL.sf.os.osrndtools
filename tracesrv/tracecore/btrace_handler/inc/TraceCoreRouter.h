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
// Router receives subscriptions and delegates messages to them
//

#ifndef __TRACECOREROUTER_H__
#define __TRACECOREROUTER_H__


// Include files
#include <kernel/kernel.h>
#include "TraceCoreMessageReceiver.h"


// Forward declarations
class DTraceCoreSendReceive;
class DTraceCoreSubscriber;
class MTraceCoreMessageSender;

/**
 * Routing item
 */
NONSHARABLE_CLASS( TRoutingItem )
    {
public:

    /**
     * Message ID
     * Higest bit (MSB) is set when subscribed OST Protocol Id
     */
    TUint32 iMessageID;
    
    /**
     * The subscriber associated with the message ID
     */
    DTraceCoreSubscriber* iSubscriber;
    
    /**
     * Header format (TMessageHeaderFormat)
     */
    TUint32 iMsgFormat;
    
    };


/**
 * Router receives subscriptions and delegates messages to them
 */
NONSHARABLE_CLASS( DTraceCoreRouter ) : public DBase, public MTraceCoreMessageReceiver
    {
public:

    /**
     * Constructor
     */
    DTraceCoreRouter();
    
    /**
     * Destructor
     */
    ~DTraceCoreRouter();
    
    /**
     * Initializes this router
     *
     * @param aSender Interface to send messages
     */
    TInt Init( MTraceCoreMessageSender& aSender );
    
    /**
     * Subscribes to a message
     *
     * @param aItem The properties of the subscription
     */
	TInt Subscribe( TRoutingItem& aItem );
	
	/**
	 * Unsubscribes from a message
	 *
	 * @param aItem The message to be unregistered
	 */
	void Unsubscribe( TRoutingItem& aItem );
	
	/**
	 * Unsubscribes all messages of given subscriber
	 *
	 * @param aSubscriber The subscriber to be unregistered
	 */
	void Unsubscribe( DTraceCoreSubscriber& aSubscriber );
	
	/**
	 * Callback when message is received
     * 
     * @param aMsg The message that was received
     * @return KErrNotFound if the message id was not found (not subscribed), KErrNone if found
	 */
    TInt MessageReceived( TTraceMessage &aMsg );

private:

    /**
     * Active subscriptions
     */
	RArray< TRoutingItem > iRoutingItems;

    /**
     * Sender abstracts the messaging interface. 
     * The pointer is passed to subscribers during call to Subscribe
     */
    MTraceCoreMessageSender* iMessageSender;
    
    /**
     * Lock to protect internal structurees.
     */
    DMutex* iLock;
    };

#endif

// End of File
