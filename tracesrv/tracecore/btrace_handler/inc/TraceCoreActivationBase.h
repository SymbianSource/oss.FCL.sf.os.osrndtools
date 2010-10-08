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
// Base class for activation classes
//

#ifndef __TRACECOREACTIVATIONBASE_H__
#define __TRACECOREACTIVATIONBASE_H__


// Include files
#include "TraceCoreSubscriber.h"
#include "TraceCoreActivation.h"

//forward declarations
class DMutex;

/**
 * Base class for activation objects. This extends the subscriber base class to receive activation updates
 */
NONSHARABLE_CLASS( DTraceCoreActivationBase ) : public DTraceCoreSubscriber, public MTraceCoreActivation
    {
public:

    /**
     * Constructor
     */
    DTraceCoreActivationBase();
    
    /**
     * Destructor
     */
    virtual ~DTraceCoreActivationBase();
    
    /**
     * Registers an activation notification interface
     * 
     * @param aNotification the notification interface
     */
    TInt RegisterActivationNotification( MTraceCoreActivationNotification& aNotification );
    
protected:

    /**
     * Subscribes to message ID and registers to activation interface list
     * 
     * @param aMessageID the message id to be subscribed
     */         
    TInt Init( TUint32 aMessageId );

    /**
     * Subscribes to message ID and registers to activation interface list
     * 
     * @param aMessageID the message id to be subscribed
     */
    TInt SubscribeMessage( TUint32 aMessageId, TMessageHeaderFormat aMsgFormat );

    /**
     * Sends notification to TraceCore internal activation notification interfaces
     * 
     * @param aFromSettings ETrue if activation was due to settings read, EFalse if from some other source
     * @param aComponentId Component ID of the activation
     */
    void NotifyInternalActivation( TBool aFromSettings, TUint32 aComponentId );
    
    /**
     * Send response
     * 
     * @param aMessage Response message
     */
    void SendResponse( TTraceMessage &aMessage );
    
    /**
     * Prevent read-check to internal structures (fast lock).
     */
    inline TInt ReadLock();
    
    /**
     * Restore read-access to internal structures.
     */
    inline void ReadUnlock(TInt aIrqLevel);
    
    
protected: //data
    
    /**
     * Lock used in 'critical path' where performance is very important.
     * It is used in those routines where structures are checked and not modified.
     */
#if defined(__SMP__)
    TSpinLock   iActivationReadLock;
#endif
    
    /**
     * Used to store Irq level between spin lock/unlock operation.
     */
    TInt iIrqLevel;
    
private:

    /**
     * Array of activation notification interfaces
     */
    RPointerArray< MTraceCoreActivationNotification > iActivationNotification;
    };


#include "TraceCoreActivationBase.inl"


#endif /* __TRACECOREACTIVATIONBASE_H__ */
