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

#include "TraceCore.h"
#include "TraceCoreOstLddIf.h" 
#include "TraceCoreDebug.h"
#include "TraceCoreMediaIfCallback.h"
#include "TraceCoreSubscriber.h"
#include "TraceCoreMessageUtils.h"
#include "TraceCoreOstHeader.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "TraceCoreOstLddIfTraces.h"
#endif

/** 
 * Default DFC queue priority
 */
const TUint8 KDefaultDfcPriority = 3;

/**
 * Prints a message
 *
 * @param aMsg Reference to a message. 
 */
static void DebugPrintMsg( const TDesC8& /*aMsg*/ )            
    {
    // Message structure print
    }

static DTraceCoreOstLddIf* TraceCoreOstLddIf = NULL;

/**
 * Constructor
 */
DTraceCoreOstLddIf::DTraceCoreOstLddIf()
: DTraceCoreMediaIf( KMediaIfSendSupported | KMediaIfSendTraceSupported  )
, iReceiveDfc( DTraceCoreOstLddIf::ReceiveDfc, this, DTraceCore::GetActivationQ(), KDefaultDfcPriority )
, iStatus( 0 )
, iTraceCoreOstConnectionRegistered( EFalse )
, iTraceCoreOstConnection( NULL )
    {
    // No implementation in constructor
    }


/**
 * Destructor
 */
DTraceCoreOstLddIf::~DTraceCoreOstLddIf()
    {
    iCallback = NULL; 
    }


/**
 * Initializes the OstLddIf
 *
 * @param aCallBack Callback to TraceCore
 */
TInt DTraceCoreOstLddIf::Init( MTraceCoreMediaIfCallback& aCallback )
    {
    iCallback = &aCallback;
    TraceCoreOstLddIf = this;
    return KErrNone;
    }


/**
 * Handle incoming messages from OstConnection
 * 
 * @param aPtr Pointer to DTraceCoreOstLddIf class.
 */
void DTraceCoreOstLddIf::ReceiveDfc( TAny* aPtr )
    {
    DTraceCoreOstLddIf* TraceCoreOstLddIf = static_cast< DTraceCoreOstLddIf* >( aPtr );
    __ASSERT_DEBUG( TraceCoreOstLddIf != NULL, Kern::Fault( "DTraceCoreOstLddIf::ReceiveDfc - NULL", KErrGeneral ) );
    TraceCoreOstLddIf->ReceiveDfc();
    }


/**
 * Processes incoming OST messages
 */
void DTraceCoreOstLddIf::ReceiveDfc()
    {
    // Determine the message we received
    if ( iReceiveBuffer.Length() > OstHeader::OstBaseProtocol::KOstHeaderLengthOffset )
        {
        DebugPrintMsg ( iReceiveBuffer );
        if ( iCallback != NULL )
            {
            NotifyCallback();
            }
        }
    }


/**
 * Notifies the callback with the incoming message
 * 
 * @return one of symbian error codes
 */
void DTraceCoreOstLddIf::NotifyCallback()
    {
    // Header length is header size + remainders
    TInt headerLength = OstHeader::OstBaseProtocol::KOstBaseHeaderSize;

    // Check message ID
    TTraceMessage traceMsg;
    traceMsg.iMessageId = iReceiveBuffer[ OstHeader::OstBaseProtocol::KOstHeaderProtocolIdOffset ];
    
    // If message ID is TraceCore Protocol, the header is longer and the message ID might be in different place
    if (traceMsg.iMessageId == OstConstants::OstBaseProtocol::KOstTraceCoreProtocol)
        {
        headerLength += OstHeader::OstTraceCoreProtocol::KHeaderSize;
        
        // Subscriber protocol
        if (iReceiveBuffer[ OstHeader::OstBaseProtocol::KOstBaseHeaderSize + 
                            OstHeader::OstTraceCoreProtocol::KProtocolIdOffset ] == 
                OstHeader::OstTraceCoreProtocol::KSubscriberProtocol)
            {
            traceMsg.iMessageId = iReceiveBuffer[ OstHeader::OstBaseProtocol::KOstBaseHeaderSize + 
                                                  OstHeader::OstTraceCoreProtocol::KSubscriberIdOffset ];
            }
        }
    
    if ( iReceiveBuffer.Length() < headerLength ) 
        {
        headerLength = iReceiveBuffer.Length();
        }

    // Create TraceMessage
    TPtrC header( iReceiveBuffer.Ptr(), headerLength );
    TPtrC data( NULL, 0 );
    if ( iReceiveBuffer.Length() > headerLength ) {
        data.Set( iReceiveBuffer.Ptr() + headerLength, iReceiveBuffer.Length() - headerLength );
    }

    traceMsg.iMsgFormat = EMessageHeaderFormatOst;
    traceMsg.iHeader = &header;
    traceMsg.iData = &data;
    
    // Set Sender media to be same as receiver
    iCallback->SetSenderMedia(this);
    
    iCallback->MessageReceived( traceMsg );
    }


/**
 * Sends messages to USB OstConnection
 * 
 * @param aMsg Message to be sent.
 * @return KErrNone if send successful
 */
TInt DTraceCoreOstLddIf::Send( TTraceMessage& aMsg )
    {
    TInt ret = KErrArgument;
    if ( aMsg.iData != NULL )
        {
        ret = TTraceCoreMessageUtils::GetMessageLength( aMsg );
        if ( ret > 0 )
            {
            // First send the phone formatted start character
            // Send trace data and end trace
            SendData( aMsg );
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
            ret = KErrArgument;
            }
        }
    
    return ret;
    }
    

/**
 * Sends message to TraceCoreOstLdd
 * 
 * @param aMsg Message to be sent.
 */
void DTraceCoreOstLddIf::SendData( TTraceMessage& aMsg )
    {
    if ( aMsg.iHeader != NULL )
        {
        // Utility function is used to merge the data
        TInt ret = TTraceCoreMessageUtils::MergeHeaderAndData( aMsg, iSendBuffer );
        
        if ( ret == KErrNone )
            {
            if(iTraceCoreOstConnection)
                {
                iTraceCoreOstConnection->TxMessage( iSendBuffer );
                }
            }
        }
    else
        {
         // Header doesn't exists, all data is in data part
        }
    }


/**
 * Similar to send, except does not generate traces, since they would loop back to this function
 * 
 * @param aMsg Message to be sent.
 * @return KErrNone if send successful
 */
TInt DTraceCoreOstLddIf::SendTrace(const TDesC8& aMsg)
    {
    TInt ret(KErrNone);
    
    if ( iTraceCoreOstConnection && iTraceCoreOstConnectionRegistered )
        {
        iTraceCoreOstConnection->TxMessage(aMsg);
        }
    else
        {
        ret = KErrGeneral;
        }
    
    return ret;
    }


/**
 * GetSet
 * 
 * @param aFunc Function choosed (@see ETraceCoreOstLddIfGetSet)
 * @return TDfc* if aFunc is EGetDfc, TDes8* EGetBuf and 0 with ESetWriter.
 */
EXPORT_C TUint32 DTraceCoreOstLddIf::GetSet(TTraceCoreOstLddIfGetSet aFunc, TUint32 aParam)
    {
    OstTraceExt2( TRACE_FLOW, DTRACECOREOSTLDDIF_GETSET_ENTRY, "> DTraceCoreOstLddIf::GetSet aFunc:%d, aParam:%d", aFunc, aParam );
    
    TUint32 ret(0);
    switch(aFunc)
        {
        case EGetDfc:
            {
            TraceCoreOstLddIf->iTraceCoreOstConnectionRegistered = ETrue;
            ret = reinterpret_cast<TUint32>(&(TraceCoreOstLddIf->iReceiveDfc));
            break;
            }
        case EGetBuf:
            {
            ret = reinterpret_cast<TUint32>(&(TraceCoreOstLddIf->iReceiveBuffer));
            break;
            }
        case ESetWriter:
            {
            // Set Writer to XTI if parameter is 0, otherwise USB
            DTraceCore* traceCore = DTraceCore::GetInstance();
            if ( traceCore != NULL )
                {
                // TraceSwitch uses 0 for XTI writer and 1 to OST writer
                // So it is not mapping one to one to enum found in TraceCoreWriter.h
                // for example to EWriterTypeXTI = 1
                // See TraceCoreWriter.h
                if(aParam==0)
                    {
                    traceCore->SwitchToWriter(EWriterTypeXTI);
                    }
                else if(aParam==1)
                    {
                    traceCore->SwitchToWriter(EWriterTypeUSBPhonet);
                    }
                // For other writers (>2) we use same as the enum
                else if(aParam > EWriterTypeUSBPhonet)
                    {
                    traceCore->SwitchToWriter((TWriterType) aParam);
                    }
                }
            break;
            }
        case ERegister:
            {
            TraceCoreOstLddIf->iTraceCoreOstConnection = (MTraceCoreOstConnectionIf*)aParam;
            break;
            }    
        default:
            __ASSERT_DEBUG( EFalse, Kern::Fault( "DTraceCoreOstLddIf::GetSet: Default case!", KErrArgument ) );
            break;
        }
    
    return ret;
    }

// End of File
