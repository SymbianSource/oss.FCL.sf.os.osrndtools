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

#include "TraceCoreSendReceive.h"
#include "TraceCorePluginIf.h"
#include "TraceCoreRouter.h"
#include "TraceCoreDebug.h"
#include "TraceCoreOstLddIf.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "TraceCoreSendReceiveTraces.h"
#endif


/**
 * Constructor
 */
DTraceCoreSendReceive::DTraceCoreSendReceive()
: iPluginInterface( NULL )
, iSendInterface( NULL )
, iMediaWriterInterface( NULL )
, iReceiver( NULL )
    {
    }


/**
 * Destructor
 */                                       
DTraceCoreSendReceive::~DTraceCoreSendReceive()
    {
    DeleteInterfaces();
    iReceiver = NULL;
    }


/**
 * Initializer creates the ISA IF and router
 */
TInt DTraceCoreSendReceive::Init( MTraceCoreMessageReceiver& aReceiver )
    {
    // Create interfaces
    TInt ret = CreateInterfaces();
    if ( ret == KErrNone )
        {        
        // Initialize all Media interfaces
        for ( int i = 0; i < iMediaInterfaces.Count() && ret == KErrNone; i++ )
            {
            DTraceCoreMediaIf* mediaIf = iMediaInterfaces[ i ];
            ret = InitMediaInterface( mediaIf );
            }
        if ( ret == KErrNone )
            {
            iReceiver = &aReceiver;
            }
        else
            {
            DeleteInterfaces();
            }
        }
    TC_TRACE( ETraceLevelFlow, Kern::Printf( "<DTraceCoreSendReceive::Init() - %d", ret ) )
    return ret;
    }


/**
 * Initializes a media interface
 */
TInt DTraceCoreSendReceive::InitMediaInterface( DTraceCoreMediaIf* aMediaIf )
    {
    TInt ret = aMediaIf->Init( *this );
    if ( ret == KErrNone )
        {
        TUint32 flags = aMediaIf->GetFlags();
        // First interface that supports sending is stored as the send interface.
        // TODO: Add a TraceCore message which can select the interface used for sending
        if ( iSendInterface == NULL && ( flags & KMediaIfSendSupported ) )
            {
            iSendInterface = aMediaIf;
            }
        // First interface that can write traces is stored to iMediaWriterInterface
        // TODO: Add a TraceCore message which can select the interface used by the writer
        if ( iMediaWriterInterface == NULL && ( flags & KMediaIfSendTraceSupported ) )
            {
            iMediaWriterInterface = aMediaIf;
            }
        // If the plug-in interface is present, it is stored to iPluginInterface
        if ( iPluginInterface == NULL && ( flags & KMediaIfPluginSupported ) )
            {
            iPluginInterface = static_cast< DTraceCorePluginIf* >( aMediaIf );
            }
        }
    TC_TRACE( ETraceLevelFlow, Kern::Printf( "<DTraceCoreSendReceive::InitMediaInterface() - %d", ret ) );
    return ret;
    }


/**
 * Creates the media interfaces. This is platform-specific code
 */
TInt DTraceCoreSendReceive::CreateInterfaces()
    {
    TInt ret( KErrNoMemory );
#ifdef __WINS__
    DTraceCoreMediaIf* pluginIf = new DTraceCorePluginIf();
    if ( pluginIf != NULL )
        {
        ret = iMediaInterfaces.Append( pluginIf );
        }
    // this is a bit of a hack here for now
    // we create a DTraceCoreOstLddIf as the writer interface
    if( ret == KErrNone)
        {
        DTraceCoreMediaIf* ostLddIf = new DTraceCoreOstLddIf();
        if ( ostLddIf != NULL)
            {
            ret = iMediaInterfaces.Append( ostLddIf );
            }
        }
#else
    //  - Plug-in API for media plug-in's. Incoming / outgoing messages.
    //  - ISA for incoming messages and media writer output
    DTraceCoreMediaIf* pluginIf = new DTraceCorePluginIf();
    DTraceCoreMediaIf* usbIf = new DTraceCoreOstLddIf();
   
    if ( pluginIf != NULL && usbIf!= NULL )
        {
        ret = iMediaInterfaces.Append( pluginIf );
        ret = ( ret == KErrNone ) ? iMediaInterfaces.Append( usbIf ) : ret;
        }
#endif // __WINS__

    TC_TRACE( ETraceLevelFlow, Kern::Printf( "<DTraceCoreSendReceive::CreateInterfaces()" ) );
    return ret;
    }


/**
 * Deletes the media interfaces
 */
void DTraceCoreSendReceive::DeleteInterfaces()
    {
    // Delete all interfaces
    iMediaInterfaces.ResetAndDestroy();
    iSendInterface = NULL;
    iMediaWriterInterface = NULL;
    iPluginInterface = NULL;
    }


/**
 * Delegates the incoming message to the router
 *
 * @param aMsg The message that was received
 * @return KErrNotFound if the message id was not found (not subscribed), KErrNone if found
 */
TInt DTraceCoreSendReceive::MessageReceived( TTraceMessage &aMsg )
    {
    OstTrace1( TRACE_FLOW, DTRACECORESENDRECEIVE_MESSAGERECEIVED_ENTRY, "> DTraceCoreSendReceive::MessageReceived - %d", &aMsg);
    TInt ret( KErrNotFound );
    if ( iReceiver != NULL )
        {
        ret = iReceiver->MessageReceived( aMsg );
        }
    OstTrace1( TRACE_FLOW, DTRACECORESENDRECEIVE_MESSAGERECEIVED_EXIT, "< DTraceCoreSendReceive::MessageReceived - %d",ret);
    return ret;
    }

/**
 * Set Sender Media
 *
 * @param Sender media
 */
TInt DTraceCoreSendReceive::SetSenderMedia(DTraceCoreMediaIf* aSenderMedia)
    {
    if (aSenderMedia != NULL)
        {
        TUint32 flags = aSenderMedia->GetFlags();
        if (flags & KMediaIfSendSupported)
            {
            iSendInterface = aSenderMedia;
            }
        if (flags & KMediaIfSendTraceSupported)
            {
            iMediaWriterInterface = aSenderMedia;
            }
        }
    return KErrNone;
    }


/**
 * Sends a message to media interface
 *
 * @param aMessage The message to be sent
 */
TInt DTraceCoreSendReceive::SendMessage( TTraceMessage &aMessage )
    {
    TInt retval = KErrGeneral;
    if ( iSendInterface != NULL )
        {      
        // Send the message
        retval = iSendInterface->Send( aMessage );
        }
    OstTrace1( TRACE_FLOW, DTRACECORESENDRECEIVE_SENDMESSAGE_EXIT, "DTraceCoreSendReceive::SendMessage %d", retval );
    return retval;
    }

// End of File
