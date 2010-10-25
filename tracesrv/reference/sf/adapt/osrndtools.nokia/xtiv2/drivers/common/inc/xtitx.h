/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:
*
*/


/** @file

Class for XTI data transmission.

*///=========================================================================


#ifndef __XtiTx_H__
#define __XtiTx_H__


//- Include Files  ----------------------------------------------------------

#include <e32def.h>

//- Namespace ---------------------------------------------------------------

										 
//- Using -------------------------------------------------------------------


//- Data Types --------------------------------------------------------------


//- Constants ---------------------------------------------------------------

const TUint8 KOsmoMsgSymbianTrace         = 0xD2; // MCU Symbian Trace message ID
const TUint8 KOsmoMsgAsciiPrintf          = 0xC0; // MCU Ascii Printf message ID
const TUint8 KOsmoMsgPhoneFormatted       = 0xC1; // Phone Formatted message ID
const TUint  KXtiChannelSize              = 0x1000;
const TUint8 KOsmoSymbianSystemChannel0   = 224;
const TUint8 KOsmoSymbianSystemChannel1   = 225;
const TUint8 KOsmosReceptionAckChannel    = 251;
const TUint8 KOsmoTraceControlChannel     = 253;
const TUint8 KOsmoTraceBoxConfigChannel   = 226;


//- Macros ------------------------------------------------------------------


//- External Data -----------------------------------------------------------


//- Variables ---------------------------------------------------------------


//- Forward Declarations ----------------------------------------------------


//- Class Definitions -------------------------------------------------------

/**

Interface class 				

*/
class XtiTx 
    {
    public:
        static XtiTx* Instance();
        
        /**
         * Start writing data to Symbian XTI channel. This method must be called before WriteData8, WriteData16 or WriteData32 can be used.
         * @param aChannel XTI channel number. Must be KOsmoSymbianSystemChannel0 or KOsmoSymbianSystemChannel1
         * @param aMessageId XTI message ID. Defines what type of messages are sent. Must be KOsmoMsgSymbianTrace or KOsmoMsgAsciiPrintf.
         * @return Standard Symbian error code
         */
		IMPORT_C static TInt StartWrite( TUint8 aChannel, TUint8 aMessageId );

        /**
         * Write 8 bits data to KOsmoSymbianSystemChannel0 or KOsmoSymbianSystemChannel1
         * XTI channel in KOsmoMsgSymbianTrace or KOsmoMsgAsciiPrintf message format.
         * Before using this method XTI channel and message type must be defined with
         * method XtiTx::StartWrite().
         * @param aChannel XTI channel number
         * @param aData Data to be written
         */
		IMPORT_C static void WriteData8( TUint8 aChannel, TUint8 aData );

        /**
         * Write 16 bits data to KOsmoSymbianSystemChannel0 or KOsmoSymbianSystemChannel1
         * XTI channel in KOsmoMsgSymbianTrace or KOsmoMsgAsciiPrintf message format.
         * Before using this method XTI channel and message type must be defined with
         * method XtiTx::StartWrite().
         * @param aChannel XTI channel number
         * @param aData Data to be written
         */
		IMPORT_C static void WriteData16( TUint8 aChannel, TUint16 aData );

        /**
         * Write 32 bits data to KOsmoSymbianSystemChannel0 or KOsmoSymbianSystemChannel1
         * XTI channel in KOsmoMsgSymbianTrace or KOsmoMsgAsciiPrintf message format.
         * Before using this method XTI channel and message type must be defined with
         * method XtiTx::StartWrite().
         * @param aChannel XTI channel number
         * @param aData Data to be written
         */
		IMPORT_C static void WriteData32( TUint8 aChannel, TUint32 aData );

        /**
         * Stop writing data to Symbian XTI channel which has been started by method StartWrite().
         * @param aChannel XTI channel number
         * @return Standard Symbian error code
         */
        IMPORT_C static TInt StopWrite( TUint8 aChannel );

        /**
         * Sends set bit rate command to Musti
         * @param aBitRate Uart bit rate
         */
		IMPORT_C static void SetBitRate( TUint aBitRate );

        /**
         * Sends acknowledgement for data length message to Musti
         */
        IMPORT_C static void SendDataLenAckMsg();

        /**
         * Sends acknowledgement for data message to Musti
         */
        IMPORT_C static void SendDataRxAckMsg();

        /**
         * Returns the flag that tells if traces are enabled
         * @return EFalse if traces not enabled in RDC, otherwise true
         */
        IMPORT_C static TBool IsTraceEnabled();

    private:
        XtiTx();    
       ~XtiTx();
       static TBool CheckMessageIdForStartWrite( TUint8 aMessageId );
       TBool CheckRDC();

    private:
        static XtiTx* iXtiTxPtr;
        TBool iSymbianChannel0Open;
        TBool iSymbianChannel1Open;
		TBool iTraceEnabled;
    };


//- Global Function Prototypes ----------------------------------------------


//- Inline Functions --------------------------------------------------------


//- Namespace ---------------------------------------------------------------

#endif // __XtiTx_H__

// End of File
