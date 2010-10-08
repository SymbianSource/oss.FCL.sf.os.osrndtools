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


#include <pn_const.h>
#include <phonetisi.h>
#include "TraceCoreMessageUtils.h"
#include "TraceCoreSubscriber.h"
#include "TraceCoreOstHeader.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "XtiIfMessageUtilsTraces.h"
#endif


/*
 * ISI header remainder length, 2 + 2 filler bytes
 */
const TInt KIsiHeaderRemainder = 4;

/*
 * Shift one byte
 */
const TInt KByteShift = 8;

/*
 * Byte mask value
 */
const TInt KByteMask = 0xFF;


/**
 * Merges the header and data into a single buffer
 * 
 * @param aMsg Message to be sent.
 * @param aMsgBlock The message block where data is merged
 * @return Symbian error code
 */
TInt TXtiIfMessageUtils::MergeHeaderAndData( const TTraceMessage& aMsg, TDes8& aTarget )
	{
    TInt ret(KErrNotSupported);
    if ( aMsg.iMsgFormat == EMessageHeaderFormatProprietary )
        {
        // Merge header and data
        aTarget.Copy( *aMsg.iHeader );
        aTarget.Append( *aMsg.iData );
         
        // Flip receiver and sender devices
        TUint8 tmp = aTarget[ ISI_HEADER_OFFSET_RECEIVERDEVICE ];
        aTarget[ ISI_HEADER_OFFSET_RECEIVERDEVICE ] = aTarget[ ISI_HEADER_OFFSET_SENDERDEVICE ];
        aTarget[ ISI_HEADER_OFFSET_SENDERDEVICE ] = tmp;
         
        // Flip receiver and sender objects
        tmp = aTarget[ ISI_HEADER_OFFSET_RECEIVEROBJECT ];
        aTarget[ ISI_HEADER_OFFSET_RECEIVEROBJECT ] = aTarget[ ISI_HEADER_OFFSET_SENDEROBJECT ];
        aTarget[ ISI_HEADER_OFFSET_SENDEROBJECT ] = tmp;
        
        // Assign message length
        TInt length = aMsg.iHeader->Length() + aMsg.iData->Length() - PN_HEADER_SIZE;
        
#ifndef __WINS__
        aTarget[ ISI_HEADER_OFFSET_LENGTH ] = length & 0xFF; // CodForChk_Dis_Magic
        aTarget[ ISI_HEADER_OFFSET_LENGTH + 1 ] = ( length >> 8 ) & 0xFF; // CodForChk_Dis_Magic
#else
        aTarget[ ISI_HEADER_OFFSET_LENGTH + 1 ] = length & 0xFF; // CodForChk_Dis_Magic
        aTarget[ ISI_HEADER_OFFSET_LENGTH] = ( length >> 8 ) & 0xFF; // CodForChk_Dis_Magic
#endif
                
        // Assign new message ID
        aTarget[ ISI_HEADER_OFFSET_MESSAGEID ] = aMsg.iMessageId;
        ret = KErrNone;
        }
    else
        if (aMsg.iMsgFormat == EMessageHeaderFormatOst)
            {
            // Merge header and data
            aTarget.Copy( *aMsg.iHeader);
            aTarget.Append( *aMsg.iData);

            
            // Assign message length
            TUint16 length = aMsg.iHeader->Length() + aMsg.iData->Length()
                    - OstHeader::OstBaseProtocol::KOstBaseHeaderSize;
            
            aTarget[ OstHeader::OstBaseProtocol::KOstHeaderLengthOffset ] = (length >> KByteShift ) & KByteMask;
            aTarget[ OstHeader::OstBaseProtocol::KOstHeaderLengthOffset + 1 ] = length & KByteMask;

            // Assign new message ID
            aTarget[ OstHeader::OstBaseProtocol::KOstHeaderProtocolIdOffset ] = aMsg.iMessageId;
            ret = KErrNone;
            }
    return ret;
    }


/**
 * Get the message length 
 * 
 * @param aMsg Message to be sent.
 * @return length or -1 if not valid
 */
TInt TXtiIfMessageUtils::GetMessageLength( const TTraceMessage& aMsg )
    {
    OstTrace0( TRACE_FLOW, TRACECOREMESSAGEUTILS_GETMESSAGELENGTH_ENTRY, "> TraceCoreMessageUtils::GetMessageLength");
    TInt msgLength;
    if ( aMsg.iMsgFormat == EMessageHeaderFormatProprietary )
        {
        // Get message length
        msgLength = aMsg.iData->Length();
        if ( aMsg.iHeader != NULL )
            {
            TInt hdrLen( aMsg.iHeader->Length() );
            // Message header length must be valid
            if ( hdrLen == ( ISI_HEADER_SIZE + KIsiHeaderRemainder ) )
                {
                msgLength += hdrLen;
                }
            else
                {
                OstTrace1( TRACE_IMPORTANT , TTRACECOREISAUTILS_GETMESSAGELENGTH_INVALID_HEADER_LENGTH,
                		"TTraceCoreIsaUtils::GetMessageLength - ISI header length was not valid. Len:%d", hdrLen );
                msgLength = -1;
                }
            }
        else
            {
            // If message length is not enough for valid ISI message, an error is returned
            if ( msgLength < ( ISI_HEADER_SIZE + KIsiHeaderRemainder ) )
                {
                OstTrace1( TRACE_IMPORTANT , TTRACECOREISAUTILS_GETMESSAGELENGTH_MESSAGE_TOO_SHORT,
                		"TTraceCoreIsaUtils::GetMessageLength - Message is too short. Len:%d", msgLength );
                msgLength = -1;
                }
            }
        }
    else
        if (aMsg.iMsgFormat == EMessageHeaderFormatOst)
            {
            // Get message length
            msgLength = aMsg.iData->Length();
            if (aMsg.iHeader != NULL)
                {
                TInt hdrLen(aMsg.iHeader->Length() );

                // Message header length must be valid
                if (hdrLen == ( OstHeader::OstBaseProtocol::KOstBaseHeaderSize ))
                    {
                    msgLength += hdrLen;
                    }
                else
                    {
                    msgLength = -1;
                    }
                }
            else
                {
                // If message length is not enough for valid ISI message, an error is returned
                if (msgLength < ( ISI_HEADER_SIZE + KIsiHeaderRemainder ))
                    {
                    msgLength = -1;
                    }
                }
            }
        else
        {
        OstTrace0( TRACE_IMPORTANT , TRACECOREMESSAGEUTILS_GETMESSAGELENGTH_NOT_SUPPORTED,
        		"TraceCoreMessageUtils::GetMessageLength - Format not supported");
        msgLength = -1;
        }
    OstTrace1( TRACE_FLOW, TRACECOREMESSAGEUTILS_GETMESSAGELENGTH_EXIT,
    		"< TraceCoreMessageUtils::GetMessageLength. Len:%d", msgLength );
    return msgLength;
    }
