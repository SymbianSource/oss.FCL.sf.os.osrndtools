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
// Trace Core OST header
// 

#ifndef __TRACECOREOSTHEADER_H__
#define __TRACECOREOSTHEADER_H__

namespace OstHeader
    {

    namespace OstBaseProtocol
        {
        /**
         * OST header size
         */
        const TInt KOstBaseHeaderSize = 4;

        /**
         * OST header Version offset
         */
        const TInt KOstHeaderVersionOffset = 0;

        /**
         * OST header version size
         */
        const TInt KOstHeaderVersionSize = 1;

        /**
         * OST header ProtocolId offset
         */
        const TInt KOstHeaderProtocolIdOffset = 1;

        /**
         * OST header ProtocolId size
         */
        const TInt KOstHeaderProtocolIdSize = 1;

        /**
         * OST header Length offset
         */
        const TInt KOstHeaderLengthOffset = 2;

        /**
         * OST header Length size
         */
        const TInt KOstHeaderLengthSize = 2;
        }

    namespace OstTraceActivation
        {
        /**
         * OST header TransactionId offset
         */
        const TInt KOstHeaderTransactionIdOffset = 4;

        /**
         * OST header TransactionId size
         */
        const TInt KOstHeaderTransactionIdSize = 1;

        /**
         * OST header MessageId offset
         */
        const TInt KOstHeaderMessageIdOffset = 5;

        /**
         * OST header Length size
         */
        const TInt KOstHeaderMessageIdSize = 1;
        
        /**
         * OST header Result offset
         */
        const TInt KOstHeaderResultOffset = 6;

        /**
         * OST header Result size
         */
        const TInt KOstHeaderResultSize = 1;
        
        
        namespace SetApplicationStatusRequest
            {
            /**
             * OST header ApplicationId offset
             */
            const TInt KApplicationIdOffset = 6;

            /**
             * OST header ApplicationId size
             */
            const TInt KApplicationIdSize = 4;

            /**
             * OST header Activation Status offset
             */
            const TInt KActivationStatusOffset = 10;

            /**
             * OST header Activation Status size
             */
            const TInt KActivationStatusSize = 1;
            
            /**
             * OST header Filler offset
             */
            const TInt KFillerOffset = 11;

            /**
             * OST header Filler size
             */
            const TInt KFillerSize = 1;

            /**
             * OST header Filler offset
             */
            const TInt KGroupIdOffset = 12;

            /**
             * OST header Filler size
             */
            const TInt KGroupIdSize = 2;
            
            /*
             readMessage[0]=0x01;  // Version 0.1
             readMessage[1]=0x01;  // Protocol ID OST Trace Activation Protocol
             readMessage[2]=0x00;  // Length high
             readMessage[3]=0x0e;  // Length low
             readMessage[4]=0x00;  // Transaction ID
             readMessage[5]=0x06;  // SetApplicationStatusRequest
             readMessage[6]=0x12;  // Application ID high (MSByte)
             readMessage[7]=0x34;  // Application ID
             readMessage[8]=0x56;  // Application ID
             readMessage[9]=0x78;  // Application ID low (LSByte)
             readMessage[10]=0x00; // Activation Status (0x00 deactivate, 0x01 activate)
             readMessage[11]=0x00; // Filler
             readMessage[12]=0x00; // Group ID 1 high
             readMessage[13]=0x03; // Group ID 1 low
             readMessage[14]=0x00; // Group ID 2 high
             readMessage[15]=0x40; // Group ID 2 low
             readMessage[16]=0x00; // Group ID 3 high
             readMessage[17]=0x41; // Group ID 3 low
             */
            }
        }

    namespace OstAsciiTraceProtocol
        {
        /**
         * OST TraceProtocol Timestamp
         */
        const TInt KTimeStampOffset = 4;

        /**
         * OST TraceProtocol Timestamp size
         */
        const TInt KTimestampSize = 8;
        
        /**
         * OST TraceProtocol Timestamp
         */
        const TInt KAsciiTraceDataOffset = 12;
        }
    
    namespace OstSimpleApplicationTraceProtocol
        {
        /**
         * OST TraceProtocol Timestamp
         */
        const TInt KTimeStampOffset = 4;

        /**
         * OST TraceProtocol Timestamp size
         */
        const TInt KTimestampSize = 8;

        /**
         * OST TraceProtocol Timestamp
         */
        const TInt KApplicationIdOffset = 12;

        /**
         * OST TraceProtocol Timestamp size
         */
        const TInt KApplicationIdSize = 4;

        /**
         * OST TraceProtocol Timestamp
         */
        const TInt KTraceGroupIdOffset = 16;

        /**
         * OST TraceProtocol Timestamp size
         */
        const TInt KTraceGroupIdSize = 2; 

        /**
         * OST TraceProtocol Timestamp
         */
        const TInt KTraceIdOffset = 18;

        /**
         * OST TraceProtocol Timestamp size
         */
        const TInt KTraceIdSize = 2; 
        
        /**
         * OST TraceProtocol Timestamp
         */
        const TInt KBinaryTraceDataOffset = 20;

        }

    namespace OstTraceCoreProtocol
        {
        /**
         * OST TraceCore Protocol Header Size
         */
        const TInt KHeaderSize = 4;
           
        /**
         * OST TraceCore Protocol Protocol ID offset in header
         */
        const TInt KProtocolIdOffset = 0;
        
        /**
         * OST TraceCore Subscriber message ID offset in header if TraceCoreProtocol is KSubscriberProtocol
         */
        const TInt KSubscriberIdOffset = 1;
        
        /**
         * OST TraceCore Protocol Subscriber Protocol ID
         */
        const TInt KSubscriberProtocol = 0x01;
        }
    }

namespace OstConstants
    {
    
    /**
     * OST KOstHeaderVersion 0.1
     */
    const TInt KOstHeaderVersion                        = 0x01;
    
    namespace OstBaseProtocol
        {
        /**
         * OST KOstSystemProtocol
         */
        const TInt KOstSystemProtocol                   = 0x00;

        /**
         * OST KOstTraceActivationProtocol
         */
        const TInt KOstTraceActivationProtocol          = 0x01;

        /**
         * OST KOstAsciiTraceProtocol
         */
        const TInt KOstAsciiTraceProtocol               = 0x02;

        /**
         * OST KOstSimpleApplicationTraceProtocol
         */
        const TInt KOstSimpleApplicationTraceProtocol   = 0x03;

        /**
         * OST KOstTrkDebuggerProtocol
         */
        const TInt KOstTrkDebuggerProtocol              = 0x90;

        /**
         * OST KOstTraceCoreProtocol
         */
        const TInt KOstTraceCoreProtocol                = 0x91;
        
        /**
         * OST KOstTerminalKeyboardProtocol
         */
        const TInt KOstTerminalKeyboardProtocol         = 0x92;

        
        }

    namespace OstTraceActivation
        {
        /**
         * OST header TransactionId offset
         */
        const TInt KGetAsciiStatusRequest               = 0x00;

        /**
         * OST header TransactionId offset
         */
        const TInt KGetAsciiStatusResponse              = 0x01;
        
        /**
         * OST header TransactionId offset
         */
        const TInt KSetAsciiStatusRequest               = 0x02;

        /**
         * OST header TransactionId offset
         */
        const TInt KSetAsciiStatusResponse              = 0x03;
        
        /**
         * OST header TransactionId offset
         */
        const TInt KGetApplicationStatusRequest         = 0x04;
                
        /**
         * OST header TransactionId offset
         */
        const TInt KGetApplicationStatusResponse        = 0x05;

        /**
         * OST header TransactionId offset
         */
        const TInt KSetApplicationStatusRequest         = 0x06;
        
        /**
         * OST header TransactionId offset
         */
        const TInt KSetApplicationStatusResponse        = 0x07;

        /**
         * OST header TransactionId offset
         */
        const TInt KMessageSyntaxError                  = 0xFE;

        /**
         * OST header TransactionId offset
         */
        const TInt KUnknownMessageError                 = 0xFF;


        namespace ActivationStatus
            {

            /**
             * OST header Filler offset
             */
            const TInt KApplicationTracesDeactivated    = 0x00;

            /**
             * OST header Filler size
             */
            const TInt KApplicationTracesActivated      = 0x01;
            

            }
        }
    }

#endif

// End of File // __TRACECOREOSTHEADER_H__
