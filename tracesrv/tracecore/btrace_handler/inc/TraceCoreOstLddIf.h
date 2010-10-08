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
// Interface class for the TraceCoreOstLdd component. 
// 

#ifndef __TraceCoreOstLddIf_H__
#define __TraceCoreOstLddIf_H__


// Include files
#include <kernel/kernel.h>
#include "TraceCoreMediaIf.h"
#include "TraceCoreOstConnectionIf.h"

/**
 * ETraceCoreOstLddIfGetSet function to choosed in DTraceCoreOstLddI::GetSet.
 */
enum TTraceCoreOstLddIfGetSet
    {
    EGetDfc = 0, // Return type TDfc*
    EGetBuf = 1,     // Return type TDes8*
    ESetWriter = 2,   // Return type 0
    ERegister  // Return type 0
    };

/**
 * Interface class for XTI
 */
class DTraceCoreOstLddIf : public DTraceCoreMediaIf
    {
public:

    /**
     * Constructor
     */
    DTraceCoreOstLddIf();
    
    /**
     * Destructor
     */
    ~DTraceCoreOstLddIf();
    
    /**
     * Initializes this interface
     * 
     * @param aCallback Callback interface which is notified when a message is received from the XTI driver
     */
	TInt Init( MTraceCoreMediaIfCallback& aCallback );
	
	/**
	 * Sends a message
	 *
	 * @param aMessage The message to be sent
	 */
	TInt Send( TTraceMessage &aMessage );

	/**
	 * 
	 * @param aFunc Function choosed (@see ETraceCoreOstLddIfGetSet)
	 * @return TDfc* if aFunc is EGetDfc, TDes8* EGetBuf and 0 with ESetWriter.
	 */
	IMPORT_C static TUint32 GetSet(TTraceCoreOstLddIfGetSet aFunc, TUint32 aParam);
	
private:
    
    /**
     * DFC function for incoming data from XTI driver
     */
    static void ReceiveDfc( TAny* aPtr );
    
    /**
     * Called from the static DFC function to process incoming data
     */
    void ReceiveDfc();
    
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
   
    /**
    * Similar to send, except does not generate traces, since they would loop back to this function
    * 
    * @param aMsg Message to be sent.
    * @return KErrNone if send successful
    */
    TInt SendTrace( const TDesC8& aMsg );
    
private:

    /**
     * DFC which is used for incoming ISI-messages
     */
    TDfc iReceiveDfc;
    
    /**
     * ISA Kernel IF Initialisation status
     */
    TInt iStatus;
    
    /** 
     * Length variable for ISA Kernel interface usage
     */
    TUint16 iNeededLength;
    
    TBool iTraceCoreOstConnectionRegistered;
    
     /**
     * Buffer for sending data
     **/
    TBuf8< 8204 > iSendBuffer;
       
    /**
     * Buffer for incoming data
     **/
    TBuf8< 8204 > iReceiveBuffer;
    
    /**
     * Pointer to TraceCoreOstConnectionIf
     **/
    MTraceCoreOstConnectionIf *iTraceCoreOstConnection;
    
    };

#endif // __TraceCoreOstLddIf_H__

// End of File
