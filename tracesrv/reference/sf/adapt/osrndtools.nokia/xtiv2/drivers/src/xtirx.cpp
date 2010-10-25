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

XtiRx driver receives the data from Musti and sends it to Trace Core.

*///=========================================================================
                                      

//- Include Files  ----------------------------------------------------------

#include <rap.h>
#include <kernel/kernel.h>
#include <rap_priv.h>
#include <PowerResourceManager.h>
#include "xtirx.h"
#include "xtitx.h"
#include "XtiDebug.h"

//- Namespace ---------------------------------------------------------------


//- Using -------------------------------------------------------------------


//- External Data -----------------------------------------------------------


//- External Function Prototypes --------------------------------------------


//- Constants ---------------------------------------------------------------

const TInt   KBitsInByte                    = 8;  // the amount of bits in byte
const TInt   KBitsInDataLengthMsg           = 32; // the amount of bits in data length message
const TInt   KMaxWakeupResponceCounterValue = 20; // the maximum number of attempts to writing to TraceCore buffer
const TInt   KWakeupResponceTimerInterval   = 100000; // time interval of attempts to writing to TraceCore buffer (microseconds)
const TUint  KBitRate                       = 115200; // bit rate
const TUint8 KDefaultDfcPriority            = 3;      // Default DFC queue priority
const TInt   KInterruptId                   = TRap::EIntIdASR001_MUSTI_BUFFERFULL; // Uart interrupt line
const TInt   KPollPeriodMs                  = 2;      // time interval of checking if the Uart RX buffer has data (milliseconds)
const TInt   KIdleTimerInterval             = 500000; // time interval for idle timer (microseconds)
const TInt   KResetTimerInterval            = 800000; // time interval for idle timer (microseconds)
const TInt   KMaxPoll                       = 25;    // the maximum number of checking if the Uart RX buffer has data 
const TUint  KMaxXtiMessageLength           = 8204;    // Maximum size of XTI message send by Musti
const TUint  KClearPoll                     = 3; // Used when clearing UART buffer


/** The name of the created XTI RX thread */
_LIT( KThreadName, "XtiRxThread" );

/** DFC thread priority */
const TInt KDfcThreadPriority = 26;


//- Macros ------------------------------------------------------------------

//- Global and Local Variables ----------------------------------------------

XtiRx* XtiRx::iXtiRxPtr = NULL;

TBool HasUartRxFifoData( TAny* aPtr );

_LIT(KXTIRXDriver, "XTI RX Driver");

//- Local Function Prototypes -----------------------------------------------

//===========================================================================

//- Local Functions ---------------------------------------------------------

//===========================================================================

//- Member Functions --------------------------------------------------------
void XtiRx::GPIOwakeIsr(TAny* aPtr)
    {
    XTI_TRACE( Kern::Printf("XtiRx::GPIOwakeIsr") )
    XTI_TRACE( Kern::Printf("XtiRx::GPIOwakeIsr <<") )
    }
    
/*
-----------------------------------------------------------------------------

    XtiRx

    XtiRx
                  
    Constructor
-----------------------------------------------------------------------------
*/

XtiRx::XtiRx()
    : iDfcQueue( NULL )
    , iReceiveDataDfc( ReceiveDataDfc, this,  KDefaultDfcPriority )
    , iGPIOWakeupDfc( GPIOWakeupDfc, this, KDefaultDfcPriority )
    , iSetPowerDfc( SetPowerDfc, this, KDefaultDfcPriority )
    , iClearPowerDfc( ClearPowerDfc, this, KDefaultDfcPriority )
    , iClocksOn( EFalse )
    , iTraceCoreDfcPtr( NULL )
    , iRxBufferPtr( NULL )
    , iRxState( KRxStateIdle )
    , iReceiveBufferFree( EFalse )
    , iMessagelength( 0 )
    , iWakeupResponseCounter( 0 )
    , iClientId( 0 )
    {
    
    // Register the PRM client
    TInt r = TPowerResourceManager::RegisterClient( iClientId, KXTIRXDriver );
    __ASSERT_DEBUG(r==KErrNone,Kern::Fault("Registration with the power manager failed",r));
    }


/*
-----------------------------------------------------------------------------
    XtiRx

    ~XtiRx
    
    Destructor
-----------------------------------------------------------------------------
*/
XtiRx::~XtiRx()
    {
    iRxBufferPtr = NULL;
    iTraceCoreDfcPtr = NULL;

    // Deregister the PRM client
    TPowerResourceManager::DeRegisterClient( iClientId );    
    iDfcQueue = NULL;
    }

/*
----------------------------------------------------------------------------

    XtiRx

    Instance

    Return an instance to XtiRx interface. This method returns the
    only XtiRx instance system can have, it is not possible to create
    new instances of this class.

    Return Values:          iXtiRxPtr* An instance to XtiRx class

-----------------------------------------------------------------------------
*/
XtiRx* XtiRx::Instance( )
    {
    XTI_TRACE( Kern::Printf("XtiRx::Instance") )
    // Allow only 1 instance of XtiRx to exists at any time
    if ( iXtiRxPtr == NULL )
        {
        iXtiRxPtr = new XtiRx();
        }
    return iXtiRxPtr;
    }

/*
----------------------------------------------------------------------------

    XtiRx

    Init

    Bind Uart interrupt, configure Uart HW and start receiving the data 

    Return Values: KErrNone or error code
    
-----------------------------------------------------------------------------
*/
TInt XtiRx::Init()      
    {
    XTI_TRACE( Kern::Printf("XtiRx::Init >>") ) 
    TInt ret( KErrNone );
    SetPowerRequirements();

    XTI_TRACE( Kern::Printf("XtiRx::Init - Bind method Isr to uart interrupt 0x%x", KInterruptId) ) 
    ret = Interrupt::Bind( KInterruptId, Isr, this );
       
    if( ret == KErrNone ) 
        {                        
        ret = Configure();
        if( ret == KErrNone )
            {                           
            ret = Start();
            }         
        }
    XTI_TRACE( Kern::Printf("XtiRx::Init <<, ret= %d", ret))
    return ret;
    }

/*
----------------------------------------------------------------------------

    XtiRx

    ConfigureUart

    Configure Uart HW.
    
    Return Value: KErrNone or error code

-----------------------------------------------------------------------------
*/
TInt XtiRx::ConfigureUart() const
{
    XTI_TRACE( Kern::Printf("XtiRx::ConfigureUart >>") )
  
    TInt ret( KErrNone );
  
    XTI_TRACE( Kern::Printf("XtiRx::ConfigureUart - Disable uart interrupt 0x%x", KInterruptId) )
    ret = Interrupt::Disable( KInterruptId );
    __ASSERT_DEBUG(ret == KErrNone, Kern::Fault( ("XtiRx::ConfigureUart"), __LINE__ ));
  
    TUint32 clock_in_hz = TRap::McuppClockInHz();
  
    TUint32 baud_rate = 2304000;  // Use Musti default bit rate

  
    // Integer part of bitduration value for Rx (the bit window)
  
    TUint32 mcuClock;
    mcuClock = clock_in_hz / baud_rate;
    TUint32 rx_bitduration = (TUint32)((mcuClock - 3) / 2);
    TUint32 rx_modulo = rx_bitduration / 5;  // Use the recommended 20%
  
  
    // Fractional part: The formula is rewritten like this in order to
    // be able to use integer math:
    //
    // rx_bitduration = clock_in_hz / (2 * baud_rate) - 3/2
    //
    // Thus the fractional part of rx_bitduration can be calculated as
    //
    //      f = f1 - f2
    //
    // where
    //
    //      f1 = fractional_part_of( clock_in_hz / (2 * baud_rate) )
    //
    //      f2 = fractional_part_of( 3/2 )
    //
    // The two fractional parts are calculated as follows:
    //
    // f1 = ( clock_in_hz % (2 * baud_rate) ) * ( 2^32 / 2 * baud_rate )
    //    = ( clock_in_hz % (2 * baud_rate) ) * ( 2^31 / baud_rate )
    //
    //
    // f2 = ( 3 % 2 ) * ( 2^32 / 2 )
    //    =     1     *   2^31
    //
    TUint32 rx_fraction = (clock_in_hz % (baud_rate << 1)) * ( (TUint32)(1<<31)/baud_rate ) - (TUint32)(1<<31);
  
    XTI_TRACE( Kern::Printf( "XtiRx Rx parameters:" ));
    XTI_TRACE( Kern::Printf( "Baud             %u", baud_rate ) );
    XTI_TRACE( Kern::Printf( "Mcupp clock freq %u", clock_in_hz ) );
    XTI_TRACE( Kern::Printf( "Write registers:" ));
    XTI_TRACE( Kern::Printf( "RX-bitduration   %u", rx_bitduration) );
    XTI_TRACE( Kern::Printf( "RX-pulseduration %u", rx_bitduration) );
    XTI_TRACE( Kern::Printf( "RX-fraction      %u", rx_fraction) );
    XTI_TRACE( Kern::Printf( "RX-modulo        %u", rx_modulo) );
  
    // Configure static Rx parameters
    // Setting up ASR001
    TRap::SetRegister32(ASR001_T_V_MODE_RUN, KRapRegXTIASR001_MODE);
    TRap::SetRegister32(rx_modulo, KRapRegXTIASR001_MODULO);
    TRap::SetRegister32(ASR001_T_V_POLARITY_POSITIVE, KRapRegXTIASR001_POLARITY);
    TRap::SetRegister32(ASR001_T_V_FLOW_OFF, KRapRegXTIASR001_FLOW);
    TRap::SetRegister32(ASR001_T_V_RXSTATE_IDLE, KRapRegXTIASR001_RXSTATE);
    TRap::SetRegister32(rx_bitduration, KRapRegXTIASR001_BITDURATION); /* Bitrate = 2304000 b/s */
    TRap::SetRegister32(rx_bitduration, KRapRegXTIASR001_PULSEDURATION);
    TRap::SetRegister32(rx_fraction, KRapRegXTIASR001_FRACTION);
    // Note that only the upper bits of the written fraction will have effect
    // e.g. 0x40000000
    //TRap::SetRegister32(0x42AAAAAA, KRapRegXTIASR001_FRACTION);
    TRap::SetRegister32(0x02, KRapRegXTIASR001_THRESHOLD);
    TRap::SetRegister32(0x07, KRapRegXTIASR001_DATABITS); /* 8 Data bits */
    TRap::SetRegister32(0x00, KRapRegXTIASR001_STOPBITS); /* 1 Stop bits */
    TRap::SetRegister32(ASR001_T_V_PARITY_NONE, KRapRegXTIASR001_PARITY); /* No parity */
    TRap::SetRegister32(0x00, KRapRegXTIASR001_WATERMARK);
  
  
    XTI_TRACE( Kern::Printf("XtiRx::ConfigureUart <<") )
    return ret;
}

/*
----------------------------------------------------------------------------

    XtiRx

    Configure

    Configure HW.
    
    Return Value: KErrNone or error code

-----------------------------------------------------------------------------
*/
TInt XtiRx::Configure() const
    {
    TInt ret( KErrNone );

    XTI_TRACE( Kern::Printf("XtiRx::Configure >>") )

    // Configure UART
    ret = ConfigureUart();
    __ASSERT_DEBUG(ret == KErrNone, Kern::Fault( ("XtiRx::Configure"), __LINE__ ));

    XTI_TRACE( Kern::Printf("XtiRx::Configure <<") )
    return ret;

    }


/*
----------------------------------------------------------------------------

    XtiRx

    Isr

    Uart interrupt service routine
    
    @param   aPtr  Pointer to XtiRx object

-----------------------------------------------------------------------------
*/
void XtiRx::Isr(TAny* aPtr)
    {
    XTI_TRACE( Kern::Printf("XtiRx::Isr >>") )
    __ASSERT_DEBUG(aPtr != NULL, Kern::Fault( ("XtiRx::Isr - Null pointer!"), 0 ));

    XTI_TRACE( Kern::Printf("XtiRx::Isr - Disable uart interrupt 0x%x", KInterruptId) )  
    TInt ret = Interrupt::Disable( KInterruptId );
    __ASSERT_DEBUG(ret == KErrNone, Kern::Fault( ("XtiRx::Isr"), __LINE__ ));
    
    XtiRx& self = *static_cast<XtiRx*>( aPtr );
    XTI_TRACE( Kern::Printf("XtiRx::Isr - Add ReceiveDataDfc") )     
    self.iReceiveDataDfc.Add();
    
    XTI_TRACE( Kern::Printf("XtiRx::Isr <<") )
    }


/*
----------------------------------------------------------------------------

    XtiRx

    ReceiveDataDfc

    Function called in DFC context after Uart interrupt. Reads the data
    from Uart RX FIFO and saves the data to the buffer which is defined
    XtiRx::Register function. 

    @param   aPtr  Pointer to XtiRx object    

-----------------------------------------------------------------------------
*/
void XtiRx::ReceiveDataDfc( TAny* aPtr )
    {
    XTI_TRACE( Kern::Printf("XtiRx::ReceiveDataDfc >>") )
    __ASSERT_DEBUG(aPtr != NULL, Kern::Fault( ("XtiRx::ReceiveDataDfc - Null pointer!"), 0 ));

    XtiRx& self = *static_cast< XtiRx* >( aPtr );
    TInt ret( KErrNone );

    XTI_TRACE( Kern::Printf("XtiRx::ReceiveDataDfc - Cancel timer") )
    self.iIdleTimer.Cancel();    
    // Start timer safety timer to clear state machine if problems
    XTI_TRACE( Kern::Printf("XtiRx::ReceiveDataDfc - Start timer safety timer to clear state machine if problems") )
    self.iIdleTimer.OneShot( KResetTimerInterval, XtiRx::ResetReceiveState, aPtr );
    
    self.SetPowerRequirements(); 
    if( self.iRxState == KRxStateIdle )
        {
        XTI_TRACE( Kern::Printf("XtiRx::ReceiveDataDfc - XTI state = KRxStateIdle") )
        TInt count(0);
        // Clear Uart RX buffer
        XTI_TRACE( Kern::Printf("XtiRx::ReceiveDataDfc - Clear Uart RX buffer") ) 
 
        while( TRap::Register32(KRapRegXTIASR001_GAUGE) != 0 || count < KClearPoll )
            {
            TInt r = Kern::PollingWait(HasUartRxFifoData, (TAny*)&self , KPollPeriodMs, KMaxPoll);
            if( r == KErrNone  )
                {
                TRap::Register32(KRapRegXTIASR001_DATA);
                }
            count++;
            }

        // check if Trace Core has read the previous data package
        if( self.iReceiveBufferFree )
            {
            if( ret == KErrNone ) 
                {
                // Send bit rate to Musti
                XTI_TRACE( Kern::Printf("XtiRx::ReceiveDataDfc - Send bit rate to Musti") ) 
                XtiTx::SetBitRate( KBitRate );
                // Waiting for the data length message
                XTI_TRACE( Kern::Printf("XtiRx::ReceiveDataDfc - Waiting for the data length message") ) 
                self.iRxState = KRxStateWaitDataLenMsg;
                }
            }
        else
            {
            XTI_TRACE( Kern::Printf("XtiRx::ReceiveDataDfc - Buffer not free -> start timer") )
            self.iWakeupResponseTimer.OneShot( KWakeupResponceTimerInterval, XtiRx::ReceiveDataDfc, aPtr );
            self.iWakeupResponseCounter++;
            if( self.iWakeupResponseCounter == KMaxWakeupResponceCounterValue )
                {
                self.iWakeupResponseTimer.Cancel();
                ret = KErrGeneral;
                } 
            }
        XTI_TRACE( Kern::Printf("XtiRx::ReceiveDataDfc - Enable uart interrupt 0x%x", KInterruptId) )    
        Interrupt::Enable( KInterruptId );
        }
    else if( self.iRxState == KRxStateWaitDataLenMsg ) 
        {
        XTI_TRACE( Kern::Printf("XtiRx::ReceiveDataDfc - XTI state = KRxStateWaitDataLenMsg") )
        self.iMessagelength = 0;
        // Read XTI message length. The number of bytes is sent as a 32-bit binary little-endian integer.
        TInt r( KErrNone );
        for( TInt i = 0 ; (i < KBitsInDataLengthMsg) ; i += KBitsInByte )
            {
            //TUint32 len = 0;
            r = Kern::PollingWait(HasUartRxFifoData, (TAny*)&self , KPollPeriodMs, KMaxPoll);
            if( r == KErrNone  )
                {
                //len = TRap::Register32(KRapRegXTIASR001_DATA) << i;
                self.iMessagelength += TRap::Register32(KRapRegXTIASR001_DATA) << i;
                }
            //__DEBUG_ONLY (Kern::Printf("XtiRx::ReceiveDataDfc - len = %d", len) );
            //self.iMessagelength += len;
            }
        __DEBUG_ONLY (Kern::Printf("XtiRx::ReceiveDataDfc - iMessagelength = %d", self.iMessagelength) );
        // acknowledgement to Musti
        
        if( r == KErrNone )
            {
            XtiTx::SendDataLenAckMsg();
            self.iRxState = KRxStateWaitDataMsg; 
            }
        else
            {
            self.iXtiRxPtr->iReceiveBufferFree = ETrue;
            self.iXtiRxPtr->iRxBufferPtr->Zero();  // set data length to zero
            self.iRxState = KRxStateIdle;
            }
        XTI_TRACE( Kern::Printf("XtiRx::ReceiveDataDfc - Enable uart interrupt 0x%x", KInterruptId) )  
        Interrupt::Enable( KInterruptId );
        }
    else
        {
        XTI_TRACE( Kern::Printf("XtiRx::ReceiveDataDfc - XTI state = KRxStateWaitDataMsg") )
        ret = self.ReadData();
        if( ret == KErrNone )
            {
            // acknowledgement to Musti
            XtiTx::SendDataRxAckMsg();   
            self.iReceiveBufferFree = EFalse;
            // Cancel reset timer
            XTI_TRACE( Kern::Printf("XtiRx::ReceiveDataDfc - Cancel reset timer") )
            self.iIdleTimer.Cancel();
            // inform Trace core that the data is in the receive buffer
            self.iTraceCoreDfcPtr->Enque();
            // Start standby timer
            XTI_TRACE( Kern::Printf("XtiRx::ReceiveDataDfc - Start standby timer") ) 
            self.iRxState = KRxStateIdle;
            self.iIdleTimer.OneShot( KIdleTimerInterval, XtiRx::StandbyMode, aPtr );
            }
        XTI_TRACE( Kern::Printf("XtiRx::ReceiveDataDfc - Enable uart interrupt 0x%x", KInterruptId) )  
        Interrupt::Enable( KInterruptId );
        }  
    XTI_TRACE( Kern::Printf("XtiRx::ReceiveDataDfc <<") )
    }


/*
----------------------------------------------------------------------------

    XtiRx

    GPIOWakeupDfc

    Function called in DFC context after GPIO interrupt. Mux the UART PIN and
    start UART clocks
    
    @param   aPtr  Pointer to XtiRx object    

-----------------------------------------------------------------------------
*/
void XtiRx::GPIOWakeupDfc( TAny* aPtr )
{
    XTI_TRACE( Kern::Printf("XtiRx::GPIOWakeupDfc >>") )
    __ASSERT_DEBUG(aPtr != NULL, Kern::Fault( ("XtiRx::GPIOWakeupDfc - Null pointer!"), 0 ));

    XtiRx& self = *static_cast< XtiRx* >( aPtr );

    self.SetPowerRequirements();
    
    XTI_TRACE( Kern::Printf("XtiRx::GPIOWakeupDfc - Enable uart interrupt 0x%x", KInterruptId) )
    TInt ret = Interrupt::Enable( KInterruptId );
    __ASSERT_DEBUG(ret == KErrNone, Kern::Fault( ("XtiRx::GPIOWakeupDfc"), __LINE__ ));   
          
    XTI_TRACE( Kern::Printf("XtiRx::GPIOWakeupDfc <<" ) )
}


/*
----------------------------------------------------------------------------

    XtiRx

    GPIODeepSleepWakeupDfc

    Function called in DFC context after GPIO interrupt in deep sleep. Mux the UART PIN and
    start UART clocks
    
    @param   aPtr  Pointer to XtiRx object    

-----------------------------------------------------------------------------
*/
void XtiRx::GPIODeepSleepWakeupDfc( TAny* aPtr )
{
    XTI_TRACE( Kern::Printf("XtiRx::GPIODeepSleepWakeupDfc >>") )
    __ASSERT_DEBUG(aPtr != NULL, Kern::Fault( ("XtiRx::GPIODeepSleepWakeupDfc - Null pointer!"), 0 ));

    XtiRx& self = *static_cast< XtiRx* >( aPtr );

    XTI_TRACE( Kern::Printf("XtiRx::GPIODeepSleepWakeupDfc - Bind method Isr to uart interrupt 0x%x", KInterruptId) ) 
    TInt ret = Interrupt::Bind( KInterruptId, Isr, aPtr );

    self.SetPowerRequirements();

    self.ConfigureUart();

    XTI_TRACE( Kern::Printf("XtiRx::GPIODeepSleepWakeupDfc - Enable uart interrupt 0x%x", KInterruptId) )
    ret = Interrupt::Enable( KInterruptId );
    __ASSERT_DEBUG(ret == KErrNone, Kern::Fault( ("XtiRx::GPIODeepSleepWakeupDfc"), __LINE__ ));   
          
    XTI_TRACE( Kern::Printf("XtiRx::GPIODeepSleepWakeupDfc <<" ) )
}

/*
----------------------------------------------------------------------------

    XtiRx

    SetPowerDfc

    Call SetPowerRequirements
    
    @param   aPtr  Pointer to XtiRx object    

-----------------------------------------------------------------------------
*/
void XtiRx::SetPowerDfc( TAny* aPtr )
{
    XTI_TRACE( Kern::Printf("XtiRx::SetPowerDfc >>") )
    __ASSERT_DEBUG(aPtr != NULL, Kern::Fault( ("XtiRx::SetPowerDfc - Null pointer!"), 0 ));

    XtiRx& self = *static_cast< XtiRx* >( aPtr );

    self.SetPowerRequirements();

    XTI_TRACE( Kern::Printf( "XtiRx::SetPowerDfc <<" ) )
}


/*
----------------------------------------------------------------------------

    XtiRx

    ClearPowerDfc

    Call ClearPowerRequirements
    
    @param   aPtr  Pointer to XtiRx object    

-----------------------------------------------------------------------------
*/
void XtiRx::ClearPowerDfc( TAny* aPtr )
{
    XTI_TRACE( Kern::Printf("XtiRx::ClearPowerDfc >>") )
    __ASSERT_DEBUG(aPtr != NULL, Kern::Fault( ("XtiRx::ClearPowerDfc - Null pointer!"), 0 ));

    XtiRx& self = *static_cast< XtiRx* >( aPtr );

    self.ClearPowerRequirements();

    XTI_TRACE( Kern::Printf( "XtiRx::ClearPowerDfc <<" ) )
}
         
         

/*
----------------------------------------------------------------------------

    XtiRx

    ReadRxData

    Read data from Uart RX FIFO

    Return Value: KErrNone or error code
-----------------------------------------------------------------------------
*/
TInt XtiRx::ReadData()
    {
    XTI_TRACE( Kern::Printf("XtiRx::ReadData >>") )
    TInt ret( KErrNone );
    TInt error( KErrNone );
    TUint32 character( 0 );
    __ASSERT_DEBUG(iRxBufferPtr != NULL, Kern::Fault( ("XtiRx::ReadData - Null pointer!"), 0 ));
    iRxBufferPtr->Zero();  // set data length to zero

    if( iMessagelength > KMaxXtiMessageLength )
        {     
        XTI_TRACE( Kern::Printf("XtiRx::ReadData - Message is greater than the maximum size of XTI message") )
        ret = KErrGeneral;
        }                    
    else
        {
        while( iMessagelength != 0 && ret == KErrNone )
            {           
            ret = Kern::PollingWait(HasUartRxFifoData, this , KPollPeriodMs, KMaxPoll);
            if( ret == KErrNone  )
                {
                character = TRap::Register32(KRapRegXTIASR001_DATA);
                XTI_TRACE( Kern::Printf("XtiRx::ReadData - data = 0x%x",character) )
                error = character >> KBitsInByte;
                character = character & KMaxTUint8;
  
                /*Check for receive errors
                Error flags are OR'ed into the high bits of char
                */
 
                if( error & ASR001_T_V_ERROR_OVERRUN) 
                    {
                     XTI_TRACE( Kern::Printf("XtiRx::ReadData - Overrun ERROR") ) 
                    ret = KErrCommsOverrun;  
                    }
                if( error & ASR001_T_V_ERROR_PARITY)
                    {
                    XTI_TRACE( Kern::Printf("XtiRx::ReadData - Parity ERROR") ) 
                    ret = KErrCommsParity;
                    }
                if( error & ASR001_T_V_ERROR_FRAMING )
                    {
                    XTI_TRACE( Kern::Printf("XtiRx::ReadData - Framing ERROR") )
                    ret = KErrCommsFrame;
                    }

                iRxBufferPtr->Append( character );
                iMessagelength--;
                }
            }
        }

    XTI_TRACE( Kern::Printf("XtiRx::ReadData <<, ret= %d", ret) )
    return ret;
    }


/*
----------------------------------------------------------------------------

    XtiRx

    Register

    This method is used to register an user to the driver.
    
    @param   aDfc         Pointer to the DFC function which is called by the XTI driver when the data is
                          available in the receive buffer.
    
    @param   aBuf         Receive buffer 

    Return Value:         KErrNone or error code  

-----------------------------------------------------------------------------
*/
EXPORT_C TInt XtiRx::Register( TDfc* aDfc, TDes8& aBuf  )
    {
    __ASSERT_DEBUG(aDfc != NULL, Kern::Fault( ("XtiRx::Register - Null pointer!"), 0 ));
    XTI_TRACE( Kern::Printf("XtiRx::Register >>") )
    TInt ret(KErrNone);

    // Create DFC thread inside of which the operations are performed
    if( iXtiRxPtr != NULL )
        {
        ret = Kern::DfcQCreate( iXtiRxPtr->iDfcQueue, KDfcThreadPriority, &KThreadName );
        }
     else
        {
        ret = KErrGeneral;
        }
        
    if(ret == KErrNone )
        {
        iXtiRxPtr->iReceiveDataDfc.SetDfcQ( iXtiRxPtr->iDfcQueue );
        iXtiRxPtr->iGPIOWakeupDfc.SetDfcQ( iXtiRxPtr->iDfcQueue );
        iXtiRxPtr->iSetPowerDfc.SetDfcQ( iXtiRxPtr->iDfcQueue );
        iXtiRxPtr->iClearPowerDfc.SetDfcQ( iXtiRxPtr->iDfcQueue );

        ret = iXtiRxPtr->Init();
        if( ret == KErrNone )
            {
            iXtiRxPtr->iTraceCoreDfcPtr = aDfc;
            iXtiRxPtr->iRxBufferPtr = &aBuf;
            iXtiRxPtr->iReceiveBufferFree = ETrue;
            }
        }
               
    XTI_TRACE( Kern::Printf("XtiRx::Register <<, ret= %d",ret) )
    iXtiRxPtr->ClearPowerRequirements();
    return ret;
    }


/*
----------------------------------------------------------------------------

    XtiRx

    SetReceiveBufferRead

    User of the driver calls this function when the receive buffer has been read

-----------------------------------------------------------------------------
*/
EXPORT_C void XtiRx::SetReceiveBufferRead()
    {
    XTI_TRACE( Kern::Printf("XtiRx::SetReceiveBufferRead >>") )
    iXtiRxPtr->iReceiveBufferFree = ETrue;
    iXtiRxPtr->iRxBufferPtr->Zero();  // set data length to zero
    XTI_TRACE( Kern::Printf("XtiRx::SetReceiveBufferRead <<") )
    }


/*
----------------------------------------------------------------------------

    XtiRx
    
    Start
                
    Start receiving characters
    
    Return Value:         KErrNone or error code

-----------------------------------------------------------------------------
*/

TInt XtiRx::Start() const 
    {   
    XTI_TRACE( Kern::Printf( "XtiRx::Start >>") )
    TInt ret(KErrNone);

    // Clear Uart RX buffer
     XTI_TRACE( Kern::Printf( "XtiRx::Start - Clear Uart RX buffer") )   
    while(  TRap::Register32(KRapRegXTIASR001_GAUGE) )
        { 
         TRap::Register32(KRapRegXTIASR001_DATA);
        }
        
    if( ret == KErrNone )    
        {
        // Enable Uart interrupt 
        XTI_TRACE( Kern::Printf( "XtiRx::Start - Enable uart interrupt  0x%x", KInterruptId) )
        ret = Interrupt::Enable( KInterruptId );
        }
    XTI_TRACE( Kern::Printf( "XtiRx::Start <<, ret= %d",ret) )
    return ret;
    }
    

/*
----------------------------------------------------------------------------

    XtiRx
    
    SetPowerRequirements
                
    Set power requirements
    
-----------------------------------------------------------------------------
*/
void XtiRx::SetPowerRequirements() 
    {   
    XTI_TRACE( Kern::Printf( "XtiRx::SetPowerRequirements >>") )
    
    if( iClocksOn == EFalse )
        {
        // Request to enable a clock resource
        XTI_TRACE( Kern::Printf( "XtiRx::SetPowerRequirements - Request to enable a clock resource") )        
      
        iClocksOn = ETrue;
        }
    XTI_TRACE( Kern::Printf( "XtiRx::SetPowerRequirements <<" ) )
    }


/*
----------------------------------------------------------------------------

    XtiRx
    
    ClearPowerRequirements
                
    Clear power requirements
    
-----------------------------------------------------------------------------
*/
void XtiRx::ClearPowerRequirements() 
    {   
    XTI_TRACE( Kern::Printf( "XtiRx::ClearPowerRequirements >>") )
    
    if( iClocksOn )
        {
        iClocksOn = EFalse;
        }
    XTI_TRACE( Kern::Printf( "XtiRx::ClearPowerRequirements <<" ) )
    }


/*
----------------------------------------------------------------------------

    XtiRx
    
    StandbyMode
                
    Set the driver to standby mode
    
-----------------------------------------------------------------------------
*/
void XtiRx::StandbyMode( TAny* aPtr ) 
    {
    XTI_TRACE( Kern::Printf( "XtiRx::StandbyMode >>") )
    __ASSERT_DEBUG(aPtr != NULL, Kern::Fault( ("XtiRx::StandbyMode - Null pointer!"), 0 ));
    XtiRx& self = *static_cast< XtiRx* >( aPtr );
    XTI_TRACE( Kern::Printf( "XtiRx::StandbyMode - Enque ClearPowerDfc") )    
    self.iClearPowerDfc.Enque();
    self.iRxState = KRxStateIdle;
    XTI_TRACE( Kern::Printf( "XtiRx::StandbyMode <<" ) )
    }    
    

/*
----------------------------------------------------------------------------

    XtiRx
    
    ResetReceiveState
                
    Reset receiver state machine.
    
-----------------------------------------------------------------------------
*/
void XtiRx::ResetReceiveState( TAny* aPtr ) 
    {
    XTI_TRACE( Kern::Printf( "XtiRx::ResetReceiveState >>") )
    __ASSERT_DEBUG(aPtr != NULL, Kern::Fault( ("XtiRx::ResetReceiveState - Null pointer!"), 0 ));
    XtiRx& self = *static_cast< XtiRx* >( aPtr );
    self.iXtiRxPtr->iReceiveBufferFree = ETrue;
    self.iXtiRxPtr->iRxBufferPtr->Zero();  // set data length to zero
    XTI_TRACE( Kern::Printf( "XtiRx::ResetReceiveState - Enque ClearPowerDfc") )    
    self.iClearPowerDfc.Enque();
    self.iRxState = KRxStateIdle;
    XTI_TRACE( Kern::Printf( "XtiRx::ResetReceiveState <<" ) )
    }   
    
    
    
    
//===========================================================================

//- Global Functions --------------------------------------------------------
                  
/*
----------------------------------------------------------------------------

    HasUartRxFifoData
                
    Check if there is data in the Uart RX FiFo. 
        
    @param   aPtr  Pointer to XtiRx object 

    Return Values: ETrue ot EFalse

-----------------------------------------------------------------------------
*/
TBool HasUartRxFifoData( TAny* aPtr  )
  {
    __ASSERT_DEBUG(aPtr != NULL, Kern::Fault( ("XtiRx::HasUartRxFifoData - Null pointer!"), 0 ));
    TBool ret( ETrue );
  
    if( TRap::Register32(KRapRegXTIASR001_GAUGE) == 0 )
        {
        XTI_TRACE( Kern::Printf("XtiRx::HasUartRxFifoData - No data in RX FIFO") )
        ret = EFalse;
        }
        
    return ret;
  }


//---------------------------------------------------------------------------
/**
    The entry point for a standard extension. Creates Xti extension.

    @return KErrNone, if successful
*/
DECLARE_STANDARD_EXTENSION()
    {
    TInt ret( KErrNoMemory );
    XtiRx* xtirx = XtiRx::Instance();
    if ( xtirx != NULL )
        {
        ret = KErrNone;
        }
    return ret;
    }                 

//- Namespace ---------------------------------------------------------------

// End of File
