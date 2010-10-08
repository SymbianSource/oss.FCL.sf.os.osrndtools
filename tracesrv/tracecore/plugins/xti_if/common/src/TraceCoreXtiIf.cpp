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


//- Include Files  ----------------------------------------------------------
#include <pn_const.h>
#include <phonetisi.h>
#include <xtirx.h>
#include <xtitx.h>

#include "TraceCore.h"
#include "TraceCoreXtiIf.h" 
#include "TraceCoreDebug.h"
#include "TraceCoreMediaIfCallback.h"
#include "TraceCoreSubscriber.h"
#include "TraceCoreMessageUtils.h"

#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "TraceCoreXtiIfTraces.h"
#endif

#include "xtipluginvariantconstants.h"

/**
 * 200 millisecond pulse time (0.2 sec)
 */
const TUint KXtiRegisterTimer = 200000;

/** 
 * Default DFC queue priority
 */
const TUint8 KDefaultDfcPriority = 3;

/*
 * ISI header remainder length, 2 + 2 filler bytes
 */
const TInt KIsiHeaderRemainder = 4;


/**
 * Prints a message
 *
 * @param aMsg Reference to a message. 
 */
static void DebugPrintMsg( const TDesC8& aMsg )            
    {
    // Phonet message structure print
    OstTraceData( TRACE_DUMP, TRACECOREXTIIF_PS_DEBUGPRINTMSG,
    		"DTraceCoreXtiIf::DebugPrintMsg - Msg via XTI IF 0x%{hex8[]}", aMsg.Ptr(), aMsg.Size() );
    }

/**
 * Constructor
 */
DTraceCoreXtiIf::DTraceCoreXtiIf()
: iReceiveDfc( DTraceCoreXtiIf::ReceiveDfc, this, DTraceCore::GetActivationQ(), KDefaultDfcPriority )
, iXtiRegisterDfc( DTraceCoreXtiIf::XtiRegister, this, DTraceCore::GetActivationQ(), KDefaultDfcPriority )
, iStatus( 0 )
    {
    // No implementation in constructor
    }

/**
 * Destructor
 */
DTraceCoreXtiIf::~DTraceCoreXtiIf()
    {
    //iCallback = NULL; 
    }

/**
 * Initializes the XTI IF
 *
 * @param aCallBack Callback to TraceCore
 */
TInt DTraceCoreXtiIf::Init( /*MTraceCoreMediaIfCallback& aCallback*/ )
    {
    TC_TRACE( ETraceLevelFlow, Kern::Printf( "> DTraceCoreXtiIf::Init" ) );
    
    // Register media plugin to TraceCore
    TInt ret = Register();
    
    StartTimer();
    TC_TRACE( ETraceLevelFlow, Kern::Printf( "< DTraceCoreXtiIf::Init" ) );
    return KErrNone;
    }

/**
 * Starts the timer which calls the initialization function
 */
void DTraceCoreXtiIf::StartTimer()
    {
    TC_TRACE( ETraceLevelFlow, Kern::Printf( "> DTraceCoreXtiIf::StartTimer" ) );
    iXtiRegisterTimer.OneShot( KXtiRegisterTimer, XtiRegisterDfc, this );
    TC_TRACE( ETraceLevelFlow, Kern::Printf( "< DTraceCoreXtiIf::StartTimer" ) );
    }

/**
 * Add registering to own Dfc
 * 
 * @param aPtr Pointer to traceCoreIsaIf class
 */
void DTraceCoreXtiIf::XtiRegisterDfc( TAny* aPtr )
    {
    TC_TRACE( ETraceLevelFlow, Kern::Printf( "> DTraceCoreXtiIf::XtiRegisterDfc" ) );
    reinterpret_cast< DTraceCoreXtiIf* >( aPtr )->iXtiRegisterDfc.Enque();
    TC_TRACE( ETraceLevelFlow, Kern::Printf( "< DTraceCoreXtiIf::XtiRegisterDfc" ) );
    }

/**
 * Timer callback function to initialize the XTI IF
 * 
 * @param aPtr Pointer to DTraceCoreXtiIf class
 */
void DTraceCoreXtiIf::XtiRegister( TAny* aPtr )
    {
    TC_TRACE( ETraceLevelFlow, Kern::Printf( "> DTraceCoreXtiIf::XtiRegister" ) );
    DTraceCoreXtiIf* traceCoreXtiIf = static_cast< DTraceCoreXtiIf* >( aPtr );
    __ASSERT_DEBUG( traceCoreXtiIf != NULL, 
            Kern::Fault( "DTraceCoreXtiIf::XtiRegister - NULL", KErrGeneral ) );
    traceCoreXtiIf->XtiRegister();
    TC_TRACE( ETraceLevelFlow, Kern::Printf( "< DTraceCoreXtiIf::XtiRegister" ) );
    }

/**
 * Called from the static callback to register to XtiRx
 */
void DTraceCoreXtiIf::XtiRegister()
    {
    TInt ret = XtiRx::Register( &iReceiveDfc, iReceiveBuffer );
    if ( ret != KErrNone )
        {
        TC_TRACE( ETraceLevelError, Kern::Printf( "DTraceCoreXtiIf::XtiRegister - Register failed: %d", ret ) );
        }
    }

/**
 * Handle incoming messages from XTI
 * 
 * @param aPtr Pointer to DTraceCoreXtiIf class.
 */
void DTraceCoreXtiIf::ReceiveDfc( TAny* aPtr )
    {
    OstTrace0( TRACE_FLOW, DTRACECOREXTIIF_PS_RECEIVEDFC_ENTRY, "> DTraceCoreXtiIf::ReceiveDfc");
    DTraceCoreXtiIf* traceCoreXtiIf = static_cast< DTraceCoreXtiIf* >( aPtr );
    __ASSERT_DEBUG( traceCoreXtiIf != NULL, 
            Kern::Fault( "DTraceCoreXtiIf::ReceiveDfc - NULL", KErrGeneral ) );
    traceCoreXtiIf->ReceiveDfc();
    OstTrace0( TRACE_FLOW, DTRACECOREXTIIF_PS_RECEIVEDFC_EXIT, "< DTraceCoreXtiIf::ReceiveDfc");
    }

/**
 * Processes incoming XTI messages
 */
void DTraceCoreXtiIf::ReceiveDfc()
    {
    // Determine the message we received
    if ( iReceiveBuffer.Length() > ISI_HEADER_OFFSET_MESSAGEID )
        {
        TUint8 resource = iReceiveBuffer[ ISI_HEADER_OFFSET_RESOURCEID ];
        
        // Check that resource is PN_EPOC_MON
        if ( resource == PN_EPOC_MON )
            {
            DebugPrintMsg ( iReceiveBuffer );
            
            NotifyCallback();
            }
        }
    XtiRx::SetReceiveBufferRead();    
    }

/**
 * Notifies the callback with the incoming message
 * 
 * @return one of symbian error codes
 */
void DTraceCoreXtiIf::NotifyCallback()
    {
    OstTrace0( TRACE_FLOW, DTRACECOREXTIIF_PS_NOTIFYCALLBACK_ENTRY, "> DTraceCoreXtiIf::NotifyCallback");
    // Header length is header size + remainders
    TInt headerLength = ISI_HEADER_SIZE + KIsiHeaderRemainder;
    if ( iReceiveBuffer.Length() < headerLength ) {
        headerLength = iReceiveBuffer.Length();
    }
    
    // Create TraceMessage
    TTraceMessage traceMsg;
    TPtrC header( iReceiveBuffer.Ptr(), headerLength );
    TPtrC data( NULL, 0 );
    if ( iReceiveBuffer.Length() > headerLength ) {
        data.Set( iReceiveBuffer.Ptr() + headerLength, iReceiveBuffer.Length() - headerLength );
    }

    traceMsg.iMsgFormat = EMessageHeaderFormatProprietary;
    traceMsg.iHeader = &header;
    traceMsg.iData = &data;
    traceMsg.iMessageId = iReceiveBuffer[ ISI_HEADER_OFFSET_MESSAGEID ];
    
    // Call base class MessageReceived
    MessageReceived( traceMsg );
    
    OstTrace0( TRACE_FLOW, DTRACECOREXTIIF_PS_NOTIFYCALLBACK_EXIT, "< DTraceCoreXtiIf::NotifyCallback");
    }

/**
 * Sends messages to XTI
 * 
 * @param aMsg Message to be sent.
 * @return KErrNone if send successful
 */
TInt DTraceCoreXtiIf::Send( TTraceMessage& aMsg )
    {
    OstTrace0( TRACE_FLOW, DTRACECOREXTIIF_PS_SEND_ENTRY, "> DTraceCoreXtiIf::Send");
    TInt ret;
    if ( aMsg.iData != NULL )
        {
        ret = TXtiIfMessageUtils::GetMessageLength( aMsg );
        if ( ret > 0 )
            {
            // First send the phone formatted start character
            ret = XtiTx::StartWrite( KChannel0, KPhoneFormattedMessage );
            if ( ret == KErrNone )
                {
                // Send trace data and end trace
                SendData( aMsg );
                XtiTx::StopWrite( KChannel0 );
                if ( aMsg.iHeader != NULL ) 
                    {
                    DebugPrintMsg( iSendBuffer );
                    }
                else
                    {
                    DebugPrintMsg( *aMsg.iData );
                    }
                }
            else
                {
                OstTrace1( TRACE_IMPORTANT , DTRACECOREXTIIF_PS_SEND_START_WRITE_FAILED,
                		"DTraceCoreXtiIf::Send - XTI StartWrite failed. Ret:%d", ret );
                }
            }
        else
            {
            ret = KErrArgument;
            }
        }
    else
        {
        OstTrace0( TRACE_IMPORTANT , DTRACECOREXTIIF_PS_SEND_INVALID_DATA, "DTraceCoreXtiIf::Send - NULL data in message");
        ret = KErrArgument;
        }
    OstTrace1( TRACE_FLOW, DTRACECOREXTIIF_PS_SEND_EXIT, "< DTraceCoreXtiIf::Send. Ret:%d", ret );
    return ret;
    }
    
/**
 * Called from the TraceCore to send message out.
 */
TInt DTraceCoreXtiIf::SendMessage( TTraceMessage &aMsg )
    {
    TInt ret = Send( aMsg );
    return ret;
    }

/**
 * Sends message to XTI
 * 
 * @param aMsg Message to be sent.
 */
void DTraceCoreXtiIf::SendData( TTraceMessage& aMsg )
    {
    const TText8* pS = NULL;
    const TText8* pE = NULL;
    if ( aMsg.iHeader != NULL )
        {
        // Utility function is used to merge the data
        TInt ret = TXtiIfMessageUtils::MergeHeaderAndData( aMsg, iSendBuffer );
        if ( ret == KErrNone )
            {
            pS = iSendBuffer.Ptr();
            pE = pS + iSendBuffer.Length();
            }
        }
    else
        {
        // Header doesn't exists, all data is in data part
        pS = aMsg.iData->Ptr();
        pE = pS + aMsg.iData->Length();
        }
    
    // Print using 8bit write
    while ( pS < pE )
        {
        XtiTx::WriteData8( KChannel0, *( pS++ ) );
        }
    }

/**
 * The entry point for a standard extension. Creates TraceCoreXtiIfPlugin extension.
 *
 *  @return KErrNone, if successful
 */
DECLARE_STANDARD_EXTENSION()    //lint !e960 !e1717 ¤/#((& Symbian
    {
    TC_TRACE( ETraceLevelNormal, Kern::Printf( "DECLARE_STANDARD_EXTENSION Start Trace Core XTI IF Plugin" ) );
    
    TInt ret = KErrNone;
    
       // Certificate check should be done here e.g. if ( XtiTx::IsTraceEnabled() )
       // CpuPage::IsRdcAvailable();
        {
        DTraceCoreXtiIf* xtiIfPlugin = new DTraceCoreXtiIf();
        
        if ( xtiIfPlugin != NULL )
            {
            ret = xtiIfPlugin->Init();
            if ( ret != KErrNone )
                {
                delete xtiIfPlugin;
                }
            }
        else
            {
            ret = KErrNoMemory;
            }
        }

	if (ret == KErrNone) 
		{
        POSTCODE( Kern::Printf("[POST][TraceCoreXtiIfPlugin][Start][OK]"); )
		}
	else
		{
        POSTCODE( Kern::Printf("[POST][TraceCoreXtiIfPlugin][FAIL][%d]", ret ); )
		}           
        
    return ret;
    }

// End of File
