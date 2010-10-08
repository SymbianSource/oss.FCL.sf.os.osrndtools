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
// Trace Core
// 
#include <kern_priv.h>
#include "TraceCoreRouter.h"
#include "TraceCoreSubscriber.h"
#include "TraceCoreDebug.h"
#include "OstTraceDefinitions.h"

#ifdef OST_TRACE_COMPILER_IN_USE
#include "TraceCoreRouterTraces.h"
#endif

// Constants

/** 
 * Constructor
 */
DTraceCoreRouter::DTraceCoreRouter()
: iMessageSender( NULL )
    {
    }


/** 
 * Destructor
 */
DTraceCoreRouter::~DTraceCoreRouter()
    {
    Kern::MutexWait(*iLock);
    iRoutingItems.Reset();
    Kern::MutexSignal(*iLock);
    iLock->Close(NULL);
    }


/**
 * Initializes this router
 *
 * @param aMessageSender The message sender interface
 */
TInt DTraceCoreRouter::Init( MTraceCoreMessageSender& aMessageSender )
    {
    iMessageSender = &aMessageSender;
    _LIT(KTraceCoreRouterLock, "DTraceCoreRouter_Lock");
    TInt err = Kern::MutexCreate(iLock, KTraceCoreRouterLock, KMutexOrdGeneral0);
    return err;
    }


/**
 * Callback for incoming messages
 *
 * @param aMsg The message
 * @return KErrNotFound if the message id was not found (not subscribed), KErrNone if found
 */
TInt DTraceCoreRouter::MessageReceived( TTraceMessage &aMsg )
    {
    OstTraceExt1( TRACE_FLOW, DTRACECOREROUTER_MESSAGERECEIVED_ENTRY, "> DTraceCoreRouter::MessageReceived. MsgId:0x%hhx", aMsg.iMessageId );
   
    TInt ret( KErrNotFound );
    TUint32 messageId = aMsg.iMessageId;
    TUint32 msgFormat = aMsg.iMsgFormat;
    
    Kern::MutexWait(*iLock);
    for ( TInt i = 0; i < iRoutingItems.Count(); i++ )
        {
        if ( iRoutingItems[ i ].iMessageID == messageId && iRoutingItems[ i ].iMsgFormat == msgFormat )
            {
            iRoutingItems[ i ].iSubscriber->MessageReceived( aMsg );
            ret = KErrNone;
            }
        }
    Kern::MutexSignal(*iLock);
        
	OstTrace1( TRACE_FLOW, DTRACECOREROUTER_MESSAGERECEIVED_EXIT, "< DTraceCoreRouter::MessageReceived. Ret:%d", ret );
	return ret;
	}


/**
 * Subscribes to a message
 * 
 * @param aRoutingItem The subscription properties.
 */
TInt DTraceCoreRouter::Subscribe( TRoutingItem& aRoutingItem )
    {
    OstTrace1( TRACE_FLOW, DTRACECOREROUTER_SUBSCRIBE_ENTRY, "> DTraceCoreRouter::Subscribe 0x%x", ( TUint )&( aRoutingItem ) );
    
    TInt err = KErrNone;
    if ( iMessageSender != NULL ) 
        {
        if ( aRoutingItem.iSubscriber != NULL )
            {
            Kern::MutexWait(*iLock);
            // Add to routingItems array
            err = iRoutingItems.Append( aRoutingItem );
            Kern::MutexSignal(*iLock);
            if ( err == KErrNone )                
                {
                // Set message sender to routing item
                aRoutingItem.iSubscriber->SetMessageSender( *iMessageSender );
                OstTraceExt2( TRACE_NORMAL, DTRACECOREROUTER_SUBSCRIBE_SUBSCRIBED_TO_MESSAGE, "DTraceCoreRouter::Subscribe - Subscribed to message. Subscriber:0x%x MsgId:0x%x", (TUint)aRoutingItem.iSubscriber, (TUint)aRoutingItem.iMessageID );
                }
            }
        // Subscriber was NULL
        else
            {         
            err = KErrArgument;
            }
        }
    // Message sender not set
    else
        {
        err = KErrGeneral;
        }                                    
    OstTrace1( TRACE_FLOW, DTRACECOREROUTER_SUBSCRIBE_EXIT, "< DTraceCoreRouter::Subscribe. Err:%d", err );
    return err;
    }


/**
 * Unsubscribes from a message
 * 
 * @param aRoutingItem The subscription properties
 */
void DTraceCoreRouter::Unsubscribe( TRoutingItem& aRoutingItem )
    {
    // Make sure not to delete item while going through items somewhere else
    Kern::MutexWait(*iLock);
	for ( TInt i = 0; i < iRoutingItems.Count(); i++ )
	    {
	    if ( iRoutingItems[ i ].iMessageID == aRoutingItem.iMessageID
	            && iRoutingItems[ i ].iSubscriber == aRoutingItem.iSubscriber )
	        {
	        OstTraceExt2( TRACE_NORMAL, DTRACECOREROUTER_UNSUBSCRIBE_UNSUBSCRIBED, "DTraceCoreRouter::Unsubscribe - Unsubscribed 0x%x MsgID:0x%x", (TUint)aRoutingItem.iSubscriber, (TUint)aRoutingItem.iMessageID );
	        iRoutingItems.Remove( i );
	        i--;
	        }
	    }
	Kern::MutexSignal(*iLock); 
    }


/**
 * Unsubscribes from all messages of given subscriber
 * 
 * @param aSubscriber The subscriber to be unregistered
 */
void DTraceCoreRouter::Unsubscribe( DTraceCoreSubscriber& aSubscriber )
    {
    OstTrace1( TRACE_FLOW, DTRACECOREROUTER_UNSUBSCRIBEALL_ENTRY, "> DTraceCoreRouter::UnsubscribeAll 0x%x", ( TUint )&( aSubscriber ) );
    
    Kern::MutexWait(*iLock);
	for ( TInt i = 0; i < iRoutingItems.Count(); i++ )
	    {
	    if ( iRoutingItems[ i ].iSubscriber == &aSubscriber )
	        {
	        OstTraceExt2( TRACE_NORMAL, DTRACECOREROUTER_UNSUBSCRIBEALL_UNSUBSCRIBED, "DTraceCoreRouter::UnsubscribeAll - Unsubscribed 0x%x MsgID:%d", (TUint)&aSubscriber, (TInt)iRoutingItems[ i ].iMessageID );
	        iRoutingItems.Remove( i );
	        i--;
	        }
	    }
	Kern::MutexSignal(*iLock);
    }

// End of File
