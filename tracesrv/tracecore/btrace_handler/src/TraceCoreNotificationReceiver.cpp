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
// Base class for notification receivers of TraceCore
//

#include "TraceCoreNotificationReceiver.h"
#include "TraceCore.h"
#include "TraceCoreNotifier.h"
#include "TraceCoreDebug.h"
#include "TraceCoreConstants.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "TraceCoreNotificationReceiverTraces.h"
#endif

#include "TraceCoreTComArgMacro.h"

/**
 * Constructor
 */
EXPORT_C MTraceCoreNotificationReceiver::MTraceCoreNotificationReceiver()
    {
    }


/**
 * Destructor
 */
EXPORT_C MTraceCoreNotificationReceiver::~MTraceCoreNotificationReceiver()
    {
    UnregisterNotificationReceiver();
    }


/**
 * Register notification receiver by component id
 * 
 * @param aComponentId The component ID
 */
EXPORT_C TInt MTraceCoreNotificationReceiver::RegisterNotificationReceiver( TUint32 aComponentId )
    {
    return RegisterNotificationReceiver( aComponentId, KAllGroups );
    }
    
/**
 * Register notification receiver by component id and group id
 * 
 * @param aComponentId The component ID
 * @param aGroupId The group ID 
 */
EXPORT_C TInt MTraceCoreNotificationReceiver::RegisterNotificationReceiver( TUint32 aComponentId, TInt32 aGroupId )
    {
    OstTraceExt2( TRACE_BORDER, MTRACECORENOTIFICATIONRECEIVER_REGISTERNOTIFICATIONRECEIVER_ENTRY, "> MTraceCoreNotificationReceiver::RegisterNotificationReceiver. CID:0x%x GID:0x%x", aComponentId, aGroupId );
    TInt ret;
    
    // Get TraceCore
    DTraceCore* traceCore = DTraceCore::GetInstance();
    if ( traceCore != NULL )
        {
        TNotificationReceiverItem notificationReceiverItem;
        notificationReceiverItem.iComponentID = aComponentId;
        notificationReceiverItem.iGroupID = aGroupId;
        notificationReceiverItem.iReceiver = this;
        
        // Register notification receiver
        ret = traceCore->GetNotifier().RegisterNotificationReceiver( notificationReceiverItem );
        }
    else
        {
        ret = KErrGeneral;
        }
    OstTrace1( TRACE_BORDER, MTRACECORENOTIFICATIONRECEIVER_REGISTERNOTIFICATIONRECEIVER_EXIT, "< MTraceCoreNotificationReceiver::RegisterNotificationReceiver. Ret:%d", ret );
    return ret;
    }
    

/**
 * Unregister notification receiver by component id
 * 
 * @param aComponentId The component ID
 */
EXPORT_C void MTraceCoreNotificationReceiver::UnregisterNotificationReceiver( TUint32 aComponentId )
    {
    
    UnregisterNotificationReceiver( aComponentId, KAllGroups );
    }
    
/**
 * Unregister notification receiver by component id and group id
 * 
 * @param aComponentId The component ID
 * @param aGroupId The group ID  
 */
EXPORT_C void MTraceCoreNotificationReceiver::UnregisterNotificationReceiver( TUint32 aComponentId, TInt32 aGroupId )
    {
    OstTraceExt2( TRACE_BORDER, MTRACECORENOTIFICATIONRECEIVER_UNREGISTERNOTIFICATIONRECEIVER_ENTRY, "> MTraceCoreNotificationReceiver::UnregisterNotificationReceiver. CID:0x%x GID:0x%x", aComponentId, aGroupId );
    DTraceCore* traceCore = DTraceCore::GetInstance();
    if ( traceCore != NULL )
        {
        TNotificationReceiverItem notificationReceiverItem;
        notificationReceiverItem.iComponentID = aComponentId;
        notificationReceiverItem.iGroupID = aGroupId;
        notificationReceiverItem.iReceiver = this;
        traceCore->GetNotifier().UnregisterNotificationReceiver( notificationReceiverItem );
        }
    }

/**
 * Unsubscribes all message ID's of this notification receiver
 */
void MTraceCoreNotificationReceiver::UnregisterNotificationReceiver()
    {
    // Get TraceCore
    DTraceCore* traceCore = DTraceCore::GetInstance();
    if ( traceCore != NULL )
        {
        
        // Unregister notification receiver
        traceCore->GetNotifier().UnregisterNotificationReceiver( *this );
        }
    }

/**
 * Callback function for trace error
 * 
 * @param aComponentId The component ID
 * @param aGroupId The group ID    
 * @param aError The reason for error
 */
EXPORT_C void MTraceCoreNotificationReceiver::TraceError( TUint32 TCOM_ARG(aComponentId), 
                                                          TUint32 TCOM_ARG(aGroupId), 
                                                          TInt TCOM_ARG(aError)  )
    {
    OstTraceExt3( TRACE_BORDER, MTRACECORENOTIFICATIONRECEIVER_TRACEERROR_ENTRY, "> MTraceCoreNotificationReceiver::TraceError. CID:0x%x GID:0x%x Err:%d", aComponentId, aGroupId, (TInt32) aError );
    }

// End of File
