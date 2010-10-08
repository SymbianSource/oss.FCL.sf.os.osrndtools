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
// Interface class for XTI IF. 


#ifndef __TRACECOREXTIIF_H__
#define __TRACECOREXTIIF_H__


// Include files
#include <kernel/kernel.h>
#include <TraceCoreMediaPlugin.h>
#include "TraceCoreMediaIf.h"

/**
 * Interface class for XTI
 */
NONSHARABLE_CLASS( DTraceCoreXtiIf ) : public DTraceCoreMediaPlugin
    {
public:

    /**
     * Constructor
     */
    DTraceCoreXtiIf();
    
    /**
     * Destructor
     */
    ~DTraceCoreXtiIf();
    
    /**
     * Initializes this interface
     */
    TInt Init( );
        
	 /**
	  * Sends a message
	  *
	  * @param aMessage The message to be sent
	  */
	 TInt Send( TTraceMessage &aMessage );
    
    /**
     * Called by TraceCore to send a message out
     */
    virtual TInt SendMessage( TTraceMessage &aMsg );
    
private:

    /**
     * Starts a timer which calls XtiRegister
     */
	void StartTimer();
	
	/**
	 * Timer callback function, register trace core to XTI driver
	 */
    static void XtiRegister( TAny* aPtr );
    
    /**
     * Called from the static timer callback function to perform registration
     */
    void XtiRegister();
    
    /**
     * DFC function for incoming data from XTI driver
     */
    static void ReceiveDfc( TAny* aPtr );
    
    /**
     * Called from the static DFC function to process incoming data
     */
    void ReceiveDfc();
    
    /**
     * Add registering to own Dfc
     */
    static void XtiRegisterDfc( TAny* aPtr );
    
    /**
     * Forwards incoming message to callback
     */
    void NotifyCallback();

    /**
     * Sends message to XTI
     * 
     * @param aMsg Message to be sent.
     * @return KErrNone if send successful
     */
    void SendData( TTraceMessage& aMsg );
    
private:

    /**
     * DFC which is used for incoming ISI-messages
     */
    TDfc iReceiveDfc;
    
    /**
     * DFC which is used to receive data from XTI driver
     */
    TDfc iXtiRegisterDfc;
    
    /**
     * Timer which is used for registration to XTI driver
     */
    TTickLink iXtiRegisterTimer;
    
    /**
     * ISA Kernel IF Initialisation status
     */
    TInt iStatus;
    
    /** 
     * Length variable for ISA Kernel interface usage
     */
    TUint16 iNeededLength;
 
     /**
     * Buffer for sending data
     **/
    TBuf8< 8204 > iSendBuffer;
       
    /**
     * Buffer for incoming data
     **/
    TBuf8< 8204 > iReceiveBuffer;
    };

#endif // __TRACECOREXTIIF_H__

// End of File
