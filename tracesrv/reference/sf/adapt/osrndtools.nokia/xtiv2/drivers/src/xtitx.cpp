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

XtiTx

*///=========================================================================


//- Include Files  ----------------------------------------------------------

#include <kernel/kern_priv.h>
#include <rap.h>
#include "xtitx.h"
//- Namespace ---------------------------------------------------------------

//- Using -------------------------------------------------------------------


//- External Data -----------------------------------------------------------


//- External Function Prototypes --------------------------------------------


//- Constants ---------------------------------------------------------------

// Definitions for XTI

const TUint8  KHwOsmoMsgAsciiPrintfLE       = 0xC3;
const TUint32 KHwOsmoChannelSize            = 0x40L;

const TInt    KBitRateBufferSize            = 20; 
const TUint8  KDataLengthAck                = 0x01;
const TUint8  KDataMsgAck                   = 0x02;

const TUint32 KHwOsmoControlOutputAddress   = KRapRegXTIOSMO + ( KOsmoTraceControlChannel * KXtiChannelSize );

const TUint32 KHwOsmoSymbianChannel0Address = KRapRegXTIOSMO + ( KOsmoSymbianSystemChannel0 * KXtiChannelSize );

const TUint32 KHwOsmoSymbianChannel1Address = KRapRegXTIOSMO + ( KOsmoSymbianSystemChannel1 * KXtiChannelSize );

const TUint32 KHwOsmoTraceBoxConfigAddress  = KRapRegXTIOSMO + ( KOsmoTraceBoxConfigChannel * KHwOsmoChannelSize );

const TUint32 KHwOsmoReceptionAckAddress    = KRapRegXTIOSMO + ( KOsmosReceptionAckChannel * KHwOsmoChannelSize );


//- Macros ------------------------------------------------------------------


//- Global and Local Variables ----------------------------------------------

XtiTx* XtiTx::iXtiTxPtr = NULL;

//- Local Function Prototypes -----------------------------------------------


//===========================================================================

//- Local Functions ---------------------------------------------------------

//===========================================================================

//- Member Functions --------------------------------------------------------

/*
-----------------------------------------------------------------------------

    XtiTx

    XtiTx
    
    Constructor
-----------------------------------------------------------------------------
*/

XtiTx::XtiTx()
    : iSymbianChannel0Open( EFalse )
     ,iSymbianChannel1Open( EFalse )
     ,iTraceEnabled( EFalse )
    {
    }


/*
-----------------------------------------------------------------------------
    XtiTx

    ~XtiTx
    
    Destructor
-----------------------------------------------------------------------------
*/
XtiTx::~XtiTx()
    {
    }

/*
----------------------------------------------------------------------------

    XtiTx

    Instance

    Return an instance to XtiTx interface. This method returns the
    only XtiTx instance system can have, it is not possible to create
    new instances of this class.

    Return Values:          iXtiTxPtr* An instance to XtiTx class

-----------------------------------------------------------------------------
*/
XtiTx* XtiTx::Instance( )
    {
    // Allow only 1 instance of XtiTx to exists at any time
    if ( iXtiTxPtr == NULL )
        {
        iXtiTxPtr = new XtiTx();
        if( iXtiTxPtr != NULL )
            {
            if( iXtiTxPtr->CheckRDC() )
                {
                iXtiTxPtr->iTraceEnabled = ETrue;
                }
            else
                {
                iXtiTxPtr->iTraceEnabled = EFalse;
                }
            }                                
        }
    return iXtiTxPtr;
    }



/*
----------------------------------------------------------------------------

    XtiTx

    StartWrite

    Start writing data to Symbian XTI channel. This method must be called before WriteData8, WriteData16 or 
    WriteData32 can be used.                 
    
    @param   aChannel     XTI channel number. Must be KOsmoSymbianSystemChannel0 or KOsmoSymbianSystemChannel1 
    
    @param   aMessageId   XTI message ID. Defines what type of messages are sent.
                          Must be KOsmoMsgSymbianTrace or KOsmoMsgAsciiPrintf.

    Return Values:        KErrNone, if succesfull. Otherwise KErrGeneral

-----------------------------------------------------------------------------
*/
EXPORT_C TInt XtiTx::StartWrite( TUint8 aChannel, TUint8 aMessageId )
    {
    TInt ret( KErrNone );
    // Message type must be MCU Symbian trace message or MCU ASCII printf message
    if( CheckMessageIdForStartWrite(aMessageId) && iXtiTxPtr->iTraceEnabled )
        {
         // OSMO channel must be 224 or 225
        if( aChannel == KOsmoSymbianSystemChannel0 )
            {
            // Check that the channel is free
            if( iXtiTxPtr->iSymbianChannel0Open == EFalse )
                {
                // Open the channel
                TRap::SetRegister8( aMessageId, KHwOsmoSymbianChannel0Address );
                iXtiTxPtr->iSymbianChannel0Open = ETrue;
                }
            else
                {
                ret = KErrGeneral;
                }
            }
        else if( aChannel == KOsmoSymbianSystemChannel1 )
            {
            // Check that the channel is free
            if( iXtiTxPtr->iSymbianChannel1Open == EFalse )
                {
                // Open the channel
                TRap::SetRegister8( aMessageId, KHwOsmoSymbianChannel1Address );
                iXtiTxPtr->iSymbianChannel1Open = ETrue;
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
        }
    else
        {
        ret = KErrGeneral;
        }
    return ret;
    }

/*
----------------------------------------------------------------------------

    XtiTx

    StopWrite

    Stop writing data to Symbian XTI channel which has been started by method StartWrite().
    
    Return Values:        KErrNone, if succesfull. Otherwise KErrGeneral                 
    
-----------------------------------------------------------------------------
*/
EXPORT_C TInt XtiTx::StopWrite( TUint8 aChannel )
    {
    TInt ret( KErrNone );
    
    // OSMO channel must be 224 or 225
    if( aChannel == KOsmoSymbianSystemChannel0 )
       {
       // Check that the channel is opened
        if( iXtiTxPtr->iSymbianChannel0Open )
            {
            // close the channel
            TRap::SetRegister8( KOsmoSymbianSystemChannel0, KHwOsmoControlOutputAddress );
            iXtiTxPtr->iSymbianChannel0Open = EFalse;
            }
        else
            {
            ret = KErrGeneral;
            }            
        }
    else if( aChannel == KOsmoSymbianSystemChannel1 )
        {
        // Check that the channel is opened
        if( iXtiTxPtr->iSymbianChannel1Open )
            {
            // close the channel
            TRap::SetRegister8( KOsmoSymbianSystemChannel1, KHwOsmoControlOutputAddress );
            iXtiTxPtr->iSymbianChannel1Open = EFalse;
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
    return ret;
    }

/*
----------------------------------------------------------------------------

    XtiTx

    WriteData8

    Write 8 bits data to KOsmoSymbianSystemChannel0 or KOsmoSymbianSystemChannel1
    XTI channel in KOsmoMsgSymbianTrace or KOsmoMsgAsciiPrintf message format.
    Before using this method XTI channel and message type must be defined with
    method XtiTx::StartWrite().
    
    @param   aData     Data to be written
    
-----------------------------------------------------------------------------
*/
EXPORT_C void XtiTx::WriteData8( TUint8 aChannel, TUint8 aData )
    {
    // OSMO channel must be 224 or 225
    if( aChannel == KOsmoSymbianSystemChannel0 )
        {
        // Check that the channel is opened
        if( iXtiTxPtr->iSymbianChannel0Open )
            {
            // Send data
            TRap::SetRegister8( aData, KHwOsmoSymbianChannel0Address );
            }
        }
    else if( aChannel == KOsmoSymbianSystemChannel1 )
        {
        // Check that the channel is opened
        if( iXtiTxPtr->iSymbianChannel1Open )
            {
            // Send data
            TRap::SetRegister8( aData, KHwOsmoSymbianChannel1Address );
            }
        }
    }


/*
----------------------------------------------------------------------------

    XtiTx

    WriteData16

    Write 16 bits data to KOsmoSymbianSystemChannel0 or KOsmoSymbianSystemChannel1
    XTI channel in KOsmoMsgSymbianTrace or KOsmoMsgAsciiPrintf message format.
    Before using this method XTI channel and message type must be defined with
    method XtiTx::StartWrite().
    
    @param   aData     Data to be written

-----------------------------------------------------------------------------
*/
EXPORT_C void XtiTx::WriteData16( TUint8 aChannel, TUint16 aData )
    {
    // OSMO channel must be 224 or 225
    if( aChannel == KOsmoSymbianSystemChannel0 )
        {
        // Check that the channel is opened
        if( iXtiTxPtr->iSymbianChannel0Open )
            {
            // Send data
            TRap::SetRegister16( aData, KHwOsmoSymbianChannel0Address );
            }
        }
    else if( aChannel == KOsmoSymbianSystemChannel1 )
        {
        // Check that the channel is opened
        if( iXtiTxPtr->iSymbianChannel1Open )
            {
            // Send data
            TRap::SetRegister16( aData, KHwOsmoSymbianChannel1Address );
            }
        }
    }


/*
----------------------------------------------------------------------------

    XtiTx

    WriteData32

    Write 32 bits data to KOsmoSymbianSystemChannel0 or KOsmoSymbianSystemChannel1
    XTI channel in KOsmoMsgSymbianTrace or KOsmoMsgAsciiPrintf message format.
    Before using this method XTI channel and message type must be defined with
    method XtiTx::StartWrite().
    
    @param   aData     Data to be written

-----------------------------------------------------------------------------
*/
EXPORT_C void XtiTx::WriteData32( TUint8 aChannel, TUint32 aData )
    {
    // OSMO channel must be 224 or 225
    if( aChannel == KOsmoSymbianSystemChannel0 )
        {
        // Check that the channel is opened
        if( iXtiTxPtr->iSymbianChannel0Open )
            {
            // Send data
            TRap::SetRegister32( aData, KHwOsmoSymbianChannel0Address );
            }
        }
    else if( aChannel == KOsmoSymbianSystemChannel1 )
        {
        // Check that the channel is opened
        if( iXtiTxPtr->iSymbianChannel1Open )
            {
            // Send data
            TRap::SetRegister32( aData, KHwOsmoSymbianChannel1Address );
            }
        }
    }    

/*
----------------------------------------------------------------------------

    XtiTx

    SetBitRate

    Sends set bit rate command to Musti

    @param   aBitRate     Uart bit rate

-----------------------------------------------------------------------------
*/
EXPORT_C void XtiTx::SetBitRate( TUint aBitRate )
    {
    if( iXtiTxPtr->iTraceEnabled )
        {
        TBuf8<KBitRateBufferSize> string;

        string.Append(_L("!B"));
	    string.AppendNum( aBitRate, EDecimal );
    
        // Send Symbian trace activation message
        TRap::SetRegister8( KHwOsmoMsgAsciiPrintfLE, KHwOsmoTraceBoxConfigAddress );

        // Send data
        for (TInt i = 0; i < string.Length(); i++ )
            {
            TRap::SetRegister8( string[i], KHwOsmoTraceBoxConfigAddress );
            }

        // Complete message
        TRap::SetRegister8( KOsmoTraceBoxConfigChannel, KHwOsmoControlOutputAddress );
        }
    }

/*
----------------------------------------------------------------------------

    XtiTx

    SendDataLenAckMsg
    
    Sends acknowledgement for data length message to Musti
   
-----------------------------------------------------------------------------
*/                                  
EXPORT_C void XtiTx::SendDataLenAckMsg( )
    {
    if( iXtiTxPtr->iTraceEnabled )
        {        
        // Send Symbian trace activation message
        TRap::SetRegister8( KHwOsmoMsgAsciiPrintfLE, KHwOsmoReceptionAckAddress );
        TRap::SetRegister8( KDataLengthAck, KHwOsmoReceptionAckAddress );
        // Complete message
        TRap::SetRegister8( KOsmosReceptionAckChannel, KHwOsmoControlOutputAddress );
        }
    }

/*
----------------------------------------------------------------------------

    XtiTx

    SendDataLenAckMsg
   
    Sends acknowledgement for data message to Musti

-----------------------------------------------------------------------------
*/                                  
EXPORT_C void XtiTx::SendDataRxAckMsg( )
    {
    if( iXtiTxPtr->iTraceEnabled )
        {
        // Send Symbian trace activation message
        TRap::SetRegister8( KHwOsmoMsgAsciiPrintfLE, KHwOsmoReceptionAckAddress );
        TRap::SetRegister8( KDataMsgAck, KHwOsmoReceptionAckAddress );
        // Complete message
        TRap::SetRegister8( KOsmosReceptionAckChannel, KHwOsmoControlOutputAddress );
        }
    }

/*
----------------------------------------------------------------------------

    XtiTx

    IsTraceEnabled

    Returns the flag that tells if traces are enabled
    
    @Return EFalse if traces not enabled in RDC, otherwise true
    
-----------------------------------------------------------------------------
*/


EXPORT_C TBool XtiTx::IsTraceEnabled()
    {
    return iXtiTxPtr != NULL && iXtiTxPtr->iTraceEnabled;
    }


/*
----------------------------------------------------------------------------

    XtiTx

    CheckMessageIdForStartWrite

    Checks message ID is valid for writing
    
    @param   aMessageId   XTI message ID. Defines what type of messages are sent.
                          Must be KOsmoMsgSymbianTrace, KOsmoMsgAsciiPrintf, or KOsmoMsgPhoneFormatted.

    @Return EFalse if traces not enabled in RDC, otherwise true
    
-----------------------------------------------------------------------------
*/


TBool XtiTx::CheckMessageIdForStartWrite( TUint8 aMessageId )
    {
    // Message type must be MCU Symbian trace message or MCU ASCII printf message or Phone Formatted
    if( aMessageId == KOsmoMsgSymbianTrace || aMessageId == KOsmoMsgAsciiPrintf || aMessageId == KOsmoMsgPhoneFormatted )
        {
        return ETrue;
        }
    return EFalse;
    }


/*
----------------------------------------------------------------------------

    XtiTx

    CheckRDC

    Checks R&D Certificate status bit in test ram
    
    @Return EFalse if traces not enabled in RDC, otherwise true
    
-----------------------------------------------------------------------------
*/


TBool XtiTx::CheckRDC()
	{
    return ETrue;
	}


//===========================================================================

//- Global Functions --------------------------------------------------------

//---------------------------------------------------------------------------
/**
    The entry point for a standard extension. Creates XtiTx extension.

    @return KErrNone, if successful
*/
DECLARE_STANDARD_EXTENSION()    
    {
    TInt ret( KErrNoMemory );

    if ( XtiTx::Instance() != NULL )
        {
        ret = KErrNone;
        }

    return ret;
    }                 

//- Namespace ---------------------------------------------------------------

// End of File
