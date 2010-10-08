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
// Registers / unregisters notification receivers and send notifications 
// to them when needed.
// 

#include "TraceCore.h"
#include "TraceCoreNotifier.h"
#include "TraceCoreNotificationReceiver.h"
#include "TraceCoreActivation.h"
#include "TraceCoreDebug.h"
#include "TraceCoreConstants.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "TraceCoreNotifierTraces.h"
#endif

#include "TraceCoreTComArgMacro.h"

   
/** 
 * Constructor
 */
DTraceCoreNotifier::DTraceCoreNotifier()
    {
    }


/** 
 * Destructor
 */
DTraceCoreNotifier::~DTraceCoreNotifier()
    {
    iNotificationReceiverItems.Reset();
    }

/**
 * Callback for trace activation
 *
 * @param aComponentId The component ID
 * @param aGroupId The group ID
 */
void DTraceCoreNotifier::TraceActivated( TUint32 aComponentId, TUint16 aGroupId )
    {
    //TODO assert that this method can be run only on iActivationQ thread. (assert debug)
    OstTraceExt2( TRACE_FLOW, DTRACECORENOTIFIER_TRACEACTIVATED_ENTRY, "> DTraceCoreNotifier::TraceActivated CID:0x%x GID:0x%hx", aComponentId, (TUint32) aGroupId );
   
    // Go through notification receivers
    for ( TInt i = 0; i < iNotificationReceiverItems.Count(); i++ )
        {
        if ( ( iNotificationReceiverItems[ i ].iComponentID == aComponentId 
                && iNotificationReceiverItems[ i ].iGroupID == aGroupId )
          || ( iNotificationReceiverItems[ i ].iComponentID == aComponentId 
                && iNotificationReceiverItems[ i ].iGroupID == KAllGroups ) )
            {

            iNotificationReceiverItems[ i ].iReceiver->TraceActivated( aComponentId, aGroupId );
            }
            //no else
        }
	}
	
/**
 * Callback for trace deactivation
 *
 * @param aComponentId The componet ID
 * @param aGroupId The group ID
 */
void DTraceCoreNotifier::TraceDeactivated( TUint32 aComponentId, TUint16 aGroupId )
    {
    OstTraceExt2( TRACE_FLOW, DTRACECORENOTIFIER_TRACEDEACTIVATED_ENTRY, "> DTraceCoreNotifier::TraceDeactivated CID:0x%x GID:0x%hx", aComponentId, (TUint32) aGroupId );
    
    // Go through notification receivers
    for ( TInt i = 0; i < iNotificationReceiverItems.Count(); i++ )
        {
        if ( ( iNotificationReceiverItems[ i ].iComponentID == aComponentId 
                && iNotificationReceiverItems[ i ].iGroupID ==  aGroupId )
          || ( iNotificationReceiverItems[ i ].iComponentID == aComponentId 
                && iNotificationReceiverItems[ i ].iGroupID == KAllGroups )
         || ( iNotificationReceiverItems[ i ].iComponentID == aComponentId 
                && aGroupId ==  KAllGroups ) )
            {
            
            // If whole component was deactivated, get group ID's from the notification array
            if ( aGroupId ==  KAllGroups )
            	{
            	iNotificationReceiverItems[ i ].iReceiver->TraceDeactivated( aComponentId, iNotificationReceiverItems[ i ].iGroupID  );
            	}
            
            // One group was deactivated
            else
            	{
                iNotificationReceiverItems[ i ].iReceiver->TraceDeactivated( aComponentId, aGroupId  );
            	}
            }
        }
	}

/**
 * Callback when there is a trace error
 * 
 * @param aComponentId The component ID
 * @param aGroupId The group ID    
 * @param aError The reason for error
 */
void DTraceCoreNotifier::TraceError( TUint32 aComponentId, TUint32 aGroupId, TInt TCOM_ARG(aError)  )
    {
    OstTraceExt3( TRACE_FLOW, DTRACECORENOTIFIER_TRACEERROR_ENTRY, "> DTraceCoreNotifier::TraceError CID:0x%x GID:0x%x Err:%d", aComponentId, aGroupId, (TInt32) aError );
    
    // Go through notification receivers
    for ( TInt i = 0; i < iNotificationReceiverItems.Count(); i++ )
        {
        if ( ( iNotificationReceiverItems[ i ].iComponentID == aComponentId 
                && iNotificationReceiverItems[ i ].iGroupID == aGroupId )
          || ( iNotificationReceiverItems[ i ].iComponentID == aComponentId 
                && iNotificationReceiverItems[ i ].iGroupID == KAllGroups ) )
            {
            iNotificationReceiverItems[ i ].iReceiver->TraceError( aComponentId, aGroupId, KErrArgument );
            OstTrace0( TRACE_API, DTRACECORENOTIFIER_TRACEERROR_RETURNED_FROM_NOTIFICATION_RECEIVER, "DTraceCoreNotifier::TraceError - Returned from notification receiver");
            }
            //no else
        }
    
    OstTrace0( TRACE_FLOW, DTRACECORENOTIFIER_TRACEERROR_EXIT, "< DTraceCoreNotifier::TraceError");
    }

/**
 * Register notification receiver
 * 
 * @param aItem The properties of the notification receiver
 */
TInt DTraceCoreNotifier::RegisterNotificationReceiver( TNotificationReceiverItem& aNotificationReceiverItem )
    {
    OstTrace1( TRACE_FLOW, DTRACECORENOTIFIER_REGISTERNOTIFICATIONRECEIVER_ENTRY, "> DTraceCoreNotifier::RegisterNotificationReceiver 0x%x", ( TUint )&( aNotificationReceiverItem ) );
   
    TInt err = KErrNone;
    if ( aNotificationReceiverItem.iReceiver != NULL )
        {
        TBool itemExist( EFalse );     
        // Check if already notification receiver to the combination
        for ( TInt i = 0; i < iNotificationReceiverItems.Count(); i++ )
            {
            if ( iNotificationReceiverItems[ i ].iComponentID == aNotificationReceiverItem.iComponentID
                    && iNotificationReceiverItems[ i ].iGroupID == aNotificationReceiverItem.iGroupID 
                    && iNotificationReceiverItems[ i ].iReceiver == aNotificationReceiverItem.iReceiver )
                {
                itemExist = ETrue;
                break;	
                }
                //no else
            }

        // If not yet receiving, append to receiver items
        if ( !itemExist )
            {      
            err = iNotificationReceiverItems.Append( aNotificationReceiverItem );
            if ( err == KErrNone )
                {
                OstTraceExt2( TRACE_NORMAL, DTRACECORENOTIFIER_REGISTERNOTIFICATIONRECEIVER_REGISTERED, "DTraceCoreNotifier::RegisterNotificationReceiver - Registered CID:0x%x GID:0x%x", (TUint) aNotificationReceiverItem.iComponentID, (TUint)aNotificationReceiverItem.iGroupID );
                
                // Send notification immediately if group is active
                TGroupId groupId = aNotificationReceiverItem.iGroupID;
                NotifyImmediately(aNotificationReceiverItem.iComponentID, groupId);       
                }
            }
        // Already registered
        else
            {
            err = KErrAlreadyExists;
            }
        }
    // Item was null
    else
        {
        err = KErrArgument;
        }
        
    OstTrace1( TRACE_FLOW, DTRACECORENOTIFIER_REGISTERNOTIFICATIONRECEIVER_EXIT, "< DTraceCoreNotifier::RegisterNotificationReceiver. Err:%d", err );
    return err;
    }


/**
* Get registered group IDs
*
* @param aComponentId The componet ID
* @param aGroupIDs The array of group IDs
*/
void DTraceCoreNotifier::GetComponentRegisteredGroupIDs( TUint32 aComponentID, RArray< TUint >& aGroupIDs )
	{
    OstTrace1( TRACE_FLOW, DTRACECORENOTIFIER_GETCOMPONENTREGISTEREDGROUPIDS_ENTRY, "> DTraceCoreNotifier::GetComponentRegisteredGroupIDs CID:0x%x", aComponentID );
	
    TBool itemExist( EFalse );
    TInt err = KErrNone;
   
    // Go through notification receivers
	for ( TInt i = 0; i < iNotificationReceiverItems.Count(); i++ )
	    {
	    if ( iNotificationReceiverItems[ i ].iComponentID == aComponentID )
	        {
	        itemExist = EFalse;
	        
	        // Check if the item exists already
	        for ( TInt j = 0; j < aGroupIDs.Count(); j++ )
	            {
	            if ( aGroupIDs[ j ] == iNotificationReceiverItems[ i ].iGroupID )
	                {
	                itemExist = ETrue;
	                break;	                    	
	                }
	            }
            
	        if (!itemExist)
	            {
	            err = aGroupIDs.Append( iNotificationReceiverItems[ i ].iGroupID );
	            }
	        }	
	    }
   
	OstTrace1( TRACE_FLOW, DTRACECORENOTIFIER_GETCOMPONENTREGISTEREDGROUPIDS_EXIT, "< DTraceCoreNotifier::GetComponentRegisteredGroupIDs - %d", err );
    
	err++; //just to remove arm compiler warning about not using this variable when ost is compiled out
	}


 /**
 * Unregister notification receiver
 *
 * @param aItem The properties of the notification receiver
 */
void DTraceCoreNotifier::UnregisterNotificationReceiver( TNotificationReceiverItem& aNotificationReceiverItem )
    {
    OstTrace1( TRACE_FLOW, DTRACECORENOTIFIER_UNREGISTERNOTIFICATIONRECEIVER_ENTRY, "> DTraceCoreNotifier::UnregisterNotificationReceiver 0x%x", ( TUint )&( aNotificationReceiverItem ) );
    
    // Go through notification receivers
	for ( TInt i = 0; i < iNotificationReceiverItems.Count(); i++ )
        {
        if ( ( iNotificationReceiverItems[ i ].iComponentID == aNotificationReceiverItem.iComponentID
                && iNotificationReceiverItems[ i ].iGroupID == aNotificationReceiverItem.iGroupID 
                && iNotificationReceiverItems[ i ].iReceiver == aNotificationReceiverItem.iReceiver )
            ||
                ( iNotificationReceiverItems[ i ].iComponentID == aNotificationReceiverItem.iComponentID
                        && KAllGroups == aNotificationReceiverItem.iGroupID 
                        && iNotificationReceiverItems[ i ].iReceiver == aNotificationReceiverItem.iReceiver ) )
            {            
            // Unregister notification receiver
            iNotificationReceiverItems.Remove( i );
            i--;
            }
            //no else
        }
    }


 /**
 * Unregister all traces of given notification receiver
 *
 * @param aNotificationReceiver The notification receiver to be unregistered
 */
void DTraceCoreNotifier::UnregisterNotificationReceiver( MTraceCoreNotificationReceiver& aNotificationReceiver )
    {
    OstTrace1( TRACE_FLOW, DTRACECORENOTIFIER_UNREGISTERNOTIFICATIONRECEIVERALL_ENTRY, "> DTraceCoreNotifier::UnregisterNotificationReceiver - All IDs 0x%x", ( TUint )&( aNotificationReceiver ) );
    
    for ( TInt i = 0; i < iNotificationReceiverItems.Count(); i++ )
	    {
	    if ( iNotificationReceiverItems[ i ].iReceiver == &aNotificationReceiver )
	        {
            OstTraceExt2( TRACE_NORMAL, DTRACECORENOTIFIER_UNREGISTERNOTIFICATIONRECEIVER_UNREGISTERED_, "DTraceCoreNotifier::UnregisterNotificationReceiver - Unregistered CID:0x%x GID:0x%x", (TUint)iNotificationReceiverItems[ i ].iComponentID, (TUint)iNotificationReceiverItems[ i ].iGroupID );
            iNotificationReceiverItems.Remove( i );
	        i--;
	        }
	        //no else
	    }
	}

/**
* Notifies immediately if group was activated
*
* @param aComponentID The component id
* @param aGroupId The group id 
*/
void DTraceCoreNotifier::NotifyImmediately( TUint32 aComponentId, TGroupId aGroupId )
   {
   OstTraceExt2( TRACE_FLOW, NOTIFY_IMMEDIATELY_ENTRY, "> DTraceCoreNotifier::NotifyImmediately. GID: 0x%x, GID: 0x%hx", aComponentId, (TUint32) aGroupId );
   
   // Send notification about activated groups immediately
   DTraceCore* traceCore = DTraceCore::GetInstance();
   if ( traceCore != NULL )
       {
       MTraceCoreActivation *activation = traceCore->GetActivation( aComponentId );
       RArray< TUint > groups = activation->GetActivatedGroups( aComponentId );
       for (TInt i = 0; i < groups.Count(); i++)
           {
           if ( aGroupId == KAllGroups || groups[ i ] == aGroupId )
               {
               OstTrace1( TRACE_BORDER , SEND_NOTIFICATION_IMMEDIATELY, "Send activation notification immediately. Group ID:%x", groups[ i ] );
               this->TraceActivated( aComponentId, groups[ i ] );
               }
           }
       }
   }


// End of File
