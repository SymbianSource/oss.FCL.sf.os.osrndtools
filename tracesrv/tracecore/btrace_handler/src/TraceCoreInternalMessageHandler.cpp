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
// TraceCore configuration message subscriber
//

#include "TraceCoreInternalMessageHandler.h"
#include "TraceCoreConstants.h"
#include "TraceCoreDebug.h"
#include "TraceCore.h"
#include "TraceCoreMessageSender.h"

#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "TraceCoreInternalMessageHandlerTraces.h"
#endif



/**
 * TraceCore protocol id
 */
const TInt KOSTTraceCoreProtocolId = 0x91;

/**
 * Version for version query
 */
const TInt KOSTVersionForVersionQuery = 0x00;

/**
 * Version / System protocol id
 */
const TInt KOSTVersionSystemProtocolId= 0x00;

/**
 * Version align request
 */
const TInt KOSTAlignVersionRequest = 0x00;

/**
 * Version align response
 */
const TInt KOSTAlignVersionResponse = 0x01;

/**
 * Version unsupported version error
 * const TInt KOSTUnsupportedVersionError = 0xFD;
 */

/**
 * Version MessageSyntaxError
 * const TInt KOSTMessageSyntaxError = 0xFE;
 */

/**
 * Version UnknownMessageError
 * const TInt KOSTUnknownMessageError = 0xFF;
 */

/**
 * Version MessageSyntaxError
 */
const TInt KOSTNoMatchingVersion = 0xFE;

/**
 * Version UnknownMessageError
 * const TInt KOSTUnknownError = 0xFF;
 */

/**
 * Higest supported version
 */
const TInt KOstHigestSupportedVersion = 0x01;

/**
 * Version protocol's transaction id offset after length
 * offset related to data after header(4bytes)
 * (4) 0 transaction id
 * (5) 1 Message id
 * (6) 2 Result
 * (7) 3 Filler
 */
const TInt KOstVersionProtocolTransactionIdOffset = 0x00;

/**
 * Version protocol's message id offset after length
 * offset related to data after header(4bytes)
 */
const TInt KOstVersionProtocolMessageIdOffset = 0x01;

/**
 * Version protocol's Result offset after length
 * offset related to data after header(4bytes)
 * used only in response
 */
const TInt KOstVersionProtocolResultRespOffset = 0x02;

/**
 * Version protocol's Filler offset after length
 * offset related to data after header(4bytes)
 * used only in response
 */
const TInt KOstVersionProtocolFillerRespOffset = 0x03;

/**
 * Version protocol's Versions field offset after length
 * offset related to data after header(4bytes)
 * used only in request
 */
const TInt KOstVersionProtocolVersionsRequestOffset = 0x02;

/**
 * Version protocol's Supported Version field offset after length
 * offset related to data after header(4bytes)
 * used only in response
 */
const TInt KOstVersionProtocolSupportedVersionsRespOffset = 0x04;

/**
 * KAlignVersionResponseDataLength
 */
const TInt KAlignVersionResponseDataLength = 0x05;

/**
 * Offset to message ID within configuration request
 * const TInt KMessageIDOffset = 0;
 */

/**
 * Media selection request length
 */
const TInt KPingReqLength = 1;

/**
 * Data length of response
 */
const TInt KResponseDataLength = 4;


/**
 * Offset to response message
 */
const TInt KRespMessageIdOffset = 0;

/**
 * Offset to response code
 */
const TInt KRespCodeOffset = 1;

/**
 * Offset to fillers
 */
const TInt KFillerOffset = 2;

/**
 * Filler byte value
 */
const TUint8 KFiller = 0;


/**
 * Constructor
 */
DTraceCoreInternalMessageHandler::DTraceCoreInternalMessageHandler()
    {
    }


/**
 * Destructor
 */    
DTraceCoreInternalMessageHandler::~DTraceCoreInternalMessageHandler()
    {
    }


/**
 * Init configuration
 */         
TInt DTraceCoreInternalMessageHandler::Init()
    {
    // Subscribe OST
    TInt err = Subscribe( KOSTTraceCoreProtocolId, EMessageHeaderFormatOst );
    if(err == KErrNone)
        {
        err = Subscribe( KOSTVersionSystemProtocolId, EMessageHeaderFormatOst );
        }
    TC_TRACE( ETraceLevelFlow, Kern::Printf( "< DTraceCoreInternalMessageHandler::Init - ret: %d", err ) );
    return err;
    }


/**
 * Callback when a trace activation message is received
 *
 * @param aMsg Reference to message
 */
void DTraceCoreInternalMessageHandler::MessageReceived( TTraceMessage &aMsg )
    {
    OstTrace1( TRACE_FLOW, DTraceCoreInternalMessageHandler_MESSAGERECEIVED_ENTRY, "> DTraceCoreInternalMessageHandler::MessageReceived - %d", aMsg.iMessageId);
    TInt aVersion( ( *aMsg.iHeader )[ 0 ] );
    
    if (aVersion > KOSTVersionForVersionQuery)
        {
        // Add version check later and return KOSTUnsupportedVersionError if version not supported
        
        //Check that there is enough data
        if ( aMsg.iData->Length() >= KPingReqLength )
            {
            TInt messageID( ( *aMsg.iData )[ 0 ] );
            
            // Select media request message
            if ( messageID == 0x00 /* ping subprotocolId 0x00 in side TraceCore protocol 0x91 */ )
                {
                SendResponse( aMsg, messageID );
                }
            }
        }
    else
        {
        // Version query        
        HandleOstVersionQuery( aMsg, 0 );
        }        
    }


/**
 * Sends a response message
 * 
 * @param aMsg the incoming message
 * @param aResult the result code
 */
void DTraceCoreInternalMessageHandler::SendResponse( TTraceMessage& aMsg, TInt aMessageId )
    {
    OstTrace1( TRACE_FLOW, DTraceCoreInternalMessageHandler_SENDRESPONSE_ENTRY, "> DTraceCoreInternalMessageHandler::SendResponse - Msg.id %d", aMsg.iMessageId);
    TUint8 respCode;
    if ( aMessageId == 0x00 )
        {
        respCode = 0x01;
        }
    else
        {
        respCode = 0x02;
        }
        
    TTraceMessage resp;
    TBuf8< KResponseDataLength > respData;
    respData.SetLength( KResponseDataLength );
    respData[ KRespMessageIdOffset ] = aMessageId;
    respData[ KRespCodeOffset ] = respCode;
    for ( TInt i = KFillerOffset; i < KResponseDataLength; i++ )
        {
        respData[ i ] = KFiller;
        }
    // The original header from the request is set back to the message
    //  -> Media API takes care of formatting the header
    resp.iHeader = aMsg.iHeader;
    resp.iData = &respData;
    resp.iMessageId = KOSTTraceCoreProtocolId;
    resp.iMsgFormat = aMsg.iMsgFormat;
    iMessageSender->SendMessage( resp );
    }

/**
 * Handle version query
 * 
 * @param aMsg the incoming message
 * @param aResult the result code
 */
void DTraceCoreInternalMessageHandler::HandleOstVersionQuery( TTraceMessage& aMsg, TInt /*aMessageId*/ )
    {
    OstTrace1( TRACE_NORMAL, DTraceCoreInternalMessageHandler_HandleOstVersionQuery_entry, "DTraceCoreInternalMessageHandler::HandleOstVersionQuery Msg.id - %d", aMsg.iMessageId);
    // Check if align request (should be! not other request under version protocol)
    TInt higestSupportedVersion( 0x00 ); // 0x00 is sent if no  supported version
    TInt messageId( ( *aMsg.iData )[ KOstVersionProtocolMessageIdOffset ] );
    
    if( messageId == KOSTAlignVersionRequest)
        {    
        // Read the list of supported versions in sender side and compare to phone side version
        TInt versions = aMsg.iData->Length() - 2;
        for( TInt i = 0; i < versions ; i++ )
            {
            TInt version( ( *aMsg.iData )[ KOstVersionProtocolVersionsRequestOffset+i ] );
            
            // Compare versions to supported version
            // (When more version supported, create list and compare to it)
            if ( version == KOstHigestSupportedVersion )
                {
                // Supported version found
                higestSupportedVersion = version;
                }
            }
            
        TUint8 respResult = KOSTNoMatchingVersion;
        
        if ( higestSupportedVersion !=  0x00)
            {
            // Supported version found
            respResult = 0x00; // Succeed
            }
        
        TTraceMessage resp;
        TBuf8< KAlignVersionResponseDataLength > respData;
        respData.SetLength( KAlignVersionResponseDataLength ); // AlignVersionResponseDataLength
        respData[ KOstVersionProtocolTransactionIdOffset ] = 0x00;
        respData[ KOstVersionProtocolMessageIdOffset ] = KOSTAlignVersionResponse;
        respData[ KOstVersionProtocolResultRespOffset ] = respResult;
        respData[ KOstVersionProtocolFillerRespOffset ] = 0x00; // Filler
        respData[ KOstVersionProtocolSupportedVersionsRespOffset ] = higestSupportedVersion;
        
        // The original header from the request is set back to the message
        //  -> Media API takes care of formatting the header
        resp.iHeader = aMsg.iHeader;
        resp.iData = &respData;
        resp.iMessageId = KOSTVersionSystemProtocolId;
        resp.iMsgFormat = aMsg.iMsgFormat;
        
        iMessageSender->SendMessage( resp );
        
        }
}

// End of File
