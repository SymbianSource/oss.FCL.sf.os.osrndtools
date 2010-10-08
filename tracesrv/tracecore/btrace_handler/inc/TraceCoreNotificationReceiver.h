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

#ifndef __TRACECORENOTIFICATIONRECEIVER_H__
#define __TRACECORENOTIFICATIONRECEIVER_H__

// Include files
#include <kernel/kernel.h>
#include <opensystemtrace_types.h>



/**
 * Base class for notification receivers of TraceCore
 */
class MTraceCoreNotificationReceiver
    {
public:

    /**
     * Constructor
     */
    IMPORT_C MTraceCoreNotificationReceiver();

    /**
     * Destructor
     */
    IMPORT_C virtual ~MTraceCoreNotificationReceiver();

    /**
     * Register notification receiver by component id
     * 
     * @param aComponentId The component ID
     */
    IMPORT_C TInt RegisterNotificationReceiver( TUint32 aComponentId );

    /**
     * Register notification receiver
     * 
     * @param aComponentId The component ID
     * @param aGroupId The group ID    
     */
    IMPORT_C TInt RegisterNotificationReceiver( TUint32 aComponentId, TInt32 aGroupId  );

    /**
     * Unregister notification receiver
     * 
     * @param aComponentId
     */
    IMPORT_C void UnregisterNotificationReceiver( TUint32 aComponentId );

    /**
     * Unregister notification receiver
     * 
     * @param aComponentId The component ID
     * @param aGroupId The group ID    
     */
    IMPORT_C void UnregisterNotificationReceiver( TUint32 aComponentId, TInt32 aGroupId  );
    
    /**
     * Callback function for Trace Activation
     * 
     * @param aComponentId
     * @param aGroupId         
     */
    virtual void TraceActivated( TUint32 aComponentId, TUint16 aGroupId  ) = 0;
    
    /**
     * Callback function for Trace Deactivation
     * 
     * @param aComponentId The component ID
     * @param aGroupId The group ID    
     */
    virtual void TraceDeactivated( TUint32 aComponentId, TUint16 aGroupId  ) = 0;
    
    /**
     * Callback function for trace error
     * 
     * @param aComponentId The component ID
     * @param aGroupId The group ID    
     * @param aError The reason for error
     */
    IMPORT_C virtual void TraceError( TUint32 aComponentId, TUint32 aGroupId, TInt aError  );

private:

    /**
     * Unregister all components of this receiver. Called from destructor.
     */
    void UnregisterNotificationReceiver();

    };

/**
 * @deprecated Please use MTraceCoreNotificationReceiver instead 
 */
class DTraceCoreNotificationReceiver : public DBase, public MTraceCoreNotificationReceiver
    {
    };

#endif

// End of File
