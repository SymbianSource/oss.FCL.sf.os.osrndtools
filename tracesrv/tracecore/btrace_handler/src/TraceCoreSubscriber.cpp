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


#include "TraceCore.h"
#include "TraceCoreSubscriber.h"
#include "TraceCoreRouter.h"
#include "TraceCoreDebug.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "TraceCoreSubscriberTraces.h"
#endif



/**
 * Constructor
 */
EXPORT_C DTraceCoreSubscriber::DTraceCoreSubscriber()
: iMessageSender( NULL )
    {
    }


/**
 * Destructor
 */
EXPORT_C DTraceCoreSubscriber::~DTraceCoreSubscriber()
    {
    iMessageSender = NULL;
    Unsubscribe();
    }

/**
 * Subscribes to a message by message ID / protocol ID
 * depending of the header format. First byte of aMessageID is always used
 * for header format.
 * 
 * @param aMessageId The message ID to be subscribed
 * @param aMsgFormat Message format e.g. EMessageHeaderFormatOst
 * 
 */
EXPORT_C TInt DTraceCoreSubscriber::Subscribe( TUint32 aMessageID, TMessageHeaderFormat aHeaderFormat )
    {
    TInt ret(KErrNone);
    // Use MSByte only for header format
    TUint32 combined = aHeaderFormat << 24 | (aMessageID & 0x00FFFFFF); // CodForChk_Dis_Magic
    DTraceCoreSubscriber::Subscribe( combined );
    return ret;
    }

/**
 * Subscribes to a message by message ID
 * 
 * @param aMessageId The message ID to be subscribed
 */
EXPORT_C TInt DTraceCoreSubscriber::Subscribe( TUint32 aMessageID )
    {
    OstTrace1( TRACE_BORDER, DTRACECORESUBSCRIBER_SUBSCRIBE_ENTRY, "> DTraceCoreSubscriber::Subscribe 0x%x", aMessageID );
    TInt ret = KErrGeneral;
    DTraceCore* traceCore = DTraceCore::GetInstance();
    if ( traceCore != NULL )
        {
        TRoutingItem routingItem;
        
        // If MSByte is empty it is Proprietary message -> Add Proprietary header format id
        // Otherwise take it from aMessageId (MSByte)
        if( !(aMessageID & 0xFF000000) ) // CodForChk_Dis_Magic explained in comment
            {
            routingItem.iMsgFormat = EMessageHeaderFormatProprietary;
            }
        else
            {
            // Extract Header Format from aMessageID
            routingItem.iMsgFormat = (aMessageID >> 24) & 0x000000FF; // CodForChk_Dis_Magic
            }
        
        // Remove Header format
        aMessageID = (aMessageID & 0x00FFFFFF); // CodForChk_Dis_Magic
        
        routingItem.iMessageID = aMessageID;
        routingItem.iSubscriber = this;
           
        ret = traceCore->GetRouter().Subscribe( routingItem );
        }
    OstTrace1( TRACE_API, DTRACECORESUBSCRIBER_SUBSCRIBE_EXIT, "DTraceCoreSubscriber::Subscribe. Return code:%d", ret );
    return ret;
    }
    

/**
 * Unsubscribes a message ID
 * 
 * @param aMessageId The message ID to be unsubscribed
 */
EXPORT_C void DTraceCoreSubscriber::Unsubscribe( TUint32 aMessageID )
    {
    OstTrace1( TRACE_BORDER, DTRACECORESUBSCRIBER_UNSUBSCRIBE_ENTRY, "> DTraceCoreSubscriber::Unsubscribe. MsgId:%d", aMessageID );
    DTraceCore* traceCore = DTraceCore::GetInstance();
    if ( traceCore != NULL )
        {
        TRoutingItem routingItem;
        routingItem.iMessageID = aMessageID;
        routingItem.iSubscriber = this;
        traceCore->GetRouter().Unsubscribe( routingItem );
        }
    }


/**
 * Called by router to set the message sender interface
 */
void DTraceCoreSubscriber::SetMessageSender( MTraceCoreMessageSender& aMessageSender )
    {
    iMessageSender = &aMessageSender;
    }


/**
 * Unsubscribes all message ID's of this subscriber
 */
void DTraceCoreSubscriber::Unsubscribe()
    {
    DTraceCore* traceCore = DTraceCore::GetInstance();
    
    // Unsubsribe if TraceCore exists
    if ( traceCore != NULL )
        {
        traceCore->GetRouter().Unsubscribe( *this );
        }
    }

// End of File
