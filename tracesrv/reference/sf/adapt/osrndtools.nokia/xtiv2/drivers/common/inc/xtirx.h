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

XTI interface class for RAPU PlatSim

*///=========================================================================


#ifndef __XTIRX_H__
#define __XTIRX_H__


//- Include Files  ----------------------------------------------------------

#include <kernel.h>
#include <d32comm.h>

//- Namespace ---------------------------------------------------------------


//- Using -------------------------------------------------------------------


//- Data Types --------------------------------------------------------------


//- Constants ---------------------------------------------------------------


//- Macros ------------------------------------------------------------------


//- External Data -----------------------------------------------------------


//- Variables ---------------------------------------------------------------


//- Forward Declarations ----------------------------------------------------


//- Class Definitions -------------------------------------------------------

/**

Interface class 				

*/
class XtiRx : public DBase 
    {			  
    enum TRxState
	    {
		KRxStateIdle,           
		KRxStateWaitDataLenMsg,
        KRxStateWaitDataMsg
        };

    public:
        static XtiRx* Instance();

        /**
         * This method is used to register a user to the driver.
         * @param aDfc Pointer to the DFC function which is called by the XTI driver when the data is available in the receive buffer.
         * @param aBuf Receive buffer
         * @return Standard Symbian error code
         */
        IMPORT_C static TInt Register( TDfc* aDfc, TDes8& aBuf );

        /**
         * User of the driver calls this function when the receive buffer has been read
         */
        IMPORT_C static void SetReceiveBufferRead();

    private:
        XtiRx();    
       ~XtiRx();
        static void Isr(TAny* aPtr);
        TInt Configure() const;
        TInt ConfigureUart() const;
		static void ReceiveDataDfc( TAny* aPtr );
		static void GPIOWakeupDfc( TAny* aPtr );
        static void GPIODeepSleepWakeupDfc( TAny* aPtr );
		TInt Start() const;
        TInt Init();
		TInt ReadData();
		void SetPowerRequirements();
		void ClearPowerRequirements();
        static void StandbyMode( TAny* aPtr );
        static void ResetReceiveState( TAny* aPtr );
        static void GPIOwakeIsr(TAny* aPtr);
        static void SetPowerDfc(TAny* aPtr);
        static void ClearPowerDfc(TAny* aPtr);

    private:
        TDfcQue*            iDfcQueue;
        TDfc                iReceiveDataDfc;
        TDfc                iGPIOWakeupDfc;
        TDfc                iSetPowerDfc;
        TDfc                iClearPowerDfc;
        static XtiRx*       iXtiRxPtr;
        TBool               iClocksOn;	
        TDfc*               iTraceCoreDfcPtr;
		TDes8*              iRxBufferPtr;
	  	TRxState            iRxState;
        TBool               iReceiveBufferFree;
        TUint32             iMessagelength;
        TTickLink           iWakeupResponseTimer;
		TInt                iWakeupResponseCounter;
        TTickLink           iIdleTimer;
        TUint               iClientId;
    };


//- Global Function Prototypes ----------------------------------------------


//- Inline Functions --------------------------------------------------------


//- Namespace ---------------------------------------------------------------

#endif // __XTIRX_H__

// End of File
