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
// Handler implementation for ascii printf
//

#ifndef __TRACECOREPRINTFTRACEHANDLER_H__
#define __TRACECOREPRINTFTRACEHANDLER_H__


// Include files
#include <kernel/kernel.h>
#include <TraceCoreNotificationReceiver.h>
#include "TraceCoreHandler.h"


// Forward declarations
class DTraceCoreWriter;
class DTraceCoreSettings;


/**
 * Handler implementation for ascii printf
 */
NONSHARABLE_CLASS( DTraceCorePrintfTraceHandler ) : public MTraceCoreNotificationReceiver, 
													public DTraceCoreHandler
		                                                   
    {
public:

	/**
     * Constructor
     */
    DTraceCorePrintfTraceHandler();
    
    /**
     * Destructor
     */
    ~DTraceCorePrintfTraceHandler();
    
    /**
     * Initializes this handler
     */
    TInt Init();
    
    /**
     * Prepares the change in writer
     * 
     * @param aWriter the new writer
     */
    void PrepareSetWriter( DTraceCoreWriter* aWriter );
  
    /**
     * Kern/RDebug::Printf handler function
     */
    static TBool PrintfHandler( const TDesC8& aText, TTraceSource aTraceSource );
    
    /**
     * Callback function for Trace Activation
     * from MTraceCoreNotificationReceiver
     * 
     * @param aComponentId
     * @param aGroupId
     * @return None          
     */
    void TraceActivated( TUint32 aComponentId, TUint16 aGroupId  );
    
    /**
     * Callback function for Trace Deactivation
     * from MTraceCoreNotificationReceiver
     * 
     * @param aComponentId
     * @param aGroupId
     * @return None     
     */
    void TraceDeactivated( TUint32 aComponentId, TUint16 aGroupId  );
    
private:
    
    /**
     * Checks if there is a dropped trace and sends dropped trace message and
     * normal trace as well if writer is ready to write that data.
     *
     * @param aHandler Printf trace handler
     */
    inline static void HandleTrace(const TDesC8& aText);    

    /**
     * Trace output function
     */
	void DebugPrint( const TDesC8& aDes );
    
    /**
     * Static instance, accessed from the printf trace handler callback
     */
    static DTraceCorePrintfTraceHandler* iInstance;
    
    /**
     * Printf trace handler
     */
    TBool ( *iPrintfHandler )( const TDesC8&, TTraceSource );
    
    /**
	 * State of the Printf activations
 	 */
	static TBool iKernPrintfActive;
	static TBool iRDebugPrintfActive;
	static TBool iPlatSecPrintfActive;

    };

#endif

// End of File
