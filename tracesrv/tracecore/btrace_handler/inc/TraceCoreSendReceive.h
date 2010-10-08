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
// Send-receive class abstracts the media used by subscribers to send / receive messages
//

#ifndef __TRACECORESENDRECEIVE_H__
#define __TRACECORESENDRECEIVE_H__


// Include files
#include <kernel/kernel.h>
#include "TraceCoreMediaIfCallback.h"
#include "TraceCoreMessageSender.h"


// Forward declarations
class DTraceCoreMediaIf;
class DTraceCorePluginIf;
class MTraceCoreMessageReceiver;


/**
 * Send-receive class abstracts the media used by subscribers to send / receive messages
 */
NONSHARABLE_CLASS( DTraceCoreSendReceive ) : public DBase, public MTraceCoreMediaIfCallback, public MTraceCoreMessageSender
    {
public:

    /**
     * Constructor
     */
    DTraceCoreSendReceive();
    
    /**
     * Destructor
     */
    ~DTraceCoreSendReceive();
    
    /**
     * Initializes this sender / receiver
     * 
     * @param aReceiver Interface where incoming messages are routed
     */
    TInt Init( MTraceCoreMessageReceiver& aReceiver );
    
    /**
     * Gets the media interface that is used by media writer
     */
    inline DTraceCoreMediaIf* GetMediaWriterIf();
    
    /**
     * Gets the media plug-in interface or NULL if it does not exist
     */
    inline DTraceCorePluginIf* GetPluginIf();
    
private:

    /**
     * MTraceCoreMediaIfCallback implementation for incoming messages
     * 
     * @param aMsg The message that was received
     * @return KErrNotFound if the message id was not found (not subscribed), KErrNone if found
     */
    TInt MessageReceived( TTraceMessage &aMsg );
    
    /**
     * Set Sender Media
     *
     * @param Sender media
     */
    TInt SetSenderMedia( DTraceCoreMediaIf* aSenderMedia );
    
    /**
     * MTraceCoreMessageSender implementation for outgoing messages
     * 
     * @param aMessage The message to be sent
     */
    TInt SendMessage( TTraceMessage &aMessage );

    /**
     * Initializes a media interface
     */
    TInt InitMediaInterface( DTraceCoreMediaIf* aMediaIf );

    /**
     * Creates the media interfaces
     */
    TInt CreateInterfaces();
    
    /**
     * Deletes the media interfaces
     */
    void DeleteInterfaces();

private:
    
    /**
     * Media interfaces that receive data.
     */
    RPointerArray< DTraceCoreMediaIf > iMediaInterfaces;
    
    /**
     * Plug-in interface
     */
    DTraceCorePluginIf* iPluginInterface;

    /**
     * Media interface for sending data
     */
    DTraceCoreMediaIf* iSendInterface;

    /**
     * Media interface used by the media writer
     */
    DTraceCoreMediaIf* iMediaWriterInterface;

    /**
     * Target for incoming messages
     */
    MTraceCoreMessageReceiver* iReceiver;
    };

#endif

#include "TraceCoreSendReceive.inl"

// End of File
