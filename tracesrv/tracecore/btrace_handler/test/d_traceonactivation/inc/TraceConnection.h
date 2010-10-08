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
//

#ifndef _TRACECONNECTION_H_
#define _TRACECONNECTION_H_

#include <kernel/kernel.h>
#include <TraceCoreSubscriber.h>

/**

Handles TraceCore connection

*/
NONSHARABLE_CLASS(DTraceConnection) : public MTraceCoreNotificationReceiver, 
									  public DTraceCoreSubscriber
    {
    public:
    DTraceConnection( );
    DTraceConnection( DTcChannel* aTCChannel );
    ~DTraceConnection();
    
    // Virtual from MTraceCoreNotificationReceiver. Called from TraceCore.
    void TraceActivated( TUint32 aComponentId, TUint16 aGroupId  );
    void TraceDeactivated( TUint32 aComponentId, TUint16 aGroupId  );
    
    // Called from DTcChannel
    TInt RegisterNotificationReceiver( TUint32 aComponentId, TInt32 aGroupId );
    void UnregisterNotificationReceiver( TUint32 aComponentId, TInt32 aGroupId  );
    
    // From DTraceCoreSubscriber
    void MessageReceived( TTraceMessage &aMsg );
    
    TInt DoSendMessage( TTraceMessage &aMsg );
    
    private:
    DTcChannel* iTcChannel;
    
    };

#endif /*_TRACECONNECTION_H_*/

