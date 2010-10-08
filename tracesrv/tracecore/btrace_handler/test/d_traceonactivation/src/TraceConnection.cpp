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

#include "TraceOnActivation.h"
#include "TraceCoreTraceActivationIf.h"
#include "TraceConnection.h"

#include <TraceCoreMessageSender.h>


//---------------------------------------------------------------------------
/**
    Constructor

*/
DTraceConnection::DTraceConnection( )
: iTcChannel( NULL )
    {

    }

//---------------------------------------------------------------------------
/**
    Constructor

    @param  aTcChannel  A pointer to the DTcChannel

*/
DTraceConnection::DTraceConnection( DTcChannel* aTcChannel )
: iTcChannel( aTcChannel )
    {
    }
    
//---------------------------------------------------------------------------
/**
    Destructor

*/
DTraceConnection::~DTraceConnection()
    {
    }
    
//---------------------------------------------------------------------------
/**
    RegisterNotificationReceiver

    @param  aComponentId  ComponentId
    @param  aGroupId GroupId

*/
TInt DTraceConnection::RegisterNotificationReceiver( TUint32 aComponentId, TInt32 aGroupId )
    {
    return MTraceCoreNotificationReceiver::RegisterNotificationReceiver( aComponentId, aGroupId );
    }

    
//---------------------------------------------------------------------------
/**
    UnregisterNotificationReceiver

    @param  aComponentId  ComponentId
    @param  aGroupId GroupId

*/
void DTraceConnection::UnregisterNotificationReceiver( TUint32 aComponentId, TInt32 aGroupId  )
    {
    MTraceCoreNotificationReceiver::UnregisterNotificationReceiver( aComponentId, aGroupId );
    }    

//---------------------------------------------------------------------------
/**
    TraceActivated

    @param  aComponentId  ComponentId
    @param  aGroupId GroupId

*/
void DTraceConnection::TraceActivated( TUint32 aComponentId, TUint16 aGroupId  )
    {
    if(iTcChannel)
        {
        iTcChannel->TraceActivated( aComponentId, aGroupId );
        }
    }

//---------------------------------------------------------------------------
/**
    TraceDeactivated

    @param  aComponentId  ComponentId
    @param  aGroupId GroupId

*/    
void DTraceConnection::TraceDeactivated( TUint32 aComponentId, TUint16 aGroupId  )
    {
    if(iTcChannel)
        {
        iTcChannel->TraceDeactivated( aComponentId, aGroupId );
        }
    }

//---------------------------------------------------------------------------

void DTraceConnection::MessageReceived( TTraceMessage& /*aMsg*/ )
	{
	}


TInt DTraceConnection::DoSendMessage( TTraceMessage &aMsg )
	{
	TInt err = KErrNotFound;
	if ( iMessageSender )
		{
		err = iMessageSender->SendMessage( aMsg );
		}	
	return err;
	}

