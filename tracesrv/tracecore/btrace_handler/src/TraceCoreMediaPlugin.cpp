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
// Trace Core media plugin
//

#include "TraceCoreMediaPlugin.h"
#include "TraceCorePluginIf.h" 
#include "TraceCore.h"
#include "TraceCoreSendReceive.h"
#include "TraceCoreMessageUtils.h"
#include "OstTraceDefinitions.h"
#include "TraceCoreSubscriber.h"

#ifdef OST_TRACE_COMPILER_IN_USE
#include "TraceCoreMediaPluginTraces.h"
#endif



/**
 * Constructor
 */
EXPORT_C DTraceCoreMediaPlugin::DTraceCoreMediaPlugin()
    {
    }


/**
 * Destructor
 */
EXPORT_C DTraceCoreMediaPlugin::~DTraceCoreMediaPlugin()
    {
    Unregister();
    }


/**
 * Registers this plug-in to TraceCore
 */
EXPORT_C TInt DTraceCoreMediaPlugin::Register()
    {
    TInt ret = KErrNone;
    
    // Get TraceCore
    DTraceCore* traceCore = DTraceCore::GetInstance();
    if ( traceCore != NULL )
        {
        
        // Get TraceCore Plugin interface and register this as a plugin
        DTraceCorePluginIf* media = traceCore->GetSendReceive().GetPluginIf();
        if ( media != NULL )
            {
            ret = media->RegisterPlugin( *this );
            }
        else
            {
            ret = KErrGeneral;
            }
        }
    else
        {
        ret = KErrGeneral;
        }
    OstTrace1( TRACE_BORDER, DTRACECOREMEDIAPLUGIN_REGISTER_EXIT, "< DTraceCoreMediaPlugin::Register. Ret:%d", ret );
    return ret;
    }


/**
 * Called by the plug-in to notify TraceCore that message was received
 * 
 * @param aMsg The message that was received
 */
EXPORT_C void DTraceCoreMediaPlugin::MessageReceived( TTraceMessage &aMsg )
    {
    OstTraceExt1( TRACE_BORDER, DTRACECOREMEDIAPLUGIN_MESSAGERECEIVED_ENTRY, "> DTraceCoreMediaPlugin::MessageReceived. MsgId:0x%hhx", aMsg.iMessageId );
    DTraceCore* traceCore = DTraceCore::GetInstance();
    if ( traceCore != NULL )
        {
        DTraceCorePluginIf* media = traceCore->GetSendReceive().GetPluginIf();
        if ( media != NULL )
            {
            media->MessageReceived( aMsg );
            }
        }
    }
    

/**
 * Merges the header and data into a single buffer
 * 
 * @param aMsg Message to be sent.
 * @param aMsgBlock The message block where data is merged
 * @return Symbian error code
 */
EXPORT_C TInt DTraceCoreMediaPlugin::MergeHeaderAndData( const TTraceMessage& aMsg, TDes8& aTarget )
	{
	TInt ret = KErrNone;
	TInt len = TTraceCoreMessageUtils::GetMessageLength( aMsg );
	if ( len > 0 )
		{
		ret = TTraceCoreMessageUtils::MergeHeaderAndData( aMsg, aTarget );
		}
	else
		{
		ret = len;
		}
	OstTrace1( TRACE_FLOW, DTRACECOREMEDIAPLUGIN_MERGEHEADERANDDATA_EXIT, "< DTraceCoreMediaPlugin::MergeHeaderAndData. Ret:%d", ret );
	return ret;
	}
 
/**
 * Unregisters this plug-in
 */
void DTraceCoreMediaPlugin::Unregister()
    {
    // Get TraceCore
    DTraceCore* traceCore = DTraceCore::GetInstance();
    if ( traceCore != NULL )
        {
        
        // Get Plugin interface and unregister this from it
        DTraceCorePluginIf* media = traceCore->GetSendReceive().GetPluginIf();
        if ( media != NULL )
            {
            media->UnregisterPlugin( *this );
            }
        }
    }

// End of File
