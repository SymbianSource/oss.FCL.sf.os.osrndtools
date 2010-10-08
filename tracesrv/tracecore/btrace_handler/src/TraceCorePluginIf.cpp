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
// TraceCore Plugin Interface
// 

//- Include Files  ----------------------------------------------------------

#include "TraceCorePluginIf.h" 
#include "TraceCoreDebug.h"
#include "TraceCoreMediaIfCallback.h"
#include "TraceCoreMediaPlugin.h"
#include "TraceCoreSubscriber.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "TraceCorePluginIfTraces.h"
#endif


/**
 * Constructor
 */
DTraceCorePluginIf::DTraceCorePluginIf()
: DTraceCoreMediaIf( KMediaIfSendSupported | KMediaIfPluginSupported )
, iMediaPlugin( NULL )
    {
    }


/**
 * Destructor
 */
DTraceCorePluginIf::~DTraceCorePluginIf()
    {
    iMediaPlugin = NULL;
    }


/**
 * Initializes the plugin IF
 *
 * @param aCallBack Callback to TraceCore
 */
TInt DTraceCorePluginIf::Init( MTraceCoreMediaIfCallback& aCallback )
    {
    iCallback = &aCallback;
    return KErrNone;
    }

/**
 * Sends messages to plug-in
 * 
 * @param aMsg Message to be sent.
 * @return KErrNone if send successful
 */
TInt DTraceCorePluginIf::Send( TTraceMessage& aMsg )
    {
    OstTraceExt1( TRACE_FLOW, DTRACECOREPLUGINIF_SEND_ENTRY, "> DTraceCorePluginIf::Send. MsgId:0x%hhx", aMsg.iMessageId );
    TInt retval = KErrNotSupported;
    if ( iMediaPlugin != NULL )
        {
        retval = iMediaPlugin->SendMessage( aMsg );
        }
    OstTrace1( TRACE_FLOW, DTRACECOREPLUGINIF_SEND_EXIT, "< DTraceCorePluginIf::Send %d", retval );
    return retval;
    }
    

/**
 * Message was received by plug-in
 * 
 * @param aMsg The message that was received
 */
void DTraceCorePluginIf::MessageReceived( TTraceMessage &aMsg )
    {
    OstTraceExt1( TRACE_FLOW, DTRACECOREPLUGINIF_MESSAGERECEIVED_ENTRY, "> DTraceCorePluginIf::MessageReceived. MsgId:0x%hhx", aMsg.iMessageId );
    if ( iCallback != NULL )
        {
        iCallback->SetSenderMedia( this ); // Added for SPLIT
        iCallback->MessageReceived( aMsg );
        }
    }


/**
 * Registers a media Plug-In
 */
TInt DTraceCorePluginIf::RegisterPlugin( DTraceCoreMediaPlugin& aPlugin )
    {
    OstTrace1( TRACE_FLOW, DTRACECOREPLUGINIF_REGISTERPLUGIN_ENTRY, "> DTraceCorePluginIf::RegisterPlugin 0x%x", ( TUint )&( aPlugin ) );
    TInt retval = KErrNone;
    
    // Register given media Plug-In
    if ( iMediaPlugin == NULL )
        {
        iMediaPlugin = &aPlugin;
        retval = KErrNone;
        }
    
    // Plug-In was already registered
    else
        {
        retval = KErrNotSupported;
        }
    OstTrace1( TRACE_FLOW, DTRACECOREPLUGINIF_REGISTERPLUGIN_EXIT, "< DTraceCorePluginIf::RegisterPlugin %d", retval );
    
    return retval;
    }


/**
 * Unregisters a media Plug-In
 */
void DTraceCorePluginIf::UnregisterPlugin( DTraceCoreMediaPlugin& aPlugin )
    {
    OstTrace1( TRACE_FLOW, DTRACECOREPLUGINIF_UNREGISTERPLUGIN_ENTRY, "> DTraceCorePluginIf::UnregisterPlugin 0x%x", ( TUint )&( aPlugin ) );
    
    // Unregister given media Plug-In
    if ( iMediaPlugin == &aPlugin )
        {
        OstTrace1( TRACE_NORMAL, DTRACECOREPLUGINIF_UNREGISTERPLUGIN_UNREGISTERED, "DTraceCorePluginIf::UnregisterPlugin - Plugin unregistered 0x%x", ( TUint )&( aPlugin ) );
        iMediaPlugin = NULL;
        }
    }

// End of File
