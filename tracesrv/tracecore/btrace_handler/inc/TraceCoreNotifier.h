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

#ifndef __TRACECORENOTIFIER_H__
#define __TRACECORENOTIFIER_H__


// Include files
#include <kernel/kernel.h>
#include <opensystemtrace_types.h>

// Forward declarations
class MTraceCoreNotificationReceiver;

/**
 * Notification receiver item
 */
NONSHARABLE_CLASS( TNotificationReceiverItem )
    {
public:

    /**
     * Component ID
     */
    TUint32 iComponentID;
    
    /**
     * Group ID
     */
    TUint32 iGroupID;

    /**
     * The notification receiver associated with the component id and group id
     */
    MTraceCoreNotificationReceiver* iReceiver;
    };


/**
 * Notification receiver receives subscriptions and delegates messages to them
 */
NONSHARABLE_CLASS( DTraceCoreNotifier ) : public DBase
    {
public:
		
    /**
    * Constructor
    */
    DTraceCoreNotifier();
    
    /**
    * Destructor
    */
    ~DTraceCoreNotifier();
    
    /**
    * Register notification receiver
    *
    * @param aItem The properties of the notification receiver
    */
	TInt RegisterNotificationReceiver( TNotificationReceiverItem& aItem );
	
	/**
	* Unregister notification receiver
    *
	* @param aItem The properties of the notification receiver
	*/
	void UnregisterNotificationReceiver( TNotificationReceiverItem& aItem );
	
	/**
	* Unregister all traces of given notification receiver
	*
	* @param aNotificationReceiver The notification receiver to be unregistered
	*/
	void UnregisterNotificationReceiver( MTraceCoreNotificationReceiver& aNotificationReceiver );

	/**
	* Get defined component's registered group IDs
	*
	* @param aComponentID The component id
	* @param aGroupIDs Array for registered group IDs
	*/
	void GetComponentRegisteredGroupIDs( TUint32 aComponentID, RArray< TUint >& aGroupIDs );
		
	/**
	* Callback when trace is activated
	*
	* @param aComponentID The component id
	* @param aGroupId The group id 
	*/
    void TraceActivated( TUint32 aComponentId, TUint16 aGroupId );
    
    /**
	* Callback when trace is deactivated
	*
	* @param aComponentID The component id
	* @param aGroupId The group id 
	*/
    void TraceDeactivated( TUint32 aComponentId, TUint16 aGroupId );

    /**
     * Callback when there is a trace error
     * 
     * @param aComponentId The component ID
     * @param aGroupId The group ID    
     * @param aError The reason for error
     */
    void TraceError( TUint32 aComponentId, TUint32 aGroupId, TInt aError  );

private:
	
    /**
    * Active subscriptions
    */
	RArray< TNotificationReceiverItem > iNotificationReceiverItems;
	
    /**
    * Notifies immediately if group was activated
    *
    * @param aComponentID The component id
    * @param aGroupId The group id 
    */
    void NotifyImmediately( TUint32 aComponentId, TGroupId aGroupId );
    
    };

#endif

// End of File
