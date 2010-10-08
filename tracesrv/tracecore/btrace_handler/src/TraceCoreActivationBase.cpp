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
// Base class for activation objects
//
#ifdef __SMP__
#include <nkernsmp/nkern.h>
#endif
#include "TraceCoreActivationBase.h"
#include "TraceCore.h"
#include "TraceCoreDebug.h"
#include "TraceCoreMessageSender.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "TraceCoreActivationBaseTraces.h"
#endif



/**
 * Constructor
 */
DTraceCoreActivationBase::DTraceCoreActivationBase()
#if defined(__SMP__)
: iActivationReadLock(TSpinLock::EOrderBTrace-1)
#endif 
    {
    }


/**
 * Destructor
 */
DTraceCoreActivationBase::~DTraceCoreActivationBase()
    {
    }


/**
 * Subscribes to message ID and registers to activation interface list
 * 
 * @param aMessageID the message id to be subscribed
 */         
TInt DTraceCoreActivationBase::Init( TUint32 aMessageId )
    {
    // Subscribes to a message by message ID
    TInt err = Subscribe( aMessageId );
    if ( err == KErrNone )
        {
        // If subscribe succeeds, GetInstance will not return NULL
        err = DTraceCore::GetInstance()->RegisterActivation( *this );
        }
 
    TC_TRACE( ETraceLevelFlow, Kern::Printf( "<DTraceCoreActivationBase::Init() - %d", err ) );
    return err;
    }

    
/**
 * Subscribes to message ID and registers to activation interface list
 * 
 * @param aMessageID the message id to be subscribed
 * @param aMsgFormat Message format e.g. EMessageHeaderFormatOst
 */
TInt DTraceCoreActivationBase::SubscribeMessage(TUint32 aMessageId, TMessageHeaderFormat aMsgFormat)
    {
    return Subscribe(aMessageId, aMsgFormat) ;
    }


/**
 * Registers an activation notification interface
 * 
 * @param aNotification the notification interface
 */
TInt DTraceCoreActivationBase::RegisterActivationNotification( MTraceCoreActivationNotification& aNotification )
    {
    OstTrace1( TRACE_FLOW, DBTRACECOREACTIVATIONBASE_REGISTERACTIVATIONNOTIFICATION_ENTRY, "> DBTraceCoreActivationBase::RegisterActivationNotification 0x%x", ( TUint )&( aNotification ) );
    
    TInt retval = iActivationNotification.Append( &aNotification );
    
    OstTrace1( TRACE_FLOW, DBTRACECOREACTIVATIONBASE_REGISTERACTIVATIONNOTIFICATION_EXIT, "< DBTraceCoreActivationBase::RegisterActivationNotification Ret:%d", retval );
    return retval;
    }


/**
 * Sends notification to TraceCore internal activation notification interfaces.
 * 
 * @pre Method should be called with writer lock (iActivationWriteLock) held.
 * 
 * @param aFromSettings ETrue if activation was due to settings read, EFalse if from some other source
 * @param aComponentId Component ID of the activation 
 */
void DTraceCoreActivationBase::NotifyInternalActivation( TBool aFromSettings, TUint32 aComponentId )
    {
    OstTraceExt2( TRACE_FLOW, DBTRACECOREACTIVATIONBASE_NOTIFYINTERNALACTIVATION_ENTRY, "> DBTraceCoreActivationBase::NotifyInternalActivation. FromSettings:%d, ComponentID:%x", (TUint32)aFromSettings, (TUint32) aComponentId );

    for ( int i = 0; i < iActivationNotification.Count(); i++ )
        {
        iActivationNotification[ i ]->ActivationChanged( *this, aFromSettings, aComponentId );
        }
    }
    

/**
 * Send response
 * 
 * @param aMessage Response message
 */
void DTraceCoreActivationBase::SendResponse( TTraceMessage& aMessage )
    {
    iMessageSender->SendMessage( aMessage );
    }
    
